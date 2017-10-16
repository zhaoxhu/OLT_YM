#ifndef __INCeventTraph
#define __INCeventTraph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*----------------------------------------------------------------------------*/

/*extern int bindMib2TrapVar( SNMPTRAP *trapvar, ulong_t trapId, ulong_t *pVarlist, const ulong_t varNum );*/
extern int sendMib2Trap( ulong_t trapid );

/*extern int bindBridgeTrapVar( SNMPTRAP *trapvar, ulong_t trapId, ulong_t *pVarlist, const ulong_t varNum );*/
extern int sendBridgeTrap( ulong_t trapid );
extern STATUS  sendPrivateTrap( ulong_t trapId, ulong_t *pVarlist, const ulong_t varNum );

extern int sendCmcCtrlTrap( ulong_t trapid, ulong_t *pVarlist, const ulong_t varNum );


/*----------------------------------------------------------------------------*/



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCeventTraph */
