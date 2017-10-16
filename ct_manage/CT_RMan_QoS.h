
/*----------------------------------------------------------------------------*/
#ifndef __INCCT_RMan_QoSh
#define __INCCT_RMan_QoSh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*#ifdef __CT_EXTOAM_SUPPORT*/
#if 0

typedef struct {
	UCHAR OUI[3];
	UCHAR extOpcode;
	UCHAR instanceBranch;
	USHORT instanceLeaf;
	UCHAR instanceWidth;
	UCHAR instanceValue;
	UCHAR variableBranch;
	USHORT variableLeaf;
	UCHAR variableWidth;
	UCHAR action;
} __attribute__((packed))  CT_ExtOam_PduData_Classification_Req_t;


#define CT_MAX_CLASS_RULES_NUM		16
#define CT_MAX_CLASS_RULE_ENTRY_NUM	12

typedef struct
{
	UCHAR	field_select;
	UCHAR	match_value[6];
	UCHAR	validation_operator;
} CT_RMan_rule_class_entry;

typedef struct {
	UCHAR  precedence;
	UCHAR  length;
	UCHAR  queue_Mapped;
	UCHAR  ethernet_Priority;
	UCHAR  number_of_entries;
	CT_RMan_rule_class_entry entries[CT_MAX_CLASS_RULE_ENTRY_NUM];
} __attribute__((packed)) CT_RMan_rule_t;

typedef struct {
	UCHAR number_of_rules;
	CT_RMan_rule_t rules[CT_MAX_CLASS_RULES_NUM];
} __attribute__((packed))  CT_RMan_Classification_t;


typedef struct {
	CT_ExtOam_PduData_Classification_Req_t	descriptor;
	CT_RMan_Classification_t				class;
} __attribute__((packed))  CT_ExtOam_PduData_Classification_Resp_t;


typedef enum
{
	CT_Class_Action_Delete = 0x00,
	CT_Class_Action_Add = 0x01,
	CT_Class_Action_Destroy = 0x02,
	CT_Class_Action_List = 0x03
} CT_RMan_Class_Rule_Action_t;

typedef enum
{
	CT_Class_FieldSel_DA_MAC			= 0x00,
	CT_Class_FieldSel_SA_MAC			= 0x01,
	CT_Class_FieldSel_Pri				= 0x02,
	CT_Class_FieldSel_VlanId			= 0x03,
	CT_Class_FieldSel_Eth_Type			= 0x04,
	CT_Class_FieldSel_Dest_IP			= 0x05,
	CT_Class_FieldSel_Src_IP			= 0x06,
	CT_Class_FieldSel_IP_Type			= 0x07,
	CT_Class_FieldSel_IP4_TosDscp		= 0x08,
	CT_Class_FieldSel_IP6_Precedence	= 0x09,
	CT_Class_FieldSel_L4_SrcPort		= 0x0a,
	CT_Class_FieldSel_L4_DestPort		= 0x0b,
	CT_Class_FieldSel_Other
} CT_RMan_Class_Field_Select_t;

typedef enum
{
	CT_Validation_Oper_NeverMatch		= 0x00,
	CT_Validation_Oper_Equal			= 0x01,
	CT_Validation_Oper_NotEqual			= 0x02,
	CT_Validation_Oper_LessOrEqual		= 0x03,
	CT_Validation_Oper_GreateOrEqual	= 0x04,
	CT_Validation_Oper_FieldExist		= 0x05,
	CT_Validation_Oper_FieldNotExist		= 0x06,
	CT_Validation_Oper_AlwaysMatch		= 0x07,
	CT_Validation_Oper_Other
} CT_RMan_validation_operators_t;

extern int CT_RMan_Classification_Marking_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_Classification_t* pClass );
extern int CT_RMan_Classification_Marking_del( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_Classification_t* pClass );
extern int CT_RMan_Classification_Marking_add( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_Classification_t* pClass );

#endif /*__CT_EXTOAM_SUPPORT*/



#ifndef CTC_MAX_CLASS_RULES_COUNT

#if 0  /*marked by wangxy 2007-05-23*/
#define CTC_MAX_CLASS_RULES_COUNT 31
#define CTC_MAX_CLASS_RULE_PAIRS 12	/* (128 - 8 (other fields))/(10 (size of a threesome))  */
#endif
#define CTC_MAX_CLASS_RULES_COUNT /*10*/32
#define CTC_MAX_CLASS_RULE_PAIRS 4

typedef enum
{
	CTC_CLASS_ACTION_DELETE_RULE	= 0x00,
	CTC_CLASS_ACTION_ADD_RULE		= 0x01,
	CTC_CLASS_ACTION_DELETE_LIST	= 0x02,
	CTC_CLASS_ACTION_LIST			= 0x03,
} CTC_STACK_classification_rule_action_t;

typedef enum
{
	CTC_FIELD_SEL_DA_MAC				= 0x00,
	CTC_FIELD_SEL_SA_MAC				= 0x01,
	CTC_FIELD_SEL_ETHERNET_PRIORITY		= 0x02,
	CTC_FIELD_SEL_VLAN_ID				= 0x03,
	CTC_FIELD_SEL_ETHER_TYPE			= 0x04,
	CTC_FIELD_SEL_DEST_IP				= 0x05,
	CTC_FIELD_SEL_SRC_IP				= 0x06,
	CTC_FIELD_SEL_IP_PROTOCOL_TYPE		= 0x07,
	CTC_FIELD_SEL_IPV4_TOS_DSCP			= 0x08,
	CTC_FIELD_SEL_IPV6_TRAFFIC_CLASS		= 0x09,
	CTC_FIELD_SEL_L4_SRC_PORT			= 0x0A,
	CTC_FIELD_SEL_L4_DEST_PORT			= 0x0B,
	CTC_FIELD_SEL_IP_VERSION			= 0x0C,
	CTC_FIELD_SEL_IP_FLOW_LABLE		= 0x0D,
	CTC_FIELD_SEL_DEST_IPV6			= 0x0E,
	CTC_FIELD_SEL_SRC_IPV6			= 0x0F,
	CTC_FIELD_SEL_DEST_IPV6_PREFIX		= 0x10,
	CTC_FIELD_SEL_SRC_IPV6_PREFIX		= 0x11
} CTC_STACK_field_selection_t;

typedef enum
{
	CTC_VALIDATION_OPERS_NEVER_MATCH			= 0x00,
	CTC_VALIDATION_OPERS_EQUAL					= 0x01,
	CTC_VALIDATION_OPERS_NOT_EQUAL				= 0x02,
	CTC_VALIDATION_OPERS_LESS_THAN_OR_EQUAL		= 0x03,
	CTC_VALIDATION_OPERS_GREATER_THAN_OR_EQUAL	= 0x04,
	CTC_VALIDATION_OPERS_FIELD_EXIST			= 0x05,
	CTC_VALIDATION_OPERS_FIELD_NOT_EXIST		= 0x06,
	CTC_VALIDATION_OPERS_ALWAYS_MATCH			= 0x07,
} CTC_STACK_validation_operators_t;



typedef struct
{
	unsigned long		match_value;
	mac_address_t	mac_address;
	PON_ipv6_addr_t	ipv6_match_value;	/*match value for IPv6 address*/
}CTC_STACK_value_t;

typedef struct
{
	CTC_STACK_field_selection_t			field_select;
	CTC_STACK_value_t					value;
	CTC_STACK_validation_operators_t	validation_operator;
} CTC_STACK_class_rule_entry;

typedef struct
{
	unsigned char			valid;
	unsigned char							queue_mapped;
	unsigned char							priority_mark;
	unsigned char							num_of_entries;
	CTC_STACK_class_rule_entry				entries[CTC_MAX_CLASS_RULE_PAIRS];
} CTC_STACK_classification_rule_t;


typedef  CTC_STACK_classification_rule_t CTC_STACK_classification_rules_t[CTC_MAX_CLASS_RULES_COUNT];

typedef struct
{
	CTC_STACK_classification_rules_t rules;

	CTC_STACK_classification_rule_action_t	action;

} CTC_STACK_classification_and_marking_t;	/* NOTE: This may exceed the standard size of 128bytes, but the frame composer blocks this with error. */



typedef enum
{
	CTC_CLASSIFICATION_DELETE_RULE	=	CTC_CLASS_ACTION_DELETE_RULE,
	CTC_CLASSIFICATION_ADD_RULE		=	CTC_CLASS_ACTION_ADD_RULE,
} CTC_STACK_classification_rule_mode_t;


typedef struct
{
	unsigned char						port_number;
	CTC_STACK_classification_rules_t		entry;
} CTC_STACK_classification_port_entry_t;

typedef  CTC_STACK_classification_port_entry_t CTC_STACK_classification_ports_t[MAX_PORT_ENTRIES_NUMBER];



extern PON_STATUS CTC_STACK_set_classification_and_marking ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			const CTC_STACK_classification_rule_mode_t   mode,
			const CTC_STACK_classification_rules_t		classification_and_marking);
extern PON_STATUS CTC_STACK_get_classification_and_marking ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			const unsigned char	port_number,
			CTC_STACK_classification_rules_t   classification_and_marking);
extern PON_STATUS CTC_STACK_get_classification_and_marking_all_port ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t	onu_id,
			unsigned char			*number_of_entries,
			CTC_STACK_classification_ports_t   ports_info );
extern PON_STATUS CTC_STACK_delete_classification_and_marking_list ( 
			const PON_olt_id_t		olt_id, 
			const PON_onu_id_t 	onu_id,
			const unsigned char   port_number);
#endif /*CTC_MAX_CLASS_RULES_COUNT*/



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCCT_RMan_QoSh */
