
/*#include "syscfg.h"*/

#ifdef __cplusplus
extern"C"
{
#endif


#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_task.h"
#include "include/man/mn_api.h"

#include "vos/vospubh/vos_sysmsg.h"
#include "softfw_pub.h"
#include "linux/ip.h"
#include "linux/if.h"
#include "sys/main/sys_main.h"


#include "manage/snmp/sn_conf.h"
#include "manage/snmp/sn_impl.h"
#include "manage/snmp/cl_sn.h"
#include "manage/mn_set.h"

#include "manage/snmp/sn_lib.h"
#include "manage/snmp/sn_sys.h"
#include "manage/snmp/sn_a_lib.h"

#include "vos/vospubh/Vos_que.h"
#include "vos/vospubh/Vos_sem.h"
#include "vos/vospubh/vos_syslog.h"
#include "manage/snmp/sn_cvtovos.h"


extern struct timeval    starttime;
extern oid snmptrap_oid[];
extern oid snmptrapenterprise_oid[];
extern oid sysuptime_oid[];
extern INT32 snmptrap_oid_len;
extern INT32 snmptrapenterprise_oid_len;
extern INT32 sysuptime_oid_len;

/* 根据trap ID和绑定对象等信息进行trap报文编码，主要用于告警同步 */
INT32 code_snmp_trap_packet( SNMPTRAP * SnmpTrap, UCHAR * packet,  size_t *out_length )
{
	struct variable_list uptime_var, snmptrap_var, enterprise_var;
	struct variable_list *v2_vars, *last_var=NULL;
	struct snmp_pdu	*template_pdu, *pdu;
	struct timeval	 now;
	long uptime;
	/*struct sockaddr_in *pduIp;
	oid temp_oid[MAX_OID_LEN];*/

	int trap = SnmpTrap->GenericType;
	int specific = SnmpTrap->SpecificType;
	oid *enterprise = NULL;
	int enterprise_length = 0;
	struct variable_list *vars = NULL;

	struct variable_list Var[ BIND_VAR_NUMS ];
	INT32 i;

	memset ( (CHAR *)Var, 0, BIND_VAR_NUMS * sizeof ( struct variable_list ) );

	if ( SnmpTrap->VarNum > BIND_VAR_NUMS )
	{
		return VOS_ERROR;
	}

	if ( SnmpTrap->VarNum == 0 )
	{
		if ( SnmpTrap->TrapOidLength != 0 )
		 {
			enterprise = SnmpTrap->TrapOid;
			enterprise_length = SnmpTrap->TrapOidLength;
			vars = NULL;
		}
	}
	else
	{
		for ( i = 0; i < SnmpTrap->VarNum; i ++ )
		{
			snmp_set_var_objid( &Var[ i ], SnmpTrap->VarOid[ i ], (ULONG)SnmpTrap->VarOidLength[ i ] );
			snmp_set_var_value( &Var[ i ], SnmpTrap->Var[ i ], (ULONG)SnmpTrap->VarLength[ i ] );
			Var[ i ].type = SnmpTrap->VarType[ i ];
			if ( ( i + 1 ) == SnmpTrap->VarNum ) 
			{
				Var[ i ].next_variable = NULL;  
			}
			else
			{
				Var[ i ].next_variable = &Var[ i + 1 ]; 
			}
		}

		if ( SnmpTrap->TrapOidLength == 0 )
		{
			vars = &Var[ 0 ] ;
		}
		else
		{
			enterprise = SnmpTrap->TrapOid;
			enterprise_length = SnmpTrap->TrapOidLength;
			vars = &Var[ 0 ] ;

		}
	}


	gettimeofday(&now, NULL);
	uptime = calculate_time_diff(&now, &starttime);
	memset (&uptime_var, 0, sizeof (struct variable_list));
	snmp_set_var_objid( &uptime_var, sysuptime_oid, sysuptime_oid_len);
	snmp_set_var_value( &uptime_var, (u_char *)&uptime, sizeof(uptime) );
	uptime_var.type           = ASN_TIMETICKS;
	uptime_var.next_variable  = &snmptrap_var;

	memset (&snmptrap_var, 0, sizeof (struct variable_list));
	snmp_set_var_objid( &snmptrap_var, snmptrap_oid, snmptrap_oid_len);
	/* value set later .... */
	snmptrap_var.type           = ASN_OBJECT_ID;
	if ( vars )
		snmptrap_var.next_variable  = vars;
	else
		snmptrap_var.next_variable  = &enterprise_var;

	last_var = vars;
	while ( last_var && last_var->next_variable )
		last_var = last_var->next_variable;

	memset (&enterprise_var, 0, sizeof (struct variable_list));
	snmp_set_var_objid( &enterprise_var,
		 snmptrapenterprise_oid, snmptrapenterprise_oid_len);
	snmp_set_var_value( &enterprise_var, (u_char *)enterprise, enterprise_length*sizeof(oid));
	enterprise_var.type           = ASN_OBJECT_ID;
	enterprise_var.next_variable  = NULL;

	v2_vars = &uptime_var;

	template_pdu = snmp_pdu_create( SNMP_MSG_TRAP );
	if ( template_pdu == NULL )
		return VOS_ERROR;
	template_pdu->trap_type     = trap;
	template_pdu->specific_type = specific;
	if ( snmp_clone_mem((void **)&template_pdu->enterprise,
				enterprise, enterprise_length*sizeof(oid)))
	{
		snmp_free_pdu( template_pdu );
		return VOS_ERROR;
	}
	template_pdu->enterprise_length = enterprise_length;
	template_pdu->flags |= UCD_MSG_FLAG_FORCE_PDU_COPY;
	template_pdu->time		 	 = uptime;

	snmp_set_var_value( &snmptrap_var,
				    (u_char *)enterprise,
				    (enterprise_length)*sizeof(oid));
	snmptrap_var.next_variable  = vars;
	last_var = NULL;	/* Don't need version info */
    

	template_pdu->version = SNMP_VERSION_2c;
	template_pdu->command = SNMP_MSG_TRAP2;
    	template_pdu->variables = v2_vars;
	if ( last_var )
		last_var->next_variable = &enterprise_var;

	pdu = snmp_clone_pdu( template_pdu );
	pdu->sessid = 0;	
	if ( snmp_trap_pdu_to_packet( pdu, packet,  out_length ) == 0 ) 
	{
		snmp_free_pdu( pdu );
		return  VOS_ERROR;
	}
		
	if ( last_var )
		last_var->next_variable = NULL;
	template_pdu->variables = NULL;
	snmp_free_pdu( template_pdu );
	
	return VOS_OK;
}    

INT32 snmp_trap_pdu_to_packet( struct snmp_pdu * pdu, UCHAR * packet,  size_t *out_length )
{
 	INT32 result;

	if( (pdu == NULL) || (packet == NULL) || (out_length == NULL) )
	{
		VOS_ASSERT(0);
		return 0;
	}

	/*pdu->flags |= UCD_MSG_FLAG_EXPECT_RESPONSE;*/

	/* check/setup the version */
	if ( pdu->version == SNMP_DEFAULT_VERSION )
	{
		pdu->version = SNMP_VERSION_2c;
	}

	if ( pdu->address.sa_family == AF_UNSPEC )
	{
		VOS_MemZero( &pdu->address, sizeof(pdu->address) );
		pdu->address.sa_family = AF_INET;
	}

	result = snmp_trap_packet_build( pdu, packet, out_length );
	if ( result < 0 )
	{
		/*snmp_free_pdu( pdu );
		pdu = NULL;*/
		return 0;
	}
	if ( ds_get_boolean( DS_LIBRARY_ID, DS_LIB_DUMP_PACKET ) )
	{
		sys_console_printf("\r\nxdump packe\r\n");
	}

	snmp_free_pdu( pdu ); 
	pdu = NULL;

	return 1;
}

INT32 snmp_trap_packet_build( struct snmp_pdu * pdu, UCHAR * packet,  size_t * out_length )
{
	UCHAR *cp,*h0e = 0;
	size_t length;
	LONG version;

	UCHAR  *community = (UCHAR*)"public";        
	size_t  community_len = VOS_StrLen(community);   

	pdu->flags &= ( ~UCD_MSG_FLAG_EXPECT_RESPONSE );
	if ( pdu->errstat == SNMP_DEFAULT_ERRSTAT )
		pdu->errstat = 0;
	if ( pdu->errindex == SNMP_DEFAULT_ERRINDEX )
		pdu->errindex = 0;

	/* save length */
	length = *out_length;

	SNMP_FREE( pdu->community );

	pdu->community = ( UCHAR * ) g_malloc( community_len );

	if ( pdu->community == NULL ) 
	{
		ASSERT( 0 );
		return -1;
	}                           /* end */
	memmove( pdu->community, community, community_len );
                
	pdu->community_len = community_len;

	/* Save current location and build SEQUENCE tag and length
            placeholder for SNMP message sequence
            (actual length will be inserted later) */
	cp = asn_build_sequence( packet, out_length,
                          ( UCHAR ) ( ASN_SEQUENCE | ASN_CONSTRUCTOR ),
                          0 );
	if ( cp == NULL )
		return -1;
	h0e = cp;

	/* store the version field */
	version = pdu->version;
	cp = asn_build_int( cp, out_length,
                     ( UCHAR ) ( ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER ),
                     ( LONG * ) & version, sizeof( version ) );
	if ( cp == NULL )
		return -1;

	/* store the community string */
	cp = asn_build_string( cp, out_length,
                        ( UCHAR ) ( ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR ),
                        pdu->community, pdu->community_len );
	if ( cp == NULL )
		return -1;

	cp = snmp_pdu_build( pdu, cp, out_length );
	if ( cp == NULL )
		return -1;

	asn_build_sequence( packet, &length,
                      ( UCHAR ) ( ASN_SEQUENCE | ASN_CONSTRUCTOR ),
                      cp - h0e );

	*out_length = cp - packet;
	
	return 0;
}

/* 测试告警同步 */
int testsyn( int flag)
{
	SNMPTRAP  trapvar;
    	UCHAR packet[ 512 ];
	size_t length = 512;
	size_t i;

	oid baseoid[] = {1,3,6,1,4,1,10072,2,20,1,1,6};
	oid ethDevIdxOid[] = {1,3,6,1,4,1,10072,2,21,2,1,1,1};
	oid ethBrdIdxOid[] = {1,3,6,1,4,1,10072,2,21,2,1,1,2};
	oid ethPortIdxOid[] = {1,3,6,1,4,1,10072,2,21,2,1,1,3};

	const ulong_t baselen = sizeof( baseoid )/sizeof(oid);

	VOS_MemSet( &trapvar, 0, sizeof(trapvar) );
	trapvar.GenericType = SNMP_TRAP_ENTERPRISESPECIFIC;
	trapvar.SpecificType = 0;

	trapvar.VarNum = 3;

	VOS_MemCpy( trapvar.TrapOid, baseoid, sizeof(baseoid));
	trapvar.TrapOid[baselen] = 54;
	trapvar.TrapOidLength = baselen+1;

	VOS_MemCpy( trapvar.VarOid[0], ethDevIdxOid, sizeof( ethDevIdxOid ) );
	trapvar.VarOidLength[0] = sizeof(ethDevIdxOid)/sizeof(oid);
	*(ulong_t*)trapvar.Var[0] = 1;
	trapvar.VarLength[0] = sizeof(ulong_t);
	trapvar.VarType[0] = ASN_INTEGER;

	VOS_MemCpy( trapvar.VarOid[1], ethBrdIdxOid, sizeof( ethBrdIdxOid ) );
	trapvar.VarOidLength[1] = sizeof(ethBrdIdxOid)/sizeof(oid);
	*(ulong_t*)trapvar.Var[1] = 3;
	trapvar.VarLength[1] = sizeof(ulong_t);
	trapvar.VarType[1] = ASN_INTEGER;

	VOS_MemCpy( trapvar.VarOid[2], ethPortIdxOid, sizeof( ethPortIdxOid ) );
	trapvar.VarOidLength[2] = sizeof(ethPortIdxOid)/sizeof(oid);
	*(ulong_t*)trapvar.Var[2] = 22;
	trapvar.VarLength[2] = sizeof(ulong_t);
	trapvar.VarType[2] = ASN_INTEGER;	

	if( flag )
	{
		return mn_send_msg_trap( &trapvar, MSG_PRI_NORMAL );
	}

	if( code_snmp_trap_packet( &trapvar, packet, &length ) == VOS_OK )
	{
		for(i=0;i<length; i++ )
		{
			if( i%20 == 0 )
				sys_console_printf("\r\n");
			sys_console_printf( "%02x ", packet[i] );
		}
		sys_console_printf("\r\n");

		return 0;
	}

	return -1;
}

#ifdef __cplusplus
}
#endif


