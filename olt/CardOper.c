
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
*   Date: 			2006/05/19
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  06/05/19 |   chenfj          |     create 
**----------|-----------|------------------
**
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "PonEventHandler.h"
#include  "V2R1_product.h"

/*|-----------------------------------
 *|  card type       hard slot        soft slot
 6700 cardslot issue:
 *|-----------------------------------
 *|   ETH                0                
 *|-----------------------------------
 *|   TDM               1
 *|-----------------------------------
 *|   SW1               2                    1
 *|-----------------------------------
 *|   SW2/PON5      3                    2
 *|-----------------------------------
 *|   PON4             4                     3 
 *|-----------------------------------
 *|   PON3             5                     4
 *|-----------------------------------
 *|   PON2             6                     5
 *|-----------------------------------
 *|   PON1             7                     6 
 *|-----------------------------------
 *|  PWR1              8               
 *|-----------------------------------
 *|  PWR2              9
 *|-----------------------------------
 *|  PWR3             10 
 *|-----------------------------------
 6100 cardslot issue:
 *|-----------------------------------
 *|  card type       hard slot        soft slot
 *|-----------------------------------
 *|   ETH                0                
 *|-----------------------------------
 *|   SW/PON1       1                    
 *|-----------------------------------
 *|   PON2              2                    
 *|-----------------------------------
 *|   PWR1             3               
 *|-----------------------------------
 *|  PWR2              4
 *|-----------------------------------
 */
/*
#define  V2R1_FPAG_BASE 0xff800000
#define  V2R1_SLOT_OFFSET1  21
#define  V2R1_SLOT_OFFSET2  22
#define  MAXPONCARD 5
*/
bool  CTC_STACK_Init = FALSE;
unsigned char *CardStatus[]= 
	{
	(unsigned char *)"unknown",
	(unsigned char *)"NotLoading",
	(unsigned char *)"Loading",
	(unsigned char *)"LoadComp",
	(unsigned char *)"Activated",
	(unsigned char *)"error",
	(unsigned char *)"deleted"
	};

unsigned char PonCardStatus[PRODUCT_MAX_TOTAL_SLOTNUM+1]; 
extern int OnuEvent_ClearRunningDataMsg_Send(short int PonPortIdx);

/* �忨��λ*/
extern LONG bms_board_reset( ULONG slotno, struct vty* vty );
/*extern short int Remove_olt ( const short int  olt_id,
					   const bool		send_shutdown_msg_to_olt,
					   const bool       reset_olt);*/
extern LONG ETH_SetDevNameBySlotPort( UCHAR *pucName, ULONG ulSlot, ULONG ulPort );

#if(EPON_MODULE_ONU_MAC_OVERFLOW_CHECK==EPON_MODULE_YES)
extern LONG clearOnuMacCheckAlarmStateBySlot( ULONG slotno, ULONG moduleType );
#endif

#if 0
void  SearchAndInsertPonCard()
{
	unsigned char CardInfo, i;
	unsigned long CardIndex;

	CardInfo = *(unsigned char *)(V2R1_FPAG_BASE + V2R1_SLOT_OFFSET1 );

	CardInfo = (CardInfo & 0xf8) >> 3;
	/*sys_console_printf("\r\n the card slot %d \r\n", CardInfo );*/

	/* new card inserted */
	for( i=0; i< MAXPONCARD; i++ )
		{
		if( CardInfo & ( 1<< i) )
			{
			switch(i)
				{
				case 0:	CardIndex = PON5;
				break;
				case 1: CardIndex = PON4;
				break;
				case 2: CardIndex = PON3;
				break;
				case 3: CardIndex = PON2;
				break;
				case 4: CardIndex = PON1;
				break;	
				default: CardIndex = 0;
				}

			if( GetOltCardslotInserted ( CardIndex ) == CARDNOTINSERT )
				{
				PonCardInserted( CardIndex+1 );
				/*SetOltCardslotInserted( CardIndex );*/
				}
			}		
		}	

	/* card pulled */
	for( i=0; i< MAXPONCARD; i++ )
		{
		if( !( CardInfo & ( 1<< i) ))
			{
			switch(i)
				{
				case 0:	CardIndex = PON5;
				break;
				case 1: CardIndex = PON4;
				break;
				case 2: CardIndex = PON3;
				break;
				case 3: CardIndex = PON2;
				break;
				case 4: CardIndex = PON1;
				break;	
				default: CardIndex = 0;
				}

			if( GetOltCardslotInserted ( CardIndex ) == CARDINSERT )
				PonCardPulled( CardIndex+1 );
			}		
		}	
}
#endif

void InitPonCardMgmtStatus(void)
{
	int i;

	for(i=1; i<=PRODUCT_MAX_TOTAL_SLOTNUM; i++)
		PonCardStatus[i] = PONCARD_NOTLOADING;
}

void ClearPonPortStatus( unsigned long CardIndex )
{
	short int PonPortIdx;
	short int i;

	PonPortIdx = GetPonPortIdxBySlot(CardIndex, FIRSTPONPORTPERCARD);
	if( PonPortIdx == VOS_ERROR )	/* modified by xieshl 20080812 */
	{
		VOS_ASSERT(0);
		return;
	}

	for(i=0; i<PONPORTPERCARD; i++)
	{
		PonChipMgmtTable[PonPortIdx+i].operStatus     =  PONCHIP_NOTLOADING;
		PonPortTable[PonPortIdx+i].PortWorkingStatus  = PONPORT_UNKNOWN;
	}
}

void UplinkCardInserted( unsigned long CardIndex )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_CARD_INSERTED, 0, 0};

	/*
	if(( CardIndex > PON1 ) ||( CardIndex < PON5 ))
		{
		sys_console_printf(" slot %d is not pon card(pon card insert) \r\n", (CardIndex));
		return;
		}
	*/
#if 0
	if( !device_chassis_is_slot_inserted(CardIndex) )	/* modified by xieshl 20100120, ���ⵥ9583 */
	{
		SetOltCardslotPulled(CardIndex);
		sys_console_printf(" slot %d is not inserted\r\n", (CardIndex));
		return;
	}
#endif      
	if(SlotCardIsPonBoard(CardIndex) != ROK )
	{
		sys_console_printf(" slot %d is not pon card(pon card insert)\r\n", (CardIndex));
		return;
	}
	
	aulMsg[3] = CardIndex;

	/*
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/

	if( GetOltCardslotInserted( CardIndex) == CARDNOTINSERT )
	{
		SetOltCardslotInserted( CardIndex );
		PonCardStatus[CardIndex] = PONCARD_NOTLOADING;
		/*ModifyCardNameBySlot(CardIndex+1);*/
		/*clear Pon port status to Initial 
		ClearPonPortStatus(CardIndex+1);*/
	}
	else
    {
		if(EVENT_DEBUG == V2R1_ENABLE)
			sys_console_printf("    %s is inserted already\r\n", CardSlot_s[CardIndex] );
		return;
	}

	if( g_PonUpdate_Queue_Id  == 0 )
	{
		VOS_ASSERT(0);
		/*sys_console_printf("  error: VOS can not create queue g_PonUpdate_Queue_Id\r\n" );*/
		return;
	}

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		/*VOS_ASSERT(0);*/
		/*sys_console_printf("  error: VOS send message err\r\n"  );*/
	}

}

extern LONG device_chassis_is_slot_inserted( ULONG slotno );
extern LONG devsm_sys_is_switchhovering();
void PonCardInserted( unsigned long module_type, unsigned long CardIndex )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_CARD_INSERTED, 0, 0};

	/*
	if(( CardIndex > PON1 ) ||( CardIndex < PON5 ))
		{
		sys_console_printf(" slot %d is not pon card(pon card insert) \r\n", (CardIndex));
		return;
		}
	*/
#if 0
	if( !device_chassis_is_slot_inserted(CardIndex) )	/* modified by xieshl 20100120, ���ⵥ9583 */
	{
		SetOltCardslotPulled(CardIndex);
		sys_console_printf(" slot %d is not inserted\r\n", (CardIndex));
		return;
	}
#endif   
	
	if(SlotCardIsPonBoard(CardIndex) != ROK )
	{
		sys_console_printf(" slot %d is not pon card(pon card insert)\r\n", (CardIndex));
		return;
	}
	
        if( devsm_sys_is_switchhovering() )
		aulMsg[2] = 1;
	aulMsg[2] = (aulMsg[2] | (module_type << 8));
	aulMsg[3] = CardIndex;

	/*
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/

	if( GetOltCardslotInserted( CardIndex) == CARDNOTINSERT )
	{
		SetOltCardslotInserted( CardIndex );
		PonCardStatus[CardIndex] = PONCARD_NOTLOADING;
		/*ModifyCardNameBySlot(CardIndex+1);*/
		/*clear Pon port status to Initial 
		ClearPonPortStatus(CardIndex+1);*/
	}
	else
    {
		if(EVENT_DEBUG == V2R1_ENABLE)
			sys_console_printf("    %s is inserted already\r\n", CardSlot_s[CardIndex] );
		return;
	}

	if( g_PonUpdate_Queue_Id  == 0 )
	{
		VOS_ASSERT(0);
		/*sys_console_printf("  error: VOS can not create queue g_PonUpdate_Queue_Id\r\n" );*/
		return;
	}

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		/*VOS_ASSERT(0);*/
		/*sys_console_printf("  error: VOS send message err\r\n"  );*/
	}

}

void PonCardPulled ( unsigned long module_type, unsigned long CardIndex )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_CARD_PULLED, 0, 0};
#if 0
	short int PonPortIdx, FirstPort;

	aulMsg[3] = CardIndex;
	CardIndex--;
	/*
	if(( CardIndex > PON1 ) ||( CardIndex < PON5 ))
		{
		sys_console_printf(" slot %d is not pon card(pon card pull)\r\n", (CardIndex +1));
		return;
		}
	*/
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_EPON )
		{
		sys_console_printf(" slot %d is not pon card(pon card pulled) \r\n", (CardIndex +1));
		return;
		}
	
	/*
	if( g_PonUpdate_Queue_Id == 0 ) g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/
	FirstPort = GetPonPortIdxBySlot(CardIndex,0);
	if( FirstPort == RERROR ) return ;
	for( PonPortIdx =0; PonPortIdx < PONPORTPERCARD; PonPortIdx++)
		{
		if( getPonChipInserted( (unsigned char)CardIndex , (unsigned char)PonPortIdx ) == PONCHIP_EXIST )
			{
			PonChipMgmtTable[PonPortIdx+FirstPort].operStatus = PONCHIP_DEL;
			PonPortTable[PonPortIdx+FirstPort].PortWorkingStatus = PONPORT_DEL;
			}
		}
	
	if( g_PonUpdate_Queue_Id  == 0 ) {
		VOS_ASSERT(0);
		sys_console_printf("  error: VOS can not create queue g_PonUpdate_Queue_Id\r\n" );
		return;
		}
#else
	aulMsg[2] = module_type;
	aulMsg[3] = CardIndex;
#endif
	PonCardStatus[CardIndex] = PONCARD_NOTLOADING;
	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		/*VOS_ASSERT(0);*/
		/*sys_console_printf("  error: VOS send message err\r\n"  );*/
	}

}

/* #define __T_PON_CONN_TIME__ */

UINT32
calculate_time_diff_ms( struct timeval * now, struct timeval * then )
{
    struct timeval tmp, diff;
    memcpy( &tmp, now, sizeof( struct timeval ) );
    tmp.tv_sec--;
    tmp.tv_usec += 1000000L;
    diff.tv_sec = tmp.tv_sec - then->tv_sec;
    diff.tv_usec = tmp.tv_usec - then->tv_usec;
    if ( diff.tv_usec > 1000000L )
    {
        diff.tv_usec -= 1000000L;
        diff.tv_sec++;
    }
    return ( ( diff.tv_sec * 1000 ) + ( diff.tv_usec / 1000 ) );
}

extern unsigned char reboot_unknown_type_times[] ;
extern int pon_load_flags;
void PonCardInsert(unsigned long module_type, unsigned long CardIndex, unsigned long switchhovering)
{
    short int PonPortIdx, i/*, j*/;
    unsigned char ponInserted, ponPhyPortNum;
    int ret;
    int pon_load_mode;
    int pon_load_needwait;
    int pon_load_timelimit;
    int pon_load_overflag;
    int PonIsLocalInsert;
    int PonChipVer;

        int iOltCurrMode;
        int iFirstResumeFailed;
        int iWaitPonChipInfoTimes;
#ifdef __T_PON_CONN_TIME__        
        struct timeval	      ponresume_starttime;
        static struct timeval ponresume_endtime = {0, 0};
        UINT32 uptime;
        struct timeval	      poncard_starttime;
#endif

#if 0
    if( GetOltCardslotInserted( CardIndex) == CARDNOTINSERT )
    {
    SetOltCardslotInserted( CardIndex );
    PonCardStatus[CardIndex] = PONCARD_NOTLOADING;
    /*ModifyCardNameBySlot(CardIndex+1);*/
    /*clear Pon port status to Initial 
    ClearPonPortStatus(CardIndex+1);*/
    }
    else {
    if(EVENT_DEBUG == V2R1_ENABLE)
    sys_console_printf("    %s is inserted already\r\n", CardSlot_s[CardIndex] );
    return;
    }
    if( !device_chassis_is_slot_inserted(CardIndex) )	/* modified by xieshl 20100120, ���ⵥ9583 */
    {
    SetOltCardslotPulled(CardIndex);
    sys_console_printf(" slot %d is not inserted\r\n", CardIndex);
    return;
    }

    if( GetOltCardslotInserted(CardIndex) == CARDNOTINSERT )
    {
    SetOltCardslotInserted(CardIndex);
    PonCardStatus[CardIndex] = PONCARD_NOTLOADING;
    /*ModifyCardNameBySlot(CardIndex+1);*/
    /*clear Pon port status to Initial 
    ClearPonPortStatus(CardIndex+1);*/
    }
#endif	

	
    /* B--modified by liwei056@2009-12-08 for New-SwitchOver Project */
    if(PonCardSlotRangeCheckByVty(CardIndex, NULL) != ROK )
    {
        sys_console_printf("\r\nPonCardInsert CardIndex=%d\r\n", CardIndex );
        VOS_ASSERT(0);
        return;
    }
	/* modified by xieshl 20120416, PON�����Ǹ��ݰ忨����������ģ��ڿ��ٰ忨��λʱ��
	    �忨���Ϳ��ܴ���unknown���������ʱû��PON�ڣ��ͻᷢ��PON���޷�������
	    ���ص����������С�����¼�*/
	ponPhyPortNum = GetSlotCardPonPortRange(module_type, CardIndex);
	if( (ponPhyPortNum == 0) || (ponPhyPortNum > CARD_MAX_PON_PORTNUM) )
	{
		sys_console_printf("\r\n slot%d: not find pon chip inserted, retry again\r\n", CardIndex );
		if( CDSMS_CHASSIS_SLOT_INSERTED(CardIndex) )
		{
			/* ע�ⲻ��ֱ�ӷ��أ������ض�һ�� */
			VOS_TaskDelay(100);
			ponPhyPortNum = GetSlotCardPonPortRange(0, CardIndex);
		}
		if( (ponPhyPortNum == 0) || (ponPhyPortNum > CARD_MAX_PON_PORTNUM) )
		{
			sys_console_printf("\r\n slot%d:pon chip inserted number=%d err\r\n", CardIndex, ponPhyPortNum );
			return;
		}
	}
	/*VOS_ASSERT((ponPhyPortNum >= 0) && (ponPhyPortNum <= CARD_MAX_PON_PORTNUM));*/

	/* modified by xieshl 201204016, PON��״̬���豸����״̬��һ��ʱ���ᵼ��PON���޷�������
	    �ر���GFA6700 slot 4�Ȱβ�ʱ����Լ��50%�ĸ��ʲ��������������Ҫ���°β�忨��
	    ������Ҫ�����ظ��Ĳ������ */
	if( SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
	{
		if( (PonCardStatus[CardIndex] == PONCARD_LOADCOMP) && ((PonPortIdx = GetPonPortIdxBySlot(CardIndex, 1)) != RERROR) )
		{
			ret = 1;
			for( i=0; i<ponPhyPortNum; i++ )
			{
				if( PonChipMgmtTable[PonPortIdx+i].operStatus != PONCHIP_LOADCOMP )
				{
					ret = 0;
					break;
				}
			}
			if( ret == 1 )
			{
				sys_console_printf("\r\n %s is inserted already\r\n", CardSlot_s[CardIndex] );
				return;
			}
		}
	}
	
#ifdef __T_PON_CONN_TIME__        
        gettimeofday(&poncard_starttime, NULL);
#endif

	sys_console_printf("  %s is to be inserting ... OK.\r\n", CardSlot_s[CardIndex]);

        /*
        ** Added by zhangxinhui, 2011-04-21
        ** Should reset the PON board if it hasn't CPU (Maybe GFA6700)
        ** Reset the board 
        */
        /*
        if ( !SYS_MODULE_SLOT_ISHAVECPU(CardIndex) )
        {
            CPI_HOOK_CALL(cdsms_sys_reset_ext ) ( CardIndex, NULL );
            VOS_TaskDelay(2*sysClkRateGet());
        }
        *//*MODIFY BY ZHAOZHG FOR PR 13947*/
        /*sys_console_printf(" OK.%d\r\n", switchhovering );*/

        pon_load_mode      = (SYS_MODULE_SLOT_ISHAVECPU(CardIndex) || (SYS_PRODUCT_TYPE == PRODUCT_E_GFA6100));/* By zhangxh, for GFA6700 */
        pon_load_needwait  = 0;

#if (defined(_EPON_10G_PMC_SUPPORT_ )  || defined(_GPON_BCM_SUPPORT_))          
		/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
		if (SYS_LOCAL_MODULE_TYPE_IS_6900_10G_EPON)
		{
		    pon_load_timelimit = 2000;
		}
		else if(SYS_LOCAL_MODULE_TYPE_IS_8000_GPON || SYS_LOCAL_MODULE_TYPE_IS_8100_GPON)
		{
			pon_load_timelimit = 2000;
		}
		else
#endif
		{
	        pon_load_timelimit = 60;
		}
        pon_load_overflag  = 0;
        pon_load_flags     = 0;

        iWaitPonChipInfoTimes = 5;
	/* modified by xieshl 20120409, �������ȡ��������״̬�ǲ�׼ȷ�ģ���Ϊ�޷���֤���������Ƿ��Ѿ����� */
        if( /*devsm_sys_is_switchhovering()*/switchhovering )
        {
            iFirstResumeFailed = -1;
        }
        else
        {
            iFirstResumeFailed = 0;
        }

#ifdef BCM_WARM_BOOT_SUPPORT
		/*added by liyang @2015-05-04*/
		if(IsWarmBoot())
			iFirstResumeFailed = -1;
#endif

        /* ��ʼ���µ� PON оƬ��Ϣ[��:PONоƬ��MAC��ַ] */
        /* get pon card Info from IIC interface */
        PonCardStatus[CardIndex] = PONCARD_LOADING;
        for(i=1; i<=ponPhyPortNum; i++ )
        {
            /* get pon card Info from IIC interface */
            GetPonChipInfo( CardIndex, i );
            PonPortIdx = GetPonPortIdxBySlot ( CardIndex, i);									
            if( PonPortIdx == RERROR ) continue;
            /*PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ONLINE;*/
                PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_NOTLOADING;			
        }

        ponInserted = ponPhyPortNum;
        for(i=1; i<=ponPhyPortNum; i++)
        {               
#ifdef __T_PON_CONN_TIME__    
            gettimeofday(&ponresume_starttime, NULL);
            uptime = calculate_time_diff_ms(&ponresume_starttime, &ponresume_endtime);
            sys_console_printf("    the prev-olt's conn to the curr-olt's conn take %lu(ms).\r\n", uptime);
#endif

            /* ��ȡ�²�����PON оƬ��λ��Ϣ */
            if( getPonChipInserted( (unsigned char)CardIndex, (unsigned char)i ) != PONCHIP_EXIST )
            {
                if (0 < --ponInserted)
                {
                    continue;
                }
                else
                {
                    if (0 < --iWaitPonChipInfoTimes)
                    {
                        VOS_TaskDelay(VOS_TICK_SECOND);

                        ponInserted = ponPhyPortNum;
                        i = -1;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            PonPortIdx = GetPonPortIdxBySlot ( CardIndex, i );									
            if( PonPortIdx == RERROR ) continue;

            PonChipVer = GetPonChipTypeByPonPort(PonPortIdx);
            if( PonPortIdx == RERROR ) continue;

            /* B--added by liwei056@2010-5-13 for OLT-API */
            if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && ((CardIndex == SYS_LOCAL_MODULE_SLOTNO) || (!SYS_MODULE_SLOT_ISHAVECPU(CardIndex))) )
            {
                /* PON��ֱ�ӹ����ߣ�Ӧ��PON���سɹ��󣬰�װOLT���ع���ӿ� */
                PonIsLocalInsert = 1;

                /* PON�������������ӻ����PON */
                iOltCurrMode = PON_OLT_MODE_NOT_CONNECTED;
                if ( 0 > iFirstResumeFailed )
                {
					VOS_TaskDelay( VOS_TICK_SECOND/2);
                    /* �Ȼָ����ܵ����� */
                    if ( 0 > (iOltCurrMode = pon_resume_olt(PonPortIdx)) )
                    {
                        /* �ָ����˵����ӣ����¸�λ���� */
                        iOltCurrMode = PON_OLT_MODE_NOT_CONNECTED;
                    }
                }
            }
            else
            {
                /* ��PON��ֱ�ӹ����ߣ���װOLTԶ�̹���ӿ� */
                PonIsLocalInsert = 0;
                if ( 0 == pon_remote_olt(PonPortIdx) )
                {
                    OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s RPC-PON service at the PonPortInsert time.\r\n", SYS_LOCAL_MODULE_SLOTNO, CardIndex, i);
                }

                /* ��PON�������߲������PON */
                iOltCurrMode = PON_OLT_MODE_CONFIGURED_AND_ACTIVATED;
            }
            /* E--added by liwei056@2010-5-13 for OLT-API */

            /*PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ONLINE;
            if( PonPortTable[PonPortIdx].PortAdminStatus != PONPORT_ENABLE ) continue;*/

            switch (iOltCurrMode)
            {
                case PON_OLT_MODE_NOT_CONNECTED:
                    if ( OLT_PONCHIP_ISPAS(PonChipVer) )
                    {
                        /* ��λ������PONоƬ�����5201�ļ��سɹ��� */
                        if ( 0 >= iFirstResumeFailed )
                        {
                            Hardware_Reset_olt1(CardIndex, i, 1, 0);
                            for (iFirstResumeFailed = i + 1; iFirstResumeFailed<=ponPhyPortNum; iFirstResumeFailed++)
                            {
                                /* ��λPONоƬ,��������Լ���ͨ���ĸ��� */
                                Hardware_Reset_olt1(CardIndex, iFirstResumeFailed, 0, 0);
                            }
                        }
                        
                        sys_console_printf("    %s/port%d is reseted.\r\n", CardSlot_s[CardIndex], i);

                        /* ��λPONоƬ���������� */
                        Hardware_Reset_olt2(CardIndex, i, 1, 0);
                    }
					#if defined(_GPON_BCM_SUPPORT_)
					else if ( OLT_PONCHIP_ISGPON(PonChipVer) )
					{
						/*�ݲ�����*/
					}
					#endif
                    else
                    {
                        /* TKоƬ������5~8�� */
                        sys_console_printf("    %s/port%d is ready.\r\n", CardSlot_s[CardIndex], i);

                        /* ��λ��Ӧ���������ж� */
                        PonIntOpen(PonPortIdx);
                    }

                    /* ��ʼ ����PON оƬ*/
                    if( ROK == (ret = pon_add_oltEx2 (CardIndex, i, PonPortIdx, PonChipVer, 0, 0, pon_load_mode)) )
                    {
                        if ( pon_load_mode > 0 )
                        {
                            pon_load_needwait = 1;
                            pon_load_overflag |= (1 << PonPortIdx);
                        }
                    }
                    else if ( pon_load_mode > 0 )
                    {
 			/* modified by xieshl 20120406, �ظ�����ʱ���ܸ�PON״̬��������ܻᵼ���豸��������쳣����
 			    ����λPON�壬ʹPON����״̬���豸����״̬ʧ�����ر��ǵ�һ��PONоƬ����ʧ��ʱ��
 			    ���׽�����ѭ��״̬*/
                       /*if(EVENT_PONADD == V2R1_ENABLE )*/
                            sys_console_printf("    %s/port%d added err, retry again\r\n", CardSlot_s[CardIndex], i);

                        /*PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ERR;
                        PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;*/	
                        /*PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_UNKNOWN;*/

                        /* modified by chenfj 2008-4-21
                        �޸�: ��������ʧ�ܵ�PON оƬ, ����������Ϊĳ��PON оƬ���쳣, ��Ӱ������PON ��

                        4:  ��PAS 5021 PON���ϵ�һƬPON, ������ʧ��,��������һ��,
                        ��Ϊ��һ��PONоƬ�����PON����5325E�ĳ�ʼ��
                        */
                        if( PonChipVer != PONCHIP_PAS5001 )
                        {
                            ret = pon_add_oltEx2(CardIndex, i, PonPortIdx, PonChipVer, 0, 0, 0);	
                        }
                        else
                        {
                            ret = pon_add_oltEx2(CardIndex, i, PonPortIdx, PonChipVer, -1, 0, 0);	
                        }
                        if ( ROK != ret )
                        {
                            /*
                            PonBoardDownloadCounter[CardIndex] ++;
                            if( PonBoardDownloadCounter[CardIndex] <= PON_DOWNLOAD_MAX_COUNTER )
                            */
                            PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ERR;
   	                     PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;
                            /*if(reboot_unknown_type_times[CardIndex] > 1 )*/
                            if(reboot_unknown_type_times[CardIndex] == 0 )	/* modified by xieshl 20120419, ֻ���ڰ帴λ3���Բ��ɹ�����������ټ��� */
                                return;
                        }
                    }

#ifdef __T_PON_CONN_TIME__        
                    gettimeofday(&ponresume_endtime, NULL);
                    uptime = calculate_time_diff_ms(&ponresume_endtime, &ponresume_starttime);
                    sys_console_printf("    %s/port%d's added take %lu(ms).\r\n", CardSlot_s[CardIndex], i, uptime);
#endif
                    break;                   
                case PON_OLT_MODE_CONNECTED_AND_NOT_CONFIGURED:
                case PON_OLT_MODE_CONFIGURED_AND_ACTIVATED:
                    if ( PonIsLocalInsert > 0 )
                    {
                        if ( 0 == pon_local_olt(PonPortIdx, PonChipVer) )
                        {
                            OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s LOCAL-PON service at the PonPortResume time.\r\n", SYS_LOCAL_MODULE_SLOTNO, CardIndex, i);
                        }
                    }
                    
                    sys_console_printf("    %s/port%d is resumed.\r\n", CardSlot_s[CardIndex], i);
                                       
                    /* B--modified by liwei056@2010-5-21 for D10165 */
#if 1
                    PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_LOADCOMP;
#else
                    PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_UP;
                    PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_UP;
#endif
                    /* E--modified by liwei056@2010-5-21 for D10165 */

#ifdef __T_PON_CONN_TIME__        
                    gettimeofday(&ponresume_endtime, NULL);
                    uptime = calculate_time_diff_ms(&ponresume_endtime, &ponresume_starttime);
                    sys_console_printf("    %s/port%d's resume take %lu(ms).\r\n", CardSlot_s[CardIndex], i, uptime);
#endif
                    /* Trap_PonResume(PonPortIdx); */
                    break;

                default:
                    VOS_ASSERT(0);
                    continue;
            }
            /* E--modified by liwei056@2009-12-08 for New-SwitchOver Project */

#if ( EPON_MODULE_EPON_5325E_MDIO == EPON_MODULE_YES )
            if(GetOltType() == V2R1_OLT_GFA6700)
            {
                if( PonChipVer == PONCHIP_PAS5201 )
                {
                    if(( i == 0 ) && ( ret == ROK ))
                    miim_vlanInit( PonPortIdx );
                }
            }
#endif

            VOS_TaskDelay( VOS_TICK_SECOND/*/10*/ );
        }

        if ( pon_load_needwait > 0 )
        {
            while((pon_load_timelimit-- > 0) && ( pon_load_overflag != pon_load_flags ))
            {
                VOS_TaskDelay(VOS_TICK_SECOND);
            }
        }

#ifdef __T_PON_CONN_TIME__        
        gettimeofday(&ponresume_starttime, NULL);
        uptime = calculate_time_diff_ms(&ponresume_starttime, &poncard_starttime);
        sys_console_printf("    Pon Card %d's insert take %lu(ms).\r\n", CardIndex, uptime);
#endif

        /* modified by chenfj 2008-3-11
        �ڼ���PONоƬͳ��PON����PONоƬ��Ŀʱ, δ�����ֳ�û��PON оƬ
        ����ƬPONоƬȫ�е����
        */
        if( ponInserted == 0 )
        	sys_console_printf("\r\nslot%d: no pon chip inserted\r\n", CardIndex);
#if 0
        pon_interface_update_for_card_insert( CardIndex );/* modified by xieshl 20080317, ���Ȱβ���PON�ӿ����Ʋ����޸� */
#endif
        PonCardStatus[CardIndex] = PONCARD_LOADCOMP;

}

extern STATUS tdmBoardAlarmStatus_update( ULONG slotno );	/* added by xieshl 20080202 */

/* wangysh add �ڰ�β�ͳ�ʼ��ʱ��Ҫ���������³�ʼ������λ�İ忨����*/
LONG epon_module_type_2name( ULONG slot, ULONG mtype )
{
    VOS_ASSERT(slot <= PRODUCT_MAX_TOTAL_SLOTNUM);

    if ( NULL != CardSlot_s[slot] )
    {
        VOS_Snprintf(CardSlot_s[slot], 18, "%s(slot%d)", typesdb_module_type2shortname(mtype), slot);
    }
    
    return 0;
}

/* PON��γ�����,ֱ�����豸�������*/
void PonCardPull(unsigned long module_type, unsigned long CardIndex )
{
	short int i,j, PonPortIdx;
	int iIfType;
	int ponPhyPortNum;
	int ret;
    
	/*1  pon card slot validity check 	*/
    /* removed by xieshl 20070907,���ﲻ���ж������ˣ��豸���������ɾ�������� */
#if 0
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_EPON )
	{
		sys_console_printf(" slot %d is not pon card(pon card pull) \r\n", (CardIndex +1));
		return;
	}	
#endif

	/* modified by xieshl 20120416, PON��״̬���豸����״̬��һ��ʱ���ᵼ��PON���޷�������
	    �ر���GFA6700 slot 4�Ȱβ�ʱ����Լ��50%�ĸ��ʲ��������������Ҫ���°β�忨��
	    ������Ҫ�����ظ��İγ����� */
	if( PonCardSlotRangeCheckByVty(CardIndex, NULL) != ROK )
		return;

	if( SYS_PRODUCT_TYPE == PRODUCT_E_EPON3 )
	{
		if( (PONCARD_NOTLOADING == PonCardStatus[CardIndex]) && (FALSE == PonChipActivatedFlag[CardIndex]) &&
			((PonPortIdx = GetPonPortIdxBySlot(CardIndex, 1)) != RERROR) )
		{
			ret = 1;
			for( i=0; i<ponPhyPortNum; i++ )
			{
				if( PonChipMgmtTable[PonPortIdx+i].operStatus != PONCHIP_NOTLOADING )
				{
					ret = 0;
					break;
				}
			}
			if( ret == 1 )
			{
				sys_console_printf("\r\n %s is pulled already\r\n", CardSlot_s[CardIndex] );
				return;
			}
		}
	}

	sys_console_printf("\r\n  %s is pulled  \r\n", CardSlot_s[CardIndex] );

	/*2  physical reset the Pon card */ 
	/* 3    */
	PonCardStatus[CardIndex] = PONCARD_NOTLOADING;
	PonChipActivatedFlag[CardIndex] = FALSE;
	
	ponPhyPortNum = GetSlotCardPonPortRange(module_type, CardIndex);
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
	tdmBoardAlarmStatus_update( CardIndex );			/* added by xieshl 20080202 */
#endif	
	for(i=1; i<=ponPhyPortNum; i++ )
	{
		PonPortIdx = GetPonPortIdxBySlot ( CardIndex, i);
		if( PonPortIdx == RERROR ) continue;
        
		if ( OLT_ADAPTER_RPC <= (iIfType = OLT_GetIFType(PonPortIdx)) )
		{
			pon_lose_olt(PonPortIdx);
			OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) release the PON-Chip(%d)'s %s-PON service at the PonCardPull time.\r\n", SYS_LOCAL_MODULE_SLOTNO, PonPortIdx, (OLT_ADAPTER_RPC == iIfType) ? "RPC" : "LOCAL" );
		}
        
		if( getPonChipInserted( (unsigned char)CardIndex, (unsigned char)i ) != PONCHIP_EXIST ) continue;
			                  
		OLT_ADD_DEBUG("   pon%d/%d is to be removed ...  ", CardIndex, i );

		if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && ((CardIndex == SYS_LOCAL_MODULE_SLOTNO) || (!SYS_MODULE_SLOT_ISHAVECPU(CardIndex))) )
		{
		    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			Pon_RemoveOlt(PonPortIdx, FALSE, FALSE);
		}
		/* B--added by liwei056@2011-3-31 for D11887 */
		else
		{
			int swap_slot, swap_port;
			short int PonPortIdx_Swap;

			if ( ROK == PonPortSwapSlotQuery(PonPortIdx, &swap_slot, &swap_port) )
			{
				if ( CardIndex != swap_slot )
				{
					/* ��֪ͨ�������ϵı����ڣ���OLT�Ѿ��Ƴ� */
					if ( RERROR != (PonPortIdx_Swap = GetPonPortIdxBySlot(swap_slot, swap_port)) )
					{
					    /*for onu swap by jinhl@2013-04-27*/
						/*��Ϊonu����ģʽ�����Ӧ���д���*/
						if(V2R1_PON_PORT_SWAP_ONU == GetPonPortHotSwapMode(PonPortIdx))
						{
						   
				            
						   (void)PonOltLoose_OnuSwapHandler(PonPortIdx, PonPortIdx_Swap);
						   
						}
						else
						{
							OLT_RdnRemoveOlt(PonPortIdx_Swap, PonPortIdx);
						}
					}
				}
			}
		}
		/* E--added by liwei056@2011-3-31 for D11887 */

		/* B--added by liwei056@2011-11-22 for D13853 */
		OLTAdv_NotifyOltInvalid(PonPortIdx);
		/* E--added by liwei056@2011-11-22 for D13853 */

		OLT_ADD_DEBUG("Ok\r\n");
#if 0        
		for( j=0; j<MAXONUPERPON; j++)
		{
			/* send onu deregister msg to NMS */
			if( GetOnuOperStatus(PonPortIdx, j) == ONU_OPER_STATUS_UP )	/* modified by xieshl 20101223, ���ⵥ11654 */
			{
				Trap_OnuDeregister( PonPortIdx, j, PON_ONU_DEREGISTRATION_HOST_REQUEST, 1 );	/* modified by xieshl 20110624, ���ⵥ13089��ǿ�Ʒ���trap */
				OltOamPtyOnuLoseNoti(PonPortIdx, (j+1));	/* zhangxinhui 20110504, ���ⵥ11907 */
			}
			
			/* send onu alarm clear msg to NMS */

			/* clear current running data */
			/* modified by xieshl 20110624, ���ⵥ13089 */
			SetOnuOperStatus(PonPortIdx, j, ONU_OPER_STATUS_DOWN );
			ClearOnuRunningData( PonPortIdx, j, 0);
		}
#else
        OnuEvent_ClearRunningDataMsg_Send(PonPortIdx);
#endif
		/*  clear PON port table running data */
		ClearPonPortRunningData( PonPortIdx );

		/* clear PON chip table running data */
		PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_NOTPRESENT;
		PonChipMgmtTable[PonPortIdx].Type = PONCHIP_UNKNOWN;
		PonChipMgmtTable[PonPortIdx].version = PONCHIP_UNKNOWN;
		PonChipMgmtTable[PonPortIdx].Err_counter = 0;
		/*PonChipDownloadCounter[PonPortIdx]=0;*/
		/*VOS_TaskDelay( VOS_TICK_SECOND/4 );*/
	}
	SetOltCardslotPulled( CardIndex );

/* modified by xieshl 20120810, �γ�PON��ʱҪ��ONU MACͳ�Ƹ澯�����ⵥ15520 */
#if(EPON_MODULE_ONU_MAC_OVERFLOW_CHECK==EPON_MODULE_YES)
	clearOnuMacCheckAlarmStateBySlot(CardIndex, module_type);
#endif
}

void PonCardActivated(unsigned long CardIndex )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_CARD_ACTIVATED, 0, 0};

	/*
	if(( CardIndex > PON1 ) ||( CardIndex < PON5 ))
		{
		sys_console_printf(" slot %d is not pon card(pon activate) \r\n", (CardIndex +1));
		return;
		}
	*/
	if(SlotCardIsPonBoard(CardIndex) != ROK )
	{
		sys_console_printf(" slot %d is not pon card(pon activated)\r\n", CardIndex);
		return;
	}
	/*Begin:32291,36826 һ��GPON�忨�쳣������һ�������忨������ȴ�10���Ӳ���ע��onu by jinhl@2017.04.29*/
	/*������ԭ��Ϊ���ذ�˳�򼤻�pon�壬��ȥ�������쳣�忨������ȴ���Ӧ��ʱ��10����;Ӧ���жϰ忨״̬������ready״̬��ȥ����*/
	if(!SYS_MODULE_IS_READY(CardIndex))
	{
		sys_console_printf(" slot %d is not ready, shoud not active it\r\n", CardIndex);
		return;
	}
	/*End:32291,36826 һ��GPON�忨�쳣������һ�������忨������ȴ�10���Ӳ���ע��onu by jinhl@2017.04.29*/
	aulMsg[3] = CardIndex;
	aulMsg[2] = devsm_sys_is_switchhovering();

	/*
	if( g_PonUpdate_Queue_Id == 0 )  g_PonUpdate_Queue_Id = VOS_QueCreate( MAXPONUPDATEMSGNUM, VOS_MSG_Q_FIFO);
	*/
	
	if( PonChipActivatedFlag[CardIndex] == TRUE ) return;
	PonChipActivatedFlag[CardIndex] = TRUE;

	if( g_PonUpdate_Queue_Id  == 0 )
	{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS can not create queue g_PonUpdate_Queue_Id ( PonCardInserted() )\r\n" );*/
		return;
	}

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		/*VOS_ASSERT(0);*/
		/*sys_console_printf("error: VOS send message err( PonCardInserted())\r\n"  );*/
	}
}

void PonPortActivated(short int PonPortIdx )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_PONPORT_ACTIVATED, 0, 0};

	aulMsg[3] = PonPortIdx;
	 
	if( g_PonUpdate_Queue_Id  == 0 )
	{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS can not create queue g_PonUpdate_Queue_Id ( PonCardInserted() )\r\n" );*/
		return;
	}

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		/*VOS_ASSERT(0);*/
		/*sys_console_printf("error: VOS send message err( PonCardInserted())\r\n"  );*/
	}
}

void PonDevReset(short int PonPortIdx )
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_PONDEV_RESET, 0, 0};

	aulMsg[3] = PonPortIdx;
	 
	if( g_PonUpdate_Queue_Id  == 0 )
	{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS can not create queue g_PonUpdate_Queue_Id ( PonCardInserted() )\r\n" );*/
		return;
	}

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		/*VOS_ASSERT(0);*/
		/*sys_console_printf("error: VOS send message err( PonCardInserted())\r\n"  );*/
	}
}

void PonDevDisConnect()
{
	unsigned long aulMsg[4] = { MODULE_OLT, FC_PONDEV_DISCONNECT, 0, 0};

	aulMsg[3] = 0;
	 
	if( g_PonUpdate_Queue_Id  == 0 )
	{
		VOS_ASSERT(0);
		/*sys_console_printf("error: VOS can not create queue g_PonUpdate_Queue_Id ( PonCardInserted() )\r\n" );*/
		return;
	}

	if( VOS_QueSend( g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL  ) != VOS_OK )
	{
		/*VOS_ASSERT(0);*/
		/*sys_console_printf("error: VOS send message err( PonCardInserted())\r\n"  );*/
	}
}
extern void ClearSinglePonSlotCounter(unsigned CardIndex );
static int  PonChipStatus( short int PonPortIdx )
{
	unsigned long slot;
	unsigned long port;
	
	CHECK_PON_RANGE

	slot = (unsigned long)GetCardIdxByPonChip( PonPortIdx );
	port = (unsigned long)GetPonPortByPonChip( PonPortIdx );

	if( getPonChipInserted( (unsigned char)slot, (unsigned char)port) != PONCHIP_EXIST ) return( PONCHIP_NOTPRESENT );
	return( PonChipMgmtTable[PonPortIdx].operStatus );
}

extern int devsm_reboot_times_default_set( ULONG slotno );
void  PonCardActivate(short int CardIndex, short int switchover_flag ) 
{
    short int i, PonPortIdx;
    int ponPhyPortNum;
    int PonIsLocalActive;

	/* 1  pon card slot validity check */
	if( SlotCardIsPonBoard(CardIndex) != ROK )
	{
		sys_console_printf(" slot %d is not pon card(pon active)\r\n", CardIndex);
		return;
	}
	/*Begin:32291,36826 һ��GPON�忨�쳣������һ�������忨������ȴ�10���Ӳ���ע��onu by jinhl@2017.04.29*/
	/*������ԭ��Ϊ���ذ�˳�򼤻�pon�壬��ȥ�������쳣�忨������ȴ���Ӧ��ʱ��10����;Ӧ���жϰ忨״̬������ready״̬��ȥ����*/
	if(!SYS_MODULE_IS_READY(CardIndex))
	{
		sys_console_printf(" slot %d is not ready, should not active it\r\n", CardIndex);
		return;
	}
	/*End:32291,36826 һ��GPON�忨�쳣������һ�������忨������ȴ�10���Ӳ���ע��onu by jinhl@2017.04.29*/
	PonChipActivatedFlag[CardIndex] = TRUE;
    
	/* 2  Active the Pon port */ 
	sys_console_printf("\r\n  pon port on %s is to be activating ...\r\n", CardSlot_s[CardIndex]);
	/*pon_interface_update_for_card_insert( CardIndex+1 );*/	/* added by xieshl 20080219 */

	/*PonChipActivatedFlag[CardIndex] = TRUE;*/

    ponPhyPortNum = GetSlotCardPonPortRange(0, CardIndex);

    /*added by luh 2012-9-18*/
	restoreOnuBwBasedMacHotCallBack( CardIndex );	/* added by xieshl 20120705, �ָ�����MAC��ַ�Ĵ������ã����ⵥ14205 */

	PonPortIdx = GetPonPortIdxBySlot( CardIndex, FIRSTPONPORTPERCARD );
	if( PonPortIdx == RERROR ) return;
	
    if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && ((CardIndex == SYS_LOCAL_MODULE_SLOTNO) || (!SYS_MODULE_SLOT_ISHAVECPU(CardIndex))) )
    {
        /* PON�������߰��򼤻��PON���� */
        PonIsLocalActive = 1;
    }
    else
    {
        /* ��PON��������ֱ�Ӽ���Զ��PON���� */
        PonIsLocalActive = 0;

        /* ��ֹ���ذ岻��PonCardInsert()��ֱ�ӵ�PonCardActivate() */
    	if( GetOltCardslotInserted( CardIndex) == CARDNOTINSERT )
    	{
    		SetOltCardslotInserted( CardIndex );
    	}
        if ( PONCARD_LOADCOMP != PonCardStatus[CardIndex] )
        {
    		PonCardStatus[CardIndex] = PONCARD_LOADCOMP;
        }        

        /* �Ȼָ�PON����ȫ������, �����ٻָ�ÿ��PON�ڵ����� */
        ResumeOltCardConfig(CardIndex);

        /*added by wangxiaoyu 2012-06-12
         * wait for the complete of sync of onu profiles for this card*/
        startOnuConfSyndReqByCard(CardIndex);
		onu_conf_card_sync_wait(CardIndex, 0);

    }

	for( i=1; i<=ponPhyPortNum; i++ )
	{
        if ( !PonIsLocalActive )
        {
            /* ��ƭ���ذ�PON������� */
            if ( PONCHIP_ERR != PonChipMgmtTable[PonPortIdx].operStatus )
            {
                PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_LOADCOMP;
            }
        }

    	/* modified by xieshl 20160617, ��PONֱ�ӹ����߲�Ӧִ�У������ִ�в�ͬ�����⣬���ܵ���ONU��ע�����ǰע�� */
        if( (switchover_flag || PonIsLocalActive == 1) && (PONCHIP_LOADCOMP == PonChipStatus(PonPortIdx)) )
        {
            if(ActivePonPort(PonPortIdx) != ROK)
            {
                VOS_SysLog(LOG_TYPE_OLT, LOG_ERR, "  Active Pon%d/%d Failed!\r\n", CardIndex, i);
            }
        }
        else
        {
            if ( !PonIsLocalActive )
            {
                /* ��PON��ֱ�ӹ����ߣ�ֻҪPONоƬ���ڣ���ʹ�����ʧ�ܣ�ҲҪ��װOLTԶ�̹���ӿ� */
                if ( 0 == pon_remote_olt(PonPortIdx) )
                {
                    OLT_ADD_DEBUG(OLT_ADD_TITLE"Slot(%d) get the PON-Chip(%d/%d)'s RPC-PON service at the PonCardActivate time.\r\n", SYS_LOCAL_MODULE_SLOTNO, CardIndex, i);
                }
            }
        }

        PonPortIdx++;
	}
       
	PonCardStatus[CardIndex] = PONCARD_ACTIVATED;
	ClearSinglePonSlotCounter(CardIndex);
	devsm_reboot_times_default_set(CardIndex);
#if 0/*removed by luh 2012-9-18 ����mac�Ĵ������Ӧ���ڼ���pon��֮ǰ����֤onu����Ļָ�*/
	restoreOnuBwBasedMacHotCallBack( CardIndex );	/* added by xieshl 20120705, �ָ�����MAC��ַ�Ĵ������ã����ⵥ14205 */
#endif
}

int GetPonBoardType(unsigned long CardIndex)
{
	short int PonPortIdx;
	short int i;
	int BoardType = PONCHIP_TYPE_MIN;
    
	PonPortIdx = GetPonPortIdxBySlot(CardIndex, FIRSTPONPORTPERCARD);
	
	for(i=0; i<PONPORTPERCARD; i++)
	{
		if(getPonChipInserted(CardIndex, i + 1) == PONCHIP_EXIST)
		{
			BoardType = GetPonChipTypeByPonPort(PonPortIdx+i);
			break;
		}
	}
    
	return( BoardType );	
}

int  PonDownloadRetryMax(unsigned long CardIndex )
{
	if( GetPonBoardType(CardIndex) != PONCHIP_PAS5001 )
	{
 		if( PonCardStatus[CardIndex] >= PONCARD_LOADCOMP )
			return( ROK );
		else
            return (RERROR );
	}
	else
	{
		/*if(( PonCardStatus[CardIndex] >= PONCARD_LOADCOMP ) ||(PonBoardDownloadCounter[CardIndex] > PON_DOWNLOAD_MAX_COUNTER))*/
		if(/*( PonCardStatus[CardIndex] >= PONCARD_LOADCOMP ) ||*/ (reboot_unknown_type_times[CardIndex] <= 1))
			return( ROK );
		else
            return( RERROR );
	}
	
	return( RERROR );
}

#if 0
static int PonCardStatusCheck1(unsigned long CardIndex )
{
	short int PonPortIdx;
	int ret = PONCARD_ERROR;
	int ponInserted = 0;
	int ponInserted1 = 0;
	int status = 0;
	int DownloadComp = RERROR;
	int PonCardType;
	int ponPhyPortNum;
    int ponChipStatus;
	unsigned char port;

    /* B--added by liwei056@2011-1-11 for AdvancedHandle */
    if (PONCARD_ACTIVATED == PonCardStatus[CardIndex])
    {
        return PONCARD_ACTIVATED;
    }
    /* E--added by liwei056@2011-1-11 for AdvancedHandle */

    ponPhyPortNum = GetSlotCardPonPortRange(CardIndex);

	PonPortIdx = GetPonPortIdxBySlot( CardIndex, FIRSTPONPORTPERCARD);
	if( PonPortIdx == RERROR ) return( ret );

	PonCardType  = GetPonBoardType(CardIndex);
	DownloadComp = PonDownloadRetryMax(CardIndex);

	ponInserted = 0;
	for(port=0; port<ponPhyPortNum; port++)
		if( getPonChipInserted( (unsigned char)CardIndex, port + 1 ) == PONCHIP_EXIST ) ponInserted += (1 << port);

	if(ponInserted == 0 )
	{
		sys_console_printf("no pon-chip inserted in slot%d\r\n", CardIndex);
		if(PonCardStatus[CardIndex] < PONCARD_NOTLOADING)
			PonCardStatus[CardIndex] = PONCARD_NOTLOADING;
		
		if(PonCardStatus[CardIndex] < PONCARD_LOADCOMP)
			return(PonCardStatus[CardIndex]);
		else if(PonCardStatus[CardIndex] == PONCARD_LOADCOMP)
		{
			PonCardStatus[CardIndex] = PONCARD_ACTIVATED;
			return(PONCARD_LOADCOMP);
		}

		else if(PonCardStatus[CardIndex] == PONCARD_ACTIVATED)
		{		
			if( PonChipActivatedFlag[CardIndex] == FALSE )
				return( PONCARD_LOADCOMP);
			else return( PONCARD_ACTIVATED );
		}
		else 
			return (PonCardStatus[CardIndex]);
	}

	ponInserted1 =0;
	for(port=0; port<ponPhyPortNum; port++)
		if(( getPonChipInserted( (unsigned char)CardIndex, port + 1 ) == PONCHIP_EXIST )
		&& (PonPortTable[PonPortIdx + port].PortAdminStatus == PONPORT_ENABLE))
			ponInserted1 += (1 << port);
	status = 0;
	for(port=0; port<ponPhyPortNum; port++)
		if(PonChipStatus(PonPortIdx+port) == PONCHIP_DEL) status += (1 << port);

	if(( ponInserted & status ) != 0 )	return ( PONCARD_NOTLOADING );

	/*����оƬ���ش����򷵻ؼ��ش���*/
	status = 0;
	for(port=0; port<ponPhyPortNum; port++)
		if(PonChipStatus(PonPortIdx+port) == PONCHIP_ERR) status += (1 << port);

	/* modified by chenfj 2008-4-21
	     �޸�: ��������ʧ�ܵ�PON оƬ, ����������Ϊĳ��PON оƬ���쳣, ��Ӱ������PON ��
	      1:  ����PON оƬ�����ش���, �򷵻ؼ��ش���
	      */
	/* modified by chenfj 2008-5-16
	   �޸�: ���������ش���,
	                 �����Դ���֮ǰ, 
	                           ��PAS5001: ���ؼ��ش���; ���������������忨
	                           ��PAS5201, �������ڼ���; Ƕ������ڲ����л���������
	                 �����Դ���֮��, ��ȫ�����Ǽ��ش���,�򷵻ؼ��ش���
	   */
	if( DownloadComp != ROK )
	{
		if(( ponInserted & status ) != 0 )
		{
			if( PonCardType == PONCHIP_PAS5001 )
				return ( PONCARD_ERROR );
			else
				return ( PONCARD_LOADING );
		}
	}
	else
    {
		if(ponInserted == status ) 
		{
			/*
			if(PonCardType == PONCHIP_PAS5201 )
				{
				if(reboot_unknown_type_times[CardIndex] > 1 )
					return( PONCARD_ERROR );
				}
			*/
    		if( PonChipActivatedFlag[CardIndex] == FALSE )
    			return ( PONCARD_ERROR);
    		else
                return( PONCARD_ERROR );
		}
	}
	

	if( PonChipActivatedFlag[CardIndex] == FALSE )
	{
		/*��оƬ���ڳ�ʼ״̬���򷵻�δ����*/
		status = 0;
		for(port=0; port<ponPhyPortNum; port++)
			if(PonChipStatus(PonPortIdx+port) == PONCHIP_NOTPRESENT) status += (1 << port);

		if(( ponInserted & status ) == ponInserted ) return ( PONCARD_NOTLOADING );

		/*������оƬ��û�м��أ��򷵻�δ����*/
		status = 0;
		for(port=0; port<ponPhyPortNum; port++)
			if(PonChipStatus(PonPortIdx+port) == PONCHIP_NOTLOADING) status += (1 << port);

		if(( ponInserted & status ) == ponInserted ) return ( PONCARD_NOTLOADING );
		
		/* оƬ�������,�򷵻ؼ������*/
		/* modified by chenfj 2008-4-21
	     �޸�: ��������ʧ�ܵ�PON оƬ, ����������Ϊĳ��PON оƬ���쳣, ��Ӱ������PON ��
	         2 : ����PONоƬҪô�������,Ҫô���ش���, ����Ϊ�������
	      */
		status = 0;
		for(port=0; port<ponPhyPortNum; port++)
			if((PonChipStatus(PonPortIdx+port) == PONCHIP_LOADCOMP) || (PonChipStatus(PonPortIdx+port) == PONCHIP_ERR)) status += (1 << port);

		if(( ponInserted & status ) == ponInserted ) return ( PONCARD_LOADCOMP );

        /* B--added by liwei056@2009-12-15 for New-SwitchHover Project */
		status = 0;
		for(port=0; port<ponPhyPortNum; port++)
			if((PonChipStatus(PonPortIdx+port) == PONCHIP_ACTIVATED) || (PonChipStatus(PonPortIdx+port) == PONCHIP_LOADCOMP)) status += (1 << port);
		if(( ponInserted & status ) == ponInserted ) return ( PONCARD_LOADCOMP );
        /* E--added by liwei056@2009-12-15 for New-SwitchHover Project */
		
	      /*������������� ���ڼ���*/
		return(PONCARD_LOADING );		  
	}
	else
    {
		/* modified by chenfj 2008-4-21
        	     �޸�: ��������ʧ�ܵ�PON оƬ, ����������Ϊĳ��PON оƬ���쳣, ��Ӱ������PON ��
	         3 :  PONоƬҪô���ش���, Ҫô����,����ΪPON �弤��
	         */
	         
	    if( ponInserted1 == 0 ) return ( PONCARD_ACTIVATED );

		status = 0;
		for(port=0; port<ponPhyPortNum; port++)
			if((PonChipStatus(PonPortIdx+port) == PONCHIP_ACTIVATED) /*|| (PonChipStatus(PonPortIdx+port) == PONCHIP_ERR)*/) status += (1 << port);

		if(( ponInserted1 & status ) != 0 /* ponInserted1 */ )
        {
            /* B--modified by liwei056@2011-1-10 for PonCardActived-BUG */
#if 1
            return ( PONCARD_LOADCOMP );
#else
            return ( PONCARD_ACTIVATED );
#endif
            /* E--modified by liwei056@2011-1-10 for PonCardActived-BUG */
        }      

		/* оƬ״̬Ҫô������ɣ�Ҫô���ش����򷵻ؼ������*/
		status = 0;
		for(port=0; port<ponPhyPortNum; port++)
        {
            ponChipStatus = PonChipStatus(PonPortIdx+port);
            if ((ponChipStatus == PONCHIP_LOADCOMP)
                || (ponChipStatus == PONCHIP_ERR))
            {
                status += (1<<port);
            }
        }      
       
		if(( ponInserted & status ) == ponInserted ) return ( PONCARD_LOADCOMP );


		/*��оƬ���ڳ�ʼ״̬���򷵻�δ����*/
		status = 0;
		for(port=0; port<ponPhyPortNum; port++)
			if(PonChipStatus(PonPortIdx+port) == PONCHIP_NOTPRESENT) status += (1<<port);

		if(( ponInserted & status ) == ponInserted )	return ( PONCARD_NOTLOADING );		
		
		/*������оƬ��û�м��أ��򷵻�δ����*/
		status = 0;
		for(port=0; port<ponPhyPortNum; port++)
			if(PonChipStatus(PonPortIdx+port) == PONCHIP_NOTLOADING ) status += (1<<port);

		if(( ponInserted & status ) == ponInserted ) return ( PONCARD_NOTLOADING );

		return( PONCARD_LOADING );
	}

	/*sys_console_printf("\r\n%s current status:%s \r\n", CardSlot_s[CardIndex], CardStatus[ret] );*/
}
#else
/* modified by xieshl 20120401, PON��״̬���豸����״̬��һ��ʱ���ᵼ��PON���޷�������
    �ر���GFA6700 slot 4�Ȱβ�ʱ����Լ��50%�ĸ��ʲ��������������Ҫ���°β�忨��
    �������Ĺ����Ǹ��ݰ�������PON�ڵ�״̬��������忨״̬��Ȼ��ӳ�䵽�豸
    ����״̬�С�
    Ŀǰ�������ǣ�PONоƬ״̬Ǩ�������豸�����PON����(tCardMgmt)��ͬ��ɵģ���
    PON������ʱ״̬Ǩ�ƽ���(��PON����ʱ)������豸����״̬���ٸı�ʱ����PON
    оƬ״̬����ʵʱˢ�£��豸������ܻ���ΪPON��״̬�쳣����λPON��:

    �豸����:		��λ---->����----->ע��---->����------->��λ
					                      |                     |                    |                       |
    					msg1	��PON״̬     ����              |                       |
    					msg2	��PON״̬-----------------|           PON״̬�쳣 
    				  
    �ڷ��ֹ���֮ǰ�����PON����������Ϣ��ѹ���ͻ����һ����ѭ��״̬
     */
static int PonCardStatusCheck1(unsigned long module_type, unsigned long CardIndex )
{
	short int port;
	short int PonPortIdx;
	int ret = PONCARD_ERROR;
	int DownloadComp = RERROR;
	int PonCardType;
	int ponPhyPortNum;

	ULONG pon_inserted_bits = 0;		/* PONоƬ��λ��־��bit=1��ʾ��λ��=0��ʾ����λ����ͬ*/
	ULONG pon_enabled_bits = 0;		/* PONоƬʹ�ܴ򿪱�־ */
	ULONG pon_deleted_bits = 0;		/* PONоƬ�ѱ��Ƴ���־ */
	ULONG pon_loadErr_bits = 0;		/* PONоƬ����ʧ�ܱ�־ */
	ULONG pon_notPresent_bits = 0;	/* PONоƬ�д��ڳ�ʼ״̬*/
	ULONG pon_notLoading_bits = 0;	/* PONоƬ�ȴ������ڼ���*/
	ULONG pon_loaded_bits = 0;		/* PONоƬ�������*/
	ULONG pon_activated_bits = 0;	/* PONоƬ�Ѽ��� */

	if (PONCARD_ACTIVATED == PonCardStatus[CardIndex])	/* added by liwei056@2011-1-11 for AdvancedHandle */
	{
		return PONCARD_ACTIVATED;
	}

	PonPortIdx = GetPonPortIdxBySlot( CardIndex, FIRSTPONPORTPERCARD);
	if( PonPortIdx == RERROR )
		return( ret );

	ponPhyPortNum = GetSlotCardPonPortRange(module_type, CardIndex);
	PonCardType  = GetPonBoardType(CardIndex);
	DownloadComp = PonDownloadRetryMax(CardIndex);

	for(port=0; port<ponPhyPortNum; port++)
	{
		if( getPonChipInserted( (unsigned char)CardIndex, port + 1 ) != PONCHIP_EXIST )
			continue;
		
		pon_inserted_bits |= (1 << port);
		if( PonPortTable[PonPortIdx + port].PortAdminStatus == PONPORT_ENABLE )
			pon_enabled_bits |= (1 << port);

		switch( PonChipStatus(PonPortIdx+port) )
		{
			case PONCHIP_DEL:
				pon_deleted_bits |= (1 << port);
				break;
			case PONCHIP_ERR:
				pon_loadErr_bits |= (1 << port);
				break;
			case  PONCHIP_NOTPRESENT:
				pon_notPresent_bits |= (1 << port);
				break;
			case PONCHIP_NOTLOADING:
				pon_notLoading_bits |= (1 << port);
				break;
			case PONCHIP_LOADCOMP:
				pon_loaded_bits |= (1 << port);
				break;
			case PONCHIP_ACTIVATED:
				pon_activated_bits |= (1 << port);
				break;
			default:
				break;
		}
	}

	if(pon_inserted_bits == 0 )
	{
		sys_console_printf("no pon-chip inserted in slot%d\r\n", CardIndex);

		if(PonCardStatus[CardIndex] < PONCARD_NOTLOADING)
			PonCardStatus[CardIndex] = PONCARD_NOTLOADING;
		else if(PonCardStatus[CardIndex] == PONCARD_LOADCOMP)
		{
			PonCardStatus[CardIndex] = PONCARD_ACTIVATED;
			return(PONCARD_LOADCOMP);
		}
		else if(PonCardStatus[CardIndex] == PONCARD_ACTIVATED)
		{		
			if( PonChipActivatedFlag[CardIndex] == FALSE )
				return( PONCARD_LOADCOMP);
			return( PONCARD_ACTIVATED );
		}
		return (PonCardStatus[CardIndex]);
	}

	if(( pon_inserted_bits & pon_deleted_bits ) != 0 )
		return ( PONCARD_NOTLOADING );
	
	/*����оƬ���ش����򷵻ؼ��ش���*/
	/* modified by chenfj 2008-4-21
	     �޸�: ��������ʧ�ܵ�PON оƬ, ����������Ϊĳ��PON оƬ���쳣, ��Ӱ������PON ��
	      1:  ����PON оƬ�����ش���, �򷵻ؼ��ش���
	      */
	/* modified by chenfj 2008-5-16
	   �޸�: ���������ش���,
	                 �����Դ���֮ǰ, 
	                           ��PAS5001: ���ؼ��ش���; ���������������忨
	                           ��PAS5201, �������ڼ���; Ƕ������ڲ����л���������
	                 �����Դ���֮��, ��ȫ�����Ǽ��ش���,�򷵻ؼ��ش���
	   */
	if( DownloadComp != ROK )
	{
		if(( pon_inserted_bits & pon_loadErr_bits ) != 0 )
		{
			if( PonCardType == PONCHIP_PAS5001 )
				return ( PONCARD_ERROR );
			else
				return ( PONCARD_LOADING );
		}
	}
	else
	{
		if(pon_inserted_bits == pon_loadErr_bits ) 
			return( PONCARD_ERROR );
	}
	
	if( PonChipActivatedFlag[CardIndex] == FALSE )
	{
		if(( pon_inserted_bits & pon_notPresent_bits ) == pon_inserted_bits )	/*��оƬ���ڳ�ʼ״̬���򷵻�δ����*/
			return ( PONCARD_NOTLOADING );

		if(( pon_inserted_bits & pon_notLoading_bits ) == pon_inserted_bits )	/*������оƬ��û�м��أ��򷵻�δ����*/
			return ( PONCARD_NOTLOADING );
		
		/* modified by chenfj 2008-4-21
		�޸�: ��������ʧ�ܵ�PON оƬ, ����������Ϊĳ��PON оƬ���쳣, ��Ӱ������PON ��
		2 : ����PONоƬҪô�������,Ҫô���ش���, ����Ϊ�������*/
		if(( pon_inserted_bits & (pon_loaded_bits | pon_loadErr_bits) ) == pon_inserted_bits )	/* оƬ�������,�򷵻ؼ������*/
			return ( PONCARD_LOADCOMP );

		if(( pon_inserted_bits & (pon_loaded_bits | pon_activated_bits) ) == pon_inserted_bits )	/* added by liwei056@2009-12-15 for New-SwitchHover Project */
			return ( PONCARD_LOADCOMP );
	}
	else
	{
		/* modified by chenfj 2008-4-21
        	     �޸�: ��������ʧ�ܵ�PON оƬ, ����������Ϊĳ��PON оƬ���쳣, ��Ӱ������PON ��
	         3 :  PONоƬҪô���ش���, Ҫô����,����ΪPON �弤��  */
		if( pon_enabled_bits == 0 )
			return ( PONCARD_ACTIVATED );

		if(( pon_enabled_bits & pon_activated_bits ) != 0 )
			return ( PONCARD_LOADCOMP );	/*( PONCARD_ACTIVATED )*//* modified by liwei056@2011-1-10 for PonCardActived-BUG */

		if( (pon_inserted_bits & (pon_loaded_bits | pon_loadErr_bits)) == pon_inserted_bits )	/* оƬ״̬Ҫô������ɣ�Ҫô���ش����򷵻ؼ������*/
			return ( PONCARD_LOADCOMP );

		if(( pon_inserted_bits & pon_notPresent_bits ) == pon_inserted_bits )	/*��оƬ���ڳ�ʼ״̬���򷵻�δ����*/
			return ( PONCARD_NOTLOADING );		
		
		if(( pon_inserted_bits & pon_notLoading_bits ) == pon_inserted_bits )	/*������оƬ��û�м��أ��򷵻�δ����*/
			return ( PONCARD_NOTLOADING );
	}
	return( PONCARD_LOADING );	      /*������������� ���ڼ���*/
}
#endif

int PonCardStatusCheck(unsigned long module_type, unsigned long CardIndex )
{
	int ret ;

	if(GetHavePonChipReset(CardIndex) == V2R1_ENABLE) 
		return(PonCardStatus[CardIndex]);
	ret = PonCardStatusCheck1(module_type, CardIndex );
	/* PonCardStatus[CardIndex] = (unsigned char)ret; */
	/*sys_console_printf(" check card %d  status %d  %d %d \r\n", CardIndex, ret , PonChipStatus(GetPonPortIdxBySlot(CardIndex, 0)),PonChipStatus(GetPonPortIdxBySlot(CardIndex, 1)));*/
	return( ret );
}

#if 0
int PonCardStatusCheck_bak(unsigned long CardIndex )
{
	short int PonPortIdx;
	int ret = PONCARD_ERROR;

	CardIndex --;
	/*
	if(( CardIndex > PON1 ) ||( CardIndex < PON5 ))
		{
		sys_console_printf(" slot %d is not pon card(pon status)\r\n", (CardIndex +1));
		return (ret );
		}
	*/
	if(__SYS_MODULE_TYPE__(CardIndex+1) != MODULE_E_GFA_EPON )
		{
		sys_console_printf(" slot %d is not pon card(pon status) \r\n", (CardIndex +1));
		return( RERROR );
		}
	
	PonPortIdx = GetPonPortIdxBySlot( CardIndex, FIRSTPONPORT);
	if( PonPortIdx == RERROR ) return( ret );

	if(((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_DEL)&&(PonPortTable[PonPortIdx].PortWorkingStatus == PONPORT_DEL))
			|| ((PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_DEL )&&(PonPortTable[PonPortIdx+1].PortWorkingStatus == PONPORT_DEL))
			||((PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_DEL )&&(PonPortTable[PonPortIdx+2].PortWorkingStatus == PONPORT_DEL))
			||((PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_DEL )&&(PonPortTable[PonPortIdx+3].PortWorkingStatus == PONPORT_DEL)))
		{
		return( PONCARD_DEL );

		}

	if( PonChipActivatedFlag[CardIndex] == FALSE )
		{

		/*��оƬ���ڳ�ʼ״̬���򷵻�δ����*/
		 if((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_NOTPRESENT) && (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_NOTPRESENT )
			&&(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_NOTPRESENT )&&(PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_NOTPRESENT ))
			{
			ret = PONCARD_NOTLOADING;
			/*return( ret );*/
			}
		 
		/*������оƬ��û�м��أ��򷵻�δ����*/
		else if((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_NOTLOADING) && (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_NOTLOADING )
			&&(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_NOTLOADING )&&(PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_NOTLOADING ))
			{
			ret = PONCARD_NOTLOADING;
			/*return( ret );*/
			}
	#if 0	 
		 /* ֻҪ��һ�����ڼ����У��򷵻�������״̬*/
		else if((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_LOADING ) || (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_LOADING )
			||(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_LOADING )||(PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_LOADING ))
			{
			ret = PONCARD_LOADING;
			/*return( ret );*/
			}
	#endif	
		/*����оƬ���ش����򷵻ؼ��ش���*/
		else if(( PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_ERR ) || (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_ERR)
			||(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_ERR) || (PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_ERR) )
			{
			ret = PONCARD_ERROR;
			}
		
		/* оƬ״̬Ҫô������ɣ�Ҫô���ش����򷵻ؼ������*/
		else if(((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_LOADCOMP)||(PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_ERR))
			&& ((PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_LOADCOMP )||(PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_ERR))
			&&((PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_LOADCOMP )||(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_ERR))
			&&((PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_LOADCOMP )||(PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_ERR)))
			{
			ret = PONCARD_LOADCOMP;
			/*return( ret );*/
			}	
		
		else{
			ret = PONCARD_LOADING ;
			}
		/*��δ��PON �弤��֮ǰ����Ӧ����PONоƬ���ڼ���״̬*/
		}
	
	else{

		if(( PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_ERR ) && (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_ERR)
			&& (PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_ERR) && (PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_ERR) )
			{
			ret = PONCARD_ERROR;
			}
		 /* оƬ״̬Ҫô������ɣ�Ҫô���ش����򷵻ؼ���*/
		else if((( PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_ERR )||( PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_ACTIVATED ))
			&& (( PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_ERR )||( PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_ACTIVATED ))
			&& (( PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_ERR )||( PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_ACTIVATED )) 
			&& (( PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_ERR )||( PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_ACTIVATED )) )

		/*ֻҪ��һ��PONоƬ�������ΪPON����*/		
			{
			ret = PONCARD_ACTIVATED;
			/*return( ret );*/
			}

		/*��оƬ���ڳ�ʼ״̬���򷵻�δ����*/
		else if((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_NOTPRESENT) && (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_NOTPRESENT )
			&&(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_NOTPRESENT )&&(PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_NOTPRESENT ))
			{
			ret = PONCARD_NOTLOADING;
			/*return( ret );*/
			}
		
		/*������оƬ��û�м��أ��򷵻�δ����*/
		else if((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_NOTLOADING) || (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_NOTLOADING )
			||(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_NOTLOADING )||(PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_NOTLOADING ))
			{
			ret = PONCARD_NOTLOADING;
			/*return( ret );*/
			}

		 /*������оƬ�����ش����򷵻ؼ��ش���*/
		else if(( PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_ERR ) || (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_ERR)
			|| (PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_ERR) || (PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_ERR) )
			{
			ret = PONCARD_ERROR;
			}

		 /* оƬ״̬Ҫô������ɣ�Ҫô���ش����򷵻ؼ������*/
		else if(((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_LOADCOMP)||(PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_ERR))
			&& ((PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_LOADCOMP )||(PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_ERR))
			&&((PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_LOADCOMP )||(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_ERR))
			&&((PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_LOADCOMP )||(PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_ERR)))
			{
			ret = PONCARD_LOADCOMP;
			/*return( ret );*/
			}
		
		else {
			ret = PONCARD_LOADING;
			}
		
		#if 0
		/* ֻҪ��һ�����ڼ����У��򷵻�������״̬*/
		else if((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_LOADING ) || (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_LOADING )
			||(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_LOADING )||(PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_LOADING ))
			{
			ret = PONCARD_LOADING;
			/*return( ret );*/
			}
		/*��û�м��ص�����£�����оƬ������ɣ��򷵻ؼ������*/
		/*ע����������£�������оƬ��û�п�ʼ���أ�*/
		else if((PonChipMgmtTable[PonPortIdx].operStatus == PONCHIP_LOADCOMP) || (PonChipMgmtTable[PonPortIdx+1].operStatus == PONCHIP_LOADCOMP )
			||(PonChipMgmtTable[PonPortIdx+2].operStatus == PONCHIP_LOADCOMP )||(PonChipMgmtTable[PonPortIdx+3].operStatus == PONCHIP_LOADCOMP ))
			{
			ret = PONCARD_LOADCOMP;
			/*return( ret );*/
			}	
		#endif

		}

	/*sys_console_printf("\r\n%s current status:%s \r\n", CardSlot_s[CardIndex], CardStatus[ret] );*/

	return( ret );
}


int  GetPonCardWorkingStatus( short int CardSlot )
{
	int Status=FALSE;
	short int PonPortIdx,i;

	CardSlot --;
	
	/*1  pon card slot validity check 
	if(( CardSlot > PON1 ) ||( CardSlot < PON5 ))
		{
		sys_console_printf(" slot %d is not pon card(pon status)\r\n", (CardSlot +1));
		return (RERROR);
		}
	*/
	if(SlotCardIsPonBoard(CardSlot+1) != ROK )
		{
		sys_console_printf(" slot %d is not pon card(pon status) \r\n", (CardSlot +1));
		return( RERROR );
		}

	if( GetOltCardslotInserted( CardSlot) == CARDNOTINSERT )
		{
		sys_console_printf(" %s is not inserted \r\n", CardSlot_s[CardSlot] );
		return( RERROR );
		}

	PonPortIdx = GetPonPortIdxBySlot(CardSlot, FIRSTPONPORT);
	if( PonPortIdx == RERROR  )  return( RERROR );
	
	for( i=0; i<PONPORTPERCARD; i++ )
		{
		if( getPonChipInserted( (unsigned char)CardSlot,(unsigned char) (1+i) ) != PONCHIP_EXIST ) continue;
		
		if( PonPortIsWorking(PonPortIdx+i ) == TRUE)
			Status = TRUE;		
		}
	return( Status );
}
#endif


/*  added by chenfj 2008-12-10
	���Է���,����5201 PON��������PONоƬ����ִ��reset pon����,�ᵼ��
	�豸����(devsm)��PON���״̬��ת�Ƶ�һ����������״̬,�ڵȴ�
	һ����ʱ��(60��)��,��״̬��δ����,���ִ������PON��ĸ�λ;
	�޸�reset pon API,����ɸ�λ�Ĺ��������ø�λ��־;�������豸����
	����PON��״̬ʱ,���и�λ��־,�򱣳�PON�嵱ǰ״̬����
*/

int GetHavePonChipReset(unsigned long CardIndex )
{
	short int PonPort;
	int Flag = V2R1_DISABLE;

	for(PonPort = 1; PonPort <= PONPORTPERCARD; PonPort++)
	{
		if( GetPonChipResetFlag(CardIndex, PonPort) == V2R1_ENABLE)
		{
			Flag = V2R1_ENABLE;
			break;
		}
	}

    return(Flag);
}
