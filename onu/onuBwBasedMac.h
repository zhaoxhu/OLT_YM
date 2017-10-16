
#ifndef INConuBwBasedMach
#define INConuBwBasedMach

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CLIE_BANDWITHDELAY_LOW
#define CLIE_BANDWITHDELAY_LOW	1
#define CLIE_BANDWITHDELAY_HIGH	2
#define CLI_EPON_BANDWIDTH_MIN	64
#endif

#define ONU_MAC_ADDR_LEN		6

typedef struct {
	UCHAR  macAddr[ONU_MAC_ADDR_LEN];
	UCHAR  rowStatus;
	UCHAR  bwClass;
	UCHAR  bwDelay;
	ULONG  assuredBw;
	ULONG  bestEffortBw;
#ifdef  PLATO_DBA_V3
	ULONG  fixedBw;
#endif
} __attribute__((packed)) onu_up_bw_node_t;


typedef struct __onu_up_bw_list_t{
	onu_up_bw_node_t node;
	struct __onu_up_bw_list_t *next;
} __attribute__((packed)) onu_up_bw_list_t;

typedef struct {
	UCHAR  macAddr[ONU_MAC_ADDR_LEN];
	UCHAR  rowStatus;
	UCHAR  bwClass;
	UCHAR  bwDelay;
	ULONG  assuredBw; 
	ULONG  bestEffortBw; 
	/*ULONG  fixedBw;*/
} __attribute__((packed)) onu_down_bw_node_t;


typedef struct __onu_down_bw_list_t{
	onu_down_bw_node_t node;
	struct __onu_down_bw_list_t *next;
} __attribute__((packed)) onu_down_bw_list_t;


#define ONU_BW_CODE_UP_INSERT		1
#define ONU_BW_CODE_UP_DELETE		2
#define ONU_BW_CODE_DOWN_INSERT		3
#define ONU_BW_CODE_DOWN_DELETE		4
#define ONU_BW_CODE_UP_SYNC			5
#define ONU_BW_CODE_DOWN_SYNC		6

typedef struct  {
	onu_sync_msg_head_t   OnuSyncMsgHead;
	UCHAR operCode;
	onu_up_bw_node_t	 node;
}__attribute__((packed)) onu_sync_bw_msg_t;


#if(RPU_MODULE_SNMP == RPU_YES)
extern LONG getOnuUBwBasedMacFixed( UCHAR *pOnuMacAddr, ULONG *pFixedBw );
extern LONG setOnuUBwBasedMacFixed( UCHAR *pOnuMacAddr, ULONG fixedBw );
extern LONG getOnuUBwBasedMacAssured( UCHAR *pOnuMacAddr, ULONG *pAssuredBw );
extern LONG setOnuUBwBasedMacAssured( UCHAR *pOnuMacAddr, ULONG assuredBw );
extern LONG getOnuUBwBasedMacBestEffort( UCHAR *pOnuMacAddr, ULONG *pBeBw );
extern LONG setOnuUBwBasedMacBestEffort( UCHAR *pOnuMacAddr, ULONG beBw );
extern LONG getOnuUBwBasedMacClass( UCHAR *pOnuMacAddr, ULONG *pBwClass );
extern LONG setOnuUBwBasedMacClass( UCHAR *pOnuMacAddr, ULONG bwClass );
extern LONG getOnuUBwBasedMacDelay( UCHAR *pOnuMacAddr, ULONG *pBwDelay );
extern LONG setOnuUBwBasedMacDelay( UCHAR *pOnuMacAddr, ULONG bwDelay );
extern LONG getOnuUBwBasedMacStatus( UCHAR *pOnuMacAddr, ULONG *pRowStatus );
extern LONG setOnuUBwBasedMacStatus( UCHAR *pOnuMacAddr, ULONG rowStatus );
extern LONG checkOnuUBwBasedMacIdx(UCHAR *pOnuMacAddr );
extern LONG getFirstOnuUBwBasedMacIdx( UCHAR *pOnuMacAddr );
extern LONG getNextOnuUBwBasedMacIdx(UCHAR *pOnuMacAddr, UCHAR *pNextOnuMacAddr );

extern LONG getOnuDBwBasedMacFixed( UCHAR *pOnuMacAddr, ULONG *pFixedBw );
extern LONG setOnuDBwBasedMacFixed( UCHAR *pOnuMacAddr, ULONG fixedBw );
extern LONG getOnuDBwBasedMacAssured( UCHAR *pOnuMacAddr, ULONG *pAssuredBw );
extern LONG setOnuDBwBasedMacAssured( UCHAR *pOnuMacAddr, ULONG assuredBw );
extern LONG getOnuDBwBasedMacBestEffort( UCHAR *pOnuMacAddr, ULONG *pBeBw );
extern LONG setOnuDBwBasedMacBestEffort( UCHAR *pOnuMacAddr, ULONG beBw );
extern LONG getOnuDBwBasedMacClass( UCHAR *pOnuMacAddr, ULONG *pBwClass );
extern LONG setOnuDBwBasedMacClass( UCHAR *pOnuMacAddr, ULONG bwClass );
extern LONG getOnuDBwBasedMacDelay( UCHAR *pOnuMacAddr, ULONG *pBwDelay );
extern LONG setOnuDBwBasedMacDelay( UCHAR *pOnuMacAddr, ULONG bwDelay );
extern LONG getOnuDBwBasedMacStatus( UCHAR *pOnuMacAddr, ULONG *pRowStatus );
extern LONG setOnuUBwBasedMacStatus( UCHAR *pOnuMacAddr, ULONG rowStatus );
extern LONG checkOnuDBwBasedMacIdx(UCHAR *pOnuMacAddr );
extern LONG getFirstOnuDBwBasedMacIdx( UCHAR *pOnuMacAddr );
extern LONG getNextOnuDBwBasedMacIdx(UCHAR *pOnuMacAddr, UCHAR *pNextOnuMacAddr );

#endif

extern LONG setOnuUpBwBasedMac( UCHAR *pMacAddr, ULONG bwClass, ULONG bwDelay, ULONG fixedBw, ULONG assuredBw, ULONG bestEffortBw );
extern LONG delOnuUpBwBasedMac( UCHAR *pMacAddr );
extern LONG setOnuDownBwBasedMac( UCHAR *pMacAddr, ULONG bwClass, ULONG bwDelay, ULONG assuredBw, ULONG bestEffortBw );
extern LONG delOnuDownBwBasedMac( UCHAR *pMacAddr );

extern LONG getFirstOnuUpBwBasedMacNode( onu_up_bw_node_t *pNode );
extern LONG getNextOnuUpBwBasedMacNode(UCHAR *pOnuMacAddr, onu_up_bw_node_t *pNextNode );
extern LONG getFirstOnuDownBwBasedMacNode( onu_down_bw_node_t *pNode );
extern LONG getNextOnuDownBwBasedMacNode(UCHAR *pOnuMacAddr, onu_down_bw_node_t *pNextNode );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* INConuBwBasedMach */
