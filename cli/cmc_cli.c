/****************************************************************************
*
*     Copyright (c) 2013 GWTT Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           GWTT Corporation
*
*     All information contained in this document is GWTT Corporation
*     company private, proprietary, and trade secret.
*
*     Changes:
*
*     Version       |  Date          |    Change        |    Author	  
*     ----------|-----------|-------------|------------
*	1.00           | 02/06/2013 |	   creation      | liwei056
*
*****************************************************************************/

#include "syscfg.h"

#ifdef __cplusplus
extern"C"
{
#endif

#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_syslog.h"
#include "vos/vospubh/vos_arg.h"
#include "vos/vospubh/vos_time.h"
#include "vos/vospubh/vos_sem.h"
#include "vos/vospubh/vos_task.h"
#include "vos/vospubh/vos_que.h"
#include "vos/vospubh/vos_ctype.h"

#include "cli/cli.h"
#include "cli/cl_cmd.h"
#include "cli/cl_mod.h"
#include "addr_aux.h"
#include "linux/inetdevice.h"

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

#include "interface/interface_task.h"
#include "sys/console/sys_console.h"

#include "Typesdb_product.h"
#include "Typesdb_module.h"

#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "../onu/OnuConfMgt.h"
#include "CmcGeneral.h"
#include "cmc_cli.h"

#if( EPON_MODULE_DOCSIS_MANAGE == EPON_MODULE_YES )
/*
#ifdef g_malloc
#undef g_malloc
#endif
*/

#define SHOW_CMC_STR               "show cmc information\n"
#define SET_CMC_STR                "config cmc\n"

#define SHOW_CMC_SVLAN_STR         "show cmc svlan config\n"
#define SET_CMC_SVLAN_STR          "config cmc svlan setting\n"

#define SHOW_CMC_CUSTOM_STR        "show cmc custom config\n"
#define SET_CMC_CUSTOM_STR         "config cmc custom setting\n"

#define SHOW_CMC_DOWNSTREAM_STR    "show the cmc's downstream settings\n"
#define SHOW_CMC_UPSTREAM_STR      "show the cmc's upstream settings\n"
#define SHOW_CMC_CHANNEL_STR       "show the cmc's channel settings\n"
#define SET_CMC_DOWNSTREAM_STR     "config cmc's down-channel(s)\n"
#define SET_CMC_UPSTREAM_STR       "config cmc's up-channel(s)\n"
#define SET_CMC_CHANNEL_STR        "config cmc's channel(s)\n"
#define SEL_CMC_CHANNEL_ALL_STR    "select all of channels\n"
#define SEL_CMC_CHANNEL_STR        "please input the channel number\n"

#define SHOW_CMC_IF_STR            "show the cmc's interface information\n"

#define SHOW_CMC_LBCFG_STR         "show the cmc's load balance configuration\n"
#define SHOW_CMC_GROUP_STR         "show the cmc's load balance group information\n"
#define SET_CMC_GROUP_STR          "config load balance group\n"
#define SEL_CMC_GROUP_ALL_STR      "select all of groups\n"
#define SEL_CMC_GROUP_STR          "please input the group number\n"

#define SHOW_CMC_GROUP_DYNAMIC_STR "show the cmc's load balance dynamic configuration\n"
#define SET_CMC_GROUP_DYNAMIC_STR  "config dynamic load balance group\n"

#define SHOW_CM_STR                "show cable-modem information\n"
#define SET_CM_STR                 "config cable-modem\n"
#define SEL_CM_ALL_STR             "all of cable modems under the cmc\n"
#define SEL_CM_STR                 "please input the cable modem's mac address\n"

#define SHOW_CMC_QOS_STR           "show the cmc's qos information\n"
#define SET_CMC_QOS_STR            "config the cmc's qos\n"

#define SHOW_CMC_SF_STR            "show the cmc's service-flow\n"
#define SET_CMC_SF_STR             "config the cmc's service-flow\n"
#define SHOW_CMC_CLS_STR           "show the cmc's classifier\n"
#define SET_CMC_CLS_STR            "config the cmc's classifier\n"


#define MAX_CHANNELID_NUMBER_IN_CHANNEL_LIST    16

enum match_type CMC_Check_ChannelId_List( char * channelid_list )
{
    int len = VOS_StrLen( channelid_list );
    ULONG interface_list[ MAX_CHANNELID_NUMBER_IN_CHANNEL_LIST ];
    int j, if_num = 0;
    int ret = 0;
    ULONG ulChannelId=0;

    char *plistbak = NULL;

    if ( ( !channelid_list ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

    VOS_MemZero( ( char * ) interface_list, sizeof( interface_list ) );
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, channelid_list );

    BEGIN_PARSE_ID_LIST_TO_ID( plistbak, ulChannelId, 1, MAX_CHANNELID_NUMBER_IN_CHANNEL_LIST, MAX_CHANNELID_NUMBER_IN_CHANNEL_LIST )
    {
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulChannelId )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                RETURN_PARSE_ID_LIST_TO_ID( no_match );  /* –¥÷ÿ∏¥µƒ∂Àø⁄»œŒ™ «¥ÌŒÛµƒ”Ô∑®  */
            }
        }
        interface_list[ if_num ] = ulChannelId;
        if_num ++;
        if ( if_num > (MAX_CHANNELID_NUMBER_IN_CHANNEL_LIST) )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            RETURN_PARSE_ID_LIST_TO_ID( no_match );
        }
        ret = 1;
    }
    END_PARSE_ID_LIST_TO_ID();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}


#define MAX_SFID_NUMBER_IN_SFID_LIST            8

enum match_type CMC_Check_SFID_List( char * sfid_list )
{
    int len = VOS_StrLen( sfid_list );
    ULONG interface_list[ MAX_SFID_NUMBER_IN_SFID_LIST ];
    int j, if_num = 0;
    int ret = 0;
    ULONG ulSFID=0;

    char *plistbak = NULL;

    if ( ( !sfid_list ) || ( len < 1 ) )
    {
        return incomplete_match;
    }

    VOS_MemZero( ( char * ) interface_list, sizeof( interface_list ) );
    plistbak = ( char * ) VOS_Malloc( len + 1, MODULE_RPU_CLI );
	if ( plistbak == NULL )
	{
	    return no_match;
	}

	VOS_StrCpy( plistbak, sfid_list );

    BEGIN_PARSE_ID_LIST_TO_ID( plistbak, ulSFID, 1, 0xFFFFFFFF, MAX_SFID_NUMBER_IN_SFID_LIST )
    {
        for ( j = 0; j <= if_num; j++ )
        {
            if ( interface_list[ j ] == ulSFID )
            {
                VOS_Free( plistbak );
                plistbak = NULL;

                RETURN_PARSE_ID_LIST_TO_ID( no_match );  /* –¥÷ÿ∏¥µƒ∂Àø⁄»œŒ™ «¥ÌŒÛµƒ”Ô∑®  */
            }
        }
        interface_list[ if_num ] = ulSFID;
        if_num ++;
        if ( if_num > (MAX_SFID_NUMBER_IN_SFID_LIST) )
        {
            VOS_Free( plistbak );
            plistbak = NULL;
            
            RETURN_PARSE_ID_LIST_TO_ID( no_match );
        }
        ret = 1;
    }
    END_PARSE_ID_LIST_TO_ID();

    VOS_Free( plistbak );

    if ( ret == 0 )
        return incomplete_match;
    else
        return exact_match;
}


static char *interleaver_strs[] = 
{ 
    "128_1",
    "128_2",
    "64_2",
    "128_3",
    "32_4",
    "128_4",
    "16_8",
    "128_5",
    "8_16",
    "128_6",
    "4_32",
    "128_7",
    "2_64",
    "128_8",
    "1_128",
    NULL 
};

int CMCUtil_GetDownstreamInterleaverCodeByName(const char *interleaver_ptr)
{
	int i;
    
	if(NULL == interleaver_ptr)
		return -1;

	for(i=0; interleaver_strs[i]!=NULL; i++)
	{
		if(!strncmp(interleaver_ptr, interleaver_strs[i], sizeof(interleaver_strs[i])))
			return i;
	}
    
	return -1;
}

int CMCUtil_GetDownstreamInterleaverNameByCode(const char interleaver_code, char interleaver_name[16])
{
	if(NULL == interleaver_name)
		return -1;

    if ( (interleaver_code >= 0) && (interleaver_code < ARRAY_SIZE(interleaver_strs) - 1) )
    {
        return VOS_Snprintf(interleaver_name, 15, "%s", interleaver_strs[interleaver_code]);
    }
    
	return -1;
}


int CMCUtil_GetCustomSubTypeByName(const char *cfgname_ptr)
{
    static unsigned char *CustomCfgStrs[] = 
    { 
        "tpid",
        "min-map-time",
        "max-map-time",
        "init-rng-period",
        "periodic-rng-period",
        "rng-backoff-start",
        "rng-backoff-end",
        "data-backoff-start",
        "data-backoff-end",
        "maplead-time-adj",
        NULL 
    };

    static unsigned char CustomCfgIds[] = 
    {
        kTPID,
        kMinMapTime,
        kMaxMapTime,
        kInitRngPeriod,
        kPeriodicRngPeriod,
        kRngBackoffStart,
        kRngBackoffEnd,
        kDataBackoffStart,
        kDataBackoffEnd,
        kMapLeadTimeAdjustment,
        0
    };

	int i;
    
	if(NULL == cfgname_ptr)
		return -1;

	for(i=0; CustomCfgStrs[i]!=NULL; i++)
	{
		if(!strncmp(cfgname_ptr, CustomCfgStrs[i], sizeof(CustomCfgStrs[i])))
			return CustomCfgIds[i];
	}
    
	return -1;
}


static unsigned char *cmc_profiles[] = 
{ 
    "qpsk",
    "16qam",
    "64qam",
    "256qam",
    "1024qam",
    NULL 
};

int CMCUtil_GetDownstreamModulationCodeByName(const char *modulation_ptr)
{
	int i;
    
	if(NULL == modulation_ptr)
		return -1;

	for(i=0; cmc_profiles[i]!=NULL; i++)
	{
		if(!strncmp(modulation_ptr, cmc_profiles[i], sizeof(cmc_profiles[i])))
			return i - 2;
	}
    
	return -1;
}

int CMCUtil_GetDownstreamModulationNameByCode(const char modulation_code, char modulation_name[16])
{
	if(NULL == modulation_name)
		return -1;

    if ( (modulation_code >= 0) && (modulation_code < 3) )
    {
        return VOS_Snprintf(modulation_name, 15, "%s", cmc_profiles[2 + modulation_code]);
    }
    
	return -1;
}


static unsigned char *cmc_types[] = 
{ 
    "scdma",
    "atdma",
    NULL 
};
static unsigned char *cmc_group[] = 
{ 
    "high_noise",
    "medium_noise",
    "low_noise",
    NULL 
};

int CMCUtil_GetUpstreamProfileCodeByCli(struct vty *vty, int argc, char *argv[])
{
    int  result;
    char type[32];
    char group[32];
    char *type_ptr;
    char *group_ptr;
    char *profile_ptr;

    profile_ptr = argv[1];
    if ( argc > 2 )
    {
        if ( argc > 3 )
        {
            type_ptr  = argv[2];
            group_ptr = argv[3];
        }
        else
        {
            if ( ('a' == argv[2][0]) || ('s' == argv[2][0]) )
            {
                type_ptr  = argv[2];
                group_ptr = NULL;
            }
            else
            {
                type_ptr  = NULL;
                group_ptr = argv[2];
            }
        }
    }
    else
    {
        type_ptr  = NULL;
        group_ptr = NULL;
    }

	/* restore use scdma. */
	if(NULL == type_ptr)
		strcpy(type, "scdma");
	else
		strcpy(type, type_ptr);

	/* default use high noise. */
	if(NULL == group_ptr)
	{
		if(!strncmp(type, "atdma", 5))
			strcpy(group, "medium_noise");
		else
			strcpy(group, "high_noise");
	}
	else
		strcpy(group, group_ptr);

	result = 0;
	if(!strncmp(type, "scdma", 5))
	{
	    if (!strncmp(group, "high_noise", 10))
        {
    		/* SCDMA High Noise Profiles (lower throughput, but higher protection against noise)
        		// 0 = QPSK
        		// 1 = 16QAM
        		// 2 = 64QAM
        		// 3 = 256QAM */
    		if(!strncmp(profile_ptr, "qpsk", 4))
    			result=0;
    		else if(!strncmp(profile_ptr, "16qam", 5))
    			result=1;
    		else if(!strncmp(profile_ptr, "64qam",5))
    			result=2;
    		else if(!strncmp(profile_ptr, "256qam",6))
    			result=3;	
    		else
    		{
        		vty_out(vty, "upstream profile type should be QPSK,16QAM,64QAM or 256QAM for SCDMA High Noise Profiles!\n");
        		return -1;
    		}
        }   
        else if (!strncmp(group, "medium_noise", 12))
        {
    		/* SCDMA Medium Noise Profiles (medium throughput, medium protection against noise):
        		// 4 = QPSK
        		// 5 = 16QAM
        		// 6 = 64QAM
        		// 7 = 256QAM */
    		if(!strncmp(profile_ptr, "qpsk", 4))
    			result=4;
    		else if(!strncmp(profile_ptr, "16qam", 5))
    			result=5;
    		else if(!strncmp(profile_ptr, "64qam",5))
    			result=6;
    		else if(!strncmp(profile_ptr, "256qam",6))
    			result=7;	
    		else
    		{
        		vty_out(vty, "upstream profile type should be QPSK,16QAM,64QAM or 256QAM for SCDMA Medium Noise Profiles!\n");
        		return -1;
    		}
        }
        else if (!strncmp(group, "low_noise", 9))
        {
    		/* SCDMA Low Noise Profiles (high throughput, low protection against noise):
        		// 8 = 64QAM
        		// 9 = 256QAM */
    		if(!strncmp(profile_ptr, "64qam",5))
    			result=8;
    		else if(!strncmp(profile_ptr, "256qam",6))
    			result=9;	
    		else
    		{
        		vty_out(vty, "upstream profile type should be 64QAM or 256QAM for SCDMA LOW Noise Profiles!\n");
        		return -1;
    		}
        }
        else if (!strncmp(group, "low_latency", 11))
        {
    		/* SCDMA Low Noise Profiles (high throughput, low protection against noise):
        		// 14 = 64QAM
        		// 15 = 256QAM */
    		if(!strncmp(profile_ptr, "64qam",5))
    			result=14;
    		else if(!strncmp(profile_ptr, "256qam",6))
    			result=15;	
    		else
    		{
        		vty_out(vty, "upstream profile type should be 64QAM or 256QAM for SCDMA LOW Latency Profiles!\n");
        		return -1;
    		}
        }
        else
        {
            VOS_ASSERT(0);
    		return -1;
        }
	}
	else if(!strncmp(type, "atdma", 5))
	{
	    if (!strncmp(group, "medium_noise", 12))
        {
            /* ATDMA Medium Noise Profiles (medium throughput, medium protection against noise):
                     // 10 = QPSK
        		// 11 = 16QAM
        		// 12 = 64QAM
        		// 13 = 256QAM */
    		if(!strncmp(profile_ptr, "qpsk", 4))
    			result=10;
    		else if(!strncmp(profile_ptr, "16qam", 5))
    			result=11;
    		else if(!strncmp(profile_ptr, "64qam",5))
    			result=12;
    		else if(!strncmp(profile_ptr, "256qam",6))
    			result=13;	
    		else
    		{
    			vty_out(vty, "upstream profile type should be QPSK,16QAM,64QAM or 256QAM for ATDMA Medium Noise Profiles!\n");
    			return CMD_WARNING;
    		}
        }
        else if (!strncmp(group, "high_noise", 10) || !strncmp(group, "low_noise", 9) || !strncmp(group, "low_latency", 11))
        {
			vty_out(vty, "\"high noise\" \"low noise\" and \"low_latency\" profile are not supported in \"atdma mode\"\n");
			return -1;
        }
        else
        {
            VOS_ASSERT(0);
    		return -1;
        }
	}
	else
	{
        VOS_ASSERT(0);
		return -1;
	}
    
	return result;
}

int CMCUtil_GetUpstreamProfileStrByCode(char profile_code, char profile_str[64])
{
    static unsigned char profile_table[][3] = { 
                                                {0, 0, 0},  /* 0 */
                                                {1, 0, 0},  /* 1 */
                                                {2, 0, 0},  /* 2 */
                                                {3, 0, 0},  /* 3 */
                                                {0, 0, 1},  /* 4 */
                                                {1, 0, 1},  /* 5 */
                                                {2, 0, 1},  /* 6 */
                                                {3, 0, 1},  /* 7 */
                                                {2, 0, 2},  /* 8 */
                                                {3, 0, 2},  /* 9 */
                                                {0, 1, 1},  /* 10 */
                                                {1, 1, 1},  /* 11 */
                                                {2, 1, 1},  /* 12 */
                                                {3, 1, 1},  /* 13 */
                                                {2, 1, 2},  /* 14 */
                                                {3, 1, 2}   /* 15 */
                                                };

    if ( (profile_code >= 0) && (profile_code < 16) )
    {
        int  profile_idx;
        int  type_idx;
        int  group_idx;
        
        profile_idx = profile_table[profile_code][0];   
        type_idx    = profile_table[profile_code][1];
        group_idx   = profile_table[profile_code][2];

    	return VOS_Snprintf(profile_str, 63, "profile %s type %s group %s", cmc_profiles[profile_idx], cmc_types[type_idx], cmc_group[group_idx]);
    }

	return -1;
}

const static char *s_WeekDay[] = 
{
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	NULL
};


extern LONG (*cdsms_file_exist_fs)(UCHAR *pFileName);
extern LONG (*cdsms_file_length_get_fs)(UCHAR *pFileName);
extern LONG (*cdsms_file_read_fs)(UCHAR *pFileName, UCHAR *pFileBuffer, ULONG *pFileLen);
extern LONG (*cdsms_file_write_fs)(UCHAR *pFileName, UCHAR *pFileBuffer, ULONG *pFileLen);

extern int parse_pon_command_parameter( struct vty *vty, ULONG *pulSlot, ULONG *pulPort , ULONG *pulOnuId, INT16 *pi16PonId );
extern int parse_onu_command_parameter( struct vty *vty, PON_olt_id_t *pPonPortIdx, PON_onu_id_t *pOnuIdx, PON_onu_id_t *pOnuId);
extern LONG CT_RMan_ONU_Init(enum node_type  node);


#if 1
/* CMCµ˜ ‘√¸¡Ó */

#if 1
/* --------------------CMCπ‹¿Ì------------------- */
DEFUN (
    set_cmc_svlan,
    set_cmc_svlan_cmd,
    "cmc svlan <0-4095>",
    SET_CMC_STR
    SET_CMC_SVLAN_STR
    "please input the vlanid(0-no cmc service, 4095-only cmc service)\n" 
    )
{
    int iVlanId;

    iVlanId = VOS_AtoI(argv[ 0 ]);

	if (0 != SetCmcSVlanID(OLT_ID_ALL, iVlanId))
	{
		vty_out(vty, "failed to set cmc's svlan(%d)\r\n", iVlanId);
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN (
    clr_cmc_svlan,
    clr_cmc_svlan_cmd,
    "undo cmc svlan",
    NO_STR
    SET_CMC_STR
    SET_CMC_SVLAN_STR
    )
{
	if (0 != SetCmcSVlanID(OLT_ID_ALL, 0))
	{
		vty_out(vty, "failed to set cmc's svlan\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN (
    show_cmc_svlan,
    show_cmc_svlan_cmd,
    "show cmc svlan",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_SVLAN_STR
    )
{
    int iVlanId;

	if (0 != GetCmcSVlanID(&iVlanId))
	{
		vty_out(vty, "failed to get cmc's svlan\r\n");
		return CMD_WARNING;
	}
    
	vty_out(vty, "CMC svlan: %d\r\n", iVlanId);

	return CMD_SUCCESS;
}


DEFUN (
    register_cmc_mac,
    register_cmc_mac_cmd,
    "cmc register <H.H.H>",
    SET_CMC_STR
    "register CMC in the system\n"
    "register a cmc manually, mac[xxxx.xxxx.xxxx]\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmcMac ) != VOS_OK )
    {
        vty_out(vty, "%% Invalid MAC address.\r\n");
		return CMD_WARNING;
    }

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != OnuMgt_RegisterCmc(PonPortIdx, OnuIdx, cmcMac))
	{
		vty_out(vty, "failed to register cmc\r\n");
		return CMD_WARNING;
	}
    
	vty_out(vty, "CMC register OK!\r\n");
	
	return CMD_SUCCESS;
}

DEFUN (
    unregister_cmc_mac,
    unregister_cmc_mac_cmd,
    "undo cmc register <H.H.H>",
    NO_STR
    SET_CMC_STR
    "unregister CMC in the system\n"
    "unregister a cmc manually, mac[xxxx.xxxx.xxxx]\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmcMac ) != VOS_OK )
    {
        vty_out(vty, "%% Invalid MAC address.\r\n");
		return CMD_WARNING;
    }

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != OnuMgt_UnregisterCmc(PonPortIdx, OnuIdx, cmcMac))
	{
		vty_out(vty, "failed to unregister cmc\r\n");
		return CMD_WARNING;
	}
    
	vty_out(vty, "CMC unregister OK!\r\n");
	
	return CMD_SUCCESS;
}


DEFUN (
    show_cmc,
    show_cmc_cmd,
    "show cmc",
    SHOW_STR
    "show registed cmc\n" 
    )
{
    ULONG slotId;
    ULONG portId;
    ULONG onuId;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
    INT16 PonPortIdx;
    INT16 OnuIdx;
	PON_onu_id_t onu_id;
	unsigned short length = sizeof(szDumpOutput);

    if ( PON_PORT_NODE == vty->node )
    {
    	if( parse_pon_command_parameter( vty, &slotId, &portId , &onuId, &PonPortIdx ) != VOS_OK )
    		return CMD_WARNING;    	

    	if (0 != OLT_DumpAllCmc(PonPortIdx, szDumpOutput, &length))
    	{
    		vty_out(vty, "failed to get cmc infos\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    		return CMD_WARNING;    	

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_DumpCmc(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length))
    	{
    		vty_out(vty, "failed to get cmc infos\r\n");
    		return CMD_WARNING;
    	}
    }

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
}   

DEFUN (
    show_cmc_alarm,
    show_cmc_alarm_cmd,
    "show cmc alarm-list",
    SHOW_STR
    SHOW_CMC_STR
    "show cmc's alarm list\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[8192] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
	if (0 != OnuMgt_DumpCmcAlarms(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length))
	{
		vty_out(vty, "failed to get cmc alarm-list\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    show_cmc_log,
    show_cmc_log_cmd,
    "show cmc log-data",
    SHOW_STR
    SHOW_CMC_STR
    "show cmc's log data\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[16384] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
	if (0 != OnuMgt_DumpCmcLogs(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length))
	{
		vty_out(vty, "failed to get cmc log-data\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    reset_cmc_board,
    reset_cmc_board_cmd,
    "reset cmc",
    CLEAR_STR
    "reset cmc board\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

	if (0 != OnuMgt_ResetCmcBoard(PonPortIdx, OnuIdx, cmcMac))
	{
		vty_out(vty, "failed to reset cmc\r\n");
		return CMD_WARNING;
	}
    
	vty_out(vty, "CMC reset OK!\r\n");
	
	return CMD_SUCCESS;
}


DEFUN (
    show_cmc_version,
    show_cmc_version_cmd,
    "show cmc version",
    SHOW_STR
    SHOW_CMC_STR
    "show cmc's version\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char cmcVersionArray[CMC_MAX_VERSION_LENGTH]= {0};
	unsigned char length = sizeof(cmcVersionArray);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
	if (0 != OnuMgt_GetCmcVersion(PonPortIdx, OnuIdx, cmcMac, cmcVersionArray, &length))
	{
		vty_out(vty, "failed to get cmc version\r\n");
		return CMD_WARNING;
	}

	vty_out(vty, "CMC Version: %s\r\n", cmcVersionArray);
	
	return CMD_SUCCESS;
} 


DEFUN (
    show_cmc_max_multicast,
    show_cmc_max_multicast_cmd,
    "show cmc max-multicasts",
    SHOW_STR
    SHOW_CMC_STR
    "show the max multicasts supported on this cmc\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned short  max_multicasts;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
	if (0 != OnuMgt_GetCmcMaxMulticasts(PonPortIdx, OnuIdx, cmcMac, &max_multicasts))
	{
		vty_out(vty, "failed to get cmc max-multicasts\r\n");
		return CMD_WARNING;
	}

	vty_out(vty, "Max Number of Multicasts: %u\n", max_multicasts);
	
	return CMD_SUCCESS;
} 


DEFUN (
    show_cmc_max_cm,
    show_cmc_max_cm_cmd,
    "show cmc max-cm",
    SHOW_STR
    SHOW_CMC_STR
    "show the max number of cable modem supported on this cmc\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned short  max_cm;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    ULONG ulIfIndex;
    ULONG ulSuffix;

    ulIfIndex = (ULONG) vty->index;
    if (!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if ( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
        
    	if (0 != OnuMgt_GetCmcMaxCm(PonPortIdx, OnuIdx, cmcMac, &max_cm))
    	{
    		vty_out(vty, "failed to get cmc max-cm\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;

        if ( 0 == getOnuConfSimpleVarByPtr(ulSuffix, vty->onuconfptr, sv_enum_cmc_max_cm, &ulValue) )
        {
            max_cm = (unsigned short)ulValue;
        }
        else
    	{
    		vty_out(vty, "failed to get cmc max-cm\r\n");
    		return CMD_WARNING;
    	}
    }
    
	vty_out(vty, "Max Number of CM: %u\r\n", max_cm);
	
	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_max_cm,
    set_cmc_max_cm_cmd,
    "cmc max-cm <0-255>",
    SET_CMC_STR
    "Set the max number of cm that cmc supported\n"
	"please input the cm number\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned short  max_cm;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    ULONG ulIfIndex;
    ULONG ulSuffix;

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if ( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
        
    	if (0 != OnuMgt_SetCmcMaxCm(PonPortIdx, OnuIdx, cmcMac, max_cm))
    	{
    		vty_out(vty, "failed to set cmc max-cm\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;

        ulValue = VOS_StrToUL(argv[0], NULL, 10);
        if ( 0 != setOnuConfSimpleVarByPtr(ulSuffix, vty->onuconfptr, sv_enum_cmc_max_cm, ulValue))
    	{
    		vty_out(vty, "failed to set cmc max-cm\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_max_cm,
    clr_cmc_max_cm_cmd,
    "undo cmc max-cm",
    NO_STR
    SET_CMC_STR
    "restore the max number of cm that cmc supported\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned short  max_cm;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    ULONG ulIfIndex;
    ULONG ulSuffix;

    max_cm = CMC_CFG_MAX_CM_DEFAULT;
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcMaxCm(PonPortIdx, OnuIdx, cmcMac, max_cm))
    	{
    		vty_out(vty, "failed to set cmc max-cm\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;

        ulValue = max_cm;
        if ( 0 != setOnuConfSimpleVarByPtr(ulSuffix, vty->onuconfptr, sv_enum_cmc_max_cm, ulValue))
    	{
    		vty_out(vty, "failed to set cmc max-cm\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 


DEFUN (
    show_cmc_clock,
    show_cmc_clock_cmd,
    "show cmc system-clock",
    SHOW_STR
    SHOW_CMC_STR
    "show cmc system clock\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    struct tm cmc_clock;

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
	if (0 != OnuMgt_GetCmcTime(PonPortIdx, OnuIdx, cmcMac, &cmc_clock))
	{
		vty_out(vty, "failed to get cmc system-clock\r\n");
		return CMD_WARNING;
	}

	vty_out(vty, "\nCurrent System Time: %04d/%02d/%02d %s %02d:%02d:%02d\n\n",
		cmc_clock.tm_year, cmc_clock.tm_mon, cmc_clock.tm_mday, s_WeekDay[cmc_clock.tm_wday], 
		cmc_clock.tm_hour, cmc_clock.tm_min, cmc_clock.tm_sec);
	
	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_clock,
    set_cmc_clock_cmd,
    "cmc system-clock use [olt|local]",
    SET_CMC_STR
    "config cmc system clock\n" 
    "select the cmc's clock source\n"
    "use olt's time as the cmc's system clock\n"
    "restore the cmc's system clock by self\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'o' == argv[0][0] )
    {
        time_t time_value;
        struct tm *cmc_clock;

        VOS_Time(&time_value);
        cmc_clock = VOS_LocalTime(&time_value);
        
        result = OnuMgt_SetCmcTime(PonPortIdx, OnuIdx, cmcMac, cmc_clock);
    }
    else
    {
        result = OnuMgt_LocalCmcTime(PonPortIdx, OnuIdx, cmcMac);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to set cmc system clock\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    show_cmc_custom_config,
    show_cmc_custom_config_cmd,
    "show cmc custom-config {[tpid|min-map-time|max-map-time|init-rng-period|periodic-rng-period|rng-backoff-start|rng-backoff-end|data-backoff-start|data-backoff-end|maplead-time-adj]}*1",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_CUSTOM_STR
    "VLAN ID TPID\n"
    "Minimum MAP time (in microseconds)\n"
    "Maximum MAP time (in microseconds)\n"
    "Initial ranging period (in milliseconds)\n"
    "Periodic ranging period (in milliseconds)\n"
    "Ranging backoff start\n"
    "Ranging backoff end\n"
    "Data backoff start\n"
    "Data backoff end\n"
    "Map Lead Time Adjustment\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char   cfg_id;
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( argc > 0 )
    {
        if ( 0 < (macLen = CMCUtil_GetCustomSubTypeByName(argv[0])) )
        {
            cfg_id = (unsigned char)macLen;
        }
        else
        {
            VOS_ASSERT(0);
    		return CMD_WARNING;
        }
    }
    else
    {
        cfg_id = 0;
    }
    
	if (0 != OnuMgt_DumpCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, szDumpOutput, &length))
	{
		vty_out(vty, "failed to get cmc custom-config\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_cc_tpid,
    set_cmc_cc_tpid_cmd,
    "cmc custom-config tpid [<0x0000-0xffff>|<0-65535>]",
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "config the CMC vlan TPID\n"
	"please input the hex number of the TPID\n"
	"please input the decimal number of the TPID\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned short  tpid;
    unsigned short  data_len;
    unsigned long   ulValue;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( ('0' == argv[0][0]) && (('x' == argv[0][1])
            || ('X' == argv[0][1])) )
    {
        ulValue = VOS_StrToUL( argv[0], NULL, 16 );
    }
    else
    {
        ulValue = VOS_StrToUL( argv[0], NULL, 10 );
    }

    tpid = (unsigned short)ulValue;
    cfg_data = (unsigned char*)&tpid;
    data_len = 2;
    cfg_id = kTPID;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-tpid\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_cc_tpid,
    clr_cmc_cc_tpid_cmd,
    "undo cmc custom-config tpid",
    NO_STR
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "restore the CMC vlan TPID\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned short  tpid;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    tpid = 0x88a8;
    cfg_data = (unsigned char*)&tpid;
    data_len = 2;
    cfg_id = kTPID;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-tpid\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_cc_map_time,
    set_cmc_cc_map_time_cmd,
    "cmc custom-config map-time <0-65535> <0-65535>",
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "Config MAP time range\n"
    "Config Minimum MAP time(in microseconds)\n"
    "Config Maximum MAP time(in microseconds)\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned short  val_min;
    unsigned short  val_max;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    val_min = (unsigned short)VOS_AtoI(argv[0]);
    val_max = (unsigned short)VOS_AtoI(argv[1]);

	if(val_min > val_max)
	{
		vty_out(vty, "Min map time should not greater than max map time!");
		return CMD_WARNING;
	}

    cfg_data = (unsigned char*)&val_min;
    data_len = 2;
    cfg_id = kMinMapTime;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-min-map-time\r\n");
		return CMD_WARNING;
	}

    cfg_data = (unsigned char*)&val_max;
    data_len = 2;
    cfg_id = kMaxMapTime;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-max-map-time\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_cc_map_time,
    clr_cmc_cc_map_time_cmd,
    "undo cmc custom-config map-time",
    NO_STR
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "restore the CMC vlan TPID\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned short  val_min;
    unsigned short  val_max;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

	val_min = 3200;
    cfg_data = (unsigned char*)&val_min;
    data_len = 2;
    cfg_id = kMinMapTime;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-min-map-time\r\n");
		return CMD_WARNING;
	}

	val_max = 8000;
    cfg_data = (unsigned char*)&val_max;
    data_len = 2;
    cfg_id = kMaxMapTime;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-max-map-time\r\n");
		return CMD_WARNING;
	}
    
	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_cc_init_rng_prd,
    set_cmc_cc_init_rng_prd_cmd,
    "cmc custom-config init-rng-period <0-4294967295>",
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "config initial ranging period\n"
	"Initial ranging period (in milliseconds)\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned short  data_len;
    unsigned long   ulValue;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    ulValue = VOS_StrToUL( argv[0], NULL, 10 );
    cfg_data = (unsigned char*)&ulValue;
    data_len = 4;
    cfg_id = kInitRngPeriod;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-init-rng-period\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_cc_init_rng_prd,
    clr_cmc_cc_init_rng_prd_cmd,
    "undo cmc custom-config init-rng-period",
    NO_STR
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "restore initial ranging period\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned long   ulValue;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    ulValue = 2000;
    cfg_data = (unsigned char*)&ulValue;
    data_len = 4;
    cfg_id = kInitRngPeriod;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-init-rng-period\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_cc_prd_rng_prd,
    set_cmc_cc_prd_rng_prd_cmd,
    "cmc custom-config periodic-rng-period <0-4294967295>",
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "config periodic ranging period\n"
	"Periodic ranging period (in milliseconds)\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned short  data_len;
    unsigned long   ulValue;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    ulValue = VOS_StrToUL( argv[0], NULL, 10 );
    cfg_data = (unsigned char*)&ulValue;
    data_len = 4;
    cfg_id = kPeriodicRngPeriod;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-periodic-rng-period\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_cc_prd_rng_prd,
    clr_cmc_cc_prd_rng_prd_cmd,
    "undo cmc custom-config periodic-rng-period",
    NO_STR
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "restore periodic ranging period\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned long   ulValue;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    ulValue = 15000;
    cfg_data = (unsigned char*)&ulValue;
    data_len = 4;
    cfg_id = kPeriodicRngPeriod;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-periodic-rng-period\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_cc_range_backoff,
    set_cmc_cc_range_backoff_cmd,
    "cmc custom-config range-backoff <0-15> <0-15>",
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "Config Ranging backoff range\n"
    "Ranging backoff start\n"
    "Ranging backoff end\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned char   val_start;
    unsigned char   val_end;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    val_start = (unsigned char)VOS_AtoI(argv[0]);
    val_end = (unsigned char)VOS_AtoI(argv[1]);

	if(val_start > val_end)
	{
		vty_out(vty, "Start value should not greater than end value!");
		return CMD_WARNING;
	}

    cfg_data = (unsigned char*)&val_start;
    data_len = 1;
    cfg_id = kRngBackoffStart;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-range-backoff-start\r\n");
		return CMD_WARNING;
	}

    cfg_data = (unsigned char*)&val_end;
    data_len = 1;
    cfg_id = kRngBackoffEnd;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-range-backoff-end\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_cc_range_backoff,
    clr_cmc_cc_range_backoff_cmd,
    "undo cmc custom-config range-backoff",
    NO_STR
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "restore Ranging backoff range\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned char   val_start;
    unsigned char   val_end;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

	val_start = 2;
    cfg_data = (unsigned char*)&val_start;
    data_len = 1;
    cfg_id = kRngBackoffStart;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-range-backoff-start\r\n");
		return CMD_WARNING;
	}

	val_end = 3;
    cfg_data = (unsigned char*)&val_end;
    data_len = 1;
    cfg_id = kRngBackoffEnd;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-range-backoff-end\r\n");
		return CMD_WARNING;
	}
    
	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_cc_data_backoff,
    set_cmc_cc_data_backoff_cmd,
    "cmc custom-config data-backoff <0-15> <0-15>",
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "Config Data backoff range\n"
    "Data backoff start\n"
    "Data backoff end\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned char   val_start;
    unsigned char   val_end;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    val_start = (unsigned char)VOS_AtoI(argv[0]);
    val_end = (unsigned char)VOS_AtoI(argv[1]);

	if(val_start > val_end)
	{
		vty_out(vty, "Ranging backoff start should not greater than Ranging backoff end!");
		return CMD_WARNING;
	}

    cfg_data = (unsigned char*)&val_start;
    data_len = 1;
    cfg_id = kDataBackoffStart;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-data-backoff-start\r\n");
		return CMD_WARNING;
	}

    cfg_data = (unsigned char*)&val_end;
    data_len = 1;
    cfg_id = kDataBackoffEnd;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-data-backoff-end\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_cc_data_backoff,
    clr_cmc_cc_data_backoff_cmd,
    "undo cmc custom-config data-backoff",
    NO_STR
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "restore Data backoff range\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned char   val_start;
    unsigned char   val_end;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

	val_start = 2;
    cfg_data = (unsigned char*)&val_start;
    data_len = 1;
    cfg_id = kDataBackoffStart;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-data-backoff-start\r\n");
		return CMD_WARNING;
	}

	val_end = 8;
    cfg_data = (unsigned char*)&val_end;
    data_len = 1;
    cfg_id = kDataBackoffEnd;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-data-backoff-end\r\n");
		return CMD_WARNING;
	}
    
	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_cc_map_lead_time,
    set_cmc_cc_map_lead_time_cmd,
    "cmc custom-config maplead-time-adj <-32767-32768>",
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "Config Map Lead Time Adjustment\n"
	"Map Lead Time Adjustment(in milliseconds)\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    unsigned short  data_len;
    short int       sValue;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    sValue = (short)VOS_AtoI(argv[0]);
    cfg_data = (unsigned char*)&sValue;
    data_len = 2;
    cfg_id = kMapLeadTimeAdjustment;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-maplead-time-adj\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_cc_map_lead_time,
    clr_cmc_cc_map_lead_time_cmd,
    "undo cmc custom-config maplead-time-adj",
    NO_STR
    SET_CMC_STR
    SET_CMC_CUSTOM_STR
    "Restore Map Lead Time Adjustment\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   cfg_id;
    short int       sValue;
    unsigned short  data_len;
    unsigned char  *cfg_data;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    sValue = 0;
    cfg_data = (unsigned char*)&sValue;
    data_len = 2;
    cfg_id = kMapLeadTimeAdjustment;
	if (0 != OnuMgt_SetCmcCustomConfig(PonPortIdx, OnuIdx, cmcMac, cfg_id, cfg_data, data_len))
	{
		vty_out(vty, "failed to set cmc custom-maplead-time-adj\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

#endif


#if 1
/* --------------------CMC∆µµ¿π‹¿Ì------------------- */
DEFUN (
    show_cmc_down_channel,
    show_cmc_down_channel_cmd,
    "show cmc downstream channel [all|<1-16>]",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_DOWNSTREAM_STR
    SHOW_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char   channel_id;
	char szDumpOutput[4096] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
	if (0 != OnuMgt_DumpCmcDownChannel(PonPortIdx, OnuIdx, cmcMac, channel_id, szDumpOutput, &length))
	{
		vty_out(vty, "failed to get channel info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    show_cmc_up_channel,
    show_cmc_up_channel_cmd,
    "show cmc upstream channel [all|<1-4>]",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_UPSTREAM_STR
    SHOW_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char   channel_id;
	char szDumpOutput[4096] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
	if (0 != OnuMgt_DumpCmcUpChannel(PonPortIdx, OnuIdx, cmcMac, channel_id, szDumpOutput, &length))
	{
		vty_out(vty, "failed to get channel info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 


DEFUN (
    close_cmc_down_channel,
    close_cmc_down_channel_cmd,
    "shutdown cmc downstream channel [all|<1-16>]",
    "close cmc's channel(s)\n"
    "close cmc's channel(s)\n"
    "close the cmc's downstream channel\n" 
    "close the cmc's downstream channel\n" 
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  channel_mode;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
        
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

        channel_mode = FALSE;
        
    	if (0 != OnuMgt_SetCmcDownChannelMode(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_mode))
    	{
    		vty_out(vty, "failed to close channel\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = FALSE;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_enable, ulValue))
    	{
    		vty_out(vty, "failed to close channel\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    open_cmc_down_channel,
    open_cmc_down_channel_cmd,
    "undo shutdown cmc downstream channel [all|<1-16>]",
    NO_STR
    "open cmc's channel(s)\n"
    "open cmc's channel(s)\n"
    "open the cmc's downstream channel\n" 
    "open the cmc's downstream channel\n" 
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  channel_mode;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
        
        channel_mode = TRUE;
    	if (0 != OnuMgt_SetCmcDownChannelMode(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_mode))
    	{
    		vty_out(vty, "failed to open channel\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = TRUE;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_enable, ulValue))
    	{
    		vty_out(vty, "failed to open channel\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    close_cmc_up_channel,
    close_cmc_up_channel_cmd,
    "shutdown cmc upstream channel [all|<1-4>]",
    "close cmc's channel(s)\n"
    "close cmc's channel(s)\n"
    "close the cmc's upstream channel\n" 
    "close the cmc's upstream channel\n" 
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  channel_mode;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

        channel_mode = FALSE;
    	if (0 != OnuMgt_SetCmcUpChannelMode(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_mode))
    	{
    		vty_out(vty, "failed to close channel\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = FALSE;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_enable, ulValue))
    	{
    		vty_out(vty, "failed to close channel\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    open_cmc_up_channel,
    open_cmc_up_channel_cmd,
    "undo shutdown cmc upstream channel [all|<1-4>]",
    NO_STR
    "open cmc's channel(s)\n"
    "open cmc's channel(s)\n"
    "open the cmc's upstream channel\n" 
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  channel_mode;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

        channel_mode = TRUE;
    	if (0 != OnuMgt_SetCmcUpChannelMode(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_mode))
    	{
    		vty_out(vty, "failed to open channel\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = TRUE;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_enable, ulValue))
    	{
    		vty_out(vty, "failed to open channel\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_up_channel_30,
    set_cmc_up_channel_30_cmd,
    "cmc upstream channel [all|<1-4>] docsis30 [enable|disable]",
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "up-channel's docsis30 function\n"
    "open up-channel's docsis30 function\n" 
    "close up-channel's docsis30 function\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  docsis30_mode;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    if ( 'e' == argv[1][0] )
    {
        docsis30_mode = TRUE;
    }
    else
    {
        docsis30_mode = FALSE;
    }
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcUpChannelD30Mode(PonPortIdx, OnuIdx, cmcMac, channel_id, docsis30_mode))
    	{
    		vty_out(vty, "failed to set channel's docsis30\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = docsis30_mode;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_d30, ulValue))
    	{
    		vty_out(vty, "failed to set channel's docsis30\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_down_channel_freq,
    set_cmc_down_channel_freq_cmd,
    "cmc downstream channel <1-16> freq <50000000-1000000000>",
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_STR 
    "config down-channel's frequency\n"
    "down-channel's frequency(unit: HZ)\n"
    "please input the frequency number(unit: HZ)\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned long  channel_freq;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = 0;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    channel_freq = VOS_StrToUL(argv[1], NULL, 10);

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
        
    	if (0 != OnuMgt_SetCmcDownChannelFreq(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_freq))
    	{
    		vty_out(vty, "failed to set channel's frequency\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = channel_freq;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_freq, ulValue))
    	{
    		vty_out(vty, "failed to set channel's frequency\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_down_channel_freq,
    clr_cmc_down_channel_freq_cmd,
    "undo cmc downstream channel <1-16> freq",
    NO_STR
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_STR 
    "restore down-channel's frequency\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned long  channel_freq;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = 0;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    channel_freq = CMC_CFG_DOWN_CHANNEL_FREQ_DEFAULT(channel_id - 1);

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcDownChannelFreq(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_freq))
    	{
    		vty_out(vty, "failed to set channel's frequency\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = channel_freq;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_freq, ulValue))
    	{
    		vty_out(vty, "failed to set channel's frequency\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_up_channel_freq,
    set_cmc_up_channel_freq_cmd,
    "cmc upstream channel <1-4> freq <5000000-100000000>",
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_STR 
    "up-channel's frequency(unit: HZ)\n"
    "please input the frequency number(unit: HZ)\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned long  channel_freq;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = 0;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    channel_freq = VOS_StrToUL(argv[1], NULL, 10);

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != OnuMgt_SetCmcUpChannelFreq(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_freq))
    	{
    		vty_out(vty, "failed to set channel's frequency\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_freq;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_freq, ulValue))
    	{
    		vty_out(vty, "failed to set channel's frequency\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_up_channel_freq,
    clr_cmc_up_channel_freq_cmd,
    "undo cmc upstream channel <1-4> freq",
    NO_STR
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_STR 
    "restore up-channel's frequency\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned long  channel_freq;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = 0;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    channel_freq = CMC_CFG_UP_CHANNEL_FREQ_DEFAULT(channel_id - 1);

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
    
    	if (0 != OnuMgt_SetCmcUpChannelFreq(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_freq))
    	{
    		vty_out(vty, "failed to set channel's frequency\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_freq;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_freq, ulValue))
    	{
    		vty_out(vty, "failed to set channel's frequency\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 


/*  Ù”⁄≤Ÿ◊˜£¨∑«≈‰÷√ */
DEFUN (
    set_cmc_auto_channel_freq,
    set_cmc_auto_channel_freq_cmd,
    "cmc [downstream|upstream] freq auto-assign {start <50000000-1000000000> step <1000000-100000000> mode [increase|decrease]}*1",
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_UPSTREAM_STR
    "config channel's frequency\n"
    "auto assign channel's frequency\n"
    "frequency's beginning\n"
    "channel's frequency(unit: HZ)\n"
    "frequency's step\n"
    "the change of frequency(unit: HZ)\n"
    "the changed mode of frequency\n"
    "the frequency is assigned to increase\n"
    "the frequency is assigned to decrease\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  step_mode;
    unsigned long  base_freq;
    unsigned long  step_freq;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 3 < argc )
    {
        if ( 'i' == argv[3][0] )
        {
            step_mode = CMC_CFG_DOWN_CHANNEL_FREQ_DIR_INCREASE;
        }
        else
        {
            step_mode = CMC_CFG_DOWN_CHANNEL_FREQ_DIR_DECREASE;
        }

        base_freq = VOS_StrToUL(argv[1], NULL, 10);
        step_freq = VOS_StrToUL(argv[2], NULL, 10);
    }
    else
    {
        if ( 'd' == argv[0][0] )
        {
            base_freq = CMC_CFG_DOWN_CHANNEL_FREQ_BASE_DEFAULT;
            step_freq = CMC_CFG_DOWN_CHANNEL_FREQ_STEP_DEFAULT;
            step_mode = CMC_CFG_DOWN_CHANNEL_FREQ_DIR_DEFAULT;
        }
        else
        {
            base_freq = CMC_CFG_UP_CHANNEL_FREQ_BASE_DEFAULT;
            step_freq = CMC_CFG_UP_CHANNEL_FREQ_STEP_DEFAULT;
            step_mode = CMC_CFG_UP_CHANNEL_FREQ_DIR_DEFAULT;
        }
    }

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

        if ( 'd' == argv[0][0] )
        {
            result = OnuMgt_SetCmcDownAutoFreq(PonPortIdx, OnuIdx, cmcMac, base_freq, step_freq, step_mode);
        }
        else
        {
            result = OnuMgt_SetCmcUpAutoFreq(PonPortIdx, OnuIdx, cmcMac, base_freq, step_freq, step_mode);
        }
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValues[3];
        USHORT usPort;

        if ( 'd' == argv[0][0] )
        {
            usPort = CMC_DOWNCHANNEL2PORT(CMC_CHANNELID_ALL);
        }
        else
        {
            usPort = CMC_UPCHANNEL2PORT(CMC_CHANNELID_ALL);
        }
        
        ulValues[0] = base_freq;
        ulValues[1] = step_freq;
        ulValues[2] = step_mode;
        result = setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_freq, ulValues);
    }

	if (0 != result)
	{
		vty_out(vty, "failed to set frequency's auto-assign\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_up_channel_width,
    set_cmc_up_channel_width_cmd,
    "cmc upstream channel [all|<1-4>] width [1600000|3200000|6400000]",
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "up-channel's width(unit: HZ)\n"
    "1.6MHZ\n" 
    "3.2MHZ\n" 
    "6.4MHZ\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned long  channel_width;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    channel_width = VOS_StrToUL(argv[1], NULL, 10);
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcUpChannelWidth(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_width))
    	{
    		vty_out(vty, "failed to set channel's width\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_width;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_width, ulValue))
    	{
    		vty_out(vty, "failed to set channel's width\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_up_channel_width,
    clr_cmc_up_channel_width_cmd,
    "undo cmc upstream channel [all|<1-4>] width",
    NO_STR
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "restore up-channel's width\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned long  channel_width;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    channel_width = CMC_CFG_UP_CHANNEL_FREQ_WIDTH_DEFAULT;
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcUpChannelWidth(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_width))
    	{
    		vty_out(vty, "failed to set channel's width\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_width;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_width, ulValue))
    	{
    		vty_out(vty, "failed to set channel's width\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_down_channel_annex,
    set_cmc_down_channel_annex_cmd,
    "cmc downstream channel [all|<1-16>] annex [a|b]",
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "down-channel's annex mode\n"
    "annex mode: a\n"
    "annex mode: b\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  annex_mode;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    if ( 'a' == argv[0][0] )
    {
        annex_mode = CMC_CFG_DOWN_CHANNEL_ANNEX_A;
    }
    else
    {
        annex_mode = CMC_CFG_DOWN_CHANNEL_ANNEX_B;
    }
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
    
    	if (0 != OnuMgt_SetCmcDownChannelAnnexMode(PonPortIdx, OnuIdx, cmcMac, channel_id, annex_mode))
    	{
    		vty_out(vty, "failed to set channel's annex mode\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = annex_mode;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_annex, ulValue))
    	{
    		vty_out(vty, "failed to set channel's annex mode\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_down_channel_annex,
    clr_cmc_down_channel_annex_cmd,
    "undo cmc downstream channel [all|<1-16>] annex",
    NO_STR
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "restore down-channel's annex mode\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  annex_mode;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    annex_mode = CMC_CFG_DOWN_CHANNEL_ANNEX_A;
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
        
    	if (0 != OnuMgt_SetCmcDownChannelAnnexMode(PonPortIdx, OnuIdx, cmcMac, channel_id, annex_mode))
    	{
    		vty_out(vty, "failed to set channel's annex mode\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = annex_mode;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_annex, ulValue))
    	{
    		vty_out(vty, "failed to set channel's annex mode\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_up_channel_type,
    set_cmc_up_channel_type_cmd,
    "cmc upstream channel [all|<1-4>] type [scdma|atdma]",
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "up-channel's type(Only support ATDMA or SCDMA mode)\n"
    "SCDMA\n" 
    "ATDMA\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  channel_type;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    if ( 'a' == argv[1][0] )
    {
        channel_type = CMC_CFG_UP_CHANNEL_TYPE_ATDMA;
    }
    else
    {
        channel_type = CMC_CFG_UP_CHANNEL_TYPE_SCDMA;
    }
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcUpChannelType(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_type))
    	{
    		vty_out(vty, "failed to set channel's type\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_type;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_type, ulValue))
    	{
    		vty_out(vty, "failed to set channel's type\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_up_channel_type,
    clr_cmc_up_channel_type_cmd,
    "undo cmc upstream channel [all|<1-4>] type",
    NO_STR
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "restore up-channel's type\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  channel_type;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    channel_type = CMC_CFG_UP_CHANNEL_TYPE_SCDMA;
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcUpChannelType(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_type))
    	{
    		vty_out(vty, "failed to set channel's type\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_type;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_type, ulValue))
    	{
    		vty_out(vty, "failed to set channel's type\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_down_channel_modulation,
    set_cmc_down_channel_modulation_cmd,
    "cmc downstream channel [all|<1-16>] modulation [64qam|256qam|1024qam]",
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "down-channel's modulation type(Only support QAM mode)\n"
    "64QAM\n"
    "256QAM\n"
    "1024QAM\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  modulation_type;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    if ( '6' == argv[1][0] )
    {
        modulation_type = CMC_CFG_DOWN_CHANNEL_MODULATION_64QAM;
    }
    else if ( '2' == argv[1][0] )
    {
        modulation_type = CMC_CFG_DOWN_CHANNEL_MODULATION_256QAM;
    }
    else if ( '1' == argv[1][0] )
    {
        modulation_type = CMC_CFG_DOWN_CHANNEL_MODULATION_1024QAM;
    }
    else
    {
        VOS_ASSERT(0);
		return CMD_WARNING;
    }
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcDownChannelModulation(PonPortIdx, OnuIdx, cmcMac, channel_id, modulation_type))
    	{
    		vty_out(vty, "failed to set channel's modulation type\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = modulation_type;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_modulation, ulValue))
    	{
    		vty_out(vty, "failed to set channel's modulation type\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_down_channel_modulation,
    clr_cmc_down_channel_modulation_cmd,
    "undo cmc downstream channel [all|<1-16>] modulation",
    NO_STR
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "restore down-channel's modulation type\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  modulation_type;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    modulation_type = CMC_CFG_DOWN_CHANNEL_MODULATION_DEFAULT;
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcDownChannelModulation(PonPortIdx, OnuIdx, cmcMac, channel_id, modulation_type))
    	{
    		vty_out(vty, "failed to set channel's modulation type\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = modulation_type;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_modulation, ulValue))
    	{
    		vty_out(vty, "failed to set channel's modulation type\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_up_channel_profile,
    set_cmc_up_channel_profile_cmd,
    "cmc upstream channel [all|<1-4>] profile [qpsk|16qam|64qam|256qam] {type [scdma|atdma]}*1 {group [high_noise|medium_noise|low_noise|low_latency]}*1",
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "up-channel's modulation type\n"
    "QPSK\n" 
    "16QAM\n" 
    "64QAM\n" 
    "256QAM\n" 
    "up-channel's type\n"
    "ATDMA\n" 
    "SCDMA\n" 
    "up-channel's ground group\n"
    "high_noise ground\n" 
    "medium_noise ground\n" 
    "low_noise ground\n" 
    "low_latency ground\n" 
    )
{
    PON_olt_id_t	PonPortIdx;
    PON_onu_id_t	OnuIdx;
    PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  channel_profile;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }

    if ( 0 <= (macLen = CMCUtil_GetUpstreamProfileCodeByCli(vty, argc, argv)) )
    {
        channel_profile = (unsigned char)macLen;
    }
    else
    {
		return CMD_WARNING;
    }
    
    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcUpChannelProfile(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_profile))
    	{
    		vty_out(vty, "failed to set channel's profile\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_profile;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_profile, ulValue))
    	{
    		vty_out(vty, "failed to set channel's profile\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_up_channel_profile,
    clr_cmc_up_channel_profile_cmd,
    "undo cmc upstream channel [all|<1-4>] profile",
    NO_STR
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "restore up-channel's profile\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  channel_profile;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
    channel_profile = CMC_CFG_UP_CHANNEL_PROFILE_DEFAULT;

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
        
    	if (0 != OnuMgt_SetCmcUpChannelProfile(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_profile))
    	{
    		vty_out(vty, "failed to set channel's profile\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_profile;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_profile, ulValue))
    	{
    		vty_out(vty, "failed to set channel's profile\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_down_channel_interleaver,
    set_cmc_down_channel_interleaver_cmd,
    "cmc downstream channel [all|<1-16>] interleaver [128_1|128_2|64_2|128_3|32_4|128_4|16_8|128_5|8_16|128_6|4_32|128_7|2_64|128_8|1_128]",
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "down-channel's interleaver value\n"
    "128:1\n"
    "128:2\n"
    "64:2\n"
    "128:3\n"
    "32:4\n"
    "128:4\n"
    "16:8\n"
    "128:5\n"
    "8:16\n"
    "128:6\n"
    "4:32\n"
    "128:7\n"
    "2:64\n"
    "128:8\n"
    "1:128\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  interleaver;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
    if ( 0 <= (macLen = CMCUtil_GetDownstreamInterleaverCodeByName(argv[1])) )
    {
        interleaver = (unsigned char)macLen;
    }
    else
    {
        VOS_ASSERT(0);
		return CMD_WARNING;
    }

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
        
    	if (0 != OnuMgt_SetCmcDownChannelInterleaver(PonPortIdx, OnuIdx, cmcMac, channel_id, interleaver))
    	{
    		vty_out(vty, "failed to set channel's interleaver value\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = interleaver;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_interleave, ulValue))
    	{
    		vty_out(vty, "failed to set channel's interleaver value\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_down_channel_interleaver,
    clr_cmc_down_channel_interleaver_cmd,
    "undo cmc downstream channel [all|<1-4>] interleaver",
    NO_STR
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "restore down-channel's interleaver value\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    unsigned char  interleaver;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
    interleaver = CMC_CFG_DOWN_CHANNEL_INTERLEAVER_DEFAULT;

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcDownChannelInterleaver(PonPortIdx, OnuIdx, cmcMac, channel_id, interleaver))
    	{
    		vty_out(vty, "failed to set channel's interleaver value\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = interleaver;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_interleave, ulValue))
    	{
    		vty_out(vty, "failed to set channel's interleaver value\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_down_channel_power,
    set_cmc_down_channel_power_cmd,
    "cmc downstream channel [all|<1-16>] power-level <400-630>",
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "down-channel's power level\n"
    "please input the power-level value(tenth of dBmv)\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    short int channel_power;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
    channel_power = VOS_AtoI(argv[1]);

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcDownChannelPower(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_power))
    	{
    		vty_out(vty, "failed to set channel's power level\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = channel_power;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_power, ulValue))
    	{
    		vty_out(vty, "failed to set channel's power level\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_down_channel_power,
    clr_cmc_down_channel_power_cmd,
    "undo cmc downstream channel [all|<1-16>] power-level",
    NO_STR
    SET_CMC_STR
    SET_CMC_DOWNSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "restore down-channel's power level\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    short int channel_power;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
    channel_power = 500;

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcDownChannelPower(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_power))
    	{
    		vty_out(vty, "failed to set channel's power level\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_DOWNCHANNEL2PORT(channel_id);
        ulValue = channel_power;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_power, ulValue))
    	{
    		vty_out(vty, "failed to set channel's power level\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_up_channel_power,
    set_cmc_up_channel_power_cmd,
    "cmc upstream channel [all|<1-4>] power-level <-130-230>",
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "up-channel's power level\n"
    "please input the power-level value(tenth of dBmv)\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    short int channel_power;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
    channel_power = VOS_AtoI(argv[1]);

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  
        
    	if (0 != OnuMgt_SetCmcUpChannelPower(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_power))
    	{
    		vty_out(vty, "failed to set channel's power level\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_power;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_power, ulValue))
    	{
    		vty_out(vty, "failed to set channel's power level\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_up_channel_power,
    clr_cmc_up_channel_power_cmd,
    "undo cmc upstream channel [all|<1-4>] power-level",
    NO_STR
    SET_CMC_STR
    SET_CMC_UPSTREAM_STR
    SET_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "restore up-channel's power level\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  channel_id;
    short int channel_power;
    ULONG ulIfIndex;
    ULONG ulSuffix;

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
    channel_power = 100;

    ulIfIndex = (ULONG) vty->index;
    if(!IsProfileNodeVty(ulIfIndex, &ulSuffix))
    {
        /* ≈‰÷√∂‘œÛ∂¡–¥ */
    	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
    	{
    		return CMD_WARNING;
    	}

    	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
    	{
    		vty_out(vty, "failed to get cmc mac-address\r\n");
    		return CMD_WARNING;
    	}  

    	if (0 != OnuMgt_SetCmcUpChannelPower(PonPortIdx, OnuIdx, cmcMac, channel_id, channel_power))
    	{
    		vty_out(vty, "failed to set channel's power level\r\n");
    		return CMD_WARNING;
    	}
    }
    else
    {
        /* ≈‰÷√Œƒº˛∂¡–¥ */
        ULONG ulValue;
        USHORT usPort;

        usPort = CMC_UPCHANNEL2PORT(channel_id);
        ulValue = channel_power;
        if ( 0 != setOnuConfPortSimpleVarByPtr(ulSuffix, vty->onuconfptr, usPort, sv_enum_cmc_channel_power, ulValue))
    	{
    		vty_out(vty, "failed to set channel's power level\r\n");
    		return CMD_WARNING;
    	}
    }

	return CMD_SUCCESS;
} 

DEFUN (
    show_cmc_up_channel_power,
    show_cmc_up_channel_power_cmd,
    "show cmc upstream channel [all|<1-4>] power-level",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_UPSTREAM_STR
    SHOW_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "show up-channel's power level\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char   channel_id;
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
	if (0 != OnuMgt_DumpCmcUpChannelPower(PonPortIdx, OnuIdx, cmcMac, channel_id, szDumpOutput, &length))
	{
		vty_out(vty, "failed to get channel power\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 


DEFUN (
    show_cmc_up_channel_signal,
    show_cmc_up_channel_signal_cmd,
    "show cmc upstream channel [all|<1-4>] signal-quality",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_UPSTREAM_STR
    SHOW_CMC_CHANNEL_STR
    SEL_CMC_CHANNEL_ALL_STR 
    SEL_CMC_CHANNEL_STR 
    "show up-channel's signal quality\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char   channel_id;
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'a' == argv[0][0] )
    {
        channel_id = CMC_CHANNELID_ALL;
    }
    else
    {
        channel_id = (unsigned char)VOS_AtoI(argv[0]);
    }
    
	if (0 != OnuMgt_DumpCmcUpChannelSignalQuality(PonPortIdx, OnuIdx, cmcMac, channel_id, szDumpOutput, &length))
	{
		vty_out(vty, "failed to get channel signal-info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    show_cmc_up_interface_stat,
    show_cmc_up_interface_stat_cmd,
    "show cmc upstream interface <1-4> [statistics|utilization]",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_UPSTREAM_STR
    SHOW_CMC_IF_STR 
    "please input the interface index\n" 
    "show cmc-interface's statistics information\n"
    "show cmc-interface's utilization information\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char   channel_type;
    unsigned char   channel_id;
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    channel_type = 2;
    channel_id   = (unsigned char)VOS_AtoI(argv[0]);

    if ( 's' == argv[1][0] )
    {
        result = OnuMgt_DumpCmcInterfaceStatistics(PonPortIdx, OnuIdx, cmcMac, channel_type, channel_id, szDumpOutput, &length);
    }
    else
    {
        result = OnuMgt_DumpCmcInterfaceUtilization(PonPortIdx, OnuIdx, cmcMac, channel_type, channel_id, szDumpOutput, &length);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to get channel stat-info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    show_cmc_down_interface_stat,
    show_cmc_down_interface_stat_cmd,
    "show cmc downstream interface <1-16> [statistics|utilization]",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_DOWNSTREAM_STR
    SHOW_CMC_IF_STR 
    "please input the interface index\n" 
    "show cmc-interface's statistics information\n"
    "show cmc-interface's utilization information\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char   channel_type;
    unsigned char   channel_id;
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    channel_type = 1;
    channel_id   = (unsigned char)VOS_AtoI(argv[0]);

    if ( 's' == argv[1][0] )
    {
        result = OnuMgt_DumpCmcInterfaceStatistics(PonPortIdx, OnuIdx, cmcMac, channel_type, channel_id, szDumpOutput, &length);
    }
    else
    {
        result = OnuMgt_DumpCmcInterfaceUtilization(PonPortIdx, OnuIdx, cmcMac, channel_type, channel_id, szDumpOutput, &length);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to get channel stat-info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 


DEFUN (
    show_cmc_mac_summary,
    show_cmc_mac_summary_cmd,
    "show cmc mac summary",
    SHOW_STR
    SHOW_CMC_STR
    "show the cmc's mac summary\n" 
    "show the cmc's mac summary\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
	if (0 != OnuMgt_DumpCmcMacStatistics(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length))
	{
		vty_out(vty, "failed to get cmc mac-info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    show_cmc_interface_summary,
    show_cmc_interface_summary_cmd,
    "show cmc interface [summary|statistics|utilization]",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_IF_STR 
    "show the cmc's all of interfaces summary\n" 
    "show the cmc's all of interfaces statistics\n" 
    "show the cmc's all of interfaces utilization\n" 
    )
{
    int  result;
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[18432] = "";      /* 18K's StackSize */
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'u' == argv[0][0] )
    {
        result = OnuMgt_DumpCmcInterfaceUtilization(PonPortIdx, OnuIdx, cmcMac, 0, 0, szDumpOutput, &length);
    }
    else
    {
        if ( 'u' == argv[0][1] )
        {
            result = OnuMgt_DumpCmcAllInterface(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length);
        }
        else
        {
            result = OnuMgt_DumpCmcInterfaceStatistics(PonPortIdx, OnuIdx, cmcMac, 0, 0, szDumpOutput, &length);
        }
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to get interface information\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 
#endif


#if 1
/* --------------------CMC∆µµ¿◊Èπ‹¿Ì------------------- */
DEFUN (
    show_cmc_group_dynamic_config,
    show_cmc_group_dynamic_config_cmd,
    "show cmc load-balance dynamic config",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_LBCFG_STR
    SHOW_CMC_GROUP_DYNAMIC_STR
    SHOW_CMC_GROUP_DYNAMIC_STR
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
	if (0 != OnuMgt_DumpCmcLoadBalancingDynConfig(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length))
	{
		vty_out(vty, "failed to get load-balance dynamic config\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_method,
    set_cmc_group_dynamic_method_cmd,
    "cmc load-balance dynamic method [disable|dynamic|static]",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance method\n"
    "close the dynamic group function\n"
    "open the dynamic group function\n"
	"only use the static group\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   method;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 's' == argv[0][0] )
    {
        method = CMC_CFG_GROUP_DYNAMIC_METHOD_STATIC;
    }
    else
    {
        if ( 'i' == argv[0][1] )
        {
            method = CMC_CFG_GROUP_DYNAMIC_METHOD_DISABLED;
        }
        else
        {
            method = CMC_CFG_GROUP_DYNAMIC_METHOD_DYNAMIC;
        }
    }
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynMethod(PonPortIdx, OnuIdx, cmcMac, method))
	{
		vty_out(vty, "failed to set load-balance dynamic method\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_method,
    clr_cmc_group_dynamic_method_cmd,
    "undo cmc load-balance dynamic method",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance method\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   method;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    method = CMC_CFG_GROUP_DYNAMIC_METHOD_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynMethod(PonPortIdx, OnuIdx, cmcMac, method))
	{
		vty_out(vty, "failed to set load-balance dynamic method\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_period,
    set_cmc_group_dynamic_period_cmd,
    "cmc load-balance dynamic period <30-3600>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance period\n"
    "please input the time span(unit:second)\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned long   period;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    period = VOS_StrToUL(argv[0], NULL, 10);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynPeriod(PonPortIdx, OnuIdx, cmcMac, period))
	{
		vty_out(vty, "failed to set load-balance dynamic period\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_period,
    clr_cmc_group_dynamic_period_cmd,
    "undo cmc load-balance dynamic period",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance period\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned long   period;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    period = CMC_CFG_GROUP_DYNAMIC_PERIOD_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynPeriod(PonPortIdx, OnuIdx, cmcMac, period))
	{
		vty_out(vty, "failed to set load-balance dynamic period\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_weight_period,
    set_cmc_group_dynamic_weight_period_cmd,
    "cmc load-balance dynamic weighted-average-period [1|2|4]",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance weighted average period(unit:second)\n"
    "1 second\n" 
    "2 seconds\n" 
    "4 seconds\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned long   period;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    period = VOS_StrToUL(argv[0], NULL, 10);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynWeightedAveragePeriod(PonPortIdx, OnuIdx, cmcMac, period))
	{
		vty_out(vty, "failed to set load-balance dynamic weighted-average-period\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_weight_period,
    clr_cmc_group_dynamic_weight_period_cmd,
    "undo cmc load-balance dynamic weighted-average-period",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance weighted average period\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned long   period;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    period = CMC_CFG_GROUP_DYNAMIC_WEIGHT_PERIOD_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynWeightedAveragePeriod(PonPortIdx, OnuIdx, cmcMac, period))
	{
		vty_out(vty, "failed to set load-balance dynamic weighted-average-period\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_overload_threshold,
    set_cmc_group_dynamic_overload_threshold_cmd,
    "cmc load-balance dynamic overload-threshold-percent <0-100>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance overload threshold percent\n"
    "please input the percent\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   percent;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    percent = (unsigned char)VOS_AtoI(argv[0]);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynOverloadThresold(PonPortIdx, OnuIdx, cmcMac, percent))
	{
		vty_out(vty, "failed to set load-balance dynamic overload-threshold-percent\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_overload_threshold,
    clr_cmc_group_dynamic_overload_threshold_cmd,
    "undo cmc load-balance dynamic overload-threshold-percent",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance overload threshold percent\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   percent;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    percent = CMC_CFG_GROUP_DYNAMIC_OVERLOAD_THRESHOLD_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynOverloadThresold(PonPortIdx, OnuIdx, cmcMac, percent))
	{
		vty_out(vty, "failed to set load-balance dynamic overload-threshold-percent\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_difference_threshold,
    set_cmc_group_dynamic_difference_threshold_cmd,
    "cmc load-balance dynamic difference-threshold-percent <0-100>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance difference threshold percent\n"
    "please input the percent\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   percent;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    percent = (unsigned char)VOS_AtoI(argv[0]);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynDifferenceThresold(PonPortIdx, OnuIdx, cmcMac, percent))
	{
		vty_out(vty, "failed to set load-balance dynamic difference-threshold-percent\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_difference_threshold,
    clr_cmc_group_dynamic_difference_threshold_cmd,
    "undo cmc load-balance dynamic difference-threshold-percent",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance difference threshold percent\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   percent;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    percent = CMC_CFG_GROUP_DYNAMIC_DIFFERENCE_THRESHOLD_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynDifferenceThresold(PonPortIdx, OnuIdx, cmcMac, percent))
	{
		vty_out(vty, "failed to set load-balance dynamic difference-threshold-percent\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_max_moves,
    set_cmc_group_dynamic_max_moves_cmd,
    "cmc load-balance dynamic max-moves <1-255>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance maximum number of modems's moves per period\n"
    "please input the modems number\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned long   move_number;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    move_number = VOS_StrToUL(argv[0], NULL, 10);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynMaxMoveNumber(PonPortIdx, OnuIdx, cmcMac, move_number))
	{
		vty_out(vty, "failed to set load-balance dynamic max-moves\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_max_moves,
    clr_cmc_group_dynamic_max_moves_cmd,
    "undo cmc load-balance dynamic max-moves",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance maximum number of modems's moves per period\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned long   move_number;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    move_number = CMC_CFG_GROUP_DYNAMIC_MOVE_MAX_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynMaxMoveNumber(PonPortIdx, OnuIdx, cmcMac, move_number))
	{
		vty_out(vty, "failed to set load-balance dynamic max-moves\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_min_hold,
    set_cmc_group_dynamic_min_hold_cmd,
    "cmc load-balance dynamic min-hold-time <1-3600>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance minimum hold time per modem\n"
    "please input the time span(unit:second)\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned long   hold_time;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    hold_time = VOS_StrToUL(argv[0], NULL, 10);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynMinHoldTime(PonPortIdx, OnuIdx, cmcMac, hold_time))
	{
		vty_out(vty, "failed to set load-balance dynamic min-hold-time\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_min_hold,
    clr_cmc_group_dynamic_min_hold_cmd,
    "undo cmc load-balance dynamic min-hold-time",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance minimum hold time per modem\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned long   hold_time;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    hold_time = CMC_CFG_GROUP_DYNAMIC_HOLDTIME_MIN_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynMinHoldTime(PonPortIdx, OnuIdx, cmcMac, hold_time))
	{
		vty_out(vty, "failed to set load-balance dynamic min-hold-time\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_group_dynamic_range_mode,
    set_cmc_group_dynamic_range_mode_cmd,
    "cmc load-balance dynamic ranging-override-mode [enable|disable]",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance ranging override mode\n"
    "open dynamic load balance group's ranging override function\n" 
    "close dynamic load balance group's ranging override function\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  range_mode;

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'e' == argv[0][0] )
    {
        range_mode = CMC_CFG_GROUP_DYNAMIC_RANGE_OVERRIDE_ENABLED;
    }
    else
    {
        range_mode = CMC_CFG_GROUP_DYNAMIC_RANGE_OVERRIDE_DISABLED;
    }
  
	if (0 != OnuMgt_SetCmcLoadBalancingDynRangeOverrideMode(PonPortIdx, OnuIdx, cmcMac, range_mode))
	{
		vty_out(vty, "failed to set load-balance dynamic ranging-override-mode\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    set_cmc_group_dynamic_atdma_dcc,
    set_cmc_group_dynamic_atdma_dcc_cmd,
    "cmc load-balance dynamic atdma-dcc-init-tech <1-4>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance DCC init-tech for ATDMA mode\n"
    "please input the tech number\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   dcc_tech;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    dcc_tech = (unsigned char)VOS_AtoI(argv[0]);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynAtdmaDccInitTech(PonPortIdx, OnuIdx, cmcMac, dcc_tech))
	{
		vty_out(vty, "failed to set load-balance dynamic atdma-dcc-init-tech\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_atdma_dcc,
    clr_cmc_group_dynamic_atdma_dcc_cmd,
    "undo cmc load-balance dynamic atdma-dcc-init-tech",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance DCC init-tech for ATDMA mode\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   dcc_tech;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    dcc_tech = CMC_CFG_GROUP_DYNAMIC_ATDMA_DCC_INIT_TECH_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynAtdmaDccInitTech(PonPortIdx, OnuIdx, cmcMac, dcc_tech))
	{
		vty_out(vty, "failed to set load-balance dynamic atdma-dcc-init-tech\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_scdma_dcc,
    set_cmc_group_dynamic_scdma_dcc_cmd,
    "cmc load-balance dynamic scdma-dcc-init-tech <1-3>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance DCC init-tech for SCDMA mode\n"
    "please input the tech number\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   dcc_tech;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    dcc_tech = (unsigned char)VOS_AtoI(argv[0]);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynScdmaDccInitTech(PonPortIdx, OnuIdx, cmcMac, dcc_tech))
	{
		vty_out(vty, "failed to set load-balance dynamic scdma-dcc-init-tech\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_scdma_dcc,
    clr_cmc_group_dynamic_scdma_dcc_cmd,
    "undo cmc load-balance dynamic scdma-dcc-init-tech",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance DCC init-tech for SCDMA mode\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   dcc_tech;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    dcc_tech = CMC_CFG_GROUP_DYNAMIC_SCDMA_DCC_INIT_TECH_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynScdmaDccInitTech(PonPortIdx, OnuIdx, cmcMac, dcc_tech))
	{
		vty_out(vty, "failed to set load-balance dynamic scdma-dcc-init-tech\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_atdma_dbc,
    set_cmc_group_dynamic_atdma_dbc_cmd,
    "cmc load-balance dynamic atdma-dbc-init-tech <1-4>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance DBC init-tech for ATDMA mode\n"
    "please input the tech number\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   dbc_tech;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    dbc_tech = (unsigned char)VOS_AtoI(argv[0]);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynAtdmaDbcInitTech(PonPortIdx, OnuIdx, cmcMac, dbc_tech))
	{
		vty_out(vty, "failed to set load-balance dynamic atdma-dbc-init-tech\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_atdma_dbc,
    clr_cmc_group_dynamic_atdma_dbc_cmd,
    "undo cmc load-balance dynamic atdma-dbc-init-tech",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance DBC init-tech for ATDMA mode\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   dbc_tech;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    dbc_tech = CMC_CFG_GROUP_DYNAMIC_ATDMA_DBC_INIT_TECH_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynAtdmaDbcInitTech(PonPortIdx, OnuIdx, cmcMac, dbc_tech))
	{
		vty_out(vty, "failed to set load-balance dynamic atdma-dbc-init-tech\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    set_cmc_group_dynamic_scdma_dbc,
    set_cmc_group_dynamic_scdma_dbc_cmd,
    "cmc load-balance dynamic scdma-dbc-init-tech <1-3>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "config dynamic load balance DBC init-tech for SCDMA mode\n"
    "please input the tech number\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   dbc_tech;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    dbc_tech = (unsigned char)VOS_AtoI(argv[0]);
    
	if (0 != OnuMgt_SetCmcLoadBalancingDynScdmaDbcInitTech(PonPortIdx, OnuIdx, cmcMac, dbc_tech))
	{
		vty_out(vty, "failed to set load-balance dynamic scdma-dbc-init-tech\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_dynamic_scdma_dbc,
    clr_cmc_group_dynamic_scdma_dbc_cmd,
    "undo cmc load-balance dynamic scdma-dbc-init-tech",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_DYNAMIC_STR
    "restore dynamic load balance DBC init-tech for SCDMA mode\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   dbc_tech;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    dbc_tech = CMC_CFG_GROUP_DYNAMIC_SCDMA_DBC_INIT_TECH_DEFAULT;
	if (0 != OnuMgt_SetCmcLoadBalancingDynScdmaDbcInitTech(PonPortIdx, OnuIdx, cmcMac, dbc_tech))
	{
		vty_out(vty, "failed to set load-balance dynamic scdma-dbc-init-tech\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    show_cmc_group,
    show_cmc_group_cmd,
    "show cmc load-balance group [all|<1-255>]",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_LBCFG_STR
    SHOW_CMC_GROUP_STR
    SEL_CMC_GROUP_ALL_STR
    SEL_CMC_GROUP_STR
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   group_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'a' == argv[0][0] )
    {
        result = OnuMgt_DumpCmcAllLoadBalancingGrp(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length);
    }
    else
    {
        group_id = (unsigned char)VOS_AtoI(argv[0]);
        result = OnuMgt_DumpCmcLoadBalancingGrp(PonPortIdx, OnuIdx, cmcMac, group_id, szDumpOutput, &length);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to get load-balance group info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    show_cmc_group_stream,
    show_cmc_group_stream_cmd,
    "show cmc load-balance group <1-255> [downstream|upstream]",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_LBCFG_STR
    SHOW_CMC_GROUP_STR
    SEL_CMC_GROUP_STR
    "show group's downstram channel setting\n" 
    "show group's upstream channel setting\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   group_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
    group_id = (unsigned char)VOS_AtoI(argv[0]);
    if ( 'd' == argv[1][0] )
    {
        result = OnuMgt_DumpCmcLoadBalancingGrpDownstream(PonPortIdx, OnuIdx, cmcMac, group_id, szDumpOutput, &length);
    }
    else
    {
        result = OnuMgt_DumpCmcLoadBalancingGrpUpstream(PonPortIdx, OnuIdx, cmcMac, group_id, szDumpOutput, &length);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to get load-balance group stream-info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 


DEFUN (
    new_cmc_group,
    new_cmc_group_cmd,
    "cmc load-balance group <1-255> [static|dynamic]",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_STR
    SEL_CMC_GROUP_STR
    "create a static load balance group\n"
    "create a dynamic load balance groups\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   group_id;
    unsigned char   group_method;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    group_id = (unsigned char)VOS_AtoI(argv[0]);

    if ( 's' == argv[1][0] )
    {
        group_method = 1;
    }
    else
    {
        group_method = 2;
    }
    
	if (0 != OnuMgt_CreateCmcLoadBalancingGrp(PonPortIdx, OnuIdx, cmcMac, group_id, group_method))
	{
		vty_out(vty, "failed to create a load-balance group\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group,
    clr_cmc_group_cmd,
    "undo cmc load-balance group <1-255>",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_STR
    SEL_CMC_GROUP_STR
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   group_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    group_id = (unsigned char)VOS_AtoI(argv[0]);
    
	if (0 != OnuMgt_DestroyCmcLoadBalancingGrp(PonPortIdx, OnuIdx, cmcMac, group_id))
	{
		vty_out(vty, "failed to destroy a load-balance group\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    add_cmc_group_channel,
    add_cmc_group_channel_cmd,
    "cmc load-balance group <1-255> add [downstream|upstream] <channel_list>",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_STR
    SEL_CMC_GROUP_STR
    "add channel(s) to a load balance group\n"
    "add downstream channel(s) to a load balance group\n"
    "add upstream channel(s) to a load balance group\n"
    "please input the channel list\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   group_id;
    unsigned char   group_method;
    unsigned char   channel_num;
    unsigned char   channel_ids[16];
    unsigned long   channel_id;
    unsigned long   channel_max;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    group_id = (unsigned char)VOS_AtoI(argv[0]);

    if ( 'd' == argv[1][0] )
    {
        group_method = 1;
        channel_max = 16;
    }
    else
    {
        group_method = 2;
        channel_max = 4;
    }

    channel_num = 0;
    BEGIN_PARSE_ID_LIST_TO_ID( argv[2], channel_id, 1, channel_max, channel_max )
    {
        channel_ids[channel_num++] = (unsigned char)channel_id;
    }
    END_PARSE_ONUID_LIST_TO_ONUID()    

    if ( 0 == channel_num )
    {
		vty_out(vty, "upstream's channel id is only 1-4.\r\n");
		return CMD_WARNING;
    }
    
    if ( 'd' == argv[1][0] )
    {
        result = OnuMgt_AddCmcLoadBalancingGrpDownstream(PonPortIdx, OnuIdx, cmcMac, group_id, channel_num, channel_ids);
    }
    else
    {
        result = OnuMgt_AddCmcLoadBalancingGrpUpstream(PonPortIdx, OnuIdx, cmcMac, group_id, channel_num, channel_ids);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to add a load-balance group's channels\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_channel,
    clr_cmc_group_channel_cmd,
    "undo cmc load-balance group <1-255> add [downstream|upstream] <channel_list>",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_STR
    SEL_CMC_GROUP_STR
    "remove channel(s) from a load balance group\n"
    "remove downstream channel(s) from a load balance group\n"
    "remove upstream channel(s) from a load balance group\n"
    "please input the channel list\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   group_id;
    unsigned char   group_method;
    unsigned char   channel_num;
    unsigned char   channel_ids[16];
    unsigned long   channel_id;
    unsigned long   channel_max;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    group_id = (unsigned char)VOS_AtoI(argv[0]);

    if ( 'd' == argv[1][0] )
    {
        group_method = 1;
        channel_max = 16;
    }
    else
    {
        group_method = 2;
        channel_max = 4;
    }

    channel_num = 0;
    BEGIN_PARSE_ID_LIST_TO_ID( argv[2], channel_id, 1, channel_max, channel_max )
    {
        channel_ids[channel_num++] = (unsigned char)channel_id;
    }
    END_PARSE_ONUID_LIST_TO_ONUID()    

    if ( 0 == channel_num )
    {
		vty_out(vty, "upstream's channel id is only 1-4.\r\n");
		return CMD_WARNING;
    }
    
    if ( 'd' == argv[1][0] )
    {
        result = OnuMgt_RemoveCmcLoadBalancingGrpDownstream(PonPortIdx, OnuIdx, cmcMac, group_id, channel_num, channel_ids);
    }
    else
    {
        result = OnuMgt_RemoveCmcLoadBalancingGrpUpstream(PonPortIdx, OnuIdx, cmcMac, group_id, channel_num, channel_ids);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to remove a load-balance group's channels\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    show_cmc_group_modem,
    show_cmc_group_modem_cmd,
    "show cmc load-balance group <1-255> [modem|actived-modem]",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_LBCFG_STR
    SHOW_CMC_GROUP_STR
    SEL_CMC_GROUP_STR
    "show the load balance group's cable modem settings\n" 
    "show active cable modem(s) in the load balance group\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   group_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
    group_id = (unsigned char)VOS_AtoI(argv[0]);
    if ( 'm' == argv[1][0] )
    {
        result = OnuMgt_DumpCmcLoadBalancingGrpModem(PonPortIdx, OnuIdx, cmcMac, group_id, szDumpOutput, &length);
    }
    else
    {
        result = OnuMgt_DumpCmcLoadBalancingGrpActivedModem(PonPortIdx, OnuIdx, cmcMac, group_id, szDumpOutput, &length);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to get load-balance group modem-info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    add_cmc_group_modem,
    add_cmc_group_modem_cmd,
    "cmc load-balance group <1-255> add modem <H.H.H> {<H.H.H>}*1",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_STR
    SEL_CMC_GROUP_STR
    "add modem(s) to a load balance group\n"
    "add modem(s) by OUI or range of mac address\n"
    "add modem(s) by one mac address or start of mac address or OUI range(OUI such as: XXXX.XX00.0000)\n"
    "add modem(s) by end of mac address\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   group_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char startMac[BYTES_IN_MAC_ADDRESS];
    char endMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    group_id = (unsigned char)VOS_AtoI(argv[0]);

    if ( GetMacAddr( ( CHAR* ) argv[ 1 ], startMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

    if ( argc > 2 )
    {
        if ( GetMacAddr( ( CHAR* ) argv[ 2 ], endMac ) != VOS_OK )
        {
            vty_out( vty, "  %% Invalid MAC address.\r\n" );
            return CMD_WARNING;
        }
    }
    else
    {
        VOS_MemCpy(endMac, startMac, BYTES_IN_MAC_ADDRESS);
    }
    
	if (0 != OnuMgt_AddCmcLoadBalancingGrpModem(PonPortIdx, OnuIdx, cmcMac, group_id, startMac, endMac))
	{
		vty_out(vty, "failed to add a load-balance group's modems\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_modem,
    clr_cmc_group_modem_cmd,
    "undo cmc load-balance group <1-255> add modem <H.H.H> {<H.H.H>}",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_STR
    SEL_CMC_GROUP_STR
    "remove modem(s) from a load balance group\n"
    "remove modem(s) by OUI or range of mac address\n"
    "remove modem(s) by one mac address or start of mac address or OUI range(OUI such as: XXXX.XX00.0000)\n"
    "remove modem(s) by end of mac address\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned char   group_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char startMac[BYTES_IN_MAC_ADDRESS];
    char endMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    group_id = (unsigned char)VOS_AtoI(argv[0]);

    if ( GetMacAddr( ( CHAR* ) argv[ 1 ], startMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

    if ( argc > 2 )
    {
        if ( GetMacAddr( ( CHAR* ) argv[ 2 ], endMac ) != VOS_OK )
        {
            vty_out( vty, "  %% Invalid MAC address.\r\n" );
            return CMD_WARNING;
        }
    }
    else
    {
        VOS_MemCpy(endMac, startMac, BYTES_IN_MAC_ADDRESS);
    }
    
	if (0 != OnuMgt_RemoveCmcLoadBalancingGrpModem(PonPortIdx, OnuIdx, cmcMac, group_id, startMac, endMac))
	{
		vty_out(vty, "failed to remove a load-balance group's modems\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 


DEFUN (
    show_cmc_group_exclude_modem,
    show_cmc_group_exclude_modem_cmd,
    "show cmc load-balance group [exclude-modem|exclude-actived-modem]",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_LBCFG_STR
    SHOW_CMC_GROUP_STR
    "show load balance group's excluded cable modem settings\n" 
    "show load balance group's active excluded cable modem(s)\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  
    
    if ( 0 == VOS_StrCmp(argv[0], "exclude-modem") )
    {
        result = OnuMgt_DumpCmcLoadBalancingGrpExcludeModem(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length);
    }
    else
    {
        result = OnuMgt_DumpCmcLoadBalancingGrpExcludeActivedModem(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to get load-balance group modem-info\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 

DEFUN (
    add_cmc_group_exclude_modem,
    add_cmc_group_exclude_modem_cmd,
    "cmc load-balance group exclude modem <H.H.H> {<H.H.H>}*1",
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_STR
    "exclude modem(s) to load balance group\n"
    "exclude modem(s) by OUI or range of mac address\n"
    "exclude modem(s) by one mac address or start of mac address or OUI range(OUI such as: XXXX.XX00.0000)\n"
    "exclude modem(s) by end of mac address\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char startMac[BYTES_IN_MAC_ADDRESS];
    char endMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], startMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

    if ( argc > 1 )
    {
        if ( GetMacAddr( ( CHAR* ) argv[ 1 ], endMac ) != VOS_OK )
        {
            vty_out( vty, "  %% Invalid MAC address.\r\n" );
            return CMD_WARNING;
        }
    }
    else
    {
        VOS_MemCpy(endMac, startMac, BYTES_IN_MAC_ADDRESS);
    }
    
	if (0 != OnuMgt_AddCmcLoadBalancingGrpExcludeModem(PonPortIdx, OnuIdx, cmcMac, startMac, endMac))
	{
		vty_out(vty, "failed to add load-balance group's exclude modems\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 

DEFUN (
    clr_cmc_group_exclude_modem,
    clr_cmc_group_exclude_modem_cmd,
    "undo cmc load-balance group exclude modem <H.H.H> {<H.H.H>}",
    NO_STR
    SET_CMC_STR
    SET_CMC_GROUP_STR
    SET_CMC_GROUP_STR
    "remove modem(s) from load balance group exclude list\n"
    "exclude modem(s) by OUI or range of mac address\n"
    "exclude modem(s) by one mac address or start of mac address or OUI range(OUI such as: XXXX.XX00.0000)\n"
    "exclude modem(s) by end of mac address\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char startMac[BYTES_IN_MAC_ADDRESS];
    char endMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], startMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

    if ( argc > 1 )
    {
        if ( GetMacAddr( ( CHAR* ) argv[ 1 ], endMac ) != VOS_OK )
        {
            vty_out( vty, "  %% Invalid MAC address.\r\n" );
            return CMD_WARNING;
        }
    }
    else
    {
        VOS_MemCpy(endMac, startMac, BYTES_IN_MAC_ADDRESS);
    }
    
	if (0 != OnuMgt_RemoveCmcLoadBalancingGrpExcludeModem(PonPortIdx, OnuIdx, cmcMac, startMac, endMac))
	{
		vty_out(vty, "failed to remove load-balance group's exclude modems\r\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
} 
#endif


#if 1
/* --------------------CMπ‹¿Ì------------------- */
DEFUN (
    show_cm,
    show_cm_cmd,
    "show cable-modem [all|<H.H.H>]",
    SHOW_STR
    SHOW_CM_STR
    SEL_CM_ALL_STR
    SEL_CM_STR
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
		return CMD_WARNING;    	

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'a' == argv[0][0] )
    {
        result = OnuMgt_DumpAllCm(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length);
    }
    else
    {
        if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
        {
            vty_out( vty, "  %% Invalid MAC address.\r\n" );
            return CMD_WARNING;
        }
        
        result = OnuMgt_DumpCm(PonPortIdx, OnuIdx, cmcMac, cmMac, szDumpOutput, &length);
    }

	if (0 != result)
	{
		vty_out(vty, "failed to get cm infos\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
}   

DEFUN (
    show_cm_history,
    show_cm_history_cmd,
    "show cable-modem [all|<H.H.H>] history",
    SHOW_STR
    SHOW_CM_STR
    SEL_CM_ALL_STR
    SEL_CM_STR
    "show cable-modem's registed history\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
		return CMD_WARNING;    	

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'a' == argv[0][0] )
    {
        result = OnuMgt_DumpAllCmHistory(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length);
    }
    else
    {
        if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
        {
            vty_out( vty, "  %% Invalid MAC address.\r\n" );
            return CMD_WARNING;
        }
        
        result = OnuMgt_DumpCmHistory(PonPortIdx, OnuIdx, cmcMac, cmMac, szDumpOutput, &length);
    }

	if (0 != result)
	{
		vty_out(vty, "failed to get history infos\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
}   

DEFUN (
    clr_cm_history,
    clr_cm_history_cmd,
    "clear cable-modem history",
    CLEAR_STR
    "clear all of cable-modem's registed history\n"
    "clear all of cable-modem's registed history\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
		return CMD_WARNING;    	

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    result = OnuMgt_ClearAllCmHistory(PonPortIdx, OnuIdx, cmcMac);
	if (0 != result)
	{
		vty_out(vty, "failed to clear history infos\r\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}   


DEFUN (
    reset_cm,
    reset_cm_cmd,
    "reset cable-modem [all|<H.H.H>]",
    CLEAR_STR
    "reset cable modem(s)"
    SEL_CM_ALL_STR
    SEL_CM_STR
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];

    if ( 'a' == argv[0][0] )
    {
        VOS_MemSet(cmMac, 0xFF, BYTES_IN_MAC_ADDRESS);
    }
    else
    {
        if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
        {
            vty_out( vty, "  %% Invalid MAC address.\r\n" );
            return CMD_WARNING;
        }
    }

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

	if (0 != OnuMgt_ResetCm(PonPortIdx, OnuIdx, cmcMac, cmMac))
	{
		vty_out(vty, "failed to reset cm\r\n");
		return CMD_WARNING;
	}
    
	vty_out(vty, "CM reset OK!\r\n");
	
	return CMD_SUCCESS;
}


DEFUN (
    show_cm_channel,
    show_cm_channel_cmd,
    "show cable-modem <H.H.H> channels [downstream|upstream]",
    SHOW_STR
    SHOW_CM_STR
    SEL_CM_STR
    "show cable-modem's binded channels\n"
    "show cable-modem's downstream channels\n"
    "show cable-modem's upstream channels\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
		return CMD_WARNING;    	

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

    if ( 'd' == argv[1][0] )
    {
        result = OnuMgt_DumpCmDownstream(PonPortIdx, OnuIdx, cmcMac, cmMac, szDumpOutput, &length);
    }
    else
    {
        
        result = OnuMgt_DumpCmUpstream(PonPortIdx, OnuIdx, cmcMac, cmMac, szDumpOutput, &length);
    }

	if (0 != result)
	{
		vty_out(vty, "failed to get cm's channel infos\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
}   


DEFUN (
    show_cm_qos,
    show_cm_qos_cmd,
    "show cable-modem <H.H.H> qos [classifier|service-flow]",
    SHOW_STR
    SHOW_CM_STR
    SEL_CM_STR
    "show cable-modem's qos information\n"
    "show cable-modem's classifier\n"
    "show cable-modem's service-flow\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
		return CMD_WARNING;    	

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

    if ( 'c' == argv[1][0] )
    {
        result = OnuMgt_DumpCmClassifier(PonPortIdx, OnuIdx, cmcMac, cmMac, szDumpOutput, &length);
    }
    else
    {
        
        result = OnuMgt_DumpCmServiceFlow(PonPortIdx, OnuIdx, cmcMac, cmMac, szDumpOutput, &length);
    }

	if (0 != result)
	{
		vty_out(vty, "failed to get cm's qos infos\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
}   

DEFUN (
    set_cm_channel,
    set_cm_channel_cmd,
    "cable-modem <H.H.H> [dcc|ucc] <channel_list>",
    SET_CM_STR
    SEL_CM_STR
    "config cable modem's downstream channel(s)"
    "config cable modem's upstream channel(s)"
    "please input the channel list\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];
    unsigned char   channel_num;
    unsigned char   channel_ids[16];
    unsigned long   channel_id;
    unsigned long   channel_max;

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'd' == argv[1][0] )
    {
        channel_max = 16;
    }
    else
    {
        channel_max = 4;
    }

    channel_num = 0;
    BEGIN_PARSE_ID_LIST_TO_ID( argv[2], channel_id, 1, channel_max, channel_max )
    {
        channel_ids[channel_num++] = (unsigned char)channel_id;
    }
    END_PARSE_ONUID_LIST_TO_ONUID()    

    if ( 0 == channel_num )
    {
		vty_out(vty, "upstream's channel id is only 1-4.\r\n");
		return CMD_WARNING;
    }
    
    if ( 'd' == argv[1][0] )
    {
        result = OnuMgt_SetCmDownstream(PonPortIdx, OnuIdx, cmcMac, cmMac, channel_num, channel_ids);
    }
    else
    {
        result = OnuMgt_SetCmUpstream(PonPortIdx, OnuIdx, cmcMac, cmMac, channel_num, channel_ids);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to set cm's channel\r\n");
		return CMD_WARNING;
	}
    

	return CMD_SUCCESS;
}

DEFUN (
    new_cm_sf,
    new_cm_sf_cmd,
    "cable-modem <H.H.H> qos service-flow create <0-7> <filename>",
    SET_CM_STR
    SEL_CM_STR
    "config cable modem's qos"
    "config a service flow"
    "create a service flow"
    "the service flow's COS value\n" 
    "qos tlv file\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];
    char szFileName[256];
    char tlv_buffer[2048];
    unsigned char   cos_value;
    unsigned char  *tlv_data;
    unsigned short  tlv_len;
    long            lFileLen;
    unsigned long   ulFileDataLen;

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    VOS_Snprintf(szFileName, 250, "/flash/sys/%s", argv[2]);
    if ( VOS_ERROR == CPI_HOOK_CALL( cdsms_file_exist_fs )(szFileName) )
    {
        vty_out( vty, "\r\n%% The specified file does not exist.\r\n" );

        return CMD_WARNING;
    }

    if ( VOS_ERROR == (lFileLen = CPI_HOOK_CALL( cdsms_file_length_get_fs )(szFileName) ) )
    {
        vty_out( vty, "\r\n%% The specified file's length get error.\r\n" );

        return CMD_WARNING;
    }
    else if ( (lFileLen < CMC_MIN_TLV_FILE_LENGTH) || (lFileLen > CMC_MAX_TLV_FILE_LENGTH) )
    {
        vty_out( vty, "\r\n%% The specified file's length(%lu) is invalid<%lu-%lu>.\r\n", lFileLen, CMC_MIN_TLV_FILE_LENGTH, CMC_MAX_TLV_FILE_LENGTH );

        return CMD_WARNING;
    }

    if ( lFileLen < sizeof(tlv_buffer) )
    {
        tlv_data = tlv_buffer;
    }
    else
    {
        if ( NULL == (tlv_data = (unsigned char *)g_malloc( lFileLen )) )
        {
            vty_out( vty, "\r\n%% The file's length(%lu) is too large to handle.\r\n", lFileLen );

            return CMD_WARNING;
        }
    }

    if ( 0 == CPI_HOOK_CALL( cdsms_file_read_fs ) (szFileName, tlv_data, &ulFileDataLen) )
    {
        tlv_len = (unsigned short)ulFileDataLen;
    }
    else
    {
        if ( tlv_data != tlv_buffer )
        {
            g_free(tlv_data);
        }
    
        vty_out( vty, "\r\n%% The specified file's data read(%lu) error.\r\n", lFileLen );

        return CMD_WARNING;
    }

    cos_value = (unsigned char)VOS_AtoI(argv[1]);

    result = OnuMgt_CreateCmServiceFlow(PonPortIdx, OnuIdx, cmcMac, cmMac, cos_value, tlv_data, tlv_len);

    if ( tlv_data != tlv_buffer )
    {
        g_free(tlv_data);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to create cm's service-flow\r\n");
		return CMD_WARNING;
	}
    

	return CMD_SUCCESS;
}

DEFUN (
    mdf_cm_sf,
    mdf_cm_sf_cmd,
    "cable-modem <H.H.H> qos service-flow change up <sfid> down <sfid> <filename>",
    SET_CM_STR
    SEL_CM_STR
    "config cable modem's qos"
    "config a service flow"
    "change a service flow"
    "change a service flow's upstream direction"
    "please input the sfid(0-no operate)\n" 
    "change a service flow's downstream direction"
    "please input the sfid(0-no operate)\n" 
    "qos tlv file\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];
    char szFileName[256];
    char tlv_buffer[2048];
    unsigned char  *tlv_data;
    unsigned short  tlv_len;
    long            lFileLen;
    unsigned long   ulFileDataLen;
    unsigned long   usfid, dsfid;

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    VOS_Snprintf(szFileName, 250, "/flash/sys/%s", argv[3]);
    if ( VOS_ERROR == CPI_HOOK_CALL( cdsms_file_exist_fs )(szFileName) )
    {
        vty_out( vty, "\r\n%% The specified file does not exist.\r\n" );

        return CMD_WARNING;
    }

    if ( VOS_ERROR == (lFileLen = CPI_HOOK_CALL( cdsms_file_length_get_fs )(szFileName) ) )
    {
        vty_out( vty, "\r\n%% The specified file's length get error.\r\n" );

        return CMD_WARNING;
    }
    else if ( (lFileLen < CMC_MIN_TLV_FILE_LENGTH) || (lFileLen > CMC_MAX_TLV_FILE_LENGTH) )
    {
        vty_out( vty, "\r\n%% The specified file's length(%lu) is invalid<%lu-%lu>.\r\n", lFileLen, CMC_MIN_TLV_FILE_LENGTH, CMC_MAX_TLV_FILE_LENGTH );

        return CMD_WARNING;
    }

    if ( lFileLen < sizeof(tlv_buffer) )
    {
        tlv_data = tlv_buffer;
    }
    else
    {
        if ( NULL == (tlv_data = (unsigned char *)g_malloc( lFileLen )) )
        {
            vty_out( vty, "\r\n%% The file's length(%lu) is too large to handle.\r\n", lFileLen );

            return CMD_WARNING;
        }
    }

    if ( 0 == CPI_HOOK_CALL( cdsms_file_read_fs ) (szFileName, tlv_data, &ulFileDataLen) )
    {
        tlv_len = (unsigned short)ulFileDataLen;
    }
    else
    {
        if ( tlv_data != tlv_buffer )
        {
            g_free(tlv_data);
        }
    
        vty_out( vty, "\r\n%% The specified file's data read(%lu) error.\r\n", lFileLen );

        return CMD_WARNING;
    }

    usfid = VOS_StrToUL(argv[1], NULL, 10);
    dsfid = VOS_StrToUL(argv[2], NULL, 10);

    result = OnuMgt_ModifyCmServiceFlow(PonPortIdx, OnuIdx, cmcMac, cmMac, usfid, dsfid, tlv_data, tlv_len);

    if ( tlv_data != tlv_buffer )
    {
        g_free(tlv_data);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to change cm's service-flow\r\n");
		return CMD_WARNING;
	}
    

	return CMD_SUCCESS;
}

DEFUN (
    clr_cm_sf,
    clr_cm_sf_cmd,
    "cable-modem <H.H.H> qos service-flow clear up <sfid> down <sfid>",
    SET_CM_STR
    SEL_CM_STR
    "config cable modem's qos"
    "config a service flow"
    "destory a service flow"
    "destory a service flow's upstream direction"
    "please input the sfid(0-no operate)\n" 
    "destory a service flow's downstream direction"
    "please input the sfid(0-no operate)\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];
    unsigned long   usfid, dsfid;

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    usfid = VOS_StrToUL(argv[1], NULL, 10);
    dsfid = VOS_StrToUL(argv[2], NULL, 10);

    result = OnuMgt_DestroyCmServiceFlow(PonPortIdx, OnuIdx, cmcMac, cmMac, usfid, dsfid);
	if (0 != result)
	{
		vty_out(vty, "failed to destroy cm's service-flow\r\n");
		return CMD_WARNING;
	}
    

	return CMD_SUCCESS;
}
#endif


#if 1
/* --------------------QoSπ‹¿Ì------------------- */
DEFUN (
    show_cmc_qos_sf,
    show_cmc_qos_sf_cmd,
    "show cmc qos service-flow [config|statistics] <sfid_list>",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_QOS_STR 
    SHOW_CMC_SF_STR 
    "show the service-flow's configuration\n" 
    "show the service-flow's statistics\n" 
    "please input the sfid list\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned short  sf_num;
    unsigned long   sf_id;
    unsigned long   sf_ids[8];
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[4096] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    sf_num = 0;
    BEGIN_PARSE_ID_LIST_TO_ID( argv[1], sf_id, 1, 0xFFFFFFFF, 8 )
    {
        sf_ids[sf_num++] = sf_id;
    }
    END_PARSE_ONUID_LIST_TO_ONUID()    

    if ( 0 == sf_num )
    {
		vty_out(vty, "sfid is null.\r\n");
		return CMD_WARNING;
    }
    
    if ( 'c' == argv[0][0] )
    {
        result = OnuMgt_DumpCmcServiceFlow(PonPortIdx, OnuIdx, cmcMac, sf_num, sf_ids, szDumpOutput, &length);
    }
    else
    {
        result = OnuMgt_DumpCmcServiceFlowStatistics(PonPortIdx, OnuIdx, cmcMac, sf_num, sf_ids, szDumpOutput, &length);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to get cmc's qos infos\r\n");
		return CMD_WARNING;
	}
    
    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }

	return CMD_SUCCESS;
}

DEFUN (
    show_cmc_qos_cls,
    show_cmc_qos_cls_cmd,
    "show cmc qos classifier config <sfid_list>",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_QOS_STR 
    SHOW_CMC_CLS_STR 
    "show the classifier's configuration\n" 
    "please input the sfid list\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned short  sf_num;
    unsigned long   sf_id;
    unsigned long   sf_ids[8];
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[4096] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    sf_num = 0;
    BEGIN_PARSE_ID_LIST_TO_ID( argv[0], sf_id, 1, 0xFFFFFFFF, 8 )
    {
        sf_ids[sf_num++] = sf_id;
    }
    END_PARSE_ONUID_LIST_TO_ONUID()    

    if ( 0 == sf_num )
    {
		vty_out(vty, "sfid is null.\r\n");
		return CMD_WARNING;
    }
    
    result = OnuMgt_DumpCmcClassifier(PonPortIdx, OnuIdx, cmcMac, sf_num, sf_ids, szDumpOutput, &length);
	if (0 != result)
	{
		vty_out(vty, "failed to get cmc's classifier\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }

	return CMD_SUCCESS;
}


DEFUN (
    add_cmc_qos_cls,
    add_cmc_qos_cls_cmd,
    "cmc qos service-flow class-name <name> add <filename>",
    SET_CMC_STR
    SET_CMC_QOS_STR
    SET_CMC_SF_STR
    "add a service class name,"
    "please input the class name(no more than 254)"
    "add a service class name\n" 
    "qos tlv file\n" 
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char szFileName[256];
    char tlv_buffer[256];
    unsigned char  *class_name;
    unsigned char  *tlv_data;
    unsigned short  tlv_len;
    long            lFileLen;
    unsigned long   ulFileDataLen;

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    class_name = argv[0];
    if ( VOS_StrLen(class_name) > 254 )
    {
        vty_out( vty, "\r\n%% Class name is to long, should be smaller than 254!" );
        return CMD_WARNING;
    }
    
    VOS_Snprintf(szFileName, 250, "/flash/sys/%s", argv[1]);
    if ( VOS_ERROR == CPI_HOOK_CALL( cdsms_file_exist_fs )(szFileName) )
    {
        vty_out( vty, "\r\n%% The specified file does not exist.\r\n" );

        return CMD_WARNING;
    }

    if ( VOS_ERROR == (lFileLen = CPI_HOOK_CALL( cdsms_file_length_get_fs )(szFileName) ) )
    {
        vty_out( vty, "\r\n%% The specified file's length get error.\r\n" );

        return CMD_WARNING;
    }
    else if ( (lFileLen < CMC_MIN_CLS_FILE_LENGTH) || (lFileLen > CMC_MAX_CLS_FILE_LENGTH) )
    {
        vty_out( vty, "\r\n%% The specified file's length(%lu) is invalid<%lu-%lu>.\r\n", lFileLen, CMC_MIN_CLS_FILE_LENGTH, CMC_MAX_CLS_FILE_LENGTH );

        return CMD_WARNING;
    }

    if ( lFileLen < sizeof(tlv_buffer) )
    {
        tlv_data = tlv_buffer;
    }
    else
    {
        VOS_ASSERT(0);
        return CMD_WARNING;
    }

    if ( 0 == CPI_HOOK_CALL( cdsms_file_read_fs ) (szFileName, tlv_data, &ulFileDataLen) )
    {
        tlv_len = (unsigned short)ulFileDataLen;
    }
    else
    {
        vty_out( vty, "\r\n%% The specified file's data read(%lu) error.\r\n", lFileLen );

        return CMD_WARNING;
    }

    result = OnuMgt_CreateCmcServiceFlowClassName(PonPortIdx, OnuIdx, cmcMac, class_name, tlv_data, tlv_len);
	if (0 != result)
	{
		vty_out(vty, "failed to create cmc's class-name\r\n");
		return CMD_WARNING;
	}
    

	return CMD_SUCCESS;
}

DEFUN (
    clr_cmc_qos_cls,
    clr_cmc_qos_cls_cmd,
    "undo cmc qos service-flow class-name <name>",
    NO_STR
    SET_CMC_STR
    SET_CMC_QOS_STR
    SET_CMC_SF_STR
    "remvoe a service class name,"
    "please input the class name"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    unsigned char  *class_name;

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    class_name = argv[0];
    if ( VOS_StrLen(class_name) > 254 )
    {
        vty_out( vty, "\r\n%% Class name is to long, should be smaller than 254!" );
        return CMD_WARNING;
    }

    result = OnuMgt_DestroyCmcServiceFlowClassName(PonPortIdx, OnuIdx, cmcMac, class_name);
	if (0 != result)
	{
		vty_out(vty, "failed to destroy cmc's class-name\r\n");
		return CMD_WARNING;
	}
    

	return CMD_SUCCESS;
}


DEFUN (
    show_cmc_channel_group,
    show_cmc_channel_group_cmd,
    "show cmc [downstream|upstream] bonding-group",
    SHOW_STR
    SHOW_CMC_STR
    SHOW_CMC_DOWNSTREAM_STR
    SHOW_CMC_UPSTREAM_STR
    "show channel bonding group settings\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
	char szDumpOutput[2048] = "";
	unsigned short length = sizeof(szDumpOutput);

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
	{
		return CMD_WARNING;
	}

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 'd' == argv[0][0] )
    {
        result = OnuMgt_DumpCmcDownChannelBondingGroup(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length);
    }
    else
    {
        result = OnuMgt_DumpCmcUpChannelBondingGroup(PonPortIdx, OnuIdx, cmcMac, szDumpOutput, &length);
    }
    
	if (0 != result)
	{
		vty_out(vty, "failed to get channel group\r\n");
		return CMD_WARNING;
	}

    if ( length < 1000 )
    {
    	vty_out(vty, "%s", szDumpOutput);
    }
    else
    {
    	vty_big_out(vty, (int)length, "%s", szDumpOutput);
    }
	
	return CMD_SUCCESS;
} 
#endif

#endif


#if 1
/* --------------------µÿ÷∑π‹¿Ì------------------- */
DEFUN  (
    show_cmc_fdb_mac,
    show_cmc_fdb_mac_cmd,
    "show cmc fdbentry mac {[counter]}*1",
    SHOW_STR
    SHOW_CMC_STR
    "Show fdbentry \n"
    "Show fdbentry MAC address\n"
    "show mac entry counter\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned short  
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
		return CMD_WARNING;    	

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( 0 < argc )
    {
   		result = ShowCmcMacLearningCounterByVty( PonPortIdx, OnuIdx, cmcMac, vty );
    }
    else
    {
   		result = ShowCmcMacLearningByVty( PonPortIdx, OnuIdx, cmcMac, vty );
    }

	if (0 != result)
	{
		vty_out(vty, "failed to get cmc's address infos\r\n");
		return CMD_WARNING;
	}

    return CMD_SUCCESS;	
}

DEFUN  (
    show_cm_fdb_mac,
    show_cm_fdb_mac_cmd,
    "show cable-modem <H.H.H> fdbentry mac {[counter]}*1",
    SHOW_STR
    SHOW_CM_STR
    SEL_CM_STR
    "Show fdbentry \n"
    "Show fdbentry MAC address\n"
    "show mac entry counter\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned short  
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
		return CMD_WARNING;    	

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

    if ( 1 < argc )
    {
   		result = ShowCmMacLearningCounterByVty( PonPortIdx, OnuIdx, cmcMac, cmMac, vty );
    }
    else
    {
   		result = ShowCmMacLearningByVty( PonPortIdx, OnuIdx, cmcMac, cmMac, vty );
    }

	if (0 != result)
	{
		vty_out(vty, "failed to get cm's address table\r\n");
		return CMD_WARNING;
	}

    return CMD_SUCCESS;	
}


DEFUN  (
    clear_cm_fdb_mac,
    clear_cm_fdb_mac_cmd,
    "clear cable-modem <H.H.H> fdbentry mac",
    CLEAR_STR
    SET_CM_STR
    SEL_CM_STR
    "clear fdbentry \n"
    "clear fdbentry MAC address\n"
    )
{
	PON_olt_id_t	PonPortIdx;
	PON_onu_id_t	OnuIdx;
	PON_onu_id_t	onu_id;
    unsigned short  
    int  result;
    int  macLen;
    char cmcMac[BYTES_IN_MAC_ADDRESS];
    char cmMac[BYTES_IN_MAC_ADDRESS];

	if( parse_onu_command_parameter(vty, &PonPortIdx, &OnuIdx, &onu_id) == VOS_ERROR )
		return CMD_WARNING;    	

	if (0 != GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &macLen))
	{
		vty_out(vty, "failed to get cmc mac-address\r\n");
		return CMD_WARNING;
	}  

    if ( GetMacAddr( ( CHAR* ) argv[ 0 ], cmMac ) != VOS_OK )
    {
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
    }

	result = OnuMgt_ResetCmAddrTbl( PonPortIdx, OnuIdx, cmcMac, cmMac, ADDR_DYNAMIC );
	if (0 != result)
	{
		vty_out(vty, "failed to clear cm's address table\r\n");
		return CMD_WARNING;
	}

    return CMD_SUCCESS;	
}

#endif


struct cmd_node onu_CMC_node =
{
    ONU_CMC_NODE,
    NULL,
    1
};

CMD_NOTIFY_REFISTER_S stCMD_ChannelId_List_Check =
{
    "<channel_list>",
    CMC_Check_ChannelId_List,
    0
};

CMD_NOTIFY_REFISTER_S stCMD_SFID_List_Check =
{
    "<sfid_list>",
    CMC_Check_SFID_List,
    0
};

int onu_CMC_init_func()
{
    return VOS_OK;
}

int onu_CMC_showrun( struct vty * vty )
{    
    return VOS_OK;
}

int onu_CMC_config_write ( struct vty * vty )
{
    return VOS_OK;
}

LONG onu_CMC_node_install()
{
    install_node( &onu_CMC_node, onu_CMC_config_write);
    onu_CMC_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_CMC);
    if ( !onu_CMC_node.prompt )
    {
        ASSERT( 0 );
        return (-IFM_E_NOMEM);
    }
    install_default( ONU_CMC_NODE );
    return VOS_OK;
}


LONG onu_CMC_module_init()
{
    struct cl_cmd_module * onu_CMC_module = NULL;

    onu_CMC_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_CMC);
    if ( !onu_CMC_module )
    {
        ASSERT( 0 );
        return -IFM_E_NOMEM;
    }

    VOS_MemZero( ( char * ) onu_CMC_module, sizeof( struct cl_cmd_module ) );

    onu_CMC_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_CMC);
    if ( !onu_CMC_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_CMC_module );
        return -IFM_E_NOMEM;
    }
    VOS_StrCpy( onu_CMC_module->module_name, "onu_CMC" );

    onu_CMC_module->init_func = onu_CMC_init_func;
    onu_CMC_module->showrun_func = onu_CMC_showrun;
    onu_CMC_module->next = NULL;
    onu_CMC_module->prev = NULL;


	if ( cmd_rugular_register( &stCMD_ChannelId_List_Check ) == no_match )
	{
        ASSERT( 0 );
	}

	if ( cmd_rugular_register( &stCMD_SFID_List_Check ) == no_match )
	{
        ASSERT( 0 );
	}


    cl_install_module( onu_CMC_module );
  
    return VOS_OK;
}


VOID CMC_CommandInstall(enum node_type  node)
{
#if 1
/* --------------------CMCπ‹¿Ì------------------- */
    if ( ONU_PROFILE_NODE != node )
    {
        install_element ( node, &register_cmc_mac_cmd);
        install_element ( node, &unregister_cmc_mac_cmd);
        install_element ( node, &reset_cmc_board_cmd);
        install_element ( node, &show_cmc_cmd);
        install_element ( node, &show_cmc_alarm_cmd);
        /* install_element ( node, &show_cmc_log_cmd); */
        install_element ( node, &show_cmc_version_cmd);
        install_element ( node, &show_cmc_max_multicast_cmd);
    }

    install_element ( node, &show_cmc_max_cm_cmd);
    install_element ( node, &set_cmc_max_cm_cmd);
    install_element ( node, &clr_cmc_max_cm_cmd);
    
    if ( ONU_PROFILE_NODE != node )
    {
        install_element ( node, &show_cmc_clock_cmd);
        install_element ( node, &set_cmc_clock_cmd);
        
        install_element ( node, &show_cmc_custom_config_cmd);
        install_element ( node, &set_cmc_cc_tpid_cmd);
        install_element ( node, &clr_cmc_cc_tpid_cmd);
        install_element ( node, &set_cmc_cc_map_time_cmd);
        install_element ( node, &clr_cmc_cc_map_time_cmd);
        install_element ( node, &set_cmc_cc_init_rng_prd_cmd);
        install_element ( node, &clr_cmc_cc_init_rng_prd_cmd);
        install_element ( node, &set_cmc_cc_prd_rng_prd_cmd);
        install_element ( node, &clr_cmc_cc_prd_rng_prd_cmd);
        install_element ( node, &set_cmc_cc_range_backoff_cmd);
        install_element ( node, &clr_cmc_cc_range_backoff_cmd);
        install_element ( node, &set_cmc_cc_data_backoff_cmd);
        install_element ( node, &clr_cmc_cc_data_backoff_cmd);
        install_element ( node, &set_cmc_cc_map_lead_time_cmd);
        install_element ( node, &clr_cmc_cc_map_lead_time_cmd);
    }
#endif

#if 1
/* --------------------CMC∆µµ¿π‹¿Ì------------------- */
    if ( ONU_PROFILE_NODE != node )
    {
        install_element ( node, &show_cmc_down_channel_cmd);
        install_element ( node, &show_cmc_up_channel_cmd);
    }
    
    install_element ( node, &close_cmc_down_channel_cmd);
    install_element ( node, &open_cmc_down_channel_cmd);
    install_element ( node, &close_cmc_up_channel_cmd);
    install_element ( node, &open_cmc_up_channel_cmd);
    
    install_element ( node, &set_cmc_up_channel_30_cmd);
    
    install_element ( node, &set_cmc_down_channel_freq_cmd);
    install_element ( node, &clr_cmc_down_channel_freq_cmd);
    install_element ( node, &set_cmc_up_channel_freq_cmd);
    install_element ( node, &clr_cmc_up_channel_freq_cmd);
    install_element ( node, &set_cmc_auto_channel_freq_cmd);
    install_element ( node, &set_cmc_up_channel_width_cmd);
    install_element ( node, &clr_cmc_up_channel_width_cmd);
    
    install_element ( node, &set_cmc_down_channel_annex_cmd);
    install_element ( node, &clr_cmc_down_channel_annex_cmd);
    
    install_element ( node, &set_cmc_up_channel_type_cmd);
    install_element ( node, &clr_cmc_up_channel_type_cmd);
    
    install_element ( node, &set_cmc_down_channel_modulation_cmd);
    install_element ( node, &clr_cmc_down_channel_modulation_cmd);
    
    install_element ( node, &set_cmc_up_channel_profile_cmd);
    install_element ( node, &clr_cmc_up_channel_profile_cmd);
    
    install_element ( node, &set_cmc_down_channel_interleaver_cmd);
    install_element ( node, &clr_cmc_down_channel_interleaver_cmd);
    
    install_element ( node, &set_cmc_down_channel_power_cmd);
    install_element ( node, &clr_cmc_down_channel_power_cmd);
    install_element ( node, &set_cmc_up_channel_power_cmd);
    install_element ( node, &clr_cmc_up_channel_power_cmd);
    install_element ( node, &show_cmc_up_channel_power_cmd);
    
    if ( ONU_PROFILE_NODE != node )
    {
        install_element ( node, &show_cmc_up_channel_signal_cmd);
        install_element ( node, &show_cmc_up_interface_stat_cmd);
        install_element ( node, &show_cmc_down_interface_stat_cmd);
        install_element ( node, &show_cmc_mac_summary_cmd);
        install_element ( node, &show_cmc_interface_summary_cmd);
    }
#endif

    if ( ONU_PROFILE_NODE != node )
    {
#if 1
/* --------------------CMC∆µµ¿◊Èπ‹¿Ì------------------- */
    install_element ( node, &show_cmc_group_dynamic_config_cmd);

    install_element ( node, &set_cmc_group_dynamic_method_cmd);
    install_element ( node, &clr_cmc_group_dynamic_method_cmd);

    install_element ( node, &set_cmc_group_dynamic_period_cmd);
    install_element ( node, &clr_cmc_group_dynamic_period_cmd);

    install_element ( node, &set_cmc_group_dynamic_weight_period_cmd);
    install_element ( node, &clr_cmc_group_dynamic_weight_period_cmd);

    install_element ( node, &set_cmc_group_dynamic_overload_threshold_cmd);
    install_element ( node, &clr_cmc_group_dynamic_overload_threshold_cmd);
    install_element ( node, &set_cmc_group_dynamic_difference_threshold_cmd);
    install_element ( node, &clr_cmc_group_dynamic_difference_threshold_cmd);

    install_element ( node, &set_cmc_group_dynamic_max_moves_cmd);
    install_element ( node, &clr_cmc_group_dynamic_max_moves_cmd);

    install_element ( node, &set_cmc_group_dynamic_min_hold_cmd);
    install_element ( node, &clr_cmc_group_dynamic_min_hold_cmd);
    
    install_element ( node, &set_cmc_group_dynamic_range_mode_cmd);

    install_element ( node, &set_cmc_group_dynamic_atdma_dcc_cmd);
    install_element ( node, &clr_cmc_group_dynamic_atdma_dcc_cmd);
    install_element ( node, &set_cmc_group_dynamic_scdma_dcc_cmd);
    install_element ( node, &clr_cmc_group_dynamic_scdma_dcc_cmd);
    install_element ( node, &set_cmc_group_dynamic_atdma_dbc_cmd);
    install_element ( node, &clr_cmc_group_dynamic_atdma_dbc_cmd);
    install_element ( node, &set_cmc_group_dynamic_scdma_dbc_cmd);
    install_element ( node, &clr_cmc_group_dynamic_scdma_dbc_cmd);

    install_element ( node, &show_cmc_group_cmd);
    install_element ( node, &show_cmc_group_stream_cmd);

    install_element ( node, &new_cmc_group_cmd);
    install_element ( node, &clr_cmc_group_cmd);

    install_element ( node, &add_cmc_group_channel_cmd);
    install_element ( node, &clr_cmc_group_channel_cmd);

    install_element ( node, &show_cmc_group_modem_cmd);
    install_element ( node, &add_cmc_group_modem_cmd);
    install_element ( node, &clr_cmc_group_modem_cmd);

    install_element ( node, &add_cmc_group_exclude_modem_cmd);
    install_element ( node, &clr_cmc_group_exclude_modem_cmd);
    install_element ( node, &show_cmc_group_exclude_modem_cmd);
#endif

#if 1
/* --------------------CMπ‹¿Ì------------------- */
    install_element ( node, &show_cm_cmd);
    install_element ( node, &show_cm_history_cmd);
    install_element ( node, &clr_cm_history_cmd);
    
    install_element ( node, &reset_cm_cmd);

    install_element ( node, &show_cm_channel_cmd);
    install_element ( node, &set_cm_channel_cmd);
    
    install_element ( node, &show_cm_qos_cmd);
    install_element ( node, &new_cm_sf_cmd);
    install_element ( node, &mdf_cm_sf_cmd);
    install_element ( node, &clr_cm_sf_cmd);
#endif

#if 1
/* --------------------QoSπ‹¿Ì------------------- */
    install_element ( node, &show_cmc_qos_sf_cmd);
    install_element ( node, &show_cmc_qos_cls_cmd);
    install_element ( node, &add_cmc_qos_cls_cmd);
    install_element ( node, &clr_cmc_qos_cls_cmd);

    install_element ( node, &show_cmc_channel_group_cmd);
#endif

#if 1
/* --------------------µÿ÷∑π‹¿Ì------------------- */
    install_element ( node, &show_cmc_fdb_mac_cmd);
    install_element ( node, &show_cm_fdb_mac_cmd);
    install_element ( node, &clear_cm_fdb_mac_cmd);
#endif
    }


    return;
}


VOID OnuCMCCommandInstall()
{
    onu_CMC_node_install();
    onu_CMC_module_init();
   
    CMC_CommandInstall(ONU_CMC_NODE);   
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
    CMC_CommandInstall(ONU_PROFILE_NODE);   
#endif
    CT_RMan_ONU_Init(ONU_CMC_NODE);

    install_element(PON_PORT_NODE, &show_cmc_cmd);
    
    install_element(CONFIG_NODE, &set_cmc_svlan_cmd);
    install_element(CONFIG_NODE, &clr_cmc_svlan_cmd);
    install_element(CONFIG_NODE, &show_cmc_svlan_cmd);
}


#endif


#ifdef	__cplusplus
}
#endif/* __cplusplus */

