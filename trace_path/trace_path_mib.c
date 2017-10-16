#ifdef __cplusplus
extern "C"{
#endif

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include "gwEponSys.h"
#include "V2R1_product.h"
#include  "trace_path_lib.h"
#include  "trace_path_main.h"
#include  "trace_path_api.h"
#include  "trace_path_mib.h"



oid userTraceMib_variables_oid[] = { 1,3,6,1,4,1,10072,2,26 };

struct variable4 userTraceMib_variables[] = {
/*  magic number        , variable type , ro/rw , callback fn  , L, oidsuffix */

#define macTraceMacAddr		1
	{macTraceMacAddr,  ASN_OCTET_STR,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 1 }},
#define macTraceOltSysId		2
	{macTraceOltSysId,  ASN_OCTET_STR,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 2 }},
#define macTraceOltBrdIdx		3
	{macTraceOltBrdIdx,  ASN_INTEGER,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 3 }},
#define macTraceOltPortIdx		4
	{macTraceOltPortIdx,  ASN_INTEGER,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 4 }},
#define macTraceOnuDevIdx	5
	{macTraceOnuDevIdx,  ASN_INTEGER,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 5 }},
#define macTraceOnuBrdIdx		6
	{macTraceOnuBrdIdx,  ASN_INTEGER,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 6 }},
#define macTraceOnuPortIdx	7
	{macTraceOnuPortIdx,  ASN_INTEGER,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 7 }},
#define macTraceSwCascaded	8
	{macTraceSwCascaded,  ASN_INTEGER,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 8 }},
#define macTraceSwMacAddr	9
	{macTraceSwMacAddr,  ASN_OCTET_STR,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 9 }},
#define macTraceSwPortIdx		10
	{macTraceSwPortIdx,  ASN_INTEGER,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 10 }},
#define macTraceMacStatus		11
	{macTraceMacStatus,  ASN_INTEGER,  RONLY,   var_userMacTraceTable, 4,  { 1, 1, 1, 11 }},


#define userLocUserId			1
	{userLocUserId,  ASN_OCTET_STR,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 1 }},
#define userLocOltSysId		2
	{userLocOltSysId,  ASN_OCTET_STR,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 2 }},
#define userLocOltBrdIdx		3
	{userLocOltBrdIdx,  ASN_INTEGER,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 3 }},
#define userLocOltPortIdx		4
	{userLocOltPortIdx,  ASN_INTEGER,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 4 }},
#define userLocOnuDevIdx		5
	{userLocOnuDevIdx,  ASN_INTEGER,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 5 }},
#define userLocOnuBrdIdx		6
	{userLocOnuBrdIdx,  ASN_INTEGER,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 6 }},
#define userLocOnuPortIdx		7
	{userLocOnuPortIdx,  ASN_INTEGER,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 7 }},
#define userLocSwCascaded		8
	{userLocSwCascaded,  ASN_INTEGER,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 8 }},
#define userLocSwMacAddr		9
	{userLocSwMacAddr,  ASN_OCTET_STR,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 9 }},
#define userLocSwPortIdx		10
	{userLocSwPortIdx,  ASN_INTEGER,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 10 }},
#define userLocRowStatus		11
	{userLocRowStatus,  ASN_INTEGER,  RWRITE,   var_userLocationTable, 4,  { 2, 1, 1, 11 }},
#define userLocUserMacAddr		12
	{userLocUserMacAddr,  ASN_OCTET_STR,  RONLY,   var_userLocationTable, 4,  { 2, 1, 1, 12 }},


#define vlanTraceSvlan			1
	{vlanTraceSvlan,  ASN_INTEGER,  RONLY,   var_vlanTraceTable, 4,  { 3, 1, 1, 1 }},
#define vlanTraceCvlan		2
	{vlanTraceCvlan,  ASN_INTEGER,  RONLY,   var_vlanTraceTable, 4,  { 3, 1, 1, 2 }},
#define vlanTraceOltSysId		3
	{vlanTraceOltSysId,  ASN_OCTET_STR,  RONLY,   var_vlanTraceTable, 4,  { 3, 1, 1, 3 }},
#define vlanTraceOltbrdIdx		4
	{vlanTraceOltbrdIdx,  ASN_INTEGER,  RONLY,   var_vlanTraceTable, 4,  { 3, 1, 1, 4 }},
#define vlanTraceOltPortIdx		5
	{vlanTraceOltPortIdx,  ASN_INTEGER,  RONLY,   var_vlanTraceTable, 4,  { 3, 1, 1, 5 }},
#define vlanTraceOnuDevIdx		6
	{vlanTraceOnuDevIdx,  ASN_INTEGER,  RONLY,   var_vlanTraceTable, 4,  { 3, 1, 1, 6 }},
#define vlanTraceOnuBrdIdx		7
	{vlanTraceOnuBrdIdx,  ASN_INTEGER,  RONLY,   var_vlanTraceTable, 4,  { 3, 1, 1, 7 }},
#define vlanTraceOnuPortIdx		8
	{vlanTraceOnuPortIdx,  ASN_INTEGER,  RONLY,   var_vlanTraceTable, 4,  { 3, 1, 1, 8 }},
};


extern LONG trace_path_fdb_init();
extern trace_vlan_data_t g_trace_vlan_buff;
extern int vlan_trace_is_still_valid();

void init_userTraceMib(void)
{
	trace_path_fdb_init();
	
    /* register ourselves with the agent to handle our mib tree */
    REGISTER_MIB("userTraceMib", userTraceMib_variables, variable4,
               userTraceMib_variables_oid);

    /* place any other initialization junk you need here */
}

#define MAC_OID_CPY(D,S)	\
	(D)[0] = (S)[0]; (D)[1] = (S)[1]; (D)[2] = (S)[2];\
	(D)[3] = (S)[3]; (D)[4] = (S)[4]; 	(D)[5] = (S)[5];

STATUS header_userMacTraceTable( struct variable *vp,
	                                oid      *name,
	                                size_t   *length,
	                                int      exact,
	                                UCHAR  *nextMacAddr)
{
	STATUS func_ret = MATCH_FAILED;
	long ret;
	const  ulong_t qnamelen = *length;
	uchar namelen = vp->namelen;
	UCHAR macAddr[6];

	if(vp == NULL || qnamelen > MAX_OID_LEN)
		return func_ret; 

	ret = snmp_oid_compare( name, namelen, vp->name, namelen );

	if(exact)
	{
		if( (ret == 0) && (qnamelen == namelen+6) )
		{
			MAC_OID_CPY( nextMacAddr, (&name[namelen]) );
			/*if( checkMacTraceIndex(nextMacAddr) == VOS_OK )*/
			{
				func_ret = MATCH_SUCCEEDED;
			}
		}
	}
	else
	{
		if( ret < 0 )
		{
			if( getMacTraceFirstIndex(nextMacAddr) == VOS_OK )
			{
				VOS_MemCpy( name, vp->name, namelen*sizeof(oid) );
				MAC_OID_CPY( (&name[namelen]), nextMacAddr );
				func_ret = MATCH_SUCCEEDED;
			}
		}
		else if(ret==0)
		{
			if( qnamelen == namelen )/*不带索引的getnext, call getfirst */
			{
				if( getMacTraceFirstIndex(nextMacAddr) == VOS_OK )
				{
					MAC_OID_CPY( (&name[namelen]), nextMacAddr );
					func_ret = MATCH_SUCCEEDED;
				}
			}
			else	 if( qnamelen == namelen+6 )	/* call getnext */
			{
				MAC_OID_CPY( macAddr, (&name[namelen]) );
				if( getMacTraceNextIndex(macAddr, nextMacAddr) == VOS_OK )
				{
					MAC_OID_CPY( (&name[namelen]), nextMacAddr );
					func_ret = MATCH_SUCCEEDED;
				}
			}
		}
	}
	if( func_ret == MATCH_SUCCEEDED )
	{
		if( MAC_ADDR_IS_INVALID(nextMacAddr) )
			func_ret = MATCH_FAILED;
		else
		{
			*length=namelen+6;
			/*MAC_OID_CPY( pUserMacAddr, nextMacAddr );*/
		}
	}
	return func_ret;
}

unsigned char *
var_userMacTraceTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
	/* variables we may use later */
	static UCHAR macAddr[6];
	static UCHAR retVal[6];
  
	if( header_userMacTraceTable(vp,name,length,exact, macAddr) == MATCH_FAILED )
		return NULL;

	switch(vp->magic) {
		case macTraceMacAddr:
			if( getMacTraceMacAddr(macAddr) == VOS_OK )
			{
				*var_len = 6;
				return (u_char *)macAddr;
			}
			break;
		case macTraceOltSysId:
			if( getMacTraceOltSysId(macAddr, (UCHAR *)retVal) == VOS_OK )
			{	
				*var_len = 6;
				return (uchar_t*)retVal;
			}
			break;
		case macTraceOltBrdIdx:
			if( getMacTraceOltBrdIdx(macAddr, (ULONG *)retVal) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)retVal;
			}
			break;
		case macTraceOltPortIdx:
			if( getMacTraceOltPortIdx(macAddr, (ULONG *)retVal) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)retVal;
			}
			break;
		case macTraceOnuDevIdx:
			if( getMacTraceOnuDevIdx(macAddr, (ULONG *)retVal) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)retVal;
			}
			break;
		case macTraceOnuBrdIdx:
			if( getMacTraceOnuBrdIdx(macAddr, (ULONG *)retVal) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)retVal;
			}
			break;
		case macTraceOnuPortIdx:
			if( getMacTraceOnuPortIdx(macAddr, (ULONG *)retVal) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)retVal;
			}
			break;
		case macTraceSwCascaded:
			if( getMacTraceSwCascaded(macAddr, (ULONG *)retVal) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)retVal;
			}
			break;
		case macTraceSwMacAddr:
			if( getMacTraceSwMacAddr(macAddr, (UCHAR *)retVal) == VOS_OK )
			{	
				*var_len = 6;
				return (uchar_t*)retVal;
			}
			break;
		case macTraceSwPortIdx:
			if( getMacTraceSwPortIdx(macAddr, (ULONG *)retVal) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)retVal;
			}
			break;
		case macTraceMacStatus:
			if( getMacTraceMacStatus(macAddr, (ULONG *)retVal) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)retVal;
			}
			break;

		default:
			ERROR_MSG("");
	}
	return NULL;
}

LONG user_id_2_oid( UCHAR *pUserId, oid *oid_idx )
{
	LONG i = 0;

	if( (pUserId == NULL) || (oid_idx == NULL) )
		return i;
	if( USER_ID_MAXLEN > MAX_OID_LEN )
		return i;
	
	for( ; i<USER_ID_MAXLEN; i++ )
	{
		if( pUserId[i] == 0 )
			break;
		oid_idx[i] = pUserId[i];
	}
	return i;
}

LONG user_oid_2_id( oid *oid_idx, LONG oid_len, UCHAR *pUserId )
{
	LONG i = 0;

	if( (pUserId == NULL) || (oid_idx == NULL) )
		return i;
	if( oid_len > USER_ID_MAXLEN )
		return i;
	
	for( ; i<oid_len; i++ )
	{
		if( oid_idx[i] == 0 )
			break;
		pUserId[i] = oid_idx[i];
	}
	pUserId[i] = 0;
	return i;
}

STATUS header_userLocationTable( struct variable *vp,
	                                oid      *name,
	                                size_t   *length,
	                                int      exact,
	                                UCHAR  *pUserId )
{
	long ret;
	STATUS func_ret = MATCH_FAILED;
	/*oid qname[MAX_OID_LEN];*/
	const  ulong_t qnamelen = *length;
	uchar namelen = vp->namelen;
	UCHAR user_id[USER_ID_MAXLEN];
	ULONG user_id_len = 0;

	if(vp == NULL || qnamelen > MAX_OID_LEN)
		return func_ret; 

	VOS_MemZero( pUserId, USER_ID_MAXLEN );
	/*VOS_MemZero( qname, sizeof(qname));
	VOS_MemCpy( qname, name, sizeof(oid)*qnamelen);
	
	ret = snmp_oid_compare( qname, namelen, vp->name, namelen );*/
	ret = snmp_oid_compare( name, namelen, vp->name, namelen );

	if(exact)
	{
		if( ret == 0 )
		{
			user_id_len = user_oid_2_id( &name[namelen], qnamelen-namelen, pUserId );
			if( user_id_len != 0 )
			{
				if( checkUserLocIndex(pUserId) == VOS_OK )
				{
					func_ret = MATCH_SUCCEEDED;
				}
			}
		}
	}
	else
	{
		if( ret < 0 )
		{
			if( getUserLocFirstIndex(pUserId) == VOS_OK )
			{
				VOS_MemCpy( name, vp->name, namelen*sizeof(oid) );
				user_id_len = user_id_2_oid( pUserId, &name[namelen] );
				if( user_id_len != 0 )
				{
					func_ret = MATCH_SUCCEEDED;
				}
			}
		}
		else if( ret == 0 )
		{
			if( qnamelen == namelen )/*不带索引的getnext, call getfirst */
			{
				if( getUserLocFirstIndex(pUserId) == VOS_OK )
				{
					user_id_len = user_id_2_oid( pUserId, &name[namelen] );
					if( user_id_len != 0 )
					{
						func_ret = MATCH_SUCCEEDED;
					}
				}
			}
			else		/* call getnext */
			{
				if( user_oid_2_id(&name[namelen], qnamelen-namelen, user_id) != 0 )
				{
					if( getUserLocNextIndex(user_id, pUserId) == VOS_OK )
					{
						user_id_len = user_id_2_oid( pUserId, &name[namelen] );
						if( user_id_len != 0 )
						{
							func_ret = MATCH_SUCCEEDED;
						}
					}
				}
			}
		}
	}
	if( func_ret == MATCH_SUCCEEDED )
	{
		*length = namelen + user_id_len;
	}
	
	return func_ret;
}

unsigned char *
var_userLocationTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
	/* variables we may use later */
	/*static ULONG ulVal[2];*/
	static UCHAR val[32];
	static UCHAR user_id[USER_ID_MAXLEN];
  
	if( header_userLocationTable(vp,name,length,exact, user_id) == MATCH_FAILED )
		return NULL;

	switch(vp->magic) {
		case userLocUserId:
			if( getUserLocUserId(user_id) == VOS_OK )
			{
				*var_len = VOS_StrLen(user_id);
				return (u_char *) user_id;
			}
			break;
		case userLocOltSysId:
			if( getUserLocOltSysId(user_id, val) == VOS_OK )
			{	
				*var_len = USER_MACADDR_LEN;
				return (uchar_t*)val;
			}
			break;
		case userLocOltBrdIdx:
			if( getUserLocOltBrdIdx(user_id, (ULONG *)val) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)val;
			}
			break;
		case userLocOltPortIdx:
			if( getUserLocOltPortIdx(user_id, (ULONG *)val) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)val;
			}
			break;
		case userLocOnuDevIdx:
			if( getUserLocOnuDevIdx(user_id, (ULONG *)val) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)val;
			}
			break;
		case userLocOnuBrdIdx:
			if( getUserLocOnuBrdIdx(user_id, (ULONG *)val) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)val;
			}
			break;
		case userLocOnuPortIdx:
			if( getUserLocOnuPortIdx(user_id, (ULONG *)val) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)val;
			}
			break;
		case userLocSwCascaded:
			if( getUserLocSwCascaded(user_id, (ULONG *)val) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)val;
			}
			break;
		case userLocSwMacAddr:
			if( getUserLocSwMacAddr(user_id, val) == VOS_OK )
			{	
				*var_len = 6;
				return (uchar_t*)val;
			}
			break;
		case userLocSwPortIdx:
			if( getUserLocSwPortIdx(user_id, (ULONG *)val) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)val;
			}
			break;
		case userLocRowStatus:
		        *write_method = write_userLocRowStatus;
			if( getUserLocRowStatus(user_id, (ULONG *)val) == VOS_OK )
			{	
				*var_len = sizeof(ULONG);
				return (uchar_t*)val;
			}
			break;
		case userLocUserMacAddr:
			if( getUserLocUserMacAddr(user_id, val) == VOS_OK )
			{	
				*var_len = 17/*VOS_StrLen(val)*/;
				return (uchar_t*)val;
			}
			break;
		default:
			ERROR_MSG("");
	}
	return NULL;
}

int write_userLocRowStatus(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
	static UCHAR user_id[USER_ID_MAXLEN];

    switch ( action ) {
        case RESERVE1:
          if (var_val_type != ASN_INTEGER) {
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long)) {
              return SNMP_ERR_WRONGLENGTH;
          }
          break;

        case RESERVE2:
          break;
          
        case ACTION:
         break;

        case COMMIT:
		if( user_oid_2_id(&name[13], name_len-13, user_id) != 0 )
		{
			if( setUserLocRowStatus( user_id, *(ULONG*)var_val) == VOS_ERROR )
				return SNMP_ERR_COMMITFAILED;
		}
		else
			return SNMP_ERR_COMMITFAILED;
          break;
    }
    return SNMP_ERR_NOERROR;
}

STATUS header_vlanTraceTable( struct variable *vp,
	                                oid      *name,
	                                size_t   *length,
	                                int      exact,
	                                USHORT  *pUserSvlan,
	                                USHORT  *PUserCvlan)
{
	long ret;
	STATUS func_ret = MATCH_FAILED;
	const  ulong_t qnamelen = *length;
	uchar namelen = vp->namelen;
	oid     qname[MAX_OID_LEN];

	if(vp == NULL || qnamelen > MAX_OID_LEN || pUserSvlan == NULL || PUserCvlan == NULL)
		return func_ret; 

    VOS_MemSet( qname, 0, sizeof(qname));
    VOS_MemCpy( qname, name, sizeof(oid)*(*length));

	ret = snmp_oid_compare( qname, namelen, vp->name, namelen );

	if(exact)
	{
		if( ret == 0 )
		{
	        *pUserSvlan = qname[namelen];
		    *PUserCvlan = qname[namelen+1];
			if(*pUserSvlan < 4096 && *PUserCvlan < 4096)
			{
                func_ret = MATCH_SUCCEEDED;
               
                if(g_trace_vlan_buff.svlan == *pUserSvlan 
                     && g_trace_vlan_buff.cvlan == *PUserCvlan
                     && vlan_trace_is_still_valid() == VOS_OK)
                {
                    /*上一个定位信息相同，不需要重新获取*/
                }
                else                   
                {
                    VOS_MemZero(&g_trace_vlan_buff, sizeof(trace_vlan_data_t));
                    g_trace_vlan_buff.svlan = *pUserSvlan;
                    g_trace_vlan_buff.cvlan = *PUserCvlan;
                }
			}
		}
	}
	else
	{
        /*do nothing*/
	}
	
	return func_ret;
}

unsigned char *
var_vlanTraceTable(struct variable *vp,
    	    oid     *name,
    	    size_t  *length,
    	    int     exact,
    	    size_t  *var_len,
    	    WriteMethod **write_method)
{
	/* variables we may use later */
	/*static ULONG ulVal[2];*/
	static UCHAR val[32];
    USHORT svlan = 0, cvlan = 0;
	static ulong_t   ulVal = 0;
	if( header_vlanTraceTable(vp,name,length,exact, &svlan, &cvlan) == MATCH_FAILED )
		return NULL;

	switch(vp->magic) {
		case vlanTraceSvlan:
			*var_len = sizeof(svlan);
			return (u_char *) svlan;
		case vlanTraceCvlan:
			*var_len = sizeof(cvlan);
			return (u_char *) cvlan;
		case vlanTraceOltSysId:
			MAC_ADDR_CPY( val, SYS_PRODUCT_BASEMAC );
            *var_len = USER_MACADDR_LEN;
            return (u_char*)val;
		case vlanTraceOltbrdIdx:
            if(getvlanTraceOltBrdIdx(svlan, cvlan, &ulVal) == VOS_OK)
            {
    			*var_len = sizeof(ulong_t);                
			    return (u_char*)&ulVal;
            }
			break;			
		case vlanTraceOltPortIdx:
            if(getvlanTraceOltPortIdx(svlan, cvlan, &ulVal) == VOS_OK)
            {
    			*var_len = sizeof(ulong_t);                
			    return (u_char*)&ulVal;
            }
			break;
		case vlanTraceOnuDevIdx:
            if(getvlanTraceOnuDevIdx(svlan, cvlan, &ulVal) == VOS_OK)
            {
    			*var_len = sizeof(ulong_t);                
			    return (u_char*)&ulVal;
            }
			break;			
		case vlanTraceOnuBrdIdx:
            if(getvlanTraceOnuBrdIdx(svlan, cvlan, &ulVal) == VOS_OK)
            {
    			*var_len = sizeof(ulong_t);                
			    return (u_char*)&ulVal;
            }
			break;
		case vlanTraceOnuPortIdx:
            if(getvlanTraceOnuPortIdx(svlan, cvlan, &ulVal) == VOS_OK)
            {
    			*var_len = sizeof(ulong_t);                
			    return (u_char*)&ulVal;
            }
			break;
		default:
			ERROR_MSG("");
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

