
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_RMan_Mainh
#define __INCCT_RMan_Mainh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
#include "cli/cli.h"
#include "cli/cl_cmd.h"
#include "cli/cl_mod.h"
#include "ifm/ifm_type.h"
#include "ifm/ifm_pub.h"
#include "ifm/ifm_gtable.h"
#include "ifm/ifm_aux.h"
#include "ifm/ifm_act.h"
#include "ifm/ifm_cli.h"
#include "ifm/ifm_task.h"
#include "ifm/ifm_lock.h"
#include "ifm/ifm_debug.h"
#include "ifm/eth/eth_aux.h"
#include "vos_ctype.h"
#endif


/*----------------------------------------------------------------------------*/

#undef __CT_EXTOAM_SUPPORT

#ifndef CTC_OBSOLETE		/* removed by xieshl 20120607 */
/* Common descriptions. */
#define CTC_STR		"ctc stack operations\n"
#define OLT_STR		"OLT configuration\n"
#define ONU_STR		"ONU configuration\n"
#define LLID_STR		"LLID configuration\n"
#define OAM_STR		"OAM protocol configuration\n"
#define DBA_STR		"DBA configuration mode\n"
#define OLT_INDEX_STR  "OLT index\n"
#define ONU_INDEX_STR  "ONU index\n"

#define CTC_STACK_CLI_ERROR_PRINT		vty_out( vty, "%% Executing error\r\n" )
#endif

#define MODULE_CT_EXT		8721
#define FC_CT_EXT_TIMER	1
#define FC_CT_EXT_SEND		2
#define FC_CT_EXT_RECV		3

typedef struct {
	UCHAR   ponId;
	UCHAR   onuId;
	USHORT dataLen;
	ULONG   dataPtr;
} CT_ExtOamCommMsg_t;


#ifndef PON_STATUS
#define PON_STATUS          short int
#endif	/*PON_STATUS*/

#ifndef MAX_MAC_ADDRESS_AUTHENTICATION_ELEMENTS
typedef short int  PON_onu_id_t;  /* C-type representation of ONU index */
#define PON_llid_t PON_onu_id_t	  /* In current version there is 1:1 ratio between ONU and LLID */

typedef short int  PON_olt_id_t;  /* C-type representation of OLT index */

/*typedef unsigned char mac_address_t [6];*/ 

#define	MAX_MAC_ADDRESS_AUTHENTICATION_ELEMENTS	128

typedef  mac_address_t mac_addresses_list_t[MAX_MAC_ADDRESS_AUTHENTICATION_ELEMENTS];

#endif

/*----------------------------------------------------------------------------*/

/*#include "CTC_STACK.h"*/
#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
#include "CT_RMan_CLI.h"
#include "CT_RMan_ONU.h"
#include "CT_RMan_EthPort.h"
#include "CT_RMan_Multicast.h"
#include "CT_RMan_QoS.h"
#include "CT_RMan_VLAN.h"
#include "CT_Onu_Auth.h"
#endif
/*#include "CT_RMan_VoipPort.h"
#include "CT_RMan_E1Port.h"
#include "CT_RMan_Stat.h"*/

extern int parse_onu_command_parameter( struct vty *vty, PON_olt_id_t *pPonPortIdx, PON_onu_id_t *pOnuIdx, PON_onu_id_t *pOnuId);
extern int parse_llid_command_parameter( ULONG devIdx, PON_olt_id_t *pOltId, PON_onu_id_t *pOnuId, PON_llid_t *pLlidId );
extern int parse_onuidx_command_parameter( ULONG devIdx, PON_olt_id_t *pPonIdx, PON_onu_id_t *pOnuIdx);
extern int parse_onu_devidx_command_parameter( struct vty *vty, ULONG *devIdx, ULONG *slotno, ULONG *pon, ULONG *onu);



#ifndef ENABLE
#define ENABLE  1
#define DISABLE 0
#endif
#define enum_s(name) #name


#define CTC_HOOK_CALL(hookfunc) (NULL == (hookfunc))?(VOS_ERROR):(*hookfunc)

extern char *macAddress_To_Strings( UCHAR *pMacAddr );
extern char *binData_To_Strings(UCHAR *pBin, ULONG len, UCHAR separator );
extern char *bitmap_To_Strings(UCHAR *pBitmap, ULONG len );

extern ULONG	CT_ExtOamDebugSwitch;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_RMan_Mainh */
