/**************************************************************
**  modify history:
** 
**	1 modified by chenfj 2008-8-1
**         增加GFA6100 产品支持
**
***************************************************************/

#include "syscfg.h"

#ifdef	__cplusplus
extern "C"
{
#endif

#include "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include  "V2R1_product.h"


#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
#include "vos/vospubh/vos_ctype.h"
#include "cli/cli.h"
#include "cli/cl_cmd.h"
#include "cli/cl_mod.h"
#include "addr_aux.h"
#include "linux/inetdevice.h"

#include "ifm/ifm_type.h"
#include "ifm/ifm_pub.h"
#include "ifm/ifm_gtable.h"
#include "ifm/ifm_aux.h"
#include "ifm/ifm_act.h"
#include "ifm/ifm_cli.h"
#include "ifm/ifm_task.h"
#include "ifm/ifm_lock.h"
#include "mn_oam.h"

#include "ifm/ifm_debug.h"

#include "interface/interface_task.h"
#include "sys/console/sys_console.h"
#include "sys/main/sys_main.h"
#include "cpi/ctss/ctss_ifm.h"
#include "manage/snmp/sn_tc.h"

#include  "GwttOam/OAM_gw.h"

#include "../superset/platform/sys/main/Sys_main.h" 
#include "../superset/cpi/typesdb/Typesdb_module.h"

#include "Tdm_comm.h"
#include "Tdm_apis.h"
#include "tdm/mn_tdm.h"
#include "gwEponSys.h"
#include "onu/ExtBoardType.h"

extern ULONG IFM_PON_CREATE_INDEX( ULONG ulSlot, ULONG ulPort , ULONG ulOnuId, ULONG ulOnuFeId);
extern ulong_t eventQueId;

extern STATUS tdmReadVersion( UCHAR calltype, ULONG callnotifier, UCHAR * softwarever, USHORT *sfverlen, UCHAR *fpgaver, USHORT *fpgaverlen );
extern STATUS  tdmLoadFpga( UCHAR calltype, ULONG callnotifier, UCHAR m1load, UCHAR m2load, UCHAR m3load );
extern STATUS  tdmRunNotify( UCHAR calltype, ULONG callnotifier );
extern STATUS  tdmStatusQuery ( UCHAR calltype, ULONG callnotifier, UCHAR *status );
extern STATUS tdmReset ( UCHAR calltype, ULONG callnotifier ,USHORT target);
extern STATUS tdm_e1portTable_get( ULONG idxs[3], e1porttable_row_entry *pEntry );
extern STATUS tdm_e1portTable_get( ULONG idxs[3], e1porttable_row_entry *pEntry );
extern STATUS tdm_e1portTable_getNext( ULONG idxs[3], e1porttable_row_entry *pEntry );
extern STATUS tdm_e1portTable_set( UCHAR leafIdx, ULONG idxs[3], UCHAR setval );
extern STATUS tdm_e1portTable_rowset( ULONG idxs[3], UCHAR alarmmask, UCHAR crcenable, UCHAR lpbctrl );
extern STATUS rpc_tdm_tdmOnuTable_get( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], tdmonutable_row_entry *pEntry );
extern STATUS tdm_tdmOnuTable_get( ULONG idxs[2], tdmonutable_row_entry *pEntry );
extern  STATUS rpc_tdm_tdmOnuTable_getNext( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], tdmonutable_row_entry *pEntry );
extern STATUS tdm_tdmOnuTable_getNext( ULONG idxs[2], tdmonutable_row_entry *pEntry );
extern STATUS rpc_tdm_tdmOnuTable_set( UCHAR calltype, ULONG callnotifier, UCHAR leafIdx, ULONG idxs[2], UCHAR setval, USHORT *lport );
extern STATUS tdm_tdmOnuTable_set( UCHAR leafIdx, ULONG idxs[2], UCHAR setval, USHORT* );
extern STATUS rpc_tdm_tdmOnuTable_rowset( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], USHORT logicalOnuIdx, UCHAR onuRowStatus );
extern STATUS tdm_tdmOnuTable_rowset( ULONG idxs[2], USHORT logicalOnuIdx, UCHAR onuRowStatus );
extern STATUS  rpc_tdm_potsLinkTable_get( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], potslinktable_row_entry *pEntry );
extern STATUS tdm_potsLinkTable_get( ULONG idxs[2], potslinktable_row_entry *pEntry );
extern STATUS tdm_potsLinkTable_getNext( ULONG idxs[2], potslinktable_row_entry *pEntry );
extern STATUS tdm_potsLinkTable_set( UCHAR leafIdx, ULONG idxs[2], UCHAR setval[32] );
extern STATUS tdm_potsLinkTable_rowset( ULONG idxs[2], ULONG onuIdx, ULONG onuPotsBrd,\
													ULONG onuPotsIdx, ULONG phonecode, UCHAR linkdesc[32], UCHAR potsLinkStatus );
extern STATUS  rpc_tdm_potsPortTable_get( UCHAR calltype, ULONG callnotifier, ULONG idxs[3], potsporttable_row_entry *pEntry );
extern STATUS tdm_potsPortTable_get( ULONG idxs[3], potsporttable_row_entry *pEntry );
extern STATUS  rpc_tdm_potsPortTable_getNext( UCHAR calltype, ULONG callnotifier, ULONG idxs[3], potsporttable_row_entry *pEntry );
extern STATUS tdm_potsPortTable_getNext( ULONG idxs[3], potsporttable_row_entry *pEntry );
extern STATUS tdm_hdlcStatTable_get( ULONG idxs, hdlcstattable_row_entry *pEntry );
extern STATUS tdm_hdlcStatTable_getNext( ULONG idxs, hdlcstattable_row_entry *pEntry );
extern STATUS tdm_sgTable_get( ULONG idxs, sgtable_row_entry *pEntry );
extern STATUS tdm_sgTable_getNext( ULONG idxs, sgtable_row_entry *pEntry );
extern STATUS tdm_sgTable_set( UCHAR leafIdx, ULONG idxs, USHORT setval );
extern STATUS tdm_sgTable_rowset( ULONG idxs, USHORT rowset[6] );

extern int GetOnuPotsLinkByDevIdx(unsigned long OnuDeviceIdx, unsigned char PotsBoard, unsigned char PotsPort, BOOL *EnableFlag, unsigned char *TdmSlot, unsigned char *SgIdx, unsigned short int *LogicPort);
extern ULONG * V2R1_Parse_Port_List( CHAR * pcPort_List );
extern LONG PON_GetSlotPortOnu( ULONG ulIfIndex, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
/*extern  STATUS test_tdm_conf_set_func( ULONG table, ULONG leaf, ULONG a, ULONG b, ULONG c, ULONG d, CHAR* buf );*/
extern STATUS mn_tdmOnuDel( ULONG *idxs );
extern int mn_tdmOnuVoiceLoopbackStart( ULONG *idxs );
extern int mn_tdmOnuVoiceLoopbackStop( ULONG *idxs );


int   tdm_init_func();
int   tdm_showrun( struct vty * vty );

int   tdm_config_write ( struct vty * vty );
LONG tdm_node_install();
LONG tdm_module_init();
enum match_type Tdm_Check_Port_List( char * sg_log_port_list );

extern LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);
extern LONG MN_Vlan_Check_exist(ULONG  vlanIndex);
extern ULONG IFM_ETH_CREATE_INDEX( ULONG ulSlot, ULONG ulPort );
extern LONG IFM_GetIfStatusApi( ULONG ulIfIndex, ULONG * pulUpOrDown );
/*extern LONG slot_port_2_swport(int slot, int user_port, ULONG *ulSwSlot, ULONG *ulSwport);*/
extern LONG IFM_GetIfAdminStatusApi( ULONG unPhyIfIdx, ULONG * pulStatus );
extern STATUS getDeviceName( const ulong_t devIdx, char* pValBuf, ulong_t *pValLen );
extern STATUS getEthPortOperStatus( ulong_t devIdx, ulong_t brdIdx, ulong_t ethIdx, ulong_t *status );
extern STATUS getEthPortAdminStatus( ulong_t devIdx, ulong_t brdIdx, ulong_t ethIdx, ulong_t *status );
extern LONG getAlarmLevel( ulong_t alm_id, ulong_t *level );
extern LONG setAlarmLevel( ulong_t alm_id, ulong_t level );
extern int DeleteOnuFromSGBySgIdx( unsigned long OnuDeviceIdx, unsigned char tdmslot, unsigned char sgidx);
extern int AddNewOnuToSg(unsigned char TdmSlot, unsigned char SgIdx, unsigned long OnuDeviceIdx, unsigned short int *logicOnu);
extern int GetOnuPotsLogicalPort (ULONG Onudevidx, ULONG OnuBoard,ULONG OnuPort,USHORT *Logicalport);
extern BOOL onuIsOnLine( ULONG onuIdx );




#ifndef TDM_CLI_NODE 
#define TDM_CLI_NODE ATM_NODE
#endif

#define STATE_PE 2
#define  STATE_PS   1
#define  CRC_ENABLE  1
#define  CRC_DISABLE  2
#define  STATE_ULS 1
#define  STATE_ULPS  2
#define  STATE_ULPE   3

#define START_TDM_POTS               1           
#define START_TDM_PORT			1
#define START_TDM_VOICE_PORT   0
#define  MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST   MAXIMUM_LOGICPORT /* 2047*/
#define END_TDM_VOICE			MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST
#define  MAX_E1_PORT              8
/*
#define  MAX_TDM_PORT_NUMBER_IN_PORT_LIST   2000
#define  END_TDM_PORT     MAX_TDM_PORT_NUMBER_IN_PORT_LIST
*/
#define  MAX_TDM_POTS_NUMBER_IN_POTS_LIST    ONU_POTS_NUM_MAX
#define  END_TDM_POTS    MAX_TDM_POTS_NUMBER_IN_POTS_LIST
#define  END_PARSE_LIST  0xaa55
#define  MAXPHONENUM_LENGTH    8

unsigned char *vlanenablestatus []=
{
	(unsigned char*)"enable",
	(unsigned char*)"disable"
};

unsigned char *crcenable[]=
{
	(unsigned char*)"enable",
	(unsigned char*)"disable"
};

unsigned char *portcode[]=
{
	(unsigned char*)"HDB3",
	(unsigned char*)"NRZ"
};

unsigned char *portImpendance[]=
{
	(unsigned char*)"75",
	(unsigned char*)"120"
};

char *get_alarm( UCHAR alarm, char *str )
{
	if( str == NULL )
		return str;
	if( (alarm & E1_ALM_ALL) == 0 )
	{
		VOS_StrCpy( str, "normal");
		return str;
	}
	str[0] = 0;
	if( alarm & E1_ALM_LOS )
		VOS_StrCat( str,"LOS");
	if( alarm & E1_ALM_LOF )
		VOS_StrCat(str,"LOF");
	if( alarm & E1_ALM_AIS)
		VOS_StrCat(str, "AIS");
	if( alarm & E1_ALM_RAI)
		VOS_StrCat(str, "RAI");
	if( alarm &  E1_ALM_SMF)
		VOS_StrCat(str, "SMF");
	if( alarm &  E1_ALM_LOFSMF)
		VOS_StrCat(str, "LOFSMF");
	if( alarm & E1_ALM_CRC3)
		VOS_StrCat(str, "CRC3");
	if( alarm &  E1_ALM_CRC6)
		VOS_StrCat(str, "CRC6");
	return str;
}

char *get_alarmmask( UCHAR mask, char *str )
{
	if( str == NULL )
		return str;
	if( (mask & E1_ALM_ALL) == 0 )
	{
		VOS_StrCpy( str, "NO");
		return str;
	}
	str[0] = 0;
	if( mask & E1_ALM_LOS )
		VOS_StrCat( str,"LOS|");
	if( mask & E1_ALM_LOF )
		VOS_StrCat(str,"LOF|");
	if( mask & E1_ALM_AIS)
		VOS_StrCat(str, "AIS|");
	if( mask & E1_ALM_RAI)
		VOS_StrCat(str, "RAI|");
	if( mask &  E1_ALM_SMF)
		VOS_StrCat(str, "SMF|");
	if( mask &  E1_ALM_LOFSMF)
		VOS_StrCat(str, "LOFSMF|");
	if( mask & E1_ALM_CRC3)
		VOS_StrCat(str, "CRC3|");
	if( mask &  E1_ALM_CRC6)
		VOS_StrCat(str, "CRC6|");
	return str;
};


unsigned char *portfun[]=
{
	(unsigned char*)"e1trans",
	(unsigned char*)"potstrans"
};

unsigned char *alarmstatus[]=
{
	(unsigned char*)"e1 los of signal",
	(unsigned char*)"e1 RCV ais",
	(unsigned char*)"e1 xmt aid",
	(unsigned char*)"e1 other failuer",
	(unsigned char*)"e1 smf lof",
	(unsigned char*)"e1 lof"
};

unsigned char *portloopback[]=
{
	(unsigned char*)"noop",
	(unsigned char*)"lpbStart",
	(unsigned char*)"lpbStop",
	(unsigned char*)"inProcess",
	(unsigned char*)""
};

unsigned char * adminstatus []=
{
	(unsigned char*)"up",
	(unsigned char*)"down",
	(unsigned char*)"testing"
};

unsigned char*opertstatus[]=
{
	(unsigned char*)"up",
	(unsigned char*)"down",
	(unsigned char*)"unkonwn",
	(unsigned char*)"loop",
	(unsigned char*)"notpresent"
};

unsigned char *ServiceStatus[]=
{
	(unsigned char *)"In service",
	(unsigned char *)"Out of service"
};

unsigned char *alarmlevel[]=
{
	(unsigned char*)"vital",
	(unsigned char*)"major",
	(unsigned char*)"minor",
	(unsigned char*)"warning"
};

unsigned char *loopbackstatus[]=
{
	(unsigned char*) "loopback",
	(unsigned char*) "noop"
};

#if  0
unsigned char* portstatus[]=
{
	(unsigned char*)"handon",
	(unsigned char*)"handoff"
};
#endif
unsigned char*hookstatus[]=
{
	(unsigned char*)"on hook",
	(unsigned char*)"off hook"

};

#define  MAXONUPOTS    ONU_POTS_NUM_MAX
#define  MAXONUPOTS_GT865     16
#define  MAXONUPOTS_GT861     ONU_POTS_NUM_MAX
#define  MAXPERBOARDPOTS_GT861  8
#define  MAXLOGICALSGPOTS   (MAXIMUM_LOGICPORT+1)  /*2048*/
/*
#define  MAXONUMUN    256
#define  PHONENUM         8
*/
#define  MAXE1NUM        MAX_E1_PORT /* 8*/
#define  PHONEDESC        32
#define  E1_LOOP_ENABLE   2
#define  E1_LOOP_DISABLE   3
#define  STATE_PSA       0
#define  VOICE_LOOP_ENABLE    1
#define  VOICE_LOOP_DISABLE   2 

#define  E1_ALM_CLASS_VITAL    1
#define  E1_ALM_CLASS_MAJOR   2
#define  E1_ALM_CLASS_MINOR   3
#define  E1_ALM_CLASS_WARNING    4
#define  MAXALARMMNU   8

unsigned char  OnuMaxPots[]= 
	{
		0,
		0,
		0,
		0,
		0,/*"GT811",*/
		0,/*"GT831",*/
		0,/*"GT831_CATV",*/
		0,/*"GT812",*/
		0,/*"GT813",*/
		0,/*"GT881",*/
		MAXONUPOTS_GT861,/*(unsigned char *)"GT861",*/
		0,/*"GT891",*/
		
		0,/*"GT810",*/
		0,/*"GT863",*/

		0,/*"other",*/
		/*(unsigned char *)"_CTC_",*/
		MAXONUPOTS_GT865,/*"GT865",*/
		0,/*"GT816",*/
		
		0,/*"GT811_A",*/
		0,/*"GT812_A",*/
		0,/*"GT831_A",*/
		0,/*"GT831_A_CATV",*/

		0, /*"GT815" */

		0/*"unknown"*/		
	};

ULONG * TDM_Parse_Voice_Link_Port_List( CHAR * voice_link_port_list );
ULONG * TDM_Parse_E1_List( CHAR * e1_list );
ULONG * TDM_Parse_Pots_List( CHAR * pots_list );
ULONG *TDM_Parse_Slot_Pots_List(CHAR* pcPort_List);



#define BEGIN_TDM_PARSE_POTS_LIST_TO_POT(pots_list,slot, port) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = TDM_Parse_Slot_Pots_List(pots_list );\
    if(_pulIfArray != NULL)\
    {\
    	slot=_pulIfArray[0];\
        for(_i=1;_pulIfArray[_i]!=0;_i++)\
        {\
            port = _pulIfArray[_i];
           /* sys_console_printf ("port=%d\r\n",port);*/

#define END_TDM_PARSE_POTS_LIST_TO_POT() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}


enum match_type Tdm_Check_Pots_List( char * pots_list )
{
    int len = VOS_StrLen( pots_list );
    ULONG interface_list[ MAX_TDM_POTS_NUMBER_IN_POTS_LIST];
    int j, if_num = 0;
    int ret = 0;
    ULONG ulPort=0,ulSlot=0;

    char *plistbak = NULL;

    if ( ( !pots_list ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

    VOS_MemZero( ( char * ) interface_list, MAX_TDM_POTS_NUMBER_IN_POTS_LIST * sizeof( ULONG ) );
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, pots_list );

    BEGIN_TDM_PARSE_POTS_LIST_TO_POT( plistbak, ulSlot,ulPort )
    {
    	if((ulSlot==0)||(ulSlot>MAX_ONU_BRD_NUM)||(ulSlot<1))
		return no_match;

        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulPort )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                VOS_Free(_pulIfArray);
                return no_match;  /* 写重复的端口认为是错误的语法  */
            }
        }
        interface_list[ if_num ] = ulPort;
        if_num ++;
        if ( if_num > MAX_TDM_POTS_NUMBER_IN_POTS_LIST )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            VOS_Free(_pulIfArray); 

            return no_match;
        }
        ret = 1;
    }
    END_TDM_PARSE_POTS_LIST_TO_POT();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}


CMD_NOTIFY_REFISTER_S stCMD_Tdm_Pots_List_Check =
{
    "<pots_list>",
    Tdm_Check_Pots_List,
    0
};

ULONG *TDM_Parse_Slot_Pots_List(CHAR* pcPort_List)
{
	 ULONG ulState = STATE_ULS;
	 ULONG ulslot=0;
	CHAR digit_temp[ 12 ];
	ULONG ulInterfaceList[ MAX_TDM_POTS_NUMBER_IN_POTS_LIST ];
	ULONG ulPortS = 0;
	ULONG ulPortE = 0;
	CHAR cToken;
	ULONG iflist_i = 0;
	ULONG list_i = 0;
	ULONG temp_i = 0;
	ULONG ulListLen = 0;
	CHAR * list;


	VOS_MemZero( ulInterfaceList, MAX_TDM_POTS_NUMBER_IN_POTS_LIST * 4 );
	ulListLen = VOS_StrLen( pcPort_List );
	list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
	if ( list == NULL )
	{
	    return NULL;
	}
	VOS_StrnCpy( list, pcPort_List, ulListLen + 1 );
	list[ ulListLen ] = ',';
	list[ ulListLen + 1 ] = '\0';

	cToken = list[ list_i ];
	
	while ( cToken != 0 )
	{
		switch ( ulState )
		{
			case  STATE_ULS :
				if ( vos_isdigit( cToken ) )
		                {
		                    digit_temp[ temp_i ] = cToken;
		                    temp_i++;
		                    if ( temp_i >= 11 )
		                    {
		                        goto error;
		                    }
		                }
	                else if ( vos_isspace( cToken ) )
	                {}
	                else if ( cToken == '/' )
	                {
/*sys_console_printf("start parse /,temp_i=%d\r\n",temp_i);*/
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulslot = ( ULONG ) VOS_AtoL( digit_temp );
/*sys_console_printf("slot=%d\r\n",ulslot);*/
				if((ulslot>MAX_ONU_BRD_NUM)||(ulslot<1))
	                    {
	                        goto error;
	                    }
				ulInterfaceList[ 0 ] = ulslot;
	                     iflist_i++;
	                    temp_i = 0;
	                    ulState = STATE_ULPS;
/*sys_console_printf("end parse /\r\n");*/
	                }
	                else if ( cToken == ',' )
	                {
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
	                    if( (ulPortS>32)||(ulPortS<1) )
	                    {
	                        goto error;
	                    }
	                    if ( 0 != ulPortS )
	                    {if(iflist_i!=0)
	                       	{ 
	                       		ulInterfaceList[ iflist_i ] = ulPortS;
	                       		 iflist_i++;
					}
				  else
				  	{
						iflist_i++;
						ulInterfaceList[ iflist_i ] = ulPortS;
						iflist_i++;
				  	}
	                    }
	                    if ( iflist_i >= MAX_TDM_POTS_NUMBER_IN_POTS_LIST)
	                    {
	                        goto error;
	                    }
	                    temp_i = 0;
	                    ulState = STATE_ULS;
	                }
	                else if ( cToken == '-' )
	                {
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
	                    temp_i = 0;
	                    ulState = STATE_ULPE;
	                }
	                else
	                {
	                    goto  error;
	                }
	                break;
		case  STATE_ULPS:
			if ( vos_isdigit( cToken ) )
	                {
/*sys_console_printf("after parse /,digital start\r\n"); */
	                    digit_temp[ temp_i ] = cToken;
	                    temp_i++;
	                    if ( temp_i >= 11 )
	                    {
	                        goto error;
	                    }
	                }
	                else if ( vos_isspace( cToken ) )
	                {}
	                else if ( cToken == ',' )
	                {
/*sys_console_printf("start parse ,\r\n"); */
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
				if((ulPortS>16)||(ulPortS<1))  /*modified by zhengyt 09-02-10,解决问题单7884*/
	                    {
	                        goto error;
	                    }
				
	                    if ( 0 != ulPortS )
	                    {if(iflist_i!=0)
	                       	{ 
	                       		ulInterfaceList[ iflist_i ] = ulPortS;
	                       		 iflist_i++;
					}
				  else
				  	{
						iflist_i++;
						ulInterfaceList[ iflist_i ] = ulPortS;
						iflist_i++;
				  	}
	                    }
	                    if ( iflist_i >= MAX_TDM_POTS_NUMBER_IN_POTS_LIST )
	                    {
	                        goto error;
	                    }
	                    temp_i = 0;
	                    ulState = STATE_ULS;
	                }
	                else if ( cToken == '-' )
	                {
/*sys_console_printf("start parse -,temp_i=%d\r\n",temp_i); */
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
/*sys_console_printf("ports=%d\r\n",ulPortS);	*/
	                    temp_i = 0;
	                    ulState = STATE_ULPE;
	                }
	                else
	                {
	                    goto error;
	                }
	                break;
			case  STATE_ULPE :
				 if ( vos_isdigit( cToken ) )
		                {
/*sys_console_printf("after parse -,digital start\r\n");  */
		                    digit_temp[ temp_i ] = cToken;
		                    temp_i++;
		                    if ( temp_i >= 11 )
		                    {
		                        goto error;
		                    }
		                }
	                else if ( vos_isspace( cToken ) )
	                {}
	                else if ( cToken == ',' )
	                {
 	                
	                    ULONG i;
	                    ULONG i_s, i_e;
/*sys_console_printf("start , temp_i=%d\r\n",temp_i);*/
	                    if ( temp_i == 0 )
	                    {
	                        goto error;
	                    }
	                    digit_temp[ temp_i ] = 0;
	                    ulPortE = ( ULONG ) VOS_AtoL( digit_temp );
/*sys_console_printf("ulportE=%d ,ulportS=%d\r\n",ulPortE,ulPortS);*/
	                    if ( ulPortE > ulPortS )
	                    {
	                        i_s = ulPortS;
	                        i_e = ulPortE;
	                    }
	                    else
	                    {
	                        i_s = ulPortE;
	                        i_e = ulPortS;
	                    }
	                   
	                    for ( i = i_s;i <= i_e;i++ )
	                    {
/*sys_console_printf("for circle i=%d,iflist_i=%d \r\n",i,iflist_i); */
	                    		if((i>32)||(i<1))
		                        {
		                            goto error;
		                        }

	                        if ( 0 != i )
	                        {if(iflist_i!=0)
	                       	{ 
	                       		ulInterfaceList[ iflist_i ] = i;
	                       		 iflist_i++;
					}
				  else
				  	{
						iflist_i++;
						ulInterfaceList[ iflist_i ] = i;
						iflist_i++;
				  	}
	                    }
	                        if ( iflist_i >= MAX_TDM_POTS_NUMBER_IN_POTS_LIST )
	                        {
	                            goto error;
	                        }
	                    }
	                    temp_i = 0;
	                    ulState = STATE_ULS;
	                }
	                else
	                {
	                    goto error;
	                }
	                break;
					
	            default:
	                goto error;
				
		}
		list_i++;
		cToken = list[ list_i ];
	}
/*sys_console_printf("last:iflist_i=%d\r\n",iflist_i);*/
 	VOS_Free( list );
	    if ( iflist_i == 0 )
	    {
	        return NULL;
	    }
	    else
	    {
	        list = VOS_Malloc( ( iflist_i + 1 ) * 4, MODULE_RPU_IFM );
/*sys_console_printf("end:g_malloc list,list=%X\r\n",list);	*/
	        if ( list == NULL )
	        {
	            return NULL;
	        }
	        VOS_MemZero( list, ( iflist_i + 1 ) * 4 );
/*for(temp_i=0;temp_i<iflist_i;temp_i++)
{
	sys_console_printf("last::%d\t",ulInterfaceList[temp_i]);
}
sys_console_printf("\r\n");*/
	        VOS_MemCpy( list, ulInterfaceList, iflist_i * 4 );
	        return ( ULONG * ) list;
	    }
	error:
	    VOS_Free( list );
	    return NULL;


}
#if  0
ULONG * TDM_Parse_Pots_List( CHAR * pcPort_List )
{
    ULONG ulState = STATE_PS;
    /*ULONG ulSlot = 0;*/
    CHAR digit_temp[ 12 ];
    ULONG ulInterfaceList[ MAX_TDM_POTS_NUMBER_IN_POTS_LIST ];
    ULONG ulPortS = 0;
    ULONG ulPortE = 0;
    CHAR cToken;
    ULONG iflist_i = 0;
    ULONG list_i = 0;
    ULONG temp_i = 0;
    /*ULONG ulIfindex;*/
    ULONG ulListLen = 0;
    CHAR * list;
    /*ULONG ulType;
    ULONG ulSwport = 0;
    ULONG ulSwSlot=0;*/

    VOS_MemZero( ulInterfaceList, MAX_TDM_POTS_NUMBER_IN_POTS_LIST * 4 );
    ulListLen = VOS_StrLen( pcPort_List );
    list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
    if ( list == NULL )
    {
        return NULL;
    }
    VOS_StrnCpy( list, pcPort_List, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken != 0 )
    {
        switch ( ulState )
        {            
            case STATE_PS:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );

                    if ( ulPortS > MAX_TDM_POTS_NUMBER_IN_POTS_LIST )
                    {
                        goto error;
                    }
			/*输出端口号**/
                    if ( 0 != ulPortS )
                    {
                        ulInterfaceList[ iflist_i ] = ulPortS;
                        iflist_i++;
                    }
                    if ( iflist_i > MAX_TDM_POTS_NUMBER_IN_POTS_LIST )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
                    temp_i = 0;
                    ulState = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PE:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    ULONG i;
                    ULONG i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortE = ( ULONG ) VOS_AtoL( digit_temp );
                    if ( ulPortE > ulPortS )
                    {
                        i_s = ulPortS;
                        i_e = ulPortE;
                    }
                    else
                    {
                        i_s = ulPortE;
                        i_e = ulPortS;
                    }
                    
                    for ( i = i_s;i <= i_e;i++ )
                    {
                        if ( i<START_TDM_POTS || i > END_TDM_POTS )
                        {
                            goto error;
                        }

                        if ( 0 != i )
                        {
                            ulInterfaceList[ iflist_i ] = i;
                            iflist_i++;
                        }
                        if ( iflist_i > MAX_TDM_POTS_NUMBER_IN_POTS_LIST )
                        {
                            goto error;
                        }
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    VOS_Free( list );
    if ( iflist_i == 0 )
    {
        return NULL;
    }
    else
    {
        list = VOS_Malloc( ( iflist_i + 1 ) * 4, MODULE_RPU_IFM );
        if ( list == NULL )
        {
            return NULL;
        }
        VOS_MemZero( list, ( iflist_i + 1 ) * 4 );
        VOS_MemCpy( list, ulInterfaceList, iflist_i * 4 );
        return ( ULONG * ) list;
    }
error:
    VOS_Free( list );
    return NULL;
}

 #endif
#if   0
#define BEGIN_TDM_PARSE_PORT_LIST_TO_PORT(port_list, port) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = TDM_Parse_Port_List(port_list);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0;_i++)\
        {\
            port = _pulIfArray[_i];

#define END_TDM_PARSE_PORT_LIST_TO_PORT() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}


enum match_type Tdm_Check_Port_List( char * port_list )
{
    int len = VOS_StrLen( port_list );
    ULONG interface_list[ MAX_TDM_PORT_NUMBER_IN_PORT_LIST];
    int j, if_num = 0;
    int ret = 0;
    ULONG ulPort=0;

    char *plistbak = NULL;

    if ( ( !port_list ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

    VOS_MemZero( ( char * ) interface_list, MAX_TDM_PORT_NUMBER_IN_PORT_LIST * sizeof( ULONG ) );
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, port_list );

    BEGIN_TDM_PARSE_PORT_LIST_TO_PORT( plistbak, ulPort )
    {
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulPort )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                VOS_Free(_pulIfArray);
                return no_match;  /* 写重复的端口认为是错误的语法  */
            }
        }
        interface_list[ if_num ] = ulPort;
        if_num ++;
        if ( if_num > MAX_TDM_PORT_NUMBER_IN_PORT_LIST )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            VOS_Free(_pulIfArray); 

            return no_match;
        }
        ret = 1;
    }
    END_TDM_PARSE_PORT_LIST_TO_PORT();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}


CMD_NOTIFY_REFISTER_S stCMD_Tdm_Port_List_Check =
{
    "<port_list>",
    Tdm_Check_Port_List,
    0
};


ULONG * TDM_Parse_Port_List( CHAR * pcPort_List )
{
    ULONG ulState = STATE_PS;
    /*ULONG ulSlot = 0;*/
    CHAR digit_temp[ 12 ];
    ULONG ulInterfaceList[ MAX_TDM_PORT_NUMBER_IN_PORT_LIST ];
    ULONG ulPortS = 0;
    ULONG ulPortE = 0;
    CHAR cToken;
    ULONG iflist_i = 0;
    ULONG list_i = 0;
    ULONG temp_i = 0;
    /*ULONG ulIfindex;*/
    ULONG ulListLen = 0;
    CHAR * list;
    /*ULONG ulType;
    ULONG ulSwport = 0;
    ULONG ulSwSlot=0;*/

    VOS_MemZero( ulInterfaceList, MAX_TDM_PORT_NUMBER_IN_PORT_LIST * 4 );
    ulListLen = VOS_StrLen( pcPort_List );
    list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
    if ( list == NULL )
    {
        return NULL;
    }
    VOS_StrnCpy( list, pcPort_List, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken != 0 )
    {
        switch ( ulState )
        {            
            case STATE_PS:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );

                    if ( ulPortS > MAX_TDM_PORT_NUMBER_IN_PORT_LIST )
                    {
                        goto error;
                    }
			/*输出端口号**/
                    if ( 0 != ulPortS )
                    {
                        ulInterfaceList[ iflist_i ] = ulPortS;
                        iflist_i++;
                    }
                    if ( iflist_i > MAX_TDM_PORT_NUMBER_IN_PORT_LIST )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
                    temp_i = 0;
                    ulState = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PE:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    ULONG i;
                    ULONG i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortE = ( ULONG ) VOS_AtoL( digit_temp );
                    if ( ulPortE > ulPortS )
                    {
                        i_s = ulPortS;
                        i_e = ulPortE;
                    }
                    else
                    {
                        i_s = ulPortE;
                        i_e = ulPortS;
                    }
                    
                    for ( i = i_s;i <= i_e;i++ )
                    {
                        if ( i<START_TDM_PORT|| i > END_TDM_PORT )
                        {
                            goto error;
                        }

                        if ( 0 != i )
                        {
                            ulInterfaceList[ iflist_i ] = i;
                            iflist_i++;
                        }
                        if ( iflist_i > MAX_TDM_PORT_NUMBER_IN_PORT_LIST )
                        {
                            goto error;
                        }
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    VOS_Free( list );
    if ( iflist_i == 0 )
    {
        return NULL;
    }
    else
    {
        list = VOS_Malloc( ( iflist_i + 1 ) * 4, MODULE_RPU_IFM );
        if ( list == NULL )
        {
            return NULL;
        }
        VOS_MemZero( list, ( iflist_i + 1 ) * 4 );
        VOS_MemCpy( list, ulInterfaceList, iflist_i * 4 );
        return ( ULONG * ) list;
    }
error:
    VOS_Free( list );
    return NULL;
}

#endif


#define BEGIN_TDM_PARSE_E1_LIST_TO_PORT(e1_list, port) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = TDM_Parse_E1_List(e1_list);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0;_i++)\
        {\
            port = _pulIfArray[_i];

#define END_TDM_PARSE_E1_LIST_TO_PORT() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}


enum match_type Tdm_Check_E1_List ( char * e1_list )
{
    int len = VOS_StrLen( e1_list );
    ULONG interface_list[ MAX_E1_PORT];
    int j, if_num = 0;
    int ret = 0;
    ULONG ulPort=0;

    char *plistbak = NULL;

    if ( ( !e1_list ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

    VOS_MemZero( ( char * ) interface_list, MAX_E1_PORT * sizeof( ULONG ) );
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, e1_list );

    BEGIN_TDM_PARSE_E1_LIST_TO_PORT( plistbak, ulPort )
    {
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulPort )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                VOS_Free(_pulIfArray);
                return no_match;  /* 写重复的端口认为是错误的语法  */
            }
        }
        interface_list[ if_num ] = ulPort;
        if_num ++;
        if ( if_num > MAX_E1_PORT )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            VOS_Free(_pulIfArray); 

            return no_match;
        }
        ret = 1;
    }
    END_TDM_PARSE_E1_LIST_TO_PORT();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}

CMD_NOTIFY_REFISTER_S stCMD_Tdm_E1_List_Check =
{
    "<e1_list>",
    Tdm_Check_E1_List,
    0
};


ULONG * TDM_Parse_E1_List( CHAR * pcPort_List )
{
    ULONG ulState = STATE_PS;
    /*ULONG ulSlot = 0;*/
    CHAR digit_temp[ 12 ];
    ULONG ulInterfaceList[ MAX_E1_PORT ];
    ULONG ulPortS = 0;
    ULONG ulPortE = 0;
    CHAR cToken;
    ULONG iflist_i = 0;
    ULONG list_i = 0;
    ULONG temp_i = 0;
    /*ULONG ulIfindex;*/
    ULONG ulListLen = 0;
    CHAR * list;
    /*ULONG ulType;
    ULONG ulSwport = 0;
    ULONG ulSwSlot=0;*/

    VOS_MemZero( ulInterfaceList, MAX_E1_PORT * 4 );
    ulListLen = VOS_StrLen( pcPort_List );
    list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
    if ( list == NULL )
    {
        return NULL;
    }
    VOS_StrnCpy( list, pcPort_List, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken != 0 )
    {
        switch ( ulState )
        {            
            case STATE_PS:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );

                    if ( ulPortS > MAX_E1_PORT )
                    {
                        goto error;
                    }
			/*输出端口号**/
                    if ( 0 != ulPortS )
                    {
                        ulInterfaceList[ iflist_i ] = ulPortS;
                        iflist_i++;
                    }
                    if ( iflist_i > MAX_E1_PORT )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
                    temp_i = 0;
                    ulState = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PE:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    ULONG i;
                    ULONG i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortE = ( ULONG ) VOS_AtoL( digit_temp );
                    if ( ulPortE > ulPortS )
                    {
                        i_s = ulPortS;
                        i_e = ulPortE;
                    }
                    else
                    {
                        i_s = ulPortE;
                        i_e = ulPortS;
                    }
                    
                    for ( i = i_s;i <= i_e;i++ )
                    {
                        if ( i<START_TDM_PORT|| i > MAX_E1_PORT)
                        {
                            goto error;
                        }

                        if ( 0 != i )
                        {
                            ulInterfaceList[ iflist_i ] = i;
                            iflist_i++;
                        }
                        if ( iflist_i > MAX_E1_PORT )
                        {
                            goto error;
                        }
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    VOS_Free( list );
    if ( iflist_i == 0 )
    {
        return NULL;
    }
    else
    {
        list = VOS_Malloc( ( iflist_i + 1 ) * 4, MODULE_RPU_IFM );
        if ( list == NULL )
        {
            return NULL;
        }
        VOS_MemZero( list, ( iflist_i + 1 ) * 4 );
        VOS_MemCpy( list, ulInterfaceList, iflist_i * 4 );
        return ( ULONG * ) list;
    }
error:
    VOS_Free( list );
    return NULL;
}


#define BEGIN_TDM_PARSE_VOICE_LINK_LIST_TO_PORT(voice_link_port_list, port) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = TDM_Parse_Voice_Link_Port_List(voice_link_port_list);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=END_PARSE_LIST;_i++)\
        {\
            port = _pulIfArray[_i];

#define END_TDM_PARSE_VOICE_LINK_LIST_TO_PORT() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}


enum match_type Tdm_Check_Voice_Link_Port_List( char * voice_link_port_list)
{
    int len = VOS_StrLen( voice_link_port_list );
    ULONG interface_list[ MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST +1];
    int j, if_num = 0,i;
    int ret = 0;
    ULONG ulPort=0;

    char *plistbak = NULL;

    if ( ( !voice_link_port_list) || ( len < 1 ) )
    {
        return incomplete_match;
    }

  /*  VOS_MemSet( ( char * ) interface_list, MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1,(MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1)  );*/
    for(i=0;i<(MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1);i++)
    	{
		interface_list[i]=MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1;
	}
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, voice_link_port_list );

    BEGIN_TDM_PARSE_VOICE_LINK_LIST_TO_PORT(plistbak, ulPort )
    {
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulPort )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                VOS_Free(_pulIfArray);
                return no_match;  /* 写重复的端口认为是错误的语法  */
            }
        }
        interface_list[ if_num ] = ulPort;
        if_num ++;
        if ( if_num > (MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1) )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            VOS_Free(_pulIfArray); 

            return no_match;
        }
        ret = 1;
    }
    END_TDM_PARSE_VOICE_LINK_LIST_TO_PORT();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}


CMD_NOTIFY_REFISTER_S stCMD_Tdm_Voice_Link_Port_List_Check =
{
    "<voice_link_port_list>",
    Tdm_Check_Voice_Link_Port_List,
    0
};

ULONG * TDM_Parse_Voice_Link_Port_List( CHAR * pcPort_List )
{
    ULONG ulState = STATE_PS;
    /*ULONG ulSlot = 0;*/
    CHAR digit_temp[ 12 ];
    ULONG ulInterfaceList[ MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1 ];
    ULONG ulPortS = 0;
    ULONG ulPortE = 0;
    CHAR cToken;
    ULONG iflist_i = 0;
    ULONG list_i = 0;
    ULONG temp_i = 0;
    /*ULONG ulIfindex;*/
    ULONG ulListLen = 0,i=0;
    CHAR * list;
    /*ULONG ulType;
    ULONG ulSwport = 0;
    ULONG ulSwSlot=0;*/

   /* VOS_MemZero( ulInterfaceList, MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST * 4 );*/
    for(i=0;i<MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1;i++)
    	{
		ulInterfaceList[i]=MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1;
	}
    ulListLen = VOS_StrLen( pcPort_List );
    list = VOS_Malloc( ulListLen + 2, MODULE_RPU_CLI );
    if ( list == NULL )
    {
        return NULL;
    }
    VOS_StrnCpy( list, pcPort_List, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken !=  0 )
    {
        switch ( ulState )
        {            
            case STATE_PS:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );

                    if ( ulPortS > MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST )
                    {
                        goto error;
                    }
			/*输出端口号**/
                    if ( 0 <= ulPortS )
                    {
                        ulInterfaceList[ iflist_i ] = ulPortS;
                        iflist_i++;
                    }
                    if ( iflist_i > (MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1))
                    {
                        goto error;
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( ULONG ) VOS_AtoL( digit_temp );
                    temp_i = 0;
                    ulState = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PE:
                if ( vos_isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( vos_isspace( cToken ) )
                {}
                else if ( cToken == ',' )
                {
                    ULONG i;
                    ULONG i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortE = ( ULONG ) VOS_AtoL( digit_temp );
                    if ( ulPortE > ulPortS )
                    {
                        i_s = ulPortS;
                        i_e = ulPortE;
                    }
                    else
                    {
                        i_s = ulPortE;
                        i_e = ulPortS;
                    }
                    
                    for ( i = i_s;i <= i_e;i++ )
                    {
                        if ( i<START_TDM_VOICE_PORT || i > END_TDM_VOICE)
                        {
                            goto error;
                        }

                        if ( 0 <=i )
                        {
                            ulInterfaceList[ iflist_i ] = i;
                            iflist_i++;
                        }
                        if ( iflist_i > (MAX_TDM_VOICE_LINK_NUMBER_IN_PORT_LIST+1) )
                        {
                            goto error;
                        }
                    }
                    temp_i = 0;
                    ulState = STATE_PS;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    VOS_Free( list );
    if ( iflist_i == 0 )
    {
        return NULL;
    }
    else
    {
        list = VOS_Malloc( ( iflist_i + 1 ) * 4, MODULE_RPU_IFM );
        if ( list == NULL )
        {
            return NULL;
        }
        VOS_MemZero( list, ( iflist_i + 1 ) * 4 );
        VOS_MemCpy( list, ulInterfaceList, iflist_i * 4 );
	*(int *)&list[iflist_i * 4] = END_PARSE_LIST;
        return ( ULONG * ) list;
    }
error:
    VOS_Free( list );
    return NULL;
}



DEFUN  (
    into_epon_tdm_node,
    into_epon_tdm_node_cmd,
    "tdm <slot/port>",
    "Select a tdm_port to config\n"
    "Specify tdm_port interface's slot and port\n")
{	ULONG rc;
	ULONG  	TDM_Slot = 0, TDM_Port = 0 ;
	CHAR    prompt[64] = { 0 };
	ULONG   ulIFIndex = 0;
	CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
	
	VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
	VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%s", "tdm", argv[0] );
	
	IFM_ParseSlotPort( argv[0], &TDM_Slot, &TDM_Port );

	if(TdmCardSlotPortCheckByVty(TDM_Slot, TDM_Port, vty) != ROK )
		return(CMD_WARNING);

	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(TDM_Slot) == MODULE_TYPE_NULL )
		{
		vty_out(vty," %% slot %d is not inserted\r\n", TDM_Slot);
		return( CMD_WARNING );
		}
	/* 2  板类型检查*/
	if(SlotCardIsTdmSgBoard(TDM_Slot) != ROK )
		{
		vty_out(vty," %% slot %d is not tdm-sig card\r\n", TDM_Slot);
		return( CMD_WARNING );
		}
	/* tdm fpga is inserted  */
	if(getTdmChipInserted((unsigned char)(TDM_Slot-1),(unsigned char)(TDM_Port-1)) != TDM_FPGA_EXIST)
		{
		vty_out(vty,"  %% %s%d/%d is not inserted\r\n", GetGFA6xxxTdmNameString(),TDM_Slot, TDM_Port);
		return(CMD_WARNING);
		}
	
	ulIFIndex = IFM_PON_CREATE_INDEX( TDM_Slot, TDM_Port, 0, 0);
	if ( ulIFIndex == 0 )
		{
       		 vty_out( vty, "  %% Can not find interface %s\r\n", ifName );
        		return CMD_WARNING;
    		}	
	
	vty->node = TDM_CLI_NODE;
	vty->index = ( VOID * ) ulIFIndex;
	
	VOS_StrCpy( prompt, "%s(epon-" );
	VOS_StrCat( prompt, ifName );
	VOS_StrCat( prompt, ")#" );
	vty_set_prompt( vty, prompt );

	rc=VOS_OK;
	return rc;

}
#ifdef  _GFA6xxx_VOICE_SG_
#endif
/*一下命令是用于查询信令网关信息*/
DEFUN(
		    set_hdlc_link_master_slave,
		    set_hdlc_link_master_slave_cmd,
		    "hdlc-link master <1-8> slave <1-8>",
		    "set hdlc link master and slave parameter\n"
		    "the master hdlc link\n"
		    "please input the master hdlc link value\n"
		    "the slave hdlc link\n"
		    "please input the slave hdlc link value\n"
)
{
	STATUS rc=VOS_ERROR;
	ULONG ulIfIndex = 0;
	STATUS result_msg;
	USHORT   master_num,slaver_num;
	ULONG  e1num_mas=0,e1num_sla=0;
	ULONG  TDM_slotid ,TDM_portid,TDM_onu;
	sgtable_row_entry  entry;
	if (argc!=2)
		/*vty_out (vty,"%% the parameter error\r\n");*/
		return CMD_WARNING;
	master_num=VOS_AtoL(argv[0]);
	slaver_num=VOS_AtoL(argv[1]);

	rc=((master_num==slaver_num)?VOS_ERROR:VOS_OK);
	if(rc!=VOS_OK)
		{
		vty_out (vty,"\r\n %% the master link and the slave link can not be the same e1\r\n");
		return(CMD_WARNING);
		}
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);
	
	result_msg=tdm_sgTable_set(LEAF_EPONSGHDLCMASTERE1, TDM_portid, master_num);
	
	if (result_msg!=VOS_OK)
		vty_out(vty,"\r\n%%  the master hdlc link set failure\r\n");
	
	result_msg=tdm_sgTable_get(TDM_portid, &entry);
	if(result_msg!=VOS_OK)
		vty_out (vty,"\r\n%% get the tdm port table failure\r\n");
	else 
		e1num_mas=entry.mastere1;
	
	result_msg=tdm_sgTable_set( LEAF_EPONSGHDLCSLAVEE1, TDM_portid, slaver_num);
	
	if(result_msg!=VOS_OK)
		vty_out(vty,"\r\n %% the slave hdlc link set failure\r\n");
	
	result_msg=tdm_sgTable_get(TDM_portid, & entry);
	
	if(result_msg!=VOS_OK)
		vty_out (vty,"\r\n %% get the tdm port table failure,please try again\r\n");
	else 
		e1num_sla=entry.slavee1;
	/*
	rc=((e1num_mas==e1num_sla)?VOS_ERROR:VOS_OK);
	if(rc!=VOS_OK)
		vty_out (vty,"\r\n %% the master num and the slave num can not be in the same e1 cluster\r\n");
	sys_console_printf ("\r\nthe  master num %d",master_num);
	sys_console_printf ("\r\nthe slave num %d",slaver_num);*/
	return  rc;
			
}

/*此处需要按照文档进行修改。*/
DEFUN(
		   show_status,
		   show_status_cmd,
		   "show status",
		   "show the appointed tdm status\n"
		   "show tdm status\n"
)
{
	STATUS rc=VOS_ERROR;
	ULONG ulIfIndex = 0;
	STATUS result_msg;
	ULONG  TDM_slotid ,TDM_portid,TDM_onu,PotsEnableList,onunum;
	sgtable_row_entry  pentry;
	int i=0,j,k=0,m=0,n=0;
	ulong devIdx[256],potslist[32],potsnum=0,idxs[3],potsboard=1,admstatus=0;
	unsigned long OnuDevIdx=0,handoffCounter=0,handonConuter=0;
	potsporttable_row_entry  entry;
	ULONG operstatus,uldevidx=1;
	e1porttable_row_entry e1entry;
	char str[64];
	
	/*if(argc!=0)
		vty_out(vty,"\r\n%% parameter error\r\n");*/
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);

	VOS_MemZero(devIdx, sizeof(devIdx));
	VOS_MemZero(potslist, sizeof(potslist));
	VOS_MemZero(idxs, sizeof(idxs));

	/*slot_port_2_swport(slotid, portid, & ulSwSlot, &ulSwport);

	ifIdex=IFM_ETH_CREATE_INDEX( ulSwSlot,  ulSwport);
	
	IFM_GetIfStatusApi(ifIdex, & operstatus);
	IFM_GetIfAdminStatusApi(ifIdex, &admstatus);*/
	getEthPortOperStatus(uldevidx, TDM_slotid, TDM_portid, & operstatus);
	getEthPortAdminStatus(uldevidx, TDM_slotid, TDM_portid, &admstatus);
	
	vty_out (vty,"\r\n admin status is:%s",adminstatus[(admstatus-1)]);
	vty_out (vty,"\r\n physical status is:%s",opertstatus[(operstatus-1)]);
	
	result_msg=tdm_sgTable_get( TDM_portid, & pentry);
	if(result_msg==VOS_OK)
		{
			vty_out (vty,"\r\n master link: %d",pentry.mastere1);
			vty_out (vty,"\r\n slave link: %d",pentry.slavee1);
			vty_out (vty,"\r\n sync-source: %d",pentry.e1clk);

			vty_out (vty,"\r\n voice vlan :%s",(vlanenablestatus[(pentry.vlanen)-1]));
			if(pentry.vlanen== ENABLE)
				{
					vty_out (vty,"\r\n vlan id :%d",pentry.vlanid);
			/*vty_out (vty,"\r\n vlan cluster id is :%d",pentry.clusterIdx);*/
					vty_out (vty,"\r\n vlan priority :%d",pentry.vlanpri);
				}
			GetAllOnuBelongToSG((uchar)TDM_slotid, (uchar)TDM_portid, &onunum, devIdx);
			
			vty_out (vty,"\r\n max voice-onu supported :%d",MAXONUPERSG);
			vty_out (vty,"\r\n voice-onu configed:%d",onunum);

			for(i=0;i<onunum;i++)
				{
					OnuDevIdx = devIdx[i] >> 8;
					GetOnuPotsLinkAll(OnuDevIdx, &PotsEnableList);
					for(j=0;j<MAXONUPOTS;j++)
						{
							if((PotsEnableList & (1<<j))!=0)
							{	potsnum++;
								potslist[k++]=j+1;
							}
						}
					/*vty_out (vty,"\r\npots link :\r\n");
					for(m=0;m<k;m++)
						{	
							vty_out (vty," %d   ",potslist[m]);
							if((m+1)%5==0)
							vty_out (vty,"\r\n");
						}*/
				}
			vty_out (vty,"\r\n max voice-link supported :%d",MAXLOGICALSGPOTS);
			vty_out (vty,"\r\n voice-link configed: %d",potsnum);
			for(m=0;m<=k;m++)
				{
					idxs[0]=OnuDevIdx;
					idxs[1]=potsboard;
					idxs[2]=potslist[m];
			
				result_msg=tdm_potsPortTable_get(idxs , &entry);
				if(result_msg==VOS_OK)
					{
						if(entry.opstatus==2)
							handoffCounter++;
						else 
							handonConuter++;
					}
				}
			vty_out (vty,"\r\n off-hook conuter:%d",handoffCounter);
			vty_out (vty,"\r\n on-hook counter:%d",handonConuter);
			/*还有需要输出的信息，需要再添加。*/
			rc=VOS_OK;

			for(n=0;n<MAXE1NUM;n++)
				{
					idxs[0]=uldevidx;
					idxs[1]=TDM_slotid;
					if(TDM_portid==2)
						idxs[2]=1*MAXE1NUM+(n+1);
					else if(TDM_portid==3)
						idxs[2]=2*MAXE1NUM+(n+1);
					else 
						idxs[2]=(n+1);
					tdm_e1portTable_get(idxs , &e1entry);
					vty_out(vty,"\r\n e1 link %d status %s",idxs[2],get_alarm(e1entry.almstatus, str));
				}
		}
	vty_out (vty,"\r\n");
	return  rc;
}


DEFUN (
		    sync_source_set,
		    sync_source_set_cmd,
		    "sync-source <1-8>",
		    "set e1 clock\n"
		    "please input the clock trace value\n"
		 )
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG TDM_portid,TDM_slotid,e1clocknum,TDM_onu;
	ULONG ulIfIndex = 0;
	/*if(argc!=1)
		vty_out(vty,"\r\n%% parameter error\r\n");*/

	e1clocknum=VOS_AtoL(argv[0]);
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);
	
	result_msg=tdm_sgTable_set(LEAF_EPONSGHDLCE1CLK, TDM_portid, e1clocknum);
	if(result_msg!=VOS_OK)
		{
			vty_out(vty,"\r\nset %s%d/%d sync-clock from e1-link%d failure\r\n",GetGFA6xxxTdmNameString(),TDM_slotid,TDM_portid,e1clocknum);
			/*vty_out(vty,"the hdlc link e1 clock is %d",e1clocknum);*/
			vty_out (vty,"\r\n");
		}
	rc=VOS_OK;
	return rc;
}

#ifdef   _GFA6xxx_VOICE_VLAN_
#endif
/*一下命令是设置和取消vlan */
DEFUN (
		   set_voice_vlan,
		   set_voice_vlan_cmd,
		   "voice-vlan priority <0-7> vlan-id <1-4094>",
		   "set voice vlan\n"
		   "set the voice vlan priority\n"
		   "please input the voice vlan priority value\n"
		   "set the voice vlan id\n"
		   "piease input the voice vlan id value\n"
)
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG TDM_portid,TDM_slotid,pri,vlanid,TDM_onu;
	ULONG ulIfIndex = 0;
	/*if(argc!=2)
		vty_out(vty,"\r\n%% parameter error\r\n");*/

#if  0 
	if ( !VOS_StrCmp( argv[ 0 ], "enable" ) )
	  	{
	  	ulable = ENABLE;
	  	}
	  else {
	  	ulable = DISABLE;
	  	}
#endif  
	pri=VOS_AtoL(argv[0]);
	vlanid=VOS_AtoL(argv[1]);

	/*sys_console_printf ("\r\n the pri %d",pri);
	sys_console_printf ("\r\n the vlanid %d",vlanid);*/

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);
/*
	sys_console_printf ("\r\nthe slotid is %d",slotid);
	sys_console_printf ("\r\nthe prot is %d",portid);
	sys_console_printf ("\r\nthe onu is %d",onudevidx);
*/
	if(MN_Vlan_Check_exist(vlanid ) != VOS_OK )
		{
		vty_out(vty,"\r\nvlan(vid=%d) is not existed in BCM switch, please created first\r\n", vlanid );
		return( CMD_WARNING );
		}

	result_msg=SetTdmSGVoiceVlanEnable(TDM_slotid, TDM_portid, pri, vlanid);
#if  0
	result_msg=tdm_sgTable_get( portid, &entry );
	if(result_msg!=VOS_OK)
		sys_console_printf("\r\n  get the data from sgtable failure \r\n");
	
	lbuf[0]=entry.mastere1;
	lbuf[1]=entry.slavee1;
	lbuf[2]=entry.e1clk;
	lbuf[3]=1;
	lbuf[4]=pri;
	lbuf[5]=vlanid;
	
	result_msg=tdm_sgTable_rowset(portid, lbuf);
	if(result_msg!=VOS_OK)
		sys_console_printf("\r\n set the voice vlan pri and vlanid failure \r\n");
	else 
		sys_console_printf("\r\n set the voice vlan pri and vlanid success \r\n");
	rc=result_msg;
#endif
	if(result_msg ==TDM_VM_ERR)
		{	
		vty_out(vty,"\r\nset the voice vlan pri and vlanid failure\r\n");
		}
	else if(result_msg==TDM_VM_ANOTHER_VID_EXIT)
		vty_out (vty,"\r\n another vid exist\r\n");
	else if( result_msg == TDM_VM_VID_EXIT )
		vty_out(vty,"\r\n vid %d and priority %d already config to %s%d/%d\r\n",vlanid, pri,GetGFA6xxxTdmNameString(),TDM_slotid, TDM_portid);
	else
		;
	vty_out (vty,"\r\n");
	return rc;
	
}

DEFUN(
		   undo_voice_vlan,
		   undo_voice_vlan_cmd,
		   "undo voice-vlan",
		   "undo voice vlan\n"
		   "voice vlan disable\n"
)
{
	STATUS result_msg;
	ULONG TDM_portid,TDM_slotid,TDM_onu;
	ULONG ulIfIndex = 0;

#if  0 
	if ( !VOS_StrCmp( argv[ 0 ], "enable" ) )
	  	{
	  	ulable = ENABLE;
	  	}
	  else {
	  	ulable = DISABLE;
	  	}
#endif  

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);
/*
	sys_console_printf ("\r\nthe slotid is %d",slotid);
	sys_console_printf ("\r\nthe prot is %d",portid);
	sys_console_printf ("\r\nthe onu is %d",onudevidx);
*/
	result_msg=SetTdmSGVoiceVlanDisable(TDM_slotid, TDM_portid);
#if  0
	result_msg=tdm_sgTable_get( portid, &entry );
	if(result_msg!=VOS_OK)
		sys_console_printf("\r\n  get the data from sgtable failure \r\n");
	
	lbuf[0]=entry.mastere1;
	lbuf[1]=entry.slavee1;
	lbuf[2]=entry.e1clk;
	lbuf[3]=2;
	lbuf[4]=pri;
	lbuf[5]=vlanid;
	
	result_msg=tdm_sgTable_rowset(portid, lbuf);
	if(result_msg!=VOS_OK)
		sys_console_printf("\r\n set the voice vlan pri and vlanid failure \r\n");
	else 
		sys_console_printf("\r\n set the voice vlan pri and vlanid success \r\n");
	rc=result_msg;
#endif
	if(result_msg!=VOS_OK)
		vty_out (vty,"\r\ndisable voice vlan failure\r\n");

	return  CMD_SUCCESS;
}

#ifdef  _GFA6xxx_VOICE_HDLC_
#endif
/*显示信令网关统计信息*/
DEFUN(
		   show_statistics_hdlc,
		   show_statistics_hdlc_cmd,
		   "show statistics hdlc",
		   "show the appointed hdlc status\n"
		   "show tdm hdlc information\n"
		   "display the result\n"
)
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG TDM_slotid,TDM_portid,TDM_onu;
	hdlcstattable_row_entry  pentry;
	/*if(argc!=0)
		vty_out(vty,"\r\n%% parameter error\r\n");*/
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);

	
	result_msg=tdm_hdlcStatTable_get(TDM_portid, & pentry);
	if(result_msg==VOS_OK)
		{
			vty_out (vty,"\r\nstatus changed notify frames: %d",pentry.potsstatusnotify);
			vty_out (vty,"\r\nstatus synchronization frames: %d",pentry.synframes);
			vty_out (vty,"\r\ndownstream ring frames: %d",pentry.ringnotify);
			vty_out (vty,"\r\ndownstream ring status synchronization frames:%d",pentry.ringsync);
			vty_out (vty,"\r\ndownstream pole reversed frames: %d",pentry.rolereverse);
			vty_out (vty,"\r\ndownstream Timeslot allocation frames: %d",pentry.tsalloc);
			vty_out (vty,"\r\ndownstream Timeslot released frames %d",pentry.tsrelease);
			vty_out (vty,"\r\nupstream test frames %d",pentry.uptest);
			vty_out (vty,"\r\nupstream idle frames %d",pentry.upidle);
			vty_out (vty,"\r\ndownstream testing frames %d",pentry.downtest);
			vty_out (vty,"\r\ndownstream idle frames %d",pentry.downidle);
			/*还有需要输出的信息，需要再添加。*/
			rc=VOS_OK;
		}
	vty_out (vty,"\r\n");
	return  rc;
}

#ifdef  _GFA6xxx_VOICE_ONU_
#endif
/*显示信令网关下配置的onu */
QDEFUN (
		   show_voice_onu_list,
		   show_voice_onu_list_cmd,
		   "show voice-onu-list",
		   "show voice onu list\n"
		   "display the result\n",
		   &eventQueId

)
{	
	STATUS rc=VOS_ERROR;
	STATUS result_msg=VOS_ERROR;
	ULONG ulIfIndex = 0;
	ULONG idxs[2]={0};
	ULONG tdmslot,tdmsgidx,tdmonu;
	ULONG devIdx[MAXONUPERSG];
	ULONG PON_slot,PON_port,PON_onuid,onunum=0,pValLen=0;
	int i;	
	tdmonutable_row_entry pentry;
	unsigned char OnuString[100] = {'\0'};
	char pValBuf[128];
	
	/*if (argc !=0)		
		vty_out(vty,"\r\n%% parameter error");*/
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&tdmsgidx , (ULONG *)&tdmonu) ;
	
	VOS_MemZero(devIdx, sizeof(devIdx));

	result_msg=GetAllOnuBelongToSG((uchar)tdmslot, (uchar)tdmsgidx, &onunum, devIdx);
	
		if(result_msg == VOS_OK)
			{
				VOS_MemZero(OnuString, sizeof(OnuString));
				vty_out (vty,"total number of voice ONU in %s%d/%d: %d\r\n",GetGFA6xxxTdmNameString(),tdmslot, tdmsgidx,onunum);
				vty_out (vty,"%s%d/%d voice ONU list:\r\n",GetGFA6xxxTdmNameString(),tdmslot,tdmsgidx);
				vty_out (vty,"onuidx  user-name         serviceStatus\r\n");/*modify 2008-4-9增加service status*/
				/*vty_out (vty,"----------------------------------------------\r\n");*/

				for(i=0;i<onunum;i++)
					{
						/*if( devIdx[i] == 0 )
							continue;*/
						devIdx[i] = (devIdx[i]>>8);
						PON_slot=GET_PONSLOT(devIdx[i])/*devIdx[i]/10000*/;
						PON_port=GET_PONPORT(devIdx[i])/*(devIdx[i]%10000)/1000*/;
						PON_onuid=GET_ONUID(devIdx[i])/*devIdx[i]%1000*/;
						idxs[0]=tdmsgidx;
						idxs[1]=devIdx[i];

						VOS_MemZero(OnuString, 100);
						getDeviceName(devIdx[i], pValBuf, & pValLen);
						sprintf(OnuString,"%d/%d/%d",PON_slot,PON_port,PON_onuid);
						vty_out (vty,"%-8s",OnuString);
						if(pValLen >= 20 )
							pValBuf[20] = '\0';
						else pValBuf[pValLen] =  '\0';
						sprintf(OnuString,"%s",pValBuf);
						vty_out (vty,"%-18s",OnuString);
						if(tdm_tdmOnuTable_get( idxs ,  &pentry)==VOS_OK)
							{
								sprintf(OnuString,"%s",ServiceStatus[(pentry.serviceIdx-1)]);
								vty_out(vty,"%s\r\n",OnuString);
							}
						/*if((i+1)%2==0)
							vty_out (vty,"\r\n");*/
					}
				vty_out (vty,"\r\n");
				rc=VOS_OK;
			}
		else 

			{
				/*sys_console_printf ("\r\n get the onu from tdm failure \r\n");*/
				return rc;
			}

	return rc;
}

#ifdef  _GFA6xxx_VOICE_E1_LINK_
#endif
/*与e1 相关的信息:包括crc,loopback,alarm的设置取消，显示*/
DEFUN (
		   set_e1_link_crc,
		   set_e1_link_crc_cmd,
		   "e1-link crc [enable|disable] <e1_list>",
		   "e1 link crc enable\n"
		   "set e1 link crc enable\n"
		   "set e1 link crc disable\n"
		   "please input the e1 list\n"
		   "the e1 list value\n"
		   )
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG TDM_slotid,TDM_portid,TDM_onu;
	ULONG ulEnable;
	ULONG idxs[3],e1list[MAXE1NUM],ule1list,counte1list=0;
	int i=0;
	
	/*if(argc!=2)
		vty_out(vty,"\r\n%% parameter error\r\n");*/

	VOS_MemZero(e1list, sizeof(e1list));
	
    if(!VOS_StrCmp(argv[0],"enable"))
    {
         ulEnable = CRC_ENABLE;
    }
    else if(!VOS_StrCmp(argv[0],"disable"))
    {
         ulEnable = CRC_DISABLE;
    }
    else
    {
        /*vty_out(vty,"%% Parameter is error.\r\n");*/
        return CMD_WARNING;
    }
	

	BEGIN_TDM_PARSE_E1_LIST_TO_PORT( argv[ 1], ule1list)
	{
		e1list[counte1list]=ule1list;
		counte1list++;
	}
	END_TDM_PARSE_E1_LIST_TO_PORT();

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);

	for(i=0;i<counte1list;i++)
		{
			idxs[0]=1;
			idxs[1]=TDM_slotid;
			if(TDM_portid==2)
				idxs[2]=1*MAXE1NUM+e1list[i];
			else if(TDM_portid==3)
				idxs[2]=2*MAXE1NUM+e1list[i];
			else
				idxs[2]=e1list[i];
		
			result_msg=tdm_e1portTable_set(LEAF_EPONE1PORTCRCENABLE, idxs , ulEnable);
			if(result_msg!=VOS_OK)
				{
					vty_out (vty,"\r\n set the e1 link crc failure\n");
				}
			rc=result_msg;
		}
	vty_out(vty,"\r\n");
	return rc;
}
#if  0
DEFUN(
		   undo_e1_link_crc_enable,
		   undo_e1_link_crc_enable_cmd,
		   "undo e1-link crc enable <e1_list>",
		   "e1 link crc disable\n"
		   "undo e1 link crc enable \n"
		   "set e1 link crc disable\n"
		   "please input the e1 list\n"
		   "the e1 list value\n"
)
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG slotid,portid,onudevidx,e1idx;
	ULONG idxs[3];
	if(argc!=1)
		vty_out(vty,"\r\n%% parameter error\r\n");
	
	e1idx=VOS_AtoL(argv[0]);

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotid, (ULONG *)&portid , (ULONG *)&onudevidx);
	
	idxs[0]=1;
	idxs[1]=slotid;
	idxs[2]=e1idx;

	result_msg=tdm_e1portTable_set( LEAF_EPONE1PORTCRCENABLE, idxs , 0);

	if(result_msg==VOS_OK)
		{
			vty_out (vty,"\r\n set the e1 link crc disable success\n");
		}
	rc=result_msg;
	return rc;
}

#endif

DEFUN(
		  set_e1_link_loopback,
		  set_e1_link_loopback_cmd,
		  "e1-link loopback [enable|disable] <e1_list>",
		  "e1 link loopback enable\n"
		  "set e1 link loopback enable\n"
		  "set e1 link loopback disable\n"
		  "please input the e1 list\n"
		  "the e1 list valut\n"
)
{
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG TDM_slotid,TDM_portid,TDM_onu;
	ULONG idxs[3],ulEnable,ule1list,counte1list=0,e1list[MAXE1NUM];
	int i=0;
	/*if(argc!=2)
		vty_out(vty,"\r\n%% parameter error\r\n");*/

	if(!VOS_StrCmp(argv[0],"enable"))
    	{
        	 ulEnable = E1_LOOP_ENABLE;
    	}
    else if(!VOS_StrCmp(argv[0],"disable"))
    	{
        	 ulEnable = E1_LOOP_DISABLE;
    	}
    else
    	{
       	/* vty_out(vty,"%% Parameter is error.\r\n");*/
        	return CMD_WARNING;
    	}



	BEGIN_TDM_PARSE_E1_LIST_TO_PORT( argv[ 1], ule1list)
		{
			e1list[counte1list]=ule1list;
			counte1list++;
		}
	END_TDM_PARSE_E1_LIST_TO_PORT();

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);

	for(i=0;i<counte1list;i++)
		{
			idxs[0]=1;
			idxs[1]=TDM_slotid;
			if(TDM_portid==2)
				idxs[2]=1*MAXE1NUM+e1list[i];
			else if(TDM_portid==3)
				idxs[2]=2*MAXE1NUM+e1list[i];
			else
				idxs[2]=e1list[i];

			result_msg=tdm_e1portTable_set(LEAF_EPONE1PORTLOOPBACKCTRL, idxs , ulEnable);
			if(result_msg!=VOS_OK)
			{
				vty_out (vty,"\r\n set the e1 link loopback failure\r\n");
			}

		}
	return CMD_SUCCESS;
}

DEFUN(
		    show_e1_link_loopback,
		    show_e1_link_loopback_cmd,
		    "show e1-link loopback {<e1_list>}*1",
		    "show e1-link loopback\n"
		    "e1-link loopback status\n"
		    "e1-link loopback\n"
		    "e1-link number\n"
		    
)
{
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG TDM_slotid,TDM_portid,TDM_onu;
	ULONG idxs[3],ule1list,counte1list=0,e1list[MAXE1NUM];
	int i=0;
	e1porttable_row_entry pEntry;
	UCHAR resultData[MAXE1NUM]={0};
	unsigned char OnuString[100] = {'\0'};

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);
	

	if(argc!=1)

	{
	for(i=0;i<MAXE1NUM;i++)
		{
			idxs[0]=1;
			idxs[1]=TDM_slotid;
			if(TDM_portid==2)
				idxs[2]=1*MAXE1NUM+(i+1);
			else if(TDM_portid==3)
				idxs[2]=2*MAXE1NUM+(i+1);
			else
				idxs[2]=(i+1);

			result_msg=tdm_e1portTable_get(idxs , & pEntry);
			if(result_msg!=VOS_ERROR)
				resultData[i]=pEntry.lpbctrl;
			
				
		}
		/*for(i=0;i<MAXE1NUM;i++)
			{
				vty_out(vty,"resultData %d=%d\r\n",(i+1),resultData[i]);
			}*/
		vty_out (vty,"\r\ne1-link                      loopstatus\r\n");
		/*vty_out (vty,"----------------------------------------------\r\n");*/

			for(i=0;i<MAXE1NUM;i++)
			{	if(TDM_portid==2)
				{
					sprintf(OnuString,"%d",(1*MAXE1NUM+(i+1)));
					vty_out (vty,"%-30s",OnuString);
					if(resultData[i]==1)
					vty_out (vty,"%s\r\n",loopbackstatus[resultData[i]]);
					else
					vty_out(vty,"%s\r\n",loopbackstatus[0]);
				}
				else if(TDM_portid==3)
				{
					sprintf(OnuString,"%d",(2*MAXE1NUM+(i+1)));
					vty_out (vty,"%-30s",OnuString);
					if(resultData[i]==1)
					vty_out (vty,"%s\r\n",loopbackstatus[resultData[i]]);
					else
					vty_out(vty,"%s\r\n",loopbackstatus[0]);
				}
				else 
				{sprintf(OnuString,"%d",(i+1));
				vty_out (vty,"%-30s",OnuString);
				if(resultData[i]==1)
				vty_out (vty,"%s\r\n",loopbackstatus[resultData[i]]);
				else
				vty_out(vty,"%s\r\n",loopbackstatus[0]);
				}
			}
		

	}
	else
	{
	BEGIN_TDM_PARSE_E1_LIST_TO_PORT( argv[ 0], ule1list)
		{
			e1list[counte1list]=ule1list;
			counte1list++;
		}
	END_TDM_PARSE_E1_LIST_TO_PORT();
	/*for(i=0;i<counte1list;i++)
		{
			vty_out(vty,"data %d=%d\r\n",(i+1),e1list[i]);
		}
		*/
	
		for(i=0;i<counte1list;i++)
		{
			idxs[0]=1;
			idxs[1]=TDM_slotid;
			if(TDM_portid==2)
				idxs[2]=1*MAXE1NUM+e1list[i];
			else if(TDM_portid==3)
				idxs[2]=2*MAXE1NUM+e1list[i];
			else
				idxs[2]=e1list[i];

			result_msg=tdm_e1portTable_get(idxs , & pEntry);
			if(result_msg!=VOS_ERROR)
				resultData[i]=pEntry.lpbctrl;
				
		}
		vty_out (vty,"%-30s","\r\ne1-link                        loopstatus\r\n");
		/*vty_out (vty,"----------------------------------------------\r\n");*/
		for(i=0;i<counte1list;i++)
			{	if(TDM_portid==2)
				{
					sprintf(OnuString,"%d",(1*MAXE1NUM+e1list[i]));
					vty_out (vty,"%-30s",OnuString);
					if(resultData[i]==1)
					vty_out (vty,"%s\r\n",loopbackstatus[resultData[i]]);
					else
					vty_out(vty,"%s\r\n",loopbackstatus[0]);
				}
				else if(TDM_portid==3)
				{
					sprintf(OnuString,"%d",(2*MAXE1NUM+e1list[i]));
					vty_out (vty,"%-30s",OnuString);
					if(resultData[i]==1)
					vty_out (vty,"%s\r\n",loopbackstatus[resultData[i]]);
					else
					vty_out(vty,"%s\r\n",loopbackstatus[0]);
				}
				else 
				{sprintf(OnuString,"%d",(i+1));
				vty_out (vty,"%-30s",OnuString);
				if(resultData[i]==1)
				vty_out (vty,"%s\r\n",loopbackstatus[resultData[i]]);
				else
				vty_out(vty,"%s\r\n",loopbackstatus[0]);
				}
			}
		
	
	}
	return CMD_SUCCESS;
		
}
#if  0
DEFUN(
		  undo_e1_link_loopback_enable,
		  undo_e1_link_loopback_enable_cmd,
		  "undo e1-link loopback enable <e1_list>",
		  "e1 link loopback disable\n"
		  "undo e1 link loopback enable\n"
		  "set e1 link loopback disable\n"
		  "please input the e1 list\n"
		  "the e1 list value \n"
)
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG slotid,portid,onudevidx,e1idx;
	ULONG idxs[3];
	if(argc!=1)
		vty_out(vty,"\r\n%% parameter error\r\n");
	
	e1idx=VOS_AtoL(argv[0]);

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotid, (ULONG *)&portid , (ULONG *)&onudevidx);
	
	idxs[0]=1;
	idxs[1]=slotid;
	idxs[2]=e1idx;

	result_msg=tdm_e1portTable_set(LEAF_EPONE1PORTLOOPBACKCTRL, idxs , 0);

	if(result_msg==VOS_OK)
		{
			vty_out (vty,"\r\n set the e1 link loopback enable success\n");
		}
	rc=result_msg;
	return rc;
}

#endif

#if   0
DEFUN(
		   set_e1_link_alarm_mask,
		   set_e1_link_alarm_mask_cmd,
		   "e1-link alarm-mask <e1_list> [all|los|rai|ais|smf|lof|crc-3|crc-6]",
		   "e1 link alarm mask set\n"
		   "please input the e1 list\n"
		   "the e1 list value\n"
		   "all e1 list \n"
		   "e1 loss of signal\n"
		   "e1 rcv ais \n"
		   "e1alarm mask AIS\n"
		   "e1 SMF LOF\n"
		   "e1 LOF\n"
		   "crc 3\n"
		   "crc 6\n"
)
{
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG slotid,portid,onudevidx,alarmmask;
	ULONG idxs[3],ule1list,counte1list=0,e1list[MAXE1NUM];
	int i=0;
	
	if(argc!=2)
		vty_out(vty,"\r\n%% parameter error\r\n");

	VOS_MemZero(e1list, sizeof(e1list));
	
	BEGIN_TDM_PARSE_E1_LIST_TO_PORT( argv[0], ule1list)
		{
			e1list[counte1list]=ule1list;
			counte1list++;
		}
	END_TDM_PARSE_E1_LIST_TO_PORT();

	if (!VOS_StrCmp((CHAR *)argv[1], "all"))
		alarmmask=E1_ALM_ALL;
	else if(!VOS_StrCmp((CHAR *)argv[1], "los"))
		alarmmask=E1_ALM_LOS;
	else if (!VOS_StrCmp((CHAR *)argv[1], "rai"))
		alarmmask=E1_ALM_RAI;
	else if (!VOS_StrCmp((CHAR *)argv[1], "ais"))
		alarmmask=E1_ALM_AIS;
	else if (!VOS_StrCmp((CHAR *)argv[1], "smf"))
		alarmmask=E1_ALM_SMF;
	else if (!VOS_StrCmp((CHAR *)argv[1], "lof"))
		alarmmask=E1_ALM_LOF;
	else if (!VOS_StrCmp((CHAR *)argv[1], "crc-3"))
		alarmmask=E1_ALM_CRC3;
	else if (!VOS_StrCmp((CHAR *)argv[1], "crc-6"))
		alarmmask=E1_ALM_CRC6;
	else
		return CMD_WARNING;
	
		

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotid, (ULONG *)&portid , (ULONG *)&onudevidx);
	

	for(i=0;i<counte1list;i++)
		{	
			idxs[0]=1;
			idxs[1]=slotid;
			if(portid==2)
				idxs[2]=1*MAXE1NUM+e1list[i];
			else if(portid==3)
				idxs[2]=2*MAXE1NUM+e1list[i];
			else
				idxs[2]=e1list[i];
			result_msg=tdm_e1portTable_set( LEAF_EPONE1PORTALARMMASK, idxs , alarmmask);

			if(result_msg!=VOS_OK)
				{
					vty_out (vty,"\r\n set the e1 link alarm mask failure\r\n");
				}
		}
	
	return CMD_SUCCESS;
}

DEFUN(
		   undo_e1_link_alarm_mask,
		   undo_e1_link_alarm_mask_cmd,
		   "undo e1-link alarm-mask <e1_list> [all|los|rai|ais|smf|lof|crc-3|crc-6]",
		   "undo e1 link alarm mask \n"
		   "the e1 port table \n"
		   "set the e1 port table alarm mask \n"
		   "the e1 list value \n"
		   "all e1 list \n"
		   "e1 loss of signal \n"
		   "e1 rcv ais \n"
		   "e1alarm mask AIS \n"
		   "e1 SMF LOF \n"
		   "e1 LOF \n"
		   "crc 3 \n"
		   "crc 6 \n"
		
)
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG slotid,portid,onudevidx,alarmmask;
	ULONG idxs[3],ule1list,counte1list=0,e1list[MAXE1NUM];
	int i=0;
	
	if(argc!=2)
		vty_out(vty,"\r\n%% parameter error\r\n");

	VOS_MemZero(e1list, sizeof(e1list));
	
	BEGIN_TDM_PARSE_E1_LIST_TO_PORT( argv[0], ule1list)
		{
			e1list[counte1list]=ule1list;
			counte1list++;
		}
	END_TDM_PARSE_E1_LIST_TO_PORT();

	if (!VOS_StrCmp((CHAR *)argv[1], "all"))
		alarmmask=E1_ALM_ALL;
	else if(!VOS_StrCmp((CHAR *)argv[1], "los"))
		alarmmask=E1_ALM_LOS;
	else if (!VOS_StrCmp((CHAR *)argv[1], "rai"))
		alarmmask=E1_ALM_RAI;
	else if (!VOS_StrCmp((CHAR *)argv[1], "ais"))
		alarmmask=E1_ALM_AIS;
	else if (!VOS_StrCmp((CHAR *)argv[1], "smf"))
		alarmmask=E1_ALM_SMF;
	else if (!VOS_StrCmp((CHAR *)argv[1], "lof"))
		alarmmask=E1_ALM_LOF;
	else if (!VOS_StrCmp((CHAR *)argv[1], "crc-3"))
		alarmmask=E1_ALM_CRC3;
	else if (!VOS_StrCmp((CHAR *)argv[1], "crc-6"))
		alarmmask=E1_ALM_CRC6;
	else
		return CMD_WARNING;

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&slotid, (ULONG *)&portid , (ULONG *)&onudevidx);

	for(i=0;i<counte1list;i++)
		{
			idxs[0]=1;
			idxs[1]=slotid;
			if(portid==2)
				idxs[2]=1*MAXE1NUM+e1list[i];
			else if(portid==3)
				idxs[2]=2*MAXE1NUM+e1list[i];
			else
				idxs[2]=e1list[i];

			result_msg=tdm_e1portTable_set( LEAF_EPONE1PORTALARMMASK, idxs , alarmmask);

			if(result_msg!=VOS_OK)
				{
					vty_out (vty,"\r\n undo the e1 link alarm mask failure\r\n");
				}
			rc=result_msg;
		}
	
	return rc;

}

#endif

#ifdef  _GFA6xxx_VOICE_ALARM_MASK_
#endif
DEFUN(
		    show_alarm_mask,
		    show_alarm_mask_cmd,
		    "show alarm-mask e1-link {<e1_list>}*1",
		    "show alarm mask\n"
		    "show alarm mask e1 link\n"
		    "e1 link value\n"
		    "input the e1 link value\n"
)
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG ule1list,counte1list=0,e1list[MAXE1NUM],idxs[3];
	int i=0;
	ULONG devidx=1,ulIfIndex,TDM_slotid,TDM_portid,TDM_onu;
	e1porttable_row_entry  entry;
	char str[64];

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);
	

	if(argc!=1)
	{
		
		for(i=1;i<=MAXE1NUM;i++)
			{
				idxs[0]=devidx;
				idxs[1]=TDM_slotid;
				if(TDM_portid==2)
					idxs[2]=1*MAXE1NUM+i;
				else if(TDM_portid==3)
					idxs[2]=2*MAXE1NUM+i;
				else
					idxs[2]=i;
				result_msg=tdm_e1portTable_get(idxs,&entry);
				if(result_msg==VOS_OK)
				{	
					vty_out(vty,"\r\ne1 link %d alarm mask :%s\r\n",i,get_alarmmask(entry.almmask,  str));
				}
				else
					return rc;
			}
	}

	
else
{
	VOS_MemZero(e1list, sizeof(e1list));
	VOS_MemZero(idxs, sizeof(idxs));
	VOS_MemZero(str, sizeof(str));

	BEGIN_TDM_PARSE_E1_LIST_TO_PORT( argv[0], ule1list)
		{
			e1list[counte1list]=ule1list;
			counte1list++;
		}
	END_TDM_PARSE_E1_LIST_TO_PORT();

	for(i=0;i<counte1list;i++)
		{	idxs[0]=devidx;
			idxs[1]=TDM_slotid;
			if(TDM_portid==2)
				idxs[2]=1*MAXE1NUM+e1list[i];
			else if(TDM_portid==3)
				idxs[2]=2*MAXE1NUM+e1list[i];
			else
				idxs[2]=e1list[i];
			result_msg=tdm_e1portTable_get(idxs,&entry);
			if(result_msg==VOS_OK)
				{	/*vty_out (vty,"\r\n alarm mask %d",entry.almmask);*/
					vty_out(vty,"\r\ne1 link %d alarm mask :%s",e1list[i],get_alarmmask(entry.almmask,  str));
				}
			else
				return rc;
			
		}
}
	vty_out(vty,"\r\n");
	return CMD_SUCCESS;
}


DEFUN(
		   show_status_e1_link,
		   show_status_e1_link_cmd,
		   "show status e1-link {<e1_list>}*1",
		   "show e1 link status\n"
		   "e1 link status\n"
		   "please input the e1 link status value\n"
		   "the e1 list value\n"
		   )
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG TDM_slotid,TDM_portid,ule1list,TDM_onuid;
	ULONG uloperstatus=0,devidx=1;
	ULONG idxs[3],e1list[MAXE1NUM];
	char str[64];
	int counte1list=0,i=0;
	e1porttable_row_entry  entry;
	unsigned char OnuString[100] = {'\0'};
	
	if(argc!=1)
	{
		ulIfIndex = ( ULONG ) ( vty->index ) ;
		PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onuid);
		vty_out (vty,"\r\nE1Idx  Status    mode     CRC    loopback   code impedance alarm");
		/*vty_out (vty,"\r\n------------------------------------------------------------------------");*/
			
		for(i=1;i<=MAXE1NUM;i++)
			{
				if(TDM_portid==2)
					i=i+MAXE1NUM;
				if(TDM_portid==3)
					i=MAXE1NUM*2+i;
				idxs[0]=devidx;
				idxs[1]=TDM_slotid;
				idxs[2]=i;

				getEthPortOperStatus(devidx, TDM_slotid, TDM_portid, &uloperstatus);

				result_msg=tdm_e1portTable_get(idxs,&entry);
				if(result_msg==VOS_OK)
				{	
					if(TDM_portid==2)
						i=i-(TDM_portid-1)*8;
					if(TDM_portid==3)
						i=i-(TDM_portid-1)*8;
					sprintf(OnuString,"%d/%d/%d",TDM_slotid,TDM_portid,i);
					vty_out(vty,"\r\n%-7s",OnuString);
					sprintf(OnuString,"%s",opertstatus[(uloperstatus-1)]);
					vty_out (vty,"%-8s",OnuString);
					sprintf(OnuString,"%s",(portfun[entry.e1portfunc-1]));
					vty_out(vty,"%-10s",OnuString);
					sprintf(OnuString,"%s",(crcenable[entry.crcenable-1]));
					vty_out(vty,"%-8s",OnuString);
					sprintf(OnuString,"%s",(portloopback[entry.lpbctrl-1]));
					vty_out(vty,"%-11s",OnuString);
					sprintf(OnuString,"%s",(portcode[entry.portcode-1]));
					vty_out(vty,"%-6s",OnuString);
					sprintf(OnuString,"%s",(portImpendance[entry.impedance-1]));
					vty_out(vty,"%-10s",OnuString);
					sprintf(OnuString,"%s",get_alarm(entry.almstatus,str));
					vty_out(vty,"%s",OnuString);
				}
				
			}
	}
	
else
{	VOS_MemZero(str, sizeof(str));

	BEGIN_TDM_PARSE_E1_LIST_TO_PORT( argv[ 0], ule1list)
		{
			e1list[counte1list]=ule1list;
			counte1list++;
		}
	END_TDM_PARSE_E1_LIST_TO_PORT();

	
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onuid);
	vty_out (vty,"\r\nE1Idx  Status    mode     CRC    loopback   code impedance alarm");
	/*vty_out (vty,"\r\n------------------------------------------------------------------------");*/
					

	for(i=0;i<counte1list;i++)
		{
			if(TDM_portid==2)
				e1list[i]=e1list[i]+MAXE1NUM;
			if(TDM_portid==3)
				e1list[i]=MAXE1NUM*2+e1list[i];

			/*slot_port_2_swport (slotid, portid, & ulSwSlot, &ulSwport);

			ifIdex=IFM_ETH_CREATE_INDEX( ulSwSlot,  ulSwport);
	
			IFM_GetIfStatusApi(ifIdex, & uloperstatus);*/

	
			idxs[0]=devidx;
			idxs[1]=TDM_slotid;
			idxs[2]=e1list[i];

			getEthPortOperStatus(devidx, TDM_slotid, TDM_portid, &uloperstatus);

			result_msg=tdm_e1portTable_get(idxs,&entry);
			if(result_msg==VOS_OK)
				{	
					if(TDM_portid==2)
						e1list[i]=e1list[i]-(TDM_portid-1)*8;
					if(TDM_portid==3)
						e1list[i]=e1list[i]-(TDM_portid-1)*8;
					sprintf(OnuString,"%d/%d/%d",TDM_slotid,TDM_portid,e1list[i]);
					vty_out(vty,"\r\n%-7s",OnuString);
					sprintf(OnuString,"%s",opertstatus[(uloperstatus-1)]);
					vty_out (vty,"%-8s",OnuString);
					sprintf(OnuString,"%s",(portfun[entry.e1portfunc-1]));
					vty_out(vty,"%-10s",OnuString);
					sprintf(OnuString,"%s",(crcenable[entry.crcenable-1]));
					vty_out(vty,"%-8s",OnuString);
					sprintf(OnuString,"%s",(portloopback[entry.lpbctrl-1]));
					vty_out(vty,"%-11s",OnuString);
					sprintf(OnuString,"%s",(portcode[entry.portcode-1]));
					vty_out(vty,"%-6s",OnuString);
					sprintf(OnuString,"%s",(portImpendance[entry.impedance-1]));
					vty_out(vty,"%-10s",OnuString);
					sprintf(OnuString,"%s",get_alarm(entry.almstatus,str));
					vty_out(vty,"%s",OnuString);
				}

		}
}
#if  0
	if(result_msg==VOS_OK)
		{	vty_out (vty,"\r\n board num :%d",portid);
			vty_out (vty,"\r\n physical port num:%d",e1idx);
			vty_out (vty,"\r\n CRC :%s",(crcenable[pentry.crcenable-1]));
			vty_out (vty,"\r\n loopback: %s",(portloopback[pentry.lpbctrl-1]));
			vty_out (vty,"\r\n alarm  status:%s",get_alarmmask(pentry.almstatus,str));
			/*vty_out (vty,"\r\n e1 function:%s",(portfun[pentry.e1portfunc-1]));
			vty_out (vty,"\r\n alarm mask:%s", get_alarmmask(pentry.almmask,str));*/
			vty_out (vty,"\r\n e1 code: %s",(portcode[pentry.portcode-1]));
			vty_out (vty,"\r\n impedance:%s",(portImpendance[pentry.impedance-1]));
			rc=VOS_OK;
		}
#endif
	vty_out (vty,"\r\n");
	return rc;
	
}


DEFUN(
		    alarm_class_e1_link,
		    alarm_class_e1_link_cmd,
		    "alarm-class olt-e1 [all|los|lof|ais|rai|smf|lofsmf|crc3|crc6] [vital|major|minor|warning]",
		    "set e1 link alarm \n"
		    "e1 link\n"
		    "all alarm \n"
		    "e1 loss of signal \n"
		    "e1 lof \n"
		    "e1alarm mask ais \n"
		    "e1 rai alarm \n"
		    "e1 smf \n"
		    "e1 lofsmf \n"
		    "crc 3 \n"
		    "crc 6 \n"
		    "vital \n"
		    "major \n"
		    "minor \n"
		    "warning \n"
)
{	
	STATUS  result_msg=VOS_ERROR;
	ULONG alarmclass=E1_ALM_CLASS_WARNING;
	UCHAR  lvl[MAXALARMMNU];
	int i=0;
	ulong_t idx[MAXALARMMNU]=
	{
		 trap_e1LosAlarm,
		 trap_e1LofAlarm,
		 trap_e1AisAlarm,
		 trap_e1RaiAlarm,
		 trap_e1SmfAlarm,
		 trap_e1LomfAlarm,
		 trap_e1Crc3Alarm,
		 trap_e1Crc6Alarm
	};
	
	
	if(argc!=2)
		vty_out(vty,"\r\n%% parameter error\r\n");

	VOS_MemZero(lvl, sizeof(lvl));

	if (!VOS_StrCmp((CHAR *)argv[1], "vital"))
		alarmclass=E1_ALM_CLASS_VITAL;
	else if (!VOS_StrCmp((CHAR *)argv[1], "major"))
		alarmclass=E1_ALM_CLASS_MAJOR;
	else if (!VOS_StrCmp((CHAR *)argv[1], "minor"))
		alarmclass=E1_ALM_CLASS_MINOR;
	else if(!VOS_StrCmp((CHAR *)argv[1], "warning"))
		alarmclass=E1_ALM_CLASS_WARNING;
	else
		return CMD_WARNING;

	if (!VOS_StrCmp((CHAR *)argv[0], "los"))
		{
			lvl[0]=alarmclass;
			setAlarmLevel(idx[0], alarmclass);
		}
	else if(!VOS_StrCmp((CHAR *)argv[0], "lof"))
		{
			lvl[1]=alarmclass;
			setAlarmLevel(idx[1], alarmclass);
		}
	else if (!VOS_StrCmp((CHAR *)argv[0], "ais"))
		{
			lvl[2]=alarmclass;
			setAlarmLevel(idx[2], alarmclass);
		}
	else if (!VOS_StrCmp((CHAR *)argv[0], "rai"))
		{
			lvl[3]=alarmclass;
			setAlarmLevel(idx[3], alarmclass);
		}
	else if (!VOS_StrCmp((CHAR *)argv[0], "smf"))
		{
			lvl[4]=alarmclass;
			setAlarmLevel(idx[4], alarmclass);
		}
	else if (!VOS_StrCmp((CHAR *)argv[0], "lofsmf"))
		{
			lvl[5]=alarmclass;
			setAlarmLevel(idx[5], alarmclass);
		}
	else if (!VOS_StrCmp((CHAR *)argv[0], "crc3"))
		{
			lvl[6]=alarmclass;
			setAlarmLevel(idx[6], alarmclass);
		}
	else if (!VOS_StrCmp((CHAR *)argv[0], "crc6"))
		{
			lvl[7]=alarmclass;
			setAlarmLevel(idx[7], alarmclass);
		}
	else if(!VOS_StrCmp((CHAR *)argv[0], "all"))
		{
			VOS_MemSet(lvl, alarmclass, sizeof(lvl));
			for(;i<MAXALARMMNU;i++)
				{
					setAlarmLevel(idx[i], alarmclass);
				}
		}
	else
		return CMD_WARNING;

	
	result_msg=tdm_E1AlarmLevelSet( lvl);
	if(result_msg!=VOS_OK)
		{
			vty_out(vty,"\r\nset e1 alarm level failure\r\n");
		}

return  CMD_SUCCESS;
	
}


DEFUN(
		    show_alarm_class,
		    show_alarm_class_cmd,
		    "show alarm-class [olt-e1]",
		    "show alarm class\n"
		    "display alarm class\n"
		    "olt e1 alarm class\n"
)
{
	STATUS  result_msg=VOS_ERROR;
	unsigned char OnuString[100] = {'\0'};
	int i=0;
	ulong_t level;

	if( VOS_MemCmp(argv[0], "olt-e1",6) == 0 )
	{
	unsigned char* alarmtype[MAXALARMMNU]=
	{
		(unsigned char*)"los alarm",
		(unsigned char*)"LOF alarm",
		(unsigned char*)"AIS alarm",
		(unsigned char*)"RAI alarm",
		(unsigned char*)"SMF alarm",
		(unsigned char*)"LOFSMF alarm",
		(unsigned char*)"CRC-3 alarm",
		(unsigned char*)"CRC-6 alarm"
	};

	ulong_t idx[MAXALARMMNU]=
	{
		 trap_e1LosAlarm,
		 trap_e1LofAlarm,
		 trap_e1AisAlarm,
		 trap_e1RaiAlarm,
		 trap_e1SmfAlarm,
		 trap_e1LomfAlarm,
		 trap_e1Crc3Alarm,
		 trap_e1Crc6Alarm
	};
	sprintf(OnuString,"alarm-type           level");
	vty_out(vty,"\r\n%s",OnuString);
	/*vty_out(vty,"\r\n-----------------------------------");*/

	for(;i<MAXALARMMNU;i++)
		{
			result_msg=getAlarmLevel( idx[i],  &level);
			if(result_msg!=VOS_OK)
				{
					vty_out(vty,"\r\nget alarm level failure\r\n");
					return CMD_WARNING;
				}
			else
				{
					sprintf(OnuString,"%s",alarmtype[i]);
					vty_out(vty,"\r\n%-21s",OnuString);
					sprintf(OnuString,"%s",alarmlevel[(level)-1]);
					vty_out(vty,"%-5s",OnuString);			
				}
		}
	vty_out(vty,"\r\n");
	}

	else{ /* other type */

		}

	vty_out(vty,"\r\n");
	return  CMD_SUCCESS;

}
#ifdef _GFA6xxx_VOICE_ADD_ONU_
#endif
/*添加和取消信令网关下的onu */
DEFUN(
		   add_voice_onu,
		   add_voice_onu_cmd,
		   "add voice-onu <slot/port/onuid>",
		   "add onu to tdm\n"
		   "add voice onu\n"
		   "input the slot/port/onuid\n"
		   )
{	
	STATUS rc=VOS_ERROR;
	LONG lRet;
	STATUS result_msg;
	ulong tdmonu =0;
	ULONG ulIfIndex = 0;
	ULONG tdmslot,tdmsgidx,onudevidx,PON_onuid;
	ULONG idxs[2];
	ULONG PON_slot,PON_port;
	unsigned short int logicOnu,LogicOnuId;
	unsigned char TdmSlot,SgIdx;
	
	/*if(argc!=1)
		vty_out(vty,"\r\n parameter error");*/

	lRet = PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&tdmsgidx , &tdmonu/**/) ;

	/*onudevidx=PON_slot*10000+PON_port*1000+PON_onuid;*/
        onudevidx=MAKEDEVID(PON_slot,PON_port,PON_onuid);
	/*sys_console_printf("\r\nonuid = %d",onudevidx);*/
	
	idxs[0]=tdmsgidx;
	idxs[1]=onudevidx;
#if 0
	result_msg=GetOnuBelongToSG( onudevidx, & TdmSlot, & SgIdx, & LogicOnuId);
	if (result_msg==VOS_OK)
		{
			sys_console_printf ("\r\n the onu have added into the sg \r\n");
			return rc;
		}
	else 
		{
#endif
			result_msg=AddOnuToSG( tdmslot,  tdmsgidx,  onudevidx, &logicOnu);
	
			if(result_msg==VOS_OK)
				{
					rc=VOS_OK;
				}
			else if( result_msg == TDM_VM_NOT_EXIST )
				{
				vty_out(vty, " %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
				return( CMD_WARNING );
				}
			else if (result_msg==TDM_VM_IN_ANOTHER_SG)		
				{	
					GetOnuBelongToSG(onudevidx, & TdmSlot, & SgIdx, & LogicOnuId);
					sys_console_printf ("\r\nthe onu is belonged to another %s(%d/%d)\r\n",GetGFA6xxxTdmNameString(),TdmSlot,SgIdx);
					return ( CMD_WARNING );
				}
			else if (result_msg==TDM_VM_IN_THIS_SG)
				{
					sys_console_printf ("\r\n the onu have in the tdm\r\n");
					return ( CMD_WARNING );
				}
			else if (result_msg==TDM_VM_NOT_SUPPORT_VOICE)
				{
					sys_console_printf ("\r\n the onu is not support voice\r\n");
					return ( CMD_WARNING );
				}
			else 
				{
					sys_console_printf ("\r\n %% executing failed\r\n");
					return ( CMD_WARNING );
				}
		
	
	return rc;	
}

DEFUN (
		   delete_voice_onu,
		   delete_voice_onu_cmd,
		   "delete voice-onu <slot/port/onuid>",
		   "deltet onu from tdm\n"
		   "delete voice onu\n"
		   "input the slot/port/onuid\n"
)
{
	LONG lRet;
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG TDM_slotid,TDM_portid,TDM_onu,onudevidx,PON_onuid;
	ULONG idxs[2];
	ULONG PON_slot,PON_port;
	uchar ret_tdmslot,ret_sgidx;
	unsigned short int logicOnu;
	/*if(argc!=1)
		vty_out(vty,"\r\n%% parameter error\r\n");*/

	lRet = PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);

	/*onudevidx=PON_slot*10000+PON_port*1000+PON_onuid;*/
        onudevidx=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	idxs[0]=TDM_portid;
	idxs[1]=onudevidx;

	if(ThisIsValidOnu(GetPonPortIdxBySlot(PON_slot, PON_port),(PON_onuid-1)) != ROK )
		{
		vty_out(vty, " %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
		}

	/*result_msg=mn_tdmOnuDel(idxs);*/
	result_msg=DeleteOnuFromSGBySgIdx(onudevidx, (unsigned char)TDM_slotid,(unsigned char) TDM_portid);
	if(result_msg == VOS_OK)
		vty_out (vty,"\r\n delete onu from tdm \r\n");
	else if (result_msg==TDM_VM_NOT_IN_ANY_SG)
		vty_out (vty,"\r\n not in any tdm \r\n");
	else if( result_msg == TDM_VM_IN_ANOTHER_SG )
		{
			if(GetOnuBelongToSG( onudevidx, &ret_tdmslot, &ret_sgidx, &logicOnu ) == TDM_VM_OK )
				vty_out(vty,"\r\n onu%d/%d/%d is belonged to another %s(%d/%d)\r\n",PON_slot, PON_port, PON_onuid, GetGFA6xxxTdmNameString(),ret_tdmslot, ret_sgidx);
			else vty_out(vty,"\r\n onu%d/%d/%d is belonged to another %s\r\n",PON_slot, PON_port, PON_onuid,GetGFA6xxxTdmNameString());			
		}
	else
		vty_out (vty,"\r\n %%executing failed\r\n");
	 
#if  0
	result_msg=tdm_tdmOnuTable_set( LEAF_EPONTDMONUROWSTATUS, idxs , RS_DESTROY, & lport);

	if(result_msg==VOS_OK)
		{
			vty_out (vty,"\r\n delete the onu to sg success\r\n");
			vty_out (vty,"\r\n the sg index is %d",portid);
			vty_out (vty,"\r\n \r\n");
		}
	rc=result_msg;
#endif
	return  CMD_SUCCESS;
	
}

#ifdef  _GFA6xxx_VOICE_ONU_POTS_
#endif
/*onu 上的pots 口语音换回和取消*/
DEFUN (
		   voice_onu_loopback,
		   voice_onu_loopback_cmd,
		   "voice-onu loopback [enable|disable] <slot/port/onuid>",
		   "set onu pots voice loopback\n"
		   "voice onu loopback\n"
		   "set voice onu loopback enable\n"
		   "set voice onu loopback disable\n"
		   "input the slot/port/onuid\n"
		  
)
{
	LONG lRet;
	STATUS result_msg;
	ULONG PON_port,PON_slot,PON_onuid,onudevidx,tdmslot,tdmsgidx,tdmonu;
	ULONG ulIfIndex = 0;
	ULONG idxs[2]={0},ulEnable;
	unsigned char ul_tdmslot,ul_sgidx;
	unsigned short int LogicOnuId;

	/*if(argc!=2)
		vty_out(vty,"\r\n%% parameter error\r\n");*/

	lRet = PON_ParseSlotPortOnu( argv[1], &PON_slot, &PON_port, &PON_onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	if(ThisIsValidOnu(GetPonPortIdxBySlot(PON_slot, PON_port),(PON_onuid-1)) != ROK )
		{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
		}

	 if(!VOS_StrCmp(argv[0],"enable"))
    {
         ulEnable = VOICE_LOOP_ENABLE;
    }
    else if(!VOS_StrCmp(argv[0],"disable"))
    {
         ulEnable = VOICE_LOOP_DISABLE;
    }
    else
    {
        /*vty_out(vty,"%% Parameter is error.\r\n");*/
        return CMD_WARNING;
    }
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&tdmsgidx , (ULONG *)&tdmonu);

	/*onudevidx=PON_slot*10000+PON_port*1000+PON_onuid;*/
        onudevidx=MAKEDEVID(PON_slot,PON_port,PON_onuid);
    
	idxs[0]=tdmsgidx;
	idxs[1]=onudevidx;
	/*sys_console_printf ("\r\n the pon slot %d,the port %d, the onu  %d",slotid,portid,onuid);
	sys_console_printf ("\r\n the tdm slot %d,the tdm port index %d",tdmslot,sgidx);*/

	if(ulEnable==VOICE_LOOP_ENABLE)
		{	
			result_msg=GetOnuBelongToSG( onudevidx, &ul_tdmslot, & ul_sgidx,& LogicOnuId);
			if(result_msg==TDM_VM_ERR)
				{
				vty_out(vty,"\r\n get onu%d/%d/%d/ belonged to %s failure\r\n",PON_slot,PON_port,PON_onuid,GetGFA6xxxTdmNameString());
				return (CMD_WARNING);
				}
			else if(result_msg==TDM_VM_OK)
				{
				if(ul_tdmslot==tdmslot && ul_sgidx==tdmsgidx)
					{
						result_msg=mn_tdmOnuVoiceLoopbackStart(idxs);
						if(result_msg!=VOS_OK)
						vty_out (vty,"\r\n set onu voice loopback failure\r\n");
					}
				else 
					{
						vty_out (vty,"\r\n onu%d/%d/%d is belonged to %s(%d/%d)\r\n",PON_slot,PON_port,PON_onuid,GetGFA6xxxTdmNameString(),ul_tdmslot, ul_sgidx);
						return (CMD_WARNING);
					}
				}
			else if (result_msg==TDM_VM_NOT_IN_ANY_SG)
				{
					vty_out (vty,"\r\n onu%d/%d/%d not in any %s\r\n",PON_slot,PON_port,PON_onuid,GetGFA6xxxTdmNameString());
					return (CMD_WARNING);
				}
			
		}
	else 
		{	result_msg=GetOnuBelongToSG( onudevidx, &ul_tdmslot, & ul_sgidx,& LogicOnuId);
			if(result_msg==TDM_VM_ERR)
			{
				vty_out(vty,"\r\n get onu%d/%d/%d/ is belonged to %s failure\r\n",PON_slot,PON_port,PON_onuid,GetGFA6xxxTdmNameString());
				return (CMD_WARNING);
			}
			else if(result_msg==TDM_VM_OK)
			{
				if(ul_tdmslot==tdmslot&&ul_sgidx==tdmsgidx)
					{
						result_msg=mn_tdmOnuVoiceLoopbackStop(idxs);
						if(result_msg!=VOS_OK)
						vty_out (vty,"\r\n stop onu voice loopback failure\r\n");
					}
				else 
					{
						vty_out (vty,"\r\n onu%d/%d/%d is belonged to another %s(%d/%d)\r\n",PON_slot,PON_port,PON_onuid,GetGFA6xxxTdmNameString(),ul_tdmslot, ul_sgidx);
						return (CMD_WARNING);
					}
			}
			else if (result_msg==TDM_VM_NOT_IN_ANY_SG)
				{
					vty_out (vty,"\r\n onu %d/%d/%d not in any %s\r\n",PON_slot,PON_port,PON_onuid,GetGFA6xxxTdmNameString());
					return (CMD_WARNING);
				}
			
			
		}
	
#if  0
	result_msg=tdm_tdmOnuTable_get( idxs ,  &entry);
	loopstatus=entry.potsloopenable;
	sys_console_printf ("\r\n the loop status is %d\r\n",loopstatus);
	
	if(loopstatus==1)
		{
			sys_console_printf ("\r\n the tdm onu loopback is already enable\r\n");
			return rc;
		}
	else if (loopstatus==0)
		{
			result_msg=tdm_tdmOnuTable_set( LEAF_EPONTDMONUROWSTATUS,  idxs, RS_CREATEANDWAIT, & lport);
			if(result_msg==VOS_OK)
			{
				sys_console_printf ("\r\n set the tdm onu table row status active success\r\n");
				result_msg=tdm_tdmOnuTable_set(LEAF_EPONTDMONUPOTSLOOPBACKCTRL, idxs , 1, & lport);
				if (result_msg==VOS_OK)
					{
						sys_console_printf ("\r\n set the tdm onu table loopback success\r\n");
						rc=VOS_OK;
					}
				else
					rc=VOS_ERROR;
			}
			else 
				return rc;
		}
	else  
		{
			result_msg=tdm_tdmOnuTable_set( LEAF_EPONTDMONUPOTSLOOPBACKCTRL, idxs, 1, & lport);
			if (result_msg==VOS_OK)
				{
					sys_console_printf ("\r\n set the tdm onu table loopback success \r\n");
					result_msg=tdm_tdmOnuTable_get(  idxs ,  &entry);
					loopstatus=entry.potsloopenable;
					sys_console_printf ("\r\n the tdm onu loopback status is %d",loopstatus);
					rc=VOS_OK;
				}
		}
#endif
#if 0
	result_msg=tdm_tdmOnuTable_get(ENUM_CALLTYPE_SYN, 0,  idxs ,  &entry);
	if (result_msg==VOS_OK)
		{
			sys_console_printf ("\r\n get the tdm onu table success\r\n");
			rowstatus=entry.rowstatus;
			if(rowstatus==6)
				{
					sys_console_printf ("\r\nthe row status is destory ,set the row status first");
					result_msg=tdm_tdmOnuTable_set(ENUM_CALLTYPE_SYN, 0, 5,  idxs , 5,& lport);
					if (result_msg==VOS_OK)
						{
							sys_console_printf ("\r\nset the row status active success");
							result_msg=tdm_tdmOnuTable_set(ENUM_CALLTYPE_SYN, 0, 6,  idxs , 1,& lport);
							if(result_msg==VOS_OK)
								{
									sys_console_printf ("set the tdm onu loopback success\r\n");
									rc=VOS_OK;
								}
							else 
								rc=VOS_ERROR;
						}
					
				}
			else
				{
						result_msg=tdm_tdmOnuTable_set(ENUM_CALLTYPE_SYN, 0, 6,  idxs , 1,& lport);
						if(result_msg==VOS_OK)
							{
								sys_console_printf ("set the tdm onu loopback success\r\n");
								rc=VOS_OK;
							}
						else 
							rc=VOS_ERROR;
				}
		}

	return  CMD_SUCCESS;
}

DEFUN (
		   undo_onu_pots_voice_loopback,
		   undo_onu_pots_voice_loopback_cmd,
		   "undo onu-pots voice-loopback <slot/port/onuid>",
		   "set onu pots voice loopback disable\n"
		   "undo onu pots voice loopback enable\n"
		   "the tdm onu table vioce loopback \n"
		   "the slot/port/onuid value like :5/1/1\n"
)
{
	LONG lRet;
	STATUS result_msg;
	ULONG portid,slotid,onuid,onudevidx;
	ULONG tdmslot,sgidx,onu;
	ULONG ulIfIndex = 0;
	ULONG idxs[2]={0};
	
	if(argc!=1)
		vty_out(vty,"\r\n%% parameter error\r\n");

	lRet = PON_ParseSlotPortOnu( argv[0], &slotid, &portid, &onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&sgidx, (ULONG *)&onu);


	sys_console_printf ("\r\n the tdm slot num %d",tdmslot);
	sys_console_printf ("\r\n the tdm  port %d",sgidx);
	


	onudevidx=slotid*10000+portid*1000+onuid;
	idxs[0]=sgidx;
	idxs[1]=onudevidx;

	result_msg=mn_tdmOnuVoiceLoopbackStop(idxs);

	if(result_msg==VOS_OK)
		vty_out (vty,"\r\n set onu voice loopback stop\r\n");
	else 
		vty_out (vty,"\r\n stop onu voice loopback failure\r\n");

	result_msg=tdm_tdmOnuTable_get(  idxs , &entry);
		{
			if (result_msg==VOS_OK)
				{
					sys_console_printf ("\r\n get the tdm onu table success \r\n");
					loopenable=entry.potsloopenable;
					sys_console_printf ("\r\n the tdm onu loopback is %d",loopenable);

					if(loopenable==1)
						{
							result_msg=tdm_tdmOnuTable_set(LEAF_EPONTDMONUPOTSLOOPBACKCTRL,  idxs ,2,&lport);
							/*result_msg=test_tdm_conf_set_func (2, 6, sgidx, onudevidx, 0, 0, (uchar*)&lport);*/

							if(result_msg==VOS_OK)
							{
								sys_console_printf ("\r\n undo the tdm onu table loopback success \r\n ");
								result_msg=tdm_tdmOnuTable_get( idxs , &entry);
								loopenable=entry.potsloopenable;
								sys_console_printf ("\r\n the tdm onu loopback is %d",loopenable);
							}
							else 
								{
									sys_console_printf ("\r\n set the tdm onu loopback disable failure\r\n");
									return rc;
								}
								
						}
					else 
						{
							sys_console_printf ("\r\n the tdm onu loopback is disable ,please set the loopback enable first");
							return rc;
						}
				}
			else 
				return rc;
		}
	rc=result_msg;
#endif
	return CMD_SUCCESS;
}

/*添加/删除onu 上的pots 口*/
DEFUN (
		   add_voice_link,
		   add_voice_link_cmd,
		   "add voice-link <slot/port/onuid> port <pots_list> <voice_link_port_list>",
		   "add voice link to sig\n"
		   "add voice link\n"
		   "input the slot/port/onuid\n"
		   "please input the port list \n"
		   /*"the port list value like 1/1-3,5 (GT865 pots range:1-16)\n"*/
		   "the pots port list value like 1/1-3,5\n"
		   "the tdm port logical port list value like 1-3,5 \n"
)
{
	STATUS rc=VOS_ERROR;
	LONG lRet;
	STATUS result_msg;
	ULONG PON_port,PON_slot,PON_onuid,onudev,onuboard=0;
	ULONG tdmslot,tdmsgidx,tdmonu,onupots,sglogport ;
	ULONG ulIfIndex = 0;
	ULONG logpots[MAXONUPOTS],pots[MAXONUPOTS],prepots[MAXONUPOTS];
	int countpots=0,countsglog=0,i=0;
	bool SupportVoice = FALSE;
	unsigned char tdmslot_ret, sgidx_ret;
	unsigned short int  LogicOnu;
	unsigned long PotsList = 0;
	BOOL   EnableFlag=FALSE;
	unsigned long OnuDeviceIdx_ret;
	unsigned char  PotsBoard_ret,PotsPort_ret;
	USHORT logicalport_ret;
	ULONG slot_ret=0,port_ret=0,pots_ret=0;
	int OnuType, errFlag = 0;
    short int PonPortIndex;
	
	/*if(argc!=3)
		vty_out(vty,"\r\n%% parameter error\r\n");*/

	lRet = PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	/*onudev=PON_slot*10000+PON_port*1000+PON_onuid;*/
        onudev=MAKEDEVID(PON_slot,PON_port,PON_onuid);

    PonPortIndex = GetPonPortIdxBySlot(PON_slot, PON_port);
	if(ThisIsValidOnu(PonPortIndex, (PON_onuid-1)) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	if(GetOnuType(PonPortIndex, (PON_onuid-1), &OnuType) != ROK )
	{
		vty_out(vty, " %% get onu%d/%d%d type err\r\n", PON_slot, PON_port, PON_onuid);
		return( CMD_WARNING);
	}
#if 0
	else
		{/*added by zhengyt 2008-7-3*/
			if(OnuType==V2R1_ONU_GT861)
				{
					VoiceBoardNumGet( onudev, BoardNum);
				}
			else
				onuboard=1;
		}
#endif
	if(!((OnuIsSupportVoice(onudev,&SupportVoice) == ROK ) && (SupportVoice ==  TRUE )))
		{
		vty_out(vty,"  %% onu %d/%d/%d is not support voice\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING);
		}

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&tdmsgidx, (ULONG *)&tdmonu);
	
	BEGIN_TDM_PARSE_VOICE_LINK_LIST_TO_PORT( argv[ 2],sglogport)
	{
		logpots[countsglog]=sglogport;
		countsglog++;
	}
	END_TDM_PARSE_VOICE_LINK_LIST_TO_PORT();

	BEGIN_TDM_PARSE_POTS_LIST_TO_POT( argv[ 1],onuboard,onupots)
	{
		if((OnuType == V2R1_ONU_GT865) &&((onupots > MAXONUPOTS_GT865)||(onuboard!=1)))
			{
			errFlag = 1;
			}
		else if((OnuType == V2R1_ONU_GT861) &&((onupots > 8)||(onuboard>5)||(onuboard<2)))/*added by zhengyt 2008-7-3*/
			{
			errFlag = 1;
			}
		else {
			prepots[countpots]=onupots;
			pots[countpots]=onupots;
			/*sys_console_printf ("potslist=%d\r\n",pots[countpots]);*/
			countpots++;
			}
	}

	END_TDM_PARSE_POTS_LIST_TO_POT();

	if(OnuType==V2R1_ONU_GT861)
		{
			for(i=0;i<countpots;i++)
				{	
					pots[i]=(onuboard-2)*8+pots[i];
				}
		}


	if(errFlag == 1 )
		{
		vty_out(vty," %% this onu is %s, the  pots list error\r\n", GetDeviceTypeString(OnuType));
		return(CMD_WARNING);
		}
	
	if (countpots != countsglog)
		{
			vty_out (vty,"\r\nport list num is not equal to tdm port logical list num \r\n");
			return  (CMD_WARNING);
		}

	rc = GetOnuBelongToSG( onudev, &tdmslot_ret, &sgidx_ret, &LogicOnu );
	if (rc ==TDM_VM_NOT_IN_ANY_SG)
		{
		rc = AddNewOnuToSg((unsigned char)tdmslot,(unsigned char)tdmsgidx,onudev,&LogicOnu);
		if( rc !=  VOS_OK )
			{
			vty_out(vty," %% add onu %d/%d/%d to %s%d/%d err\r\n",PON_slot,PON_port, PON_onuid, GetGFA6xxxTdmNameString(),tdmslot,tdmsgidx);
			return( CMD_WARNING);
			}
		}
	else if( rc == TDM_VM_ERR )
		{
		vty_out (vty,"\r\n %% executing failed\r\n");	
		return( CMD_WARNING );
		}
	else if (rc ==TDM_VM_NOT_SUPPORT_VOICE)
		{
		vty_out (vty,"\r\n onu is not support voice\r\n");
		return( CMD_WARNING );
		}
	
	else if ( rc == TDM_VM_OK )
		{
		if((tdmslot_ret  != tdmslot ) || (sgidx_ret  != tdmsgidx ))
			{
			vty_out(vty," %% onu%d/%d/%d is belonged to another %s(%d/%d)\r\n",PON_slot, PON_port, PON_onuid,GetGFA6xxxTdmNameString(),tdmslot_ret,sgidx_ret);
			return( CMD_WARNING );
			}
		}
	
	rc = GetOnuPotsLinkAll(onudev, &PotsList);
	if( rc == VOS_OK )
		{
		ULONG setval = 0, newPotsBoard , newPotsPort;
		ULONG idxs[2] = { 0, 0 };
		
		for(i=0; i< countpots; i++)
			{
			if( (PotsList & ( 1<< (pots[i]-1)))  == 0)
				{	
					newPotsBoard=1;
					newPotsPort=pots[i];
					idxs[0] = tdmsgidx;
					idxs[1] = logpots[i];
					setval = 5;
				#if 0
					if(OnuType==V2R1_ONU_GT861)
						{/*added by zhengyt 2008-7-3*/
							newPotsBoard=1;
							newPotsPort=pots[i];
							idxs[0] = tdmsgidx;
							idxs[1] = logpots[i];
							setval = 5;
						}
					else
						{	
							if(onuboard!=1)
							{
								vty_out(vty,"onuboard error\r\n");
								return VOS_ERROR;
							}
							else
							{
								newPotsPort = pots[i];
								newPotsBoard = onuboard;
								idxs[0] = tdmsgidx;
								idxs[1] = logpots[i];
								setval = 5;
							}
							
						}
				#endif
					rc=GetOnuPotsLinkBySgLogicPort((unsigned char) tdmslot, (unsigned char) tdmsgidx, (unsigned short int) logpots[i], &EnableFlag, & OnuDeviceIdx_ret, & PotsBoard_ret, & PotsPort_ret);
					if(rc==TDM_VM_NOT_EXIST_LINK_POT)
						{
							if( tdm_potsLinkTable_set( LEAF_EPONPOTSLINKROWSTATUS, idxs, (UCHAR*)&setval ) == VOS_OK )
							{
								tdm_potsLinkTable_set( LEAF_EPONPOTSLINKONUDEV, idxs, (UCHAR*)&onudev );
								tdm_potsLinkTable_set( LEAF_EPONPOTSLINKONUBOARD, idxs, (UCHAR*)&newPotsBoard );
								tdm_potsLinkTable_set( LEAF_EPONPOTSLINKONUPOTS, idxs, (UCHAR*)&newPotsPort );
								setval = RS_ACTIVE;
								tdm_potsLinkTable_set( LEAF_EPONPOTSLINKROWSTATUS, idxs,  (UCHAR*)&setval);
								/*vty_out(vty,"the set potslinktable rowstatus =%d,onudev=%d,onuboard=%d,onuport=%d\r\n",setval,onudev,newPotsBoard,newPotsPort);*/
							}
							else
								vty_out(vty, " %% add voice-link onu%d/%d/%d pots %d err\r\n",PON_slot, PON_port, PON_onuid,pots[i]);

						}
					else if (rc==VOS_OK)
					{	if(PotsPort_ret==pots[i])
							{
							if(OnuDeviceIdx_ret==onudev)
								vty_out(vty," %% pots %d already configed\r\n",pots[i]);
							else
								{
								slot_ret=GET_PONSLOT(OnuDeviceIdx_ret)/*(OnuDeviceIdx_ret/10000)*/;
								/*vty_out(vty,"\r\nOnuDeviceIdx_ret = %d",OnuDeviceIdx_ret);*/
								port_ret=GET_PONPORT(OnuDeviceIdx_ret)/*((OnuDeviceIdx_ret%10000)/1000)*/;
								pots_ret=GET_ONUID(OnuDeviceIdx_ret)/*(OnuDeviceIdx_ret%1000)*/;
								vty_out(vty," %% logical pots %d already configed:onu %d/%d/%d port %d\r\n",logpots[i],slot_ret,port_ret,pots_ret,PotsPort_ret);
								
								}
							}
						
						else
							{
							slot_ret=GET_PONSLOT(OnuDeviceIdx_ret)/*(OnuDeviceIdx_ret/10000)*/;
							port_ret=GET_PONPORT(OnuDeviceIdx_ret)/*((OnuDeviceIdx_ret%10000)/1000)*/;
							pots_ret=GET_ONUID(OnuDeviceIdx_ret)/*(OnuDeviceIdx_ret%1000)*/;
							vty_out(vty," %% logical pots %d already configed:onu %d/%d/%d port %d\r\n",logpots[i],slot_ret,port_ret,pots_ret,PotsPort_ret);
							}
						}
					
				}
			else 
				{
				if(GetOnuPotsLogicalPort(onudev, onuboard, pots[i], & logicalport_ret)==TDM_VM_OK)
					{
						if(logicalport_ret==logpots[i])
							vty_out(vty," %% voice-link onu %d/%d/%d port %d/%d already configed\r\n",PON_slot, PON_port,PON_onuid,onuboard,prepots[i]);
						else
							vty_out(vty," %% voice-link onu %d/%d/%d port %d/%d already configed:logical port %d\r\n",PON_slot, PON_port,PON_onuid,onuboard,prepots[i],logicalport_ret);
					}
				/*
				if( countpots == 1 )
					vty_out(vty," %% voice-link onu %d/%d/%d pots %d already configed\r\n",slotid, portid, onuid,pots[i]);*/
				}
			}
		
			/*通过OAM使能ONU上的POT口*/
			/*VOS_ASSERT( 0 );*/
		/*if( onuIsOnLine( onudev ) )
			{
			PotsList = 0;			
			for(i=0; i< countpots; i++)
				{
				PotsList  |=  (1 << (pots[i] -1));
				}
			SetOnuPotsEnable( onudev, ENABLE, countpots, PotsList );
			}*/		/* removed by xieshl 20080527 */
		}
	else {
		vty_out (vty,"\r\n %% executing failed\r\n");	
		}

	return( CMD_SUCCESS );

	for(i=0;i<countsglog;i++)
		{	
	#if  0
			result_msg=tdm_potsLinkTable_set(ENUM_CALLTYPE_SYN, 0, 8,  idxs , (uchar*)&rowstatus);
			if(result_msg==VOS_OK)
			sys_console_printf ("\r\n set the pots link %d row  status  creat and wait success\r\n",i+1);
			
			result_msg=tdm_potsLinkTable_set(ENUM_CALLTYPE_SYN, 0, 5, idxs ,(uchar*)&pots[i]);
			if(result_msg==VOS_OK)
			vty_out (vty,"\r\n set the pots link row onu pots  %d success\r\n",i+1);
			
			result_msg=tdm_potsLinkTable_set(ENUM_CALLTYPE_SYN, 0, 3, idxs , (uchar*)&onudev);
			if(result_msg==VOS_OK)
			vty_out (vty,"\r\n set the pots link row  onu dev %d success\r\n",i+1);
			
			result_msg=tdm_potsLinkTable_set(ENUM_CALLTYPE_SYN, 0, 4,  idxs , (uchar*)&onuboard);
			if(result_msg==VOS_OK)
			vty_out (vty,"\r\n set the pots link row  onu board %d success\r\n",i+1);
			
			rowstatus=1;
			result_msg=tdm_potsLinkTable_set(ENUM_CALLTYPE_SYN,0, 8,  idxs , (uchar*)&rowstatus);
			if(result_msg==VOS_OK)
			vty_out (vty,"\r\n add onu pots link %d success\r\n",i+1);
			
			result_msg=tdm_potsLinkTable_get(ENUM_CALLTYPE_SYN, 0, idxs, &entry);
			if (result_msg==VOS_OK)
				{
					sys_console_printf ("\r\n the link row status is: %d",entry.linkrowstatus);
					sys_console_printf ("\r\n the link sgidx is: %d",entry.linksgIdx);
					sys_console_printf ("\r\n the link portidx is: %d",entry.linksgportIdx);

				}
#endif

			result_msg=AddOnuPotsLinkToSg(tdmslot, tdmsgidx, logpots[i], onudev, onuboard, pots[i]);
			if (result_msg==VOS_OK)
				;
			else if (result_msg==TDM_VM_EXIST_ANOTHER_LINK_POT)
				vty_out (vty,"\r\nthe pots link %d exist another link port\r\n",pots[i]);
			else if (result_msg==TDM_VM_IN_ANOTHER_SG)
				vty_out (vty,"\r\n pots link %d is in another tdm port\r\n ",pots[i]);
			else if (result_msg==TDM_VM_EXIST_LINK_POT)
				vty_out (vty,"\r\n pots link %d exist link \r\n",pots[i]);
			else if (result_msg==TDM_VM_NOT_SUPPORT_VOICE)
				vty_out (vty,"\r\n the onu is not support voice\r\n");
			else 
				vty_out (vty,"\r\n add onu pots link failure %d\r\n",result_msg);
			
			
		}	
	rc=result_msg;
	return rc;
}

DEFUN (
		   delete_voice_link,
		   delete_voice_link_cmd,
		   "delete voice-link <slot/port/onuid> port <pots_list>",
		   "delete onu pots link \n"
		   "delete voice link \n"
		   "input the slot/port/onuid\n"
		   /*"the port list like 1/1-3,5 (GT865 pots range:1-16)\n"*/
		   "the port list like 1/1-3,5\n"
		   "please input the port list value like : 1-3,5 \n"
)
{
	int  rc=VOS_ERROR;
	LONG lRet;
	ULONG PON_port,PON_slot,PON_onuid,onudev;
	ULONG tdmslot,tdmsgidx,tdmonu,onupots;
	ULONG ulIfIndex = 0;
	ULONG pots[MAXONUPOTS],ulpots=0;
	unsigned char potsboard=1;
	bool SupportVoice = FALSE;
	unsigned long PotsList = 0;
	unsigned char  tdmslot_ret,sgidx_ret;
	unsigned short int LogicOnu;

	int countpots=0,i=0;
	int OnuType, errFlag = 0,BoardNum=0;
    short int PonPortIndex;

	/*if(argc!=2)
		vty_out(vty,"\r\n%% parameter error\r\n");*/

	lRet = PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

    PonPortIndex = GetPonPortIdxBySlot(PON_slot, PON_port);
	if(ThisIsValidOnu(PonPortIndex, (PON_onuid-1)) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}
		
	if(GetOnuType(PonPortIndex, (PON_onuid-1), &OnuType) != ROK )
	{
		vty_out(vty, " %% get onu%d/%d%d type err\r\n", PON_slot, PON_port, PON_onuid);
		return( CMD_WARNING);
	}

	/*onudev=PON_slot*10000+PON_port*1000+PON_onuid;*/
        onudev=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	if(!((OnuIsSupportVoice(onudev,&SupportVoice) == ROK ) && (SupportVoice ==  TRUE )))
		{
		vty_out(vty,"  %% onu %d/%d/%d is not support voice\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING);
		}
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&tdmsgidx, (ULONG *)&tdmonu);
		
	BEGIN_TDM_PARSE_POTS_LIST_TO_POT( argv[ 1], BoardNum,onupots)
	{
		if((OnuType == V2R1_ONU_GT865) &&((onupots > MAXONUPOTS_GT865)||(BoardNum!=1)))
			{
			errFlag = 1;
			}
		else if((OnuType == V2R1_ONU_GT861) &&((onupots > 8)||(BoardNum<2)||(BoardNum>5)))/*added by zhengyt 2008-7-3*/
			{
			errFlag = 1;
			}
		else
			{pots[countpots]=onupots;
			countpots++;
			}
	}
	END_TDM_PARSE_POTS_LIST_TO_POT();
	if(errFlag==1)
		{
			vty_out(vty," %% this onu is %s, the  pots list error\r\n", GetDeviceTypeString(OnuType));
			return(CMD_WARNING);
		}
	rc = GetOnuBelongToSG( onudev, &tdmslot_ret, &sgidx_ret, &LogicOnu );
	if (rc ==TDM_VM_NOT_IN_ANY_SG)
		{
		vty_out (vty,"\r\nonu%d/%d%d is not in any %s\r\n", PON_slot, PON_port, PON_onuid, GetGFA6xxxTdmNameString());
		return( CMD_WARNING );
		}
	else if( rc == TDM_VM_ERR )
		{
		vty_out (vty,"\r\n %% executing failed\r\n");	
		return( CMD_WARNING );
		}
	else if (rc ==TDM_VM_NOT_SUPPORT_VOICE)
		{
		vty_out (vty,"\r\nonu%d/%d/%d is not support voice\r\n",PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
		}
	
	if( rc == TDM_VM_OK )
		{
		if((tdmslot_ret  != tdmslot ) || (sgidx_ret  != tdmsgidx ))
			{
			vty_out(vty," %% onu%d/%d/%d is belonged to another %s(%d/%d)\r\n",PON_slot, PON_port, PON_onuid,GetGFA6xxxTdmNameString(),tdmslot_ret,sgidx_ret);
			return( CMD_WARNING );
			}
		}
		
	rc = GetOnuPotsLinkAll(onudev, &PotsList);
	if( rc == VOS_OK )
		{
		if( PotsList != 0 )
			{
			for(i=0; i< countpots; i++)
				{
				if(OnuType==V2R1_ONU_GT861)/*added by zhengyt 2008-7-3*/
					{	
						ulpots=pots[i];
						pots[i]=(BoardNum-2)*8+pots[i];
					}
				else
					{
						ulpots=pots[i];
					}
				if( (PotsList & ( 1<< (pots[i]-1))) != 0)
					{
					rc =DeleteOnuPotsLinkFromSg(onudev, potsboard, pots[i]);
					if (rc ==VOS_OK)
						;
					else if(rc ==TDM_VM_NOT_SUPPORT_VOICE)
						;
					else
						vty_out (vty,"\r\n delete voice-link onu %d/%d/%d pots %d err\r\n",PON_slot, PON_port, PON_onuid,pots[i]);
					}
				else vty_out(vty,"\r\n voice-link onu %d/%d/%d pots %d not configed\r\n",PON_slot, PON_port, PON_onuid,ulpots);
				}
			}
		}
	else {
		vty_out (vty,"\r\n %% executing failed\r\n");	
		}
#if 0
	for (i=0;i<countpots;i++)
		{
			result_msg=GetOnuPotsLinkByDevIdx(onudev, potsboard, (uchar)pots[i], & EnableFlag, & TdmSlot, & SgIdx, &LogicPort);
			if (result_msg==VOS_OK)
				{
					/*vty_out (vty,"\r\n Get onu pots link by devidx success \r\n");
					vty_out (vty,"\r\n the enable flag :%d\r\n",EnableFlag);*/
					if(EnableFlag==2)
						{	
							vty_out (vty,"\r\n the onu pots link disable \r\n");
							return rc;
						}
					else
						{
							result_msg=DeleteOnuPotsLinkFromSg(onudev, potsboard, pots[i]);
							if (result_msg==VOS_OK)
								;
							else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
								;
							else
								vty_out (vty,"\r\n %% executing failed\r\n");
						}
				}
			else if (result_msg==TDM_VM_NOT_IN_ANY_SG)
				vty_out (vty,"\r\n tdm is not in any tdm port\r\n");
			else if (result_msg==TDM_VM_NOT_SUPPORT_VOICE)
				vty_out (vty,"\r\n tdm is not support voice \r\n");
			else 
				{
					vty_out (vty,"\r\n delete onu pots link failure \r\n");
					return rc;
				}
				
		}
	rc=result_msg;
#endif
	return  CMD_SUCCESS;
}

DEFUN(
		    set_voice_link_property,
		    set_voice_link_property_cmd,
		    "set voice-link-property <slot/port/onuid> <1-5> <1-16> <phonenumber> <description>",
		    "set onu pots property\n"
		    "set onu pots phone num and description\n"
		    "input the slot/port/onuid\n"
		    "the onu board list\n"
		    "the port list \n"
		    "the phone numn, eg.62961177\n"
		    "the description of the phone\n"
)
{	STATUS rc=VOS_ERROR;
	LONG lRet;
	STATUS result_msg=VOS_ERROR;
	ULONG PON_slotid,PON_portid,PON_onuid;
	ULONG ulIfIndex = 0,onuboard=0;
	ULONG tdmslot,tdmsgidx,onu,potslink,OnuDeviceIdx,Potsboard=1,ulpotslink;
	char description[PHONEDESC],ulphoennum[MAXPHONENUM_LENGTH];
	ULONG idxs[2];
	BOOL  EnableFlag;
	unsigned char  ulTdmSlot,ulSgIdx,ul_tdmslot,ul_sgidx;
	unsigned short int  ulLogicPort,ul_logiconuid,ul_PonportIdx;
	int OnuType=0;
	
	/*if(argc!=5)
	{
		vty_out(vty,"\r\n%% parameter error\r\n");
		return rc;
	}*/

	lRet = PON_ParseSlotPortOnu( argv[0], &PON_slotid, &PON_portid, &PON_onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;	
	/*OnuDeviceIdx=PON_slotid*10000+PON_portid*1000+PON_onuid;*/
        OnuDeviceIdx=MAKEDEVID(PON_slotid,PON_portid,PON_onuid);

	ul_PonportIdx=GetPonPortIdxBySlot(PON_slotid, PON_portid);
	if(ThisIsValidOnu(ul_PonportIdx,(PON_onuid-1)) != ROK )
		{
		vty_out(vty, " %% onu %d/%d/%d not exist\r\n", PON_slotid, PON_portid, PON_onuid );
		return( CMD_WARNING );
		}
	
	if(GetOnuType(ul_PonportIdx, PON_onuid-1, &OnuType) != ROK)
		{
		vty_out(vty, " %% get onu%d/%d%d type err\r\n", PON_slotid, PON_portid, PON_onuid);
		return( CMD_WARNING);
		}
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&tdmsgidx, (ULONG *)&onu);

	VOS_MemZero(description, sizeof(description));
	VOS_MemZero(idxs, sizeof(idxs));
	VOS_MemZero(ulphoennum, sizeof(ulphoennum));

	if(VOS_StrLen(argv[3]) > MAXPHONENUM_LENGTH)
		vty_out(vty,"\r\n%% parameter 3 exceed maximum string length");
	
	VOS_MemCpy(ulphoennum, argv[3], MAXPHONENUM_LENGTH);
	
	onuboard=VOS_AtoL(argv[1]);
	ulpotslink=VOS_AtoL(argv[2]);

	if(OnuType!=V2R1_ONU_GT861)/*modified by zhengyt 2008-7-22,解决问题单6910.*/
		{
			if(onuboard!=1)
			{
				sys_console_printf ("\r\nthe onu %d/%d/%d is %s,onuboard list error\r\n",PON_slotid,PON_portid,PON_onuid, GetDeviceTypeString(OnuType));
				return rc;
			}
		}
	else
		{
			if(onuboard==1)
			{
				sys_console_printf ("\r\nthe onu %d/%d/%d is %s,onuboard list among 2-5\r\n",PON_slotid,PON_portid,PON_onuid, GetDeviceTypeString(OnuType));
				return rc;
			}
			if(ulpotslink>8)
			{
				sys_console_printf("\r\nthe onu %d/%d/%d is %s,pots link list among 1-8\r\n",PON_slotid,PON_portid,PON_onuid,GetDeviceTypeString(OnuType));
				return rc;
			}
		}

	if(OnuType==V2R1_ONU_GT861)/*added by zhengyt 2008-7-3*/
	{
		potslink=(onuboard-2)*8+ulpotslink;
	}
	else
	potslink=ulpotslink;
	
	/*phonenum=atobcd(argv[2]);*/
	VOS_MemCpy(description, argv[4], PHONEDESC);

	result_msg=GetOnuBelongToSG(OnuDeviceIdx, & ul_tdmslot, & ul_sgidx, &ul_logiconuid);
	if(result_msg==VOS_OK)
{		if(ul_tdmslot==tdmslot&&ul_sgidx==tdmsgidx)
	
		{result_msg=GetOnuPotsLinkByDevIdx(OnuDeviceIdx, (unsigned char )Potsboard,( unsigned char) potslink, & EnableFlag, & ulTdmSlot, &ulSgIdx, &ulLogicPort);
		if(result_msg==VOS_OK)
			{	
				if(EnableFlag==1)
				{
					idxs[0]=tdmsgidx;
					idxs[1]=potslink;
					result_msg=SetOnuPotsLinkPhoneNumByDevIdx( OnuDeviceIdx, Potsboard, (uchar) potslink, (uchar*)ulphoennum);
					/*result_msg=tdm_potsLinkTable_set(LEAF_EPONPOTSLINKPHONECODE, idxs , (uchar*)&phonenum);*/
					if(result_msg==VOS_OK)
						;
					else 
						vty_out(vty,"set onu pots phone code failure\r\n");
					result_msg=SetOnuPotsLinkDescByDevIdx( OnuDeviceIdx, Potsboard, (uchar) potslink, description);
					/*result_msg=tdm_potsLinkTable_set(LEAF_EPONPOTSLINKDESC, idxs , description);*/
					if(result_msg==VOS_OK)
						{
							rc=VOS_OK;
						}
					else
						vty_out(vty,"set onu pots description failure\r\n");
				}
				else
					vty_out(vty,"onu %d/%d/%d board %d pots %d not config\r\n",PON_slotid,PON_portid,PON_onuid,onuboard,ulpotslink);
			}
		else if(result_msg==TDM_VM_NOT_IN_ANY_SG)
			vty_out(vty,"onu%d/%d/%d not in any tdm\r\n ",PON_slotid,PON_portid,PON_onuid);
		else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
			vty_out(vty,"onu%d/%d/%d not support voice\r\n",PON_slotid,PON_portid,PON_onuid);
		else if(result_msg==TDM_VM_ERR)
			vty_out(vty,"\r\n %% executing failed\r\n");
		
			}
	else
		vty_out (vty,"\r\nonu %d/%d/%d is belonged to another %s(%d/%d)\r\n",PON_slotid,PON_portid,PON_onuid,GetGFA6xxxTdmNameString(), ul_tdmslot,ul_sgidx);
}
	else if(result_msg==TDM_VM_NOT_IN_ANY_SG)
			vty_out(vty,"onu%d/%d/%d not in any %s\r\n",PON_slotid,PON_portid,PON_onuid, GetGFA6xxxTdmNameString());
		else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
			vty_out(vty,"onu%d/%d/%d not support voice\r\n",PON_slotid,PON_portid,PON_onuid);
		else if(result_msg==TDM_VM_ERR)
			vty_out(vty,"\r\n %% executing failed\r\n");
	
return rc;

}

#ifdef  _GFA6xxx_VOICE_ONU_POTS_LINK_
#endif
/*显示所有配置的pots 口信息*/
DEFUN (
		   show_voice_link_mapping,
		   show_voice_link_mapping_cmd,
		   "show voice-link-mapping",
		   "show pots link mapping\n"
		   "pots link mapping\n"
)
{	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG ulIfIndex = 0,potslist[MAXONUPOTS], listnum=0,idxs[3],devIdx[MAXONUPERSG];
	ULONG TDM_slotid,TDM_portid,TDM_onu,potsonuboard=1,PotsEnableList,OnuDevIdx=0;
	ULONG PON_slot=0,PON_port=0,PON_onuid=0,PON_portIdx=0;
	unsigned long onunum;
	int i,j=0,k=0,m,OnuType=0;
	potsporttable_row_entry entry;
	unsigned char OnuString[100] = {'\0'};
	ULONG lRet;
	
	/*if(argc!=0)
		vty_out(vty,"\r\n%% parameter error\r\n");*/
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;		
	lRet = PON_GetSlotPortOnu( ulIfIndex,  &TDM_slotid, &TDM_portid , &TDM_onu);
	if( lRet != VOS_OK )
		return CMD_WARNING;
/*	
#ifdef V2R1_GFA6700
	if ((TDM_slotid < (PON5+1))  || (TDM_slotid > (PON1+1)))
		{
	   vty_out( vty, "  %% Error slot %d.\r\n", TDM_slotid );
		return CMD_WARNING;
		}
	if (TRUE  == SYS_MODULE_IS_RUNNING(SYS_LOCAL_MODULE_SLOTNO) )
		{
		if(MODULE_E_GFA_SIG != SYS_MODULE_TYPE(TDM_slotid))
			{
			vty_out( vty, "  %% Error slot %d. This slot is not tdm\r\n", TDM_slotid );
			return CMD_WARNING;
			}
		}
#endif

	if ( (TDM_portid < TDM_SG_MIN) || (TDM_portid > TDM_SG_MAX) )
		{
		vty_out( vty, "  %% no exist tdm port %d/%d. \r\n",TDM_slotid, TDM_portid);
		return CMD_WARNING;
		}	
*/
	VOS_MemZero(potslist, sizeof(potslist));

	VOS_MemZero(idxs, sizeof(idxs));
	VOS_MemZero(devIdx, sizeof(devIdx));
	
	/*result_msg=GetOnuPotsLinkAll(onudevidx, & PotsEnableList);*/
	result_msg=GetAllOnuBelongToSG((uchar)TDM_slotid, (uchar)TDM_portid, &onunum, devIdx);

	
	if(result_msg==VOS_OK)
		{	
			VOS_MemZero(OnuString, sizeof(OnuString));
			sprintf(OnuString,"tdmid  logicport  onuidx  phy-port");
			vty_out(vty,"\r\n%s",OnuString);
			/*vty_out(vty,"\r\n----------------------------------------");*/
									
			for(i=0;i<onunum;i++)
				{		
						OnuDevIdx = (devIdx[i] >> 8);
						GetOnuPotsLinkAll(OnuDevIdx, & PotsEnableList);
						listnum =0;
						VOS_MemZero(potslist, sizeof(potslist));
						k=0;

						for(j=0;j<MAXONUPOTS;j++)
							if((PotsEnableList & (1<<j))!=0)
								{	
									potslist[k++]=(j+1);
									listnum++;
								}
						/*vty_out (vty,"pots num = %d\r\n",listnum );*/
						for(m=0;m<listnum;m++)
						{	
							idxs[0]=OnuDevIdx;
							idxs[1]=potsonuboard;
							idxs[2]=potslist[m];

							PON_slot=GET_PONSLOT(OnuDevIdx)/*OnuDevIdx/10000*/;
							PON_port=GET_PONPORT(OnuDevIdx)/*(OnuDevIdx%10000)/1000*/;
							PON_onuid=GET_ONUID(OnuDevIdx)/*OnuDevIdx%1000*/;

							PON_portIdx=GetPonPortIdxBySlot(PON_slot, PON_port);
							if(GetOnuType(PON_portIdx,  PON_onuid-1, &OnuType) != ROK)
								{
								vty_out(vty, " %% get onu%d/%d%d type err\r\n", PON_slot, PON_port, PON_onuid);
								return( CMD_WARNING);
								}

							if(OnuType==V2R1_ONU_GT861)/*added by zhengyt 2008-7-3*/
								rc=tdm_ExtpotsPortTable_get( idxs , &entry);
								
							else
								rc=tdm_potsPortTable_get (idxs , &entry);
						
							if (rc!=VOS_OK)
								return(CMD_WARNING );
							else 
								{		
									/*vty_out (vty,"onu %d/%d/%d pots %d -------tdm-sg logicPort %d\r\n",ulslot,ulport,ulonuid,potslist[m],entry.logicalport);*/
									VOS_MemZero(OnuString, sizeof(OnuString));
									sprintf(OnuString,"%d/%d",TDM_slotid,TDM_portid);
									vty_out(vty,"\r\n%-7s",OnuString);
									sprintf(OnuString,"%d",entry.logicalport);
									vty_out(vty,"%-11s",OnuString);
									sprintf(OnuString,"%d/%d/%d",PON_slot,PON_port,PON_onuid);
									vty_out(vty,"%-8s",OnuString);
									sprintf(OnuString,"%d/%d",entry.brdIdx,entry.potsIdx);
									vty_out(vty,"%s",OnuString);
									/*sprintf(OnuString,"\r\nonu %d/%d/%d pots %d",ulslot,ulport,ulonuid,potslist[m]);
									vty_out (vty,"%-20s",OnuString);
									sprintf(OnuString,"---");
									vty_out (vty,"%-5s",OnuString);
									sprintf (OnuString," tdm logical port %d",entry.logicalport);
									vty_out (vty,"%-20s",OnuString);*/
								}
						}
				}
			
		}
	vty_out (vty,"\r\n");
	return rc;
}

DEFUN(
		  show_voice_link_onu,
		  show_voice_link_onu_cmd,
		  "show voice-link-onu <slot/port/onuid> {<pots_list>}*1",
		  "show onu pots information\n"
		  "onu pots information\n"
		  "input the slot/port/onuid\n"
		  "the port list value like 1/1-3,1/5\n"
)
{	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	ULONG ulIfIndex = 0;
	ULONG TDM_slotid,TDM_portid,onudevidx,TDM_onu,onupots;
	ulong lRet,PON_ulslot,PON_ulport, PON_onuid;
	ulong pots[MAXONUPOTS],idxs[3];
	int countpots =0,i,onuboard=0,j=0;
	uchar Potsboard=1;
	 bool SupportVoice = FALSE;
	unsigned char PhonuNumber[MAXPHONENUM_LENGTH],Description[PHONEDESC];
	potsporttable_row_entry entry;
	uchar  ultdmslot,ulsgidx;
	unsigned short int  LogicOnuId;
    short int PonPortIndex;
	unsigned char OnuString[100] = {'\0'};
	int OnuType, errFlag = 0;

	/* 取<slot/port/onuid>  参数*/
	lRet = PON_ParseSlotPortOnu( argv[0], &PON_ulslot, &PON_ulport, &PON_onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

    PonPortIndex = GetPonPortIdxBySlot(PON_ulslot, PON_ulport);
	if(ThisIsValidOnu(PonPortIndex, (PON_onuid-1)) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_ulslot, PON_ulport, PON_onuid );
		return( CMD_WARNING );
	}
	if(GetOnuType(PonPortIndex, (PON_onuid-1), &OnuType) != ROK )
	{
		vty_out(vty, " %% get onu%d/%d%d type err\r\n", PON_ulslot, PON_ulport, PON_onuid);
		return( CMD_WARNING);
	}
	/*onudevidx=PON_ulslot*10000+PON_ulport*1000+PON_onuid;*/
        onudevidx=MAKEDEVID(PON_ulslot,PON_ulport,PON_onuid);

	if(!((OnuIsSupportVoice(onudevidx,&SupportVoice) == ROK ) && (SupportVoice ==  TRUE )))
		{
		vty_out(vty,"  %% onu %d/%d/%d is not support voice\r\n", PON_ulslot, PON_ulport, PON_onuid );
		return( CMD_WARNING);
		}

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);

	VOS_MemZero(PhonuNumber, sizeof(PhonuNumber));
	VOS_MemZero(Description, sizeof(Description));

	if(argc!=1)
	{
	BEGIN_TDM_PARSE_POTS_LIST_TO_POT( argv[ 1], onuboard,onupots)
		{
			if((OnuType == V2R1_ONU_GT865) &&((onupots > MAXONUPOTS_GT865)||(onuboard!=1)))
			{
			errFlag = 1;
			}

			else if ((OnuType==V2R1_ONU_GT861)&&((onupots>8)||(onuboard>5)||(onuboard<2)))/*added by zhengyt 2008-7-3*/
			{
			errFlag=1;
			}
			/*
			其他ONU 类型,待扩展
			else{  

				}
			*/
			else
			{
				pots[countpots]=onupots;
				countpots++;
			}
		}
	END_TDM_PARSE_POTS_LIST_TO_POT();

	if(errFlag == 1 )
		{
		vty_out(vty," %% this onu is %s, the  pots list error\r\n", GetDeviceTypeString(OnuType));
		return(CMD_WARNING);
		}

	result_msg=GetOnuBelongToSG( onudevidx, &ultdmslot, &ulsgidx, &LogicOnuId);
	if(result_msg==VOS_OK)

	{
			if((ultdmslot==TDM_slotid)&&(ulsgidx==TDM_portid))
				{	
					vty_out(vty,"\r\nonuidx  pots  group  logicport  off-hook  E1/TS  phonecode  description");
					/*vty_out(vty,"\r\n------------------------------------------------------------------------------");*/
					/*vty_out (vty,"\r\nonu %d/%d/%d belong to tdm port %d",ulslot,ulport,onu_id,ulsgidx);*/
					for(i=0;i<countpots;i++)
					{	idxs[0]=onudevidx;
						idxs[1]=Potsboard;

						if(OnuType==V2R1_ONU_GT861)/*added by zhengyt 2008-7-3*/
							{
								idxs[2]=(onuboard-2)*8+pots[i];
							}
						else
							idxs[2]=pots[i];
						sprintf (OnuString,"%d/%d/%d",PON_ulslot, PON_ulport,PON_onuid);
						vty_out(vty,"\r\n%-8s",OnuString);
						sprintf(OnuString,"%d/%d",onuboard,pots[i]);
						vty_out(vty,"%-6s",OnuString);
						result_msg=GetOnuPotsLinkDescByDevIdx(onudevidx,  Potsboard, idxs[2], Description);
						if(result_msg==VOS_OK)
						{		
							if(OnuType==V2R1_ONU_GT861)
								result_msg=tdm_ExtpotsPortTable_get( idxs , &entry);/*added by zhengyt 2008-7-3*/
							else
								result_msg=tdm_potsPortTable_get( idxs, &entry);
								
							sprintf(OnuString,"%d/%d",ultdmslot,ulsgidx);
							vty_out(vty,"%-9s",OnuString);
							if(result_msg==VOS_OK)
							{	
								if(entry.linkstatus==1)
									{
										sprintf(OnuString,"%d",entry.logicalport);
										vty_out(vty,"%-11s",OnuString);
									}
								else
									{
										continue;
									}
								
								VOS_StrCpy(OnuString, hookstatus[(entry.opstatus-1)]);
								if(VOS_StrCmp(OnuString, hookstatus[1])==0)
									{	sprintf(OnuString,"%s","Y");
										vty_out(vty,"%-8s",OnuString);
										sprintf(OnuString,"%d",entry.e1Idx);
										vty_out(vty,"%2s",OnuString);
										vty_out(vty,"/");
										sprintf(OnuString,"%d",entry.e1tsIdx);
										vty_out(vty,"%-4s",OnuString);
									}
								else 
									{
										sprintf(OnuString,"%s","N");
										vty_out(vty,"%-8s",OnuString);
										vty_out(vty,"       ");
									}
								/*vty_out (vty,"  tdm logical pots :%d\r\n",entry.logicalport);*/
				   				rc=result_msg;
							}
					
							/*vty_out (vty,"  description: %s\r\n",Description);*/
							result_msg=GetOnuPotsLinkPhoneNumByDevIdx(onudevidx,  Potsboard, idxs[2], PhonuNumber);
							if(result_msg==VOS_OK)
							{									
								sprintf(OnuString,"%s",PhonuNumber);
								vty_out(vty,"%-11s",OnuString);
								/*vty_out (vty,"  phone code: %s\r\n",PhonuNumber);*/
							}
						sprintf(OnuString,"%s",Description);
						vty_out(vty,"%s",OnuString);
					
						}
						else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
							vty_out (vty,"\r\nonu %d/%d/%d does not support voice\r\n",PON_ulslot,PON_ulport,PON_onuid);
						else if (result_msg==TDM_VM_NOT_EXIST_LINK_POT)
							vty_out (vty,"\r\nonu %d/%d/%d not exist link pot\r\n",PON_ulslot,PON_ulport,PON_onuid);
						else 
							vty_out (vty,"            ");
							
					}
			}
			else
				vty_out (vty,"\r\nonu %d/%d/%d in tdm %d/%d\r\n",PON_ulslot,PON_ulport,PON_onuid,ultdmslot,ulsgidx);
			
	}
	else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
			vty_out (vty,"\r\nonu %d/%d/%d not support voice\r\n", PON_ulslot,PON_ulport,PON_onuid);
	else if(result_msg==TDM_VM_NOT_IN_ANY_SG)
			vty_out (vty,"\r\nonu %d/%d/%d not in any tdm\r\n", PON_ulslot,PON_ulport,PON_onuid);
	else 
			return rc;
	}
	
else 
{
	result_msg=GetOnuBelongToSG( onudevidx, & ultdmslot, & ulsgidx, & LogicOnuId);
	if(result_msg==VOS_OK)
		{
			if((ultdmslot==TDM_slotid )&& (ulsgidx==TDM_portid))
				{	
					vty_out(vty,"\r\nonuidx  pots  group  logicport  off-hook  E1/TS  phonecode  description");
					/*vty_out(vty,"\r\n------------------------------------------------------------------------------");*/
					/*vty_out (vty,"\r\nonu %d/%d/%d belong to tdm port %d",ulslot,ulport,onu_id,ulsgidx);*/
					countpots = MAXONUPOTS;
					if( OnuType == V2R1_ONU_GT865 )
						countpots = MAXONUPOTS_GT865;
					else if(OnuType==V2R1_ONU_GT861)
						countpots=MAXPERBOARDPOTS_GT861;
					else{ /* 其他ONU 类型,待扩展*/

						}
					
					if(OnuType==V2R1_ONU_GT861)
					{
						for(j=2;j<=5;j++)
						{
							for(i=1;i<=countpots;i++)
							{
								idxs[0]=onudevidx;
								idxs[1]=Potsboard;
								idxs[2]=(j-2)*8+i;

								sprintf (OnuString,"%d/%d/%d",PON_ulslot, PON_ulport,PON_onuid);
									vty_out(vty,"\r\n%-8s",OnuString);
									sprintf(OnuString,"%d/%d",j/*BoardList[j]*/,i);
									vty_out(vty,"%-6s",OnuString);
									result_msg=GetOnuPotsLinkDescByDevIdx(onudevidx,  Potsboard, idxs[2], Description);
									if(result_msg==VOS_OK)
									{	
										result_msg=tdm_ExtpotsPortTable_get( idxs , &entry);
										sprintf(OnuString,"%d/%d",ultdmslot,ulsgidx);
										vty_out(vty,"%-9s",OnuString);
										if(result_msg==VOS_OK)
										{	
											if(entry.linkstatus==1)
												{
													sprintf(OnuString,"%d",entry.logicalport);
													vty_out(vty,"%-11s",OnuString);
												}
											else
												{
													continue;
												}
											
											VOS_StrCpy(OnuString, hookstatus[(entry.opstatus-1)]);
											if(VOS_StrCmp(OnuString, hookstatus[1])==0)
												{	sprintf(OnuString,"%s","Y");
													vty_out(vty,"%-8s",OnuString);
													sprintf(OnuString,"%d",entry.e1Idx);
													vty_out(vty,"%2s",OnuString);
													vty_out(vty,"/");
													sprintf(OnuString,"%d",entry.e1tsIdx);
													vty_out(vty,"%-4s",OnuString);
												}
											else 
												{
													sprintf(OnuString,"%s","N");
													vty_out(vty,"%-8s",OnuString);
													vty_out(vty,"       ");
												}
											/*vty_out (vty,"  tdm logical pots :%d\r\n",entry.logicalport);*/
							   				rc=result_msg;
										}
								
										/*vty_out (vty,"  description: %s\r\n",Description);*/
										result_msg=GetOnuPotsLinkPhoneNumByDevIdx(onudevidx,  Potsboard, idxs[2], PhonuNumber);
										if(result_msg==VOS_OK)
										{									
											sprintf(OnuString,"%s",PhonuNumber);
											vty_out(vty,"%-11s",OnuString);
											/*vty_out (vty,"  phone code: %s\r\n",PhonuNumber);*/
										}
									sprintf(OnuString,"%s",Description);
									vty_out(vty,"%s",OnuString);
								
									}
								else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
									vty_out (vty,"\r\nonu %d/%d/%d does not support voice\r\n",PON_ulslot,PON_ulport,PON_onuid);
								else if (result_msg==TDM_VM_NOT_EXIST_LINK_POT)
									vty_out (vty,"\r\nonu %d/%d/%d not exist link pot\r\n",PON_ulslot,PON_ulport,PON_onuid);
								else 
									vty_out (vty,"            ");
								}
							}
						}
					
					else
						{	
						for(i=1;i<=countpots;i++)
						{	
							idxs[0]=onudevidx;
							idxs[1]=Potsboard;
							idxs[2]=i;
							sprintf (OnuString,"%d/%d/%d",PON_ulslot, PON_ulport,PON_onuid);
							vty_out(vty,"\r\n%-8s",OnuString);
							sprintf(OnuString,"%d/%d",Potsboard,i);
							vty_out(vty,"%-6s",OnuString);
							result_msg=GetOnuPotsLinkDescByDevIdx(onudevidx,  Potsboard, idxs[2], Description);
							if(result_msg==VOS_OK)
							{	
								result_msg=tdm_potsPortTable_get( idxs, &entry);
								sprintf(OnuString,"%d/%d",ultdmslot,ulsgidx);
								vty_out(vty,"%-9s",OnuString);
								if(result_msg==VOS_OK)
								{	
									if(entry.linkstatus==1)
										{
											sprintf(OnuString,"%d",entry.logicalport);
											vty_out(vty,"%-11s",OnuString);
										}
									else
										{
											continue;
										}
									
									VOS_StrCpy(OnuString, hookstatus[(entry.opstatus-1)]);
									if(VOS_StrCmp(OnuString, hookstatus[1])==0)
										{	sprintf(OnuString,"%s","Y");
											vty_out(vty,"%-8s",OnuString);
											sprintf(OnuString,"%d",entry.e1Idx);
											vty_out(vty,"%2s",OnuString);
											vty_out(vty,"/");
											sprintf(OnuString,"%d",entry.e1tsIdx);
											vty_out(vty,"%-4s",OnuString);
										}
									else 
										{
											sprintf(OnuString,"%s","N");
											vty_out(vty,"%-8s",OnuString);
											vty_out(vty,"       ");
										}
									/*vty_out (vty,"  tdm logical pots :%d\r\n",entry.logicalport);*/
					   				rc=result_msg;
								}
						
								/*vty_out (vty,"  description: %s\r\n",Description);*/
								result_msg=GetOnuPotsLinkPhoneNumByDevIdx(onudevidx,  Potsboard, idxs[2], PhonuNumber);
								if(result_msg==VOS_OK)
								{									
									sprintf(OnuString,"%s",PhonuNumber);
									vty_out(vty,"%-11s",OnuString);
									/*vty_out (vty,"  phone code: %s\r\n",PhonuNumber);*/
								}
							sprintf(OnuString,"%s",Description);
							vty_out(vty,"%s",OnuString);
						
							}
							else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
								vty_out (vty,"\r\nonu %d/%d/%d does not support voice\r\n",PON_ulslot,PON_ulport,PON_onuid);
							else if (result_msg==TDM_VM_NOT_EXIST_LINK_POT)
								vty_out (vty,"\r\nonu %d/%d/%d not exist link pot\r\n",PON_ulslot,PON_ulport,PON_onuid);
							else 
								vty_out (vty,"            ");
							}
						}
					}
#if 0
					for(i=1;i<=countpots;i++)
					{	idxs[0]=onudevidx;
						idxs[1]=Potsboard;
						if(OnuType==V2R1_ONU_GT861)/*added by zhengyt 2008-7-3*/
							{	/*VoiceBoardNumGet( onudevidx, &BoardNum, BoardList );*/
								for(j=2;j<=5;j++)
								{
									/*if(BoardList[j]!=0)*/
										idxs[2]=(j-2)*8+i;
									sprintf (OnuString,"%d/%d/%d",PON_ulslot, PON_ulport,PON_onuid);
									vty_out(vty,"\r\n%-8s",OnuString);
									sprintf(OnuString,"%d/%d",j/*BoardList[j]*/,i);
									vty_out(vty,"%-6s",OnuString);
									result_msg=GetOnuPotsLinkDescByDevIdx(onudevidx,  Potsboard, idxs[2], Description);
									if(result_msg==VOS_OK)
									{	
										result_msg=tdm_ExtpotsPortTable_get( idxs , &entry);
										sprintf(OnuString,"%d/%d",ultdmslot,ulsgidx);
										vty_out(vty,"%-9s",OnuString);
										if(result_msg==VOS_OK)
										{	
											if(entry.linkstatus==1)
												{
													sprintf(OnuString,"%d",entry.logicalport);
													vty_out(vty,"%-11s",OnuString);
												}
											else
												{
													continue;
												}
											
											VOS_StrCpy(OnuString, hookstatus[(entry.opstatus-1)]);
											if(VOS_StrCmp(OnuString, hookstatus[1])==0)
												{	sprintf(OnuString,"%s","Y");
													vty_out(vty,"%-8s",OnuString);
													sprintf(OnuString,"%d",entry.e1Idx);
													vty_out(vty,"%2s",OnuString);
													vty_out(vty,"/");
													sprintf(OnuString,"%d",entry.e1tsIdx);
													vty_out(vty,"%-4s",OnuString);
												}
											else 
												{
													sprintf(OnuString,"%s","N");
													vty_out(vty,"%-8s",OnuString);
													vty_out(vty,"       ");
												}
											/*vty_out (vty,"  tdm logical pots :%d\r\n",entry.logicalport);*/
							   				rc=result_msg;
										}
								
										/*vty_out (vty,"  description: %s\r\n",Description);*/
										result_msg=GetOnuPotsLinkPhoneNumByDevIdx(onudevidx,  Potsboard, idxs[2], PhonuNumber);
										if(result_msg==VOS_OK)
										{									
											sprintf(OnuString,"%s",PhonuNumber);
											vty_out(vty,"%-11s",OnuString);
											/*vty_out (vty,"  phone code: %s\r\n",PhonuNumber);*/
										}
									sprintf(OnuString,"%s",Description);
									vty_out(vty,"%s",OnuString);
								
									}
								else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
									vty_out (vty,"\r\nonu %d/%d/%d does not support voice\r\n",PON_ulslot,PON_ulport,PON_onuid);
								else if (result_msg==TDM_VM_NOT_EXIST_LINK_POT)
									vty_out (vty,"\r\nonu %d/%d/%d not exist link pot\r\n",PON_ulslot,PON_ulport,PON_onuid);
								else 
									vty_out (vty,"            ");
								}
					
							}
						else
						{	
						idxs[2]=i;
						sprintf (OnuString,"%d/%d/%d",PON_ulslot, PON_ulport,PON_onuid);
						vty_out(vty,"\r\n%-8s",OnuString);
						sprintf(OnuString,"%d/%d",Potsboard,i);
						vty_out(vty,"%-6s",OnuString);
						result_msg=GetOnuPotsLinkDescByDevIdx(onudevidx,  Potsboard, idxs[2], Description);
						if(result_msg==VOS_OK)
						{	
							result_msg=tdm_potsPortTable_get( idxs, &entry);
							sprintf(OnuString,"%d/%d",ultdmslot,ulsgidx);
							vty_out(vty,"%-9s",OnuString);
							if(result_msg==VOS_OK)
							{	
								if(entry.linkstatus==1)
									{
										sprintf(OnuString,"%d",entry.logicalport);
										vty_out(vty,"%-11s",OnuString);
									}
								else
									{
										continue;
									}
								
								VOS_StrCpy(OnuString, hookstatus[(entry.opstatus-1)]);
								if(VOS_StrCmp(OnuString, hookstatus[1])==0)
									{	sprintf(OnuString,"%s","Y");
										vty_out(vty,"%-8s",OnuString);
										sprintf(OnuString,"%d",entry.e1Idx);
										vty_out(vty,"%2s",OnuString);
										vty_out(vty,"/");
										sprintf(OnuString,"%d",entry.e1tsIdx);
										vty_out(vty,"%-4s",OnuString);
									}
								else 
									{
										sprintf(OnuString,"%s","N");
										vty_out(vty,"%-8s",OnuString);
										vty_out(vty,"       ");
									}
								/*vty_out (vty,"  tdm logical pots :%d\r\n",entry.logicalport);*/
				   				rc=result_msg;
							}
					
							/*vty_out (vty,"  description: %s\r\n",Description);*/
							result_msg=GetOnuPotsLinkPhoneNumByDevIdx(onudevidx,  Potsboard, idxs[2], PhonuNumber);
							if(result_msg==VOS_OK)
							{									
								sprintf(OnuString,"%s",PhonuNumber);
								vty_out(vty,"%-11s",OnuString);
								/*vty_out (vty,"  phone code: %s\r\n",PhonuNumber);*/
							}
						sprintf(OnuString,"%s",Description);
						vty_out(vty,"%s",OnuString);
					
						}
						else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
							vty_out (vty,"\r\nonu %d/%d/%d does not support voice\r\n",PON_ulslot,PON_ulport,PON_onuid);
						else if (result_msg==TDM_VM_NOT_EXIST_LINK_POT)
							vty_out (vty,"\r\nonu %d/%d/%d not exist link pot\r\n",PON_ulslot,PON_ulport,PON_onuid);
						else 
							vty_out (vty,"            ");
						}
					}

			}
#endif
			else
				vty_out (vty,"\r\nonu %d/%d/%d is belonged to another %s(%d/%d)\r\n",PON_ulslot, PON_ulport,PON_onuid,GetGFA6xxxTdmNameString(),ultdmslot,ulsgidx);
			
		}
	else if(result_msg==TDM_VM_NOT_SUPPORT_VOICE)
			vty_out (vty,"\r\nonu %d/%d/%d not support voice\r\n", PON_ulslot, PON_ulport,PON_onuid);
	else if(result_msg==TDM_VM_NOT_IN_ANY_SG)
			vty_out (vty,"\r\nonu %d/%d/%d not in any tdm\r\n",PON_ulslot, PON_ulport,PON_onuid);
	else 
			return rc;
}
	
	vty_out(vty,"\r\n");
	return CMD_SUCCESS;
}

DEFUN(
		  show_voice_link_olt,
		  show_voice_link_olt_cmd,
		  "show voice-link-olt <voice_link_port_list>",
		  "show voice link olt information\n"
		  "voice link information\n"
		  "the voice link information value \n"
)
{
	STATUS rc=VOS_ERROR;
	STATUS result_msg=VOS_ERROR;
	ULONG ulIfIndex = 0;
	ULONG TDM_slotid,TDM_portid,TDM_onu,portlist,ports[MAXLOGICALSGPOTS],countport=0;
	ulong idxs[3],indexs[2];
	ULONG PON_slot,PON_port,PON_onuid,PON_portIdx;
	unsigned char  Description[PHONEDESC],PhonuNumber[MAXPHONENUM_LENGTH];
	potsporttable_row_entry entry;
	potslinktable_row_entry  pentry;
	unsigned char OnuString[100] = {'\0'};
	int OnuType=V2R1_DEVICE_UNKNOWN,ulpots=0,ulboard=0;
	
	/*if(argc!=1)
		vty_out(vty,"\r\n%% parameter error\r\n");*/

	VOS_MemZero(Description, sizeof(Description));
	VOS_MemZero(PhonuNumber, sizeof(PhonuNumber));
	VOS_MemZero(idxs, sizeof(idxs));
	VOS_MemZero(ports, sizeof(ports));

	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onu);

	sprintf(OnuString,"tdmid  logicport   onuidx  pots  off-hook  E1/TS  phonecode  description");
	vty_out(vty,"\r\n%s",OnuString);
	/*vty_out(vty,"\r\n-------------------------------------------------------------------------------");*/


	BEGIN_TDM_PARSE_VOICE_LINK_LIST_TO_PORT( argv[ 0], portlist)
	{
		ports[countport]=portlist;
		countport++;
		/*vty_out(vty,"\r\nport %d",portlist);
		vty_out(vty,"\r\ncount %d",countport);*/
		indexs[0]=TDM_portid;
		indexs[1]=portlist;
		result_msg=tdm_potsLinkTable_get( indexs , & pentry);
		if(result_msg!=VOS_OK )
			{
				sprintf(OnuString,"%d/%d",TDM_slotid,TDM_portid);
				vty_out(vty,"\r\n%-8s",OnuString);
				sprintf(OnuString,"%d",portlist);
				vty_out(vty,"%-11s",OnuString);
				/*vty_out(vty,"\r\nnot exist link port\r\n");*/
			}
		else 
			{
				PON_slot=GET_PONSLOT(pentry.devIdx)/*pentry.devIdx/10000*/;
				PON_port=GET_PONPORT(pentry.devIdx)/*(pentry.devIdx%10000)/1000*/;
				PON_onuid=GET_ONUID(pentry.devIdx)/*pentry.devIdx%1000*/;

				PON_portIdx=GetPonPortIdxBySlot(PON_slot, PON_port);
				if(GetOnuType(PON_portIdx, PON_onuid-1,  &OnuType) != ROK )
					{
					vty_out(vty, " %% get onu%d/%d%d type err\r\n", PON_slot, PON_port, PON_onuid);
					return( CMD_WARNING);
					}
		
				if(OnuType==V2R1_ONU_GT861)
					{	
						ulpots=(int)pentry.potsIdx;
						if(ulpots>8)
							ulpots=ulpots%8;
						if(ulpots==0)
							ulpots=8;
						ulboard=(int)((pentry.potsIdx)/8+2);
					}
				else if(OnuType == V2R1_ONU_GT865)
					{	
						ulboard=1;
						ulpots=pentry.potsIdx;
					}

				/*sys_console_printf("ulpots=%d,pentry->brdIdx=%d,pentry->potsIdx=%d\r\n",ulpots,ulboard,pentry.potsIdx);*/
				sprintf(OnuString,"%d/%d",TDM_slotid,TDM_portid);
				vty_out(vty,"\r\n%-8s",OnuString);
				sprintf(OnuString,"%d",portlist);
				vty_out(vty,"%-11s",OnuString);
				sprintf(OnuString,"%d/%d/%d",PON_slot,PON_port,PON_onuid);
				vty_out(vty,"%-8s",OnuString);
				sprintf(OnuString,"%d/%d",ulboard,ulpots);
				vty_out(vty,"%-6s",OnuString);

				idxs[0]=pentry.devIdx;
				idxs[1]=1;
				idxs[2]=pentry.potsIdx;

				result_msg=tdm_potsPortTable_get( idxs , & entry);
				if(result_msg==VOS_OK)
					{
						if(entry.opstatus==2)
							{
								sprintf(OnuString,"%s","Y");
								vty_out (vty,"%-10s",OnuString);
								sprintf(OnuString,"%d/%d",entry.e1Idx, entry.e1tsIdx);
								vty_out (vty,"%-7s",OnuString);
								/*sprintf(OnuString,"%d",entry.phonecode);*/
								bcdtoa(entry.phonecode, OnuString);
								vty_out(vty,"%-11s",OnuString);
								sprintf(OnuString,"%s",entry.linkdesc);
								vty_out (vty,"%s",OnuString);
							}
						else
							{
								sprintf(OnuString,"%s","N");
								vty_out(vty,"%-10s",OnuString);
								vty_out(vty,"       ",OnuString);
								/*sprintf(OnuString,"%d",entry.phonecode);*/
								bcdtoa(entry.phonecode, OnuString);
								vty_out(vty,"%-11s",OnuString);
								sprintf(OnuString,"%s",entry.linkdesc);
								vty_out (vty,"%s",OnuString);
							}
						
					}
				else 
					return rc;
			}
		
	}
	END_TDM_PARSE_VOICE_LINK_LIST_TO_PORT();

#if   0		

	for(i=0;i<countport;i++)
		{
			indexs[0]=portid;
			indexs[1]=ports[i];
			result_msg=tdm_potsLinkTable_get( indexs , & pentry);
			if(result_msg!=VOS_OK )
				{
					sprintf(OnuString,"%d/%d",slotid,portid);
					vty_out(vty,"\r\n%-8s",OnuString);
					sprintf(OnuString,"%d",ports[i]);
					vty_out(vty,"%-11s",OnuString);
					/*vty_out(vty,"\r\nnot exist link port\r\n");*/
				}
			else 
				{
					ulslot=pentry.devIdx/10000;
					ulport=(pentry.devIdx%10000)/1000;
					ulonuid=pentry.devIdx%1000;
					sprintf(OnuString,"%d/%d",slotid,portid);
					vty_out(vty,"\r\n%-8s",OnuString);
					sprintf(OnuString,"%d",ports[i]);
					vty_out(vty,"%-11s",OnuString);
					sprintf(OnuString,"%d/%d/%d",ulslot,ulport,ulonuid);
					vty_out(vty,"%-8s",OnuString);
					sprintf(OnuString,"%d",pentry.potsIdx);
					vty_out(vty,"%-6s",OnuString);

					idxs[0]=pentry.devIdx;
					idxs[1]=1;
					idxs[2]=pentry.potsIdx;

					result_msg=tdm_potsPortTable_get( idxs , & entry);
					if(result_msg==VOS_OK)
						{
							if(entry.opstatus==2)
								{
									sprintf(OnuString,"%s","Y");
									vty_out (vty,"%-10s",OnuString);
									sprintf(OnuString,"%d",entry.e1tsIdx);
									vty_out (vty,"%-7s",OnuString);
									sprintf(OnuString,"%d",entry.phonecode);
									vty_out(vty,"%-11s",OnuString);
									sprintf(OnuString,"%s",entry.linkdesc);
									vty_out (vty,"%s",OnuString);
								}
							else
								{
									sprintf(OnuString,"%s","N");
									vty_out(vty,"%-10s",OnuString);
									vty_out(vty,"       ",OnuString);
									sprintf(OnuString,"%d",entry.phonecode);
									vty_out(vty,"%-11s",OnuString);
									sprintf(OnuString,"%s",entry.linkdesc);
									vty_out (vty,"%s",OnuString);
								}
						
						}
					else 
						return rc;
				}


			result_msg=GetOnuPotsLinkBySgLogicPort((unsigned char) slotid,( unsigned char) portid, (unsigned short int) ports[i], & PotsEnable, & OnuDeviceIdx, & PotsBoard, & PotsPort);
			if(result_msg==VOS_OK)
				{	
					ulslot=OnuDeviceIdx/10000;
					ulport=(OnuDeviceIdx%10000)/1000;
					ulonuid=OnuDeviceIdx%1000;
					sprintf(OnuString,"%d/%d",slotid,portid);
					vty_out(vty,"\r\n%-8s",OnuString);
					sprintf(OnuString,"%d",ports[i]);
					vty_out(vty,"%-11s",OnuString);
					sprintf(OnuString,"%d/%d/%d",ulslot,ulport,ulonuid);
					vty_out(vty,"%-8s",OnuString);
					sprintf(OnuString,"%d",PotsPort);
					vty_out(vty,"%-6s",OnuString);
					
					/*vty_out (vty,"\r\nonu device : onu %d/%d/%d",ulslot,ulport,ulonuid);
					vty_out (vty,"\r\nbelong to tdm :%d",portid);
					vty_out (vty,"\r\npots status :%d",PotsEnable);
					vty_out (vty,"\r\npots board :%d",PotsBoard);
					vty_out (vty,"\r\npots port :%d",PotsPort);*/

					idxs[0]=OnuDeviceIdx;
					idxs[1]=PotsBoard;
					idxs[2]=PotsPort;
					result_msg=tdm_potsPortTable_get( idxs , & entry);
					if(result_msg==VOS_OK)
						{
							if(entry.opstatus==2)
								{
									sprintf(OnuString,"%s","Y");
									vty_out (vty,"%-10s",OnuString);
									sprintf(OnuString,"%d",entry.e1tsIdx);
									vty_out (vty,"%-7s",OnuString);
								}
							else
								{
									sprintf(OnuString,"%s","N");
									vty_out(vty,"%-10s",OnuString);
									vty_out(vty,"       ",OnuString);
								}
						
						}
					else 
						return rc;

					result_msg=GetOnuPotsLinkPhoneNumBySgLogicPort((unsigned char) slotid, (unsigned char) portid, (unsigned short int) ports[i],  PhonuNumber);
					if(result_msg==VOS_OK)
						{
							sprintf(OnuString,"%s",PhonuNumber);
							vty_out(vty,"%-11s",OnuString);
						}				
					else 
						return rc;

					result_msg=GetOnuPotsLinkDescBySgLogicPort((unsigned char) slotid, (unsigned char )portid,( unsigned short int) ports[i], Description);
					if(result_msg==VOS_OK)
						{
							sprintf(OnuString,"%s",Description);
							vty_out (vty,"%s",OnuString);
						}					
					else 
						return rc;
						}
			else if(result_msg==TDM_VM_NOT_EXIST_LINK_POT)
				{
					sprintf(OnuString,"%d/%d",slotid,portid);
					vty_out(vty,"\r\n%-8s",OnuString);
					sprintf(OnuString,"%d",ports[i]);
					vty_out(vty,"%-11s",OnuString);
					/*vty_out(vty,"\r\nnot exist link port\r\n");*/
				}
				
			else
				return rc;
#endif


		
	
#if  0
	result_msg=GetAllOnuBelongToSG((unsigned char )slotid, (unsigned char )sgidx, & OnuNumber,  OnuDeviceIdx);
	if(result_msg==VOS_OK)
		{
			for(i=0;i<OnuNumber;i++)
			{
				OnuDevIdx = (OnuDeviceIdx[i] >> 8);
				GetOnuPotsLinkAll(OnuDevIdx, & PotsEnableList);
				listnum =0;
				VOS_MemZero(potslist, sizeof(potslist));
				k=0;

				for(j=0;j<MAXONUPOTS;j++)
					
					if((PotsEnableList & (1<<j))!=0)
						{	
							potslist[k++]=(j+1);
							listnum++;
						}
				for(m=0;m<listnum;m++)
					{	
						idxs[0]=OnuDevIdx;
						idxs[1]=potsonuboard;
						idxs[2]=potslist[m];
							
						rc=tdm_potsPortTable_get (idxs , &entry);

						if(rc==VOS_OK)
							{
								vty_out (vty,"\r\nlogical num :%d\r\n",entry.logicalport);
								vty_out (vty,"phone num :%d\r\n",entry.phonecode);
								vty_out (vty,"describtion :%s\r\n",entry.linkdesc);
								vty_out (vty,"status:%s\r\n",portstatus[(entry.linkstatus-1)]);
							}
						else 
							vty_out (vty,"\r\nget pots port table failure\r\n");
					}
					
			}
		}
	else 
		vty_out (vty,"\r\nshow sg pots information failure\r\n");

#endif
	vty_out(vty,"\r\n");
	return CMD_SUCCESS;
}

DEFUN(
		  show_voice_link_mapping_value,
		  show_voice_link_mapping_value_cmd,
		  "show voice-link-mapping <slot/port/onuid> {<pots_list>}*1",
		  "show pots link mapping\n"
		  "pots link mapping\n"
		  "input the slot/port/onuid\n"
		  "input port list value,like 1-3,5\n"
)
{	STATUS rc=VOS_ERROR;
	STATUS result_msg;
	LONG lRet;
	bool SupportVoice = FALSE;
	ULONG TDM_slotid, TDM_portid, TDM_onuid,onudevidx,PotsEnableList,ulIfIndex;
	ULONG potslist[32],listnum=0,potsonuboard=1,idxs[3],portlist;
	int i,j=0,m=0;
	int OnuType, errFlag = 0,BoardNum=0;
	potsporttable_row_entry entry;
	unsigned char OnuString[100] = {'\0'};
	ULONG PON_ulslot,PON_ulport,PON_ulonu,ports[MAXONUPOTS],countport=0;
	UCHAR ul_tdmslot,ul_sgidx;
	unsigned short int  ul_logiconuid;
    short int PonPortIndex;

	/* 取<slot/port/onuid> 参数*/
	lRet = PON_ParseSlotPortOnu( argv[0], &PON_ulslot, &PON_ulport, &PON_ulonu );
	if( lRet != VOS_OK )
		return CMD_WARNING;

    PonPortIndex = GetPonPortIdxBySlot(PON_ulslot, PON_ulport);
	if(ThisIsValidOnu(PonPortIndex, (PON_ulonu-1)) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_ulslot, PON_ulport, PON_ulonu );
		return( CMD_WARNING );
	}
	if(GetOnuType(PonPortIndex, (PON_ulonu-1), &OnuType) != ROK )
	{
		vty_out(vty, " %% get onu%d/%d%d type err\r\n", PON_ulslot, PON_ulport, PON_ulonu);
		return( CMD_WARNING);
	}
	/*onudevidx =PON_ulslot*10000+PON_ulport*1000+PON_ulonu;*/
        onudevidx=MAKEDEVID(PON_ulslot,PON_ulport,PON_ulonu);
	if(!((OnuIsSupportVoice(onudevidx,&SupportVoice) == ROK ) && (SupportVoice ==  TRUE )))
		{
		vty_out(vty,"  %% onu %d/%d/%d is not support voice\r\n", PON_ulslot, PON_ulport, PON_ulonu );
		return( CMD_WARNING);
		}

	VOS_MemZero(potslist, sizeof(potslist));
	VOS_MemZero(idxs, sizeof(idxs));
	VOS_MemZero(ports, sizeof(ports));

	/* TDM 板所在槽位*/
	ulIfIndex = ( ULONG ) ( vty->index ) ;
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&TDM_slotid, (ULONG *)&TDM_portid , (ULONG *)&TDM_onuid);

	result_msg=GetOnuBelongToSG(onudevidx, &ul_tdmslot, &ul_sgidx, &ul_logiconuid);
	if(result_msg == VOS_OK)
	{	
		if(( ul_tdmslot == TDM_slotid )&& (ul_sgidx == TDM_portid))
			{
			sprintf(OnuString,"group  logicport  onuidx   phy-port");
			vty_out(vty,"\r\n%s",OnuString);
			/*vty_out(vty,"\r\n--------------------------------------------------");*/

			if(argc != 1)
			{
				BEGIN_TDM_PARSE_POTS_LIST_TO_POT( argv[1],BoardNum, portlist)
				{
					if((OnuType == V2R1_ONU_GT865) &&((portlist > MAXONUPOTS_GT865)||(BoardNum!=1)))
					{
					errFlag = 1;
					}
					else if((OnuType==V2R1_ONU_GT861)&&((portlist>8)||(BoardNum>5)||(BoardNum<2)))
					{
					errFlag = 1;
					}
					else
					{
						ports[countport]=portlist;
						countport++;
					}
				}
				END_TDM_PARSE_POTS_LIST_TO_POT();

				if(errFlag == 1 )
				{
				vty_out(vty," %% this onu is %s, the  pots list error\r\n", GetDeviceTypeString(OnuType));
				return(CMD_WARNING);
				}

				for(i=0;i<countport;i++)
					{
						idxs[0]=onudevidx;
						idxs[1]=potsonuboard;

						if(OnuType==V2R1_ONU_GT861)
							{	
								idxs[2]=(BoardNum-2)*8+ports[i];
								rc=tdm_ExtpotsPortTable_get( idxs , &entry);
							}
							
						else
							{	
								idxs[2]=ports[i];
								rc=tdm_potsPortTable_get (idxs , &entry);
							}
							
										
						if (entry.linkstatus!=ENABLE)
							{
								sprintf(OnuString,"%d/%d",TDM_slotid, TDM_portid);
								vty_out(vty,"\r\n%-7s",OnuString);
								sprintf(OnuString,"%s","no config");
								vty_out(vty,"%-11s",OnuString);
								sprintf(OnuString,"%d/%d/%d",PON_ulslot,PON_ulport, PON_ulonu);
								vty_out(vty,"%-9s",OnuString);
								sprintf(OnuString,"%d/%d",BoardNum,ports[i]);
								vty_out(vty,"%s",OnuString);
								
							}
					
						else 
							{	
								sprintf(OnuString,"%d/%d", TDM_slotid, TDM_portid);
								vty_out(vty,"\r\n%-8s",OnuString);
								sprintf(OnuString,"%d",entry.logicalport);
								vty_out(vty,"%-10s",OnuString);
								sprintf(OnuString,"%d/%d/%d",PON_ulslot,PON_ulport, PON_ulonu);
								vty_out(vty,"%-9s",OnuString);
								sprintf(OnuString,"%d/%d",BoardNum,ports[i]);
								vty_out(vty,"%s",OnuString);
								
							}
							
					}
				
			}
			else
				{	
					result_msg=GetOnuPotsLinkAll(onudevidx, & PotsEnableList);
					if(result_msg==VOS_OK)
						{
							for(i=0;i<MAXONUPOTS;i++)
							if((PotsEnableList & (1<<i))!=0)
								{	
									potslist[j++]=(i+1);
									listnum++;
								}
							for(m=0;m<listnum;m++)
									{	
										idxs[0]=onudevidx;
										idxs[1]=potsonuboard;
										idxs[2]=potslist[m];

										if(OnuType==V2R1_ONU_GT861)/*added by zhengyt 2008-7-3*/
											rc=tdm_ExtpotsPortTable_get( idxs , &entry);
								
										else
											rc=tdm_potsPortTable_get (idxs , &entry);
											
											
#if  0
										if(VoiceBoardNumGet( onudevidx, &BoardNum)==VOS_OK)
											vty_out(vty,"SUCCESS\r\n");
										
										if(OnuType==V2R1_ONU_GT865)
											{
												idxs[2]=potslist[m];
												rc=tdm_potsPortTable_get (idxs , &entry);
											}
										else
											{
												idxs[2]=(BoardNum-2)*8+potslist[m];
												rc=tdm_potsPortTable_get( idxs , &entry);
												vty_out(vty,"onu is gt861,and pots =%d\r\n",idxs[2]);
											}
#endif						
										if (rc!=VOS_OK)
											return(CMD_WARNING );
										else 
											{	
												sprintf(OnuString,"%d/%d",TDM_slotid,TDM_portid);
												vty_out(vty,"\r\n%-8s",OnuString);
												sprintf(OnuString,"%d",entry.logicalport);
												vty_out(vty,"%-10s",OnuString);
												sprintf(OnuString,"%d/%d/%d",PON_ulslot,PON_ulport, PON_ulonu);
												vty_out(vty,"%-9s",OnuString);
												sprintf(OnuString,"%d/%d",entry.brdIdx,entry.potsIdx);
												vty_out(vty,"%s",OnuString);
												/*vty_out (vty,"\r\nonu %d/%d/%d pots %d -------tdm logicPort %d\r\n",slotid,portid,onuid,potslist[m],entry.logicalport);
												sprintf(OnuString,"\r\nonu %d/%d/%d pots %d",slotid,portid,onuid,potslist[m]);
												vty_out (vty,"%-20s",OnuString);
												sprintf(OnuString,"---");
												vty_out (vty,"%-5s",OnuString);
												sprintf (OnuString," tdm logical port %d",entry.logicalport);
												vty_out (vty,"%-20s",OnuString);*/

											}
										}
						}
					else
						vty_out(vty,"Get onu potslink all failuer\r\n");
				}
			}
		else
			vty_out (vty,"\r\nonu %d/%d/%d is belonged to another %s(%d/%d)\r\n",PON_ulslot, PON_ulport, PON_ulonu,GetGFA6xxxTdmNameString(),ul_tdmslot,ul_sgidx);
		}
	else if( result_msg == TDM_VM_NOT_SUPPORT_VOICE )
		{
		vty_out(vty,"\r\nonu %d/%d/%d not support vocie\r\n", PON_ulslot, PON_ulport, PON_ulonu );
		return CMD_SUCCESS;
		}
	else if( result_msg == TDM_VM_NOT_IN_ANY_SG )
		{
		vty_out(vty,"\r\nonu %d/%d/%d not in any %s\r\n", PON_ulslot, PON_ulport, PON_ulonu,GetGFA6xxxTdmNameString());
		return CMD_SUCCESS;
		}
	
	vty_out (vty,"\r\n");
	return CMD_SUCCESS;
}

#ifdef  _GFA6xxx_ONU_VOICE_OAM_DEBUG 
#endif
/* 以下命令用于查询ONU上语音配置及状态*/
DEFUN (
		   onu_voice_vlan_show,
		   onu_voice_vlan_show_cmd,
		   "debug onu voice-vlan <slot/port/onuid>",
		   "show onu voice information\n"
		   "show onu voice information\n"
		   "show onu voice vlan\n"
		   "input the slot/port/onuid\n"
)
{
	LONG lRet;
	ULONG portid,slotid,onuid,onuDevIdx;
	ULONG tdmslot,sgidx,onu;
	ULONG ulIfIndex = 0;
	unsigned char TdmSlot1=0, SgIdx1=0;
	unsigned short int  OnuLogicId=0;
	int ret;
	short int PonPortIdx = 0;
	VoiceVlanEnable VoiceVlan;

	lRet = PON_ParseSlotPortOnu( argv[0], &slotid, &portid, &onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(slotid, portid, vty) != ROK)
		return(CMD_WARNING);
	
	/*onuDevIdx =slotid*10000+portid*1000+onuid;*/
        onuDevIdx=MAKEDEVID(slotid,portid,onuid);
    PonPortIdx = GetPonPortIdxBySlot(slotid, portid);
	if(ThisIsValidOnu(PonPortIdx, (onuid-1)) != ROK )
		{
		vty_out(vty, " %% onu %d/%d/%d not exist\r\n", slotid, portid, onuid );
		return( CMD_WARNING );
		}

	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&sgidx, (ULONG *)&onu);

	ret = GetOnuBelongToSG(onuDevIdx, &TdmSlot1, &SgIdx1, &OnuLogicId);
	if( ret == TDM_VM_NOT_SUPPORT_VOICE )
		{
		vty_out(vty,"\r\n onu %d/%d/%d not support voice service\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
		}
	else if( ret == TDM_VM_NOT_IN_ANY_SG )
		{
		vty_out(vty,"\r\n onu %d/%d/%d is not configed to any %s\r\n",slotid, portid,onuid, GetGFA6xxxTdmNameString());
		return( CMD_WARNING );
		}
	if((TdmSlot1 != tdmslot ) ||(SgIdx1 != sgidx ))
		{
		vty_out(vty,"\r\n onu %d/%d/%d is belonged to another %s(%d/%d)\r\n",slotid, portid,onuid, GetGFA6xxxTdmNameString(),TdmSlot1, SgIdx1);
		return( CMD_WARNING );
		}

	if( GetOnuOperStatus(PonPortIdx, onuid-1) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty,"\r\n onu %d/%d/%d is off-line\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
		}
	ret = GetOnuVoiceVlan( onuDevIdx, &VoiceVlan);
	if( ret != VOS_OK )
		{
		vty_out(vty," %% Execute err\r\n");
		return( CMD_WARNING );
		}
	vty_out(vty," onu %d/%d/%d voice vlan information\r\n",slotid, portid,onuid );
	if(VoiceVlan.Enable == V2R1_ENABLE )
		vty_out(vty,"voice vlan enabled\r\n");
	else vty_out(vty,"voice vlan disabled\r\n");
	vty_out(vty,"voice vlan priority:%d\r\n", VoiceVlan.Priority );
	vty_out(vty,"voice vlan id:%d\r\n",VoiceVlan.VlanId );

	return( CMD_SUCCESS );
	
}

DEFUN (
		   onu_voice_mac_show,
		   onu_voice_mac_show_cmd,
		   "debug onu voice-mac <slot/port/onuid>",
		   "show onu voice information\n"
		   "show onu voice information\n"
		   "show onu voice mac\n"
		   "input the slot/port/onuid\n"
)
{
	LONG lRet;
	ULONG portid,slotid,onuid,onuDevIdx;
	ULONG tdmslot,sgidx,onu;
	ULONG ulIfIndex = 0;
	unsigned char TdmSlot1=0, SgIdx1=0;
	unsigned short int  OnuLogicId=0;
	int ret;
	short int PonPortIdx = 0;
	unsigned char VoiceSrcMac[BYTES_IN_MAC_ADDRESS], VoiceDstMac[BYTES_IN_MAC_ADDRESS];
	unsigned char VoiceEnable;

	lRet = PON_ParseSlotPortOnu( argv[0], &slotid, &portid, &onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;
	
	if(PonCardSlotPortCheckWhenRunningByVty(slotid, portid, vty) != ROK)
		return(CMD_WARNING);
	
	/*onuDevIdx =slotid*10000+portid*1000+onuid;*/
        onuDevIdx=MAKEDEVID(slotid,portid,onuid);
	PonPortIdx = GetPonPortIdxBySlot(slotid, portid);
	if(ThisIsValidOnu(PonPortIdx, (onuid-1)) != ROK )
		{
		vty_out(vty, " %% onu %d/%d/%d not exist\r\n", slotid, portid, onuid );
		return( CMD_WARNING );
		}
	
	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&sgidx, (ULONG *)&onu);

	ret = GetOnuBelongToSG(onuDevIdx, &TdmSlot1, &SgIdx1, &OnuLogicId);
	if( ret == TDM_VM_NOT_SUPPORT_VOICE )
		{
		vty_out(vty,"\r\n onu %d/%d/%d not support voice service\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
		}
	else if( ret == TDM_VM_NOT_IN_ANY_SG )
		{
		vty_out(vty,"\r\n onu %d/%d/%d is not configed to any %s\r\n",slotid, portid,onuid,GetGFA6xxxTdmNameString());
		return( CMD_WARNING );
		}
	if((TdmSlot1 != tdmslot ) ||(SgIdx1 != sgidx ))
		{
		vty_out(vty,"\r\n onu %d/%d/%d is belonged to another %s(%d/%d)\r\n",slotid, portid,onuid, GetGFA6xxxTdmNameString(),TdmSlot1, SgIdx1);
		return( CMD_WARNING );
		}

	if( GetOnuOperStatus(PonPortIdx, onuid-1) != ONU_OPER_STATUS_UP )
		{
		vty_out(vty,"\r\n onu %d/%d/%d is off-line\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
		}
	ret = GetOnuVoiceEnable(onuDevIdx, &VoiceEnable, VoiceSrcMac, VoiceDstMac);
	if( ret != VOS_OK )
		{
		vty_out(vty," %% Execute err\r\n");
		return( CMD_WARNING );
		}
	vty_out(vty," onu %d/%d/%d voice MAC information\r\n",slotid, portid,onuid );
	if( VoiceEnable == V2R1_ENABLE )
		vty_out(vty,"voice service enabled\r\n");
	else vty_out(vty,"voice service disabled\r\n");
	vty_out(vty,"voice src mac:%02x%02x.%02x%02x.%02x%02x\r\n",VoiceSrcMac[0],VoiceSrcMac[1],VoiceSrcMac[2],VoiceSrcMac[3],VoiceSrcMac[4],VoiceSrcMac[5]);
	vty_out(vty,"voice dst mac:%02x%02x.%02x%02x.%02x%02x\r\n",VoiceDstMac[0],VoiceDstMac[1],VoiceDstMac[2],VoiceDstMac[3],VoiceDstMac[4],VoiceDstMac[5] );

	return( CMD_SUCCESS );
	
}

DEFUN (
		   onu_pots_link_show,
		   onu_pots_link_show_cmd,
		   "debug onu pots-link <slot/port/onuid> ",
		   "show onu pots information\n"
		   "show onu pots information\n"
		   "show onu pots link enabled\n"
		   "input the slot/port/onuid\n"
)
{
	LONG lRet;
	ULONG portid,slotid,onuid,onuDevIdx;
	ULONG tdmslot,sgidx,onu;
	ULONG ulIfIndex;
	unsigned char TdmSlot1=0, SgIdx1=0;
	unsigned short int  OnuLogicId=0;
	int ret,ONU_POTS;
	short int PonPortIdx;
    short int OnuIdx;
	unsigned long PotsLink = 0;
	unsigned int i,OnuType=0,BoardNum=1;

	lRet = PON_ParseSlotPortOnu( argv[0], &slotid, &portid, &onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(slotid, portid, vty) != ROK)
		return(CMD_WARNING);
	
	/*onuDevIdx =slotid*10000+portid*1000+onuid;*/
        onuDevIdx=MAKEDEVID(slotid,portid,onuid);
	PonPortIdx = GetPonPortIdxBySlot(slotid, portid);
    OnuIdx = onuid - 1;
	if(ThisIsValidOnu(PonPortIdx,OnuIdx) != ROK )
	{
		vty_out(vty, " %% onu %d/%d/%d not exist\r\n", slotid, portid, onuid );
		return( CMD_WARNING );
	}

	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&sgidx, (ULONG *)&onu);

	ret = GetOnuBelongToSG(onuDevIdx, &TdmSlot1, &SgIdx1, &OnuLogicId);
	if( ret == TDM_VM_NOT_SUPPORT_VOICE )
		{
		vty_out(vty,"\r\n onu %d/%d/%d not support voice service\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
		}
	else if( ret == TDM_VM_NOT_IN_ANY_SG )
		{
		vty_out(vty,"\r\n onu %d/%d/%d is not configed to any %s\r\n",slotid, portid,onuid,GetGFA6xxxTdmNameString());
		return( CMD_WARNING );
		}
	if((TdmSlot1 != tdmslot ) ||(SgIdx1 != sgidx ))
		{
		vty_out(vty,"\r\n onu %d/%d/%d is belonged to another %s(%d/%d)\r\n",slotid, portid,onuid, GetGFA6xxxTdmNameString(), TdmSlot1, SgIdx1);
		return( CMD_WARNING );
		}

	GetOnuType(PonPortIdx, OnuIdx, &OnuType);
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out(vty,"\r\n onu %d/%d/%d is off-line\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
	}
	
	ret = VOS_ERROR;
	if(OnuType==V2R1_ONU_GT865)
		{
			ONU_POTS=(ONU_POTS_ALL&0xffff);
			ret = GetOnuPotsEnable(onuDevIdx,ONU_POTS_NUM_MAX,ONU_POTS, &PotsLink);

		}
	else if(OnuType==V2R1_ONU_GT861)/*added by zhengyt 2008-7-4*/
		{	
			ret = GetOnuPotsEnable(onuDevIdx,ONU_POTS_NUM_MAX,ONU_POTS_ALL, &PotsLink);

		}
		
	if( ret != VOS_OK )
		{
		vty_out(vty," %% Execute failed\r\n");
		return( CMD_WARNING );
		}
	vty_out(vty," onu %d/%d/%d pots link list\r\n",slotid, portid,onuid );

	
	if(OnuType==V2R1_ONU_GT865)
		{
			for(i=0;i<16;i++)
				{
					if((PotsLink & (1<<i)) != 0)
						vty_out(vty,"voice pots %d/%d enable\r\n",BoardNum,(i+1));
					else vty_out(vty,"voice pots %d/%d disable\r\n",BoardNum, (i+1));
				}
		}
	else if(OnuType==V2R1_ONU_GT861)
		{
			for(BoardNum=2;BoardNum<=5;BoardNum++)
				{
					for(i=0;i<8;i++)
					{
						if((PotsLink&(1<<((BoardNum-2)*8+i)))!=0)
							vty_out(vty,"voice pots %d/%d enable\r\n",BoardNum,(i+1));
						else vty_out(vty,"voice pots %d/%d disable\r\n",BoardNum,(i+1));
					}
			}
		}
#if  0
	for(i=0; i< ONU_POTS_NUM_MAX; i++)
		{
		if(OnuType==V2R1_ONU_GT865)
			{
				if((PotsLink & (1<<i)) != 0)
					vty_out(vty,"voice pots %d enable\r\n",(i+1));
				else vty_out(vty,"voice pots %d disable\r\n", (i+1));
			}
		else  if(OnuType==V2R1_ONU_GT861)
			{
				VoiceBoardNumGet( onuDevIdx, &BoardNum);
					{
						if((PotsLink&(1<<((BoardNum-2)*8+i)))!=0)
							vty_out(vty,"voice pots %d enable\r\n",(i+1));
						else vty_out(vty,"voice pots %d disable\r\n",(i+1));
					}
			}
		}
#endif	
	return( CMD_SUCCESS );
	
}

DEFUN (
		   onu_pots_status_show,
		   onu_pots_status_show_cmd,
		   "debug onu pots-status <slot/port/onuid>",
		   "show onu pots information\n"
		   "show onu pots information\n"
		   "show onu pots status\n"
		   "input the slot/port/onuid\n"
)
{
	LONG lRet;
	ULONG portid,slotid,onuid,onuDevIdx;
	ULONG tdmslot,sgidx,onu;
	ULONG ulIfIndex;
	unsigned char TdmSlot1=0, SgIdx1=0;
	unsigned short int  OnuLogicId=0;
	int ret;
	short int PonPortIdx;
    short int OnuIdx;
	unsigned long PotsStatus = 0;
	unsigned int  i,OnuType=0,BoardNum=1;

	lRet = PON_ParseSlotPortOnu( argv[0], &slotid, &portid, &onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(slotid, portid, vty) != ROK)
		return(CMD_WARNING);
	
	/*onuDevIdx =slotid*10000+portid*1000+onuid;*/
        onuDevIdx=MAKEDEVID(slotid,portid,onuid);
	PonPortIdx = GetPonPortIdxBySlot(slotid, portid);
    OnuIdx = onuid - 1;
	if(ThisIsValidOnu(PonPortIdx,OnuIdx) != ROK )
	{
		vty_out(vty, " %% onu %d/%d/%d not exist\r\n", slotid, portid, onuid );
		return( CMD_WARNING );
	}

	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&sgidx, (ULONG *)&onu);

	ret = GetOnuBelongToSG(onuDevIdx, &TdmSlot1, &SgIdx1, &OnuLogicId);
	if( ret == TDM_VM_NOT_SUPPORT_VOICE )
		{
		vty_out(vty,"\r\n onu %d/%d/%d not support voice service\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
		}
	else if( ret == TDM_VM_NOT_IN_ANY_SG )
		{
		vty_out(vty,"\r\n onu %d/%d/%d is not configed to any %s\r\n",slotid, portid,onuid,GetGFA6xxxTdmNameString());
		return( CMD_WARNING );
		}
	if((TdmSlot1 != tdmslot ) ||(SgIdx1 != sgidx ))
		{
		vty_out(vty,"\r\n onu %d/%d/%d is configed to another %s(%d/%d)\r\n",slotid, portid,onuid, GetGFA6xxxTdmNameString(),TdmSlot1, SgIdx1);
		return( CMD_WARNING );
		}

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out(vty,"\r\n onu %d/%d/%d is off-line\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
	}
	ret = GetOnuPotsStatus(onuDevIdx,&PotsStatus);
	if( ret != VOS_OK )
		{
		vty_out(vty," %% Execute failed\r\n");
		return( CMD_WARNING );
		}

	GetOnuType( PonPortIdx,  OnuIdx, &OnuType);
	
	vty_out(vty," onu %d/%d/%d pots status list\r\n",slotid, portid,onuid );
	if(OnuType==V2R1_ONU_GT865)
		{
			for(i=0; i< 16; i++)
				{	
					if((PotsStatus & (1<<i)) != 0)
						vty_out(vty,"voice pots %d/%-2d off-hook\r\n",BoardNum,(i+1));
					else vty_out(vty,"voice pots %d/%-2d on-hook\r\n",BoardNum, (i+1));
				}
		}
	else if(OnuType==V2R1_ONU_GT861)/*added by zhengyt 2008-7-4*/
		{
			for(BoardNum=2;BoardNum<=5;BoardNum++)
				{	
				for(i=0;i<8;i++)
					{
						if((PotsStatus&(1<<((BoardNum-2)*8+i)))!=0)
							vty_out(vty,"voice pots %d/%-2d off_hook\r\n",BoardNum,(i+1));
						else vty_out(vty,"voice pots %d/%-2d on-hook\r\n",BoardNum,(i+1));
					}
				}
		}
#if  0
	for(i=0; i< ONU_POTS_NUM_MAX; i++)
		{
		if(OnuType==V2R1_ONU_GT865)
			{
				if((PotsStatus & (1<<i)) != 0)
					vty_out(vty,"voice pots %2d off-hook\r\n",(i+1));
				else vty_out(vty,"voice pots %2d on-hook\r\n", (i+1));
			}
		else if(OnuType==V2R1_ONU_GT861)
			{	
				VoiceBoardNumGet( onuDevIdx, &BoardNum);
				if((PotsStatus&(1<<(BoardNum-2)*8+i))!=0)
					vty_out(vty,"voice pots %2d off_hook\r\n",(i+1));
				else vty_out(vty,"voice pots %2d on-hook\r\n",(i+1));
			}
		}
#endif
	return( CMD_SUCCESS );	
}

DEFUN (
		   onu_pots_loop_show,
		   onu_pots_loop_show_cmd,
		   "debug onu pots-loop <slot/port/onuid>",
		   "show onu pots information\n"
		   "show onu pots information\n"
		   "show onu pots loop\n"
		   "input the slot/port/onuid\n"
)
{
	LONG lRet;
	ULONG portid,slotid,onuid,onuDevIdx;
	ULONG tdmslot,sgidx,onu;
	ULONG ulIfIndex;
	unsigned char TdmSlot1=0, SgIdx1=0;
	unsigned short int  OnuLogicId=0;
	int ret;
	short int PonPortIdx;
    short int OnuIdx;
	unsigned char PotsLoop = 0;

	lRet = PON_ParseSlotPortOnu( argv[0], &slotid, &portid, &onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(slotid, portid, vty) != ROK)
		return(CMD_WARNING);
	
	/*onuDevIdx =slotid*10000+portid*1000+onuid;*/
        onuDevIdx=MAKEDEVID(slotid,portid,onuid);
	PonPortIdx = GetPonPortIdxBySlot(slotid, portid);
    OnuIdx = onuid - 1;
	if(ThisIsValidOnu(PonPortIdx,OnuIdx) != ROK )
	{
		vty_out(vty, " %% onu %d/%d/%d not exist\r\n", slotid, portid, onuid );
		return( CMD_WARNING );
	}

	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&sgidx, (ULONG *)&onu);

	ret = GetOnuBelongToSG(onuDevIdx, &TdmSlot1, &SgIdx1, &OnuLogicId);
	if( ret == TDM_VM_NOT_SUPPORT_VOICE )
		{
		vty_out(vty,"\r\n onu %d/%d/%d not support voice service\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
		}
	else if( ret == TDM_VM_NOT_IN_ANY_SG )
		{
		vty_out(vty,"\r\n onu %d/%d/%d is not configed to any %s\r\n",slotid, portid,onuid,GetGFA6xxxTdmNameString());
		return( CMD_WARNING );
		}
	if((TdmSlot1 != tdmslot ) ||(SgIdx1 != sgidx ))
		{
		vty_out(vty,"\r\n onu %d/%d/%d is configed to another %s(%d/%d)\r\n",slotid, portid,onuid, GetGFA6xxxTdmNameString(),TdmSlot1, SgIdx1);
		return( CMD_WARNING );
		}

	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		vty_out(vty,"\r\n onu %d/%d/%d is off-line\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
	}
	ret = GetOnuVoiceLoopbackStatus(onuDevIdx,&PotsLoop);
	if( ret != VOS_OK )
		{
		vty_out(vty," %% Execute failed\r\n");
		return( CMD_WARNING );
		}
	if( PotsLoop == V2R1_ONU_POTS_LOOP_ENABLE )
		vty_out(vty," onu %d/%d/%d pots loop \r\n",slotid, portid,onuid );
	else vty_out(vty," onu %d/%d/%d pots no loop \r\n",slotid, portid,onuid );

	return( CMD_SUCCESS );	
}

DEFUN (
		   tdm_reset,
		   tdm_reset_cmd,
		   "reset",
		   "reset\n"
)
{	
	STATUS result_msg=VOS_ERROR;
	
	result_msg=tdmReset(ENUM_CALLTYPE_NOACK, 0, 0);
	if(result_msg!=VOS_OK)
		vty_out(vty,"\r\n reset tdm failure\r\n");

	else 
		return CMD_SUCCESS;

	return CMD_SUCCESS;
		
}



int tdm_init_func()
{
    return VOS_OK;
}

int tdm_showrun( struct vty * vty )
{    
    return VOS_OK;
}


int tdm_config_write ( struct vty * vty )
{
    return VOS_OK;
}

struct cmd_node tdm_port_node =
{
   TDM_CLI_NODE,
    NULL,
    1
};

LONG tdm_node_install()
{
    install_node( &tdm_port_node, tdm_config_write);
    tdm_port_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_PON);
    if ( !tdm_port_node.prompt )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }
    install_default( TDM_CLI_NODE );
    return VOS_OK;
}

LONG tdm_module_init()
{
    struct cl_cmd_module * tdm_module = NULL;

    tdm_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_PON);
    if ( !tdm_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) tdm_module, sizeof( struct cl_cmd_module ) );

    tdm_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_PON );
    if ( !tdm_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( tdm_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( tdm_module->module_name, "tdm" );

    tdm_module->init_func = tdm_init_func;
    tdm_module->showrun_func = tdm_showrun;
    tdm_module->next = NULL;
    tdm_module->prev = NULL;
/*************/

 if(cmd_rugular_register(&stCMD_Tdm_Pots_List_Check)==no_match)
 	{
		ASSERT( 0 );
 	}


  if ( cmd_rugular_register( &stCMD_Tdm_E1_List_Check ) == no_match )
    {
	ASSERT( 0 );
    }

if( cmd_rugular_register( &stCMD_Tdm_Voice_Link_Port_List_Check)==no_match)
{
	ASSERT( 0 );
}
/***************/

   cl_install_module( tdm_module );

    return VOS_OK;
}



#if 0
enum match_type V2R1_Check_Tdm_List( char * onuid_list )
{
    int len = VOS_StrLen( sg_index );
    ULONG interface_list[ MAX_SGIDX_NUMBER_IN_SG_LIST ];
    int j, if_num = 0;
    int ret = 0;
    ULONG ulOnuId=0;

    char *plistbak = NULL;

    if ( ( !sg_index ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

#endif

LONG tdm_CommandInstall()
{
	install_element( CONFIG_NODE, &into_epon_tdm_node_cmd );
	/*install_element( TDM_CLI_NODE, &into_epon_tdm_sg_node_cmd );*/
	install_element(TDM_CLI_NODE,&set_hdlc_link_master_slave_cmd);
	install_element(TDM_CLI_NODE,&show_status_cmd);
	install_element(TDM_CLI_NODE,&sync_source_set_cmd);
	install_element(TDM_CLI_NODE,&set_voice_vlan_cmd);
	install_element(TDM_CLI_NODE, &undo_voice_vlan_cmd);
       install_element(TDM_CLI_NODE,&show_statistics_hdlc_cmd);
	install_element(TDM_CLI_NODE,&show_voice_onu_list_cmd);
	install_element(TDM_CLI_NODE,&set_e1_link_crc_cmd);
	install_element(TDM_CLI_NODE,&set_e1_link_loopback_cmd);
	install_element(TDM_CLI_NODE,&show_status_e1_link_cmd);
	install_element(TDM_CLI_NODE,&add_voice_onu_cmd);
	install_element(TDM_CLI_NODE,&delete_voice_onu_cmd);
	install_element(TDM_CLI_NODE,&voice_onu_loopback_cmd);
	install_element(TDM_CLI_NODE,&add_voice_link_cmd);
	install_element(TDM_CLI_NODE,&delete_voice_link_cmd);
	install_element(TDM_CLI_NODE,&show_voice_link_mapping_cmd);
	install_element(TDM_CLI_NODE,&show_voice_link_onu_cmd);
	install_element(TDM_CLI_NODE,&show_voice_link_mapping_value_cmd);
	install_element(TDM_CLI_NODE,&show_voice_link_olt_cmd);
	install_element(TDM_CLI_NODE,&set_voice_link_property_cmd);
	install_element(TDM_CLI_NODE,&tdm_reset_cmd);
	install_element(TDM_CLI_NODE,&show_alarm_mask_cmd);
	install_element( CONFIG_NODE,&show_alarm_class_cmd);
	install_element(CONFIG_NODE,&alarm_class_e1_link_cmd);
	install_element(TDM_CLI_NODE,&show_e1_link_loopback_cmd);
	
	
#ifdef ONU_VOICE_OAM_DEBUG
#endif
	install_element(TDM_CLI_NODE,&onu_voice_vlan_show_cmd);
	install_element(TDM_CLI_NODE,&onu_voice_mac_show_cmd);
	install_element(TDM_CLI_NODE,&onu_pots_link_show_cmd);
	install_element(TDM_CLI_NODE,&onu_pots_status_show_cmd);
	install_element(TDM_CLI_NODE,&onu_pots_loop_show_cmd);
	
	return( VOS_OK );
}

LONG TDM_CliInit()
{  
	tdm_node_install();
	tdm_module_init();
	tdm_CommandInstall();
    return VOS_OK;
}

#endif
