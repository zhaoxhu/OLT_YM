/*
 * onuConfMgt.h
 *
 *  Created on: 2011-6-1
 *      Author: wangxy
 */

#ifndef ONUCONFMGT_H_
#define ONUCONFMGT_H_

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#define ONUID_MAP

ULONG getOnuConfFileCounter();

/*ϵͳ���֧�ֵ������ļ�����*/
#define ONU_MAX_CONF_NUM_USE    ((sysPhysMemTop() <= 0x10000000)?500:2000)
#define ONU_RESERVE_CONF_NUM    50
#define ONU_MAX_SIZE_FILE   (15*1024)
#define ONU_MAX_CONF_SIZE   (ONU_MAX_CONF_NUM_USE*ONU_MAX_SIZE_FILE)
#define ONU_MAX_CONF_NUM (ONU_MAX_CONF_NUM_USE+ONU_RESERVE_CONF_NUM)

#define ONU_MAX_DATABUF_SIZE	(((ONU_MAX_CONF_SIZE/3)>0x600000)?0x600000:(ONU_MAX_CONF_SIZE/3)) /*���ݱ��桢�ָ�ʱ��������ڴ�*/
/*#define ONU_MAX_DATABUF_SIZE (getOnuConfFileCounter()<=100?(getOnuConfFileCounter()*ONU_MAX_SIZE_FILE):(100*ONU_MAX_SIZE_FILE))*/

#define ONU_MAX_PORT 32

#define ONU_MAX_VLAN 64
#define ONU_MAX_VLAN_TRANS 15
#define ONU_MAX_VLAN_AGG_GROUP 4
#define ONU_MAX_VLAN_AGG_ENTRY 8
#define ONU_MAX_VLAN_TRUNK_ENTRY 31
#define ONU_MAX_QINQ_GROUP  16

#define ONU_MAX_IGMP_GROUP  4
#define ONU_MAX_IGMP_VLAN 50
#define ONU_MAX_IGMP_VLAN_SWITCH_GROUP 8

/*ONU����ģ�������Ϣ�����������*/
#define ONU_CONF_DEBUG_LVL_GENERAL 0
#define ONU_CONF_DEBUG_LVL_CDP  1

#define ONU_SEPCAL_FUNCTION 0x10000000
#define ONU_CONF_CARD_SYNC_WAIT_TIME	600/*180*/  /*modi by luh, pon�� �����ļ��ָ���ʱ�ȴ�ʱ��10����*/

#define DEFAULT_ONU_CONF "onuconfdef"
#define DEFAULT_GWD_ONU_CONF    "gwdshare"

#ifdef ONUID_MAP
#define ONU_CONF_SAVE_FILE_HEAD "/flash/sys/onuconf.data"
#else
#define ONU_CONF_SAVE_FILE_HEAD "/flash/sys/onuconfdata_"
#endif

/*�ڴ�����������Ͷ��壬�����ڴ����롢�ͷ�*/
enum{
    ONU_CONF_MEM_DATA_ID,
    ONU_CONF_MEM_HASH_ID,
    ONU_CONF_MEM_MAX_ID
};

enum{
    MODE_CTC_SAVE_CONF_NORMAL,                      
    MODE_GW_SAVE_CONF_NORMAL,       
    MODE_GW_SAVE_CONF_WITHOUT_NOTICE,        
    MODE_GW_SAVE_NONE
};
enum{
    ONU_NODE_EXIT_MODE_SAVE_NOTICE, /*��ʾ�û������ļ����޸ģ�ѡ���Ƿ񱣴����ü�ӳ���ϵ*/
    ONU_NODE_EXIT_MODE_SAVE_WITHOUT_NOTICE, /*����˽�������ļ���ӳ���ϵ���˳�ONU�ڵ㣬����ʾ�û�*/
    ONU_NODE_EXIT_MODE_SAVE_NONE, /*�²�����˽�������ļ���ԭ���Ĺ����ļ��޲�ͬ���˳�ONU�ڵ�ʱɾ��˽���ļ����ָ�ԭ�������ļ���ӳ���ϵ*/
    ONU_NODE_EXIT_MODE_NOOP /*û�к���������ֱ���˳�ONU�ڵ�*/
};
enum{
    ONU_CONF_IFS_TYPE_LOCALE,
    ONU_CONF_IFS_TYPE_REMOTE,
    ONU_CONF_IFS_TYPE_MAX
};


enum{
    ONU_CONF_GWD_TYPE,
    ONU_CONF_CTC_TYPE
};

enum{
    ONU_CONF_VLAN_MODE_TRANSPARENT,
    ONU_CONF_VLAN_MODE_TAG,
    ONU_CONF_VLAN_MODE_TRANSLATION,
    ONU_CONF_VLAN_MODE_AGG,
    ONU_CONF_VLAN_MODE_TRUNK,
    ONU_CONF_VLAN_MODE_MIXED,
    ONU_CONF_VLAN_MODE_UNKNOWN
};


enum
{
    CTC_QINQ_MODE_NONE_C,        
    CTC_QINQ_MODE_PER_PORT_C,    
    CTC_QINQ_MODE_PER_VLAN_C    
} ;

enum{
	ONU_CONF_PORT_INGRESS_ACT_NONE,
    ONU_CONF_PORT_INGRESS_ACT_DROP,
    ONU_CONF_PORT_INGRESS_ACT_PAUSE
};

enum{
	ONU_CONF_PORT_INGRESS_BURST_NONE,
    ONU_CONF_PORT_INGRESS_BURST_12K,
    ONU_CONF_PORT_INGRESS_BURST_24K,
    ONU_CONF_PORT_INGRESS_BURST_48K,
    ONU_CONF_PORT_INGRESS_BURST_96K
};

#define ONU_CONF_GWD_VLAN_ENABLE ONU_CONF_VLAN_MODE_TRUNK
#define ONU_CONF_GWD_VLAN_DISABLE ONU_CONF_VLAN_MODE_TRANSPARENT

/*�����ļ�������¼��*/
typedef enum{
    EM_ONU_PROFILE_CREATED,
    EM_ONU_PROFILE_MODIFIED,
    EM_ONU_PROFILE_DELETED,
    EM_ONU_PROFILE_INVALID
}onu_profile_op_code_t;

/*�����ļ�������¼*/
typedef struct{
    char filename[ONU_CONFIG_NAME_LEN];
    onu_profile_op_code_t op;
}onu_profile_op_record_t;

typedef struct{
	int * data;
	int number;
	int growth;
	int maxnum;
	int (*add)(void *, void *element);
	int (*removeat)(void *, int);
	int (*removeall)(void *);
	void* (*getat)(void*, int);
	int (*count)(void *);
}dataarray_t;

#if 0
#define data_array_getat(x, y) (x).getat((&(x)), (y))
#define data_array_removeat(x,y) (x).removeat((&(x)), (y))
#define data_array_add(x,y) (x).add((&(x)), (y))
#define data_array_removeall(x) (x).removeall((&x))
#define data_array_count(x) (x).count((&(x)))
#else
#define data_array_getat(x, y) (x)->getat((x), (y))
#define data_array_removeat(x,y) (x)->removeat((x), (y))
#define data_array_add(x,y) (x)->add((x), (y))
#define data_array_removeall(x) (x)->removeall((x))
#define data_array_count(x) (x)->count((x))
#endif

typedef struct onuConfMacHashEntry{
    char mac[6];
    char name[ONU_CONFIG_NAME_LEN];
}onuConfMacHashEntry_t;

/*
 * onu id�������ļ���ӳ���ϵԪ������
 * name: ���������ļ�����
 */
typedef struct onuConfOnuIdMapEntry{
    char name[ONU_CONFIG_NAME_LEN];
    UCHAR configResult;
    UCHAR configRestoreFlag;
    UCHAR undoconfigFlag;
}onuConfOnuIdMapEntry_t;

typedef struct ONUVlanEntry{
    short int vlanid;
    ULONG allPortMask;
    ULONG untagPortMask;
}PACKED ONUVlanEntry_t;

typedef ONUVlanEntry_t ONUVlanEntryArry_t[ONU_MAX_VLAN];

typedef struct ONUVlanTransEntry{
    short int ovid;
    short int svid;
}ONUVlanTransEntry_t;

typedef ONUVlanTransEntry_t ONUVlanTransEntryArry_t[ONU_MAX_VLAN_TRANS];
typedef short int ONUVlanAggEntryArry_t[ONU_MAX_VLAN_AGG_GROUP][ONU_MAX_VLAN_AGG_ENTRY];
typedef short int ONUVlanTrunkEntryArry_t [ONU_MAX_VLAN_TRUNK_ENTRY];

typedef union {
    ONUVlanTransEntryArry_t transArry;
    ONUVlanAggEntryArry_t aggArry;
    ONUVlanTrunkEntryArry_t trunkArry;
}PACKED ONUVlanExtEntry_t;

typedef struct ONUVlanExtConf{
    UCHAR numberOfEntry;
    UCHAR numberOfAggTables;
    USHORT targetVlanId[ONU_MAX_VLAN_AGG_GROUP];
    USHORT onuVlanAggEntryNum[ONU_MAX_VLAN_AGG_GROUP];
    ONUVlanExtEntry_t extEntry;
}PACKED ONUVlanExtConf_t;

typedef struct ONUVlanConf{

    UCHAR portIsolateEnable;

    UCHAR onuVlanEntryNum;

    ULONG vmtransparentportmask;
    ULONG vmtranslationportmask;
    ULONG vmtagportmask;
    ULONG vmtrunkportmask;
    ULONG vmaggportmask;

    ONUVlanEntryArry_t entryarry;

}PACKED ONUVlanConf_t;

typedef struct ONUQinqEntry{
    UCHAR qinqMode;
    USHORT qinqEntries[ONU_MAX_QINQ_GROUP];
    UCHAR   numberOfEntry;/*add by shixh20100712*/
}PACKED ONUQinqEntry_t;

typedef struct ONUIgmpEntry{
    ULONG gda;
    ULONG multicastVlanId;
    ULONG mapPortMask;
}PACKED ONUIgmpEntry_t;

typedef ONUIgmpEntry_t ONUIgmpEntryArry_t[ONU_MAX_IGMP_GROUP];

typedef enum{
    sv_enum_atu_aging,
    sv_enum_atu_limit,
    sv_enum_igmp_enable,
    sv_enum_igmp_auth_enable,
    sv_enum_igmp_hostage,
    sv_enum_igmp_groupage,
    sv_enum_igmp_max_response_time,
    sv_enum_igmp_max_group,
    sv_enum_port_isolate,
    sv_enum_port_atu_learn_enable,
    sv_enum_port_atu_flood_enable,
    sv_enum_port_enable,
    sv_enum_port_mode,
    sv_enum_port_fec_enable,
    sv_enum_port_linkmon_enable,
    sv_enum_port_modemon_enable,
    sv_enum_port_loop_detect_enable,
    sv_enum_port_ingress_rate_type,
    sv_enum_port_ingress_rate_limit,
    sv_enum_port_egress_limit,
    sv_enum_port_ingress_rate_action,
    sv_enum_port_ingress_rate_burst,
    sv_enum_port_default_vid,
    sv_enum_port_default_priority,
    sv_enum_port_vlan_accept_type,
    sv_enum_port_ingress_vlan_filter,
    sv_enum_port_igmp_fastleave_enable,
    sv_enum_port_igmp_max_group,
    sv_enum_port_igmp_tag_strip,
    sv_enum_port_qoset_idx,
    sv_enum_port_qoset_ip_enable,
    sv_enum_port_qoset_user_enable,
    sv_enum_port_qoset_rule,
    sv_enum_port_pause_enable,
    sv_enum_port_mirror_ingress_fromlist,
    sv_enum_port_mirror_ingress_tolist,
    sv_enum_port_mirror_egress_fromlist,
    sv_enum_port_mirror_egress_tolist,
    sv_enum_igmp_group_gda,
    sv_enum_igmp_group_vlanId,
    sv_enum_igmp_group_portmask,
    sv_enum_igmp_fastleave_enable,
    sv_enum_VlanEntryNum,
    sv_enum_qosAlgorithm,
    sv_enum_ingressRateLimitBase,
    sv_enum_onu_holdover_time,
    sv_enum_onu_transparent_enable,    
    sv_enum_onu_fec_enable,
#if (EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES)
    sv_enum_cmc_max_cm,
    sv_enum_cmc_channel_enable,
    sv_enum_cmc_channel_annex,
    sv_enum_cmc_channel_d30,
    sv_enum_cmc_channel_type,
    sv_enum_cmc_channel_profile,
    sv_enum_cmc_channel_modulation,
    sv_enum_cmc_channel_interleave,
    sv_enum_cmc_channel_power,
    sv_enum_cmc_channel_width,
    sv_enum_cmc_channel_freq,
#endif
    sv_enum_max_num

}simple_var_enum_t;

#define CMC_CHANNEL_DIR_NONE              0
#define CMC_CHANNEL_DIR_DOWNLINK          1
#define CMC_CHANNEL_DIR_UPLINK            2
#define CMC_CHANNEL_DIR_BOTH              3

#define CMC_CHANNELID_NONE                0xFF
#define CMC_CHANNELID_UP                  0xFF00
#define CMC_CHANNELID_DOWN                0xFF
#define CMC_UPCHANNEL2PORT(channel_id)    (((channel_id) << 8) | 0xFF)
#define CMC_DOWNCHANNEL2PORT(channel_id)  (0xFF00 | (channel_id))
#define CMC_PORT2UPCHANNEL(port_id)       ((port_id) >> 8)
#define CMC_PORT2DOWNCHANNEL(port_id)     ((port_id) & CMC_CHANNELID_DOWN)

typedef struct ONUPortConf{

    UCHAR atuLearnEnable;
    UCHAR atuFloodEnable;
    UCHAR enable;
    UCHAR mode;
    UCHAR fecEnable;
    UCHAR loopDetectEnable;
    UCHAR pauseEnable;

    UCHAR ingressRateType;
    int ingressRateLimit;
    int egressRateLimit;
    UCHAR ingressRateAction;
    UCHAR ingressBurstMode;


    int defaultVid;
    UCHAR vlanTransparentEnable;
    UCHAR vlanAcceptFrameType;
    UCHAR vlanIngressFilter;
    /*int vlanMode;*/

    UCHAR igmpFastLeaveEnable;
    UCHAR igmpMaxGroup;
    UCHAR igmpTagStrip;
    USHORT igmpVlan[ONU_MAX_IGMP_VLAN];
    USHORT igmpVlanNum;

    ONUVlanExtConf_t extVlanConf;
    ONUQinqEntry_t qinqEntry;

    UCHAR qosSetSel;
    UCHAR qosRule;
    UCHAR qosIpEnable;
    UCHAR qosUserEnable;
    UCHAR qosPrioReplace[8];

}PACKED ONUPortConf_t;

typedef ONUPortConf_t ONUPortsConf_t[ONU_MAX_PORT];

/*
 * QoS�������ݵ��������
 */

#define IPV6_ADDRESS_SIZE  4
typedef union {
    ULONG addr_32[IPV6_ADDRESS_SIZE];
    USHORT addr_16[IPV6_ADDRESS_SIZE<<1];
    UCHAR  addr_8 [IPV6_ADDRESS_SIZE<<2];
} ipv6_addr_t;

#define MAX_CLASS_RULES_COUNT 32
#define MAX_CLASS_RULE_PAIRS 4

typedef enum
{
    FIELD_SEL_DA_MAC                = 0x00,
    FIELD_SEL_SA_MAC                = 0x01,
    FIELD_SEL_ETHERNET_PRIORITY     = 0x02,
    FIELD_SEL_VLAN_ID               = 0x03,
    FIELD_SEL_ETHER_TYPE            = 0x04,
    FIELD_SEL_DEST_IP               = 0x05,
    FIELD_SEL_SRC_IP                = 0x06,
    FIELD_SEL_IP_PROTOCOL_TYPE      = 0x07,
    FIELD_SEL_IPV4_TOS_DSCP         = 0x08,
    FIELD_SEL_IPV6_TRAFFIC_CLASS        = 0x09,
    FIELD_SEL_L4_SRC_PORT           = 0x0A,
    FIELD_SEL_L4_DEST_PORT          = 0x0B,
    FIELD_SEL_IP_VERSION            = 0x0C,
    FIELD_SEL_IP_FLOW_LABLE     = 0x0D,
    FIELD_SEL_DEST_IPV6         = 0x0E,
    FIELD_SEL_SRC_IPV6          = 0x0F,
    FIELD_SEL_DEST_IPV6_PREFIX      = 0x10,
    FIELD_SEL_SRC_IPV6_PREFIX       = 0x11
} qos_field_selection_t;

typedef enum
{
    VALIDATION_OPERS_NEVER_MATCH            = 0x00,
    VALIDATION_OPERS_EQUAL                  = 0x01,
    VALIDATION_OPERS_NOT_EQUAL              = 0x02,
    VALIDATION_OPERS_LESS_THAN_OR_EQUAL     = 0x03,
    VALIDATION_OPERS_GREATER_THAN_OR_EQUAL  = 0x04,
    VALIDATION_OPERS_FIELD_EXIST            = 0x05,
    VALIDATION_OPERS_FIELD_NOT_EXIST        = 0x06,
    VALIDATION_OPERS_ALWAYS_MATCH           = 0x07,
} qos_validation_operators_t;

enum{
    QOS_ERR_NO_DATA = -1,
    QOS_ERR_OK,
    QOS_ERR_PARAM
};

typedef union
{
    unsigned long       match_value;
    mac_address_t   mac_address;
    PON_ipv6_addr_t ipv6_match_value;   /*match value for IPv6 address*/
}PACKED qos_value_t;

typedef struct
{
    qos_field_selection_t         field_select;
    qos_value_t                   value;
    qos_validation_operators_t    validation_operator;
}PACKED qos_class_rule_entry;

#define QOS_MAX_CLASS_RULE_PAIRS    CTC_MAX_CLASS_RULE_PAIRS
#define QOS_MAX_RULE_COUNT_PER_SET  8
#define QOS_MAX_SET_COUNT           4

typedef struct
{
    BOOL                                    valid;
    unsigned char                           queue_mapped;
    unsigned char                           priority_mark;
    unsigned char                           num_of_entries;
    unsigned char                           entrymask;
    qos_class_rule_entry              		entries[QOS_MAX_CLASS_RULE_PAIRS];
}PACKED qos_classification_rule_t;

typedef  qos_classification_rule_t qos_classification_rules_t[QOS_MAX_RULE_COUNT_PER_SET];
/*
 * QoS��������
 */
typedef struct OnuConfStacticMacData{
     mac_address_t   mac_address;
     ULONG portlist;
     ULONG vid;
     char Mac_priority;
}OnuConfStactic_Mac_Data;
typedef OnuConfStactic_Mac_Data OnuConfStactic_Mac_Data_t[8];

typedef struct onu_qos_map
{
    UCHAR qosDscpQueue[8][8];
    UCHAR queue[8];
}onu_qos_map_t;

#define ONU_VLAN_RULE 16
#define VLAN_BAT_MODE_VLANID 1
#define VLAN_BAT_MODE_VLANID_PORT 2
#define VLAN_BAT_MODE_VLANRANGE 3
#define VLAN_BAT_MODE_VLANRANGE_PORT 4
#define VLAN_BAT_MODE_VLANRANGE_ONEPORT 5
typedef struct
{
	ULONG begin_onuid;
	ULONG end_onuid;
	ULONG begin_vid;
	ULONG end_vid;
	ULONG portlist;
	USHORT step; /*step between two close onu*/
    USHORT port_step;
	USHORT mode;/*1-tag 2-untagged*/
	USHORT rule_mode;
}onu_vlan_bat_rule_t;
typedef struct
{
    onu_vlan_bat_rule_t rule[ONU_VLAN_RULE];
    char number_rules;
}onu_vlan_bat_rules_t;


/* B--added by liwei056@2013-5-15 for CMC's ConfigAsCTC */
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
#define CMC_DSCHANNEL_NUM 16
#define CMC_USCHANNEL_NUM 4

typedef struct ONUCmcConf
{
    UCHAR cmcIsEnabled;
    UCHAR maxCm;
    
    UCHAR aucDsChannelEnable[2];
    UCHAR aucUsChannelEnable[1];
    UCHAR aucUsChannelD30[1];
    UCHAR aucUsChannelType[1];
    UCHAR aucDsChannelAnnex[2];
    UCHAR aucUsChannelProfile[2];
    UCHAR aucDsChannelModulation[4];
    UCHAR aucDsChannelInterleaver[8];

    ULONG aulDsChannelFreq[CMC_DSCHANNEL_NUM];
    ULONG aulUsChannelFreq[CMC_USCHANNEL_NUM];
    ULONG aulUsChannelFreqWidth[CMC_USCHANNEL_NUM];

    SHORT asUsChannelPower[CMC_USCHANNEL_NUM];
    SHORT asDsChannelPower[CMC_DSCHANNEL_NUM];
}PACKED ONUCmcConf_t;
#endif
/* E--added by liwei056@2013-5-15 for CMC's ConfigAsCTC */


typedef struct ONUConfigData{

    ULONG owner; /*��ǰ���ļ�������ID*/
    char confname[ONU_CONFIG_NAME_LEN];
    UCHAR share;
    UCHAR onuType;
    UCHAR dirty;    /*1:lock;0:unlock*/
    UCHAR onulist[SYS_MAX_PON_PORTNUM][MAXONUPERPONNOLIMIT/8+1];

    UCHAR reserved; /*�����ֽڣ�Ҳ��Ϊ�ļ����ݵ�ͷ*/

    int atuLimit;
    int atuAging;
    UCHAR linkMonEnable;
    UCHAR modeMonEnable;

    USHORT holdover;
    UCHAR fecenable;

    ONUPortsConf_t portconf;
    ONUVlanConf_t vlanconf;

    int ingressMirrorFromList;
    int egressMirrorFromList;
    int ingressMirrorToList;
    int egressMirrorToList;

    UCHAR igmpEnable;
    UCHAR igmpAuthEnable;
    UCHAR igmpFastLeaveEnable;
    int igmpHostAgeTime;
    int igmpGroupAgeTime;
    int igmpMaxResponseTime;
    USHORT igmpMaxGroup;

    onu_qos_map_t qosMap;
    UCHAR qosAlgorithm;
    UCHAR ingressRateLimitBase;
    ONUIgmpEntryArry_t igmpGroupArry;
    OnuConfStactic_Mac_Data_t stacticeMacData;
    qos_classification_rules_t qosset[QOS_MAX_SET_COUNT];
	onu_vlan_bat_rules_t rules;
    
    gw_qos_classification_rules_t qos_rules[QOS_MAX_RULE_COUNT_PATH];
    gw_pas_rules_t pas_rules[PAS_MAX_RULE_COUNT_PATH];
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
    ONUCmcConf_t cmcCfg;
#endif
}PACKED ONUConfigData_t;

typedef struct onuConfMemItem{
char used;
ONUConfigData_t data;
}onuConfDataItem_t;

#if 0
typedef struct{
    ctss_onu_set_long_f       SetAtuAgingTime;
    ctss_onu_set_long_f       SetAtuLimit;

    ctss_onu_set_long_f       SetIgmpEnable;
    ctss_onu_set_long_f       SetIgmpGroupAge;	
    ctss_onu_set_long_f       SetIgmpHostAge;
    ctss_onu_set_long_f       SetIgmpMaxResTime;
    ctss_onu_set_long_f       SetIgmpMaxGroup;

    ctss_onu_set_long_f       SetPortLinkMon;
    ctss_onu_set_long_f       SetPortModeMon;
    ctss_onu_set_long_f       SetPortIsolate;

    ctss_onu_set_long_f       SetVlanEnable;
    ctss_onu_set_long_f       SetVlanAdd;
    ctss_onu_set_vlan_port_add_f    SetVlanPortAdd;
    ctss_onu_set_vlan_port_del_f    SetVlanPortDel;
     ctss_onu_set_vlan_transation_f         SetEthPortVlanTranNewVid;
    ctss_onu_set_vlan_transation_del_f   SetEthPortVlanDelTranNewVid;
    ctss_onu_set_vlan_agg_f                 SetEthPortVlanAgg;
    ctss_onu_set_vlan_agg_del_f           SetEthPortVlanDelAgg;
    
    ctss_onu_set_vlan_qinq_enable_f          SetPortQinQEnable;
    ctss_onu_set_vlan_qinq_add_f              SetPortQinQvlantagadd;
    ctss_onu_set_vlan_qinq_del_f              SetPortQinQvlantagdel;
    
    ctss_onu_set_long_f       SetVlanDel;
    ctss_onu_set_port_long_f    SetVlanMode;
    ctss_onu_set_port_long_f    SetPortPvid;

    ctss_onu_set_portmirror_long_f    SetPortmirrorfrom;/*new added by luh     2011.7.13*/
    ctss_onu_set_port_long_f    SetPortmirrorto;
    ctss_onu_set_long_f         DeletePortmirrorto;
    ctss_onu_add_igmpgroup_f    SetPortigmpgroupadd;
    ctss_onu_delete_igmpgroup_f SetPortigmpgroupdelete;/*new added by luh       2011.7.13*/
    ctss_onu_get_igmpgroup_f    GetPortigmpgroup;
    ctss_onu_set_port_long_f    SetPortEnable;
    ctss_onu_set_port_long_f    SetPortFcMode;
    ctss_onu_set_port_long_f    SetPortMode;
    ctss_onu_set_port_long_f    SetPortLoopDetect;
    ctss_onu_set_port_long_f    SetPortStatisticFluash;
    ctss_onu_set_port_long_f    SetPortAtuLearn;
    ctss_onu_set_port_long_f    SetPortAtuFlood;

    ctss_onu_set_port_long_f    SetPortFrameTypeAccess;
    ctss_onu_set_port_long_f    SetPortIngressVlanFilter;

/*    ctss_onu_set_port_long_f    SetPortIngressRate;*/
	ctss_onu_set_port_ingress_rate_f SetPortIngressRate;
    ctss_onu_set_port_long_f    SetPortEgressRate;
    ctss_onu_set_port_long_f	SetPortAnRestart;
    ctss_onu_set_port_long_f	SetPortPauseEnable;

    ctss_onu_set_long_f        SetPasFlush;
    ctss_onu_set_long_f        SetTemperature;
    ctss_onu_set_long_f        SetMgtConfig;
    ctss_onu_set_long_f        SetMgtReset;
    ctss_onu_set_long_f        SetMgtLaser;

    ctss_onu_set_port_long_f   SetPortDefPrio;

    ctss_onu_set_qos_rule_f     SetQosRule;
    ctss_onu_set_qos_class_f    SetQosClass;
    ctss_onu_clr_qos_rule_f     ClrQosRule;
    ctss_onu_clr_qos_class_f    ClrQosClass;

    ctss_onu_set_port_long_f    SetQosSel;
    ctss_onu_set_port_long_f    ClrQosSel;
    ctss_onu_set_long_f         SetIngressRateLimitBase;
    ctss_onu_qos_prioreplace_f  SetQosReplace;
    ctss_onu_set_vlan_port_del_f SetQosPrioToQueue;
    ctss_onu_set_vlan_port_del_f SetQosDscpToQueue;
    ctss_onu_set_port_long_f    SetPortQosetRule;
    ctss_onu_set_port_long_f    SetPortQosetIpEnable;
    ctss_onu_set_port_long_f    SetPortQosetUserEnable;
    ctss_onu_set_long_f         SetqosAlgorithm;
    ctss_onu_set_port_long_f    SetPortMulticastVlan;
    ctss_onu_set_port_long_f    SetPortIgmpTagStrip;
    ctss_onu_set_port_long_f    SetPortIgmpGroupNum;
    ctss_onu_set_long_f         SetIgmpAuthEnable;
    ctss_onu_set_port_long_f	SetMulticastFastLeave;

    ctss_onu_set_port_statistic_state_f SetPortStatisticState;
    
    ctss_onu_get_func_f       GetFunction;
}onu_conf_mgt_if_t;
#endif

/*ONU�����ļ��򿪱�ʶ*/
typedef enum{
    ONU_CONF_OPEN_FLAG_READ,
    ONU_CONF_OPEN_FLAG_WRITE,
}onu_conf_op_flag_t;

/*onu���ûָ�������ʶ
 * undo����ָ���ļ������������ɾ�����ļ���ONU�ϵ�����
 * do:��ָ���ļ��ļ���������������ûָ�
 * noop:��ָ��*/
typedef enum{
    ONU_CONF_RES_ACTION_UNDO,
    ONU_CONF_RES_ACTION_DO,
    ONU_CONF_RES_ACTION_NOOP
}onu_conf_res_act_t;

typedef enum
{
    OnuProfile_code_min = 0,
    OnuProfile_Add,
    OnuProfile_Add_SyncUnicast,    
    OnuProfile_Add_SyncBroadcast,
    OnuProfile_Delete,
    OnuProfile_Delete_SyncUnicast,
    OnuProfile_Delete_SyncBroadcast,
    OnuProfile_Modify,
    OnuProfile_Modify_SyncUnicast,    
    OnuProfile_Modify_SyncBroadcast,
    OnuMap_Update,
    OnuMap_Update_SyncUnicast,
    OnuMap_Update_SyncBroadcast,
    OnuAction_Do_AssociateAndSync,    
    OnuAction_Undo_AssociateAndSync,
    OnuProfile_code_max,
}g_OnuProfile_action_code_t;

typedef enum
{
    OnuProfile_Part_min = 0,
    OnuProfile_Part_CTC,
    OnuProfile_Part_CMC,
    OnuProfile_Part_ALL,
    OnuProfile_Part_max,
}g_OnuProfile_part_code_t;

/*ONU���ûָ����г�Ա����*/
typedef struct{
    short int pon_id;
    short int onu_id;
    char mac[6];
    onu_conf_res_act_t action;
    char filename[ONU_CONFIG_NAME_LEN];
    UCHAR  trynum;
    UCHAR  flags;
}onu_conf_res_queue_e;

typedef struct onu_conf_res_queue_list{
    onu_conf_res_queue_e * e;
    struct onu_conf_res_queue_list *next;
}onu_conf_res_queue_list_t;

extern ULONG g_onuConfSemId;
extern ULONG onu_conf_sem_deb;

extern long sys_console_printf(const char *format,...);

#if 0
#define ONU_CONF_SEM_TAKE if(VOS_SemTake(g_onuConfSemId, WAIT_FOREVER) == VOS_OK)
#define ONU_CONF_SEM_GIVE VOS_SemGive(g_onuConfSemId);
#else
#define ONU_CONF_SEM_TAKE {\
		if(onu_conf_sem_deb) sys_console_printf( "ONUCONF_SEM_TAKE:%s %d\r\n", __FILE__, __LINE__); \
		if(VOS_SemTake(g_onuConfSemId, WAIT_FOREVER) == VOS_OK) \
		{\
		if(onu_conf_sem_deb) sys_console_printf( "ONUCONF_SEM_TAKE OK:%s %d\r\n", __FILE__, __LINE__); \
		}\
}

#define ONU_CONF_SEM_GIVE {\
		if(onu_conf_sem_deb) sys_console_printf( "ONUCONF_SEM_GIVE:%s %d\r\n", __FILE__, __LINE__); \
			    if(VOS_SemGive(g_onuConfSemId) == VOS_OK) {\
			        {if(onu_conf_sem_deb) sys_console_printf( "ONUCONF_SEM_GIVE OK:%s %d\r\n", __FILE__, __LINE__);} } \
}
#endif



#define ONU_CONF_ENABLE 2
#define ONU_CONF_DISABLE 1

ULONG IFM_ONU_PROFILE_CREATE_INDEX(const char *name);

/*
 * ��ȡָ�����Ƶ������ļ��Ĺ�ϣ����
 * szName:ָ���������ļ�����
 * return:��ϣ����
*/
int getOnuConfHashIndex(const char *szName);

/*
 * ��ʼ����ϣͰ
 * depth:Ͱ��ֵ
 * return:1--�ɹ� 0--ʧ��
 */
int initOnuConfHashBucket(const int depth);

/*
 * ��ȡָ�����Ƶ������ļ�����ָ��
 * name:ָ���������ļ�����
 * return: �����ļ�ָ��
 */
ONUConfigData_t *getOnuConfFromHashBucket(const char *name);

/*
 * ��ָ�����Ƶ����������ļ�д���ϣͰ
 * name:ָ���������ļ�����
 * ptr:�����ļ��ĵ�ַ
 * return: 0--�ɹ�  -1--ʧ��
 */
int setOnuConfToHashBucket(const char *name, void *ptr);

/*
 * ��ѯĳ��ONU�����������Ƿ�Ϊ��������
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * return: TRUE/FALSE
 */
int onuConfIsShared(USHORT ponid, USHORT onuid);

int onuConfCheckByPtr(int suffix, void *pdata);

int startOnuConfSyndRequestWitchDstSlot(int slotno);

int sendOnuConfReportMsg(USHORT slot, USHORT pon, USHORT onu, char *name, int rcode);

int sendOnuConfSyndReqAckMsg(int slot, int result);

int sendOnuConfToAllPonBoard(const char *name);

int sendOnuConfMacToAllPonBoard(const char *mac, const char* name);

int sendOnuConfSyndReqMsgByName(USHORT slot, const char *name);

int sendOnuConfSyndUndoAssociationByNameMsg(USHORT slot, char *name, int nlen);

void startOnuConfSyndBroadcastRequest();

int sendOnuConfSyndBroadcastReqMsg(const char *name);

int sendOnuConfDebugReportMsg(USHORT dstslot, USHORT srcslot, int len, char *text);

void setOnuConfSyndBroadcastCardMask(int slot);

int portListLongToString(ULONG list, char *str);

void show_onu_conf_Dataheader(struct vty *vty,short int slot, short int pon);
#if 1
/*added by luh*/
uchar getOnuConfOnuType(USHORT ponid, USHORT onuid);
/*
 * ����ĳ��ONU���������ݵ�����Vlan�˿��б�
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * vid: Vlan ID��
 * allportmask: ���onu Vlan ���ж˿��б�ĵ�ַ
 * untagmask: ���onu Vlan ����δ�ӱ�ǩ�Ķ˿��б��ַ
 * return: �ɹ�����ROK/ʧ�ܷ���RERROR;
 */
int get_onuconf_vlanPortlist(SHORT ponid, SHORT onuid, USHORT vid, ULONG *allportmask, ULONG *untagmask);
int set_onuconf_vlanPortlist(SHORT ponid, SHORT onuid, USHORT vid, ULONG allportmask, ULONG untagmask);
int del_onuconf_vlan(SHORT ponid, SHORT onuid, USHORT vid);
int get_ctc_portVlanConfig(SHORT ponid, SHORT onuid, USHORT port, CTC_STACK_vlan_configuration_entry_t * pconf);
int SetOnuConfOnuType(SHORT ponid, SHORT onuid,UCHAR num);
int isOnuConfDefault(SHORT ponid, SHORT onuid);

/*
 * ��ȡĳ��ONU���������ݵ�ȱʡvlan ģʽ
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * port:  ONU�����ñ��еĶ˿ں�1-32
 * return: Vlan ID/0;
 */
int getOnuConfPortVlanMode(SHORT ponid, SHORT onuid,int port);
/*
 * ����ĳ��ONU���������ݵ�ȱʡvlan ģʽ
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * port:  ONU�����ñ��еĶ˿ں�1-32
 * num:   ��Ҫ���õ�ȱʡVlanģʽ
 * return: �ɹ�����1/ʧ�ܷ���0;
 */
int SetOnuConfPortVlanMode(SHORT ponid, SHORT onuid,int port, int num);
/*
 * ��ȡĳ��ONU���������ݵ�Vlan�˿ڸ���ʹ��״̬
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * return: ״ֵ̬/0;
 */
int getOnuVlanportIsolateEnable(USHORT ponid, USHORT onuid);
/*
 * ����ĳ��ONU���������ݵ�Vlan�˿ڸ���ʹ��״̬
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * num:   ��Ҫ���õ�ʹ��״̬Enable/Disable
 * return: �ɹ�����1/ʧ�ܷ���0;
 */
int SetOnuVlanportIsolateEnable(USHORT ponid, USHORT onuid,int num);
/*
 * ��ȡĳ��ONU���������ݵ�Vlan�ӿ���
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * return: �ӿ���/0;
 */
ushort getonuVlanEntryNum(USHORT ponid, USHORT onuid);
/*
 * ����ĳ��ONU���������ݵ�Vlan�ӿ���
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * num:   ��Ҫ����Vlan�ӿ���
 * return: �ɹ�����1/ʧ�ܷ���0;
 */
int SetonuVlanEntryNum(USHORT ponid, USHORT onuid,ushort num);
/*
 * ��ȡĳ��ONU���������ݵ�Vlan id
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * VlanPort: Vlan���ñ��������
 * return: Vlan id/0;
 */
short int getOnuConfvlanid(USHORT ponid, USHORT onuid,int VlanPort);
/*
 * ����ĳ��ONU���������ݵ�Vlan id
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * VlanPort: Vlan���ñ��������
 * num:   ��Ҫ����Vlan id
 * return: �ɹ�����1/ʧ�ܷ���0;
 */
int SetOnuConfvlanid(USHORT ponid, USHORT onuid,int VlanPort,short int num);
/*
 * ��ȡĳ��ONU���������ݵ�����Vlan�˿��б�
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * VlanPort: Vlan���ñ��������
 * return: �б�ֵ/0;
 */
ulong getOnuConfvlanallPortMask(USHORT ponid, USHORT onuid,int VlanPort);
/*
 * ����ĳ��ONU���������ݵ�����Vlan�˿��б�
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * VlanPort: Vlan���ñ��������
 * num:   ��Ҫ����Vlan id
 * return: �ɹ�����1/ʧ�ܷ���0;
 */
int SetOnuConfvlanallPortMask(USHORT ponid, USHORT onuid,int VlanPort,ulong num);
/*
 * ��ȡĳ��ONU���������ݵ�����δ�ӱ�ǩ��Vlan�˿��б�
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * VlanPort: Vlan���ñ��������
 * return: �б�ֵ/0;
 */
ulong getOnuConfvlanuntagPortMask(USHORT ponid, USHORT onuid,int VlanPort);
/*
 * ����ĳ��ONU���������ݵ�����δ�ӱ�ǩ��Vlan�˿��б�
 * ponid: ONU����PON�˿�����
 * onuid: ONU��PON�˿��ڵ��豸����
 * VlanPort: Vlan���ñ��������
 * num:   ��Ҫ����Vlan id
 * return: �ɹ�����1/ʧ�ܷ���0;
 */
int SetOnuConfvlanuntagPortMask(USHORT ponid, USHORT onuid,int VlanPort,ulong num);

ULONG onuconf_get_VlanEntryNum(USHORT ponid, USHORT onuid);

int getOnuConfPortSimpleVar(SHORT ponid, SHORT onuid, USHORT port, int vcode, ULONG *uv);

#endif
#if 1/*add by luh 2011/7/12  */
int onuconf_get_vlanEntryByIdx(USHORT ponid, USHORT onuid, ULONG idx,int *vid, ULONG *allmask, ULONG *untagmask);
int onuconf_get_vlanEntryByIdxByPtr(int suffix, void *pdata,ULONG idx,int *vid, ULONG *allmask, ULONG *untagmask);
int set_OnuConf_Ctc_EthPortVlanTranNewVid(SHORT ponid, SHORT onuid, USHORT port, ULONG inVid, ULONG newVid);
int set_OnuConf_Ctc_EthPortVlanTranNewVidByPtr(int suffix, void *pdata, USHORT port, ULONG inVid, ULONG newVid);
int del_OnuConf_Ctc_EthPortVlanTranNewVid(SHORT ponid, SHORT onuid, USHORT port, ULONG inVid);
int del_OnuConf_Ctc_EthPortVlanTranNewVidByPtr(int suffix, void *pdata, USHORT port, ULONG inVid);
int set_OnuConf_Ctc_EthPortVlanAgg(SHORT ponid, SHORT onuid, USHORT port, USHORT inVid[8], USHORT targetVid);
int set_OnuConf_Ctc_EthPortVlanAggByPtr(int suffix, void *pdata, USHORT port, USHORT inVid[8], ULONG targetVid);
int del_OnuConf_Ctc_EthPortVlanAgg(SHORT ponid, SHORT onuid, USHORT port, ULONG targetVid);
int del_OnuConf_Ctc_EthPortVlanAggByPtr(int suffix, void *pdata, USHORT port, ULONG targetVid);
int getOnuConfSimpleVar(SHORT ponid, SHORT onuid, int vcode, ULONG *uv);
int getOnuConfSimpleVarByPtr(int suffix, void *pdata, int vcode, ULONG *uv);
int setOnuConfSimpleVar(SHORT ponid, SHORT onuid, int vcode, ULONG uv);
int setOnuConfSimpleVarByPtr(int suffix, void *pdata, int vcode, ULONG uv);
int getOnuConfPortSimpleVarByPtr(int suffix, void *pdata, USHORT port, int vcode, ULONG *uv);
int setOnuConfPortSimpleVar(SHORT ponid, SHORT onuid, USHORT port, int vcode, ULONG uv);
int setOnuConfPortSimpleVarByPtr(int suffix, void *pdata, USHORT port, int vcode, ULONG uv);

int getOnuConf_igmp_groupdata(SHORT ponid, SHORT onuid, int num,int vcode, ULONG*uv);
int getOnuConf_igmp_groupdataByPtr(int suffix, void* pd, int num,int vcode, ULONG*uv);
int SetOnuConf_igmp_groupdata(SHORT ponid, SHORT onuid, int num,int vcode, ULONG uv);
int SetOnuConf_igmp_groupdataByPtr(int suffix, void* pd, int num,int vcode, ULONG uv);
int setOnuConfQosDscpToQueue(SHORT ponid, SHORT onuid, int dscpnum, int queue);
int setOnuConfQosDscpToQueueByPtr(int suffix, void *pdata, int dscpnum, int queue);
int getOnuConfQosDscpToQueue(SHORT ponid, SHORT onuid, int dscpnum, int *queue);
int getOnuConfQosDscpToQueueByPtr(int suffix, void *pdata, int dscpnum, int *queue);

int setOnuConfQosPrioReplace(SHORT ponid, SHORT onuid, USHORT port,int oldprio, int newprio);
int setOnuConfQosPrioReplaceByPtr(int suffix, void *pdata, USHORT port, int oldprio, int newprio);
int getOnuConfQosPrioReplace(SHORT ponid, SHORT onuid,USHORT port,int oldprio, int *newprio);
int getOnuConfQosPrioReplaceByPtr(int suffix, void *pdata, USHORT port,int oldprio, int *newprio);
int setOnuConfQosPrioToQueue(SHORT ponid, SHORT onuid,int prio,int queueNum);
int setOnuConfQosPrioToQueueByPtr(int suffix, void *pdata, int prio,int queueNum);
int getOnuConfQosPrioToQueue(SHORT ponid, SHORT onuid,int prio,int *queueNum);
int getOnuConfQosPrioToQueueByPtr(int suffix, void *pdata,int prio,int *queueNum);
int setOnuConfQosRule(SHORT ponid, SHORT onuid, USHORT qossetid, USHORT ruleid, UCHAR queue, UCHAR prio);
int setOnuConfQosRuleByPtr(int suffix, void *pdata, USHORT qossetid, USHORT ruleid, UCHAR queue, UCHAR prio);

int getOnuVlanConfig(short int ponid, short int onuid, ONUVlanConf_t *conf);
int getOnuVlanConfigByPtr(short int suffix, void * pdata, ONUVlanConf_t *conf);
#endif

#if 1 /*Vlan ��������ؽӿ�*/
extern int Get_OnuNumFromOnuRangeByVty(struct vty *vty,ULONG begin_slot, ULONG begin_port, ULONG begin_onuid, ULONG end_slot, ULONG end_port, ULONG end_onuid);
extern int Set_OnuProfile_VlanBat_Mode_Vid(struct vty* vty, int suffix, void *pdata, ULONG begin_onuid, ULONG end_onuid, ULONG vid, ULONG portlist, USHORT mode);
extern int Set_OnuProfile_VlanBat_Mode_Range(struct vty* vty, int suffix, void *pdata, ULONG begin_onuid, ULONG end_onuid, ULONG begin_vid,ULONG end_vid, ULONG onu_step, ULONG mode, ULONG port_step);
extern int Show_OnuProfile_VlanBat_RuleListByPtr(struct vty* vty, int suffix, void *pd);
extern int Show_OnuProfile_VlanBat_RuleList(struct vty* vty, ULONG slot, ULONG port, ULONG onuid);
extern int Delete_onuProfile_VlanBat_Rule(struct vty* vty, int suffix, void *pd, ULONG num);
extern int IsBelongToVlanRules(ULONG vid, ULONG slot, ULONG port, ULONG onuid);

#endif
ONUConfigData_t *OnuConfigProfile_init();

int generateOnuConfClMemFile( char *name, unsigned char flags, struct vty *vty, char *pfile, short int slot, short int port, short int onuid);

#define isOnuConfExist(name) (( getOnuConfFromHashBucket(name) == NULL )?FALSE:TRUE)

int onuconfHaveAssociatedOnu(const char *name);

int onuconfUndoAssociationByName(const char *name);

/*void onuconfAssociateOnuId(short int pon_id, short int onu_id, char *filename);*/

int onu_profile_rename(const char *filename, const char *rename);

int onuconfSwitchPonPort(short int src_slot, short int src_port, short int dst_slot, short int dst_port);

int delOnuFromRestoreQueue(SHORT pon_id, SHORT onu_id);

/*onu���ù�����ڴ��������ͷ�*/
void *onuconf_malloc(int module);
void onuconf_free(void *data, int module);

int setOnuTransmission_flag(int enable);
int getOnuTransmission_flag();

/*������Ϣ��־�޸ĺ���*/
void onuconf_DebugLevelSet(int level);
void onuconf_DebugLevelClr(int level);
int onuconf_isDebugLevelSet(int level);

#define BEGIN_ONUCONF_DEBUG_PRINT( level ) \
    if(onuconf_isDebugLevelSet(level)) \
    {

#define END_ONUCONF_DEBUG_PRINT() \
}


#define ONUCONF_TRACE(lv, a) \
        if(onuconf_isDebugLevelSet(lv)) \
            sys_console_printf a;

int onuconf_SetDirtyFlag(const char *filename);
int onuconf_ClrDirtyFlag(const char *filename);
int fcb_onuProfileCreation(ULONG argv[4]);
int fcb_onuProfileModified(ULONG argv[4]);
int fcb_onuProfileDelete(ULONG argv[4]);

/*---------------------------- �¼��ص������������ ----------------------------------------*/

#define MAX_EVENT_CALLBACK_CLIENTS 50
enum{
    EVENT_ONUPROFILE_ACTION,
    EVENT_ONUPROFILE_CREATE,
    EVENT_ONUPROFILE_MODIFIED,
    EVENT_ONUPROFILE_DELETE,
    EVENT_MAX_NUM
};

#define CHECK_EVENTID_VALID(x) ((x) >= 0 && (x) < EVENT_MAX_NUM)
#define CHECK_EVENT_HANDLE_VALID(x) ((x) >=0 && (x) < MAX_EVENT_CALLBACK_CLIENTS)

typedef int (*event_callback_func)(ULONG argv[4]);  /*�¼��ص������Ĺ��������������������ݲ�ͬ���¼����ж���*/

int registerEventCallback(int eventid, event_callback_func  pfunc);
int unregisterEventCallback(int eventid, int callbck_handle);
void processEventCallbackList(int eventid, ULONG argv[4]);
void initEventCallbackFunctions();

/*-----------------------------------------------------------------------------------*/

/*------------------------------------ ɾ����ʱ�����ļ���������� ----------------------------*/
#define AUTO_ONU_PROFILE_TIME_OUT 300

enum{
    auto_onu_profile_add_msg,
    auto_onu_profile_delete_msg,
    auto_onu_profile_time_msg,
    auto_onu_profile_cancle_check_msg
};/*��ʱ�ļ�����������Ҫ�������Ϣ����*/

void initAuoOnuProfileCheckTask();
int sendAutoOnuProfileAddMsg(const char *filename);
int sendAutoOnuProfileDeleteMsg(const char *filename);
int sendAutoOnuProfileCancelCheckMsg(const char *filename);
int sendAutoOnuProfileTimerMsg();

dataarray_t * dataarray_create(const int growth);
void dataarray_destroy(dataarray_t *parray);

/*-----------------------------------------------------------------------------------*/

/*----------------------------�����ļ���������--------------------------------------------*/
ONUConfigData_t * openOnuConfigFile(const char * name, onu_conf_op_flag_t mode );
int closeOnuConfigFile(const char * name);
/*-----------------------------------------------------------------------------------*/
ULONG onuconf_build_lzma_data(char * data, const int length, char ** outptr);
int onuconf_signal_cardsyncfinished(int slot);
int onuconf_wait_cardsyncfinished_signal(int slot, int timeout);
#endif /* ONUCONFMGT_H_ */
