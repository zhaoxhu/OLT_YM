
#ifndef TDM_APIS_H
#define TDM_APIS_H

#ifndef	PACKED
#define	PACKED	__attribute__((packed))
#endif


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

/*定义处理函数的指针类型*/
typedef	STATUS (*FUNC_PRC)( const char*, USHORT, void* );

enum{
	MSG_TYPE_DEVMANAGE = 0x10,
	MSG_TYPE_CONFIG = 0x20,
	MSG_TYPE_EVENT = 0x30,
	MSG_TYPE_TRANSFILE=0x40,
	MSG_TYPE_TRACEDEBUG = 0x50
};



enum{
	E1PORTTABLE_INDEX = 1,
	TDMONUTABLE_INDEX,
	POTSLINKTABLE_INDEX,
	POTSPORTTABLE_INDEX,
	SGTABLE_INDEX,
	SGHDLCSTATTABLE_INDEX	
};

/*语音管理API返回值定义*/
enum{
	TDM_VM_ERR = -1,
	TDM_VM_OK,
	TDM_VM_VID_EXIT,
	TDM_VM_VNAME_EXIT,
	TDM_VM_DEL_NEXIT,
	TDM_VM_ANOTHER_VID_EXIT
};

enum{
	TDM_VM_NOT_ONLINE = 1,
	TDM_VM_IN_ANOTHER_SG,
	TDM_VM_IN_THIS_SG,
	TDM_VM_NOT_IN_ANY_SG,
	TDM_VM_NOT_EXIST,  /* onu not exist */
	TDM_VM_NOT_SUPPORT_VOICE = 6,
	TDM_VM_EXIST_ANOTHER_LINK_POT,
	TDM_VM_EXIST_LINK_POT,
	TDM_VM_NOT_EXIST_LINK_POT
};

enum{
	TDM_TRANS_ERR=-1,
	TDM_TRANS_OK,
	TDM_TRANS_STOP_TDM_EXCEPT,
	TDM_TRANS_STOP_TIMEOUT,
	TDM_TRANS_STOP_RECV_TIMEOUT,
};

enum{
	TDM_APP_FILE,
	TDM_FPGA_FILE
};

typedef struct{
	UCHAR devIdx;
	UCHAR brdIdx;
	UCHAR e1portIdx;
	UCHAR clusterIdx;	
	UCHAR e1portfunc;
	UCHAR almstatus;
	UCHAR almmask;
	UCHAR crcenable;
	UCHAR lpbctrl;
	UCHAR portcode;
	UCHAR impedance;
}PACKED e1porttable_row_entry;

typedef struct{
	UCHAR sgIdx;
	ULONG devIdx;	
	USHORT logiconuIdx;
	UCHAR serviceIdx;
	UCHAR rowstatus;
	UCHAR potsloopenable;
}PACKED tdmonutable_row_entry;

typedef struct{
	UCHAR linksgIdx;
	USHORT linksgportIdx;
	ULONG devIdx;
	UCHAR brdIdx;
	UCHAR potsIdx;
	ULONG phonecode;
	UCHAR linkdesc[32];
	UCHAR linkrowstatus;
}PACKED potslinktable_row_entry;

typedef struct{
	ULONG devIdx;
	UCHAR brdIdx;
	UCHAR potsIdx;
	UCHAR oltbrdidx;
	UCHAR e1clusterIdx;
	UCHAR e1Idx;
	UCHAR e1tsIdx;
	UCHAR sgifIdx;
	USHORT logicalport;
	UCHAR opstatus;
	UCHAR linkstatus;
	ULONG phonecode;
	UCHAR linkdesc[32];
}PACKED potsporttable_row_entry;

typedef struct{
	UCHAR sgifIdx;
	USHORT clusterIdx;
	USHORT mastere1;
	USHORT slavee1;
	USHORT e1clk;
	USHORT vlanen;
	USHORT vlanid;
	USHORT vlanpri;
}PACKED sgtable_row_entry;

typedef struct{
	UCHAR sgifIdx;
	ULONG potsstatusnotify;
	ULONG synframes;
	ULONG ringnotify;
	ULONG ringsync;
	ULONG rolereverse;
	ULONG tsalloc;
	ULONG tsrelease;
	ULONG uptest;
	ULONG upidle;
	ULONG downtest;
	ULONG downidle;
}PACKED hdlcstattable_row_entry;

typedef struct{
	UCHAR reserved1;
	UCHAR reserved2;
	UCHAR lvls[8];
}PACKED tdm_e1_alm_lvl_entry;

FUNC_PRC getRecvHandler( int msgType, int msgSubType, int tableIdx );

STATUS registRecvHandler( int msgType, int msgSubType, int tableIdx, FUNC_PRC handler );

STATUS tdmReadVersion( UCHAR calltype, ULONG callnotifier, UCHAR * softwarever, 
								USHORT *sfverlen, UCHAR *fpgaver, USHORT *fpgaverlen );

STATUS tdmLoadFpga( UCHAR calltype, ULONG callnotifier, UCHAR m1load, UCHAR m2load, UCHAR m3load );

STATUS tdmRunNotify( UCHAR calltype, ULONG callnotifier );

STATUS tdmStatusQuery ( UCHAR calltype, ULONG callnotifier, UCHAR *status );

STATUS tdmReset ( UCHAR calltype, ULONG callnotifier, USHORT target );

#if 0

STATUS tdmConfGet( UCHAR calltype, ULONG callnotifier, UCHAR tableIndex, UCHAR leafIndex, ULONG *idxs, UCHAR idxCounter, 
						UCHAR* varNum, UCHAR **  pValField, UCHAR * const valLen );

STATUS tdmConfGetNext( UCHAR calltype, ULONG callnotifier, UCHAR tableIndex, UCHAR leafIndex, ULONG *idxs, UCHAR idxCounter, 
								UCHAR *varNum, UCHAR * const pValField, UCHAR * const valLen );

STATUS tdmConfSet( UCHAR calltype, ULONG callnotifier, UCHAR tableIndex, UCHAR leafIndex, ULONG *idxs, UCHAR idxCounter, 
						UCHAR * const pValField, const UCHAR valLen );

STATUS tdmConfRowSet( UCHAR calltype, ULONG callnotifier, UCHAR tableIndex, UCHAR leafIndex, ULONG *idxs, UCHAR idxCounter, 
								const UCHAR varNum, UCHAR ** const pValField, const UCHAR *valLen );
#endif

LONG atobcd( const UCHAR* );
int bcdtoa( const ULONG, UCHAR* const );

STATUS tdm_e1portTable_get( ULONG idxs[3], e1porttable_row_entry *pEntry );
STATUS tdm_e1portTable_getNext( ULONG idxs[3], e1porttable_row_entry *pEntry );
STATUS tdm_e1portTable_set( UCHAR leafIdx, ULONG idxs[3], UCHAR setval );
STATUS tdm_e1portTable_rowset( ULONG idxs[3], UCHAR alarmmask, UCHAR crcenable, UCHAR lpbctrl );

STATUS tdm_tdmOnuTable_get( ULONG idxs[2], tdmonutable_row_entry *pEntry );
STATUS tdm_tdmOnuTable_getNext( ULONG idxs[2], tdmonutable_row_entry *pEntry );
STATUS tdm_tdmOnuTable_set( UCHAR leafIdx, ULONG idxs[2], UCHAR setval, USHORT* );
STATUS tdm_tdmOnuTable_rowset( ULONG idxs[2], USHORT logicalOnuIdx, UCHAR onuRowStatus );

STATUS tdm_potsLinkTable_get( ULONG idxs[2], potslinktable_row_entry *pEntry );
STATUS tdm_potsLinkTable_getNext( ULONG idxs[2], potslinktable_row_entry *pEntry );
STATUS tdm_potsLinkTable_set( UCHAR leafIdx, ULONG idxs[2], UCHAR setval[32] );
STATUS tdm_potsLinkTable_rowset( ULONG idxs[2], ULONG onuIdx, ULONG onuPotsBrd, 
			ULONG onuPotsIdx, ULONG phonecode, UCHAR linkdesc[32], UCHAR potsLinkStatus );

STATUS tdm_sgTable_get( ULONG idxs, sgtable_row_entry *pEntry );
STATUS tdm_sgTable_getNext( ULONG idxs, sgtable_row_entry *pEntry );
STATUS tdm_sgTable_set( UCHAR leafIdx, ULONG idxs, USHORT setval );
STATUS tdm_sgTable_rowset( ULONG idxs, USHORT rowset[6] );

STATUS tdm_potsPortTable_get( ULONG idxs[3], potsporttable_row_entry *pEntry );
STATUS tdm_potsPortTable_getNext( ULONG idxs[3], potsporttable_row_entry *pEntry );

STATUS tdm_hdlcStatTable_get( ULONG idxs, hdlcstattable_row_entry *pEntry );
STATUS tdm_hdlcStatTable_getNext( ULONG idxs, hdlcstattable_row_entry *pEntry );

STATUS tdm_E1AlarmLevelGet( tdm_e1_alm_lvl_entry *pEntry );
STATUS tdm_E1AlarmLevelSet( UCHAR lvl[8] );

STATUS tdmDebugQuery ( UCHAR calltype, ULONG callnotifier, ULONG *debug );

STATUS tdmDebugSet ( UCHAR calltype, ULONG callnotifier, ULONG vty, ULONG debug );

void  tdmEventMsgProg(unsigned char *, unsigned int);

void init_tdm_process_function( void );

int startUpdateTdm(const int fileType, const char *name, int ftpmode, 
							const char *host, const char *user, const char *pwd);

STATUS tdm_ExtpotsPortTable_get( ULONG idxs[3], potsporttable_row_entry *pEntry );

#endif

