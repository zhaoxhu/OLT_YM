

#ifdef __cplusplus
extern"C"{
#endif

#include  "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "gwEponSys.h"

/*#include "sys/main/sys_main.h"*/

#include "tdm_apis.h"
#include "tdm_comm.h"
#include "onu/ExtBoardType.h"

/* begin: added by jianght 20090205  */
#include "e1/e1_apis.h"
#include "eventMain.h"
/* end: added by jianght 20090205 */


typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR e1portIdx;
}PACKED e1porttable_get_pdu;


typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR e1portIdx;
	UCHAR setval;
}PACKED e1porttable_set_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR e1portIdx;	
	UCHAR alarmmask;
	UCHAR crcenable;
	UCHAR lpbctrl;
}PACKED e1porttable_rowset_pdu;


typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR sgIdx;
	ULONG devIdx;
}PACKED tdmonutable_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR sgIdx;
	ULONG devIdx;
	UCHAR setval;
}PACKED tdmonutable_set_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR sgIdx;
	ULONG devIdx;
	USHORT logiconuIdx;
	UCHAR rowstatus;
}PACKED tdmonutable_rowset_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR linksgIdx;
	USHORT linksgportIdx;
}PACKED potslinktable_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR linksgIdx;
	USHORT linksgportIdx;
	UCHAR setval[32];
}PACKED potslinktable_set_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR linksgIdx;
	USHORT linksgportIdx;
	ULONG onuIdx;
	UCHAR onuPotsBrd;
	UCHAR onuPotsIdx;
	ULONG linkphonecode;
	UCHAR linkdesc[32];
	UCHAR potsLinkRowStatus;
}PACKED potslinktable_rowset_pdu;


typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	ULONG devIdx;
	UCHAR brdIdx;
	UCHAR potsIdx;
}PACKED potsporttable_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR sgifIdx;
}PACKED sgtable_get_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR sgifIdx;
	USHORT setval;
}PACKED sgtable_set_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR sgifIdx;
	USHORT mastere1;
	USHORT slavee1;
	USHORT e1clk;
	USHORT vlanen;
	USHORT vlanid;
	USHORT vlanpri;
}PACKED sgtable_rowset_pdu;

typedef struct{
	UCHAR tableIdx;
	UCHAR leafIdx;
	UCHAR sgifIdx;
}PACKED hdlcstattable_get_pdu;



typedef struct{
	char softwarever[9];
	char fpgaver[5];
}ver_entry;

typedef struct{
	USHORT seriealno;
	ULONG filelen;
	ULONG offset;
	USHORT sendlen;
}PACKED tdm_trans_req_pdu, tdm_trans_ack_pdu;

typedef tdm_e1_alm_lvl_entry tdm_e1_alm_lvl_set_pdu;

/*响应包处理函数集合*/
typedef struct{
	UCHAR msgType;
	UCHAR msgSubType;
	UCHAR tableIdx;
	FUNC_PRC prcfunc;
}prc_func_t;


extern UCHAR g_tdmDebugFlag;

extern STATUS process_rtn_sg_onu_get_all(const char *pBuf, USHORT len, void *ret_ptr); 
extern STATUS process_rtn_pots_port_get_all( const char *pBuf, USHORT len, void * pData );

#define	MAX_RTN_PROCESS	100
int frnum = 0;
/*
static const UCHAR upvmac_pre[5] = {0x00, 0x0f, 0xe9, 0x01, 0x00};
static const UCHAR downvmac_pre[4] = { 0x00, 0x0f, 0xe9, 0x00 };
*/
prc_func_t gPrcFuncArray[MAX_RTN_PROCESS] = {
	{ MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_READVER, 0, NULL},
	{ MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_LOADFPGA, 0,  NULL},
	{ MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_RUN, 0, NULL },
	{ MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_QUERY, 0, NULL },
	{ MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_RESET, 0, NULL },
		 
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1PORTTABLE_INDEX, NULL },		
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1PORTTABLE_INDEX, NULL },	
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1PORTTABLE_INDEX, NULL },	
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1PORTTABLE_INDEX, NULL },
		 
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, TDMONUTABLE_INDEX, NULL },		
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, TDMONUTABLE_INDEX, NULL },	
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, TDMONUTABLE_INDEX, NULL },	
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, TDMONUTABLE_INDEX, NULL },	
      
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, POTSLINKTABLE_INDEX, NULL },		
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, POTSLINKTABLE_INDEX, NULL },	
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, POTSLINKTABLE_INDEX, NULL },	
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, POTSLINKTABLE_INDEX, NULL },		
      
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, SGTABLE_INDEX, NULL },		
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, SGTABLE_INDEX, NULL },	
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, SGTABLE_INDEX, NULL },	
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, SGTABLE_INDEX, NULL },	
      
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, POTSPORTTABLE_INDEX, NULL },		
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, POTSPORTTABLE_INDEX, NULL },
      
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, SGHDLCSTATTABLE_INDEX, NULL },		
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, SGHDLCSTATTABLE_INDEX, NULL },		

	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_ALLONU, 0, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_ALLPOTS, 0, NULL },
      
	{ MSG_TYPE_TRACEDEBUG, MSG_SUBTYPE_DEBUG_RES, 0, NULL },
	{ MSG_TYPE_TRACEDEBUG, MSG_SUBTYPE_DEBUG_SET_RES, 0, NULL },

	/*added by wangxy 2007-11-08*/
	{ MSG_TYPE_TRANSFILE, MSG_SUBTYPE_UPDATE_RES, 0, NULL },
	{ MSG_TYPE_TRANSFILE, MSG_SUBTYPE_TRANS_RES, 0, NULL },	
	{ MSG_TYPE_TRANSFILE, MSG_SUBTYPE_TRANS_END_RES, 0, NULL },
	{ MSG_TYPE_TRANSFILE, MSG_SUBTYPE_UPDATE_OK, 0, NULL },
	/*end*/

	/* begin: added by jianght 20090205 新增3个表 */
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1LINKTABLE_INDEX, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1LINKTABLE_INDEX, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1LINKTABLE_INDEX, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1LINKTABLE_INDEX, NULL },

	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1PORTTABLE_INDEX_new, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1PORTTABLE_INDEX_new, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1PORTTABLE_INDEX_new, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1PORTTABLE_INDEX_new, NULL },

	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1VLANTABLE_INDEX, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1VLANTABLE_INDEX, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1VLANTABLE_INDEX, NULL },
	{ MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1VLANTABLE_INDEX, NULL }
	/* end: added by jianght 20090205 */
};

#if 0

typedef struct{
	UCHAR table_index;
	UCHAR index_num;
	UCHAR index_len[4];
}table_index_desc_t;


static table_index_desc_t g_index_desc_table[6] = {
	{ E1PORTTABLE_INDEX, 4, {1,1,1,1} },
	{ TDMONUTABLE_INDEX, 1, {4,0,0,0} },
	{ POTSLINKTABLE_INDEX, 2, {1,2,0,0} },
	{ POTSPORTTABLE_INDEX, 3, {3,1,1,0} },
	{ SGTABLE_INDEX, 1, {1,0,0,0} },
	{ SGHDLCSTATTABLE_INDEX, 1, {1,0,0,0} }
};

static UCHAR getIndexLen( ULONG table_index, ULONG offset )
{
	if( table_index > 6 || table_index == 0 ) return 0;
	if( offset > g_index_desc_table[table_index].index_num || offset == 0)
		return 0;

	return g_index_desc_table[table_index-1].index_len[offset-1];
}
#endif
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
/* Modified begin: added by jianght 20090205  */
int swapToMinOnuIdx( ULONG onuIdx )
/* Modified end: added by jianght 20090205 */
{
	int ret = -1;

	ULONG ponIdx = 0, onuIndex = 0;

	if( parseOnuIndexFromDevIdx( onuIdx, &ponIdx, &onuIndex ) != -1 )
	{
		short int swapPort = 0;
		ULONG idx = 0;
		
		if( PonPortSwapPortQuery( ponIdx, &swapPort ) == ROK )
		{
			ULONG newOnuIdx = parseDevidxFromPonOnu( swapPort, onuIndex );
			if( newOnuIdx != 0 && newOnuIdx < onuIdx )
				idx = newOnuIdx;
			else
				idx = onuIdx;
		}
		else
			idx = onuIdx;

		ret = idx;
	}

	return ret;
}

/* Modified begin: added by jianght 20090205  */
int reverseSwapOnuIdx( ULONG onuIdx )
/* Modified end: added by jianght 20090205 */
{
	int ret = -1;
	ULONG ponIdx = 0, onuIndex = 0;

	if( parseOnuIndexFromDevIdx( onuIdx, &ponIdx, &onuIndex ) != -1 )
	{
		short int swapPort = 0;
		if( PonPortSwapPortQuery( ponIdx, &swapPort ) == ROK )
		{
			if( PonPortHotStatusQuery( ponIdx ) == V2R1_PON_PORT_SWAP_ACTIVE )
				ret = onuIdx;
			else
				ret = parseDevidxFromPonOnu( swapPort, onuIndex );
		}
		else
			ret = onuIdx;
	}
	
	return ret;
}

static STATUS process_rtn_readver( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = sizeof( ver_entry)+8;

	if( pdu->type == MSG_TYPE_DEVMANAGE && pdu->subType == MSG_SUBTYPE_READVER && 
		len >= length && pdu->msgCode == 0 )
	{
		VOS_MemCpy( ret_ptr, pdu->msgData, sizeof( ver_entry ) );
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_loadfpga( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 8;

	if( pdu->type == MSG_TYPE_DEVMANAGE && pdu->subType == MSG_SUBTYPE_LOADFPGA&& 
		len >= length && pdu->msgCode == 0 )
		rc = VOS_OK;
	
	return rc;
}

static STATUS process_rtn_runnotify( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_OK;
	
	return rc;
}

static STATUS process_rtn_reset( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_OK;
	
	return rc;
}

static STATUS process_rtn_query( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 9;

	if( pdu->type == MSG_TYPE_DEVMANAGE && pdu->subType == MSG_SUBTYPE_QUERY&& 
		len >= length && pdu->msgCode == 0 )
	{
		*(UCHAR*)ret_ptr = (UCHAR)pdu->msgData[0];
		rc = VOS_OK;
	}
	
	return rc;
}


static STATUS process_rtn_conf_set( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 10;

	if( pdu->type == MSG_TYPE_CONFIG 
		&& ( pdu->subType == MSG_SUBTYPE_SET_RES 
			|| pdu->subType == MSG_SUBTYPE_SETROW_RES 
			|| pdu->subType == MSG_SUBTYPE_E1_LVL_SET ) 
		&& len >= length 
		&& pdu->msgCode == 0 )
	{
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_tdmOnuTable_set( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 10;

	if( pdu->type == MSG_TYPE_CONFIG && 
		( pdu->subType == MSG_SUBTYPE_SET_RES || pdu->subType == MSG_SUBTYPE_SETROW_RES ) &&
		len >= length && pdu->msgCode == 0 )
	{
		*(USHORT*)ret_ptr = *(USHORT*)(pdu->msgData);
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_conf_e1porttable_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 8+sizeof(e1porttable_row_entry)+2;

	if( pdu->type == MSG_TYPE_CONFIG && 
		( pdu->subType == MSG_SUBTYPE_GETNEXT_RES || pdu->subType == MSG_SUBTYPE_GET_RES ) &&
		len >= length && pdu->msgCode == 0 &&
		pdu->msgData[0] == E1PORTTABLE_INDEX )
	{
		VOS_MemCpy( ret_ptr, pdu->msgData+2,  sizeof( e1porttable_row_entry ) );
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_conf_tdmonutable_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 8+sizeof(tdmonutable_row_entry)+2;

	if( pdu->type == MSG_TYPE_CONFIG && 
		( pdu->subType == MSG_SUBTYPE_GETNEXT_RES || pdu->subType == MSG_SUBTYPE_GET_RES ) &&
		len >= length && pdu->msgCode == 0 &&
		pdu->msgData[0] == TDMONUTABLE_INDEX )
	{
		tdmonutable_row_entry *p = (tdmonutable_row_entry*)(pdu->msgData+2);

		if( g_tdmDebugFlag & TDM_DEBUG_INFO )
		{
			sys_console_printf( "\r\n" );
			reportd( "devIdx:", (int)p->devIdx );
			reportd( "sgIdx:", (int)p->sgIdx );
			reportd( "logicOnu:", (int)p->logiconuIdx );
			reportd( "serviceStaus:", (int)p->serviceIdx );
			reportd( "rowStatus:", (int)p->rowstatus );
			reportd( "potsloopen:", (int)p->potsloopenable );
			sys_console_printf( "\r\n" );
		}
		VOS_MemCpy( ret_ptr, pdu->msgData+2,  sizeof( tdmonutable_row_entry ));
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_conf_potslinktable_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 8+sizeof(potslinktable_row_entry)+2;

	if( pdu->type == MSG_TYPE_CONFIG && 
		( pdu->subType == MSG_SUBTYPE_GETNEXT_RES || pdu->subType == MSG_SUBTYPE_GET_RES ) &&
		len >= length && pdu->msgCode == 0 &&
		pdu->msgData[0] == POTSLINKTABLE_INDEX )
	{

		potslinktable_row_entry *p = (potslinktable_row_entry*)(pdu->msgData+2);

		if( g_tdmDebugFlag & TDM_DEBUG_INFO )
		{
			sys_console_printf( "\r\n" );
			reportd( "linksgIdx:", (int)p->linksgIdx );
			reportd( "linksgportIdx:", (int)p->linksgportIdx );
			reportd( "devIdx:", (int)p->devIdx );
			reportd( "brdIdx:", (int)p->brdIdx );
			reportd( "potsIdx:", (int)p->potsIdx );
			reportd( "phonecode:", (int)p->phonecode );
			reports( "linkdesc:", p->linkdesc );
			reportd( "linkrowstatus:", (int)p->linkrowstatus );
			sys_console_printf( "\r\n" );
		}
		
		VOS_MemCpy( ret_ptr, pdu->msgData+2,  sizeof( potslinktable_row_entry ));
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_conf_sgtable_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 8+sizeof(sgtable_row_entry)+2;

	if( pdu->type == MSG_TYPE_CONFIG && 
		( pdu->subType == MSG_SUBTYPE_GETNEXT_RES || pdu->subType == MSG_SUBTYPE_GET_RES ) &&
		len >= length && pdu->msgCode == 0 &&
		pdu->msgData[0] == SGTABLE_INDEX )
	{

		sgtable_row_entry *p = (sgtable_row_entry*)(pdu->msgData+2);

		if( g_tdmDebugFlag & TDM_DEBUG_INFO )
		{
			sys_console_printf( "\r\n" );
			reportd( "sgifIdx:", (int)p->sgifIdx );
			reportd( "clusterIdx:", (int)p->clusterIdx );
			reportd( "mastere1:", (int)p->mastere1 );
			reportd( "slavee1:", (int)p->slavee1 );
			reportd( "e1clk:", (int)p->e1clk );
			reportd( "vlanen:", (int)p->vlanen );		
			reportd( "vlanid:", (int)p->vlanid );
			reportd( "vlanpri:", (int)p->vlanpri );	
			sys_console_printf( "\r\n" );
		}
		
		VOS_MemCpy( ret_ptr, pdu->msgData+2,  sizeof( sgtable_row_entry ) );
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_conf_potsporttable_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 8+sizeof(potsporttable_row_entry)+2;

	if( pdu->type == MSG_TYPE_CONFIG && 
		( pdu->subType == MSG_SUBTYPE_GETNEXT_RES || pdu->subType == MSG_SUBTYPE_GET_RES ) &&
		len >= length && pdu->msgCode == 0 &&
		pdu->msgData[0] == POTSPORTTABLE_INDEX )
	{
		VOS_MemCpy( ret_ptr, pdu->msgData+2,  sizeof( potsporttable_row_entry ) );
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_conf_hdlcstattable_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 8+sizeof(hdlcstattable_row_entry)+2;

	if( pdu->type == MSG_TYPE_CONFIG && 
		( pdu->subType == MSG_SUBTYPE_GETNEXT_RES || pdu->subType == MSG_SUBTYPE_GET_RES ) &&
		len >= length && pdu->msgCode == 0 &&
		pdu->msgData[0] == SGHDLCSTATTABLE_INDEX )
	{
		VOS_MemCpy( ret_ptr, pdu->msgData+2,  sizeof( hdlcstattable_row_entry ) );
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_debug_get( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 8+4;

	if( pdu->type == MSG_TYPE_CONFIG && 
		( pdu->subType == MSG_SUBTYPE_DEBUG_RES &&
		len >= length && pdu->msgCode == 0 ) )
	{
		*(ULONG*)ret_ptr = *(ULONG*)pdu->msgData;
		rc = VOS_OK;
	}
	
	return rc;
}

static STATUS process_rtn_debug_set( const char *pdata, USHORT len, void* ret_ptr )
{
	STATUS rc = VOS_ERROR;

	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 8;

	if( pdu->type == MSG_TYPE_CONFIG && 
		( pdu->subType == MSG_SUBTYPE_DEBUG_SET_RES &&
		len >= length && pdu->msgCode == 0 ) )
		rc = VOS_OK;
	
	return rc;
}

static STATUS process_rtn_e1_lvl_get(const char *pdata, USHORT len, void *ret_ptr)
{
	STATUS ret = VOS_ERROR;
	
	tdm_pdu_t * pdu = (tdm_pdu_t*)pdata;
	const USHORT length = 18;

	if( pdu->type == MSG_TYPE_CONFIG 
		&& ( pdu->subType == MSG_SUBTYPE_E1_LVL_GET
			&& len >= length && pdu->msgCode == 0 ))
	{
		memcpy( ret_ptr, pdu->msgData, 10 );
		ret = VOS_OK;
	}
	
	return ret;
}

static STATUS process_rtn_no_match( const char* pdata, USHORT len, void *ret_ptr )
{
	tdm_pdu_t * pdu = (tdm_pdu_t*) pdata;
	
	sys_console_printf( "\r\n\r\nNO found match process rtn function: msgType=%d, msgSubType=%d, tableIndex=%d",
		(int)pdu->type, (int)pdu->subType, (int)pdu->msgData[0] );

	return VOS_OK;
}

/* begin: added by jianght 20090205  */
extern void init_e1_process_function(void);
/* end: added by jianght 20090205 */

void init_tdm_process_function( void )
{
	initTdmComm();
	
	registRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_READVER, 0, process_rtn_readver );
	registRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_LOADFPGA, 0, process_rtn_loadfpga );
	registRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_QUERY, 0, process_rtn_query );

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1PORTTABLE_INDEX , process_rtn_conf_e1porttable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1PORTTABLE_INDEX , process_rtn_conf_e1porttable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1PORTTABLE_INDEX , process_rtn_conf_set );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1PORTTABLE_INDEX , process_rtn_conf_set );

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, TDMONUTABLE_INDEX , process_rtn_conf_tdmonutable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, TDMONUTABLE_INDEX , process_rtn_conf_tdmonutable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, TDMONUTABLE_INDEX , process_rtn_tdmOnuTable_set );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, TDMONUTABLE_INDEX , process_rtn_tdmOnuTable_set );	

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, POTSLINKTABLE_INDEX , process_rtn_conf_potslinktable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, POTSLINKTABLE_INDEX , process_rtn_conf_potslinktable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, POTSLINKTABLE_INDEX , process_rtn_conf_set );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, POTSLINKTABLE_INDEX , process_rtn_conf_set );	

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, SGTABLE_INDEX , process_rtn_conf_sgtable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, SGTABLE_INDEX , process_rtn_conf_sgtable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, SGTABLE_INDEX , process_rtn_conf_set );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, SGTABLE_INDEX , process_rtn_conf_set );	

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, POTSPORTTABLE_INDEX , process_rtn_conf_potsporttable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, POTSPORTTABLE_INDEX , process_rtn_conf_potsporttable_get );

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, SGHDLCSTATTABLE_INDEX , process_rtn_conf_hdlcstattable_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, SGHDLCSTATTABLE_INDEX , process_rtn_conf_hdlcstattable_get );	

	registRecvHandler( MSG_TYPE_TRACEDEBUG, MSG_SUBTYPE_DEBUG_RES, 0 , process_rtn_debug_get );
	registRecvHandler( MSG_TYPE_TRACEDEBUG, MSG_SUBTYPE_DEBUG_SET_RES, 0 , process_rtn_debug_set );

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_ALLONU, 0 , process_rtn_sg_onu_get_all );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_ALLPOTS, 0 , process_rtn_pots_port_get_all );

	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_E1_LVL_GET, 0, process_rtn_e1_lvl_get );
	registRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_E1_LVL_SET, 0, process_rtn_conf_set );

	
	initTdmUpgradeRtnProcessFunc();
	
	/* begin: added by jianght 20090205  */
	init_e1_process_function();
        /* end: added by jianght 20090205 */
	
}

 FUNC_PRC getRecvHandler( int msgType, int msgSubType, int tableIdx )
{
	UINT i=0;
	for( ; i<sizeof(gPrcFuncArray)/sizeof(prc_func_t);i++ )
		if( gPrcFuncArray[i].msgType == msgType && gPrcFuncArray[i].msgSubType == msgSubType &&
		gPrcFuncArray[i].tableIdx == tableIdx && gPrcFuncArray[i].prcfunc != NULL )
			return gPrcFuncArray[i].prcfunc;

	return process_rtn_no_match;
}

 USHORT buildPduHead( tdm_pdu_t *ppdu,  int msgType, int msgSubType )
{
	ppdu->type = msgType;
	ppdu->subType = msgSubType;
	ppdu->msgCode = 0;

	return 8;
}

STATUS registRecvHandler( int msgType, int msgSubType, int tableIdx, FUNC_PRC handler )
{
	STATUS rc = VOS_ERROR;
	UINT i=0;
	int firstblank = -1;
	
	for( i=0; i<sizeof(gPrcFuncArray)/sizeof(prc_func_t ); i++ )
	{
		if( gPrcFuncArray[i].msgType == msgType && gPrcFuncArray[i].msgSubType == msgSubType && 
			gPrcFuncArray[i].tableIdx == tableIdx )
		{
			gPrcFuncArray[i].prcfunc = handler;
			rc = VOS_OK;
			break;
		}
		else if( gPrcFuncArray[i].msgType == 0 && gPrcFuncArray[i].msgSubType == 0 && gPrcFuncArray[i].tableIdx == 0 && firstblank == -1)
			firstblank = i;
	}

	if( rc == VOS_ERROR && firstblank != -1 )
	{
		gPrcFuncArray[firstblank].msgType = msgType;
		gPrcFuncArray[firstblank].msgSubType = msgSubType;
		gPrcFuncArray[firstblank].tableIdx = tableIdx;
		gPrcFuncArray[firstblank].prcfunc = handler;
		rc = VOS_OK;
	}

	return rc;
}

BOOL isRegistedHandler( int msgType, int msgSubType, int tableIdx )
{
	return ( getRecvHandler( msgType, msgSubType, tableIdx ) == NULL )?FALSE:TRUE;
}


STATUS tdmReadVersion( UCHAR calltype, ULONG callnotifier, UCHAR * softwarever, 
								USHORT *sfverlen, UCHAR *fpgaver, USHORT *fpgaverlen )
{
	
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0;
	char *pRecv = NULL;
	tdm_pdu_t* pdu = NULL;

	if( calltype == ENUM_CALLTYPE_SYN && 
		(softwarever == NULL || sfverlen==NULL || fpgaver==NULL || fpgaverlen==NULL ) )
		return rc;

	
	pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu, MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_READVER );


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{		
		if( calltype == ENUM_CALLTYPE_SYN )
		{

			ver_entry verentry;
			FUNC_PRC pHandler = getRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_READVER, 0 );
			
			if( pHandler && VOS_OK == (*pHandler)( (char*)pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, &verentry  ) )
			{
				VOS_MemCpy( softwarever, verentry.softwarever, 9 );
				*sfverlen = 9;
				VOS_MemCpy( fpgaver, verentry.fpgaver, 5 );
				*fpgaverlen = 5;				
				rc = VOS_OK;
				tdmCommMsgFree( (char*)pRecv);
			}
		}
		else if( calltype == ENUM_CALLTYPE_FUNC || calltype == ENUM_CALLTYPE_MSG )
			rc = VOS_OK;
				
	}	
	return rc;
	
}


STATUS tdmLoadFpga( UCHAR calltype, ULONG callnotifier, UCHAR m1load, UCHAR m2load, UCHAR m3load )
{
	
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0;
	char *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );
	
	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu,  MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_LOADFPGA );
	
	pdu->msgData[0]=m1load;
	pdu->msgData[1]=m2load;
	pdu->msgData[2]=m3load;

	pdulen+=3;


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN  )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_LOADFPGA, 0 ))( (char*)pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL  ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );			
		}
				
	}
	
	return rc;
	
}

STATUS tdmRunNotify( UCHAR calltype, ULONG callnotifier )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0;
	char *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );
	
	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu,  MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_RUN );


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN  )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_RUN, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL  ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );			
		}
				
	}
	
	return rc;
}

STATUS tdmStatusQuery ( UCHAR calltype, ULONG callnotifier, UCHAR *status )
{
	
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0;
	char *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );
	
	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu,  MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_QUERY );


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_QUERY, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, status  ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );			
		}
				
	}

	return rc;
	
}

/*begin: 
	modified by wangxiaoyu 2008-01-09
	added 3rd parameter 'target', 1 -- means reset the first FPGA;
								  2 -- means reset the second FPGA;	
								  3 -- means reset the third FPGA;
								  others -- means reset TDM board
end*/
STATUS tdmReset ( UCHAR calltype, ULONG callnotifier, USHORT target )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0;
	char *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	
	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu,  MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_RESET );

	/*begin: added by wangxiaoyu 2008-01-09*/
	pdu->msgCode = target;
	/*end*/

	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_RESET, 0 ))( (char*)pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );			
		}
				
	}
	
	return rc;	
}

#if E1PORTTABLE_FUNC
#endif
#if 0
/*E1告警屏蔽已解决，但是crc使能还未解决。add by shixh@20070401*/
/*当TDM板从启时恢复先前配置的CRC值*/
STATUS retrieve_tdm_e1portTable()
{
	e1porttable_row_entry  *pEntry;
	ULONG idxs[3];
	UCHAR leafIdx,Val;
	int i;
	idxs[0]=1;
	idxs[1]=get_gfa_tdm_slotno();
	for(i=1;i<=24;i++)
	{
		idxs[2]=i;
		if(tdm_e1portTable_get(idxs,pEntry)==VOS_OK)
			{
			tdm_e1portTable_set(8, idxs , pEntry->crcenable);
			}

	}
	return VOS_OK;
}
#endif
static STATUS __tdm_e1portTable_get( ULONG *idxs, e1porttable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	e1porttable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ );
	p = (e1porttable_get_pdu*)pdu->msgData;
	p->devIdx = idxs[0];
	p->brdIdx = idxs[1];
	/*p->clusterIdx=idxs[2];
	p->e1portIdx = idxs[3];*/
	p->e1portIdx = idxs[2];
	p->tableIdx = E1PORTTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( e1porttable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, E1PORTTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( (char*)pRecv );
	}
	
	return rc;		
}

static STATUS __tdm_e1portTable_getNext( ULONG *idxs, e1porttable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	e1porttable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ );
	p = (e1porttable_get_pdu*)pdu->msgData;
	p->devIdx = idxs[0];
	p->brdIdx = idxs[1];
	/*p->clusterIdx=idxs[2];
	p->e1portIdx = idxs[3];*/
	p->e1portIdx = idxs[2];
	p->tableIdx = E1PORTTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( e1porttable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, E1PORTTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}		

	return rc;		
}

/* modified by xieshl 20080530, E1阻抗信息从EEPROM中读取，问题单#6730 */
/* added by xieshl 20080331, E1阻抗无法从GFA-SIG板获取，将来从EEPROM中读取, 在该函数中重新翻译 */
STATUS tdm_e1portTable_get( ULONG idxs[3], e1porttable_row_entry *pEntry )
{
	UCHAR tdmno;
	STATUS rc = __tdm_e1portTable_get( idxs, pEntry );
	if( rc == VOS_OK )
	{
		if( idxs[2] <= 8 )
			tdmno = 1;
		else if( idxs[2] <= 16 )
			tdmno = 2;
		else /*if( idxs[2] <= 24 )*/
			tdmno = 3;
		pEntry->impedance = get_tdm_e1_impedance( idxs[1], tdmno );		/* 该值保存在TDM板EEPROM中 */
	}
	return rc;
}

STATUS tdm_e1portTable_getNext( ULONG idxs[3], e1porttable_row_entry *pEntry )
{
	UCHAR tdmno;
	STATUS rc = __tdm_e1portTable_getNext( idxs, pEntry );
	if( rc == VOS_OK )
	{
		pEntry->brdIdx = get_gfa_sg_slotno(); 
		
		if( pEntry->e1portIdx <= 8 )
			tdmno = 1;
		else if( pEntry->e1portIdx <= 16 )
			tdmno = 2;
		else /*if( pEntry->e1portIdx <= 24 )*/
			tdmno = 3;
		pEntry->impedance = get_tdm_e1_impedance( pEntry->brdIdx, tdmno );		/* 该值保存在TDM板EEPROM中*/
	}
	return rc;
}
/* end 20080331 */

STATUS tdm_e1portTable_set( UCHAR leafIdx, ULONG idxs[3], UCHAR setval )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	e1porttable_set_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_REQ );
	p = (e1porttable_set_pdu*)pdu->msgData;
	p->devIdx = idxs[0];
	p->brdIdx = idxs[1];
	/*p->clusterIdx=idxs[2];
	p->e1portIdx = idxs[3];*/
	p->e1portIdx = idxs[2];
	p->tableIdx = E1PORTTABLE_INDEX;
	p->leafIdx = leafIdx;
	p->setval = setval;
	pdulen = headlen+sizeof( e1porttable_set_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, E1PORTTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}		
	return rc;
}

STATUS tdm_e1portTable_rowset( ULONG idxs[3], UCHAR alarmmask, UCHAR crcenable, UCHAR lpbctrl )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	e1porttable_rowset_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_REQ);
	p = (e1porttable_rowset_pdu*)pdu->msgData;
	p->devIdx = idxs[0];
	p->brdIdx = idxs[1];
/*	p->clusterIdx=idxs[2];
	p->e1portIdx = idxs[3];*/
	p->e1portIdx = idxs[2];
	p->tableIdx = E1PORTTABLE_INDEX;
	p->leafIdx = 0;
	p->alarmmask = alarmmask;
	p->crcenable = crcenable;
	p->lpbctrl = lpbctrl;
	pdulen = headlen+sizeof( e1porttable_rowset_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, E1PORTTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}
	
	return rc;
}

#if TDMONUTABLE_FUNC
#endif

static STATUS rpc_tdm_tdmOnuTable_get( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], tdmonutable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	tdmonutable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ );
	p = (tdmonutable_get_pdu*)pdu->msgData;
	p->sgIdx = idxs[0];
	p->devIdx = idxs[1];
	p->tableIdx = TDMONUTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( tdmonutable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, TDMONUTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );
		}
					
	}		

	
	return rc;	
}

STATUS tdm_tdmOnuTable_get( ULONG idxs[2], tdmonutable_row_entry *pEntry )
{
	STATUS ret = VOS_ERROR;
	ULONG onuIdx = 0;
	
	if( (onuIdx = swapToMinOnuIdx( idxs[1] ) ) > 0 )
	{
		ULONG newIdxs[2] = {0,0};
		newIdxs[0] = idxs[0];
		newIdxs[1] = onuIdx;
		ret = rpc_tdm_tdmOnuTable_get( ENUM_CALLTYPE_SYN, 0, newIdxs, pEntry );
		if( ret == VOS_OK )
			pEntry->devIdx = idxs[1];
	}
	return ret;
}

static STATUS rpc_tdm_tdmOnuTable_getNext( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], tdmonutable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	tdmonutable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ );
	p = (tdmonutable_get_pdu*)pdu->msgData;

	p->sgIdx = idxs[0];
	p->devIdx = idxs[1];
	p->tableIdx = TDMONUTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( tdmonutable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, TDMONUTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );
		}
					
	}		

	
	return rc;	
	
}

STATUS tdm_tdmOnuTable_getNext( ULONG idxs[2], tdmonutable_row_entry *pEntry )
{
	STATUS ret = VOS_ERROR;
	ULONG onuIdx = 0;

	if( idxs[1] == 0 && rpc_tdm_tdmOnuTable_getNext( ENUM_CALLTYPE_SYN, 0, idxs, pEntry ) == VOS_OK )
	{
		pEntry->devIdx = reverseSwapOnuIdx( pEntry->devIdx );
		ret = VOS_OK;
	}
	else if( idxs[1] != 0 )
	{
		onuIdx = swapToMinOnuIdx( idxs[1] );
		idxs[1] = onuIdx;
		if( rpc_tdm_tdmOnuTable_getNext( ENUM_CALLTYPE_SYN, 0, idxs, pEntry ) == VOS_OK )
		{
			pEntry->devIdx = reverseSwapOnuIdx( pEntry->devIdx );
			ret = VOS_OK;
		}
	}
	
	return ret;	
}

static STATUS rpc_tdm_tdmOnuTable_set( UCHAR calltype, ULONG callnotifier, UCHAR leafIdx, ULONG idxs[2], UCHAR setval, USHORT *lport )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	tdmonutable_set_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_REQ );
	p = (tdmonutable_set_pdu*)pdu->msgData;

	p->sgIdx = idxs[0];
	p->devIdx = idxs[1];
	p->tableIdx = TDMONUTABLE_INDEX;
	p->leafIdx = leafIdx;
	p->setval = setval;
	pdulen = headlen+sizeof( tdmonutable_set_pdu );

	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, TDMONUTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, lport) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );
		}
					
	}		
	return rc;	
}

STATUS tdm_tdmOnuTable_set( UCHAR leafIdx, ULONG idxs[2], UCHAR setval, USHORT *lport )
{
	STATUS ret = VOS_ERROR;

	USHORT logicPort = 0;
	if( (idxs[1] = swapToMinOnuIdx( idxs[1] )) > 0 && rpc_tdm_tdmOnuTable_set( ENUM_CALLTYPE_SYN, 0, 
		leafIdx, idxs, setval , &logicPort ) == VOS_OK )
	{
		*lport = logicPort;
		ret = VOS_OK;
	}
	
	return ret;
}

static STATUS rpc_tdm_tdmOnuTable_rowset( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], USHORT logicalOnuIdx, UCHAR onuRowStatus )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	tdmonutable_rowset_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_REQ);
	
	p = (tdmonutable_rowset_pdu*)pdu->msgData;

	p->sgIdx = idxs[0];
	p->devIdx = idxs[1];
	p->tableIdx = TDMONUTABLE_INDEX;
	p->leafIdx = 0;
	p->logiconuIdx = logicalOnuIdx;

	p->rowstatus = onuRowStatus;
	pdulen = headlen+sizeof( tdmonutable_rowset_pdu );

	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, TDMONUTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );
		}
					
	}
	
	return rc;	
}

STATUS tdm_tdmOnuTable_rowset( ULONG idxs[2], USHORT logicalOnuIdx, UCHAR onuRowStatus )
{
	STATUS ret = VOS_ERROR;

	if( (idxs[1] = swapToMinOnuIdx( idxs[1] ) )!= -1 && rpc_tdm_tdmOnuTable_rowset( ENUM_CALLTYPE_SYN, 0,
		idxs, logicalOnuIdx, onuRowStatus ) == VOS_OK )
		ret = VOS_OK;

	return ret;
}

#if POTSLINKTABLE_FUNC
#endif

static STATUS rpc_tdm_potsLinkTable_get( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], potslinktable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	potslinktable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ );
	p = (potslinktable_get_pdu*)pdu->msgData;
	p->linksgIdx = idxs[0];
	p->linksgportIdx = idxs[1];
	p->tableIdx = POTSLINKTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( potslinktable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, POTSLINKTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );
		}
					
	}		

	return rc;	
	
}

#if 0
STATUS tdm_extPotsLinkTable_get( ULONG idxs[2], potslinktable_row_entry *pEntry )
{	
	int  ulboard=0,ulpots=0;
	STATUS ret=VOS_ERROR;
	
	if( rpc_tdm_potsLinkTable_get( ENUM_CALLTYPE_SYN, 0, idxs, pEntry ) == VOS_OK )
	{
		pEntry->devIdx = reverseSwapOnuIdx( pEntry->devIdx );
		VoiceBoardNumGet( pEntry->devIdx, &ulboard);
		ulpots=(int)pEntry->potsIdx;
		if(ulpots>8)
			ulpots%=8;
		pEntry->brdIdx=(UCHAR)ulboard;
		pEntry->potsIdx=(UCHAR)ulpots;
		ret = VOS_OK;
	}
	
}
#endif
STATUS tdm_potsLinkTable_get( ULONG idxs[2], potslinktable_row_entry *pEntry )
{
	STATUS ret = VOS_ERROR;

	if( rpc_tdm_potsLinkTable_get( ENUM_CALLTYPE_SYN, 0, idxs, pEntry ) == VOS_OK )
	{
		pEntry->devIdx = reverseSwapOnuIdx( pEntry->devIdx );
		ret = VOS_OK;
	}
	return ret;
}

static STATUS rpc_tdm_potsLinkTable_getNext( UCHAR calltype, ULONG callnotifier, ULONG idxs[2], potslinktable_row_entry *pEntry )
{
	
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	potslinktable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ );
	p = (potslinktable_get_pdu*)pdu->msgData;
	p->linksgIdx = idxs[0];
	p->linksgportIdx = idxs[1];
	p->tableIdx = POTSLINKTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( potslinktable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, POTSLINKTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );
		}
					
	}		
	
	return rc;	
	
}

STATUS tdm_potsLinkTable_getNext( ULONG idxs[2], potslinktable_row_entry *pEntry )
{
	STATUS ret = VOS_ERROR;
	
	if( rpc_tdm_potsLinkTable_getNext( ENUM_CALLTYPE_SYN, 0, idxs, pEntry ) == VOS_OK )
	{
		pEntry->devIdx =  reverseSwapOnuIdx( pEntry->devIdx );
		ret = VOS_OK;
	}

	return ret;
}

STATUS tdm_potsLinkTable_set( UCHAR leafIdx, ULONG idxs[2], UCHAR setval[32] )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	potslinktable_set_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_REQ );
	p = (potslinktable_set_pdu*)pdu->msgData;
	p->linksgIdx = idxs[0];
	p->linksgportIdx = idxs[1];
	p->tableIdx =POTSLINKTABLE_INDEX;
	p->leafIdx = leafIdx;

	VOS_MemCpy( p->setval, setval, 32 );

	if( leafIdx != 7 )
		pdulen = headlen+sizeof( potslinktable_set_pdu )-28;
	else
		pdulen = headlen+sizeof( potslinktable_set_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, POTSLINKTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}		
	return rc;	
}

STATUS tdm_potsLinkTable_rowset( ULONG idxs[2], ULONG onuIdx, ULONG onuPotsBrd, 
			ULONG onuPotsIdx, ULONG phonecode, UCHAR linkdesc[32], UCHAR potsLinkStatus )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	potslinktable_rowset_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_REQ);
	
	p = (potslinktable_rowset_pdu*)pdu->msgData;
	p->linksgIdx = idxs[0];
	p->linksgportIdx = idxs[1];
	p->tableIdx = POTSLINKTABLE_INDEX;
	p->leafIdx = 0;
	p->linkphonecode = phonecode;
	VOS_MemCpy( p->linkdesc, linkdesc, 32 );
	p->onuIdx = onuIdx;
	p->onuPotsBrd = onuPotsBrd;
	p->onuPotsIdx = onuPotsIdx;
	p->potsLinkRowStatus = potsLinkStatus;
	

	pdulen = headlen+sizeof( potslinktable_rowset_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, POTSLINKTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}
	
	return rc;		
}

#if POTSPORTTABLE_FUNC /*! lint553*/
#endif

static STATUS rpc_tdm_potsPortTable_get( UCHAR calltype, ULONG callnotifier, ULONG idxs[3], potsporttable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	potsporttable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ );
	p = (potsporttable_get_pdu*)pdu->msgData;
	p->devIdx = idxs[0];
	p->brdIdx = idxs[1];
	p->potsIdx = idxs[2];
	p->tableIdx = POTSPORTTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( potsporttable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, POTSPORTTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );
		}
					
	}		

	return rc;		
}

STATUS   VoiceBoardNumGet(ulong onuDevIdx,int *BoardNum,int BoardList[4])
{
	short int slot=0,port=0,onuid=0,PonPortIdx=0;
	int OnuType=0,i,j=0;
	int MaxSlotNum=1;

	slot =GET_PONSLOT(onuDevIdx)/*onuDevIdx/10000*/;
	port=GET_PONPORT(onuDevIdx)/*(onuDevIdx%10000)/1000*/;
	onuid=GET_ONUID(onuDevIdx)/*onuDevIdx%1000*/;

	/*sys_console_printf ("onudev=%d,slot=%d,port=%d,onuid=%d\r\n",onuDevIdx,slot,port,onuid);*/

	PonPortIdx=GetPonPortIdxBySlot(slot, port);
	if( PonPortIdx == VOS_ERROR )	/* modified by xieshl 20080811, 发现snmp异常，原因不详，这里增加判断 */
	{
		/* 20080812, 基本确认没有判断返回值造成 */
		/*sys_console_printf(" ** VoiceBoardNumGet: GetPonPortIdxBySlot error\r\n");*/
		return PonPortIdx;
	}
	if( GetOnuType( PonPortIdx,onuid-1, &OnuType) == VOS_ERROR )
	{
		/*sys_console_printf(" ** VoiceBoardNumGet: GetOnuType error\r\n");*/
		return VOS_ERROR;
	}

	/* modified by chenfj 2009-5-20*/
	MaxSlotNum = GetOnuSubSlotNum(OnuType);
	if(OnuType!=V2R1_ONU_GT861)
		return  VOS_ERROR;
	else
	{
		for(i=2;i<=MaxSlotNum;i++)
		{
			if( (ExtBrdMgmtTable[PonPortIdx][onuid].BrdMgmtTable[i].brdType==OAM_ONU_SLOT_GT_8FXS_A) ||
				(ExtBrdMgmtTable[PonPortIdx][onuid].BrdMgmtTable[i].brdType==OAM_ONU_SLOT_GT_8FXS_B))
			{
				BoardList[j]=i;
				j++;
			}
		}	
		*BoardNum=j;
	}
	
	return VOS_OK;
}


STATUS tdm_ExtpotsPortTable_get( ULONG idxs[3], potsporttable_row_entry *pEntry )
{
	STATUS ret=VOS_ERROR;
	
	ULONG  getIdxs[3]={0,0,0};
	int  ulboardNum=0;
	int ulpots=0;
	
	getIdxs[1]=idxs[1];
	getIdxs[2]=idxs[2];
	getIdxs[0]=swapToMinOnuIdx( idxs[0] );

	ulboardNum=idxs[2]%8;
	if(ulboardNum==0)
		ulboardNum=((idxs[2]/8)+1);
	else
		ulboardNum=((idxs[2]/8)+2);
	
	/*sys_console_printf ("1: in ext get: idx[0]=%d,idx[1]=%d,idx[2]=%d\r\n",getIdxs[0],getIdxs[1],getIdxs[2]);*/

	if( (getIdxs[0] != -1 )&& (rpc_tdm_potsPortTable_get( ENUM_CALLTYPE_SYN, 0, getIdxs, pEntry ) == VOS_OK) )
	{	
		
		pEntry->devIdx = idxs[0];
		pEntry->brdIdx= (UCHAR)ulboardNum;
		ulpots=(idxs[2]%8);
		if(ulpots==0)
			ulpots=8;
		pEntry->potsIdx= (UCHAR)ulpots;
		/*sys_console_printf("2:dev=%d,brd=%d,pots=%d\r\n",pEntry->devIdx,pEntry->brdIdx,pEntry->potsIdx);*/
		ret = VOS_OK;
	}

	return ret;
}

STATUS tdm_potsPortTable_get( ULONG idxs[3], potsporttable_row_entry *pEntry )
{
	STATUS ret = VOS_ERROR;

	ULONG newIdxs[3] = { 0,0,0 };
	newIdxs[1] = idxs[1];
	newIdxs[2] = idxs[2];
	newIdxs[0] = swapToMinOnuIdx( idxs[0] );

	if(idxs[1]!=1)
		{
			newIdxs[2]=(idxs[1]-2)*8+idxs[2];
		}

	if( (newIdxs[0] != -1 )&& (rpc_tdm_potsPortTable_get( ENUM_CALLTYPE_SYN, 0, newIdxs, pEntry ) == VOS_OK) )
	{
		pEntry->devIdx = idxs[0];
		pEntry->brdIdx=(UCHAR)idxs[1];
		pEntry->potsIdx=(UCHAR)idxs[2];
		ret = VOS_OK;
	}

	return ret;
}

static STATUS rpc_tdm_potsPortTable_getNext( UCHAR calltype, ULONG callnotifier, ULONG idxs[3], potsporttable_row_entry *pEntry )
{
	
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	potsporttable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ );
	p = (potsporttable_get_pdu*)pdu->msgData;
	p->devIdx= idxs[0];
	p->brdIdx = idxs[1];
	p->potsIdx = idxs[2];
	p->tableIdx = POTSPORTTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( potsporttable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, POTSPORTTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );
		}
					
	}		
	
	return rc;	
	
}

STATUS tdm_potsPortTable_getNext( ULONG idxs[3], potsporttable_row_entry *pEntry )
{
	STATUS ret = VOS_ERROR;

	if( (idxs[0] == 0) && (rpc_tdm_potsPortTable_getNext( ENUM_CALLTYPE_SYN, 0, idxs, pEntry ) == VOS_OK) )
	{
		pEntry->devIdx = reverseSwapOnuIdx( pEntry->devIdx );
		ret = VOS_OK;
	}
	else if( idxs[0] != 0 )
	{
		idxs[0] = swapToMinOnuIdx( idxs[0] );

		if( rpc_tdm_potsPortTable_getNext( ENUM_CALLTYPE_SYN, 0, idxs, pEntry ) == VOS_OK )
		{
			pEntry->devIdx = reverseSwapOnuIdx( pEntry->devIdx );
			ret = VOS_OK;
		}
	}	

	return ret;
}


#if HDLCSTATTABLE_FUNC
#endif

STATUS tdm_hdlcStatTable_get( ULONG idxs, hdlcstattable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	hdlcstattable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );
	PDU_CHECK( pdu );

	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ );
	p = (hdlcstattable_get_pdu*)pdu->msgData;
	p->sgifIdx = idxs;

	p->tableIdx = SGHDLCSTATTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( hdlcstattable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, SGHDLCSTATTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}		

	return rc;		
}


STATUS tdm_hdlcStatTable_getNext( ULONG idxs, hdlcstattable_row_entry *pEntry )
{
	
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	hdlcstattable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ );
	p = (hdlcstattable_get_pdu*)pdu->msgData;
	p->sgifIdx = idxs;
	p->tableIdx = SGHDLCSTATTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( hdlcstattable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, SGHDLCSTATTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}		
	
	return rc;	
	
}


#if SGTTABLE_FUNC
#endif
STATUS tdm_sgTable_get( ULONG idxs, sgtable_row_entry *pEntry )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	sgtable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ );
	p = (sgtable_get_pdu*)pdu->msgData;
	p->sgifIdx= idxs;

	p->tableIdx = SGTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( sgtable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_RES, SGTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}		

	return rc;		
	
}

STATUS tdm_sgTable_getNext( ULONG idxs, sgtable_row_entry *pEntry )
{
	
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	sgtable_get_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ );
	p = (sgtable_get_pdu*)pdu->msgData;
	p->sgifIdx = idxs;
	p->tableIdx = SGTABLE_INDEX;
	p->leafIdx = 0;
	pdulen = headlen+sizeof( sgtable_get_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_RES, SGTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry ) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}		
	
	return rc;	
		
}

STATUS tdm_sgTable_set( UCHAR leafIdx, ULONG idxs, USHORT setval )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	sgtable_set_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_REQ );
	p = (sgtable_set_pdu*)pdu->msgData;
	p->sgifIdx = idxs;
	p->tableIdx = SGTABLE_INDEX;
	p->leafIdx = leafIdx;
	p->setval = setval;
	pdulen = headlen+sizeof( sgtable_set_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_RES, SGTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}		
	return rc;			
}

STATUS tdm_sgTable_rowset( ULONG idxs, USHORT rowset[6] )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	char *pRecv = NULL;
	sgtable_rowset_pdu * p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu,  MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_REQ);
	
	p = (sgtable_rowset_pdu*)pdu->msgData;
	p->sgifIdx = idxs;

	p->mastere1 = rowset[0];
	p->slavee1= rowset[1];
	p->e1clk = rowset[2];
	p->vlanen = rowset[3];
	p->vlanid = rowset[4];
	p->vlanpri = rowset[5];

	p->tableIdx = SGTABLE_INDEX;
	p->leafIdx = 0;


	pdulen = headlen+sizeof( sgtable_rowset_pdu );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{
		rc = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_SETROW_RES, SGTABLE_INDEX ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
			rc = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}
	
	return rc;	
}

#if 0

STATUS tdmConfGet( UCHAR calltype, ULONG callnotifier, UCHAR tableIndex, UCHAR leafIndex, ULONG *idxs, UCHAR idxCounter, 
						UCHAR* varNum, UCHAR ** pValField, UCHAR * const valLen )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	tdm_pdu_t *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	char vars[10][4] = { 0 };
	UCHAR varlen[10]={0};
	int i=0;

	vars[0][0] = tableIndex;
	vars[0][1] = 0;
	varlen[0]=1;
	varlen[1]=1;
	for( i=0; i<idxCounter; i++ )
	{
		int len = getIndexLen( tableIndex, i+1 );
		switch( len )
		{
			case 1:
				vars[i+2][0]=idxs[i];
				break;
			case 2:
				*(short int*)vars[i+2]=idxs[i];
				break;
			case 4:
				*(ULONG*)vars[i+2]=idxs[i];
				break;
		}
		varlen[i+2] = len;
	}

	
	pdulen = buildTdmPdu( pdu, ssid, seno, MSG_TYPE_CONFIG, MSG_SUBTYPE_GET_REQ, idxCounter+2, vars, varlen );


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &(char*)pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_GET_RES))( (char*)pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, varNum,  pValField, valLen) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( (char*)pRecv );
		}
				
	}
	
	return rc;	
}

STATUS tdmConfGetNext( UCHAR calltype, ULONG callnotifier, UCHAR tableIndex, UCHAR leafIndex, ULONG *idxs, UCHAR idxCounter, 
								UCHAR *varNum, UCHAR * const pValField, UCHAR * const valLen )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	tdm_pdu_t *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	char vars[10][4] = { 0 };
	UCHAR varlen[10]={0};
	int i=0;

	vars[0][0] = tableIndex;
	vars[0][1] = 0;
	varlen[0]=1;
	varlen[1]=1;
	for( i=0; i<idxCounter; i++ )
	{
		int len = getIndexLen( tableIndex, i+1 );
		switch( len )
		{
			case 1:
				vars[i+2][0]=idxs[i];
				break;
			case 2:
				*(short int*)vars[i+2]=idxs[i];
				break;
			case 4:
				*(ULONG*)vars[i+2]=idxs[i];
				break;
		}
		varlen[i+2] = len;
	}

	
	pdulen = buildTdmPdu( pdu, ssid, seno, MSG_TYPE_CONFIG, MSG_SUBTYPE_GETNEXT_REQ, idxCounter+2, vars, varlen );


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_GETNEXT_RES))( (char*)pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, varNum,  pValField, valLen) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( (char*)pRecv );
		}
				
	}
	
	return rc;		
}

STATUS tdmConfSet( UCHAR calltype, ULONG callnotifier, UCHAR tableIndex, UCHAR leafIndex, ULONG *idxs, UCHAR idxCounter,
						UCHAR * const pValField, const UCHAR valLen )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	tdm_pdu_t *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	char vars[10][4] = { 0 };
	UCHAR varlen[10]={0};
	int i=0;

	vars[0][0] = tableIndex;
	vars[0][1] = leafIndex;
	varlen[0]=1;
	varlen[1]=1;
	for( i=0; i<idxCounter; i++ )
	{
		int len = getIndexLen( tableIndex, i+1 );
		switch( len )
		{
			case 1:
				vars[i+2][0]=idxs[i];
				break;
			case 2:
				*(short int*)vars[i+2]=idxs[i];
				break;
			case 4:
				*(ULONG*)vars[i+2]=idxs[i];
				break;
		}
		varlen[i+2] = len;
	}

	VOS_MemCpy( vars[2+idxCounter], pValField, valLen );
	varlen[2+idxCounter] = valLen;

	pdulen = buildTdmPdu( pdu, ssid, seno, MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_REQ, idxCounter+3, vars, varlen );


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_SET_RES))( (char*)pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL,  NULL, NULL) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( (char*)pRecv );
		}
				
	}
	
	return rc;		
}

STATUS tdmConfRowSet( UCHAR calltype, ULONG callnotifier, UCHAR tableIndex, UCHAR leafIndex, ULONG *idxs, UCHAR idxCounter, 
								const UCHAR varNum, UCHAR ** const pValField, const UCHAR *valLen )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0, headlen=0;
	tdm_pdu_t *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	char vars[10][80] = { 0 };
	UCHAR varlen[10]={0};
	int i=0;

	vars[0][0] = tableIndex;
	vars[0][1] = leafIndex;
	varlen[0]=1;
	varlen[1]=1;
	for( i=0; i<idxCounter; i++ )
	{
		int len = getIndexLen( tableIndex, i+1 );
		switch( len )
		{
			case 1:
				vars[i+2][0]=idxs[i];
				break;
			case 2:
				*(short int*)vars[i+2]=idxs[i];
				break;
			case 4:
				*(ULONG*)vars[i+2]=idxs[i];
				break
		}
		varlen[i+2] = len;
	}

	for( i=0; i<varNum; i++ )
	{
		VOS_MemCpy( vars[2+idxCounter], pValField[i], valLen[i] );
		varlen[2+idxCounter]=valLen[i];
	}

	
	pdulen = buildTdmPdu( pdu, ssid, seno, MSG_TYPE_CONFIG, MSG_SUBTYPE_SET_REQ, idxCounter+varNum+2, vars, varlen );


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_DEVMANAGE, MSG_SUBTYPE_SET_RES))( (char*)pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL,  NULL, NULL) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( (char*)pRecv );
		}
				
	}
	
	return rc;		
}

#endif

STATUS tdmDebugQuery ( UCHAR calltype, ULONG callnotifier, ULONG *debug )
{
	
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0/*, headlen=0*/;
	char *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );
	
	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu,  MSG_TYPE_TRACEDEBUG, MSG_SUBTYPE_DEBUG_REQ );


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN )
		{
			if( VOS_OK != (*getRecvHandler( MSG_TYPE_TRACEDEBUG, MSG_SUBTYPE_DEBUG_REQ, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, debug ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );			
		}
				
	}	

	return rc;
}

STATUS tdmDebugSet ( UCHAR calltype, ULONG callnotifier, ULONG vty, ULONG debug )
{
	STATUS rc = VOS_ERROR;
	USHORT   pdulen=0, recvlen=0/*, headlen=0*/;
	char *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	char *pData = pdu->msgData;
	
	PDU_CHECK( pdu );
	
	pdulen = buildPduHead( pdu,  MSG_TYPE_TRACEDEBUG, MSG_SUBTYPE_DEBUG_SET_REQ );

	*(ULONG*)pData = vty;
	*(ULONG*)(pData+4) = debug;

	pdulen += 8;


	if( VOS_OK == tdmCommSendMsg( calltype, callnotifier, (char*)pdu, pdulen, &pRecv, &recvlen ) )
	{

		rc = VOS_OK;
		
		if( calltype == ENUM_CALLTYPE_SYN )
		{

			if( VOS_OK != (*getRecvHandler( MSG_TYPE_TRACEDEBUG, MSG_SUBTYPE_DEBUG_SET_RES, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL  ) )
				rc = VOS_ERROR;
			
			tdmCommMsgFree( pRecv );			
		}
				
	}		

	return rc;
}

/*begin:
added by wangxiaoyu 2008-01-10
对TDM主动上传的事件进行处理
end*/
void  tdmEventMsgProg( unsigned char *pBuf, unsigned int length)
{
	tdm_comm_msg_t *pMsg = (tdm_comm_msg_t*)pBuf;
	tdm_pdu_t *ppdu = &pMsg->pdu;

	switch((int)pMsg->pdu.subType)
	{
		case MSG_SUBTYPE_E1_ALM:
			{
				int (*prog[8])(ULONG,ULONG,ULONG)={
					e1LosAlarm_EventReport,
					e1LofAlarm_EventReport,
					e1AisAlarm_EventReport,
					e1RaiAlarm_EventReport,
					e1SmfAlarm_EventReport,
					e1LomfAlarm_EventReport,
					e1Crc3Alarm_EventReport,
					e1Crc6Alarm_EventReport
				};
			
				UCHAR sg = ppdu->msgData[0];
				UCHAR e1 = ppdu->msgData[1];
				int type = ppdu->msgData[2];
				int i=0;

				/* begin: added by jianght 20090319  */
				#if 0
				if ( 0 != get_gfa_e1_slotno() )
				{
					if ( VOS_OK != tdm_e1_alarm_set(sg - 1, e1 - 1, (USHORT)type) )
					{
						sys_console_printf("tdmEventMsgProg()::tdm_e1_alarm_set()  error! \r\n");
					}
					/*sys_console_printf("alarm  sg=%d  e1=%d   alarm type=0x%02x \r\n", sg, e1, type);*/
				}
				#endif

				for(;i<8;i++)
				{
					if(0x80>>i == type )
					{
						(*prog[i])(OLT_DEV_ID, get_gfa_e1_slotno(),(sg - 1) * 8 + e1);
						#if 0
						if ( 0 != get_gfa_e1_slotno() )
						{
							/* E1 */
							(*prog[i])(OLT_DEV_ID, get_gfa_e1_slotno(),(sg - 1) * 8 + e1);
						} 
						else
						{
							/* 语音 */
							(*prog[i])(OLT_DEV_ID, get_gfa_sg_slotno(),(sg-1)*8+e1);
						}
						#endif
						break;
					}
				}
				/* end: added by jianght 20090319 */
			}
			break;
		case MSG_SUBTYPE_E1_ALMCLR:
			{
				int (*prog[8])(ULONG,ULONG,ULONG)={
					e1LosAlarmClear_EventReport,
					e1LofAlarmClear_EventReport,
					e1AisAlarmClear_EventReport,
					e1RaiAlarmClear_EventReport,
					e1SmfAlarmClear_EventReport,
					e1LomfAlarmClear_EventReport,
					e1Crc3AlarmClear_EventReport,
					e1Crc6AlarmClear_EventReport
				};
			
				UCHAR sg = ppdu->msgData[0];
				UCHAR e1 = ppdu->msgData[1];
				int type = ppdu->msgData[2];
				int i=0;

				/* begin: added by jianght 20090319  */
				#if 0
				if ( 0 != get_gfa_e1_slotno() )
				{
					if ( VOS_OK != tdm_e1_alarm_clear(sg - 1, e1 - 1, (USHORT)type) )
					{
						sys_console_printf("tdmEventMsgProg()::tdm_e1_alarm_clear()  error! \r\n");
					}
				}
				#endif

				for(;i<8;i++)
				{
					if(0x80>>i == type )
					{
						(*prog[i])(OLT_DEV_ID, get_gfa_e1_slotno(),(sg - 1) * 8 + e1);
						#if 0
						if ( 0 != get_gfa_e1_slotno() )
						{
							/* E1 */
							(*prog[i])(OLT_DEV_ID, get_gfa_e1_slotno(),(sg - 1) * 8 + e1);
						} 
						else
						{
							/* 语音 */
							(*prog[i])(OLT_DEV_ID, get_gfa_sg_slotno(),(sg-1)*8+e1);
						}
						#endif
						break;
					}
				}
				/* end: added by jianght 20090319 */
			}
			break;
		case MSG_SUBTYPE_VOICE_IS:
			{
				ULONG devIdx = *(ULONG*)ppdu->msgData;
				tdmServiceAbortAlarmClear_EventReport( devIdx );
			}
			break;
		case MSG_SUBTYPE_VOICE_OOS:
			{
				ULONG devIdx = *(ULONG*)ppdu->msgData;
				tdmServiceAbortAlarm_EventReport( devIdx );
			}
			break;
		case MSG_SUBTYPE_ALM_SIG_RESET:	/* added by xieshl 20080324 */
			{
				if( ppdu->msgCode == 0 )
					tdmResetAlarm_EventReport();
			}
			break;
		/* begin: added by jianght 20090205  */
		case E1_OOS:
			{
				UCHAR sg = ppdu->msgData[0];
				UCHAR e1 = ppdu->msgData[1];

				/*sys_console_printf("OOS Alarm  sg=%d  e1=%d   type=0x%02x \r\n", sg, e1, type);*/

				/*if ( VOS_OK != tdm_e1_alarm_set(sg-1, e1-1, 0x0100) )
				{
					E1_ERROR_INFO_PRINT(("tdmEventMsgProg()::tdm_e1_alarm_set()  error! \r\n"));
				}*/
				E1OutOfService_EventReport(1, get_gfa_e1_slotno(), (sg - 1) * MAX_E1_PER_FPGA + e1);
			}
			break;
		case E1_OOS_RECOV:
			{
				UCHAR sg = ppdu->msgData[0];
				UCHAR e1 = ppdu->msgData[1];

				/*sys_console_printf("OOS Clear  sg=%d  e1=%d   type=0x%02x \r\n", sg, e1, type);*/
				
				/*if ( VOS_OK != tdm_e1_alarm_clear(sg-1, e1-1, 0x0100) )
				{
					E1_ERROR_INFO_PRINT(("tdmEventMsgProg()::tdm_e1_alarm_clear()  error! \r\n"));
				}*/
				
				E1OutOfServiceClear_EventReport(1, get_gfa_e1_slotno(), (sg - 1) * MAX_E1_PER_FPGA + e1);
			}
			break;
		/* end: added by jianght 20090205 */
		default:
			break;
	}

	/* begin: added by jianght 20090629 */
	/* 需要释放内存 */
	VOS_Free( (void *)pBuf );
	/* end: added by jianght 20090629 */
}

STATUS tdm_E1AlarmLevelSet( UCHAR lvl[8] )
{
	STATUS ret = VOS_ERROR;

	USHORT   recvlen=0, headlen=0;
	char *pRecv = NULL;

	tdm_e1_alm_lvl_set_pdu *p = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu, MSG_TYPE_CONFIG, MSG_SUBTYPE_E1_LVL_SET);
	
	p = (tdm_e1_alm_lvl_set_pdu*)pdu->msgData;
	p->reserved1 = 0;
	p->reserved2 = 0;
	memcpy( p->lvls, lvl, 8 );

	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, headlen+sizeof(tdm_e1_alm_lvl_set_pdu), &pRecv, &recvlen ) )
	{
		ret = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_E1_LVL_SET, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, NULL) )
			ret = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}
	
	return ret;
}

STATUS tdm_E1AlarmLevelGet( tdm_e1_alm_lvl_entry *pEntry )
{
	STATUS ret = VOS_ERROR;

	USHORT   recvlen=0, headlen=0;
	char *pRecv = NULL;
	
	tdm_pdu_t* pdu = (tdm_pdu_t*)tdmCommMsgAlloc( );

	PDU_CHECK( pdu );
	
	headlen = buildPduHead( pdu, MSG_TYPE_CONFIG, MSG_SUBTYPE_E1_LVL_GET);


	if( VOS_OK == tdmCommSendMsg( ENUM_CALLTYPE_SYN, 0, (char*)pdu, headlen, &pRecv, &recvlen ) )
	{
		ret = VOS_OK;
			
		if( VOS_OK != (*getRecvHandler( MSG_TYPE_CONFIG, MSG_SUBTYPE_E1_LVL_GET, 0 ))( pRecv+PDU_OFFSET_IN_MSG, recvlen-PDU_OFFSET_IN_MSG, pEntry) )
			ret = VOS_ERROR;
			
		tdmCommMsgFree( pRecv );
	}
		

	return ret;
}

#if 0
void test_tdm_func( int func )
{
	char softver[9]="", fpgaver[5]="";
	USHORT sfverlen=0, fgverlen=0;
	UCHAR status = 0;
	ULONG debug = 0, vty = 0;

	switch( func )
	{
		case 1:		
			tdmReadVersion( ENUM_CALLTYPE_SYN, 0, softver, &sfverlen, fpgaver, &fgverlen );
			break;
		case 2:
			tdmLoadFpga( ENUM_CALLTYPE_SYN, 0, 1,0,1 );
			break;
		case 3:
			tdmRunNotify( ENUM_CALLTYPE_NOACK, 0 );
			break;
		case 4:
			tdmStatusQuery( ENUM_CALLTYPE_SYN, 0, &status );
			break;
		case 5:
			tdmReset( ENUM_CALLTYPE_NOACK, 0, 0 );
			break;
		case 6:
			tdmDebugQuery( ENUM_CALLTYPE_SYN, 0, &debug );
			break;
		case 7:
			tdmDebugSet( ENUM_CALLTYPE_SYN, 0, vty, debug );
			break;				
	}
}

STATUS test_tdm_conf_get_func( ULONG table,  ULONG a, ULONG b, ULONG c, ULONG d, ULONG flag, char *sz, ULONG *len )
{
	e1porttable_row_entry entry;
	tdmonutable_row_entry tdmOnuEntry;
	potslinktable_row_entry potslinkEntry;
	potsporttable_row_entry potsPortEntry;
	hdlcstattable_row_entry hdlcStatEntry;
	sgtable_row_entry sgEntry;

	STATUS rc = VOS_ERROR;
	
	ULONG idxs[4]={0,0,0,0};

	idxs[0]=a;
	idxs[1]=b;
	idxs[2]=c;
	idxs[3]=d;

	switch ( table )
	{
		case E1PORTTABLE_INDEX:
			if( flag == 1 )
				rc = tdm_e1portTable_get( idxs, &entry );
			else
				rc = tdm_e1portTable_getNext( idxs, &entry );
			*len = sizeof(entry);
			memcpy( sz, &entry,  *len );
		break;

		case TDMONUTABLE_INDEX:
			if( flag == 1 )
				rc = tdm_tdmOnuTable_get( idxs, &tdmOnuEntry );
			else
				rc = tdm_tdmOnuTable_getNext( idxs, &tdmOnuEntry );
			*len = sizeof(tdmOnuEntry);
			memcpy( sz, &tdmOnuEntry,  *len );
		break;

		case POTSLINKTABLE_INDEX:
			if( flag == 1 )
				rc = tdm_potsLinkTable_get( idxs, &potslinkEntry );
			else
				rc = tdm_potsLinkTable_getNext( idxs, &potslinkEntry );
			*len = sizeof(potslinkEntry);
			memcpy( sz, &potslinkEntry,  *len );
		break;

		case POTSPORTTABLE_INDEX:
			if( flag == 1 )
				rc = tdm_potsPortTable_get( idxs, &potsPortEntry );
			else
				rc = tdm_potsPortTable_getNext( idxs, &potsPortEntry );
			*len = sizeof(potsPortEntry);
			memcpy( sz, &potsPortEntry,  *len );
			break;

		case SGHDLCSTATTABLE_INDEX:
			if( flag == 1 )
				rc = tdm_hdlcStatTable_get( idxs[0], &hdlcStatEntry );
			else
				rc = tdm_hdlcStatTable_getNext( idxs[0], &hdlcStatEntry );
			*len = sizeof(hdlcStatEntry);
			memcpy( sz, &hdlcStatEntry,  *len );
			break;

		case SGTABLE_INDEX:
			if( flag == 1 )
				rc = tdm_sgTable_get( idxs[0], &sgEntry );
			else
				rc = tdm_sgTable_getNext( idxs[0], &sgEntry );
			*len = sizeof(sgEntry);
			memcpy( sz, &sgEntry,  *len );
			break;
	}
	
	return rc;
	
}

STATUS test_tdm_conf_set_func( ULONG table, ULONG leaf, ULONG a, ULONG b, ULONG c, ULONG d, CHAR* buf )
{
	STATUS rc = VOS_ERROR;
	
	ULONG idxs[4]={0,0,0,0};

	USHORT lport = 0;

	idxs[0]=a;
	idxs[1]=b;
	idxs[2]=c;
	idxs[3]=d;
	
	switch ( table )
	{
		case E1PORTTABLE_INDEX:
				rc = tdm_e1portTable_set( leaf, idxs, buf[0] );
		break;

		case TDMONUTABLE_INDEX:
				rc = tdm_tdmOnuTable_set( leaf, idxs, buf[0], &lport );
		break;

		case POTSLINKTABLE_INDEX:
				rc = tdm_potsLinkTable_set( leaf, idxs, buf );
		break;

		case SGTABLE_INDEX:
				rc = tdm_sgTable_set( leaf, idxs[0], buf[0] );
			break;
	}	

	return rc;
}

void testCommChannel( const int num )
{
	e1porttable_row_entry entry;
	ULONG idxs[3]={0,0,0};
	int i = 0, tok=0, terr=0;

	reportd( "begin ticks", VOS_GetTick() );
	for( ;i<num; i++ )
	{
		if( tdm_e1portTable_getNext( idxs, &entry ) == VOS_OK )
			tok++;
		else
			terr++;
	}

	reportd( "end ticks", VOS_GetTick() );

	reportd( "tok:", tok );
	reportd( "terr:", terr );
}
extern int ethSendToMII(void *sendBuffP , unsigned int packetSize);
void testCommChannel_1( const int num, const int ticks )
{

	int i = 0;
	char macsrc[6] = {0x00,0x05,0x3b,0xff,0x01,0x03};
	char macdes[6] = {0x00,0x01,0x0b,0x0f,0x01,0x07};
	static char szSend[80] = "";
	memcpy( szSend, macdes, 6 );
	memcpy( szSend+6, macsrc, 6 );

	szSend[12] = 0x08;
	szSend[13] = 0x00;

	for( ; i<num; i++ )
	{		
		*(unsigned long *)(szSend+14) = i;

		if( ethSendToMII( szSend, 60 ) == VOS_OK )
			frnum = i+1;
		VOS_TaskDelay( ticks );
	}
	
}
#endif

#endif	/* EPON_MODULE_TDM_SERVICE */

#ifdef __cplusplus
}
#endif

