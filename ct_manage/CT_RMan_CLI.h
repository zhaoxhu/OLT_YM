
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_RMan_CLIh
#define __INCCT_RMan_CLIh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CTC_OBSOLETE		/* removed by xieshl 20120607 */
#if 0
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

#if 0	/*ifndef CTC_STACK_EXIT_OK*/
#define CTC_STACK_EXIT_OK				            0
#define CTC_STACK_EXIT_ERROR			            -1
#define CTC_STACK_TIME_OUT				            -2 
#define CTC_STACK_NOT_IMPLEMENTED		            -3
#define CTC_STACK_PARAMETER_ERROR		            -4
#define CTC_STACK_HARDWARE_ERROR		            -5
#define CTC_STACK_MEMORY_ERROR			            -6
#define CTC_STACK_QUERY_FAILED			            (-17001)
#define CTC_STACK_ONU_NOT_AVAILABLE		            (-17002)
#define CTC_STACK_OLT_NOT_EXIST			            (-17003)
#define CTC_STACK_EXIT_ERROR_ONU_DBA_THRESHOLDS     (-17004)
#define CTC_STACK_TFTP_SEND_FAIL					(-17005)
#define	CTC_STACK_RETURNED_INCONSISTENT_VALUES		(-17006)
#define	CTC_STACK_ALREADY_EXIST						(-17007)
#define	CTC_STACK_TABLE_FULL						(-17008)
#define	CTC_STACK_VARIABLE_INDICATION_BAD_PARAM		(-17009)
#define	CTC_STACK_VARIABLE_INDICATION_NO_RESOURCE	(-17010)
#endif


#if 0
/* Common descriptions. */
#define SHOW_STR "Show running system information\n"
#define IP_STR "IP information\n"
#define IPV6_STR "IPv6 information\n"
#define NO_STR "Negate a command or set it to its defaults\n"
#define CLEAR_STR "Reset functions\n"
#define RIP_STR "RIP information\n"
#define BGP_STR "BGP information\n"
#define OSPF_STR "OSPF information\n"
#define NEIGHBOR_STR "Specify neighbor router\n"
#define DEBUG_STR "Debugging functions (see also 'undebug')\n"
#define UNDEBUG_STR "Disable debugging functions (see also 'debug')\n"
#define ROUTER_STR "Enable a routing process\n"
#define AS_STR "AS number\n"
#define MBGP_STR "MBGP information\n"
#define MATCH_STR "Match values from routing table\n"
#define SET_STR "Set configuration values\n"
#define OUT_STR "Filter outgoing routing updates\n"
#define IN_STR  "Filter incoming routing updates\n"
#define V4NOTATION_STR "specify by IPv4 address notation(e.g. 0.0.0.0)\n"
#define OSPF6_NUMBER_STR "Specify by number\n"
#define INTERFACE_STR "Interface infomation\n"
#define IFNAME_STR "Interface name(e.g. ep0)\n"
#define IP6_STR "IPv6 Information\n"
#define OSPF6_STR "Open Shortest Path First (OSPF) for IPv6\n"
#define OSPF6_ROUTER_STR "Enable a routing process\n"
#define OSPF6_INSTANCE_STR "<1-65535> Instance ID\n"
#define SECONDS_STR "<1-65535> Seconds\n"
#define ROUTE_STR "Routing Table\n"
#define PREFIX_LIST_STR "Build a prefix list\n"
#define OSPF6_DUMP_TYPE_LIST \
"(neighbor|interface|area|lsa|zebra|config|dbex|spf|route|lsdb|redistribute|hook|asbr|prefix|abr)"
#define ISIS_STR "IS-IS information\n"
#define AREA_TAG_STR "[area tag]\n"
#define CTC_STR "ctc stack operations\n"

#define CONF_BACKUP_EXT ".sav"

#define OLT_STR	       "OLT configuration\n"
#define ONU_STR	       "ONU configuration\n"
#define LLID_STR       "LLID configuration\n"
#define OAM_STR		   "OAM protocol configuration\n"
#define DBA_STR        "DBA configuration mode\n"
#define DBA_INTERNAL_STR "Internal DBA configuration\n"
#define DBA_SOCRATES_STR "Socrates(c) DBA configuration\n"
#define DBA_PLATO_STR "Plato(c) DBA configuration\n"
#define DBA_ARCHIMEDES_STR "Archimedes(c) DBA configuration\n"
#define OLT_INDEX_STR  "OLT index\n"
#define ONU_INDEX_STR  "ONU index\n"
#define LLID_INDEX_STR "LLID index\n"
#define LLID_MAC_STR   "LLID MAC address\n"
#define ENTER_LLID_MAC_ADDR_STR "defining LLID by MAC address\n"
#define ONU_MAC_STR   "ONU MAC address\n"
#define ENTER_ONU_MAC_ADDR_STR "defining ONU by MAC address\n"
#define INIT_STR       "Initalize operation\n"
#define TERM_STR       "Terminate operation\n"
#define SYSTEM_STR	   "Set Passave API system parameters during runtime\n"
#define ALARM_STR      "Alarm event generation thresholds\n"
#define LOG_STR		   "Change the current logging configuration\n"
#define LOG_PAS_SOFT_STR  "PAS-SOFT creates the log messages\n"
#define LOG_FW_STR     "OLT FW create the log messages\n"
#define OPEN_ENCRYPT_STR "Open encryption configuration\n"
#define MARVELL_SWITCH_STR "Marvell switch configuration\n"
/* IPv4 only machine should not accept IPv6 address for peer's IP
   address.  So we replace VTY command string like below. */
#ifdef HAVE_IPV6
#define NEIGHBOR_CMD       "neighbor (A.B.C.D|X:X::X:X) "
#define NO_NEIGHBOR_CMD    "no neighbor (A.B.C.D|X:X::X:X) "
#define NEIGHBOR_ADDR_STR  "Neighbor address\nIPv6 address\n"
#define NEIGHBOR_CMD2      "neighbor (A.B.C.D|X:X::X:X|WORD) "
#define NO_NEIGHBOR_CMD2   "no neighbor (A.B.C.D|X:X::X:X|WORD) "
#define NEIGHBOR_ADDR_STR2 "Neighbor address\nNeighbor IPv6 address\nNeighbor tag\n"
#else
#define NEIGHBOR_CMD       "neighbor A.B.C.D "
#define NO_NEIGHBOR_CMD    "no neighbor A.B.C.D "
#define NEIGHBOR_ADDR_STR  "Neighbor address\n"
#define NEIGHBOR_CMD2      "neighbor (A.B.C.D|WORD) "
#define NO_NEIGHBOR_CMD2   "no neighbor (A.B.C.D|WORD) "
#define NEIGHBOR_ADDR_STR2 "Neighbor address\nNeighbor tag\n"
#endif /* HAVE_IPV6 */

#endif
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_RMan_CLIh */
