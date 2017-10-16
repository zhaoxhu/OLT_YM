/***************************************************************
*
*						Module Name:  CmcGeneral.c
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
*   Date: 			2013/03/13
*   Author:		liwei056
*   content:
**  History:
**   Date            |    Name            |     Description
**---- ------- |-------------|------------------ 
**  2013/03/13  |   liwei056         |     create 
**------------|-------------|------------------
***************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include  "CmcGeneral.h"
#include  "../onu/Onu_manage.h"
#include  "../onu/OnuConfMgt.h"

#if( EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES )

typedef enum _cmc_msg_type
{
    CMC_MSG_TIMER,
    CMC_MSG_EVENT,
    CMC_MSG_CDP,
    CMC_MSG_MAX
}cmc_msg_type;

typedef struct _cmc_msg_head
{
    USHORT msg_code;
    USHORT msg_len;
    SHORT  olt_idx;
    SHORT  onu_idx;
    UCHAR  cmc_mac[6];
    UCHAR  cm_mac[6];
    UCHAR  msg_bodys[0];
}cmc_msg_head;


#define CMC_MAIN_QUEUE_LENGTH      1024

#define CMC_MAX_INDEX_HANDLERS  CMC_EVENT_CODE_MAX
#define CMC_MAX_CLIENT	        1

typedef void (*cmc_general_handler_function_t)(short int PonPortIdx, short int OnuIdx, unsigned char CmcMac[6], unsigned char CmMac[6]);

typedef struct CMC_handlers_t 
{
    cmc_general_handler_function_t functions[CMC_MAX_CLIENT][CMC_MAX_INDEX_HANDLERS];
} CMC_handlers_t;

CMC_handlers_t                 cmc_handlers;

unsigned long cmc_main_queue;
unsigned long cmc_main_task;


unsigned char *onu_regflags; 
unsigned char *cmc_rdyflags; 

#define CMC_IS_REGED(byte_pos, bit_pos)   (onu_regflags[byte_pos] & (bit_pos))
#define CMC_IS_READY(byte_pos, bit_pos)   (cmc_rdyflags[byte_pos] & (bit_pos))
#define CMC_SET_REGED(byte_pos, bit_pos)  (onu_regflags[byte_pos] |= (bit_pos))
#define CMC_SET_READY(byte_pos, bit_pos)  (cmc_rdyflags[byte_pos] |= (bit_pos))
#define CMC_CLR_REGED(byte_pos, bit_pos)  (onu_regflags[byte_pos] &= ~(bit_pos))
#define CMC_CLR_READY(byte_pos, bit_pos)  (cmc_rdyflags[byte_pos] &= ~(bit_pos))

#define ONUENTRY_IDX(olt_idx, onu_idx)  (olt_idx * MAXONUPERPON + onu_idx)
#define CMC_BYTEPOS(olt_idx, onu_idx)   (ONUENTRY_IDX(olt_idx, onu_idx) >> 3)
#define CMC_BITPOS(olt_idx, onu_idx)    (1 << (ONUENTRY_IDX(olt_idx, onu_idx) & 7))
 

unsigned short cmc_service_vlanid;


#if( EPON_MODULE_DOCSIS_MANAGE_MIB == EPON_MODULE_YES )

static ULONG s_sem_cmc_mib;

static mac_address_t s_selected_cmc;
static short int s_selected_olt;
static short int s_selected_onu;

static mac_address_t gCmMac;
static int gCos;
static int gUsSfid,gDsSfid;
static unsigned char *gpTlvData = NULL;
static int gTlvDataLen;
static int gStatus;

#define CMC_MIB_LOCK()        VOS_SemTake(s_sem_cmc_mib, WAIT_FOREVER)
#define CMC_MIB_UNLOCK()      VOS_SemGive(s_sem_cmc_mib)


void cmcMib_Mgmt_InitParamters(void)
{
    VOS_MemZero(s_selected_cmc, sizeof(s_selected_cmc));
    s_selected_olt = -1;
    s_selected_onu = -1;

    return;
}

void cmcMib_Sf_InitParamters(void)
{
    VOS_MemZero(gCmMac, sizeof(gCmMac));
    gCos = -1;
    gUsSfid = -1;
    gDsSfid = -1;
    gTlvDataLen = 0;
    gStatus = 0;
    if ( gpTlvData != NULL )
    {
        VOS_Free(gpTlvData);
        gpTlvData = NULL;
    }

    return;
}

int cmcMib_Sf_HandlerOperStatusCode(int status)
{
	STATUS ret = VOS_ERROR;

    if ( !OLT_LOCAL_ISVALID(s_selected_olt) || !ONU_IDX_ISVALID(s_selected_onu) )
    {
        /* 必须先指定具体的CMC */
        return ret;
    }
	
	switch(status)
	{
		case cmc_sf_op_idle:
		{
            CMC_MIB_LOCK();    
			cmcMib_Sf_InitParamters();
            ret = VOS_OK;
            CMC_MIB_UNLOCK();    
			break;
		}
		case cmc_sf_op_create: /* Create service flow.... */
		{
			if((-1 != gCos) && (NULL != gpTlvData) && (0 != gTlvDataLen))
				ret = OnuMgt_CreateCmServiceFlow(s_selected_olt, s_selected_onu, s_selected_cmc, gCmMac, (unsigned char)gCos, gpTlvData, (unsigned short)gTlvDataLen);
			break;
		}
		case cmc_sf_op_change: /* Change service flow.... */
		{
			if( (NULL != gpTlvData) && (0 != gTlvDataLen) &&
				((gUsSfid != 0) || (gDsSfid != 0)))
				ret = OnuMgt_ModifyCmServiceFlow(s_selected_olt, s_selected_onu, s_selected_cmc, gCmMac, (unsigned long)gUsSfid, (unsigned long)gDsSfid, gpTlvData, (unsigned short)gTlvDataLen);
			
			break;
		}
		case cmc_sf_op_delete: /* Delete service flow.... */
		{
			if((gUsSfid != 0) || (gDsSfid != 0))
				ret = OnuMgt_DestroyCmServiceFlow(s_selected_olt, s_selected_onu, s_selected_cmc, gCmMac, (unsigned long)gUsSfid, (unsigned long)gDsSfid);
			break;
		}
			
		default:
	}
	
	return ret;
}


STATUS getBrcmCmcControllerMgmtVar(ULONG cmc_id,ULONG typeId,MIB_VALUE *retVar)
{
    STATUS iRlt = VOS_OK;

    CMC_MIB_LOCK();

    switch (typeId)
    {
        case cmc_mgmt_register:
        case cmc_mgmt_unregister:
            /* 暂时不支持 */
            VOS_MemZero(retVar->aucVal, BYTES_IN_MAC_ADDRESS);

            break;
        case cmc_mgmt_select:
            VOS_MemCpy(retVar->aucVal, s_selected_cmc, BYTES_IN_MAC_ADDRESS);

            break;
        default:
    		iRlt = VOS_ERROR;
    }

    CMC_MIB_UNLOCK();
    
	return iRlt;
}

STATUS setBrcmCmcControllerMgmtVar(ULONG cmc_id,ULONG typeId,MIB_VALUE *setVar)
{
    STATUS iRlt = VOS_ERROR;
    short int olt_idx, onu_idx;
    short int OnuEntryIdx;

    switch (typeId)
    {
        case cmc_mgmt_register:
            if ( 0 <= (OnuEntryIdx = GetOnuEntryByMac(setVar->aucVal)) )
            {
                olt_idx = OnuEntryIdx / MAXONUPERPON;
                onu_idx = OnuEntryIdx % MAXONUPERPON;

                iRlt = OnuMgt_RegisterCmc(olt_idx, onu_idx, setVar->aucVal);
            }

            break;
        case cmc_mgmt_unregister:
            if ( 0 <= (OnuEntryIdx = GetOnuEntryByMac(setVar->aucVal)) )
            {
                olt_idx = OnuEntryIdx / MAXONUPERPON;
                onu_idx = OnuEntryIdx % MAXONUPERPON;

                iRlt = OnuMgt_UnregisterCmc(olt_idx, onu_idx, setVar->aucVal);
            }

            break;
        case cmc_mgmt_select:
            if ( 0 <= (OnuEntryIdx = GetOnuEntryByMac(setVar->aucVal)) )
            {
                olt_idx = OnuEntryIdx / MAXONUPERPON;
                onu_idx = OnuEntryIdx % MAXONUPERPON;

                CMC_MIB_LOCK();
                s_selected_olt = olt_idx;
                s_selected_onu = onu_idx;
                VOS_MemCpy(s_selected_cmc, setVar->aucVal, BYTES_IN_MAC_ADDRESS);
                iRlt = VOS_OK;
                CMC_MIB_UNLOCK();
            }

            break;
        default:
    }

	return iRlt;
}


STATUS getBrcmCmcControllerServiceFlowVar(ULONG sf_id,ULONG typeId,MIB_VALUE *retVar)
{
    STATUS iRlt = VOS_OK;

    CMC_MIB_LOCK();

    switch (typeId)
    {
        case cmc_sf_mac:
            VOS_MemCpy(retVar->aucVal, gCmMac, BYTES_IN_MAC_ADDRESS);

            break;
        case cmc_sf_cos:
            retVar->ulVal = gCos;

            break;
        case cmc_sf_usid:
            retVar->ulVal = gUsSfid;

            break;
        case cmc_sf_dsid:
            retVar->ulVal = gDsSfid;

            break;
        case cmc_sf_tlv:
            retVar->ulValPair[0] = (ULONG)gpTlvData;
            retVar->ulValPair[1] = gTlvDataLen;

            break;
        case cmc_sf_status:
            retVar->ulVal = gStatus;

            break;
        default:
    		iRlt = VOS_ERROR;
    }

    CMC_MIB_UNLOCK();
    
	return iRlt;
}

STATUS setBrcmCmcControllerServiceFlowVar(ULONG cmc_id,ULONG typeId,MIB_VALUE *setVar)
{
    STATUS iRlt = VOS_OK;

    CMC_MIB_LOCK();

    switch (typeId)
    {
        case cmc_sf_mac:
            VOS_MemCpy(gCmMac, setVar->aucVal, BYTES_IN_MAC_ADDRESS);
            CMC_MIB_UNLOCK();

            break;
        case cmc_sf_cos:
            CMC_MIB_LOCK();
            gCos = setVar->ulVal;
            CMC_MIB_UNLOCK();

            break;
        case cmc_sf_usid:
            CMC_MIB_LOCK();
            gUsSfid = setVar->ulVal;
            CMC_MIB_UNLOCK();

            break;
        case cmc_sf_dsid:
            CMC_MIB_LOCK();
            gDsSfid = setVar->ulVal;
            CMC_MIB_UNLOCK();

            break;
        case cmc_sf_tlv:
            CMC_MIB_LOCK();
            if ( (0 == setVar->ulValPair[0]) || (0 == setVar->ulValPair[1]) )
            {
                if ( NULL != gpTlvData )
                {
                    VOS_Free(gpTlvData);
                    gpTlvData = NULL;
                }
                gTlvDataLen = 0;
            }
            else
            {
                if ( gpTlvData != NULL )
                {
                    VOS_Free(gpTlvData);
                    gTlvDataLen = 0;
                }
                if ( NULL != (gpTlvData = VOS_Malloc(setVar->ulValPair[1], MODULE_RPU_CMC)) )
                {
                    gTlvDataLen = setVar->ulValPair[1];
                    VOS_MemCpy(gpTlvData, (void*)setVar->ulValPair[0], gTlvDataLen);
                }
                else
                {
            		iRlt = VOS_ERROR;
                }
            }

            break;
        case cmc_sf_status:
            gStatus = setVar->ulVal;

            break;
        default:
    		iRlt = VOS_ERROR;
    }

    CMC_MIB_UNLOCK();

    if ( cmc_sf_status == typeId )
    {
        iRlt = cmcMib_Sf_HandlerOperStatusCode(gStatus);
    }
    
	return iRlt;
}

#endif


int ShowCmcMacLearningByVty( short int PonPortIdx, short int OnuIdx, mac_address_t cmc_mac, struct vty *vty )
{
	short int PonChipType;
	unsigned short active_records;
	int i, n;
	int slot, port;

	slot = GetCardIdxByPonChip(PonPortIdx );
	port = GetPonPortByPonChip(PonPortIdx);

	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(slot+1) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slot);
		return( RERROR );
		}*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slot) != ROK)
	{
		vty_out(vty," %% slot %d is not pon card\r\n", slot);
		return( RERROR );
	}
	
	if( getPonChipInserted((unsigned char)slot, (unsigned char)port) !=  PONCHIP_EXIST )
	{
		vty_out(vty, "\r\n %% %s/port%d not exist\r\n", CardSlot_s[slot], port );
		return( RERROR );
	} 

	if( PonPortIsWorking(PonPortIdx) == FALSE ) 
	{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[slot], port );
		return( RERROR );
	}
    
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "\r\n %% onu%d/%d is off-line\r\n", slot, port);
		return( RERROR );
	}

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISTK(PonChipType))
	{
      	if( OLT_CALL_ISERROR( OnuMgt_GetCmcMacAddrTbl(PonPortIdx, OnuIdx, cmc_mac, &active_records, Mac_addr_table) ) )
        {
            active_records = 0;
        }   
     }
    else
    {
        active_records = 0;
    }
  
	if( active_records == 0 ) 
	{
		vty_out(vty, "total Learned mac counter=0\r\n");
		return( ROK );
	}
    
  	vty_out(vty, "\r\ncmc%d/%d/%d mac-learning list\r\n", slot, port, (OnuIdx+1));
	vty_out(vty, "total Learned mac counter from cmc=%d\r\n\r\n", active_records);
	if ( 0 == active_records ) return(ROK );
	
	vty_out(vty, "   Mac addr         type      CmId\r\n");
	vty_out(vty, "-----------------------------------------\r\n");

    n = active_records;
	for(i=0; i<n; i++)
	{
		vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_table[i].mac_address[0], Mac_addr_table[i].mac_address[1],
													Mac_addr_table[i].mac_address[2], Mac_addr_table[i].mac_address[3],
													Mac_addr_table[i].mac_address[4], Mac_addr_table[i].mac_address[5] );
		if( Mac_addr_table[i].type <= 2 )
			vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_table[i].type] );
		vty_out(vty, "   %2d\r\n", Mac_addr_table[i].logical_port );
	}
        
	vty_out(vty, "\r\n");

	return( ROK );
}

int ShowCmcMacLearningCounterByVty( short int PonPortIdx, short int OnuIdx, mac_address_t cmc_mac, struct vty *vty )
{
	short int  PonChipType;
	unsigned short active_records;
	int slot, port;
	
	CHECK_PON_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slot);
		return( CMD_WARNING );
		}*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slot) != ROK)
	{
		vty_out(vty," %% slot %d is not pon card\r\n", slot);
		return( CMD_WARNING );
	}
	
	if( getPonChipInserted((unsigned char )(slot), (unsigned char )(port) ) !=  PONCHIP_EXIST )
	{
		vty_out(vty, "\r\n  pon%d/%d not exist\r\n", slot, port);
		return( ROK );
	} 

	if( PonPortIsWorking(PonPortIdx) == FALSE ) 
	{
		vty_out(vty, "\r\n  pon%d/%d not working\r\n",slot,port);
		return( ROK );
	}
    
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "\r\n %% onu%d/%d is off-line\r\n", slot, port);
		return( RERROR );
	}

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	vty_out(vty, "\r\n  cmc%d/%d/%d mac-learning counter", slot, port, OnuIdx + 1);
	

	if( OLT_PONCHIP_ISTK(PonChipType))
	{
		if( OLT_CALL_ISERROR( OnuMgt_GetCmcMacAddrTbl(PonPortIdx, OnuIdx, cmc_mac, &active_records, NULL) ) )
		{
			vty_out(vty, "=0\r\n");
			return( ROK );
		}
		if( active_records == 0 ) 
		{
			vty_out(vty, "=0\r\n");
			return( ROK );
		}
	}
	else
    {/* 其他PON芯片类型处理*/
		vty_out(vty, "=0\r\n");
		return( ROK );
	}
	
	vty_out(vty, "=%d\r\n\r\n",active_records);

	return( ROK );
}


int ShowCmMacLearningByVty( short int PonPortIdx, short int OnuIdx, mac_address_t cmc_mac, mac_address_t cm_mac, struct vty *vty )
{
	short int PonChipType;
	unsigned short active_records;
	int i, n;
	int slot, port;

	slot = GetCardIdxByPonChip(PonPortIdx );
	port = GetPonPortByPonChip(PonPortIdx);

	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(slot+1) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slot);
		return( RERROR );
		}*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slot) != ROK)
	{
		vty_out(vty," %% slot %d is not pon card\r\n", slot);
		return( RERROR );
	}
	
	if( getPonChipInserted((unsigned char)slot, (unsigned char)port) !=  PONCHIP_EXIST )
	{
		vty_out(vty, "\r\n %% %s/port%d not exist\r\n", CardSlot_s[slot], port );
		return( RERROR );
	} 

	if( PonPortIsWorking(PonPortIdx) == FALSE ) 
	{
		vty_out(vty, "\r\n %% %s/port%d not working\r\n", CardSlot_s[slot], port );
		return( RERROR );
	}
    
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "\r\n %% onu%d/%d is off-line\r\n", slot, port);
		return( RERROR );
	}

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	if( OLT_PONCHIP_ISTK(PonChipType))
	{
      	if( OLT_CALL_ISERROR( OnuMgt_GetCmMacAddrTbl(PonPortIdx, OnuIdx, cmc_mac, cm_mac, &active_records, Mac_addr_table) ) )
        {
            active_records = 0;
        }   
     }
    else
    {
        active_records = 0;
    }
  
	if( active_records == 0 ) 
	{
		vty_out(vty, "total Learned mac counter=0\r\n");
		return( ROK );
	}
    
  	vty_out(vty, "\r\ncmc%d/%d/%d mac-learning list from cm[%02x%02x.%02x%02x.%02x%02x]:\r\n",
                                                            slot, port, (OnuIdx+1),
                                                            cm_mac[0], cm_mac[1],
                                                            cm_mac[2], cm_mac[3],
                                                            cm_mac[4], cm_mac[5]);
	vty_out(vty, "total Learned mac counter from cm=%d\r\n\r\n", active_records);
	if ( 0 == active_records ) return(ROK );
 	
	vty_out(vty, "   Mac addr         type      CmId\r\n");
	vty_out(vty, "-----------------------------------------\r\n");

    n = active_records;
	for(i=0; i<n; i++)
	{
		vty_out(vty, "%02x%02x.%02x%02x.%02x%02x", Mac_addr_table[i].mac_address[0], Mac_addr_table[i].mac_address[1],
													Mac_addr_table[i].mac_address[2], Mac_addr_table[i].mac_address[3],
													Mac_addr_table[i].mac_address[4], Mac_addr_table[i].mac_address[5] );
		if( Mac_addr_table[i].type <= 2 )
			vty_out(vty, "     %s  ", v2r1AddrTableAgeingMode[Mac_addr_table[i].type] );
		vty_out(vty, "   %2d\r\n", Mac_addr_table[i].logical_port );
	}
    
	vty_out(vty, "\r\n");
    
	return( ROK );
}

int ShowCmMacLearningCounterByVty( short int PonPortIdx, short int OnuIdx, mac_address_t cmc_mac, mac_address_t cm_mac, struct vty *vty )
{
	short int  PonChipType;
	unsigned short active_records;
	int slot, port;
	
	CHECK_PON_RANGE

	slot = GetCardIdxByPonChip(PonPortIdx);
	port = GetPonPortByPonChip(PonPortIdx);

	/* 1 板在位检查*/
	/*if( __SYS_MODULE_TYPE__(slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", slot);
		return( CMD_WARNING );
		}*/
	/* 2 pon 板类型检查*/
	if(SlotCardIsPonBoard(slot) != ROK)
	{
		vty_out(vty," %% slot %d is not pon card\r\n", slot);
		return( CMD_WARNING );
	}
	
	if( getPonChipInserted((unsigned char )(slot), (unsigned char )(port) ) !=  PONCHIP_EXIST )
	{
		vty_out(vty, "\r\n  pon%d/%d not exist\r\n", slot, port);
		return( ROK );
	} 

	if( PonPortIsWorking(PonPortIdx) == FALSE ) 
	{
		vty_out(vty, "\r\n  pon%d/%d not working\r\n",slot,port);
		return( ROK );
	}
    
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out( vty, "\r\n %% onu%d/%d is off-line\r\n", slot, port);
		return( RERROR );
	}

	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
		
	vty_out(vty, "\r\n  pon%d/%d mac-learning counter",slot, port);

	if( OLT_PONCHIP_ISTK(PonChipType))
	{
		if( OLT_CALL_ISERROR( OnuMgt_GetCmMacAddrTbl(PonPortIdx, OnuIdx, cmc_mac, cm_mac, &active_records, NULL) ) )
		{
			vty_out(vty, "=0\r\n");
			return( ROK );
		}
		if( active_records == 0 ) 
		{
			vty_out(vty, "=0\r\n");
			return( ROK );
		}
	}
	else
    {/* 其他PON芯片类型处理*/
		vty_out(vty, "=0\r\n");
		return( ROK );
	}
	
	vty_out(vty, "=%d\r\n\r\n",active_records);

	return( ROK );
}


int SetCmcSVlanID(short int PonPortIdx, int iVlanId)
{
    return OLT_SetCmcServiceVID(PonPortIdx, iVlanId);
}

int GetCmcSVlanID(int *iVlanId)
{
    *iVlanId = cmc_service_vlanid;

    return 0;
}

int ResumeCmcSVlanID(short int PonPortIdx)
{
    return OLT_SetCmcServiceVID(PonPortIdx, -1 - cmc_service_vlanid);
}


#if 1
int Cmc_general_assign_handler_function 
                           ( CMC_handlers_t					   *handlers,
	                         const unsigned short int           handler_function_index,
                        	 const cmc_general_handler_function_t 	handler_function,
                             unsigned short                    *handler_id )
{
    int client;

    /* Check that the index is in the range */
    if (handler_function_index > (CMC_MAX_INDEX_HANDLERS-1) )
    {
        return (PARAMETER_ERROR);
    }   
    
    /* Find free client */
    client = -1;
    while ((handlers->functions[++client][handler_function_index] != NULL) &&
           (client < (CMC_MAX_CLIENT-1))
          ) ;
          
    
    if (client == CMC_MAX_CLIENT) 
    {
        return (EXIT_ERROR);
    }

    handlers->functions[client][handler_function_index] = handler_function;
    
    (*handler_id) = (client << 8) | (handler_function_index & 0xFF); 

    return (EXIT_OK);

}


int Cmc_general_delete_handler_function 
                           (       CMC_handlers_t                 *handlers,
                             const unsigned short                  handler_id )

{
    int client,handler_function_index;

    client                 = handler_id >> 8;
    handler_function_index = handler_id & 0xFF;

    /* Check that the index is in the range */
    if (handler_function_index > (CMC_MAX_INDEX_HANDLERS-1) )
    {
        return (PARAMETER_ERROR);
    }   

    if (client > (CMC_MAX_CLIENT-1) )
    {
        return (PARAMETER_ERROR);
    }   

    handlers->functions[client][handler_function_index] = NULL;   

    return (EXIT_OK);
}


int Cmc_general_get_handler_function
                           (       CMC_handlers_t                 *handlers,
	                         const unsigned short int              handler_function_index,
                             const unsigned short                  client_number ,
                             cmc_general_handler_function_t       *handler_function )
{                                
    unsigned int client;
    unsigned int number;

    number = 0;
    (*handler_function) = NULL;
    
    /* Look for the client_number'th client that is not NULL */
    for (client=0; (client<CMC_MAX_CLIENT) && (number != client_number) ; client++)
    {
        if (handlers->functions[client][handler_function_index] == NULL) 
        {
            number++;
        }
    }
 
    if ((number == client_number)) 
    {
        (*handler_function) = handlers->functions[number][handler_function_index];
    }

    return EXIT_OK;
}

int notify_cmc_event(int CmcEventCode, short int PonPortIdx, short int OnuIdx, unsigned char CmcMac[6], unsigned char CmMac[6])
{
    ULONG aulMsg[ 4 ] = {0};
    LONG lResult;
    cmc_msg_head *pstMsg;

    if ( NULL != (pstMsg = (cmc_msg_head*)VOS_Malloc(sizeof(cmc_msg_head), MODULE_RPU_CMC)) )
    {
        pstMsg->msg_code = CmcEventCode;
        pstMsg->msg_len = 0;
        pstMsg->olt_idx = PonPortIdx;
        pstMsg->onu_idx = OnuIdx;
        VOS_MemCpy(pstMsg->cmc_mac, CmcMac, 6);
        if ( CmMac )
        {
            VOS_MemCpy(pstMsg->cm_mac, CmMac, 6);
        }
        else
        {
            VOS_MemSet(pstMsg->cm_mac, 0, 6);
        }
    
        aulMsg[ 0 ] = MODULE_RPU_CMC;
        aulMsg[ 1 ] = CMC_MSG_EVENT;
        aulMsg[ 2 ] = ( ULONG ) CmcEventCode;
        aulMsg[ 3 ] = ( ULONG ) pstMsg;

        lResult = VOS_QueSend( cmc_main_queue, aulMsg, NO_WAIT, MSG_PRI_NORMAL );
        if ( lResult != VOS_OK )
        {
            return VOS_ERROR;
        }
    }
    else
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }

    return VOS_OK;
}


/* CMC配置恢复后，才认为产品Ready */
int cmcReadyHandler(short int PonPortIdx, short int OnuIdx, unsigned char CmcMac[6], unsigned char CmMac[6])
{
    int result = 0;
    int OnuEntryIdx = ONUENTRY_IDX(PonPortIdx, OnuIdx);
    int CmcBytePos = OnuEntryIdx >> 3;
    int CmcBitPos = 1 << (OnuEntryIdx & 7);
    
    if ( !CMC_IS_READY(CmcBytePos, CmcBitPos) )
    {
        CMC_SET_READY(CmcBytePos, CmcBitPos);

        if ( CMC_IS_REGED(CmcBytePos, CmcBitPos) )
        {
            /* CMC配置文件恢复 */
            result = addOnuToRestoreQueue(PonPortIdx, OnuIdx, ONU_CONF_RES_ACTION_DO, OnuProfile_Part_CMC);
        }
        else{} /* 等待CmcOnu的Register完成事件 */
    }

    return result;
}

int cmcTimerDispatch()
{
    VOS_ASSERT(0);
    return 0;
}


int cmcCdpDispatch()
{
    VOS_ASSERT(0);
    return 0;
}

int cmcEventDispatch(cmc_msg_head *pstEventMsg)
{
    int result;

    VOS_ASSERT(pstEventMsg);

    switch (pstEventMsg->msg_code)
    {
        case CMC_EVENT_CMC_ARRIVAL:
        case CMC_EVENT_CMC_READY:
            result = cmcReadyHandler(pstEventMsg->olt_idx, pstEventMsg->onu_idx, pstEventMsg->cmc_mac, pstEventMsg->cm_mac);
            break;
    }

    return result;
}

/* ONU注册时扩展信息更新回调 */
LONG onuCmcMgmt_OnuRegCallback( OnuEventData_s data )
{
    if ( OnuIsCMC(data.PonPortIdx, data.OnuIdx) )
    {
        /* CMC进入CMC扩展发现状态机 */
        int OnuEntryIdx = ONUENTRY_IDX(data.PonPortIdx, data.OnuIdx);
        int CmcBytePos = OnuEntryIdx >> 3;
        int CmcBitPos = 1 << (OnuEntryIdx & 7);
        
        CMC_SET_REGED(CmcBytePos, CmcBitPos);
        if ( CMC_IS_READY(CmcBytePos, CmcBitPos) )
        {
            /* CMC配置文件恢复 */
            addOnuToRestoreQueue(data.PonPortIdx, data.OnuIdx, ONU_CONF_RES_ACTION_DO, OnuProfile_Part_CMC);
        }
        else
        {
            /* 等待CmcController的Ready事件 */
        }
    }
    
	return VOS_OK;
}

/* ONU离线时扩展信息更新回调 */
LONG onuCmcMgmt_OnuDeregCallback( OnuEventData_s data )
{
    if ( OnuIsCMC(data.PonPortIdx, data.OnuIdx) )
    {
        /* CMC进入CMC离线处理 */
        int result;
        int OnuEntryIdx = ONUENTRY_IDX(data.PonPortIdx, data.OnuIdx);
        int CmcBytePos = OnuEntryIdx >> 3;
        int CmcBitPos = 1 << (OnuEntryIdx & 7);
        
        CMC_CLR_REGED(CmcBytePos, CmcBitPos);
        if ( 0 == (result = OnuMgt_UnregisterCmc(data.PonPortIdx, data.OnuIdx, data.onu_mac)) )
        {
            CMC_CLR_READY(CmcBytePos, CmcBitPos);
        }
    }
    
	return VOS_OK;
}
#endif


static void cmc_main(void)
{
    LONG lRlt;
    ULONG ulRcvMsg[ 4 ];
    VOID *pMsg;

    while ( SYS_LOCAL_MODULE_RUNNINGSTATE != MODULE_RUNNING )
    {
        VOS_TaskDelay(VOS_TICK_SECOND);
    }

    while ( 1 )
    {
        if ( VOS_ERROR == (lRlt = VOS_QueReceive( cmc_main_queue, ulRcvMsg, WAIT_FOREVER )) )
        {
            VOS_ASSERT( 0 );
            continue;
        }
        
		if( lRlt == VOS_NO_MSG ) continue;

        pMsg = (VOID*)ulRcvMsg[ 3 ];
        
        if ( MODULE_RPU_CMC == ulRcvMsg[ 0 ] )
        {
            switch ( ulRcvMsg[ 1 ] )
            {
                case CMC_MSG_TIMER:
                    cmcTimerDispatch(ulRcvMsg[ 2 ]);
                    break;
                case CMC_MSG_CDP:
                    cmcCdpDispatch( (cmc_msg_head*)pMsg );
                    CDP_FreeMsg(pMsg);
                    pMsg = NULL;
                    break;
                case CMC_MSG_EVENT:
                    cmcEventDispatch( (cmc_msg_head*)pMsg );
                    break;
                default:
                    VOS_ASSERT(0);
            }
        }
        else
        {
			SYS_MSG_S *pSysMsg = (SYS_MSG_S *)pMsg;
            
			if( (NULL != pSysMsg) && (AWMC_CLI_BASE == pSysMsg->usMsgCode) )
            {
				decode_command_msg_packet( pSysMsg, VOS_MSG_TYPE_QUE );
            }         
        }
        
        if ( NULL != pMsg )
        {
            VOS_Free( pMsg );
        }
    }
   
    cmc_main_task = 0;
    VOS_TaskExit(0);
    
    return;
}


int cmc_init()
{
    ULONG ulSize;

    ulSize = MAXONU >> 2;
    if ( NULL == (onu_regflags = (U8*)VOS_Malloc(ulSize, MODULE_RPU_CMC)) )
    {
    	VOS_ASSERT( 0 );
        return VOS_ERROR;
    }
    cmc_rdyflags = onu_regflags + (ulSize >> 1);
    VOS_MemZero(onu_regflags, ulSize);

    cmc_service_vlanid = 0;
    VOS_MemZero(&cmc_handlers, sizeof(cmc_handlers));

    cmc_main_queue = VOS_QueCreate(CMC_MAIN_QUEUE_LENGTH, VOS_MSG_Q_FIFO);
    if( 0 == cmc_main_queue )
    {
    	VOS_ASSERT( 0 );
        return VOS_ERROR;
    }

    cmc_main_task = VOS_TaskCreate("tCmcMain", TASK_PRIORITY_NORMAL, (VOS_TASK_ENTRY)cmc_main, NULL);
    if ( 0 == cmc_main_task )
    {
    	VOS_ASSERT( 0 );
        return VOS_ERROR;
    }

    VOS_QueBindTask( ( VOS_HANDLE ) cmc_main_task, cmc_main_queue );

	reigster_onuevent_callback(ONU_EVENT_CODE_REGISTER, (g_OnuEventFuction_callback)onuCmcMgmt_OnuRegCallback);
	reigster_onuevent_callback(ONU_EVENT_CODE_DEREGISTER, (g_OnuEventFuction_callback)onuCmcMgmt_OnuDeregCallback);
	reigster_onuevent_callback(ONU_EVENT_CODE_REMOVE, (g_OnuEventFuction_callback)onuCmcMgmt_OnuDeregCallback);
    
#if( EPON_MODULE_DOCSIS_MANAGE_MIB == EPON_MODULE_YES )
    if ( 0 == (s_sem_cmc_mib = VOS_SemMCreate(VOS_SEM_Q_PRIORITY)) )
    {
        VOS_ASSERT(0);
    }

    cmcMib_Mgmt_InitParamters();
    cmcMib_Sf_InitParamters();
#endif

    return 0;
}


#endif


#ifdef __cplusplus
}
#endif

