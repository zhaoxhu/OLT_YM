
/***************************************************************
*
*						Module Name:  PonChipMgmt.c
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
*   Date: 			2006/04/20
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  06/04/20  |   chenfj          |     create 
**----------|-----------|------------------
**  major modified History:
**  
**  1  2008/6/30  chenfj
**      增加产品类型识别, 修改PON芯片管理MAC 地址及
**      加载/跟踪等数据的定义和初始化修改PON 芯片
**      ID 与槽位/端口之间的映射
** 
***************************************************************/
#ifdef __cplusplus
extern "C"
  {
#endif


/*
#include "OSSRV_expo.h"

#include "PAS_expo.h"
*/

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "includefromPas.h"


/* wangysh add*/
#include "sys/devsm/devsm_main.h"
#include "cpi/cdsms/cdsms_main.h"
#include "cpi/cdsms/cdsms_file.h"
#include "device/device_flash.h"

extern int fpga_ver_get( int fileID );


/* pon chip status */
unsigned char *PONchipStatus[] = {
		(unsigned char *)"pon chip_undefined ",
		(unsigned char *)"pon chip_Up(Activated)",
		(unsigned char *)"pon chip_notLoading",	
		(unsigned char *)"pon chip_Testing",
		(unsigned char *)"pon chip_Loading",
		(unsigned char *)"pon chip_Down",
		(unsigned char *)"pon chip_Dormant",
		(unsigned char *)"pon chip_Notpresent",
		(unsigned char *)"pon chip_err"			
		};

/* pon chip name */
static const char* s_apszChipNames[PONCHIP_TYPE_MAX+1] = {
                                    "NULL",
                                        
                                    "PAS5001",
                                    "PAS5201",
                                    "PAS5204",
                                    "PAS8411",
                                    "PAS???1",
                                    "PAS???2",

                                    "TK3723",
                                    "BCM55524",
                                    "BCM55538",
                                    "BCM68622",
                                    "BCM????2",

                                    "CT???1",
                                    "CT???2",
                                    "CT???3",
                                    
                                    "GW?????",
                                    "GPON",
                                    "LAST"
                                    };

/* pon chip type */
unsigned char *PonChipType_s[PONCHIP_TYPE_MAX+1] = {
		(unsigned char *)"pon chip_UnKnown", /* 0 */

		(unsigned char *)"pon_chip_PAS",     /* 1 */
		(unsigned char *)"pon_chip_PAS",     /* 2 */
		(unsigned char *)"pon_chip_PAS",     /* 3 */
		(unsigned char *)"pon_chip_PAS",     /* 4 */
		(unsigned char *)"pon_chip_PAS",     /* 5 */
		(unsigned char *)"pon_chip_PAS",     /* 6 */

		(unsigned char *)"pon_chip_TK",      /* 7 */
		(unsigned char *)"pon_chip_BCM",     /* 8 */
		(unsigned char *)"pon_chip_BCM",     /* 9 */
		(unsigned char *)"pon_chip_BCM",     /* 10 */
		(unsigned char *)"pon_chip_BCM",     /* 11 */

		(unsigned char *)"pon_chip_CT",      /* 12 */
		(unsigned char *)"pon_chip_CT",      /* 13 */
		(unsigned char *)"pon_chip_CT",      /* 14 */
		
		(unsigned char *)"pon_chip_GW",      /* 15 */
		(unsigned char *)"pon_chip_GPON",      /* 16 */
		(unsigned char *)"pon chip_MAX"      /* 17 */
		};

/* pon interface type */
unsigned char *PonInterfaceType[] = {
		(unsigned char *)"1000Base_PX OLT",
		(unsigned char *)"1000Base_PX ONU",
		(unsigned char *)"1000Base_PX10D OLT",
		(unsigned char *)"1000Base_PX10D ONU",
		(unsigned char *)"1000Base_PX10U OLT",
		(unsigned char *)"1000Base_PX10U ONU",
		(unsigned char *)"1000Base_PX20D OLT",
		(unsigned char *)"1000Base_PX20D ONU",
		(unsigned char *)"1000Base_PX20U OLT",
		(unsigned char *)"1000Base_PX20U ONU"
		};

/*
**     Return codes handling
**
**     Definination of string to each return code, in order to print error 
**     string to the CLI.
**     
*/

struct CLI_return_code_str_t  PAS_CLI_return_code_str[] =
{
  { PAS_EXIT_OK, "exit ok" },
  { PAS_EXIT_ERROR, "exit error" },
  { PAS_TIME_OUT, "time out" },
  { PAS_NOT_IMPLEMENTED, "not implemented" },
  { PAS_PARAMETER_ERROR, "parameter error" },
  { PAS_HARDWARE_ERROR, "hardware error" },
  { PAS_MEMORY_ERROR, "memory error" },
  { PAS_PC_UART_ERROR, "pc uart error" },
  { PAS_PC_CRC_ERROR, "pc crc error" },
  { PAS_PARSE_ANSWER_ERROR, "parse answer error" },
  { PAS_OLT_NOT_EXIST, "OLT not exist" },
  { PAS_QUERY_FAILED, "query failed" },
  { PAS_NOT_SUPPORTED_IN_CURRENT_HARDWARE_VERSION, "not supported in current HW version" },
  { PAS_OAM_LINK_NOT_ESTABLISHED, "OAM link not established" },
  { PAS_ONU_NOT_AVAILABLE, "ONU not available" },
  { PAS_DBA_DLL_NOT_LOADED, "dba dll not loaded" },
  { PAS_DBA_ALREADY_RUNNING, "dba already running" },
  { PAS_DBA_NOT_RUNNING, "dba not running" },
  { PAS_ADDRESS_TABLE_FULL, "address table is full" },
  { PAS_ADDRESS_TABLE_ENTRY_LIMITER_FULL, "address table entry limiter is full" },
  
  
  { 1, NULL },
};

/*unsigned char Host_Default_Mac[] = { 0x00, 0x04, 0xac, 0x00, 0x01, 0xe2 };*/
unsigned char PAS_Mgmt_Mac[] = { 0x00, 0x0c, 0xd5, 0x00, 0x01, 0x00};
unsigned char PAS_Data_Mac[] = {0x00,0x0c,0xd5,0x62,0xbb,0x00};  /*{ 0x00, 0x0c, 0xd5, 0x00, 0x00, 0x00}; */
unsigned char PonChipSlotAddr[] = {0x00, 0x00, 0x00,  0xec, 0xf0, 0xf4, 0xf8, 0xfc};
int PonChipActivatedFlag[PRODUCT_MAX_TOTAL_SLOTNUM+1] = {FALSE};   /* MAC_SLOT_ID */
int PonPhyDebug[SYS_MAX_PON_PORTNUM]    = {2};  /* MAXPON */
int PonMsgDebug[SYS_MAX_PON_PORTNUM]    = {2};  /* MAXPON */
int PonOamMsgDebug[SYS_MAX_PON_PORTNUM] = {2};
short int PonOamDebugOltId = 0;
short int PonOamDebugLlid = 0;
short int PonOamDebugSend = 0;
short int PonOamDebugRec = 0;
unsigned char OnuMacAddrDebug[BYTES_IN_MAC_ADDRESS] = {0};
unsigned short int TkPonPhyMsgCmdOpFirst = 0;
unsigned short int TkPonPhyMsgCmdOpSecond = 0;
unsigned short int TkPonPhyMsgEventOp = 0;
unsigned int PON_DOWNLOAD_MAX_COUNTER = 3;
unsigned char PonChipDownloadCounter[SYS_MAX_PON_PORTNUM] = {0};
unsigned char PonBoardDownloadCounter[PRODUCT_MAX_TOTAL_SLOTNUM+1] = {0}; /* MAC_SLOT_ID */
unsigned char PonChipResetFlag[SYS_MAX_PON_PORTNUM] = {V2R1_DISABLE};
void update_file_ver( int fileID );

/*extern int i2c_data_get( UINT slot_id, UINT type, UINT reg, UCHAR *pdata, UINT len);*/

int setPonTraceFlag(short int PonPortIdx,  int flag )
{
	CHECK_PON_RANGE
	PonMsgDebug[PonPortIdx] = flag;
	return( ROK );
}

void ClearPonActivatedFlag()
{
	unsigned char i;
    
	for(i=0; i<SYS_CHASSIS_SLOTNUM; i++)
		PonChipActivatedFlag[i] = FALSE;
}

void ClearSinglePonSlotCounter(unsigned CardIndex )
{
	short int i;
	short int pon;
    
	for(i=1; i<=PONPORTPERCARD; i++)
	{
		/* modified by xieshl 20080812 */
		pon = GetPonPortIdxBySlot(CardIndex, i);
		if( pon != RERROR )
			PonChipDownloadCounter[pon] = 0;
	}
	PonBoardDownloadCounter[CardIndex] = 0;
}

void ClearPonDownloadCounter()
{
	unsigned i;
	for(i=0; i<MAXPON; i++)
		PonChipDownloadCounter[i] = 0;
	for(i=0; i<SYS_CHASSIS_SLOTNUM; i++)
		PonBoardDownloadCounter[i] = 0;
}

void InitPonMsgDebugFlag()
{
	int i;
	for(i=0; i<MAXPON; i++)
	{
		PonPhyDebug[i] = V2R1_DISABLE;
		PonMsgDebug[i] = V2R1_DISABLE;
		PonOamMsgDebug[i] = V2R1_DISABLE;
	}	
}

#if 1
#undef OLT_ID_CODE_FOR_SMALLMEM

int GetGlobalCardIdxByPonChip( short int PonPortGlobalIdx )
{
    int iCardIdx;
   
    if ( OLT_ISLOCAL(PonPortGlobalIdx) )
    {
        iCardIdx = PonPortGlobalIdx / PONPORTPERCARD;
        if ( (iCardIdx >= 0) && (iCardIdx < SYS_MAX_PON_CARDNUM) )
        {
            VOS_ASSERT(iCardIdx < MAX_PON_BOARD_DEVNUM);
            iCardIdx = PON[iCardIdx];
        }
        else
        {
            VOS_ASSERT(0);
            return RERROR;
        }
    }
    else
    {
        iCardIdx = OLT_SLOT_ID(PonPortGlobalIdx);
    }

    return iCardIdx;
}

int GetCardIdxByPonChip( short int PonChipIdx )
{
    int iCardIdx;

    if ( OLT_ISLOCAL(PonChipIdx) )
    {
        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
        {
        	iCardIdx = PonChipIdx / PONPORTPERCARD;
            if ( (iCardIdx >= 0) && (iCardIdx < SYS_MAX_PON_CARDNUM) )
            {
                VOS_ASSERT(iCardIdx < MAX_PON_BOARD_DEVNUM);
                iCardIdx = PON[iCardIdx];
            }
            else
            {
                VOS_ASSERT(0);
                return RERROR;
            }
        }
        else
        {
            iCardIdx = SYS_LOCAL_MODULE_SLOTNO;
        }
    }
    else
    {
        iCardIdx = OLT_SLOT_ID(PonChipIdx);
    }

    return iCardIdx;
}

int GetPonPortByPonChip( short int PonChipIdx )
{
    int PonPort;
       
    if ( OLT_ISLOCAL(PonChipIdx) )
    {
#ifdef OLT_ID_CODE_FOR_SMALLMEM
        int iCardIdx;
        
        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
        {
            iCardIdx = PonChipIdx / PONPORTPERCARD;
            if ( (iCardIdx >= 0) && (iCardIdx < SYS_MAX_PON_CARDNUM) )
            {
                VOS_ASSERT(iCardIdx < MAX_PON_BOARD_DEVNUM);
                iCardIdx = PON[iCardIdx];
            }
            else
            {
                VOS_ASSERT(0);
                return RERROR;
            }
        }
        else
        {
            iCardIdx = SYS_LOCAL_MODULE_SLOTNO;
        }        

        if ( SYS_MODULE_IS_UPLINK_PON(iCardIdx) )
        {
            PonPort = (PonChipIdx % PONPORTPERCARD) + 5; 
        }
        else
#endif
        {
            PonPort = (PonChipIdx % PONPORTPERCARD) + 1; 
        }
    }
    else
    {
        PonPort = OLT_PORT_ID(PonChipIdx);
    }
    
	return (PonPort);
}

int GetGlobalPonPortByPonChip( short int PonPortGlobalIdx )
{
	return GetPonPortByPonChip(PonPortGlobalIdx);
}

short int GetPonPortIdxBySlot( short int slot, short int port )
{
    int i, n;
    short int PonPortIdx = RERROR;

    if ( SYS_MODULE_IS_LOCAL(slot) )
    {        
        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
        {
            n = MAX_PON_BOARD_NUM;
            for (i=0; i<n; i++)
            {
                if ( slot == PON[i] )
                {
#ifdef OLT_ID_CODE_FOR_SMALLMEM
                    if ( SYS_MODULE_IS_UPLINK_PON(slot) )
                    {
                        PonPortIdx = port-1;
                        if (SYS_MODULE_IS_8100_PON(slot)&&((0 <= PonPortIdx) && (PonPortIdx < MAX_PONPORT_PER_BOARD)))
                        {
                            PonPortIdx += i * PONPORTPERCARD;
                        }
						else if (!SYS_MODULE_IS_8100_PON(slot)&&(0 <= (PonPortIdx = port-5)))
                        {
                            PonPortIdx += i * PONPORTPERCARD;
                        }
                        else
                        {
                            PonPortIdx = RERROR;
                        }
                    }
                    else
#endif
				/*changed by wangying 确保8100的PON口不超出上限。*/
				if (SYS_LOCAL_MODULE_TYPE_IS_8100_PON)
				{
					PonPortIdx = port-1;
					if (((0 <= PonPortIdx) && (PonPortIdx < MAX_PONPORT_PER_BOARD)))
					{
					  PonPortIdx += i * PONPORTPERCARD;
					}
					else
					{
					    PonPortIdx = RERROR;
					}	  
				}
				 else
	                    {
	                        if ( 0 <= (PonPortIdx = port - 1) )
	                        {
	                            PonPortIdx += i * PONPORTPERCARD;
	                        }
	                        else
	                        {
	                            PonPortIdx = RERROR;
	                        }
	                    }

                    break;
                }
            }
        }
		else
		{
            if ( slot == SYS_LOCAL_MODULE_SLOTNO )
            {
#ifdef OLT_ID_CODE_FOR_SMALLMEM
                if ( SYS_MODULE_IS_UPLINK_PON(slot) )
                {
					if( 0 > (PonPortIdx = port - 5) )
					{
                        PonPortIdx = RERROR;
                    }
                }
                else
#endif
                {
                    if ( 0 > (PonPortIdx = port - 1) )
                    {
                        PonPortIdx = RERROR;
                    }
                }
            }
            else
            {
                PonPortIdx = OLT_DEVICE_ID(slot, port);
            }
        }	
    }
    else
    {
        PonPortIdx = OLT_DEVICE_ID(slot, port);
    }
    
    return( PonPortIdx );
}

short int GetGlobalPonPortIdxBySlot( short int slot, short int port )
{
    int i, n;
    short int PonPortIdx = RERROR;

    if ( SYS_MODULE_IS_LOCAL(slot) )
    {
        n = MAX_PON_BOARD_DEVNUM;
        for (i=0; i<n; i++)
        {
            if ( slot == PON[i] )
            {
#ifdef OLT_ID_CODE_FOR_SMALLMEM
                if ( SYS_MODULE_IS_UPLINK_PON(slot) )
                {
                    if ( 0 <= (PonPortIdx = port - 5) )
                    {
                        PonPortIdx += i * PONPORTPERCARD;
                    }
                    else
                    {
                        PonPortIdx = RERROR;
                    }
                }
                else
#endif
                {
                    if ( 0 <= (PonPortIdx = port - 1) )
                    {
                        PonPortIdx += i * PONPORTPERCARD;
                    }
                    else
                    {
                        PonPortIdx = RERROR;
                    }
                }
                
                break;
            }
        }
    }
    else
    {
        PonPortIdx = OLT_DEVICE_ID(slot, port);
    }

    return( PonPortIdx );
}

short int GetGlobalPonPortIdx( short int PonPortIdx )
{
    if ( PRODUCT_IS_DISTRIBUTE )
    {
        if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
        {
            /* 主控板定位全局PON */
            if ( OLT_ISREMOTE(PonPortIdx) )
            {
                if ( SYS_MODULE_IS_LOCAL(OLT_SLOT_ID(PonPortIdx)) )
                {
                    PonPortIdx = GetGlobalPonPortIdxBySlot(OLT_SLOT_ID(PonPortIdx), OLT_PORT_ID(PonPortIdx));
                }
            }
        }
        else    
        {
            if ( OLT_ISLOCAL(PonPortIdx) )
            {
                if ( PonPortIdx < MAXPON )
                {
                    /* PON卡只需作自己卡内OLT 的设备内ID全局映射 */
#ifdef OLT_ID_CODE_FOR_SMALLMEM
                    if ( SYS_LOCAL_MODULE_TYPE_IS_UPLINK_PON )
                    {
                        PonPortIdx += 5; 
                    }
                    else
#endif
                    {
                        PonPortIdx += 1; 
                    }
                    
                    PonPortIdx = GetGlobalPonPortIdxBySlot(SYS_LOCAL_MODULE_SLOTNO, PonPortIdx);
                }
            }
            else
            {
                if ( SYS_MODULE_IS_LOCAL(OLT_SLOT_ID(PonPortIdx)) )
                {
                    PonPortIdx = GetGlobalPonPortIdxBySlot(OLT_SLOT_ID(PonPortIdx), OLT_PORT_ID(PonPortIdx));
                }
            }
        }   
    }    
    else
    {
        /* 集中式管理的PON，无需转换ID */
    }

    return PonPortIdx;
}

short int GetLocalPonPortIdx( short int PonPortIdx )
{
    if ( PonPortIdx >= MAXPON )
    {
        if ( OLT_ISLOCAL(PonPortIdx) )
        {
            if ( !SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
            {
                int slot, port;

                slot = GetGlobalCardIdxByPonChip(PonPortIdx);
                port = GetGlobalPonPortByPonChip(PonPortIdx);
                
                PonPortIdx = OLT_DEVICE_ID(slot, port);
            }
            else
            {
                VOS_ASSERT(0);
            }
        }
        else
        {
            if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
            {
                if ( SYS_MODULE_IS_LOCAL(OLT_SLOT_ID(PonPortIdx)) )
                {
                    PonPortIdx = GetPonPortIdxBySlot(OLT_SLOT_ID(PonPortIdx), OLT_PORT_ID(PonPortIdx));
                }
            }
            else
            {
                if ( SYS_LOCAL_MODULE_SLOTNO == OLT_SLOT_ID(PonPortIdx) )
                {
                    PonPortIdx = GetPonPortIdxBySlot(OLT_SLOT_ID(PonPortIdx), OLT_PORT_ID(PonPortIdx));
                }
            }
        }
    }

    return PonPortIdx;
}

/* B--added by liwei056@2013-3-15 for D16711 */
short int GetBasePonPortIdx( short int PonPortBaseIdx, short int PonPortIdx )
{
    if ( SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER )
    {
        if ( OLT_ISREMOTE(PonPortBaseIdx) && OLT_ISNET(PonPortBaseIdx) )
        {
            /* 逻辑板定位全局PON */
#if ( EPON_MODULE_PON_REMOTE_MANAGE == EPON_MODULE_YES )
            /* 不可能2个都是逻辑OLT  */
            OLT_LOCAL_ASSERT(PonPortIdx);
            
            /* 设备间OLT 的ID映射 */
            PonPortIdx = OLTRM_GetRemoteLogicalOltID(PonPortBaseIdx);
#else
            PonPortIdx = RERROR;
#endif
        }
        else
        {
            if ( PRODUCT_IS_DISTRIBUTE )
            {
                /* PON板定位全局PON */
                int iBaseSlot, iPonSlot, iPonPort;
                
                iBaseSlot = GetGlobalCardIdxByPonChip(PonPortBaseIdx);
                iPonSlot  = GetGlobalCardIdxByPonChip(PonPortIdx);
                iPonPort  = GetGlobalPonPortByPonChip(PonPortIdx);
                if ( iBaseSlot == iPonSlot )
                {
                    PonPortIdx = iPonPort - 1;
                }
                else
                {
                    PonPortIdx = OLT_DEVICE_ID(iPonSlot, iPonPort);
                }
            }
            else
            {
                /* 集中式管理的PON，无需转换ID */
            }
        }
    }    
    else    
    {
        if ( OLT_ISLOCAL(PonPortIdx) )
        {
            if ( OLT_ISLOCAL(PonPortBaseIdx) )
            {
                if ( PonPortIdx >= MAXPON )
                {
                    PonPortIdx = GetLocalPonPortIdx(PonPortIdx);
                }
            }
            else
            {
                VOS_ASSERT( PonPortIdx < MAXPON );
                PonPortIdx = OLT_DEVICE_ID(SYS_LOCAL_MODULE_SLOTNO, PonPortIdx + 1);
            }
        }
        else
        {
            if ( SYS_LOCAL_MODULE_SLOTNO == OLT_SLOT_ID(PonPortIdx) )
            {
                PonPortIdx = GetPonPortIdxBySlot(OLT_SLOT_ID(PonPortIdx), OLT_PORT_ID(PonPortIdx));
            }
        }
    }   

    return PonPortIdx;
}
/* E--added by liwei056@2013-3-15 for D16711 */

/* B--added by liwei056@2013-7-17 for 6900M */
int PonPortIsLocal( short int PonChipIdx )
{
    if ( PRODUCT_IS_DISTRIBUTE )
    {
        if ( PonChipIdx < MAXPON )
        {
            if ( SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER )
            {
                if ( SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
                {
                    short int sLocalPonIdxBegin = GetGlobalPonPortIdxBySlot(SYS_LOCAL_MODULE_SLOTNO, 1);
                    short int sLocalPonIdxEnd = sLocalPonIdxBegin + PONPORTPERCARD - 1;

                    if ( (PonChipIdx > sLocalPonIdxEnd) || (PonChipIdx < sLocalPonIdxBegin) )
                    {
                        return FALSE;
                    }
                }
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}
/* E--added by liwei056@2013-7-17 for 6900M */
#endif

int GetOltChipVendorID(short int PonChipType)
{
    int iPonChipVendor = PONCHIP_VENDOR_UNKNOWN;

    if ( OLT_PONCHIP_ISPAS(PonChipType) )
    {
        iPonChipVendor = PONCHIP_VENDOR_PAS;    
    }
    else if ( OLT_PONCHIP_ISTK(PonChipType) )
    {
        iPonChipVendor = PONCHIP_VENDOR_TK;    
    }
    else if ( OLT_PONCHIP_ISCT(PonChipType) )
    {
        iPonChipVendor = PONCHIP_VENDOR_CT;    
    }
    else if ( OLT_PONCHIP_ISGW(PonChipType) )
    {
        iPonChipVendor = PONCHIP_VENDOR_GW;    
    }
    else
    {
        VOS_ASSERT(0);
    }
    
    return iPonChipVendor;
}

char *GetVendorName(unsigned int PonChipVendorId)
{
    static const char *apcszPonChipVendors[PONCHIP_VENDOR_MAX+1] = {
                                                                    "unknown",
                                                                    "PMC",  
                                                                    "TK",  
                                                                    "BCM",  
                                                                    "CT",  
                                                                    "GW",
                                                                    "error"
                                                                    };

    if ( PonChipVendorId > PONCHIP_VENDOR_MAX )
    {
        PonChipVendorId = PONCHIP_VENDOR_MAX;
    }
    
    return apcszPonChipVendors[PonChipVendorId];
}

/*****************************************************
 *
 *    Function:  V2R1_GetPonchipType(short int PonChipIdx)
 *
 *    Param: short int PonChipIdx -- the specific pon chip 
 *
 *    Desc:   
 *
 *    Return:   pon chip type
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
short int V2R1_GetPonchipType(short int PonChipIdx)
{
	int PonCardIdx;
	int cardInserted;
	short int PonPortIdx;

	PonPortIdx = PonChipIdx;
	CHECK_PON_RANGE

	PonCardIdx = GetCardIdxByPonChip( PonChipIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );

#if 1
    return OLTAdv_GetChipTypeID(PonChipIdx);
#else
	if(( PonChipMgmtTable[PonChipIdx].Type > PONCHIP_TYPE_MIN ) && ( PonChipMgmtTable[PonChipIdx].Type < PONCHIP_TYPE_MAX )) return ( PonChipMgmtTable[PonChipIdx].Type );

	return ( RERROR );
#endif
}

short int  GetPonChipTypeByPonPort( short int PonPortIdx )
{
	short int PonChipIdx;

	CHECK_PON_RANGE
	PonChipIdx = GetPonChipIdx(PonPortIdx);
	if(PonChipIdx == RERROR ) return ( RERROR );

	return( V2R1_GetPonchipType( PonChipIdx ));
}

/*****************************************************
 *
 *    Function:  V2R1_GetPonChipVersion( short int PonChipIdx )
 *
 *    Param: short int PonChipIdx -- the specific pon chip 
 *
 *    Desc:   
 *
 *    Return:   pon chip VERSION;
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
/* modified for PAS-SOFT V5.1*/ 
short int  V2R1_GetPonChipVersion( short int PonChipIdx )
{
	short int PonChipType = 0;

	PonChipType = V2R1_GetPonchipType(PonChipIdx );
	if( PonChipType == RERROR )
		return RERROR;
	
	
	if( OLT_PONCHIP_ISPAS(PonChipType) )
		{
		if(( PonChipMgmtTable[PonChipIdx].version == PONCHIP_PAS5001 )
            || ( PonChipMgmtTable[PonChipIdx].version == PONCHIP_PAS5201 )
            || ( PonChipMgmtTable[PonChipIdx].version == PONCHIP_PAS5204 )
            || ( PonChipMgmtTable[PonChipIdx].version == PONCHIP_PAS8411 )/*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
            ) 
			return ( PonChipMgmtTable[PonChipIdx].version);
		}

	else{ /* other pon chip handler */

		}
	
	return ( RERROR );

}

/*extern  long CTC_StackSupported;*/
/*****************************************************
 *
 *    Function:  int  IsSupportCTCOnu(short int PonPortIdx )
 *
 *    Param: short int PonChipIdx -- the specific pon chip 
 *
 *    Desc:   
 *
 *    Return:  whether CTC ONU is  supported by this pon chip or not
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int  IsSupportCTCOnu(short int PonPortIdx )
{
	CHECK_PON_RANGE

	/*if( PonPortIsWorking( PonPortIdx) != TRUE ) return( RERROR  );*/
	return( PonPortTable[PonPortIdx].CTC_Supported );
}

/*****************************************************
 *
 *    Function:  GetPonchipMgmtMacAddr( short int PonChipIdx, unsigned char *MacAddr )
 *
 *    Param: short int PonChipIdx -- the specific pon chip 
 *                unsigned char *MacAddr -- output the pon chip mgmt channel mad addr
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int  GetPonchipMgmtMacAddr( short int PonChipIdx, unsigned char *MacAddr )
{
	int PonCardIdx;
	int cardInserted;

	if(MacAddr == NULL ) return( RERROR );
	VOS_MemZero(MacAddr,BYTES_IN_MAC_ADDRESS);
	
	PonCardIdx = GetCardIdxByPonChip( PonChipIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );

	VOS_MemCpy( MacAddr , PonChipMgmtTable[PonChipIdx].MgmtPathMAC, BYTES_IN_MAC_ADDRESS );
	return( ROK );
}

int GetPonchipMgmtMacAddr2( short int PonChipIdx, unsigned char *MacAddr )
{
    VOS_ASSERT(MacAddr);
    OLT_LOCAL_ASSERT(PonChipIdx);
   
	VOS_MemCpy( MacAddr , PonChipMgmtTable[PonChipIdx].MgmtPathMAC, BYTES_IN_MAC_ADDRESS );

	return( ROK );
}

int GetPonchipMgmtMacAddr3( int slot, int port, unsigned char *MacAddr )
{
    short int olt_id = GetPonPortIdxBySlot( (short int)slot, (short int)port );

    VOS_ASSERT(MacAddr);

    if ( OLT_LOCAL_ISVALID(olt_id) )    
    {
        VOS_MemCpy(MacAddr, PonChipMgmtTable[olt_id].MgmtPathMAC, BYTES_IN_MAC_ADDRESS);
    }
    else
    {
		return VOS_ERROR;
    }

	return( ROK );
}

/*****************************************************
 *
 *    Function:   GetPonchipFirmwareVer( short int PonChipIdx, unsigned char *FirmwareVer )
 *
 *    Param: short int PonChipIdx -- the specific pon chip 
 *                unsigned char *FirmwareVer -- output the pon chip Frimware version
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
 int  GetPonchipFirmwareVer( short int PonPortIdx, unsigned char *FirmwareVer )
{
	int PonCardIdx;
	int cardInserted;
	int len;

	if( FirmwareVer == NULL ) return( RERROR );
	FirmwareVer[0] = '\0';
	
	CHECK_PON_RANGE
	
	PonCardIdx = GetCardIdxByPonChip( PonPortIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );

	len = VOS_StrLen(PonChipMgmtTable[PonPortIdx].FirmwareVer);
	if(len > 32 ) len = 32;
	VOS_StrnCpy(FirmwareVer,  PonChipMgmtTable[PonPortIdx].FirmwareVer, len);	/* modified by xieshl 20091216, 防止丢失结束符 */
	FirmwareVer[len] = 0;
	return( ROK );
}

static int GetPonchipHostSwVer(short int PonPortIdx, unsigned char *HostSwVer)
{
	int PonCardIdx;
	int cardInserted;
	int len;

	if( HostSwVer == NULL ) return( RERROR );
	HostSwVer[0] = '\0';
	
	CHECK_PON_RANGE
	
	PonCardIdx = GetCardIdxByPonChip( PonPortIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );

	len = VOS_StrLen(PonChipMgmtTable[PonPortIdx].HostSwVer);
	if(len > 32 ) len = 32;
	VOS_StrnCpy(HostSwVer, PonChipMgmtTable[PonPortIdx].HostSwVer, len);	/* modified by xieshl 20091216, 防止丢失结束符 */
	HostSwVer[len] = 0;
	return( ROK );
}

static int  GetPonchipDBAVer( short int PonPortIdx, unsigned char *DBA_type, unsigned char *DBA_ver)
{
	short int ret = RERROR;
	unsigned char DBA_type_Size, DBA_ver_Size;
#if 0        
	short int PonChipType, PonChipVer;
#else
	OLT_DBA_version_t stDBAinfo;
#endif
	
	CHECK_PON_RANGE

	DBA_type_Size = 10;
	DBA_ver_Size = 10;

	if(( DBA_type == NULL ) ||( DBA_ver == NULL )) return( RERROR );
	DBA_type[0] = '\0';
	DBA_ver[0] = '\0';
	
#if 1        
    ret = OLT_GetDBAVersion(PonPortIdx, &stDBAinfo);
    if ( OLT_CALL_ISOK(ret)  )
    {
        ret = 0;
		DBA_ver[0] = 'V';

        VOS_ASSERT(DBA_type_Size < sizeof(stDBAinfo.szDBAname));
        VOS_ASSERT(DBA_ver_Size < sizeof(stDBAinfo.szDBAversion));
               
        VOS_MemCpy(DBA_type, stDBAinfo.szDBAname, DBA_type_Size);
        VOS_MemCpy(DBA_ver+1, stDBAinfo.szDBAversion, DBA_ver_Size);
    }
#else
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx);
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}

	if( PonChipType == PONCHIP_PAS )
	{
            
		DBA_ver[0] = 'V';

#ifdef PLATO_DBA_V3
		ret = PLATO3_get_info( PonPortIdx, DBA_type, DBA_type_Size, &DBA_ver[1], DBA_ver_Size );
#else
		ret = PLATO2_get_info( PonPortIdx, DBA_type, DBA_type_Size, &DBA_ver[1], DBA_ver_Size );
#endif
		if( ret == PAS_EXIT_OK )
		{
			DBA_ver_Size++;
			if(EVENT_DEBUG == V2R1_ENABLE)
			{
				sys_console_printf("\r\nGet DBA Info ok \r\n");
				/*sys_console_printf(" DBA name: %s ,  name len : %d \r\n", DBA_type, DBA_type_Size);
				sys_console_printf(" DBA version: %s, version len: %d \r\n", DBA_ver, DBA_ver_Size );*/
			}
			DBA_type[DBA_type_Size] = '\0';
			DBA_ver[DBA_ver_Size]   = '\0';
			ret = ROK;
		}
		else if( ret  != PAS_EXIT_OK ) 
		{
			sys_console_printf("\r\nGet DBA Info err, RetCode : %d (GetDBAInfo()) \r\n", ret);
			ret = RERROR;
		}
	}
	else
    { /* other pon chip handler */	
	}
#endif
    
	return( ret );
}

/* added by chenfj 2008-3-27
     取PON 板固件版本/ 软件版本及DBA 版本
     slot 取值: 4 - 8
     */
int GetPonCardVersion( unsigned int slot, unsigned char *FirmwareVer, unsigned char *HostSwVer, unsigned char *DBA_type, unsigned char *DBA_ver)
{
	short int PonPortIdx = RERROR;
	unsigned char i, flag = PONCHIP_NOT_EXIST;
	unsigned char DBA_type1[128], DBA_ver1[128];
    
	if(SlotCardIsPonBoard(slot) != ROK )
	{
		return( RERROR );
	}

	for(i=1; i<PONPORTPERCARD; i++)
	{
		if( getPonChipInserted( (unsigned char)slot, i ) == PONCHIP_EXIST )
		{
			PonPortIdx = GetPonPortIdxBySlot(slot, i);
			if( OLTAdv_IsExist(PonPortIdx) ==  TRUE )
			{
				flag = PONCHIP_EXIST;
				break;
			}
		}
	}
	if( flag != PONCHIP_EXIST ) return ( RERROR );

	if( FirmwareVer != NULL ) GetPonchipFirmwareVer(PonPortIdx, FirmwareVer);
	if( HostSwVer != NULL )  GetPonchipHostSwVer(PonPortIdx, HostSwVer);
	if(GetPonchipDBAVer(PonPortIdx, DBA_type1, DBA_ver1) == ROK )
	{
		if( DBA_type != NULL ) 
			VOS_StrCpy(DBA_type, DBA_type1);
		if( DBA_ver != NULL )
			VOS_StrCpy(DBA_ver, DBA_ver1);
	}
    
	return( ROK );	
}
#ifdef __PON_CNI_WORK_MODE__
	/*2008-8-12  chenfj
 	这几个函数没有用, 先注释掉
 	*/
/*****************************************************
 *
 *    Function:   GetPonchipCNIWorkingMode ( short int PonChipIdx, unsigned char *CNIParam )
 *
 *    Param: short int PonChipIdx -- the specific pon chip 
 *                unsigned char *CNIParam -- output the pon chip CNI interface mode 
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
 int  GetPonchipCNIWorkingMode ( short int PonChipIdx, unsigned char *CNIParam )
{
	int PonCardIdx;
	int cardInserted;

	if( CNIParam == NULL ) return( RERROR );
	
	PonCardIdx = GetCardIdxByPonChip( PonChipIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );

	*CNIParam =  PonChipMgmtTable[PonChipIdx].WorkingMode.bus_mode ;
	return( ROK );
}

/*****************************************************
 *
 *    Function:    GetPonchipEPONWorkingMode ( short int PonChipIdx, short int EponParam )
 *
 *    Param: short int PonChipIdx -- the specific pon chip 
 *                short int *EponParam -- output the pon chip Epon interface mode 
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int  GetPonchipEPONWorkingMode ( short int PonChipIdx, short int *EponParam )
{
	int PonCardIdx;
	int cardInserted;

	if( EponParam == NULL ) return( RERROR );
	
	PonCardIdx = GetCardIdxByPonChip( PonChipIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );

	*EponParam = PonChipMgmtTable[PonChipIdx].WorkingMode.laserOnTime ;
	return( ROK );
}

/*****************************************************
 *
 *    Function:   GetPonchipWorkingMode ( short int PonChipIdx, PonChipWorkingMode_S *WorkingParam )
 *
 *    Param: short int PonChipIdx -- the specific pon chip 
 *                PonChipWorkingMode_S *WorkingParam -- output the pon chip Epon&CNI interface mode 
 *    Desc:   
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int  GetPonchipWorkingMode ( short int PonChipIdx, PonChipWorkingMode_S *WorkingParam )
{
	int PonCardIdx;
	int cardInserted;

	if( WorkingParam == NULL ) return( RERROR );
	
	PonCardIdx = GetCardIdxByPonChip( PonChipIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );

	VOS_MemCpy( WorkingParam, &(PonChipMgmtTable[PonChipIdx].WorkingMode), sizeof( PonChipWorkingMode_S));
	return( ROK );
}
#endif

/*****************************************************
 *
 *    Function:   GetPonchipWorkingStatus( short int PonChipIdx )
 *
 *    Param: short int PonChipIdx -- the specific pon chip 
 *                 
 *    Desc:   
 *
 *    Return:   pon chip status
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int   GetPonchipWorkingStatus( short int PonChipIdx )
{
	int PonCardIdx;
	int cardInserted;
	
	PonCardIdx = GetCardIdxByPonChip( PonChipIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );

	return ( PonChipMgmtTable[PonChipIdx].operStatus );
}

#if 0
/*****************************************************
 *
 *    Function:   GetPonchipWorkingStatusAll( )
 *
 *    Param:  none
 *                 
 *    Desc:   
 *
 *    Return:   all pon chip status
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
 int  GetPonchipWorkingStatusAll()
{
	int PonCardIdx;
	int cardInserted;
	int PonChipIdx;
	int allPonchipStatus=0;
	int status;
	
	for( PonChipIdx = 0; PonChipIdx < MAXPONCHIP; PonChipIdx ++ )
		{
			PonCardIdx = GetCardIdxByPonChip( PonChipIdx );
			if( PonCardIdx == RERROR ) continue;
	
			cardInserted =  GetOltCardslotInserted(  PonCardIdx );
			if( cardInserted !=  CARDINSERT) continue;

			status = GetPonchipWorkingStatus( PonChipIdx );
			if( (status == PONCHIP_UP ) || (status == PONCHIP_TESTING))
				allPonchipStatus = allPonchipStatus | ( 1 << PonChipIdx ) ;
		}
	
	return ( allPonchipStatus );
}

/*****************************************************
 *
 *    Function:   PonChipFWVersionCompare( short int PonChipIdx )
 *
 *    Param:  short int PonChipIdx -- the specific pon chip
 *                 
 *    Desc:   this function get the FW version from the pon chip , and compared with the
 *                newest version from flash;
 *
 *    Return:   whether PON chip Frimware version is consistent
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int  PonChipFWVersionCompare( short int PonChipIdx )
{
	PAS_device_versions_t  PonVersion;
	int PonCardIdx;
	int cardInserted;
	int PonChipStatus;
	short int PonChipType=PONCHIP_UNKNOWN;
	short int PonChipVer = 0;
	short int PonPortIdx;
	unsigned int PonFWVersionFromFlash;
	unsigned char ver[100];

	PonPortIdx = PonChipIdx;
	
	CHECK_PON_RANGE
	PonCardIdx = GetCardIdxByPonChip( PonChipIdx );
	if( PonCardIdx == RERROR ) return  ( RERROR );
	
	cardInserted =  GetOltCardslotInserted(  PonCardIdx );
	if( cardInserted !=  CARDINSERT) return ( RERROR );
	
	PonChipStatus = GetPonchipWorkingStatus( PonChipIdx );
	if( PonChipStatus == RERROR ) return ( RERROR );
	if( PonChipStatus > PONCHIP_TESTING ){
		if( PonChipStatus >= PONCHIP_ERR )
			PonChipStatus = 0;
		sys_console_printf(" the PON chip current status is %s  ( PonChipFWVersionCompare() )\r\n", PONchipStatus[PonChipStatus] );
		return ( RERROR );
		}

	PonChipType = V2R1_GetPonchipType( PonChipIdx);
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}
	
	if( PonChipType == PONCHIP_UNKNOWN  ){
		sys_console_printf(" the PON chip type error (PonChipFWVersionCompare() )\r\n" );
		return ( RERROR );
		}

	PonPortIdx = GetPonPortIdx(PonChipIdx);

	if(PonChipType == PONCHIP_PAS )
		{
		/*
		PonChipVer = V2R1_GetPonChipVersion(PonChipIdx);*/
		if((PonChipVer != PONCHIP_PAS5001) && (PonChipVer != PONCHIP_PAS5201)) return( RERROR );
		
		if(PonChipVer == PONCHIP_PAS5001)
		if( PAS_get_device_versions_v4 ( PonPortIdx, PON_OLT_ID, &PonVersion ) != PAS_EXIT_OK ){
			sys_console_printf("%s/port%d get version Info err ( PonChipFWVersionCompare() )\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
			return( RERROR );
			}
		
		if(PonChipVer == PONCHIP_PAS5201)
		if(PAS_get_olt_versions(PonPortIdx, &PonVersion ) != PAS_EXIT_OK ){
			sys_console_printf("%s/port%d get version Info err ( PonChipFWVersionCompare() )\r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx) );
			return( RERROR );
			}
		}
	
	else{ /* other pon chip handler */
		return( RERROR );
		}

	PonFWVersionFromFlash = GetPonFwVersionFromFlash ( PonChipVer, ver );

	if( PonFWVersionFromFlash != ROK )  return( RERROR );
	
	if( VOS_StrCmp( ver/*(char*)PonFWVersionFromFlash*/, ( char *)&(PonVersion.firmware_major ) ) != 0) {
		sendPonSoftUpdateMsg( PonPortIdx);
		/*AlarmDispatch ( AlarmSource( OLT Pon Chip ), 
				unsigned int Len, 
				unsigned int TrapId( VERSION_CONSISTENT_NOT), 
				unsigned int AlarmClass, 
				char *AlarmDec);
				*/
		return ( VERSION_CONSISTENT_NOT );
		}

	return ( VERSION_CONSISTENT );
}
#endif

/*****************************************************
 *
 *    Function:   GetPonFwVersionFromFlash ( short int PonChipType )
 *
 *    Param: short int PonChipType -- the PON chip type
 *                 
 *    Desc:  the different pon chip using different firmware image
 *
 *    Return:   the version of the specific pon chip firmware 
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
unsigned int GetPonFwVersionFromFlash ( short int PonChipType, unsigned char ver[100] )
{
	if( ver == NULL )  return( RERROR );

	return(  GetPonChipFWImageInfo( PonChipType, NULL, NULL, ver ) );
	/*
	if(PonChipType == PONCHIP_PAS5001 )
		{
		ret = GetPonChipFWInfo( PONCHIP_TYPE_STRING_PAS5001, &location, &length, ver );
		return ( ret ); 
		}
	else if( PonChipType == PONCHIP_PAS5201 )
		{
		ret = GetPonChipFWInfo( PONCHIP_TYPE_STRING_PAS5201, &location, &length, ver );
		return ( ret ); 
		}

	else if( PonChipType == PONCHIP_GW ){ return ( FLASH_BASE + GW_FIRMWARE_IMAGE_OFFSET );}
	else if( PonChipType == PONCHIP_TK ) { return ( FLASH_BASE + TK_FIRMWARE_IMAGE_OFFSET ); }
	else return ( RERROR );
	*/
}

/*****************************************************
 *
 *    Function:   sendPonSoftUpdateMsg( short int PonPortIdx )
 *
 *    Param:  short int PonPortIdx -- the specific Pon port
 *                 
 *    Desc:  send a message to PON software Update Module
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
int sendPonSoftUpdateMsg( short int PonPortIdx )
{
	/*PonUpdateInfo_S *PonSoftUpdateData;*/
	LONG ret;
	/*GeneralMsgBody_S OnuRegisterMsg; */
	unsigned long aulMsg[4] = { MODULE_PONUPDATE, FC_PONUPDATE, 0,0 };
	
	CHECK_PON_RANGE
	/*PonSoftUpdateData = (PonUpdateInfo_S *)g_malloc( sizeof(PonUpdateInfo_S));*/
	/*
	PonSoftUpdateData = (PonUpdateInfo_S *) VOS_Malloc( sizeof(PonUpdateInfo_S), MODULE_OLT );
	if( PonSoftUpdateData  == NULL ){
		sys_console_printf("Error, Malloc buffer not satified ( sendPonSoftUpdateMsg())\r\n" );
		return( RERROR );
		}

	PonSoftUpdateData->PonPortIdx = PonPortIdx;
	PonSoftUpdateData->PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
		
	aulMsg[2] = sizeof(PonUpdateInfo_S);
	aulMsg[3] = ( int )PonSoftUpdateData;
	*/
	aulMsg[2] = PonPortIdx;
	ret = VOS_QueSend(  g_PonUpdate_Queue_Id, aulMsg, NO_WAIT, MSG_PRI_NORMAL);
	
	if( ret !=  VOS_OK ){

		VOS_ASSERT( 0 );
		/*VOS_Free( (void *)PonSoftUpdateData );*/
		return( RERROR );
		}	
	return ( ROK );
}

/*****************************************************
 *
 *    Function:   PonChipMgmtInfoInit()
 *
 *    Param:  none
 *                 
 *    Desc:  Initialize the Pon chip Information table
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
/*int ClearPonChipMgmtInfo( short int PonChipIdx )
{
	if( (PonChipIdx < 0 ) || ( PonChipIdx > MAXPONCHIP )){
		return( RERROR );
		}
	PonChipMgmtTable[PonChipIdx].Type = PONCHIP_UNKNOWN;
	PonChipMgmtTable[PonChipIdx].version = PONCHIP_UNKNOWN;
	VOS_MemSet( PonChipMgmtTable[PonChipIdx].DataPathMAC, 0xff, BYTES_IN_MAC_ADDRESS );
	VOS_MemSet( PonChipMgmtTable[PonChipIdx].MgmtPathMAC, 0xff, BYTES_IN_MAC_ADDRESS );
	PonChipMgmtTable[PonChipIdx].adminStatus = PONCHIP_UP;
	PonChipMgmtTable[PonChipIdx].operStatus = PONCHIP_NOTPRESENT;
	VOS_MemSet( PonChipMgmtTable[PonChipIdx].FirmwareVer, 0x0, sizeof( PonChipMgmtTable[PonChipIdx].FirmwareVer) );
	VOS_MemSet(&( PonChipMgmtTable[PonChipIdx].WorkingMode), 0x0, sizeof( PonChipMgmtTable[PonChipIdx].WorkingMode) );
	PonChipMgmtTable[PonChipIdx].WorkingMode.MacType = PON_UNKNOWN;
	PonChipMgmtTable[PonChipIdx].WorkingMode.AGCTime = 0;
	PonChipMgmtTable[PonChipIdx].WorkingMode.CDRTime = 0;
	PonChipMgmtTable[PonChipIdx].WorkingMode.laserOffTime = 0;
	PonChipMgmtTable[PonChipIdx].WorkingMode.laserOnTime = 0;
	PonChipMgmtTable[PonChipIdx].Err_counter = 0;
	return( ROK );
}*/ /* removed by xieshl 20100207 */

int  ClearPonChipMgmtInfoAll()
{
	int whichPon;

	for( whichPon = 0; whichPon < MAXPON/*OLTMgmt.MaxPonPort*/; whichPon ++ )
	{
		PonChipMgmtTable[whichPon].Type = PONCHIP_UNKNOWN;
		PonChipMgmtTable[whichPon].TypeName[0] = 0;
		PonChipMgmtTable[whichPon].version = PONCHIP_UNKNOWN;
		VOS_MemSet( PonChipMgmtTable[whichPon].DataPathMAC, 0xff, BYTES_IN_MAC_ADDRESS );
		VOS_MemSet( PonChipMgmtTable[whichPon].MgmtPathMAC, 0xff, BYTES_IN_MAC_ADDRESS );
		PonChipMgmtTable[whichPon].adminStatus = PONCHIP_UP;
		PonChipMgmtTable[whichPon].operStatus = PONCHIP_NOTPRESENT;
		VOS_MemSet( PonChipMgmtTable[whichPon].FirmwareVer, 0x0, sizeof( PonChipMgmtTable[whichPon].FirmwareVer) );
		VOS_MemSet(&( PonChipMgmtTable[whichPon].WorkingMode), 0x0, sizeof( PonChipMgmtTable[whichPon].WorkingMode) );
		PonChipMgmtTable[whichPon].WorkingMode.MacType = PON_UNKNOWN;
		PonChipMgmtTable[whichPon].WorkingMode.AGCTime = 0;
		PonChipMgmtTable[whichPon].WorkingMode.CDRTime = 0;
		PonChipMgmtTable[whichPon].WorkingMode.laserOffTime = 0;
		PonChipMgmtTable[whichPon].WorkingMode.laserOnTime = 0;
		PonChipMgmtTable[whichPon].Err_counter = 0;
	}

    return ( ROK );
}

int InitPonChipMgmtInfoAll()
{
    ULONG ulSize;

    VOS_ASSERT(NULL == PonChipMgmtTable);
    ulSize = sizeof(PONChipInfo_S) * MAXPON;
    if ( NULL == (PonChipMgmtTable = (PONChipInfo_S*)VOS_Malloc(ulSize, MODULE_OLT)) )
    {
#ifdef __T_PON_MEM__        
        sys_console_printf("PonChipTotalNum:%d, sizeof(PonChipUnit)=%d, sizeof(PonChipTbl)=%d, g_malloc Fail.\r\n", MAXPON, sizeof(PONChipInfo_S), ulSize);
        VOS_TaskDelay(VOS_TICK_SECOND);
#endif
        VOS_ASSERT(0);
        return RERROR;
    }
#ifdef __T_PON_MEM__        
    else
    {
        sys_console_printf("PonChipTotalNum:%d, sizeof(PonChipUnit)=%d, sizeof(PonChipTbl)=%d, g_malloc OK.\r\n", MAXPON, sizeof(PONChipInfo_S), ulSize);
    }
#endif

    return ClearPonChipMgmtInfoAll();
}

int  SetPonChipDataPathMac( short int PonPortIdx )
{
	int cardIdx;
	int portIdx;

	CHECK_PON_RANGE

	cardIdx = GetCardIdxByPonChip( PonPortIdx );
	portIdx = GetPonPortByPonChip( PonPortIdx );
	if( (cardIdx == RERROR) || (portIdx == RERROR) ) return ( RERROR );

	VOS_MemCpy( PonChipMgmtTable[PonPortIdx].DataPathMAC, PAS_Data_Mac, BYTES_IN_MAC_ADDRESS );
    switch (GetOltType())
    {
        case V2R1_OLT_GFA6700:
    		PonChipMgmtTable[PonPortIdx].DataPathMAC[5] = PonChipSlotAddr[cardIdx - 1] + portIdx - 1;

            break;
        case V2R1_OLT_GFA6100:
    		PonChipMgmtTable[PonPortIdx].DataPathMAC[5] = (PonPortIdx << 4);

            break;
        default:
            PonChipMgmtTable[PonPortIdx].DataPathMAC[3] = 0x70;  /* 确保DataMac与MgtMac不同 */
            PonChipMgmtTable[PonPortIdx].DataPathMAC[4] = (unsigned char)cardIdx;
            PonChipMgmtTable[PonPortIdx].DataPathMAC[5] = (unsigned char)portIdx;

            break;
    }

	return( ROK );
}


/***************************************************
  *               bit 7	-	-	-	-	-	-	-	- bit 0
  *				const			slot_id			pon_id	    OLT_ID
  *				1	1	1		x	x	x		y	y 
  *OLT_id19 :	1	1	1		0	1	1		1	1 		0xef
  *OLT_id18 : 	1	1	1		0	1	1		1	0		0xee
  *OLT_id17 :	1	1	1		0	1	1		0	1		0xed
  *OLT_id16 :	1	1	1		0	1	1		0	0		0xec ( slot 4 )
  *OLT_id15 :	1	1	1		1	0	0		1	1		0xf3
  *OLT_id14 : 	1	1	1		1	0	0		1	0		0xf2	
  *OLT_id13 : 	1	1	1		1	0	0		0	1		0xf1
  *OLT_id12 :	1	1	1		1	0	0		0	0		0xf0  ( slot 5 )
  *  ...				...				...			 ...			...     ( slot 7 )
  *OLT_id3:  	1	1	1		1	1	1		1	1		0xff
  *OLT_id2:  	1	1	1		1	1	1		1	0		0xfe
  *OLT_id1:  	1	1	1		1	1	1		0	1		0xfd
  *OLT_id0:  	1	1	1		1	1	1		0	0		0xfc   ( slot 8 )
  ***************************************************
   PON 槽位         MAC-address            PON-ID
      2/1                0x000CD50001 - 00         0
      2/2                0x000CD50001 - 10         1
      3/1                0x000CD50001 - 20         2
      3/2                0x000CD50001 - 30         3
******************************************************/
int SetPonchipMgmtPathMac( short int PonPortIdx )
{
	int cardIdx;
	int portIdx;
	
	CHECK_PON_RANGE

	cardIdx = GetCardIdxByPonChip( PonPortIdx );
	portIdx = GetPonPortByPonChip( PonPortIdx );
	if( (cardIdx == RERROR) || (portIdx == RERROR) ) return ( RERROR );

	VOS_MemCpy( PonChipMgmtTable[PonPortIdx].MgmtPathMAC, PAS_Mgmt_Mac, BYTES_IN_MAC_ADDRESS );
    switch ( GetOltType() )
    {
        case V2R1_OLT_GFA6700:
    		PonChipMgmtTable[PonPortIdx].MgmtPathMAC[5] = PonChipSlotAddr[cardIdx - 1] + portIdx - 1;
            
            break;
        case V2R1_OLT_GFA6100:
    		PonChipMgmtTable[PonPortIdx].MgmtPathMAC[5] = (PonPortIdx << 4);
                
            break;
        default:
            PonChipMgmtTable[PonPortIdx].MgmtPathMAC[3] = 0x69;
            PonChipMgmtTable[PonPortIdx].MgmtPathMAC[4] = (unsigned char)cardIdx;
            PonChipMgmtTable[PonPortIdx].MgmtPathMAC[5] = (unsigned char)portIdx;
            
            break;
    }
    
	return( ROK );
}


/* PON芯片的管理MAC不可设置，则需回写 */
/* 【可以保证GW OAM正常，但对环路检测会有影响】 */
int SetPonchipMgmtPathMac2( short int PonPortIdx, unsigned char mac_addr[BYTES_IN_MAC_ADDRESS] )
{
	VOS_MemCpy( PonChipMgmtTable[PonPortIdx].MgmtPathMAC, mac_addr, BYTES_IN_MAC_ADDRESS );
    
	return( ROK );
}

int *GetPonChipMgmtPathMac(short int PonPortIdx )
{
	/*CHECK_PON_RANGE*/

	return(int *)( PonChipMgmtTable[PonPortIdx].MgmtPathMAC );
}

int InitPonChipMgmtInfoByFlash(short int PonPortIdx )
{
	short int PonChipType;

	CHECK_PON_RANGE
	/* readPonChipInfoFromFlash(PonPortIdx); */

#if 0		
	PonChipType = GetPonChipTypeByPonPort(  PonPortIdx );
	if( OLT_PONCHIP_ISPAS(PonChipType) ) 
	{
		/*PonChipVer = PonChipType;*/
		PonChipType = PONCHIP_PAS;
	}
	
	if( PonChipType == PONCHIP_PAS)
    {
		SetPonChipDataPathMac( PonPortIdx );
		SetPonchipMgmtPathMac( PonPortIdx );	
		return( ROK );
	}
	else
    {
		/*
		SetPonChipDataPathMac( PonPortIdx );
		SetPonchipMgmtPathMac( PonPortIdx );	
		*/
	}
#else
	SetPonChipDataPathMac( PonPortIdx );
	SetPonchipMgmtPathMac( PonPortIdx );	
	return( ROK );
#endif

	/* copy firmware version to PonChipMgmtTable[PonPortIdx].FirmwareVer */
	/* copy CNI and PON interface working mode to PonChipMgmtTable[PonPortIdx].WorkingMode */
	return ( RERROR );
}

/*****************************************************
 *
 *    Function:   GetPonDeviceVersion( short int PonPortIdx)
 *
 *    Param:   short int PonPortIdx -- the specific pon port
 *                 
 *    Desc: Get the Pon Chip Info
 *
 *    Return:    ROK
 *
 *    Notes: 
 *
 *    modified:
 *
 ******************************************************/
int GetPonDeviceVersion( short int PonPortIdx)
 {
	PON_device_versions_t device_versions;
	int ret = PAS_EXIT_ERROR;
	unsigned char Temp[32], *Ptr;
	unsigned char len=0;
#if 0
	short int PonChipType;
	short int PonChipVer = 0;
#endif	

	CHECK_PON_RANGE

#if 1
	ret = OLT_GetVersion(PonPortIdx, &device_versions);
#else
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 )
        || ( PonChipType == PONCHIP_PAS5201 )
        || ( PonChipType == PONCHIP_PAS5204 )
        ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}

	if((PonChipVer != PONCHIP_PAS5001) && (PonChipVer != PONCHIP_PAS5201)) return( RERROR );
	if( PonChipVer == PONCHIP_PAS5001 )
	{
		ret = PAS_get_device_versions_v4( PonPortIdx,  PON_OLT_ID, &device_versions);
	}
	else if( PonChipVer == PONCHIP_PAS5201 )
	{
		ret = PAS_get_olt_versions( PonPortIdx, &device_versions);
	}
#endif	
	if( OLT_CALL_ISOK(ret) )
    {
		VOS_MemSet( Temp, 0, sizeof( Temp ) );
		Ptr = &Temp[0];
		sprintf(&Temp[0],"V%u.%u.%u.%u",
					device_versions.firmware_major,
					device_versions.firmware_minor,
					device_versions.build_firmware,
					device_versions.maintenance_firmware);
		len = VOS_StrLen(Ptr);
		VOS_MemCpy( PonChipMgmtTable[PonPortIdx].FirmwareVer, Temp, len );
		PonChipMgmtTable[PonPortIdx].FirmwareVer[len] = '\0';

		VOS_MemSet( Temp, 0, sizeof( Temp ) );
		Ptr = &Temp[0];
		len = 0;
		VOS_MemCpy( &Temp[len], "V", 1 );
		len = 1;
		sprintf(&Temp[len], "%d", device_versions.host_major );
		len = VOS_StrLen(Ptr);			
		VOS_MemCpy(&Temp[len], "R", 1);
		len = VOS_StrLen(Ptr);
		sprintf(&Temp[len], "%02d", device_versions.host_minor );
		len = VOS_StrLen(Ptr);
		VOS_MemCpy( &Temp[len],  "B", 1);
		len = VOS_StrLen(Ptr);
		sprintf(&Temp[len], "%02d0",device_versions.host_compilation );
		len = VOS_StrLen(Ptr);
		VOS_MemCpy( PonChipMgmtTable[PonPortIdx].HostSwVer, Temp, len );
		PonChipMgmtTable[PonPortIdx].HostSwVer[len] = '\0';

		VOS_MemSet( Temp, 0, sizeof( Temp ) );
		VOS_MemCpy( Temp, "V", 1 );
		sprintf(&Temp[1], "%04x.%u", (unsigned short)device_versions.hardware_major, (unsigned short)device_versions.hardware_minor );
		VOS_MemCpy(&Temp[7], "B0", 2);
		VOS_MemCpy( PonChipMgmtTable[PonPortIdx].HardVer, Temp, 9 );
		PonChipMgmtTable[PonPortIdx].HardVer[9] = '\0';
					
		PonChipMgmtTable[PonPortIdx].WorkingMode.MacType = device_versions.system_mac;
		PonPortTable[PonPortIdx].MaxOnu = device_versions.ports_supported ;
		PonPortTable[PonPortIdx].MaxLLID = PonPortTable[PonPortIdx].MaxOnu + 1;
	
		return( ROK );
	}

	return( RERROR );
 }

int GetPonChipFWVersion( char *version, int *len)
{
	short int PonPortIdx;
	
	if((version == NULL ) ||(len == NULL )) return( RERROR );
	for(PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx++)
		{
		if( PonPortIsWorking( PonPortIdx) == TRUE)
			{
			*len = VOS_StrLen(PonChipMgmtTable[PonPortIdx].FirmwareVer);
			VOS_MemCpy( version, PonChipMgmtTable[PonPortIdx].FirmwareVer,*len);
			return( ROK );
			}
		}
	return( RERROR );
}

int  GetPonDeviceCapabilities( short int PonPortIdx )
{	
	short int ret;
#if 0
	short int PonChipType;
	short int PonChipVer = 0;
	short int PonChipIdx;
#else
    OLT_optical_capability_t stOpticCap;
#endif

	CHECK_PON_RANGE

#if 1
    ret = OLT_GetOpticalCapability(PonPortIdx, &stOpticCap);
	if( OLT_CALL_ISOK(ret) )
    {
		/* print these info */
		if( EVENT_DEBUG == V2R1_ENABLE)
        {
			sys_console_printf("\r\n%s/port%d capabilities \r\n", CardSlot_s[GetCardIdxByPonChip(PonPortIdx)], GetPonPortByPonChip(PonPortIdx)   );
			sys_console_printf("   Laser on/off timer %d/%d \r\n", stOpticCap.laser_on_time, stOpticCap.laser_off_time );
			sys_console_printf("   AGC / CDR lock time %d / %d \r\n", stOpticCap.agc_lock_time, stOpticCap.cdr_lock_time );
        }

        return( ROK );
	}
#else
	PonChipType = GetPonChipTypeByPonPort( PonPortIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
	{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
	}
	PonChipIdx = GetPonChipIdx( PonPortIdx );

	if( PonChipType == PONCHIP_PAS )
	{
		/*modified for PAS-SOFT V5.1
		PonChipVer = V2R1_GetPonChipVersion(PonPortIdx);
		*/
		if((PonChipVer != PONCHIP_PAS5001) &&(PonChipVer != PONCHIP_PAS5201)) return( RERROR );
		if( PonChipVer == PONCHIP_PAS5001 )
		{
			PAS_physical_capabilities_t  device_capabilities;
			ret = PAS_get_device_capabilities_v4( PonPortIdx,  PON_OLT_ID, &device_capabilities);
	
			if( ret == PAS_EXIT_OK ){
				/* print these info */
				/*if( EVENT_DEBUG == V2R1_ENABLE){
				sys_console_printf("\r\n%s/port%d capabilities \r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx)   );
				sys_console_printf("   Laser on/off timer %d/%d \r\n", device_capabilities.laser_on_time, device_capabilities.laser_off_time );
				sys_console_printf("   Max grant number %d \r\n", device_capabilities.onu_grant_fifo_depth );
				sys_console_printf("   AGC / CDR lock time %d / %d \r\n", device_capabilities.agc_lock_time, device_capabilities.cdr_lock_time );
				sys_console_printf("   pon tx signal %d \r\n", device_capabilities.pon_tx_signal );
				}*/

				/* save these info to PonChipMgmtTable[] 

				PonPortTable[PonPortIdx].MaxOnu = device_versions.ports_supported ;*/
				PonChipMgmtTable[PonChipIdx].WorkingMode.laserOnTime = device_capabilities.laser_on_time;
				PonChipMgmtTable[PonChipIdx].WorkingMode.laserOffTime = device_capabilities.laser_off_time;
				PonChipMgmtTable[PonChipIdx].WorkingMode.AGCTime = device_capabilities.agc_lock_time;
				PonChipMgmtTable[PonChipIdx].WorkingMode.CDRTime = device_capabilities.cdr_lock_time;
				return( ROK );
			}
		}
		else if( PonChipVer == PONCHIP_PAS5201 )
		{
			PON_olt_optics_configuration_t   optics_configuration;
			bool   pon_tx_signal;
				
			/* Page: 31, 6.2.1 get device capability */
			ret = PAS_get_olt_optics_parameters( PonPortIdx, &optics_configuration, &pon_tx_signal);
			if( ret == PAS_EXIT_OK )
			{
				/* print these info */
				/*if( EVENT_DEBUG == V2R1_ENABLE){
				sys_console_printf("\r\n%s/port%d capabilities \r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx)  );
				sys_console_printf("   Laser on/off timer 0/0 \r\n" );
				sys_console_printf("   AGC / CDR lock time %d / %d \r\n", optics_configuration.agc_lock_time, optics_configuration.cdr_lock_time );
				sys_console_printf("   pon tx signal %d \r\n", pon_tx_signal );
				}*/

				/* save these info to PonChipMgmtTable[] 

				PonPortTable[PonPortIdx].MaxOnu = device_versions.ports_supported ;*/
				PonChipMgmtTable[PonChipIdx].WorkingMode.laserOnTime = 0;
				PonChipMgmtTable[PonChipIdx].WorkingMode.laserOffTime = 0;
				PonChipMgmtTable[PonChipIdx].WorkingMode.AGCTime = optics_configuration.agc_lock_time;
				PonChipMgmtTable[PonChipIdx].WorkingMode.CDRTime = optics_configuration.cdr_lock_time;
				return( ROK );
			}
		}			
	}
#endif

	return( RERROR );
}


void GetPonChipInfo( short int CardSlot , short int port)
{
	short int PonPortIdx;
	int iPonChipType = ponChipType_null;

	if(SlotCardIsPonBoard(CardSlot) != ROK )
	{
		sys_console_printf(" slot %d is not pon card \r\n", CardSlot);
		return;
	}
	
	if(( port <= 0 ) || ( port > CARD_MAX_PON_PORTNUM ))
    {
		sys_console_printf("error: pon%d/%d out of range\r\n", CardSlot, port );
		return;
	}

	PonPortIdx = GetPonPortIdxBySlot(CardSlot, port);
	if( PonPortIdx == RERROR ) return;
	
	/*read pon chip info */
	if( getPonChipInserted((unsigned char)CardSlot,(unsigned char)port ) == PONCHIP_EXIST )
	{
		iPonChipType = getPonChipType((unsigned char)CardSlot,(unsigned char)port);
		PonChipMgmtTable[PonPortIdx].Type = iPonChipType;
		
		/* modified for PAS-SOFT V5.1
		if( CardSlot == 4 )
			{
			PonChipMgmtTable[PonPortIdx].Type = PONCHIP_PAS5001;
			PonPortTable[PonPortIdx].CTC_Supported = FALSE;
			}
		else if (CardSlot == 5) 
			{
			PonChipMgmtTable[PonPortIdx].Type = PONCHIP_PAS5201;
			PonPortTable[PonPortIdx].CTC_Supported = TRUE;
			}
		*/
		PonChipMgmtTable[PonPortIdx].version = PonChipMgmtTable[PonPortIdx].Type;
		PonPortTable[PonPortIdx].CTC_Supported = getCtcStackSupported(); /* getPonCtcStackSupported( CardSlot+1, portIdx+1);*/
		
		switch (iPonChipType)
		{
			case ponChipType_GPON:
				SetPonPortType(PonPortIdx, GPONTYPE2G1GDOLT);
				break;
			case ponChipType_BCM55538: /*10GEPON*/
				UpdateXGEPonPortInfo(PonPortIdx);
				break;
			default: /*1G EPON*/
			/* include:
			ponChipType_PAS5001
			ponChipType_PAS5201
			ponChipType_PAS5204
			ponChipType_PAS8411
			ponChipType_BCM55524
			ponChipType_TK3723*/
				SetPonPortType(PonPortIdx, EPONMAUTYPE1000BASEPX20UOLT);
				break;
		}
		/*PonPortTable[PonPortIdx].type = EPONMAUTYPE1000BASEPX20UOLT;*/
	}
	/*
	PonPortTable[PonPortIdx].type = EPONMAUTYPE1000BASEPX20UOLT;
	if(SYS_MODULE_TYPE(CardSlot+1) ==  MODULE_E_GFA_EPON )
		PonChipMgmtTable[PonPortIdx].Type = PONCHIP_PAS;
	*/
	/* set the user pon table */
	InitPonChipMgmtInfoByFlash( PonPortIdx );

	/*PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_ONLINE;*/
	
}

/*****************************************************
 *
 *    Function: ShowPonChipStatus( short int PonChipIdx )
 *
 *    Param:  short int PonChipIdx -- the pon chip Index 
 *                 
 *    Desc:  print the specific pon chip current status
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
#if 0	/* removed by xieshl 20091216, 调试函数 */
void ShowPonChipStatus( short int PonChipIdx )
{
	if((PonChipIdx < 0 ) ||( PonChipIdx > MAXPONCHIP )){
		sys_console_printf("\r\nerror: %s/port%d out of range( PonChipStatus())\r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx)  );
		return;
		}

	if( (PonChipMgmtTable[PonChipIdx].operStatus < PONCHIP_UP ) ||(PonChipMgmtTable[PonChipIdx].operStatus > PONCHIP_NOTPRESENT)){
		sys_console_printf("\r\nerror: Pon Chip status %d out of range( PonChipStatus())\r\n", PonChipMgmtTable[PonChipIdx].operStatus );
		return;
		}

	sys_console_printf("\r\n%s/port%d current status is %d %s \r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx), PonChipMgmtTable[PonChipIdx].operStatus, PONchipStatus[PonChipMgmtTable[PonChipIdx].operStatus] );
}

void ShowPonChipStatusAll()
{
	short int PonChipIdx;

	sys_console_printf("\r\n display the pon chip mgmt status: \r\n");

	sys_console_printf(" Pon chip \t\t current status \r\n\r\n");

	for(PonChipIdx=0; PonChipIdx<MAXPONCHIP; PonChipIdx++){
		sys_console_printf("%s/port%d  \t%s \r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx),PONchipStatus[PonChipMgmtTable[PonChipIdx].operStatus]);
		/*sys_console_printf("   %d      %d   \t%s  \r\n", PonChipIdx,  PonChipMgmtTable[PonChipIdx].operStatus, PONchipStatus[PonChipMgmtTable[PonChipIdx].operStatus] );*/
		}
	sys_console_printf("\r\n");
}

int  SetPonChipStatus( short int PonPortIdx, int status )
{
	CHECK_PON_RANGE

	if((status >= PONCHIP_UP)&&(status<= PONCHIP_ERR))
		PonChipMgmtTable[PonPortIdx].operStatus  = status;
	return( ROK );
}

/*****************************************************
 *
 *    Function: ShowPonChipType( short int PonChipIdx )
 *
 *    Param:  short int PonChipIdx -- the pon chip Index 
 *                 
 *    Desc:  print the specific pon chip interface type
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
void ShowPonChipType( short int PonChipIdx )
{
	short int PonChipType;
	
	if( (PonChipIdx < 0 ) ||( PonChipIdx > MAXPONCHIP )){
		sys_console_printf("error:%s/port%d out of range\r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx)  );
		return;
		}
	PonChipType = V2R1_GetPonchipType( PonChipIdx );

	if( PonChipType != RERROR ){
		sys_console_printf("\r\n%s/port%d Info \r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx)  );
		sys_console_printf(" type : %s \r\n", PonChipType_s[PonChipType] );
		sys_console_printf(" interface para: %s \r\n", PonInterfaceType[PonChipType] );
		}
}

void ShowPonChipTypeAll()
{
	short int i;
	short int PonChipType;

	sys_console_printf("\r\n display the pon chip type: \r\n");

	sys_console_printf(" Pon chip \t   type  \t     interface para\r\n");

	for( i=0; i<MAXPONCHIP; i++){
		sys_console_printf("  %d ",CardSlot_s[GetCardIdxByPonChip(i)], GetPonPortByPonChip(i) );
		PonChipType = V2R1_GetPonchipType( i );
		if( PonChipType == RERROR ){
			sys_console_printf("\t    %s \t     unknown \r\n", PonChipType_s[0] );
			
			}
		else {
			sys_console_printf("\t   %s \t  %s \r\n", PonChipType_s[PonChipType], PonInterfaceType[PonChipType]);
			}
		}
}


void ShowPonChipInfo( short int PonChipIdx )
{

	short int PonChipType;
	short int PonChipVer = RERROR;
	
	if( (PonChipIdx < 0 ) ||( PonChipIdx > MAXPONCHIP )){
		sys_console_printf("error: Pon chip Index %d out of range\r\n", PonChipIdx );
		return;
		}
	PonChipType = V2R1_GetPonchipType( PonChipIdx );
	if(( PonChipType == PONCHIP_PAS5001 ) || ( PonChipType == PONCHIP_PAS5201 ) ) 
		{
		PonChipVer = PonChipType;
		PonChipType = PONCHIP_PAS;
		}

	if( PonChipType == RERROR ){
		sys_console_printf("  %s/port%d type is unknown, No Info about it \r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx) );
		}
	else{

		int counter =0;
		char mac_string[20];
		
		sys_console_printf("\r\n%s/port%d Info: \r\n", CardSlot_s[GetCardIdxByPonChip(PonChipIdx)], GetPonPortByPonChip(PonChipIdx) );
		sys_console_printf("   Type:%s \r\n", PonChipType_s[PonChipType] );

		/* modified for PAS-SOFT V5.1*/ 
		if( PonChipType == PONCHIP_PAS )
			{
			/*PonChipVer = V2R1_GetPonChipVersion(PonChipIdx );*/
			if( PonChipVer != RERROR )
				sys_console_printf("   subType:%s\r\n",pon_chip_type2name(PonChipVer)/* PasPonChipVer_s[PonChipVer]*/);
			}
		
		for (counter=0; counter < sizeof(mac_address_t); counter ++)
			sprintf (&mac_string[5*counter],"0x%02x ", PonChipMgmtTable[PonChipIdx].MgmtPathMAC[counter]);
		sys_console_printf("   the Mgmt Path mac is: %s \r\n", mac_string );

		for (counter=0; counter < sizeof(mac_address_t); counter ++)
			sprintf (&mac_string[5*counter],"0x%02x ", PonChipMgmtTable[PonChipIdx].DataPathMAC[counter]);
		sys_console_printf("   the Data Path mac is: %s \r\n", mac_string );

		sys_console_printf("   the Mgmt status is: %d\r\n", PonChipMgmtTable[PonChipIdx].adminStatus);
		sys_console_printf("   the Oper status is: %s \r\n",PONchipStatus[PonChipMgmtTable[PonChipIdx].operStatus] );
		sys_console_printf("   the Firmware version is: %s \r\n", PonChipMgmtTable[PonChipIdx].FirmwareVer);
		sys_console_printf("   the CNI port MAC type: %s\r\n", PON_mac_s[PonChipMgmtTable[PonChipIdx].WorkingMode.MacType]);
		sys_console_printf("   the AGC time is %d \r\n", PonChipMgmtTable[PonChipIdx].WorkingMode.AGCTime );
		sys_console_printf("   the CDR time is %d \r\n", PonChipMgmtTable[PonChipIdx].WorkingMode.CDRTime );
		}	

}
#endif


/*****************************************************
 *
 *    Function: int  GetPonChipFW( unsigned char *PonChipTypeString ,int *location, int  *length, unsigned char *ver )
 *
 *    Param:  short int PonChipType -- PON CHIP type 
 *                 
 *    Desc:  search the firmware file information for the specific ponchip 
 *
 *    Return:   ROK
 *
 *    Notes: None
 *
 *    modified:
 *
 ******************************************************/
extern STATUS xflash_file_read( int fileID, unsigned char * readbuf, int * size );
static SEM_ID ponLoadSem = 0;

int  GetPonChipFWImageInfo( short int PonChipType, int *location, int  *length, unsigned char version[100] )
{
	/* modified by xieshl 20160106, 运行过程中升级版本，存在读取错误问题，改为直接到flash中读 */
    /*static UCHAR buffer_version[100]="";
    static unsigned short buffer_chiptype = 0;
    static unsigned short buffer_length = 0;*/
    UCHAR file[2048+4]="";
    int size = 2048, i=0;
    driver_file_desc_t *pdesc = NULL;
    unsigned char *PonChipTypeString = NULL;
    char *pFileBuf = NULL, *pReadBuf = NULL;
    unsigned int compress_flag = 0;
    unsigned int src_len, dst_len;
    int iFileLen, iFilePos, iReadLen;
    int iRlt;
	unsigned char props[80];
	char *tmpBuf = NULL;
    	
    if( (location == NULL) && (length == NULL) && (version == NULL) )
    	return( RERROR );
    /*
    if(( VOS_StrCmp(  PonChipTypeString , PONCHIP_TYPE_STRING_PAS5001 ) != 0 ) &&( VOS_StrCmp(  PonChipTypeString , PONCHIP_TYPE_STRING_PAS5201 ) != 0 ))
    	return( RERROR );
    */
    if (0 == ponLoadSem) 
        ponLoadSem = VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_FULL);
    
    VOS_SemTake(ponLoadSem, WAIT_FOREVER);

    /* B--modified by liwei056@2012-12-21 for FileVersionCmpUpgrade */
    /* 仅取文件尺寸或版本 */
    /*if ( buffer_chiptype == PonChipType )
    {
        if ( length != NULL )
        {
            *length = buffer_length;
        }
        
        if ( version != NULL )
        {
            VOS_MemCpy(version, buffer_version, 100);
        }

        if ( location == NULL )
        {
            VOS_SemGive(ponLoadSem);

            OLT_ADD_DEBUG(OLT_ADD_TITLE"pontype(%d) successed to load fw's version or length from mem.\r\n", PonChipType);
            
            return 0;
        }
    }*/

    /* B--modified by liwei056@2010-7-20 for Flash-VFS Support */
    if ( location != NULL )
    {
        if ( length == NULL ) return( RERROR );
    
        /* 要取文件数据 */
        pon_load_get(PonChipType, location, length, NULL, NULL);
        if ( (NULL != *location) && (length > 0) )
        {
            VOS_SemGive(ponLoadSem);

            OLT_ADD_DEBUG(OLT_ADD_TITLE"pontype(%d) successed to load fw from mem.\r\n", PonChipType);

            return 0;
        }
        else
        {
    		OLT_ADD_DEBUG(OLT_ADD_TITLE"pontype(%d) ready to load fw from flash.\r\n", PonChipType);
        }
    }
    
    switch (PonChipType)
    {
        case PONCHIP_PAS5001:
    		PonChipTypeString = PONCHIP_TYPE_STRING_PAS5001;
            break;
        case PONCHIP_PAS5201:
        case PONCHIP_PAS5204:
    		PonChipTypeString = PONCHIP_TYPE_STRING_PAS5201;
            break;
        case PONCHIP_PAS8411:
    		PonChipTypeString = PONCHIP_TYPE_STRING_PAS8411;
            break;
        case PONCHIP_BCM55524: 
    		PonChipTypeString = PONCHIP_TYPE_STRING_BCM55524;
            break;
        case PONCHIP_BCM55538: 
    		PonChipTypeString = PONCHIP_TYPE_STRING_BCM55538;
            break;
		#if defined(_GPON_BCM_SUPPORT_)
		case PONCHIP_GPON: 
    		PonChipTypeString = PONCHIP_TYPE_STRING_GPON;
            break;
		#endif
        default:
            VOS_SemGive(ponLoadSem);
            VOS_ASSERT(0);
        	return( RERROR );
    }
    		
	VOS_MemZero( file, size );
	xflash_file_read(PON_FIRMWARE_FLASH_FILE_ID, file, &size );
	pdesc = (driver_file_desc_t*)file;	

	for( i=0; i<(2048/128); i++ )
	{
		unsigned char *p = (char*)&pdesc->file_len;
		unsigned char *p1 = (char*)&pdesc->loc_offset;

		if( pdesc->dev_type[0] == 0 )
			continue;

		if( VOS_StrCmp( pdesc->dev_type, PonChipTypeString ) == 0 )
		{
            compress_flag = pdesc->compress_flag;
        
			src_len = MAKE_UNSIGNED_LONG_PARAM( MAKE_UNSIGNED_SHORT_PARAM( *(p+3), *(p+2) ),  MAKE_UNSIGNED_SHORT_PARAM(*(p+1), *p) );
			dst_len =  /* FLASH_BASE + FIRMWARE_IMAGE_OFFSET + */MAKE_UNSIGNED_LONG_PARAM( MAKE_UNSIGNED_SHORT_PARAM(*(p1+3),*(p1+2)), MAKE_UNSIGNED_SHORT_PARAM(*(p1+1), *p1));

            /*buffer_chiptype = PonChipType;
            buffer_length = src_len;
            VOS_MemCpy( buffer_version, pdesc->file_ver, sizeof(pdesc->file_ver) );*/
            VOS_MemCpy( version, pdesc->file_ver, sizeof(pdesc->file_ver) );

			break;
		}
		pdesc++;
	}
	
	if( i >= 16 ) 
	{     
        VOS_SemGive(ponLoadSem);
        return( RERROR);
	}
    
    /*if ( version != NULL )
    {
        VOS_MemCpy(version, buffer_version, 100);
    }

    if ( length != NULL )
    {
        *length = buffer_length;
    }*/
    
    if ( location != NULL )
    {
        iRlt = RERROR;
        
        iFileLen = src_len;
        iFilePos = dst_len;
        iReadLen = iFileLen + iFilePos;
        size = iReadLen;

        if ( NULL != (pReadBuf = g_malloc(iReadLen)) )
        {
            xflash_file_read(PON_FIRMWARE_FLASH_FILE_ID, pReadBuf, &size);
            if (size == iReadLen)
            {
                if ( 0 == compress_flag )
                {
                    /* 无压缩Firmware，直接截取 */
                    if ( NULL != (pFileBuf = g_malloc(iFileLen)) )
                    {
                        memcpy(pFileBuf, pReadBuf+iFilePos, iFileLen);
                    }
                }
                else
                {
                	/*Begin:解压通过压缩工具压缩的FIRMWARE byjinhl @2016.12.08*/
                    /* 需要解压Firmware，最大解压文件16MB */
                    src_len = iFileLen;
                    dst_len = 16777216;
                    if ( NULL != (pFileBuf = g_malloc(dst_len)) )
                    {
                    	src_len = src_len-13;
						
						memcpy(props, (char *)((unsigned long)pReadBuf + iFilePos), 13);
						iRlt = LzmaUncompress(pFileBuf,&dst_len,(unsigned char *)((unsigned long)pReadBuf + iFilePos+13),&src_len,props,13);
						if((iRlt ==0)||(iRlt ==6) )
                        {
                            iFileLen = dst_len;
                        
                            /* 解压文件小于16MB，超过1MB，则值得释放部分空间 */
                            if ( dst_len <= 16777216 - 1048576 )
                            {
                                if ( NULL != (tmpBuf = g_malloc(dst_len)) )
                                {
                                    memcpy(tmpBuf, pFileBuf, dst_len);
                                    g_free(pFileBuf);
                                    pFileBuf = tmpBuf;
                                }
                            }
                        }
                        else
                        {
                        	sys_console_printf("Can't uncompress firm in LZMA format.\r\n");
                            g_free(pFileBuf);
                            pFileBuf = NULL;

                            VOS_ASSERT(0);
                        }
						/*End:解压通过压缩工具压缩的FIRMWARE byjinhl @2016.12.08*/
                    }
                }

                if ( NULL != pFileBuf )
                {
                    VOS_TaskLock();
                    pon_load_set(PonChipType, pFileBuf, iFileLen, NULL, 0);
                    pon_load_get(PonChipType, location, length, NULL, NULL);
                    VOS_TaskUnlock();
                    
                    iRlt = ROK;
                }
            }

            g_free(pReadBuf);
        }    
        
        OLT_ADD_DEBUG(OLT_ADD_TITLE"pontype(%d) %s to load fw from flash.\r\n", PonChipType, (0 == iRlt) ? "successed" : "failed");
    }
    /* E--modified by liwei056@2010-7-20 for Flash-VFS Support */
    else
    {
        iRlt = ROK;

        OLT_ADD_DEBUG(OLT_ADD_TITLE"pontype(%d) successed to load fw's version or length from flash.\r\n", PonChipType);
    }
    /* E--modified by liwei056@2012-12-21 for FileVersionCmpUpgrade */
    
    VOS_SemGive(ponLoadSem);
    return( iRlt);
}

int  GetPonChipDBAFileInfo( short int PonChipType, int *location, int  *length, unsigned char version[100] )
{
	UCHAR file[2048]="";
	int size = 2048, i=0;
	driver_file_desc_t *pdesc = NULL;
	unsigned char *DBA_type;
	char *pFileBuf, *pReadBuf;
	int  iFileLen, iFilePos, iReadLen;
    int iRlt;
		
	if(/*( PonChipTypeString == NULL ) ||*/(location == NULL) || ( length == NULL ) /* || (version == NULL)*/)
		return( RERROR );

    if (0 == ponLoadSem) 
        ponLoadSem = VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_FULL);

    VOS_SemTake(ponLoadSem, WAIT_FOREVER);
    /* B--modified by liwei056@2010-7-20 for Flash-VFS Support */
    pon_load_get(PonChipType, NULL, NULL, location, length);
    if (NULL == *location)
    {
		OLT_ADD_DEBUG(OLT_ADD_TITLE"pontype(%d) failed to load dba from mem.\r\n", PonChipType);

    	if(OLT_PONCHIP_ISPAS5001(PonChipType) ) 
    		DBA_type = PONCHIP_DBA_TYPE_PAS5001;
    	else if(OLT_PONCHIP_ISPAS1G(PonChipType))
        {
    		if( V2R1_CTC_STACK )
    			DBA_type = PONCHIP_DBA_TYPE_PAS5201_CTC;
    		else
                DBA_type = PONCHIP_DBA_TYPE_PAS5201;
    	}
#if defined(_EPON_10G_PMC_SUPPORT_)            
		/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		else if( OLT_PONCHIP_ISPAS10G(PonChipType) )
        {
    		if( V2R1_CTC_STACK )
    			DBA_type = PONCHIP_DBA_TYPE_PAS8411;/*因PMC提供的DBA文件中只有类型"DBA_PAS8411"*/
    		else
                DBA_type = PONCHIP_DBA_TYPE_PAS8411;
    	}
		/*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
#endif
    	else /* other pon chip handler */
        {   
            VOS_SemGive(ponLoadSem);
    		return( RERROR );
        }
    	
    	xflash_file_read(PON_DBA_FLASH_FILE_ID, file, &size );
    	pdesc = (driver_file_desc_t*)file;

    	for( i=0; i<(2048/128); i++ )
    	{
			unsigned char *p = (char*)&pdesc->file_len;
			unsigned char *p1 = (char*)&pdesc->loc_offset;

			if( pdesc->dev_type[0] == 0 )
				continue;

			if( VOS_StrCmp( pdesc->dev_type, DBA_type ) == 0 )
			{
				*length = MAKE_UNSIGNED_LONG_PARAM( MAKE_UNSIGNED_SHORT_PARAM( *(p+3), *(p+2) ),  MAKE_UNSIGNED_SHORT_PARAM(*(p+1), *p) );
                if ( version != NULL )
                {
    				VOS_MemCpy( version, pdesc->file_ver, sizeof(pdesc->file_ver));
                }
				*location =  /*FLASH_BASE + DBA_IMAGE_OFFSET + */MAKE_UNSIGNED_LONG_PARAM( MAKE_UNSIGNED_SHORT_PARAM(*(p1+3),*(p1+2)), MAKE_UNSIGNED_SHORT_PARAM(*(p1+1), *p1));
				break;
			}
			pdesc++;
    	}
    	
    	if( i >= 16 ) 
        {   
            VOS_SemGive(ponLoadSem);
    		return( RERROR );
        }

        iFileLen = *length;
        iFilePos = *location;
        iReadLen = iFileLen + iFilePos;
        size = iReadLen;
        iRlt = RERROR;
        if ( NULL != (pReadBuf = g_malloc(iReadLen)) )
        {
            xflash_file_read(PON_DBA_FLASH_FILE_ID, pReadBuf, &size);
            if (size == iReadLen)
            {
                if ( NULL != (pFileBuf = g_malloc(iFileLen)) )
                {
                    memcpy(pFileBuf, pReadBuf+iFilePos, iFileLen);

                    VOS_TaskLock();
                    pon_load_set(PonChipType, NULL, 0, pFileBuf, iFileLen);
                    pon_load_get(PonChipType, NULL, NULL, location, length);
                    iRlt = ROK;
                    VOS_TaskUnlock();
                }
            }

            g_free(pReadBuf);
        }

        VOS_SemGive(ponLoadSem);
        return( iRlt);
    }
    else
    {
		OLT_ADD_DEBUG(OLT_ADD_TITLE"pontype(%d) successed to load dba from mem.\r\n", PonChipType);
    }
    /* E--modified by liwei056@2010-7-20 for Flash-VFS Support */
    
    VOS_SemGive(ponLoadSem);
    return( ROK );
}

/* wangysh add 根据fileID ，获取文件的版本信息，然后直接写入devsm_module结构*/
#if 0
STATUS file_ver_get( int fileID )
{
    char * fileName,pBuf;
    int fd,i,len;
    driver_file_desc_t *pdesc;
    SFT_VER *ver;
    UCHAR *pNew;
    fileName = get_file_name(fileID);
    if( NULL == fileName )
        return;

    switch( fileID )
    {
        case FLASH_RES_7:
        case FLASH_RES_10:
            if( fileID == FLASH_RES_7 )
                ver = &devsm_module[SYS_LOCAL_MODULE_SLOTNO].pon_fw_ver;
            else if( fileID == FLASH_RES_10 )
                ver = &devsm_module[SYS_LOCAL_MODULE_SLOTNO].pon_dba_ver;
            else
                break;
            if( ( fd = open(fileName,O_RDONLY,0) ) == VOS_OK )
            {
				pBuf = g_malloc(2048);
                if( NULL != pBuf )
                {
	                if( VOS_OK == read( fd,pBuf,2048 ) )
	                {
	                    pdesc = (driver_file_desc_t *)pBuf;
	                    for( i=0;i<2048/128;i++ )
	                    {
	                        if( 0 == pdesc->dev_type[0] )
	                            continue;
	                        else
	                        {
	                            len = VOS_StrLen( pdesc->file_ver );
	                            while( ( ver->size - VOS_StrLen(ver->pData)) <= len )
	                            {
	                                pNew = VOS_Realloc( ver->pData, 2*ver->size, MODULE_RPU_LOAD);
	                                if( pNew == NULL )
	                                    return VOS_ERROR;
	                                VOS_MemZero(pNew[ver->size], ver->size);
	                                ver->size*=2;
	                                ver->pData = pNew;
	                            }
	                            VOS_StrCpy( &ver->pData[VOS_StrLen(ver->pData)],pdesc->file_ver );
	                        }
	                    }
	                }
	                free( pBuf );
                }
            }
        break;
        case FLASH_SYSFILE_INI:
        break;
        default:
        break;
    }
}

/* Vxx.xx.xx.xx*/
STATUS parseFileVer( UCHAR * pBuf,sys_version_no *sft_ver  )
{
    UCHAR *p = NULL,*q = NULL;
    int flag=0;
    USHORT *a;
    
    a = (USHORT *)sft_ver;
    
    if( (p = VOS_StrToK( pBuf+1, ".")) != NULL )
    {
        while( ( q = VOS_StrToK(NULL,".") ) != NULL )
        {
            flag = 1;
            *a = VOS_AtoI(p);
            a++;
            p=q;
        }
        if( flag != 0 )
        {
            *a = VOS_AtoI(p);
            return VOS_OK;
        }
        else
            return VOS_ERROR;
    }
    if( flag == 0 )
        return VOS_ERROR;    
}
#endif

STATUS update_file_entry(LONG fileID)
{
    int i;
	for(i=0;i<=MAX_UPDATE_FILE;i++)
	{
        if( devsm_module[SYS_LOCAL_MODULE_SLOTNO].file_version[i].fileID == fileID )
        {
        	VOS_MemZero(&devsm_module[SYS_LOCAL_MODULE_SLOTNO].file_version[i],sizeof(update_file));
        }
	}
	return VOS_OK;
}

STATUS find_empty_entry(update_file **p)
{
    int i;
    for(i=0;i<=MAX_UPDATE_FILE;i++)
    {
        if(devsm_module[SYS_LOCAL_MODULE_SLOTNO].file_version[i].fileID == 0)
        {
            *p = &(devsm_module[SYS_LOCAL_MODULE_SLOTNO].file_version[i]);
            return VOS_OK;
        }
    }
    return VOS_ERROR;
}

extern unsigned char *um_encrypt_password ( char * passwd );
int fpga_ver_get( int fileID )
{
    char *pBuf,*p;
    int i,len,alloc_size;
    driver_file_desc_t *pdesc;
    update_file *ver;
	alloc_size = device_flash_filesize_get(fileID);
	if (alloc_size <= 0)
	{
		return VOS_ERROR;
	}
    pBuf = (char *)malloc(alloc_size);
    if( NULL == pBuf )
        return VOS_ERROR;
    VOS_MemZero(pBuf, alloc_size);
    if( VOS_ERROR == CPI_HOOK_CALL(cdsms_file_read)(fileID,pBuf,(LONG*)&alloc_size,0))
    {
        free( (void*)pBuf );
        return VOS_ERROR;
    }
    pdesc = (driver_file_desc_t *)pBuf;
    if( 0 != pdesc->dev_type[0] )
    {
        len = VOS_StrLen( pdesc->file_ver );
        if( len < 92 )
            pdesc->file_ver[len] = 0;
        else
        {
            free( pBuf );
            return VOS_ERROR;
        }
        if( VOS_OK == devsm_find_empty_entry(&ver) )
        {
            p = um_encrypt_password(pdesc->file_ver);
            if( NULL != p )
            {
                if(VOS_StrLen(p)<FILE_VER_MD5)
                {
                    VOS_StrCpy(ver->sft_ver,p);
                    ver->fileID = fileID;
                    ver->valid = VOS_YES;
                    VOS_Free(p);
                }
                else
                {
                    VOS_Free(p);
                }
            }
            else
            {
                free( pBuf );
                return VOS_ERROR;
            }
        }
    }
    
    free( (void*)pBuf );
    return VOS_OK;
    
}




STATUS fw_dba_ver_get( int fileID )
{
    char *pBuf,*p;
    int i,len,alloc_size;
    driver_file_desc_t *pdesc;
    update_file *ver;

    alloc_size = 2048;
    pBuf = (char *)VOS_Malloc(2048,MODULE_RPU_LOAD);
    if( NULL == pBuf )
        return VOS_ERROR;
        
    VOS_MemZero(pBuf, alloc_size);
    if( VOS_ERROR == CPI_HOOK_CALL(cdsms_file_read)(fileID,pBuf,(LONG*)&alloc_size,0))
    {
        VOS_Free( (void*)pBuf );
        return VOS_ERROR;
    }
       
    pdesc = (driver_file_desc_t *)pBuf;
    for( i=0;i<16;i++ )/*2048/128*/
    {
        if( 0 == pdesc->dev_type[0] )
        {
            pdesc++;
            continue;
        }
        else
        {
            len = VOS_StrLen( pdesc->file_ver );
            if( len < 92 )
                pdesc->file_ver[len] = 0;
            else
            {
                pdesc++;
                continue;
            }
            if( VOS_OK == find_empty_entry(&ver) )
            {
                p = um_encrypt_password(pdesc->file_ver);
                if( NULL != p )
	            {
	                if(VOS_StrLen(p)<FILE_VER_MD5)
	                {
	                    VOS_StrCpy(ver->sft_ver,p);
	                    ver->fileID = fileID;
	                    ver->valid = VOS_YES;
	                    VOS_Free(p);
	                }
	                else
	                {
	                    VOS_Free(p);
	                    pdesc++;
	                    continue;
	                }
	            }
            }
            else
            {
                VOS_Free( (void*)pBuf );
                return VOS_ERROR;
            }
            pdesc++;
        }
    }
    VOS_Free( (void*)pBuf );
    return VOS_OK;
    
#if 0
    if( ( fd = open(fileName,O_RDONLY,0) ) != VOS_ERROR )
    {
		pBuf = (char *)VOS_Malloc(2048,MODULE_RPU_LOAD);
        if( NULL != pBuf )
        {
		    VOS_MemZero(pBuf, 2048);
            if( VOS_ERROR != read( fd,pBuf,2048 ) )
            {
                pdesc = (driver_file_desc_t *)pBuf;
                for( i=0;i<16;i++ )/*2048/128*/
                {
                    if( 0 == pdesc->dev_type[0] )
                        continue;
                    else
                    {
                        len = VOS_StrLen( pdesc->file_ver );
                        if( len < 92 )
                            pdesc->file_ver[len] = 0;
                        else
                        {
                            close(fd);
                            return VOS_ERROR;
                        }
                        if( VOS_OK == find_empty_entry(&ver) )
                        {
                            ver->fileID = fileID;
                            p = um_encrypt_password(pdesc->file_ver);
                            if( NULL == p )
                                return VOS_ERROR;
                            if(VOS_StrLen(p)<FILE_VER_MD5)
                            {
                                VOS_StrCpy(ver->sft_ver,p);
                                VOS_Free(p);
                            }
                            else
                            {
                                VOS_Free(p);
                                return VOS_ERROR;
                            }
                            /*parseFileVer(pdesc->file_ver,&(ver->sft_ver));*/
                            pdesc++;
                        }
                        else
                        {
                            close(fd);
                            return VOS_OK;
                        }
                    }
                }
                close(fd);
            }
            VOS_Free( (void*)pBuf );
        }
    }
#endif
}

STATUS fw_gpon_ver_get(int fileID)
	{
		char *pBuf,*p,*name;
		int i,len,alloc_size;
		driver_file_desc_t *pdesc;
		update_file *ver;
	
		name = get_file_name(fileID);
		if( NULL == name )
			return VOS_ERROR;	
		if(VOS_OK != device_file_is_exist(name))
		{
			return VOS_ERROR;
		}
		alloc_size = device_flash_filesize_get(fileID);
		if (alloc_size <= 0)
		{
			return VOS_ERROR;
		}
		pBuf = (char *)malloc(alloc_size);
		if( NULL == pBuf )
			return VOS_ERROR;
		VOS_MemZero(pBuf, alloc_size);
		if( VOS_ERROR == CPI_HOOK_CALL(cdsms_file_read)(fileID,pBuf,(LONG*)&alloc_size,0))
		{
			free( (void*)pBuf );
			return VOS_ERROR;
		}
		pdesc = (driver_file_desc_t *)pBuf;
		if( 0 != pdesc->dev_type[0] )
		{
			len = VOS_StrLen( pdesc->file_ver );
			if( len < 92 )
				pdesc->file_ver[len] = 0;
			else
			{
				free( pBuf );
				return VOS_ERROR;
			}
			if( VOS_OK == devsm_find_empty_entry(&ver) )
			{
				p = um_encrypt_password(pdesc->file_ver);
				if( NULL != p )
				{
					if(VOS_StrLen(p)<FILE_VER_MD5)
					{
						VOS_StrCpy(ver->sft_ver,p);
						ver->fileID = FLASH_FIRMWARE_GPON;
						ver->valid = VOS_YES;
						VOS_Free(p);
					}
					else
					{
						VOS_Free(p);
					}
				}
				else
				{
					free( pBuf );
					return VOS_ERROR;
				}
			}
		}
		
		free( (void*)pBuf );
		return VOS_OK;	
	}




#if 0
STATUS dba_ver_get( int fileID )
{
    char * fileName,*pBuf,*p;
    int fd,i,len;
    driver_file_desc_t *pdesc;
    update_file *ver;
    fileName = get_file_name(fileID);
    if( NULL == fileName )
        return;

    if( ( fd = open(fileName,O_RDONLY,0) ) != VOS_ERROR )
    {
		pBuf = (char *)VOS_Malloc(2048,MODULE_RPU_LOAD);
        if( NULL != pBuf )
        {
	        VOS_MemZero(pBuf, 2048);
            if( VOS_ERROR != read( fd,pBuf,2048 ) )
            {
                pdesc = (driver_file_desc_t *)pBuf;
                for( i=0;i<2;i++ )
                {
                    if( 0 == pdesc->dev_type[0] )
                        continue;
                    else
                    {
                        len = VOS_StrLen( pdesc->file_ver );
                        if( len < 92 )
                            pdesc->file_ver[len] = 0;
                        else
                        {
                            VOS_Free( pBuf );
                            return VOS_ERROR;
                        }
                        pdesc->file_ver[len] = 0;
                        if( VOS_OK == find_empty_entry(&ver) )
                        {
                            ver->fileID = fileID;
                            p = um_encrypt_password(pdesc->file_ver);
                            if( NULL == p )
                                return VOS_ERROR;
                            if(VOS_StrLen(p)<FILE_VER_MD5)
                            {
                                VOS_StrCpy(ver->sft_ver,p);
                                VOS_Free(p);
                            }
                            else
                            {
                                VOS_Free(p);
                                return VOS_ERROR;
                            }
                            pdesc++;
                        }
                        else
                        {
                            VOS_Free( pBuf );
                            sys_console_printf("\r\n update file exceed");
                            return VOS_ERROR;
                        }
                    }
                }
            }
            VOS_Free( pBuf );
        }
    }
}

#endif

STATUS onufile_ver_get( int fileID )
{
    char *pBuf,*p,*pdesc/*,file_ver*/;
    int i,len,alloc_size,off_set;
    update_file *ver;

    alloc_size = 2048;
    pBuf = (char *)VOS_Malloc(2048,MODULE_RPU_LOAD);
    if( NULL == pBuf )
        return VOS_ERROR;
        
    VOS_MemZero(pBuf, alloc_size);
    if( VOS_ERROR == CPI_HOOK_CALL(cdsms_file_read)(fileID,pBuf,(LONG*)&alloc_size,0))
    {
        VOS_Free( (void*)pBuf );
        return VOS_ERROR;
    }
       
    pdesc = pBuf;
    off_set = ONU_TYPE_LEN+ONU_OFFSET_LEN+ONU_FILE_LEN+ONU_COMPRESS_FLAG_LEN+ONU_FILE_FACT_LEN+4;
    
    for( i=0;i<16;i++ )/*2048/128*/
    {
        if( 0 == (*pdesc) )
        {
            pdesc += 128;
            continue;
        }
        else
        {
            len = VOS_StrLen( pdesc+off_set );
            if( len < 88 )
            {
                *(pdesc+off_set+len) = 0;
            }
            else
            {
                pdesc += 128;
                continue;
            }
            if( VOS_OK == find_empty_entry(&ver) )
            {
                p = um_encrypt_password(pdesc+off_set);
                if( NULL != p )
	            {
	                if(VOS_StrLen(p)<FILE_VER_MD5)
	                {
	                    VOS_StrCpy(ver->sft_ver,p);
	                    ver->fileID = fileID;
	                    ver->valid = VOS_YES;
	                    VOS_Free(p);
	                }
	                else
	                {
	                    VOS_Free(p);
	                    pdesc += 128;
	                    continue;
	                }
	            }
            }
            else
            {
                VOS_Free( (void*)pBuf );
                return VOS_ERROR;
            }
            pdesc += 128;
        }
    }
    VOS_Free( (void*)pBuf );
    return VOS_OK;
    
}
STATUS sysfile_ver_get( int fileID )
{
    char *pBuf,*p;
    int len,alloc_size;
    driver_file_desc_t *pdesc;
    update_file *ver;

    alloc_size = 128;
    pBuf = (char *)VOS_Malloc(alloc_size,MODULE_RPU_LOAD);
    if( NULL == pBuf )
        return VOS_ERROR;
	VOS_MemZero(pBuf, alloc_size);
    if( VOS_ERROR == CPI_HOOK_CALL(cdsms_file_read)(fileID,pBuf,(LONG*)&alloc_size,0))
    {
        VOS_Free( pBuf );
        return VOS_ERROR;
    }

    pdesc = (driver_file_desc_t *)pBuf;
    if( 0 != pdesc->dev_type[0] )
    {
        len = VOS_StrLen( pdesc->file_ver );
        if( len < 92 )
            pdesc->file_ver[len] = 0;
        else
        {
            VOS_Free( pBuf );
            return VOS_ERROR;
        }
        if( VOS_OK == find_empty_entry(&ver) )
        {
            p = um_encrypt_password(pdesc->file_ver);
            if( NULL != p )
            {
                if(VOS_StrLen(p)<FILE_VER_MD5)
                {
                    VOS_StrCpy(ver->sft_ver,p);
                    ver->fileID = fileID;
                    ver->valid = VOS_YES;
                    VOS_Free(p);
                }
                else
                {
                    VOS_Free(p);
                }
            }
            else
            {
                VOS_Free( pBuf );
                return VOS_ERROR;
            }
        }
    }

    VOS_Free( pBuf );
    return VOS_OK;
#if 0    
    if( ( fd = open(fileName,O_RDONLY,0) ) != VOS_ERROR )
    {
		pBuf = (char *)VOS_Malloc(128,MODULE_RPU_LOAD);
        if( NULL != pBuf )
        {
	        VOS_MemZero(pBuf, 128);
            if( VOS_ERROR != read( fd,pBuf,128 ) )
            {
                pdesc = (driver_file_desc_t *)pBuf;
                if( 0 == pdesc->dev_type[0] )
                    return VOS_ERROR;
                else
                {
                    len = VOS_StrLen( pdesc->file_ver );
                    if( len < 92 )
                        pdesc->file_ver[len] = 0;
                    else
                    {
                        VOS_Free( pBuf );
                        return VOS_ERROR;
                    }
                    if( VOS_OK == find_empty_entry(&ver) )
                    {
                        ver->fileID = fileID;
                        p = um_encrypt_password(pdesc->file_ver);
                        if( NULL == p )
                            return VOS_ERROR;
                        if(VOS_StrLen(p)<FILE_VER_MD5)
                        {
                            VOS_StrCpy(ver->sft_ver,p);
                            VOS_Free(p);
                        }
                        else
                        {
                            VOS_Free(p);
                            return VOS_ERROR;
                        }
                        /*pdesc++;*/
                    }
                    else
                        return VOS_OK;
                }
            }
            VOS_Free( pBuf );
        }
    }
#endif
    
}

STATUS appfile_ver_get( int fileID )
{
	char *pBuf,*p,*name;
	int i,len,alloc_size;
	driver_file_desc_t *pdesc;
	update_file *ver;

	name = get_file_name(fileID);
    if( NULL == name )
        return VOS_ERROR;	
	if(VOS_OK != device_file_is_exist(name))
	{
		return VOS_ERROR;
	}
	alloc_size = device_flash_filesize_get(fileID);
	if (alloc_size <= 0)
	{
		return VOS_ERROR;
	}
	pBuf = (char *)malloc(alloc_size);
	if( NULL == pBuf )
		return VOS_ERROR;
	VOS_MemZero(pBuf, alloc_size);
	if( VOS_ERROR == CPI_HOOK_CALL(cdsms_file_read)(fileID,pBuf,(LONG*)&alloc_size,0))
	{
		free( (void*)pBuf );
		return VOS_ERROR;
	}
	pdesc = (driver_file_desc_t *)pBuf;
	if( 0 != pdesc->dev_type[0] )
	{
		len = VOS_StrLen( pdesc->file_ver );
		if( len < 92 )
			pdesc->file_ver[len] = 0;
		else
		{
			free( pBuf );
			return VOS_ERROR;
		}
		if( VOS_OK == devsm_find_empty_entry(&ver) )
		{
			p = um_encrypt_password(pdesc->file_ver);
			if( NULL != p )
			{
				if(VOS_StrLen(p)<FILE_VER_MD5)
				{
					VOS_StrCpy(ver->sft_ver,p);
					ver->fileID = fileID;
					ver->valid = VOS_YES;
					VOS_Free(p);
				}
				else
				{
					VOS_Free(p);
				}
			}
			else
			{
				free( pBuf );
				return VOS_ERROR;
			}
		}
	}
	
	free( (void*)pBuf );
	return VOS_OK;	
}



void update_all_file_ver(void)
{
	update_file_ver(FLASH_RES_5);
	update_file_ver(FLASH_RES_7);
	update_file_ver(FLASH_SYSFILE_INI);
	update_file_ver(FLASH_RES_10);
	update_file_ver(FLASH_PATCH_FILE);
	update_file_ver(FLASH_APP_CODE_ARAD);
	update_file_ver(FLASH_APP_CODE_GPON);
}
void update_file_ver( int fileID )
{
	if(!( 
			fileID==FLASH_RES_5 || 
	    	fileID == FLASH_RES_7 ||
	    	fileID == FLASH_SYSFILE_INI || 
	    	fileID == FLASH_RES_10||
		fileID == FLASH_APP_CODE_ARAD||
		fileID == FLASH_APP_CODE_GPON||
		fileID == FLASH_FIRMWARE_GPON
	    )
	  )
	    return;
	    
	update_file_entry(fileID);
	switch(fileID)
	{
		case FLASH_RES_5:
			onufile_ver_get(fileID);
		break;
		case FLASH_RES_7:
			fw_dba_ver_get(fileID);
		break;
		case FLASH_SYSFILE_INI:
			sysfile_ver_get(fileID);
		break;
		case FLASH_RES_10:
			fw_dba_ver_get(fileID);
		break;
		case FLASH_PATCH_FILE:
			PatchModFileVerGet(fileID);
		break;
		case FLASH_FPGA_8000_SW:
			fpga_ver_get(fileID);
		break;
		case FLASH_APP_CODE_ARAD:
		case FLASH_APP_CODE_GPON:
		case FLASH_FIRMWARE_GPON:
			appfile_ver_get(fileID);
		break;
		default:
		break;
	}
}
/* added by chenfj 2007-8-31 
	PAS-SOFT软件包中定义的CLI命令*/
#ifdef PAS_CLI_DEFINED
#endif
const char *error_string_lookup(short int error_code)
{
	int counter;

	for(counter=0 ; PAS_CLI_return_code_str[counter].return_code_str != NULL ; counter++)
		if( PAS_CLI_return_code_str[counter].return_code_val == error_code)
			return PAS_CLI_return_code_str[counter].return_code_str;

	return("Error code was not found!!!");
}
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
static void Print_oam_mii_phy_advertisement (struct vty *vty, const PON_oam_mii_phy_advertisement_t  *mii_phy_advertisement )
{

	vty_out(vty,"10base TX half duplex            : %s%s",Pon_BooleanString(mii_phy_advertisement->_10base_tx_half_duplex),VTY_NEWLINE);
	vty_out(vty,"10base TX full duplex            : %s%s",Pon_BooleanString(mii_phy_advertisement->_10base_tx_full_duplex),VTY_NEWLINE);
	vty_out(vty,"100base TX half duplex           : %s%s",Pon_BooleanString(mii_phy_advertisement->_100base_tx_half_duplex),VTY_NEWLINE);
	vty_out(vty,"100base TX full duplex           : %s%s",Pon_BooleanString(mii_phy_advertisement->_100base_tx_full_duplex),VTY_NEWLINE);
	vty_out(vty,"100base T4                       : %s%s",Pon_BooleanString(mii_phy_advertisement->_100base_t4),VTY_NEWLINE);
	vty_out(vty,"Pause                            : %s%s",Pon_BooleanString(mii_phy_advertisement->pause),VTY_NEWLINE);
	vty_out(vty,"Asymmetric pause                 : %s%s",Pon_BooleanString(mii_phy_advertisement->asymmetric_pause),VTY_NEWLINE);
}

static void Print_oam_gmii_phy_advertisement (struct vty *vty, const PON_oam_gmii_phy_advertisement_t  *gmii_phy_advertisement )
{
	vty_out(vty,"1000base TX half duplex          : %s%s",Pon_BooleanString(gmii_phy_advertisement->_1000base_tx_half_duplex),VTY_NEWLINE);
	vty_out(vty,"1000base TX full duplex          : %s%s",Pon_BooleanString(gmii_phy_advertisement->_1000base_tx_full_duplex),VTY_NEWLINE);
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/

int CLI_Print_oam_information (struct vty *vty, const PON_oam_information_t   *oam_information)
{
    								 
	OAM_1_2_tlv_tuple_t  *oam_1_2_standard_information;
	OAM_2_0_tlv_t		 *oam_2_0_standard_information;
	OAM_3_3_tlv_t        *oam_3_3_standard_information;
    

	/* OAM general information */
	vty_out (vty,"OAM general information%s=========================%s",VTY_NEWLINE, VTY_NEWLINE);

	vty_out (vty,"OAM                : ");
    switch(oam_information->oam_information_reference_standard) 
    {
    case OAM_STANDARD_VERSION_1_2:
		    vty_out (vty,"1.2%s",VTY_NEWLINE);
    	break;
    case OAM_STANDARD_VERSION_2_0:
            vty_out (vty,"2.0%s",VTY_NEWLINE);
    	break;
    case OAM_STANDARD_VERSION_3_3:
            vty_out (vty,"Standard%s",VTY_NEWLINE);
        break;
    default:
            vty_out (vty,"Unsupported%s",VTY_NEWLINE);
    }
    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
	vty_out (vty,"Passave originated : %s%s", Pon_BooleanString(oam_information->passave_originated), VTY_NEWLINE);

	vty_out (vty,"Vendor code        : %u%sModel number       : %u%s%s",
			 oam_information->passave_specific_oam_information.vendor_code,
			 VTY_NEWLINE,
			 oam_information->passave_specific_oam_information.model_number, 
			 VTY_NEWLINE, VTY_NEWLINE);


	/* print the OAM information according to OAM standards version */
	switch(oam_information->oam_information_reference_standard) 
	{
	case OAM_STANDARD_VERSION_1_2:
	        {
		        oam_1_2_standard_information = (OAM_1_2_tlv_tuple_t  *)oam_information->oam_standard_information;

		        vty_out (vty,"OAM 1.2 information%s=========================%s",VTY_NEWLINE, VTY_NEWLINE);

		        vty_out (vty,"Loopback state        :%s%sOAM configuration     : ", 
				         OAM_loopback_state_s[oam_1_2_standard_information->state.loopback_state], VTY_NEWLINE);

		        if		(oam_1_2_standard_information->configuration.mode == OAM_PASSIVE_MODE) vty_out (vty,"%s, ",enum_s(OAM_PASSIVE_MODE));
		        else if (oam_1_2_standard_information->configuration.mode == OAM_ACTIVE_MODE)  vty_out (vty,"%s, ",enum_s(OAM_ACTIVE_MODE));
		        else vty_out (vty,"Illegal oam mode, ");
		        
		        if		(oam_1_2_standard_information->configuration.unidirectional_support == OAM_UNIDIRECTIONAL_SUPPORT_DISABLE) vty_out (vty,"%s, ",enum_s(OAM_UNIDIRECTIONAL_SUPPORT_DISABLE));
		        else if (oam_1_2_standard_information->configuration.unidirectional_support == OAM_UNIDIRECTIONAL_SUPPORT_ENABLE)  vty_out (vty,"%s, ",enum_s(OAM_UNIDIRECTIONAL_SUPPORT_ENABLE));
		        else vty_out (vty,"Illegal unidirectional_support field,");

		        if		(oam_1_2_standard_information->configuration.loopback_support == OAM_LOOPBACK_NOT_SUPPORTED) vty_out (vty,"%s, ",enum_s(OAM_LOOPBACK_NOT_SUPPORTED));
		        else if (oam_1_2_standard_information->configuration.loopback_support == OAM_LOOPBACK_SUPPORTED)     vty_out (vty,"%s, ",enum_s(OAM_LOOPBACK_SUPPORTED));
		        else vty_out (vty,"Illegal loopback_support field,");

		        vty_out(vty,"%sMaximum PDU size     : %d%s",VTY_NEWLINE, oam_1_2_standard_information->pdu_configuration.maximum_pdu_size, VTY_NEWLINE);
		        vty_out(vty,"Enterprise Identifier : %u%s",oam_1_2_standard_information->extension.enterprise_identifier, VTY_NEWLINE);
		        vty_out(vty,"Device Identifier     : %x%s",oam_1_2_standard_information->extension.device_identifier, VTY_NEWLINE);
		        vty_out(vty,"Version Identifier    : %x%s",oam_1_2_standard_information->extension.version_identifier, VTY_NEWLINE);
            }
		break;
	case OAM_STANDARD_VERSION_2_0:
	        {
		        oam_2_0_standard_information = (OAM_2_0_tlv_t *)oam_information->oam_standard_information;

		        vty_out (vty,"OAM standard information%s=========================%s",VTY_NEWLINE, VTY_NEWLINE);
		        vty_out (vty,"State              : " );
		        if (oam_2_0_standard_information->state.multiplexer_action == OAM_MULTIPLEXER_ACTION_FORWARDING) 
			        vty_out (vty,"%s, ",enum_s(OAM_MULTIPLEXER_ACTION_FORWARDING));
		        else if (oam_2_0_standard_information->state.multiplexer_action == OAM_MULTIPLEXER_ACTION_DISCARDING) 
			        vty_out (vty,"%s, ",enum_s(OAM_MULTIPLEXER_ACTION_DISCARDING));
		        else vty_out (vty,"Illegal multiplexer_action field, ");

		        if (oam_2_0_standard_information->state.parser_action == OAM_PARSER_ACTION_FORWARDING)
			        vty_out (vty,"%s%s",enum_s(OAM_PARSER_ACTION_FORWARDING), VTY_NEWLINE);
		        else if (oam_2_0_standard_information->state.parser_action == OAM_PARSER_ACTION_LOOPING_BACK)
			        vty_out (vty,"%s%s",enum_s(OAM_PARSER_ACTION_LOOPING_BACK), VTY_NEWLINE);
		        else if (oam_2_0_standard_information->state.parser_action == OAM_PARSER_ACTION_DISCARDING)
			        vty_out (vty,"%s%s",enum_s(OAM_PARSER_ACTION_DISCARDING),VTY_NEWLINE);
		        else if (oam_2_0_standard_information->state.parser_action == OAM_PARSER_ACTION_RESERVED)
			        vty_out (vty,"%s%s",enum_s(OAM_PARSER_ACTION_RESERVED),VTY_NEWLINE);
		        else vty_out (vty,"Illegal parser_action field%s", VTY_NEWLINE);

		        vty_out (vty,"Configuration      : ");

		        if (oam_2_0_standard_information->configuration.organization_specific_information == OAM_INTERPRETING_ORGANIZATION_SPECIFIC_INFORMATION_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_ORGANIZATION_SPECIFIC_INFORMATION_SUPPORTED));
		        else if (oam_2_0_standard_information->configuration.organization_specific_information == OAM_INTERPRETING_ORGANIZATION_SPECIFIC_NFORMATION_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_ORGANIZATION_SPECIFIC_NFORMATION_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal organization_specific_information field, ");

		        if (oam_2_0_standard_information->configuration.organization_specific_events == OAM_INTERPRETING_ORGANIZATION_SPECIFIC_EVENTS_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_ORGANIZATION_SPECIFIC_EVENTS_SUPPORTED));
		        else if (oam_2_0_standard_information->configuration.organization_specific_events == OAM_INTERPRETING_ORGANIZATION_SPECIFIC_EVENTS_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_ORGANIZATION_SPECIFIC_EVENTS_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal organization_specific_events field, ");

		        if (oam_2_0_standard_information->configuration.organization_specific_oampdu == OAM_INTERPRETING_ORGANIZATION_SPECIFIC_OAMPDU_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_ORGANIZATION_SPECIFIC_OAMPDU_SUPPORTED));
		        else if (oam_2_0_standard_information->configuration.organization_specific_oampdu == OAM_INTERPRETING_ORGANIZATION_SPECIFIC_OAMPDU_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_ORGANIZATION_SPECIFIC_OAMPDU_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal organization_specific_oampdu field, ");

		        if (oam_2_0_standard_information->configuration.variable_retrieval == OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_SUPPORTED));
		        else if (oam_2_0_standard_information->configuration.variable_retrieval == OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal variable_retrieval field, ");

		        if (oam_2_0_standard_information->configuration.link_events == OAM_INTERPRETING_LINK_EVENTS_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_LINK_EVENTS_SUPPORTED));
		        else if (oam_2_0_standard_information->configuration.link_events == OAM_INTERPRETING_LINK_EVENTS_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_LINK_EVENTS_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal link_events field, ");

		        if (oam_2_0_standard_information->configuration.loopback_support == OAM_LOOPBACK_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_LOOPBACK_SUPPORTED));
		        else if (oam_2_0_standard_information->configuration.loopback_support == OAM_LOOPBACK_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_LOOPBACK_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal loopback_support field, ");

		        if (oam_2_0_standard_information->configuration.unidirectional_support == OAM_UNIDIRECTIONAL_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_UNIDIRECTIONAL_SUPPORTED));
		        else if (oam_2_0_standard_information->configuration.unidirectional_support == OAM_UNIDIRECTIONAL_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_UNIDIRECTIONAL_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal unidirectional_support field, ");

		        if (oam_2_0_standard_information->configuration.oam_mode == OAM_MODE_ACTIVE)
			        vty_out (vty,"%s%s",enum_s(OAM_MODE_ACTIVE), VTY_NEWLINE);
		        else if (oam_2_0_standard_information->configuration.oam_mode == OAM_MODE_PASSIVE)
			         vty_out (vty,"%s%s",enum_s(OAM_MODE_PASSIVE), VTY_NEWLINE);
		        else vty_out (vty,"Illegal oam_mode field%s", VTY_NEWLINE);

		        vty_out (vty,"Maximum PDU size   : %d%s", oam_2_0_standard_information->pdu_configuration.maximum_pdu_size, VTY_NEWLINE);

		        vty_out (vty,"Version identifiers: Version identifier: 0x%x, device identifier: 0x%x, OUI:0x%x%s",
				         oam_2_0_standard_information->vendor_identifier.version_identifier, 
				         oam_2_0_standard_information->vendor_identifier.device_identifier, 
				         oam_2_0_standard_information->vendor_identifier.oui,VTY_NEWLINE);
	        } 
		break;
    case OAM_STANDARD_VERSION_3_3:
	        {
                oam_3_3_standard_information = (OAM_3_3_tlv_t *)oam_information->oam_standard_information;

		        vty_out (vty,"OAM standard information%s=========================%s",VTY_NEWLINE, VTY_NEWLINE);
		        vty_out (vty,"State              : " );
		        if (oam_3_3_standard_information->state.multiplexer_action == OAM_MULTIPLEXER_ACTION_FORWARDING) 
			        vty_out (vty,"%s, ",enum_s(OAM_MULTIPLEXER_ACTION_FORWARDING));
		        else if (oam_3_3_standard_information->state.multiplexer_action == OAM_MULTIPLEXER_ACTION_DISCARDING) 
			        vty_out (vty,"%s, ",enum_s(OAM_MULTIPLEXER_ACTION_DISCARDING));
		        else vty_out (vty,"Illegal multiplexer_action field, ");

		        if (oam_3_3_standard_information->state.parser_action == OAM_PARSER_ACTION_FORWARDING)
			        vty_out (vty,"%s%s",enum_s(OAM_PARSER_ACTION_FORWARDING),VTY_NEWLINE);
		        else if (oam_3_3_standard_information->state.parser_action == OAM_PARSER_ACTION_LOOPING_BACK)
			        vty_out (vty,"%s%s",enum_s(OAM_PARSER_ACTION_LOOPING_BACK),VTY_NEWLINE);
		        else if (oam_3_3_standard_information->state.parser_action == OAM_PARSER_ACTION_DISCARDING)
			        vty_out (vty,"%s%s",enum_s(OAM_PARSER_ACTION_DISCARDING),VTY_NEWLINE);
		        else if (oam_3_3_standard_information->state.parser_action == OAM_PARSER_ACTION_RESERVED)
			        vty_out (vty,"%s%s",enum_s(OAM_PARSER_ACTION_RESERVED),VTY_NEWLINE);
		        else vty_out (vty,"Illegal parser_action field%s",VTY_NEWLINE);

		        vty_out (vty,"Configuration      : ");

		        if (oam_3_3_standard_information->configuration.variable_retrieval == OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_SUPPORTED));
		        else if (oam_3_3_standard_information->configuration.variable_retrieval == OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_SENDING_VARIABLE_RESPONSE_OAMPDUS_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal variable_retrieval field, ");

		        if (oam_3_3_standard_information->configuration.link_events == OAM_INTERPRETING_LINK_EVENTS_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_LINK_EVENTS_SUPPORTED));
		        else if (oam_3_3_standard_information->configuration.link_events == OAM_INTERPRETING_LINK_EVENTS_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_INTERPRETING_LINK_EVENTS_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal link_events field, ");

		        if (oam_3_3_standard_information->configuration.loopback_support == OAM_LOOPBACK_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_LOOPBACK_SUPPORTED));
		        else if (oam_3_3_standard_information->configuration.loopback_support == OAM_LOOPBACK_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_LOOPBACK_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal loopback_support field, ");

		        if (oam_3_3_standard_information->configuration.unidirectional_support == OAM_UNIDIRECTIONAL_SUPPORTED)
			        vty_out (vty,"%s, ",enum_s(OAM_UNIDIRECTIONAL_SUPPORTED));
		        else if (oam_3_3_standard_information->configuration.unidirectional_support == OAM_UNIDIRECTIONAL_NOT_SUPPORTED)
			         vty_out (vty,"%s, ",enum_s(OAM_UNIDIRECTIONAL_NOT_SUPPORTED));
		        else vty_out (vty,"Illegal unidirectional_support field, ");

		        if (oam_3_3_standard_information->configuration.oam_mode == OAM_MODE_ACTIVE)
			        vty_out (vty,"%s%s",enum_s(OAM_MODE_ACTIVE),VTY_NEWLINE);
		        else if (oam_3_3_standard_information->configuration.oam_mode == OAM_MODE_PASSIVE)
			         vty_out (vty,"%s%s",enum_s(OAM_MODE_PASSIVE),VTY_NEWLINE);
		        else vty_out (vty,"Illegal oam_mode field%",VTY_NEWLINE);

		        vty_out (vty,"Maximum PDU size   : %d%s", oam_3_3_standard_information->pdu_configuration.maximum_pdu_size,VTY_NEWLINE);

		        vty_out (vty,"Vendor identifiers : 0x%x%sOUI                : 0x%x%s",
				         oam_3_3_standard_information->vendor_identifier.identifier,VTY_NEWLINE,
				         oam_3_3_standard_information->vendor_identifier.oui,VTY_NEWLINE);
	        } 
		break;
	default:
   		vty_out (vty,"Unsupported OAM standard specific information%s",VTY_NEWLINE);
	}
    

	/*Begin: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
	if (oam_information->passave_originated == TRUE)
	{
		/* PHY status information */
		vty_out (vty,"%sPHY status information%s======================%s",VTY_NEWLINE,VTY_NEWLINE,VTY_NEWLINE);
		vty_out (vty,"PHY extended register capability : %s%s",
			Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_phy_status.phy_extended_register_capability),VTY_NEWLINE);
		vty_out (vty,"PHY auto negotiation ability     : %s%s",
			Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_phy_status.phy_auto_negotiation_ability),VTY_NEWLINE);
		vty_out (vty,"Remote fault                     : %s%s",
			Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_phy_status.remote_fault),VTY_NEWLINE);
		vty_out (vty,"Autonegotiation complete         : %s%s",
			Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_phy_status.auto_negotiation_complete),VTY_NEWLINE);

		vty_out (vty,"PHY MII TAF:%s",VTY_NEWLINE);
		Print_oam_mii_phy_advertisement (vty, &oam_information->passave_specific_oam_information.uni_port_phy_status.phy_advertisement.mii_phy_advertisement);
		vty_out (vty,"PHY GMII advertisement:%s",VTY_NEWLINE);
		Print_oam_gmii_phy_advertisement (vty, &oam_information->passave_specific_oam_information.uni_port_phy_status.phy_advertisement.gmii_phy_advertisement);
		vty_out (vty,"Preferable port type             : %s%s", PON_master_slave_s[oam_information->passave_specific_oam_information.uni_port_phy_status.phy_advertisement.gmii_phy_advertisement.preferable_port_type],VTY_NEWLINE);

		vty_out (vty,"PHY Partner MII TAF:%s",VTY_NEWLINE);
		Print_oam_mii_phy_advertisement (vty, &oam_information->passave_specific_oam_information.uni_port_phy_status.link_partner_advertisement.mii_phy_advertisement );
		vty_out (vty,"PHY Partner GMII advertisement:%s",VTY_NEWLINE);
		Print_oam_gmii_phy_advertisement (vty, &oam_information->passave_specific_oam_information.uni_port_phy_status.link_partner_advertisement.gmii_phy_advertisement );
		
		vty_out(vty,"Link partner is auto-negotiation capable : %s%s",
			Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_phy_status.link_partner_is_auto_negotiation_capable),VTY_NEWLINE);

		vty_out (vty,"PHY master-slave manual mode setting     : %s%s",PON_master_slave_s[oam_information->passave_specific_oam_information.uni_port_phy_status.phy_master_slave_manual_mode_setting],VTY_NEWLINE);

		vty_out (vty,"PHY master-slave manual mode		 : %s%s",
			Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_phy_status.phy_master_slave_manual_mode),VTY_NEWLINE);

		vty_out (vty,"Master-slave configuration resolution	 : %s%s",PON_master_slave_s[oam_information->passave_specific_oam_information.uni_port_phy_status.master_slave_configuration_resolution],VTY_NEWLINE);

		vty_out (vty,"Master-Slave configuration fault : %s%s",
			Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_phy_status.master_slave_configuration_fault),VTY_NEWLINE);

		vty_out (vty,"%s",VTY_NEWLINE);

		/* MAC status information */
		vty_out (vty,"(UNI port) MAC status information%s=================================%s",VTY_NEWLINE,VTY_NEWLINE);
		vty_out (vty,"UNI port MAC type                 : %s%s",PON_mac_s[oam_information->passave_specific_oam_information.uni_port_mac_status.uni_port_mac_type],VTY_NEWLINE);
		vty_out (vty,"UNI port MII type rate            : %d Mb/Sec%s",oam_information->passave_specific_oam_information.uni_port_mac_status.uni_port_mii_type_rate,VTY_NEWLINE);
		vty_out (vty,"UNI port MII type duplex          : %s%s",PON_duplex_status_s[oam_information->passave_specific_oam_information.uni_port_mac_status.uni_port_mii_type_duplex],VTY_NEWLINE);
		vty_out (vty,"UNI autonegotiation               : %s%s",Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_mac_status.uni_autonegotiation),VTY_NEWLINE);
		vty_out (vty,"UNI port link                     : %s%s",Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_mac_status.uni_port_link),VTY_NEWLINE);
		vty_out (vty,"UNI master                        : %s%s",PON_master_slave_s[oam_information->passave_specific_oam_information.uni_port_mac_status.uni_master],VTY_NEWLINE);
		vty_out (vty,"Auto-negotiation resolution error : %s%s",Pon_BooleanString (oam_information->passave_specific_oam_information.uni_port_mac_status.auto_negotiation_resolution_error),VTY_NEWLINE);
		vty_out (vty,"Resolution master                 : %s%s",PON_master_slave_s[oam_information->passave_specific_oam_information.uni_port_mac_status.resolution_master],VTY_NEWLINE);
		vty_out (vty,"Resolution pause Rx               : %s%s",Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_mac_status.resolution_pause_Rx),VTY_NEWLINE);
		vty_out (vty,"Resolution pause Tx               : %s%s",Pon_BooleanString(oam_information->passave_specific_oam_information.uni_port_mac_status.resolution_pause_Tx),VTY_NEWLINE);
	}
    /*End: for 10G EPON of PMC8411 by jinhl @2012-11-12*/
 	return (EXIT_OK);
}

#ifdef  PRODUCT_EPON3_GFA6100_TEST
int  InitPonChipInfo(short int PonPortIdx )
{
    int CardSlot;

	if(PonPortIdx >= MAXPON ) return( RERROR );
	PonCardStatus[0] = PONCARD_LOADING;
    
    CardSlot = GetCardIdxByPonChip(PonPortIdx);
	if(GetOltCardslotInserted(CardSlot) != CARDINSERT )
		SetOltCardslotInserted(CardSlot);
	
	PonChipMgmtTable[PonPortIdx].Type = PONCHIP_PAS5201;
	PonChipMgmtTable[PonPortIdx].version = PonChipMgmtTable[PonPortIdx].Type;
	PonPortTable[PonPortIdx].CTC_Supported = 0;
	SetPonPortType(PonPortIdx, EPONMAUTYPE1000BASEPX20UOLT);

	SetPonChipDataPathMac(PonPortIdx );
	SetPonchipMgmtPathMac(PonPortIdx);
	/*
	VOS_MemCpy( PonChipMgmtTable[PonPortIdx].MgmtPathMAC, PAS_Mgmt_Mac, BYTES_IN_MAC_ADDRESS );
	PonChipMgmtTable[PonPortIdx].MgmtPathMAC[5] = PonPortIdx * 16;
	VOS_MemCpy( PonChipMgmtTable[PonPortIdx].DataPathMAC, PAS_Data_Mac, BYTES_IN_MAC_ADDRESS );
	PonChipMgmtTable[PonPortIdx].DataPathMAC[5] = PonPortIdx * 16;
	*/
	PonChipMgmtTable[PonPortIdx].operStatus = PONCHIP_NOTLOADING;
	PonChipActivatedFlag[CardSlot] = TRUE;
	
	return( ROK );
}

int   downloadPon( short int PonPortIdx)
{
	int ret;
    int CardSlot;
    int CardPort;

    CardSlot = GetCardIdxByPonChip(PonPortIdx);
    CardPort = GetPonPortByPonChip(PonPortIdx);
    
	InitPonChipInfo(PonPortIdx );

	Hardware_Reset_olt1(CardSlot, CardPort, 1, 1);
	Hardware_Reset_olt2(CardSlot, CardPort, 1, 1);

	if( ( ret = pon_add_olt ( PonPortIdx) ) == ROK )
	{

	}
	else
    {
		if(EVENT_PONADD == V2R1_ENABLE )
			sys_console_printf("   pon%d/%d added err \r\n", CardSlot, CardPort);
        
		PonChipMgmtTable[PonPortIdx].operStatus    =  PONCHIP_ERR;
		PonPortTable[PonPortIdx].PortWorkingStatus = PONPORT_DOWN;

		do
        {
		 	PonChipDownloadCounter[PonPortIdx]++;
            /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
			if(OLTAdv_IsExist(PonPortIdx) == TRUE )
				Pon_RemoveOlt( PonPortIdx, FALSE, FALSE);
			ClearPonRunningData( PonPortIdx );

        	Hardware_Reset_olt1(CardSlot, CardPort, 0, 1);
			VOS_TaskDelay(VOS_TICK_SECOND);
        	Hardware_Reset_olt2(CardSlot, CardPort, 1, 1);
			ret = pon_add_olt(PonPortIdx );	
		}while((ret != ROK ) && (PonChipDownloadCounter[PonPortIdx] < (PON_DOWNLOAD_MAX_COUNTER-1) ));
	}
    
	return(ROK);
}

int downloadPonByVty(short int PonPortIdx, struct vty *vty)
{
	InitPonChipInfo(PonPortIdx );
	Add_PonPort(PonPortIdx);
	return(ROK);
}

int RemovePonByVty(short int PonPortIdx, struct vty *vty)
{
	Del_PonPort(PonPortIdx);
	return(ROK);
}

#endif

#if 0
/* this command free the memory of statistics_data after printing */
short int Print_raw_statistics_buffers ( const short int			  olt_id, 
										 const short int			  collector_id,
										 const PON_raw_statistics_t   raw_statistics_type,
										 const short int			  statistics_parameter,
										       void				     *statistics_data,
										 const PON_timestamp_t		  timestamp,
										 struct vty					 *vty )
{
	char												return_buffer_header[250];
	char												collector_s[10];
	char												statistics_parameter_s[10];
/*	PON_errored_symbol_period_alarm_raw_data_t			errored_symbol_period_alarm_data;
	PON_errored_frame_alarm_raw_data_t					errored_frame_alarm_data;
	PON_errored_frame_period_alarm_raw_data_t			errored_frame_period_alarm_data;
	PON_errored_frame_seconds_summary_alarm_raw_data_t  errored_frame_second_alarm_data;
	long double											errored_symbol_window_for_print;
	long double											errored_symbol_threshold_for_print;
	long double											errored_symbols_for_print;
	long double											error_running_total_for_print;
*/
	short int OnuIdx;

	if (collector_id == PON_OLT_ID)
	{
		strcpy (collector_s, "OLT");
	}
	else if (collector_id == PON_OLT_AND_ONU_COMBINED_ID)
	{
		strcpy (collector_s, "OLT & ONU");
	} else
	{ /* ONU */
		char onu_collector_s[10];
		
		OnuIdx = GetOnuIdxByLlid(olt_id, collector_id);
		sprintf (onu_collector_s,"ONU %d", OnuIdx);
		strcpy (collector_s, onu_collector_s);
	}

	if (statistics_parameter == PON_BROADCAST_LLID) /* Assumption: statistics parameter this high must be broadcast LLID. There may be special statistics types where this assumption is not valid */
		sprintf (statistics_parameter_s, "Broadcast");
	else
		sprintf (statistics_parameter_s, "%d", statistics_parameter);

		sprintf (return_buffer_header, "show statistics%sOLT id              : %d%sCollector id        : %s%sStatistics type     : %s%sStatistics parameter: %s%sTimestamp           : %u%sData:",
				 VTY_NEWLINE,
				 olt_id,
				 VTY_NEWLINE,
				 collector_s,
				 VTY_NEWLINE,
				 PON_raw_statistics_s[raw_statistics_type],
				 VTY_NEWLINE,
				 statistics_parameter_s,
 				 VTY_NEWLINE,
                 timestamp,
				 VTY_NEWLINE);

	switch (raw_statistics_type)
	{
	case PON_RAW_STAT_ONU_BER:
		{
			PON_onu_ber_raw_data_t *data = (PON_onu_ber_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER
			WRITE_ULONG_PARAMETER                 (data->error_bytes    ,error_bytes      );
			WRITE_LONG_DOUBLE_PARAMETER           (data->used_byte_count,used_byte_count  );
			WRITE_LONG_DOUBLE_PARAMETER           (data->good_byte_count,good_byte_count  );
			break;
		}
	case PON_RAW_STAT_OLT_BER:
		{
            PON_olt_ber_raw_data_t *data = (PON_olt_ber_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER                 (data->error_bytes         ,error_bytes          );  
            WRITE_LONG_DOUBLE_PARAMETER           (data->byte_count          ,byte_count           );  
            WRITE_ULONG_PARAMETER                 (data->error_onu_bytes     ,error_onu_bytes      );  
            WRITE_LONG_DOUBLE_PARAMETER           (data->onu_byte_count      ,onu_byte_count       );  
            WRITE_LONG_DOUBLE_PARAMETER           (data->onu_total_byte_count,onu_total_byte_count ); 		
			break;
		}
	case PON_RAW_STAT_ONU_FER:
        {
            PON_onu_fer_raw_data_t *data = (PON_onu_fer_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER                 (data->received_error         ,received_error        );  
            WRITE_ULONG_PARAMETER                 (data->received_ok            ,received_ok           );  
            WRITE_ULONG_PARAMETER                 (data->firmware_received_ok   ,firmware_received_ok  );  
 			break;
        }
	case PON_RAW_STAT_LLID_BROADCAST_FRAMES:
        {
            PON_llid_Broadcast_frames_raw_data_t *data = (PON_llid_Broadcast_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER                 (data->llid_frames_received                ,llid_frames_received                );  
            WRITE_ULONG_PARAMETER                 (data->broadcast_frames_received           ,broadcast_frames_received           );  
            WRITE_ULONG_PARAMETER                 (data->invalid_llid_error_frames_received  ,invalid_llid_error_frames_received  );  
            WRITE_ULONG_PARAMETER                 (data->llid_broadcast_error_frames_received,llid_broadcast_error_frames_received);  
			break;
        }
	case PON_RAW_STAT_TOTAL_FRAMES:
        {
            PON_total_frames_raw_data_t *data = (PON_total_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER                 (data->received_ok         ,received_ok         );  
            WRITE_ULONG_PARAMETER                 (data->received_error      ,received_error      );  
            WRITE_ULONG_PARAMETER                 (data->transmitted_ok      ,transmitted_ok      );  
            WRITE_ULONG_PARAMETER                 (data->total_bad           ,total_bad           );
			break;
		}
	case PON_RAW_STAT_TOTAL_OCTETS:
        {
            PON_total_octets_raw_data_t *data = (PON_total_octets_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER
            WRITE_LONG_DOUBLE_PARAMETER              (data->received_ok   ,received_ok   );  
            WRITE_LONG_DOUBLE_PARAMETER              (data->received_error,received_error);  
            WRITE_LONG_DOUBLE_PARAMETER              (data->transmitted_ok,transmitted_ok);  
            WRITE_LONG_DOUBLE_PARAMETER              (data->received_total,received_total);	
			break;
		}
	case PON_RAW_STAT_TRANSMITTED_BYTES_PER_LLID:
        {
            PON_transmitted_bytes_per_llid_raw_data_t *data = (PON_transmitted_bytes_per_llid_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER
            WRITE_LONG_DOUBLE_PARAMETER   (data->transmitted_ok   ,transmitted_ok   );  	
			break;
		}
	case PON_RAW_STAT_TRANSMITTED_FRAMES_PER_PRIORITY:
		{
            PON_transmitted_frames_per_priority_raw_data_t *data = (PON_transmitted_frames_per_priority_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->transmitted_ok[0]   ,transmitted_ok[0]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[1]   ,transmitted_ok[1]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[2]   ,transmitted_ok[2]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[3]   ,transmitted_ok[3]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[4]   ,transmitted_ok[4]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[5]   ,transmitted_ok[5]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[6]   ,transmitted_ok[6]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[7]   ,transmitted_ok[7]   );  
			break;
		}
	case PON_RAW_STAT_TRANSMITTED_FRAMES_PER_LLID:
		{
            PON_transmitted_frames_per_llid_raw_data_t *data = (PON_transmitted_frames_per_llid_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->pon_ok      ,pon_ok   );  
            WRITE_ULONG_PARAMETER (data->system_ok   ,system_ok);  
			break;
		}
	case PON_RAW_STAT_DROPPED_FRAMES:
       {
            PON_dropped_frames_raw_data_t *data = (PON_dropped_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->dropped[0]   ,dropped[0]   );  
            WRITE_ULONG_PARAMETER (data->dropped[1]   ,dropped[1]   );  
            WRITE_ULONG_PARAMETER (data->dropped[2]   ,dropped[2]   );  
            WRITE_ULONG_PARAMETER (data->dropped[3]   ,dropped[3]   );  
            WRITE_ULONG_PARAMETER (data->dropped[4]   ,dropped[4]   );  
            WRITE_ULONG_PARAMETER (data->dropped[5]   ,dropped[5]   );  
            WRITE_ULONG_PARAMETER (data->dropped[6]   ,dropped[6]   );  
            WRITE_ULONG_PARAMETER (data->dropped[7]   ,dropped[7]   );  
			break;
		}
	case PON_RAW_STAT_DROPPED_FRAMES_PER_LLID:
       {
            PON_dropped_frames_per_llid_raw_data_t *data = (PON_dropped_frames_per_llid_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->policer_dropped ,policer_dropped );  
            WRITE_ULONG_PARAMETER (data->mismatch_dropped,mismatch_dropped);  
			break;
	   }
	case PON_RAW_STAT_TOTAL_DROPPED_RX_FRAMES:
		{
            PON_total_dropped_rx_frames_raw_data_t *data = (PON_total_dropped_rx_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->total_control_dropped,total_control_dropped);  
            WRITE_ULONG_PARAMETER (data->source_alert_dropped ,source_alert_dropped );  
            WRITE_ULONG_PARAMETER (data->pon_match            ,pon_match            );  
			break;
		}
	case PON_RAW_STAT_TOTAL_DROPPED_TX_FRAMES:
		{	
            PON_total_tx_dropped_frames_raw_data_t *data = (PON_total_tx_dropped_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER                  (data->total_pon_dropped         ,total_pon_dropped       );  
            WRITE_ULONG_PARAMETER                  (data->total_system_dropped      ,total_system_dropped    );  
			break;
        }
	case PON_RAW_STAT_SINGLE_COLLISION_FRAMES:
		{	
            PON_single_collision_frames_raw_data_t *data = (PON_single_collision_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->single_collision_frames   ,single_collision_frames   );  
			break;
		}
	case PON_RAW_STAT_MULTIPLE_COLLISION_FRAMES:
		{
            PON_multiple_collision_frames_raw_data_t *data = (PON_multiple_collision_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->multiple_collision_frames   ,multiple_collision_frames   );  
			break;
		}
	case PON_RAW_STAT_FRAME_CHECK_SEQUENCE_ERRORS:
		{
            PON_frame_check_sequence_errors_raw_data_t *data = (PON_frame_check_sequence_errors_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
		    WRITE_ULONG_PARAMETER (data->received   ,received   );  
			break;
		}
	case PON_RAW_STAT_ALIGNMENT_ERRORS:
		{
            PON_alignmnet_errors_raw_data_t *data = (PON_alignmnet_errors_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received   ,received   );  
			break;
		}
	case PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS_PER_LLID:
		{
            PON_in_range_length_errors_per_llid_raw_data_t *data = (PON_in_range_length_errors_per_llid_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received   ,received   );  
			break;
		}
	case PON_RAW_STAT_IN_RANGE_LENGTH_ERRORS:
		{
            PON_in_range_length_errors_raw_data_t *data = (PON_in_range_length_errors_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->system_received   ,system_received   );  
			break;
		}
	case PON_RAW_STAT_FRAME_TOO_LONG_ERRORS:
		{
            PON_frame_too_long_errors_raw_data_t *data = (PON_frame_too_long_errors_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
			if (vty->node == PAS_CNI_NODE || vty->node == PAS_PAS5201_CNI_NODE)
			{
                WRITE_ULONG_PARAMETER (data->system_received,system_received);  
			}
			else
			{
                WRITE_ULONG_PARAMETER (data->pon_received   ,pon_received   );  
                WRITE_ULONG_PARAMETER (data->system_received,system_received);  
			}
			break;
		}
	case PON_RAW_STAT_FRAME_TOO_LONG_ERRORS_PER_LLID:
		{
            PON_frame_too_long_errors_per_llid_raw_data_t *data = (PON_frame_too_long_errors_per_llid_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received   ,received   );  
			break;
		}
	case PON_RAW_STAT_HEC_FRAMES_ERRORS:
		{
            PON_hec_frames_errors_raw_data_t *data = (PON_hec_frames_errors_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received   ,received   );  
			break;
		}
	case PON_RAW_STAT_UNSUPPORTED_MPCP_FRAMES:
		{
            PON_unsupported_mpcp_frames_raw_data_t *data = (PON_unsupported_mpcp_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received   ,received   );  
			break;
		}
	case PON_RAW_STAT_HOST_FRAMES:
		{
            PON_host_frames_raw_data_t *data = (PON_host_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->pon_received_ok      ,PON received ok      );  
            WRITE_ULONG_PARAMETER (data->system_received_ok   ,System received ok   );  
            WRITE_ULONG_PARAMETER (data->pon_transmitted_ok   ,PON transmitted ok   );  
            WRITE_ULONG_PARAMETER (data->system_transmitted_ok,System transmitted ok);  
            WRITE_ULONG_PARAMETER (data->pon_dropped          ,PON dropped          );  
            WRITE_ULONG_PARAMETER (data->system_dropped       ,System dropped       );  
			break;
		}
	case PON_RAW_STAT_HOST_OCTETS:
        {
            PON_host_octets_raw_data_t *data = (PON_host_octets_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received_ok   ,received_ok   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok,transmitted_ok);  
  			break;
		}
	case PON_RAW_STAT_HOST_MESSAGES:
        {
            PON_host_messages_raw_data_t *data = (PON_host_messages_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
		    WRITE_ULONG_PARAMETER (data->received_from_firmware_ok   ,received_from_firmware_ok   );  
            WRITE_ULONG_PARAMETER (data->sent_to_firmware_ok         ,sent_to_firmware_ok         );  
			break;
		}
	case PON_RAW_STAT_PAUSE_FRAMES:
        {
            PON_pause_frames_raw_data_t *data = (PON_pause_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->pon_received_ok      ,pon_received_ok      );  
            WRITE_ULONG_PARAMETER (data->system_received_ok   ,system_received_ok   );  
            WRITE_ULONG_PARAMETER (data->pon_transmitted_ok   ,pon_transmitted_ok   );  
            WRITE_ULONG_PARAMETER (data->system_transmitted_ok,system_transmitted_ok);  
			break;	
		}
	case PON_RAW_STAT_PAUSE_TIME:
        {
            PON_pause_time_raw_data_t *data = (PON_pause_time_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->system_port   ,system_port   );  
			break;
		}
	case PON_RAW_STAT_REGISTRATION_FRAMES:
        {
            PON_registration_frames_raw_data_t *data = (PON_registration_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->register_request_transmitted_ok,register_request_transmitted_ok);  
            WRITE_ULONG_PARAMETER (data->register_received_ok           ,register_received_ok           );  
            WRITE_ULONG_PARAMETER (data->register_ack_transmitted_ok    ,register_ack_transmitted_ok    );  
			break;
		}
	case PON_RAW_STAT_OAM_FRAMES:
        {
            PON_oam_frames_raw_data_t *data = (PON_oam_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
		    WRITE_ULONG_PARAMETER (data->received_ok   ,received_ok   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok,transmitted_ok);  
			break; 
		}
	case PON_RAW_STAT_GRANT_FRAMES:
        {
            PON_grant_frames_raw_data_t *data = (PON_grant_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received_ok        ,received_ok        );  
            WRITE_ULONG_PARAMETER (data->transmitted_dba_ok ,transmitted_dba_ok );  
            WRITE_ULONG_PARAMETER (data->transmitted_ctrl_ok,transmitted_ctrl_ok);  
			break; 
		}
	case PON_RAW_STAT_REPORT_FRAMES:
        {
            PON_report_frames_raw_data_t *data = (PON_report_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received_ok   ,received_ok   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok,transmitted_ok);  
			break; 
		}
	case PON_RAW_STAT_MULTICAST_FRAMES:
        {
            PON_multicast_frames_raw_data_t *data = (PON_multicast_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received_ok   ,received_ok   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok,transmitted_ok);  
			break;
		}
	case PON_RAW_STAT_BROADCAST_FRAMES:
		{
            PON_broadcast_frames_raw_data_t *data = (PON_broadcast_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received_ok   ,received_ok   ); 
            WRITE_ULONG_PARAMETER (data->transmitted_ok,transmitted_ok);  
			break;
		}
	case PON_RAW_STAT_P2P_FRAMES:
		{
            PON_p2p_frames_raw_data_t *data = (PON_p2p_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received_ok                  ,received_ok                  );  
            WRITE_ULONG_PARAMETER (data->dropped_by_policer           ,dropped_by_policer           );  
            WRITE_ULONG_PARAMETER (data->dropped_by_access_control    ,dropped_by_access_control    );  
            WRITE_ULONG_PARAMETER (data->dropped_due_to_tx_buffer_full,dropped_due_to_tx_buffer_full); 
			WRITE_ULONG_PARAMETER (data->transmitted_ok               ,transmitted_ok               );  
			break;
		}
	case PON_RAW_STAT_BRIDGE_FRAMES:
		{
            PON_bridge_frames_raw_data_t *data = (PON_bridge_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->received   ,received   );  
			break;
		}
	case PON_RAW_STAT_PROMISCUOUS_STATUS:
		{
            PON_promiscuous_status_raw_data_t *data = (PON_promiscuous_status_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ENABLE_DISABLE_PARAMETER (data->status   ,Promiscuos status);  
			break;
		}
	case PON_RAW_STAT_DUPLEX_STATUS:
		{
            PON_duplex_status_raw_data_t *data = (PON_duplex_status_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_DUPLEX_PARAMETER  (data->status          ,status          );  
            WRITE_DUPLEX_PARAMETER  (data->mac_capabilities,mac_capabilities);  
			break;
		}
	case PON_RAW_STAT_RTT:
		{
            PON_rtt_raw_data_t *data = (PON_rtt_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
		    WRITE_ULONG_PARAMETER (data->rtt   ,rtt   );  
			break;
		}
	case PON_RAW_STAT_RATE_CONTROL_ABILITY:
		{
            PON_rate_control_ability_raw_data_t *data = (PON_rate_control_ability_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_TRUE_FALSE_PARAMETER (data->rate_control_ability   ,rate_control_ability   );  
			break;
		}
	case PON_RAW_STAT_MPCP_STATUS:
		{
            PON_mpcp_status_raw_data_t *data = (PON_mpcp_status_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ENABLE_DISABLE_PARAMETER  (data->admin_state                ,admin_state                 );  
            WRITE_MPCP_MODE_PARAMETER       (data->mode                       ,mode                        );  
            WRITE_MPCP_STATUS_PARAMETER     (data->registration_state         ,registration_state          );  
            WRITE_ULONG_PARAMETER           (data->unsupported                ,unsupported                 );  
            WRITE_ULONG_PARAMETER           (data->mac_ctrl_frames_transmitted,mac_ctrl_frames_transmitted );  
            WRITE_ULONG_PARAMETER           (data->mac_ctrl_frames_received   ,mac_ctrl_frames_received    );  
            WRITE_ULONG_PARAMETER           (data->maximum_pending_grants     ,maximum_pending_grants      );  
			break;
		}
	case PON_RAW_STAT_OAM_STATUS:
		{
            PON_oam_status_raw_data_t *data = (PON_oam_status_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER           (data->id          ,id         );  
            WRITE_ENABLE_DISABLE_PARAMETER  (data->admin_state ,admin_state);  
            WRITE_OAM_STATUS_PARAMETER      (data->mode        ,mode       );  
			break;
		}
	case PON_RAW_STAT_OAM_INFORMATION_DATA:
		{
            PON_oam_information_data_raw_data_t *data = (PON_oam_information_data_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_MAC_PARAMETER (data->oam_remote_mac_adress   ,oam_remote_mac_adress  );  
 			print_oam_2_0_configuration(vty, &(((PON_oam_information_data_raw_data_t *)statistics_data)->oam_remote_configuration));
			pas_vty_out(vty,"Remote_pdu_configuration: %d%s",((PON_oam_information_data_raw_data_t *)statistics_data)->oam_remote_pdu_configuration,VTY_NEWLINE);
			pas_vty_out(vty,"Local flag fields:%s",VTY_NEWLINE);
			print_oam_2_0_flag_field(vty, &(((PON_oam_information_data_raw_data_t *)statistics_data)->oam_local_flags_field));
			pas_vty_out(vty,"Remote flag fields:%s",VTY_NEWLINE);
			print_oam_2_0_flag_field(vty, &(((PON_oam_information_data_raw_data_t *)statistics_data)->oam_remote_flags_field));
			pas_vty_out (vty,"OAM 2.0 State: %s",VTY_NEWLINE);

            WRITE_OAM_MULTIPLEXER_ACTION_PARAMETER(data->oam_remote_state.multiplexer_action,oam_remote_state.multiplexer_action);
            WRITE_OAM_PARSER_ACTION_PARAMETER     (data->oam_remote_state.parser_action     ,oam_remote_state.parser_action     );
            WRITE_LONG_HEX_PARAMETER              (data->oam_remote_vendor_oui              ,oam_remote_vendor_oui              );
            WRITE_LONG_HEX_PARAMETER              (data->oam_remote_vendor_id_device_number ,oam_remote_vendor_id_device_number );
            WRITE_LONG_HEX_PARAMETER              (data->oam_remote_vendor_id_version       ,oam_remote_vendor_id_version       );
			break;
		}
	case PON_RAW_STAT_OAM_FRAMES_COUNTERS:
		{
			PON_oam_frames_counters_raw_data_t *data = (PON_oam_frames_counters_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER                                                   
            WRITE_ULONG_PARAMETER (data->unsupported_codes_rx           ,unsupported_codes_rx           );
            WRITE_ULONG_PARAMETER (data->information_tx                 ,information_tx                 );
            WRITE_ULONG_PARAMETER (data->information_rx                 ,information_rx                 );
            WRITE_ULONG_PARAMETER (data->event_notification_tx          ,event_notification_tx          );
            WRITE_ULONG_PARAMETER (data->unike_event_notification_rx    ,unike_event_notification_rx    );
            WRITE_ULONG_PARAMETER (data->duplicate_event_notification_rx,duplicate_event_notification_rx);  
            WRITE_ULONG_PARAMETER (data->oam_loopback_control_tx        ,oam_loopback_control_tx        );  
            WRITE_ULONG_PARAMETER (data->oam_loopback_control_rx        ,oam_loopback_control_rx        );  
            WRITE_ULONG_PARAMETER (data->oam_variable_request_tx        ,oam_variable_request_tx        );  
            WRITE_ULONG_PARAMETER (data->oam_variable_request_rx        ,oam_variable_request_rx        ); 
            WRITE_ULONG_PARAMETER (data->oam_variable_response_tx       ,oam_variable_response_tx       );
            WRITE_ULONG_PARAMETER (data->oam_variable_response_rx       ,oam_variable_response_rx       );
            WRITE_ULONG_PARAMETER (data->oam_organization_specific_tx   ,oam_organization_specific_tx   );
            WRITE_ULONG_PARAMETER (data->oam_organization_specific_rx   ,oam_organization_specific_rx   );
            WRITE_ULONG_PARAMETER (data->frames_lost_due_to_oam_error   ,frames_lost_due_to_oam_error   );
			break;
		}
	case PON_RAW_STAT_ERRORED_SYMBOL_PERIOD_ALARM:
		{
			PON_errored_symbol_period_alarm_raw_data_t *data = (PON_errored_symbol_period_alarm_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64 (data->errored_symbol_window     ,errored_symbol_window    );
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64 (data->errored_symbol_threshold  ,errored_symbol_threshold );
            WRITE_ULONG_PARAMETER                  (data->timestamp                 ,timestamp                );
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64 (data->errored_symbols           ,errored_symbols          );
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64 (data->error_running_total       ,error_running_total      );
            WRITE_ULONG_PARAMETER                  (data->event_running_total       ,event_running_total      );
			break;
		}
	case PON_RAW_STAT_ERRORED_FRAME_ALARM:
		{
            PON_errored_frame_alarm_raw_data_t *data = (PON_errored_frame_alarm_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER                  (data->errored_frame_window      ,errored_frame_window    );
            WRITE_ULONG_PARAMETER                  (data->errored_frame_threshold   ,errored_frame_threshold );
            WRITE_ULONG_PARAMETER                  (data->timestamp                 ,timestamp                );
            WRITE_ULONG_PARAMETER                  (data->errored_frames            ,errored_frames           );
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64 (data->error_running_total       ,error_running_total      );
            WRITE_ULONG_PARAMETER                  (data->event_running_total       ,event_running_total      );
 			break;
		}
	case PON_RAW_STAT_ERRORED_FRAME_PERIOD_ALARM:
        {
            PON_errored_frame_period_alarm_raw_data_t *data = (PON_errored_frame_period_alarm_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER                  (data->errored_frame_window      ,errored_frame_window    );
            WRITE_ULONG_PARAMETER                  (data->errored_frame_threshold   ,errored_frame_threshold );
            WRITE_ULONG_PARAMETER                  (data->timestamp                 ,timestamp                );
            WRITE_ULONG_PARAMETER                  (data->errored_frames            ,errored_frames           );
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64 (data->error_running_total       ,error_running_total      );
            WRITE_ULONG_PARAMETER                  (data->event_running_total       ,event_running_total      );
			break;
		}
	case PON_RAW_STAT_ERRORED_FRAME_SECONDS_SUMMARY_ALARM:
		{
			PON_errored_frame_seconds_summary_alarm_raw_data_t *data = (PON_errored_frame_seconds_summary_alarm_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER                  (data->errored_frame_seconds_summary_window      ,errored_frame_seconds_summary_window    );
            WRITE_ULONG_PARAMETER                  (data->errored_frame_seconds_summary_threshold   ,errored_frame_seconds_summary_threshold );
            WRITE_ULONG_PARAMETER                  (data->timestamp                                 ,timestamp                               );
            WRITE_ULONG_PARAMETER                  (data->errored_frame_seconds_summary             ,errored_frame_seconds_summary           );
            WRITE_ULONG_PARAMETER                  (data->error_running_total                       ,error_running_total                     );
            WRITE_ULONG_PARAMETER                  (data->event_running_total                       ,event_running_total                     );
			break;
		}
    case PON_RAW_STAT_STANDARD_OAM_STATUS                     :
        {
            PON_oam_3_3_status_raw_data_t *data = (PON_oam_3_3_status_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
               WRITE_ULONG_PARAMETER(data->id,OAM id)
                WRITE_ENABLE_DISABLE_PARAMETER(data->admin_state,Admin state);
                WRITE_DISCOVERY_STATE_PARAMETER(data->discovery_state,Discovery_state);
                WRITE_TRUE_FALSE_PARAMETER(data->local_flags.oam_3_3_critical_event    ,Local flags - critical event    );
                WRITE_TRUE_FALSE_PARAMETER(data->local_flags.oam_3_3_remote_evaluating ,Local flags - Remote evaluating );
                WRITE_TRUE_FALSE_PARAMETER(data->local_flags.oam_3_3_remote_stable     ,Local flags - Remote stable     );
                WRITE_TRUE_FALSE_PARAMETER(data->local_flags.oam_3_3_local_stable      ,Local flags - Local stable      );
                WRITE_TRUE_FALSE_PARAMETER(data->local_flags.oam_3_3_local_evaluating  ,Local flags - Local evaluating  );
                WRITE_TRUE_FALSE_PARAMETER(data->local_flags.oam_3_3_dying_gasp        ,Local flags - Dying gasp        );
                WRITE_TRUE_FALSE_PARAMETER(data->local_flags.oam_3_3_link_fault        ,Local flags - Link fault        );
                
                WRITE_TRUE_FALSE_PARAMETER(data->remote_flags.oam_3_3_critical_event    ,Remote flags - critical event    ); 
                WRITE_TRUE_FALSE_PARAMETER(data->remote_flags.oam_3_3_remote_evaluating ,Remote flags - Remote evaluating ); 
                WRITE_TRUE_FALSE_PARAMETER(data->remote_flags.oam_3_3_remote_stable     ,Remote flags - Remote stable     ); 
                WRITE_TRUE_FALSE_PARAMETER(data->remote_flags.oam_3_3_local_stable      ,Remote flags - Local stable      ); 
                WRITE_TRUE_FALSE_PARAMETER(data->remote_flags.oam_3_3_local_evaluating  ,Remote flags - Local evaluating  ); 
                WRITE_TRUE_FALSE_PARAMETER(data->remote_flags.oam_3_3_dying_gasp        ,Remote flags - Dying gasp        ); 
                WRITE_TRUE_FALSE_PARAMETER(data->remote_flags.oam_3_3_link_fault        ,Remote flags - Link fault        );; 

                WRITE_OAM_STATUS_PARAMETER(data->mode,status mode);

                WRITE_USHORT_PARAMETER(data->local_pdu_configuration,Local pdu configuration);
                WRITE_ULONG_PARAMETER (data->local_revision,Local revision);
                
                WRITE_SUPPORTED_MODE_PARAMETER(data->local_configuration.variable_retrieval    ,Local configuration - Variable_retrieval     );
                WRITE_SUPPORTED_MODE_PARAMETER(data->local_configuration.link_events           ,Local configuration - Link events            );
                WRITE_SUPPORTED_MODE_PARAMETER(data->local_configuration.loopback_support      ,Local configuration - Loopback support       );
                WRITE_SUPPORTED_MODE_PARAMETER(data->local_configuration.unidirectional_support,Local configuration - Unidirectional support );
                WRITE_OAM_MODE_PARAMETER      (data->local_configuration.oam_mode              ,Local configuration - Oam mode               );

		    }
        break;
    case PON_RAW_STAT_STANDARD_OAM_PEER_STATUS                :
        {
            PON_oam_peer_status_raw_data_t *data = (PON_oam_peer_status_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_DISCOVERY_STATE_PARAMETER(data->discovery_state,Discovery_state);
            WRITE_MAC_PARAMETER(data->mac_address         ,MAC address);;
            WRITE_LONG_HEX_PARAMETER(data->vendor_oui        ,Vendor OUI);
            WRITE_LONG_HEX_PARAMETER(data->vendor_identifier,Vendor identifier);
            WRITE_SUPPORTED_MODE_PARAMETER(data->configuration.variable_retrieval    ,configuration - Variable_retrieval     );
            WRITE_SUPPORTED_MODE_PARAMETER(data->configuration.link_events           ,configuration - Link events            );
            WRITE_SUPPORTED_MODE_PARAMETER(data->configuration.loopback_support      ,configuration - Loopback support       );
            WRITE_SUPPORTED_MODE_PARAMETER(data->configuration.unidirectional_support,configuration - Unidirectional support );
            WRITE_OAM_MODE_PARAMETER      (data->configuration.oam_mode              ,configuration - Oam mode               );
            WRITE_USHORT_PARAMETER(data->pdu_configuration,pdu configuration);
            WRITE_ULONG_PARAMETER (data->revision,Revision);
        }
        break;
    case PON_RAW_STAT_STANDARD_OAM_LOOPBACK_STATUS            :
        {
            PON_oam_loopback_status_data_t *data = (PON_oam_loopback_status_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_OAM_MULTIPLEXER_ACTION_PARAMETER(data->oam_local_state.multiplexer_action ,Local state  - multiplexer action );
            WRITE_OAM_PARSER_ACTION_PARAMETER     (data->oam_local_state.parser_action      ,Local state  - parser action      );
            WRITE_OAM_MULTIPLEXER_ACTION_PARAMETER(data->oam_remote_state.multiplexer_action,Remote state - multiplexer action );
            WRITE_OAM_PARSER_ACTION_PARAMETER     (data->oam_remote_state.parser_action     ,Remote state - parser action      );
        }
        break;
    case PON_RAW_STAT_STANDARD_OAM_STATISTIC                  :
        {
            PON_oam_statistic_raw_data_t *data = (PON_oam_statistic_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER(data->information_tx					 , information_tx				   );
            WRITE_ULONG_PARAMETER(data->information_rx					 , information_rx				   );
            WRITE_ULONG_PARAMETER(data->unique_event_notification_tx	 , unique_event_notification_tx	   );
            WRITE_ULONG_PARAMETER(data->unique_event_notification_rx	 , unique_event_notification_rx	   );
            WRITE_ULONG_PARAMETER(data->duplicate_event_notification_tx	 , duplicate_event_notification_tx );
            WRITE_ULONG_PARAMETER(data->duplicate_event_notification_rx	 , duplicate_event_notification_rx );
            WRITE_ULONG_PARAMETER(data->oam_loopback_control_tx			 , oam_loopback_control_tx		   );
            WRITE_ULONG_PARAMETER(data->oam_loopback_control_rx			 , oam_loopback_control_rx		   );
            WRITE_ULONG_PARAMETER(data->oam_variable_request_tx			 , oam_variable_request_tx		   );
            WRITE_ULONG_PARAMETER(data->oam_variable_request_rx			 , oam_variable_request_rx		   );
            WRITE_ULONG_PARAMETER(data->oam_variable_response_tx		 , oam_variable_response_tx		   );
            WRITE_ULONG_PARAMETER(data->oam_variable_response_rx		 , oam_variable_response_rx		   );
            WRITE_ULONG_PARAMETER(data->oam_organization_specific_tx	 , oam_organization_specific_tx	   );
            WRITE_ULONG_PARAMETER(data->oam_organization_specific_rx	 , oam_organization_specific_rx	   );
            WRITE_ULONG_PARAMETER(data->unsupported_codes_tx             , unsupported_codes_tx            );
            WRITE_ULONG_PARAMETER(data->frames_lost_due_to_oam_error	 , frames_lost_due_to_oam_error	   );
        }
        break;
    case PON_RAW_STAT_STANDARD_OAM_EVENT_CONFIG               :
        {
            PON_oam_event_config_raw_data_t *data = (PON_oam_event_config_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64(data->errored_symbol_period_event.errored_symbol_window   , Errored symbol period event - errored symbol window    );
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64(data->errored_symbol_period_event.errored_symbol_threshold, Errored symbol period event - errored symbol threshold );
            WRITE_ULONG_PARAMETER(data->errored_frame_period_event.errored_frame_window    , Errored frame period event - errored frame window    );
            WRITE_ULONG_PARAMETER(data->errored_frame_period_event.errored_frame_threshold , Errored frame period event - errored frame threshold );
            WRITE_USHORT_PARAMETER(data->errored_frame_event.errored_frame_window    , Errored frame event - errored frame window    ); 
            WRITE_ULONG_PARAMETER (data->errored_frame_event.errored_frame_threshold , Errored frame event - errored frame threshold ); 
            WRITE_USHORT_PARAMETER(data->errored_frame_seconds_summary.errored_frame_seconds_summary_window    , Errored frame seconds summary - window    ); 
            WRITE_USHORT_PARAMETER(data->errored_frame_seconds_summary.errored_frame_seconds_summary_threshold , Errored frame seconds summary - threshold ); 
        }
        break;
    case PON_RAW_STAT_STANDARD_OAM_LOCAL_EVENT_LOG            :
    case PON_RAW_STAT_STANDARD_OAM_REMOTE_EVENT_LOG           :
        {
            pon_oam_event_log_raw_data_t *data = (pon_oam_event_log_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_USHORT_PARAMETER                (data->errored_frame_event.timestamp               , Errored frame event- timestamp              );
            WRITE_USHORT_PARAMETER                (data->errored_frame_event.errored_frame_window    , Errored frame event- errored_frame_window   );
            WRITE_ULONG_PARAMETER                 (data->errored_frame_event.errored_frame_threshold , Errored frame event- errored_frame_threshold);
            WRITE_ULONG_PARAMETER                 (data->errored_frame_event.errored_frames		     , Errored frame event- errored_frames		   );
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64(data->errored_frame_event.error_running_total	 , Errored frame event- error_running_total	   );
            WRITE_ULONG_PARAMETER                 (data->errored_frame_event.event_running_total	 , Errored frame event- event_running_total	   );
            WRITE_USHORT_PARAMETER                (data->errored_frame_period_event.timestamp               , Errored frame period event - timestamp              );
            WRITE_ULONG_PARAMETER                 (data->errored_frame_period_event.errored_frame_window    , Errored frame period event - errored_frame_window   );
            WRITE_ULONG_PARAMETER                 (data->errored_frame_period_event.errored_frame_threshold , Errored frame period event - errored_frame_threshold);
            WRITE_ULONG_PARAMETER                 (data->errored_frame_period_event.errored_frames		    , Errored frame period event - errored_frames		   );
	        WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64(data->errored_frame_period_event.error_running_total	    , Errored frame period event - error_running_total	   );
            WRITE_ULONG_PARAMETER                 (data->errored_frame_period_event.event_running_total	    , Errored frame period event - event_running_total	   );
            WRITE_USHORT_PARAMETER                (data->errored_frame_secs_summary_event.timestamp                               , Errored frame secs summary event - timestamp           );                   
            WRITE_USHORT_PARAMETER                (data->errored_frame_secs_summary_event.errored_frame_seconds_summary_window    , Errored frame secs summary_event - summary window      ); 
            WRITE_USHORT_PARAMETER                (data->errored_frame_secs_summary_event.errored_frame_seconds_summary_threshold , Errored frame secs summary_event - summary threshold   ); 
            WRITE_USHORT_PARAMETER                (data->errored_frame_secs_summary_event.errored_frame_seconds_summary			  , Errored frame secs summary_event - summary			   ); 
            WRITE_ULONG_PARAMETER                 (data->errored_frame_secs_summary_event.error_running_total					  , Errored frame secs summary_event - error running total );					 
            WRITE_ULONG_PARAMETER                 (data->errored_frame_secs_summary_event.event_running_total					  , Errored frame secs summary_event - event running total );					 
            WRITE_USHORT_PARAMETER                (data->errored_frame_symbol_period_event.timestamp                , Errored frame symbol period event - timestamp                 );
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64(data->errored_frame_symbol_period_event.errored_symbol_window	, Errored frame symbol period event - errored symbol window		);
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64(data->errored_frame_symbol_period_event.errored_symbol_threshold , Errored frame symbol period event - errored symbol threshold  );
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64(data->errored_frame_symbol_period_event.errored_symbols			, Errored frame symbol period event - errored symbols			);
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64(data->errored_frame_symbol_period_event.error_running_total		, Errored frame symbol period event - error running total		);
            WRITE_ULONG_PARAMETER                 (data->errored_frame_symbol_period_event.event_running_total		, Errored frame symbol period event - event running total		);
        }
        break;
    case PON_RAW_STAT_STANDARD_OAM_MPCP_STATUS                :
        {
            PON_oam_standard_mpcp_status_raw_data_t *data = (PON_oam_standard_mpcp_status_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
	        WRITE_ULONG_PARAMETER           (data->id                     , id                     );
	        WRITE_MPCP_MODE_PARAMETER       (data->mode				      , mode				   );
            WRITE_ULONG_PARAMETER           (data->link_id                , link id                );
            WRITE_MAC_PARAMETER             (data->remote_mac_address     , remote mac_address     );
            WRITE_MPCP_STATUS_PARAMETER     (data->registration_state	  , registration state	   );
            WRITE_ULONG_PARAMETER           (data->transmit_elapsed       , transmit elapsed       );
	        WRITE_ULONG_PARAMETER           (data->receive_elapsed        , receive elapsed        );
	        WRITE_USHORT_PARAMETER          (data->round_trip_time        , round trip time        );
            WRITE_USHORT_PARAMETER          (data->maximum_pending_grants , maximum pending grants );
            WRITE_ENABLE_DISABLE_PARAMETER  (data->admin_state		      , admin state		 	   );
            WRITE_LONG_HEX_PARAMETER        (data->on_time                , on_time                );
            WRITE_LONG_HEX_PARAMETER        (data->off_time               , off_time               );
            WRITE_LONG_HEX_PARAMETER        (data->sync_time              , sync_time              );
        }
        break;
    case PON_RAW_STAT_STANDARD_OAM_MPCP_STATISTIC             :
        {
            PON_mpcp_statistic_raw_data_t *data = (PON_mpcp_statistic_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER(data->mac_ctrl_frames_transmitted    , mac_ctrl frames transmitted    );
            WRITE_ULONG_PARAMETER(data->mac_ctrl_frames_received       , mac_ctrl frames received       );
            WRITE_ULONG_PARAMETER(data->discovery_windows_sent         , discovery windows sent         );
            WRITE_ULONG_PARAMETER(data->discovery_timeout              , discovery timeout              );
            WRITE_ULONG_PARAMETER(data->register_request_transmitted_ok, register request transmitted ok);
            WRITE_ULONG_PARAMETER(data->register_request_received_ok   , register request received ok   );
            WRITE_ULONG_PARAMETER(data->register_ack_transmitted_ok	   , register ack transmitted ok	);
            WRITE_ULONG_PARAMETER(data->register_ack_received_ok	   , register ack received ok	    );
            WRITE_ULONG_PARAMETER(data->transmitted_report	           , transmitted report	            );
            WRITE_ULONG_PARAMETER(data->received_report	               , received report	            );
            WRITE_ULONG_PARAMETER(data->transmited_gate                , transmited gate                );
            WRITE_ULONG_PARAMETER(data->receive_gate	               , receive gate	                );
            WRITE_ULONG_PARAMETER(data->register_transmited	           , register transmited	        );
            WRITE_ULONG_PARAMETER(data->register_received	           , register received	            );
        }
        break;
    case PON_RAW_STAT_STANDARD_OAM_OMP_EMULATION_STATUS       :
        {
            PON_omp_emulation_status_raw_data_t *data = (PON_omp_emulation_status_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER             (data->id   , id   );
            WRITE_OMP_EMULATION_TYPE_PARAMETER(data->type , type );
        }
        break;
    case PON_RAW_STAT_STANDARD_OAM_OMP_EMULATION_STATISTICS   :
        {
            PON_omp_emulation_statistics_raw_data_t *data = (PON_omp_emulation_statistics_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER(data->sld_errors	     ,sld errors	    ); 
            WRITE_ULONG_PARAMETER(data->crc8_errors	     ,crc8 errors	    ); 
            WRITE_ULONG_PARAMETER(data->bad_llid	     ,bad llid	        ); 
            WRITE_ULONG_PARAMETER(data->good_llid	     ,good llid	        ); 
            WRITE_ULONG_PARAMETER(data->onu_pon_cast_llid,onu pon cast llid ); 
            WRITE_ULONG_PARAMETER(data->olt_pon_cast_llid,olt pon cast llid ); 
        }
        break;
    case PON_RAW_STAT_STANDARD_OAM_MAU_STATUS                 :
        {
            PON_mau_status_raw_data_t *data = (PON_mau_status_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
	        WRITE_ULONG_PARAMETER       (data->pcs_coding_violation     , pcs coding_violation     );
            WRITE_FEC_ABILITY_PARAMETER (data->fec_ability              , fec ability              );
            WRITE_FEC_MODE_PARAMETER    (data->fec_mode                 , fec mode                 );
            WRITE_ULONG_PARAMETER       (data->fec_corrected_blocks     , fec corrected blocks     );
            WRITE_ULONG_PARAMETER       (data->fec_uncorrectable_blocks , fec uncorrectable blocks );
        }
        break;
    case PON_RAW_STAT_RECEIVED_FRAMES_TO_CPU_PER_PRIORITY:
       {
            PON_received_frames_to_cpu_per_priority_raw_data_t *data = (PON_received_frames_to_cpu_per_priority_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER (data->received_ok[0]   ,received_ok[0]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[1]   ,received_ok[1]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[2]   ,received_ok[2]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[3]   ,received_ok[3]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[4]   ,received_ok[4]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[5]   ,received_ok[5]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[6]   ,received_ok[6]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[7]   ,received_ok[7]   );  
			break;
		}
     case PON_RAW_STAT_TRANSMITTED_FRAMES_FROM_CPU_PER_PRIORITY:
       {
            PON_transmitted_frames_from_cpu_per_priority_raw_data_t *data = (PON_transmitted_frames_from_cpu_per_priority_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER (data->transmitted_ok[0]   ,transmitted_ok[0]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[1]   ,transmitted_ok[1]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[2]   ,transmitted_ok[2]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[3]   ,transmitted_ok[3]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[4]   ,transmitted_ok[4]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[5]   ,transmitted_ok[5]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[6]   ,transmitted_ok[6]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[7]   ,transmitted_ok[7]   );  
			break;
		}
   case PON_RAW_STAT_CPU_PORTS_OCTET:
	   {
            PON_cpu_ports_octets_raw_data_t *data = (PON_cpu_ports_octets_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER (data->to_cpu     ,to_cpu     );  
            WRITE_ULONG_PARAMETER (data->from_cpu   ,from_cpu   );  
			break;
		}
   case PON_RAW_STAT_CPU_PORTS_FRAMES:
	   {
            PON_cpu_ports_frames_raw_data_t *data = (PON_cpu_ports_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER (data->to_cpu     ,to_cpu     );  
            WRITE_ULONG_PARAMETER (data->from_cpu   ,from_cpu   );  
			break;
		}
    case PON_RAW_STAT_TOTAL_DROPPED_CPU_RX_FRAMES:
	   {
            PON_total_dropped_cpu_rx_frames_raw_data_t *data = (PON_total_dropped_cpu_rx_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER (data->total_pon_dropped     ,total_pon_dropped      );  
            WRITE_ULONG_PARAMETER (data->total_system_dropped  ,total_system_dropped   );  
			break;
		}
    case PON_RAW_STAT_ENCRYPT_FRAMES:
	   {
            PON_encrypt_frames_raw_data_t *data = (PON_encrypt_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER (data->encrypt ,encrypt );  
            WRITE_ULONG_PARAMETER (data->decrypt ,decrypt );  
			break;
	   }
    case PON_RAW_STAT_START_END_SYMBOL_FRAMES:
	   {
            PON_start_end_symbols_frames_raw_data_t *data = (PON_start_end_symbols_frames_raw_data_t*)statistics_data;
			PRINT_BUFFER_HEADER 
            WRITE_ULONG_PARAMETER (data->s_symbol ,s_symbol );  
            WRITE_ULONG_PARAMETER (data->t_symbol ,t_symbol );  
			break;
	   }

    case PON_RAW_STAT_HOST_MESSAGES_OCTETS                            : 
        {
            PON_host_messages_octets_raw_data_t *data = (PON_host_messages_octets_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->sent_to_firmware              ,sent_to_firmware              );  
            WRITE_ULONG_PARAMETER (data->received_with_error_from_host ,received_with_error_from_host );  
		    break;
        }
    case PON_RAW_STAT_DISCARDED_UNKNOWN_DESTINATION_ADDRESS           : 
        {
            PON_discarded_unknown_destination_address_raw_data_t *data = (PON_discarded_unknown_destination_address_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->frames ,frames );  
            WRITE_ULONG_PARAMETER (data->octets ,octets );  
		    break;
        }
    case PON_RAW_STAT_RECEIVED_FRAMES_PER_PRIORITY                    : 
        {
            PON_received_frames_per_priority_raw_data_t *data = (PON_received_frames_per_priority_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->received_ok[0]   ,received_ok[0]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[1]   ,received_ok[1]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[2]   ,received_ok[2]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[3]   ,received_ok[3]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[4]   ,received_ok[4]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[5]   ,received_ok[5]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[6]   ,received_ok[6]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[7]   ,received_ok[7]   );  
		    break;
        }
    case PON_RAW_STAT_RECEIVED_OCTETS_PER_PRIORITY                    : 
        {
            PON_received_octets_per_priority_raw_data_t *data = (PON_received_octets_per_priority_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->received_ok[0]   ,received_ok[0]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[1]   ,received_ok[1]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[2]   ,received_ok[2]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[3]   ,received_ok[3]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[4]   ,received_ok[4]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[5]   ,received_ok[5]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[6]   ,received_ok[6]   );  
            WRITE_ULONG_PARAMETER (data->received_ok[7]   ,received_ok[7]   );  
		    break;
        }
    case PON_RAW_STAT_TRANSMITTED_OCTETS_PER_PRIORITY                 : 
        {
            PON_transmitted_octets_per_priority_raw_data_t *data = (PON_transmitted_octets_per_priority_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->transmitted_ok[0]   ,transmitted_ok[0]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[1]   ,transmitted_ok[1]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[2]   ,transmitted_ok[2]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[3]   ,transmitted_ok[3]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[4]   ,transmitted_ok[4]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[5]   ,transmitted_ok[5]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[6]   ,transmitted_ok[6]   );  
            WRITE_ULONG_PARAMETER (data->transmitted_ok[7]   ,transmitted_ok[7]   );  
		    break;
        }
    case PON_RAW_STAT_UPLINK_VLAN_DISCARDED_FRAMES                    : 
        {
            PON_uplink_vlan_discarded_frames_raw_data_t *data = (PON_uplink_vlan_discarded_frames_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->tagged_frames	      ,tagged_frames	    );  
            WRITE_ULONG_PARAMETER (data->untagged_frames	  ,untagged_frames	    ); 
            WRITE_ULONG_PARAMETER (data->null_tagged_frames   ,null_tagged_frames   );  
            WRITE_ULONG_PARAMETER (data->nested_tagged_frames ,nested_tagged_frames ); 
            WRITE_ULONG_PARAMETER (data->discarded_frames	  ,discarded_frames	    );  
		    break;
        }
    case PON_RAW_STAT_UPLINK_VLAN_DISCARDED_OCTETS                    : 
        {
            PON_uplink_vlan_discarded_octets_raw_data_t *data = (PON_uplink_vlan_discarded_octets_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->tagged_octets	      ,tagged_octets	    );  
            WRITE_ULONG_PARAMETER (data->untagged_octets	  ,untagged_octets	    ); 
            WRITE_ULONG_PARAMETER (data->null_tagged_octets   ,null_tagged_octets   );  
            WRITE_ULONG_PARAMETER (data->nested_tagged_octets ,nested_tagged_octets ); 
            WRITE_ULONG_PARAMETER (data->discarded_octets	  ,discarded_octets	    );  
		    break;
        }
    case PON_RAW_STAT_DOWNLINK_VLAN_DISCARDED_FRAMES                  : 
        {
            PON_downlink_vlan_discarded_frames_raw_data_t *data = (PON_downlink_vlan_discarded_frames_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->nested_tagged_frames	        , nested_tagged_frames	       );  
            WRITE_ULONG_PARAMETER (data->destination_discarded_frames   , destination_discarded_frames );  
		    break;
        }
    case PON_RAW_STAT_DOWNLINK_VLAN_DISCARDED_OCTETS                  : 
        {
            PON_downlink_vlan_discarded_octets_raw_data_t *data = (PON_downlink_vlan_discarded_octets_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->nested_tagged_octets	        , nested_tagged_octets	       );  
            WRITE_ULONG_PARAMETER (data->destination_discarded_octets   , destination_discarded_octets );  
		    break;
        }
    case PON_RAW_STAT_FEC_FRAMES                                      : 
        {
            PON_fec_frames_raw_data_t *data = (PON_fec_frames_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->fixed_frames	            , fixed_frames	             );  
            WRITE_ULONG_PARAMETER (data->unfixed_frames	            , unfixed_frames	         );  
            WRITE_ULONG_PARAMETER (data->good_frames	      	    , good_frames	      	     );  
            WRITE_ULONG_PARAMETER (data->wrong_parity_number        , wrong_parity_number        );  
		    break;
        }
    case PON_RAW_STAT_FEC_BLOCKS_AND_OCTETS                           : 
        {
            PON_fec_blocks_and_octets_raw_data_t *data = (PON_fec_blocks_and_octets_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER                   (data->good_blocks                  , good_blocks                  );  
            WRITE_ULONG_PARAMETER                   (data->fixed_octets                 , fixed_octets                 );  
            WRITE_ULONG_PARAMETER                   (data->unfixed_octets               , unfixed_octets               );  
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64  (data->good_octets                  , good_octets                  );  
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64  (data->rx_overhead_octets           , rx_overhead_octets           );  
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64  (data->tx_overhead_octets           , tx_overhead_octets           );  
		    break;
        }
    case PON_RAW_STAT_TOTAL_DROPPED_RX_GOOD_FRAMES           :      
        {
            PON_total_dropped_rx_good_frames_raw_data_t *data = (PON_total_dropped_rx_good_frames_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->downlink_dropped    ,downlink_dropped    );  
            WRITE_ULONG_PARAMETER (data->uplink_dropped      ,uplink_dropped );  
		    break;
        }
    case PON_RAW_STAT_TOTAL_OCTETS_IN_DROPPED_RX_GOOD_FRAMES : 
        {
            PON_total_octets_in_dropped_rx_good_frames_raw_data_t *data = (PON_total_octets_in_dropped_rx_good_frames_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64 (data->downlink_dropped    ,downlink_dropped    );  
            WRITE_LONG_DOUBLE_PARAMETER_FROM_INT64 (data->uplink_dropped      ,uplink_dropped      );  
		    break;
        }
    case PON_RAW_STAT_EGRESS_DROPPED_FRAMES_PER_PRIORITY :
        {
            PON_egress_dropped_frames_per_priority_raw_data_t *data = (PON_egress_dropped_frames_per_priority_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->dropped[0]   ,dropped[0]   );  
            WRITE_ULONG_PARAMETER (data->dropped[1]   ,dropped[1]   );  
            WRITE_ULONG_PARAMETER (data->dropped[2]   ,dropped[2]   );  
            WRITE_ULONG_PARAMETER (data->dropped[3]   ,dropped[3]   );  
            WRITE_ULONG_PARAMETER (data->dropped[4]   ,dropped[4]   );  
            WRITE_ULONG_PARAMETER (data->dropped[5]   ,dropped[5]   );  
            WRITE_ULONG_PARAMETER (data->dropped[6]   ,dropped[6]   );  
            WRITE_ULONG_PARAMETER (data->dropped[7]   ,dropped[7]   );  
            break;
        }
    case PON_RAW_STAT_DOWNLINK_RECEIVED_FRAMES_OCTETS_PER_VID :
        {
            PON_downlink_received_frames_octets_per_vid_raw_data_t *data = (PON_downlink_received_frames_octets_per_vid_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->received_frames   ,received_frames   );  
            WRITE_ULONG_PARAMETER (data->received_octets   ,received_octets   );  
            break;
        }
    case PON_RAW_STAT_P2P_GLOBAL_FRAMES_DROPPED :
        {
            PON_p2p_global_frames_dropped_raw_data_t *data = (PON_p2p_global_frames_dropped_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->egress_dropped   ,egress_dropped   );  
            WRITE_ULONG_PARAMETER (data->ram_dropped      ,ram_dropped      );  
            break;
        }
    case PON_RAW_STAT_IPG :
        {
            PON_ipg_raw_data_t *data = (PON_ipg_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->short_ipg   ,short_ipg   );  
            break;
        }
    case PON_RAW_STAT_TRANSMITTED_GOOD_FRAMES_OCTETS_FROM_PQ :
        {
            PON_transmitted_good_frames_octets_from_pq_raw_data_t *data = (PON_transmitted_good_frames_octets_from_pq_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->uplink_pq_frames     ,uplink_pq_frames     );  
            WRITE_ULONG_PARAMETER (data->uplink_pq_octets     ,uplink_pq_octets     );  
            WRITE_ULONG_PARAMETER (data->downlink_pq_frames   ,downlink_pq_frames   );  
            WRITE_ULONG_PARAMETER (data->downlink_pq_octets   ,downlink_pq_octets   );  
            break;
        }
	case PON_RAW_STAT_UNICAST_MULTICAST_PON_FRAMES:
		{
            PON_unicast_multicast_pon_frames_raw_data_t *data = (PON_unicast_multicast_pon_frames_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_ULONG_PARAMETER (data->rx_multicast_frames     ,rx_multicast_frames );  
            WRITE_ULONG_PARAMETER (data->tx_multicast_frames     ,tx_multicast_frames );  
            WRITE_ULONG_PARAMETER (data->rx_unicast_frames		 ,rx_unicast_frames   );  
            WRITE_ULONG_PARAMETER (data->tx_unicast_frames		 ,tx_unicast_frames   );  
            break;
        }
		break;
	case PON_RAW_STAT_FEC_STATUS:
		{
            PON_fec_status_raw_data_t *data = (PON_fec_status_raw_data_t*)statistics_data;
            PRINT_BUFFER_HEADER
            WRITE_SHORT_PARAMETER (data->number_of_llids_with_fec     ,Number of llids with FEC );  
            break;
        }
		break;

	default:
			pas_vty_out (vty,"%s Error, unsupported statistics type: %d%s", 
					 return_buffer_header, raw_statistics_type,VTY_NEWLINE);

		return (EXIT_ERROR);
		break;
	}
    PON_FREE_FUNCTION(statistics_data);
    return (EXIT_OK);
}
#endif


/*  added by chenfj 2008-12-10
	测试发现,若对5201 PON板上所有PON芯片连续执行reset pon命令,会导致
	设备管理(devsm)中PON 板的状态机转移到一个不期望的状态,在等待
	一定的时间(60秒)后,若状态还未正常,则会执行整个PON板的复位;
	修改reset pon API,在完成复位的过程中设置复位标志;并在向设备管理
	报告PON板状态时,若有复位标志,则保持PON板当前状态不变
*/
int SetPonChipResetFlag(int slot, int port )
{
	short int PonPortIdx;

	if(PonCardSlotPortCheckByVty(slot, port, NULL) != ROK ) return( RERROR);
	PonPortIdx = GetPonPortIdxBySlot((unsigned short)slot, (unsigned short)port);
	CHECK_PON_RANGE

	PonChipResetFlag[PonPortIdx] = V2R1_ENABLE;
	return(ROK);
}

int ClearPonChipResetFlag(int slot, int port )
{
	short int PonPortIdx;

	if(PonCardSlotPortCheckByVty(slot, port, NULL) != ROK ) return( RERROR);
	PonPortIdx = GetPonPortIdxBySlot((unsigned short)slot, (unsigned short)port);
	CHECK_PON_RANGE

	PonChipResetFlag[PonPortIdx] = V2R1_DISABLE;
	return(ROK);
}

int GetPonChipResetFlag(int slot, int port)
{
	short int PonPortIdx;
	if(PonCardSlotPortCheckByVty(slot, port, NULL) != ROK) return( RERROR);
	PonPortIdx = GetPonPortIdxBySlot((unsigned short)slot, (unsigned short)port);
	CHECK_PON_RANGE

	return(PonChipResetFlag[PonPortIdx]);
}

char *pon_chip_typename( unsigned long pon_type )
{
	char *pName = NULL;

    if ( pon_type < PONCHIP_TYPE_MAX )
    {
		pName = PonChipType_s[pon_type];
    }
    else
    {
		pName = PonChipType_s[0];
    }
    
	return pName;
}

char *pon_chip_type2name( unsigned long pon_type )
{
	char *pName = NULL;

    if ( pon_type < PONCHIP_TYPE_MAX )
    {
		pName = s_apszChipNames[pon_type];
    }
    else
    {
		pName = s_apszChipNames[0];
    }
    
	return pName;
}

/*************************************************************
	added by chenfj 2009-2-10
	用于匹配PON 芯片固件和DBA 版本；每次PON 
	芯片启动时比较一次；之后每分钟做一次版本比较。
	若有不匹配，则在串口输出信息，并FLASH 中记录以备查询
	注: 以后更换PAS-SOFT 版本时，需扩充下面数组
**************************************************************/
#define PAS_VERSION_V5_3_0

#ifdef  PAS_SOFT_VERSION_V5_3_13
#undef  PAS_VERSION_V5_3_0
#undef  PAS_VERSION_V5_3_7
#undef  PAS_VERSION_V5_3_9
#undef  PAS_VERSION_V5_3_10
#undef  PAS_VERSION_V5_3_11
#undef  PAS_VERSION_V5_3_12
#define PAS_VERSION_V5_3_13
#elif defined  PAS_SOFT_VERSION_V5_3_12
#undef  PAS_VERSION_V5_3_0
#undef  PAS_VERSION_V5_3_7
#undef  PAS_VERSION_V5_3_9
#undef  PAS_VERSION_V5_3_10
#undef  PAS_VERSION_V5_3_11
#define PAS_VERSION_V5_3_12
#elif defined  PAS_SOFT_VERSION_V5_3_11
#undef  PAS_VERSION_V5_3_0
#undef  PAS_VERSION_V5_3_7
#undef  PAS_VERSION_V5_3_9
#undef  PAS_VERSION_V5_3_10
#define PAS_VERSION_V5_3_11
#elif defined  PAS_SOFT_VERSION_V5_3_9
#undef  PAS_VERSION_V5_3_0
#undef  PAS_VERSION_V5_3_7
#define PAS_VERSION_V5_3_9
#elif defined  PAS_SOFT_VERSION_V5_3_5
#undef  PAS_VERSION_V5_3_0
#define PAS_VERSION_V5_3_7
#endif

#ifdef  PAS_VERSION_V5_3_0
unsigned char *PON_FirmWare_Version[PONCHIP_TYPE_MAX] = 
{
	"",

	"V2.12.11.0",
	"V5.2.12.0",
	"V5.2.12.0",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
	
unsigned char *PON_Software_Version[PONCHIP_TYPE_MAX] = 
{
	"",
        
	"V5R03B020",
	"V5R03B020",
	"V5R03B020",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};

#elif defined  PAS_VERSION_V5_3_7 
unsigned char *PON_FirmWare_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V2.12.11.0",
	"V5.2.29.0",
	"V5.2.29.0",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};


unsigned char *PON_Software_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V5R03B280",
	"V5R03B280",
	"V5R03B280",
	"",
	"",
	"",
	
	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
#elif defined  PAS_VERSION_V5_3_9 
unsigned char *PON_FirmWare_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V2.12.11.0",
	"V5.2.39.0",
	"V5.2.39.0",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};


unsigned char *PON_Software_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V5R03B410",
	"V5R03B410",
	"V5R03B410",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
#elif defined  PAS_VERSION_V5_3_11 
unsigned char *PON_FirmWare_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V2.12.11.0",
	"V5.2.44.1",
	"V5.2.44.1",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};


unsigned char *PON_Software_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V5R03B520",
	"V5R03B520",
	"V5R03B520",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
#elif defined  PAS_VERSION_V5_3_12 
unsigned char *PON_FirmWare_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V2.12.17.0",
	"V5.2.56.0",
	"V5.2.56.0",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};


unsigned char *PON_Software_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V5R03B650",
	"V5R03B650",
	"V5R03B650",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
#elif defined  PAS_VERSION_V5_3_13 
unsigned char *PON_FirmWare_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V2.12.17.0",
	"V5.2.59.0",
	"V5.2.59.0",
	"V2.3.51.0",/*8.0.11则为2.3.41.0*//*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};


unsigned char *PON_Software_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"V5R03B700",
	"V5R03B700",
	"V5R03B700",
	"V2R01B590",/*8.0.11则为V2R01B540*//*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
#endif


unsigned char *Pas5001_DBA_Name[PONCHIP_TYPE_MAX] = 
{
	"",
	"Plato DBA",
	"Plato DBA",
	"Plato DBA",
	"Plato DBA",/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};

#ifdef PLATO_DBA_V3_5
unsigned char *Pas5001_DBA_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"3.5",
	"3.5",
	"3.5",
	"4.3",/*for 10G EPON of PMC8411 RUN by jinhl @2012-11-12*/
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
#elif defined PLATO_DBA_V3_3
unsigned char *Pas5001_DBA_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"3.3",
	"3.3",
	"3.3",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
#elif defined PLATO_DBA_V3_1
unsigned char *Pas5001_DBA_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"3.3",
	"3.3",
	"3.3",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
#else
unsigned char *Pas5001_DBA_Version[PONCHIP_TYPE_MAX] = 
{
	"",
	"2.",
	"2.",
	"2.",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
    
	""
};
#endif

unsigned int PonPortFirmwareAndDBAVersionMatch(short int PonPortIdx)
{
   	int ret;
	short int PonChipType;
    bool firm_is_ok, dba_is_ok;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if( OLT_PONCHIP_ISVALID(PonChipType) )
	{
    	if((ret = GetPonDeviceVersion(PonPortIdx)) != ROK ) return(RERROR);
    	if((ret = GetDBAInfo(PonPortIdx)) != ROK) return(RERROR);

		if( ('\0' == PON_Software_Version[PonChipType][0]) || (VOS_StrCmp(PonChipMgmtTable[PonPortIdx].HostSwVer, PON_Software_Version[PonChipType]) == 0) )
		{
		    if ( '\0' == PON_FirmWare_Version[PonChipType][0] )
            {
                firm_is_ok = OLTAdv_ChkVersion(PonPortIdx);    
            }
            else
            {
                firm_is_ok = (VOS_StrCmp(PonChipMgmtTable[PonPortIdx].FirmwareVer, PON_FirmWare_Version[PonChipType]) == 0) ? 1 : 0;
            }
            
			if( firm_is_ok )
            {
				if(PonPortTable[PonPortIdx].firmwareMismatchAlarm == V2R1_ENABLE )
				{
					 /* 清除不匹配告警
					 sys_console_printf("Pon%d/%d firmware version is match\r\n", slot, port);*/
					PonPortTable[PonPortIdx].firmwareMismatchAlarm = V2R1_DISABLE;
					FlashAlarm_PonPortVersionMismatch(PonPortIdx, PON_OLT_BINARY_FIRMWARE, V2R1_DISABLE);
				}
			}
			else
			{
				/*sys_console_printf("ERR:Pon%d/%d firmware version is mis-match\r\n", slot, port);
				if(PonPortTable[PonPortIdx].firmwareMismatchAlarm != V2R1_ENABLE )
					{*/
					 /* 产生不匹配告警*/
					FlashAlarm_PonPortVersionMismatch(PonPortIdx, PON_OLT_BINARY_FIRMWARE, V2R1_ENABLE);
					PonPortTable[PonPortIdx].firmwareMismatchAlarm = V2R1_ENABLE;
					return(V2R1_ENABLE);
				/*	}*/
			}

            if ( ('\0' == Pas5001_DBA_Name[PonChipType][0]) || (VOS_StrnCmp(&DBA_name[0],Pas5001_DBA_Name[PonChipType],VOS_StrLen(Pas5001_DBA_Name[PonChipType])) == 0) )
            {
    			if ( '\0' == Pas5001_DBA_Version[PonChipType][0] )
                {
                    dba_is_ok = OLTAdv_ChkDBAVersion(PonPortIdx);    
                }
                else
                {
                    dba_is_ok = (VOS_StrnCmp(&DBA_Version[0],Pas5001_DBA_Version[PonChipType],VOS_StrLen(Pas5001_DBA_Version[PonChipType])) == 0) ? 1 : 0;
                }

    			if( dba_is_ok )
                {
    				if(PonPortTable[PonPortIdx].DBAMismatchAlarm == V2R1_ENABLE )
					{
					/* 清除不匹配告警
					sys_console_printf("Pon%d/%d DBA version is match\r\n", slot, port);*/
					PonPortTable[PonPortIdx].DBAMismatchAlarm = V2R1_DISABLE;
					FlashAlarm_PonPortVersionMismatch(PonPortIdx, PON_OLT_BINARY_DBA_DLL, V2R1_DISABLE);
					}
				}
    			else
				{
				/*sys_console_printf("ERR:Pon%d/%d DBA version is mis-match\r\n", slot, port);
				if(PonPortTable[PonPortIdx].DBAMismatchAlarm != V2R1_ENABLE )
					{*/
					/* 产生不匹配告警*/
					FlashAlarm_PonPortVersionMismatch(PonPortIdx, PON_OLT_BINARY_DBA_DLL, V2R1_ENABLE);
					PonPortTable[PonPortIdx].DBAMismatchAlarm = V2R1_ENABLE;
					return(V2R1_ENABLE);
				/*	}*/
				}
            }
		}
	}

	return(ROK);	

}

int PonPortFirmwareAndDBAVersionMatchByVty(short int PonPortIdx, struct vty *vty)
{
	int ret;
	short int PonChipType;
    bool firm_is_ok, dba_is_ok;

	CHECK_PON_RANGE

	PonChipType = GetPonChipTypeByPonPort(PonPortIdx);
	if( OLT_PONCHIP_ISVALID(PonChipType) )
    {
    	if(OLTAdv_IsExist(PonPortIdx) != TRUE ) return(RERROR);
    	if((ret = GetDBAInfo(PonPortIdx)) !=  ROK) return(RERROR);
    	if((ret = GetPonDeviceVersion(PonPortIdx)) != ROK ) return(RERROR);

		if( ('\0' == PON_Software_Version[PonChipType][0]) || (VOS_StrCmp(PonChipMgmtTable[PonPortIdx].HostSwVer, PON_Software_Version[PonChipType]) == 0) )
		{
		    if ( '\0' == PON_FirmWare_Version[PonChipType][0] )
            {
                firm_is_ok = OLTAdv_ChkVersion(PonPortIdx);    
            }
            else
            {
                firm_is_ok = (VOS_StrCmp(PonChipMgmtTable[PonPortIdx].FirmwareVer, PON_FirmWare_Version[PonChipType]) == 0) ? 1 : 0;
            }
            
			if( !firm_is_ok )
			{
    			FlashAlarm_PonPortVersionMismatch(PonPortIdx, PON_OLT_BINARY_FIRMWARE, V2R1_ENABLE);
    			PonPortTable[PonPortIdx].firmwareMismatchAlarm = V2R1_ENABLE;
    			/*ShutdownPonPort(PonPortIdx);*/
    			/*
        			if(vty != NULL)
        				vty_out(vty,"ERR:Pon%d/%d firmware version is mis-match\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx) );
        			else 
    				sys_console_printf("ERR:Pon%d/%d firmware version is mis-match\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
    				*/
			}

            if ( ('\0' == Pas5001_DBA_Name[PonChipType][0]) || (VOS_StrnCmp(&DBA_name[0],Pas5001_DBA_Name[PonChipType],VOS_StrLen(Pas5001_DBA_Name[PonChipType])) == 0) )
            {
    			if ( '\0' == Pas5001_DBA_Version[PonChipType][0] )
                {
                    dba_is_ok = OLTAdv_ChkDBAVersion(PonPortIdx);    
                }
                else
                {
                    dba_is_ok = (VOS_StrnCmp(&DBA_Version[0],Pas5001_DBA_Version[PonChipType],VOS_StrLen(Pas5001_DBA_Version[PonChipType])) == 0) ? 1 : 0;
                }

    			if( !dba_is_ok )
    			{
        			FlashAlarm_PonPortVersionMismatch(PonPortIdx, PON_OLT_BINARY_DBA_DLL, V2R1_ENABLE);
        			PonPortTable[PonPortIdx].DBAMismatchAlarm = V2R1_ENABLE;
        			/*ShutdownPonPort(PonPortIdx);*/
        			/*
            			if(vty != NULL)
            				vty_out(vty,"ERR:Pon%d/%d DBA version is mis-match\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
            			else
            				sys_console_printf("ERR:Pon%d/%d DBA version is mis-match\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx));
            			*/
    			}
			}
		}
    }   
    
	return(ROK);
}

void AllPonPortFirmwareAndDBAVersionMatchByVty(struct vty *vty)
{
	short int PonPortIdx;

	for(PonPortIdx=0; PonPortIdx < MAXPON; PonPortIdx++)
		PonPortFirmwareAndDBAVersionMatchByVty(PonPortIdx,vty);
}

#if 0
short int  SendTstMsgToPonPort(short int PonPortIdx)
{
	short int send_result;
	PASCOMM_msg_connection_test_t connection_test_msg;
	PASCOMM_msg_connection_response_t	connection_response_msg;
	PASCOMM_msg_t					response_msg_type;;

	connection_test_msg.char1 = 0xFE;
	connection_test_msg.char2 = 0x12;
	connection_test_msg.short1 = 0x1234;
	connection_test_msg.short2 = 0xFEDC;
	connection_test_msg.long1 = 0xFEDCBA09;
	connection_test_msg.long2 = 0x87654321;
	connection_test_msg.long3 = 0x1;
	connection_test_msg.long4 = 0x1234;
	connection_test_msg.double1.msb = 0xFEDCBA09;
	connection_test_msg.double1.lsb = 0x87654321;
	PON_unsigned__int64_2_long_double(&connection_test_msg.double2, &connection_test_msg.double1);
	connection_test_msg.mac_address[0] = 0xFE;
	connection_test_msg.mac_address[1] = 10;
	connection_test_msg.mac_address[2] = 11;
	connection_test_msg.mac_address[3] = 12;
	connection_test_msg.mac_address[4] = 13;
	connection_test_msg.mac_address[5] = 14;
	connection_test_msg.buffer_size = 5;
	connection_test_msg.buffer[0]= 0x23;
	connection_test_msg.buffer[1]= 0x45;
	connection_test_msg.buffer[2]= 0x67;
	connection_test_msg.buffer[3]= 0x89;
	connection_test_msg.buffer[4]= 0x0a;

	send_result = Send_msg ( PonPortIdx, 
								 PASCOMM_MSG_CONNECTION_TEST, 
								 (void *)&connection_test_msg, 
								 &response_msg_type, 
								 (void *)&connection_response_msg,
								 FALSE /* remote acknowledge */) ;	
	return(send_result);

}
#endif



/*
int Init_cli()
{

	CLI_activate();
}
*/
#ifdef __cplusplus

}
#endif
