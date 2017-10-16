#ifndef TRACE_PATH_API_H
#define TRACE_PATH_API_H

#ifdef __cplusplus
extern "C"{
#endif


extern LONG getMacTraceMacAddr( UCHAR *pMacAddr );
extern LONG getMacTraceOltSysId( UCHAR *pMacAddr, UCHAR *pOltSysMac );
extern LONG getMacTraceOltBrdIdx( UCHAR *pMacAddr, ULONG *pOltBrdIdx );
extern LONG getMacTraceOltPortIdx( UCHAR *pMacAddr, ULONG *pOltPortIdx );
extern LONG getMacTraceOnuDevIdx( UCHAR *pMacAddr, ULONG *pOnuDevIdx );
extern LONG getMacTraceOnuBrdIdx( UCHAR *pMacAddr, ULONG *pOnuBrdIdx );
extern LONG getMacTraceOnuPortIdx( UCHAR *pMacAddr, ULONG *pOnuPortIdx );
extern LONG getMacTraceSwCascaded( UCHAR *pMacAddr, ULONG *pSwFlag );
extern LONG getMacTraceSwMacAddr( UCHAR *pMacAddr, UCHAR *pSwMacAddr );
extern LONG getMacTraceSwPortIdx( UCHAR *pMacAddr, ULONG *pSwPortIdx );
extern LONG getMacTraceMacStatus( UCHAR *pMacAddr, ULONG *pMacStatus );
extern LONG getMacTraceFirstIndex( UCHAR *pFirstMacAddr );
extern LONG getMacTraceNextIndex( UCHAR *pMacAddr, UCHAR *pNextMacAddr );
extern LONG checkMacTraceIndex( UCHAR *pMacAddr );
extern LONG swMac2Cascaded( UCHAR *pSwMacAddr );


#define USER_LOC_ROWSTATUS_ACTIVE	1
#define USER_LOC_ROWSTATUS_NOTREADY	3
#define USER_LOC_ROWSTATUS_DESTROY	6


extern LONG getUserLocFirstIndex( UCHAR *pFirstUserId );
extern LONG getUserLocNextIndex( UCHAR *pUserId, UCHAR *pNextUserId );
extern LONG checkUserLocIndex( UCHAR *pUserId );
extern LONG getUserLocUserId( UCHAR *pUserId );
extern LONG getUserLocOltSysId( UCHAR *pUserId, UCHAR *pOltSysMac );
extern LONG getUserLocOltBrdIdx( UCHAR *pUserId, ULONG *pOltBrdIdx );
extern LONG getUserLocOltPortIdx( UCHAR *pUserId, ULONG *pOltPortIdx );
extern LONG getUserLocOnuDevIdx( UCHAR *pUserId, ULONG *pOnuDevIdx );
extern LONG getUserLocOnuBrdIdx( UCHAR *pUserId, ULONG *pOnuBrdIdx );
extern LONG getUserLocOnuPortIdx( UCHAR *pUserId, ULONG *pOnuPortIdx );
extern LONG getUserLocSwCascaded( UCHAR *pUserId, ULONG *pSwFlag );
extern LONG getUserLocSwMacAddr( UCHAR *pUserId, UCHAR *pSwMac );
extern LONG getUserLocSwPortIdx( UCHAR *pUserId, ULONG *pSwPortIdx );
extern LONG getUserLocRowStatus( UCHAR *pUserId, ULONG *pRowStatus );
extern LONG setUserLocRowStatus( UCHAR *pUserId, ULONG rowStatus );

extern LONG getvlanTraceOltBrdIdx( USHORT svlan, USHORT cvlan, ULONG *pOltBrdIdx );
extern LONG getvlanTraceOltPortIdx( USHORT svlan, USHORT cvlan, ULONG *pOltPortIdx );
extern LONG getvlanTraceOnuDevIdx( USHORT svlan, USHORT cvlan, ULONG *pOnuDevIdx );
extern LONG getvlanTraceOnuBrdIdx( USHORT svlan, USHORT cvlan, ULONG *pOnuBrdIdx );
extern LONG getvlanTraceOnuPortIdx( USHORT svlan, USHORT cvlan, ULONG *pOnuPortIdx );
extern int SearchOnuVlanPort(short int PonPortIdx, short int OnuIdx, USHORT vid, ULONG *brdIdx, ULONG *port);

extern LONG getUserLocTrapVarBindData( ULONG userid_hash_idx, UCHAR *pUserMac, UCHAR *pUserId, user_loc_t* pLoc );


#ifdef __cplusplus
}
#endif

#endif	/* TRACE_PATH_API_H */
