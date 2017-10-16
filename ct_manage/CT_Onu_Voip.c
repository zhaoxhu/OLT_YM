#ifdef	__cplusplus
extern "C"
{
#endif

#include  "OltGeneral.h"
#include  "gwEponSys.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "includeFromPas.h"
#include  "lib_gwEponMib.h"
#include  "CT_RMan_Main.h"
#include  "CT_Onu_Auth.h"
#include  "Cdp_pub.h"
#include "CT_Onu_Voip.h"

char *ctconu_iad_status_str[] = 
{
    "REGISTERING",
    "IDLE",
    "OFFHOOK",
    "DAILING",
    "RINGING",
    "RINGBACK",
    "CONNECTING",
    "CONNECTED",
    "DISCONNECTING",
    "REGISTER FAILED",
    "INACTIVE"
};

char *ctconu_iad_servive_str[]=
{
    "END LOCAL",
    "END REMOTE",
    "END AUTO",
    "NORMAL"
};
char *ctconu_iad_codec_str[]=
{
    "G711A",
    "G729",
    "G711U",
    "G723",
    "G726",
    "T38"
};
void ctc_voip_copy(char *d, char *s, int len)
{
	int i, endflag = 0;
	for( i=0; i<len; i++ )
	{
		if( *s != '\0' )
		{
			*d = *s;
			d++;
			endflag = 1;
		}
		else if( endflag )
			break;
		s++;
	}
	*d = 0;    
}
#define CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)\
{\
    if(GetOnuVoipAbility(PonPortIdx, OnuIdx) != SUPPORTING)\
    {\
        vty_out(vty, "onu not support voip.\r\n");\
        return CMD_WARNING;\
    }\
}
#define CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)\
{\
	if ( ONU_OPER_STATUS_UP != GetOnuOperStatus( PonPortIdx, OnuIdx))\
    {\
		vty_out(vty, "onu is off-line.\r\n");\
		return CMD_WARNING;\
    }\
}

int GetOnuVoipAbility(short int PonPortIdx, short int OnuIdx)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    int enable = 0;
    ONU_MGMT_SEM_TAKE;
    enable = OnuMgmtTable[OnuEntry].VoIP_supporting;    
    ONU_MGMT_SEM_GIVE;    
    return enable;
}
unsigned char GetOnuVoipMaxPortNum(short int PonPortIdx, short int OnuIdx)
{
    int OnuEntry = PonPortIdx*MAXONUPERPON + OnuIdx;
    unsigned char num = 0;
    ONU_MGMT_SEM_TAKE;
    num = OnuMgmtTable[OnuEntry].POTS_ports_number;    
    ONU_MGMT_SEM_GIVE;    
    return num;
}
int IsSupportH248(short int PonPortIdx, short int OnuIdx)
{
    int lRet = 0;
    CTC_STACK_voip_iad_info_t iad_info;
    VOS_MemZero(&iad_info, sizeof(CTC_STACK_voip_iad_info_t));
    lRet = OnuMgt_GetIADInfo(PonPortIdx, OnuIdx, &iad_info);   
    if(lRet == VOS_OK)
    {
        if(iad_info.voip_protocol != CTC_STACK_VOIP_PROTOCOL_H248)
            lRet = VOS_ERROR;
    }
    else
        lRet = VOS_ERROR;
    return lRet;
}
int IsSupportSip(short int PonPortIdx, short int OnuIdx)
{
    int lRet = 0;
    CTC_STACK_voip_iad_info_t iad_info;
    VOS_MemZero(&iad_info, sizeof(CTC_STACK_voip_iad_info_t));
    lRet = OnuMgt_GetIADInfo(PonPortIdx, OnuIdx, &iad_info);   
    if(lRet == VOS_OK)
    {
        if(iad_info.voip_protocol != CTC_STACK_VOIP_PROTOCOL_SIP)
            lRet = VOS_ERROR;
    }
    else
        lRet = VOS_ERROR;
    return lRet;
}
#if 1
DEFUN(
    config_ctc_voice_ip_mode_func,
    config_ctc_voice_ip_mode_cmd,
    "voice ip mode [ip|dhcp|pppoe]",
    "Config voice module information\n"
    "IP information\n"
    "Voice ip Mode\n"  
    "Static IP mode, default\n"
    "DHCP mode\n"
    "PPPoE/PPPOE+ mode\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    int code = voice_global_config_ip_mode;
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
        
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    if(VOS_StrCmp(argv[0], "ip") == 0)
        global_param.voice_ip_mode = CTC_STACK_VOIP_VOICE_IP_STATIC;
    else if(VOS_StrCmp(argv[0], "dhcp") == 0)
        global_param.voice_ip_mode = CTC_STACK_VOIP_VOICE_IP_DHCP;
    else
        global_param.voice_ip_mode = CTC_STACK_VOIP_VOICE_IP_PPPOE;
    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice ip mode set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(
    config_ctc_voice_ipaddr_func,
    config_ctc_voice_ipaddr_cmd,
    "voice ip address <A.B.C.D/M>" ,
    "Config voice module information\n"
    "IP information\n"
    "IP address information\n"
    "IP address and length of mask\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = voice_global_config_ip_addr;
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    if ( VOS_OK != IpListToIp( argv[0], &global_param.iad_ip_addr, &global_param.iad_net_mask) )
	{
		vty_out( vty, "%% Invalid IP address.\r\n" );
		return CMD_SUCCESS;
	}
    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice ip addr set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(
    config_ctc_voice_ipaddr1_func,
    config_ctc_voice_ipaddr1_cmd,
    "voice ip address <A.B.C.D> <A.B.C.D>" ,
    "Config voice module information\n"
    "IP information\n"
    "IP address information\n"
    "IP address\n"
    "IP subnet mask\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	int ulRet = 0;
    int code = voice_global_config_ip_addr;
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    global_param.iad_ip_addr = get_long_from_ipdotstring(argv[0]);
    global_param.iad_net_mask = get_long_from_ipdotstring(argv[1]);
    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice ip addr set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(
    undo_ctc_voice_ipaddr_func,
    undo_ctc_voice_ipaddr_cmd,
    "undo voice ip address",
    "Negate a command or set its defaults\n"
    "Config voice module information\n"
    "IP information\n"
    "IP address information\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = voice_global_config_ip_addr;
    
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%undo voice ip addr failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(
    show_ctc_voice_ip_func,
    show_ctc_voice_ip_cmd,
    "show voice ip address",
    "Show running system information\n"
    "Voice module information\n"
    "IP information\n"
    "IP address information\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    char ipstr[33]={0};
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    
    lRet = OnuMgt_GetVoipGlobalConfig(PonPortIdx, OnuIdx, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice ip addr get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        get_ipdotstring_from_long(ipstr, global_param.iad_ip_addr);
        vty_out(vty, " %-13s: %s\r\n", "Voice IP", ipstr);
        get_ipdotstring_from_long(ipstr, global_param.iad_net_mask);        
        vty_out(vty, " %-13s: %s\r\n", "Voice IP Mask", ipstr);
    }
    return CMD_SUCCESS;
}

DEFUN(
    config_ctc_static_voice_ipgateway_func,
    config_ctc_static_voice_ipgateway_cmd,
    "voice ip gateway <A.B.C.D>",
    "Config voice module information\n"
    "IP information\n"
    "Establish static gateway routes\n"
    "IP gateway address\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    int code = voice_global_config_ip_gw;
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    global_param.iad_def_gw = get_long_from_ipdotstring(argv[0]);
    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice ip gateway set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}
DEFUN(
    show_ctc_vocie_ip_gateway_func,
    show_ctc_vocie_ip_gateway_cmd,
    "show voice ip gateway",
    "Show running system information\n"
    "Voice module information\n"
    "IP information\n"
    "IP gateway\n" 
    )
{	
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    char ipstr[33]={0};
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    
    lRet = OnuMgt_GetVoipGlobalConfig(PonPortIdx, OnuIdx, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice ip gateway get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        get_ipdotstring_from_long(ipstr, global_param.iad_def_gw);
        vty_out(vty, " Voice IP Gateway: %s\r\n", ipstr);
    }
    return CMD_SUCCESS;
}
DEFUN(
    undo_ctc_static_voice_ipgateway_func,
    undo_ctc_static_voice_ipgateway_cmd,
    "undo voice ip gateway",
    "Negate a command or set its defaults\n"
    "Config voice module information\n"
    "IP information\n"
    "Establish static gateway routes\n"
    )
{	
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    int code = voice_global_config_ip_gw;
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%undo voice ip gateway failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(
    config_ctc_voice_pppoe_mode_func,
    config_ctc_voice_pppoe_mode_cmd,
    "voice pppoe mode [auto|chap|pap]",
    "Config voice module information\n"
    "Pppoe information\n"
    "Voice ip Mode\n"   
    "Auto mode, Default\n"
    "Challenge Handshake Authentication Protocol\n"
    "Password Authentication Protocol\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    int code = voice_global_config_pppoe_mode;
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    if(VOS_StrCmp(argv[0], "auto") == 0)
        global_param.pppoe_mode = CTC_STACK_PPPOE_MODE_AUTO;
    else if(VOS_StrCmp(argv[0], "chap") == 0)
        global_param.pppoe_mode = CTC_STACK_PPPOE_MODE_CHAP;
    else
        global_param.pppoe_mode = CTC_STACK_PPPOE_MODE_PAP;
    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice pppoe mode set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}
DEFUN(
    config_ctc_voice_pppoe_username_func,
    config_ctc_voice_pppoe_username_cmd,
    "voice pppoe username <name>",
    "Config voice module information\n"
    "Pppoe information\n"
    "Username\n"
    "Please input the Username\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = voice_global_config_pppoe_username;
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    if(strlen(argv[0])>VOIP_PPPOE_USER_SIZE)
    {
        VOS_MemCpy(global_param.pppoe_user, argv[0], VOIP_PPPOE_USER_SIZE-1);
    }
    else
        VOS_StrCpy(global_param.pppoe_user, argv[0]);
    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice pppoe user name set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}
DEFUN(
    config_ctc_voice_pppoe_pwd_func,
    config_ctc_voice_pppoe_pwd_cmd,
    "voice pppoe password <password>",
    "Config voice module information\n"
    "Pppoe information\n"
    "Password\n"
    "Please input the password\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    int code = voice_global_config_pppoe_password;
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    if(strlen(argv[0])>VOIP_PPPOE_PASSWD_SIZE)
    {
        VOS_MemCpy(global_param.pppoe_passwd, argv[0], VOIP_PPPOE_PASSWD_SIZE-1);
    }
    else
        VOS_StrCpy(global_param.pppoe_passwd, argv[0]);
    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice pppoe user password set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(
    config_ctc_voice_vlan_func,
    config_ctc_voice_vlan_cmd,
    "voice vlan <1-4094>" ,
    "Config voice module information\n"
    "Vlan information\n"
    "Please input the Vlan Vid(1-4094)\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;

	ULONG ulRet;
    int code = voice_global_config_vlan;
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    global_param.cvlan_id = VOS_AtoL(argv[0]);
    
    lRet = OnuMgt_SetVoipGlobalConfig(PonPortIdx, OnuIdx, code, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice vlan set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}
DEFUN(
    show_ctc_voice_config_func,
    show_ctc_voice_config_cmd,
    "show voice configuration" ,
    "Show running system information\n"
    "Config voice module information\n"
    "Voice configuration\n"
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;

	ULONG ulRet;
    char ipstr[33]={0};
    CTC_STACK_voip_global_param_conf_t global_param;    
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&global_param, sizeof(CTC_STACK_voip_global_param_conf_t));
    
    lRet = OnuMgt_GetVoipGlobalConfig(PonPortIdx, OnuIdx, &global_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice global config get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        vty_out(vty, "Voice Global Parameter Config:\r\n");
        if(global_param.voice_ip_mode==CTC_STACK_VOIP_VOICE_IP_STATIC)
        {
            vty_out(vty, "  %-19s: %s\r\n", "Ip mode", "Static Ip");
            get_ipdotstring_from_long(ipstr, global_param.iad_ip_addr);
            vty_out(vty, "  %-19s: %s\r\n", "Voice IP",ipstr);
            get_ipdotstring_from_long(ipstr, global_param.iad_net_mask);        
            vty_out(vty, "  %-19s: %s\r\n", "Voice IP Mask", ipstr);
            get_ipdotstring_from_long(ipstr, global_param.iad_def_gw);
            vty_out(vty, "  %-19s: %s\r\n", "Voice IP Gateway", ipstr);
        }
        else if(global_param.voice_ip_mode==CTC_STACK_VOIP_VOICE_IP_DHCP)
        {
            vty_out(vty, "  %-19s: %s\r\n", "Ip mode", "DHCP");            
        }
        else
        {
            vty_out(vty, "  %-19s: %s\r\n", "Ip mode", "PPPoE");
            vty_out(vty, "  %-19s: %s\r\n", "PPPoE Mode", global_param.pppoe_mode?(global_param.pppoe_mode==CTC_STACK_PPPOE_MODE_CHAP?"CHAP":"PAP"):"Auto");            
            vty_out(vty, "  %-19s: %s\r\n", "PPPoE User name", global_param.pppoe_user);
            vty_out(vty, "  %-19s: %s\r\n", "PPPoE User Password", global_param.pppoe_passwd);            
        }
        if(global_param.tag_flag==CTC_STACK_VOIP_TAGGED_FLAG_TRANSPARENT)
            vty_out(vty, "  %-19s: %s\r\n", "Voice Vlan mode", "transparent");      
        else
            vty_out(vty, "  %-19s: %d\r\n", "Voice Vlan id", global_param.cvlan_id);            
    }
    return CMD_SUCCESS;
}
#endif

#if 1
DEFUN(ctc_h248_local_port_config,
	ctc_h248_local_port_config_cmd,
	"h248 local port <1-65535>",
	"config H248\n" 
	"Media gateway config\n"
	"port config\n" 
	"port number\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = h248_config_local_port;
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));
    h248_param.mg_port = VOS_AtoL(argv[0]);

    lRet = OnuMgt_SetH248Config(PonPortIdx, OnuIdx, code, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%H248 local port set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(ctc_h248_mgc_address_config,
	ctc_h248_mgc_address_config_cmd,
	"h248 mgc address <address>",
	"config H248\n" 
	"Media gateway controller config\n"
	"address config\n"
	"ip address (xx.xx.xx.xx)\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = h248_config_mgc_addr;
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));    
    h248_param.mgcip = get_long_from_ipdotstring(argv[0]);

    lRet = OnuMgt_SetH248Config(PonPortIdx, OnuIdx, code, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%h248 mgc address set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}

DEFUN(ctc_h248_mgc_port_config,
	ctc_h248_mgc_port_config_cmd,
	"h248 mgc port <1-65535>",
	"config H248\n" 
	"Media gateway controller config\n"
	"port config\n" 
	"port number\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = h248_config_mgc_port;
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));
    h248_param.mgccom_port_num = VOS_AtoL(argv[0]);

    lRet = OnuMgt_SetH248Config(PonPortIdx, OnuIdx, code, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%H248 mgc port set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}
DEFUN(ctc_h248_mgcbackup_address,
	ctc_h248_mgcbackup_address_cmd,
	"h248 mgc-backup address <address>",
	"config H248\n" 
	"backup mgc config\n"
	"backup mgc address config\n"
	"ip address (xx.xx.xx.xx)\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = h248_config_mgc_backup_addr;
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));    
    h248_param.back_mgcip = get_long_from_ipdotstring(argv[0]);

    lRet = OnuMgt_SetH248Config(PonPortIdx, OnuIdx, code, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%H248 back-up mgc address set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}
DEFUN(ctc_h248_mgcbackup_port,
	ctc_h248_mgcbackup_port_cmd,
	"h248 mgc-backup port <1-65535>",
	"config H248\n" 
	"backup mgc  config\n"
	"backup mgc port config\n"
	"Port number\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = h248_config_mgc_backup_port;
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));
    h248_param.back_mgccom_port_num = VOS_AtoL(argv[0]);

    lRet = OnuMgt_SetH248Config(PonPortIdx, OnuIdx, code, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%h248 back-up mgc address set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(ctc_h248_registermode_config,
	ctc_h248_registermode_config_cmd,
	"h248 registermode [ip|domain]",
	"config H248\n" 
	"Registermode : ip or domain\n"
	"Using ip address\n"
	"Using local domain name\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = h248_config_regiser_mode;
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));
    h248_param.reg_mode = VOS_StrCmp(argv[0], "ip")==0?CTC_STACK_H248_REG_MODE_IP:CTC_STACK_H248_REG_MODE_DOMAIN;

    lRet = OnuMgt_SetH248Config(PonPortIdx, OnuIdx, code, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%H248 register mode set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}

DEFUN(ctc_h248_local_domain_config,
	ctc_h248_local_domain_config_cmd,
	"h248 local domain <domainname>",
	"config H248\n" 
	"Media gateway config\n"
	"domainname config\n" 
	"domainname <domainname>\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = h248_config_local_domain;
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));
    if(strlen(argv[0])>=H248_MID_SIZE)
        VOS_MemCpy(h248_param.mid, argv[0], H248_MID_SIZE);
    else
        VOS_StrCpy(h248_param.mid, argv[0]);

    lRet = OnuMgt_SetH248Config(PonPortIdx, OnuIdx, code, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%H248 local pppoe domain set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(ctc_h248_mgheartbeat,
	ctc_h248_mgheartbeat_cmd,
	"h248 heartbeat mg-heartbeat [enable|disable]",
	"config H248\n" 
	"heartbeat config\n"
	"mg-heartbeat config\n"
	"send mg-heartbeat msg\n"
	"do not send mg-heartbeat msg\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = h248_config_heartbeat_enable;
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));
    
    h248_param.heart_beat_mode = VOS_StrCmp(argv[0], "enable")?CTC_STACK_H248_HEART_BEAT_MODE_OFF:CTC_STACK_H248_HEART_BEAT_MODE_CTC_STO;

    lRet = OnuMgt_SetH248Config(PonPortIdx, OnuIdx, code, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%H248 heartbeat enable set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}

DEFUN(ctc_h248_heartbeatimer_config,
	ctc_h248_heartbeatimer_config_cmd, 
	"h248 heartbeat time <0-3600>", 
	"config H248\n" 
	"heartbeat config\n"
	"heartbeat timer config\n" 
	"Heartbeat timer \n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = h248_config_heartbeat_time;
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));
    h248_param.heart_beat_cycle = VOS_AtoL(argv[0]);

    lRet = OnuMgt_SetH248Config(PonPortIdx, OnuIdx, code, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%H248 heartbeat time set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}

DEFUN(ctc_h248_local_name_config,
	ctc_h248_local_name_config_cmd,
	"h248 local name <localnameprefix> <0-65535> <0-65535> <1-65535> [fix|unfix] <1-10>",
	"config H248\n" 
	"Media gateway config\n"
	"localname config\n"
	"localnameprefix config\n"
	"start id\n"
	"end id\n"
	"id increment\n"
	"localnamesuffix fixed length\n"
	"rtpnamesuffix unfixed length\n"
	"localnamesuffix length config\n"
	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG length;
    ULONG len = 0;
	ULONG ulRet;
    int slot = 0, port = 0;
    int data = 0;
    char name[H248_USER_TID_NAME_SIZE]={0};
    char name1[64]={0};
    CTC_STACK_h248_user_tid_config_t user_tid_config;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&user_tid_config, sizeof(CTC_STACK_h248_user_tid_config_t));
    if(VOS_StrCmp(argv[4], "fix") == 0)
    {
        int i = 1;
        char *p;
        length = VOS_AtoL(argv[5]);
        data = VOS_AtoL(argv[1]);
        
        while(length>=i)
        {   
            name[length-i] = data%10+'0';
            data /= 10;
            i++;   
        }
        VOS_StrCpy(name1, argv[0]);        
        VOS_StrCpy(&name1[strlen(name1)], name);
        len = strlen(name1);        
        if(len>=H248_USER_TID_NAME_SIZE)
            VOS_MemCpy(user_tid_config.user_tid_name, name1, H248_USER_TID_NAME_SIZE);
        else
            VOS_MemCpy(&(user_tid_config.user_tid_name[H248_USER_TID_NAME_SIZE-len]), name1, len);
        lRet = OnuMgt_SetH248UserTidConfig(PonPortIdx, OnuIdx, 1, &user_tid_config);

        VOS_MemZero(name, H248_USER_TID_NAME_SIZE);
        VOS_MemZero(name1, 64); 
        VOS_MemZero(&user_tid_config, sizeof(CTC_STACK_h248_user_tid_config_t));        
        if(VOS_AtoL(argv[1])+VOS_AtoL(argv[3])<VOS_AtoL(argv[2]))
        {
            data = VOS_AtoL(argv[1])+VOS_AtoL(argv[3]);
            i = 1;
            while(length>=i)
            {   
                name[length-i] = data%10+'0';
                data /= 10;
                i++;   
            }
            VOS_StrCpy(name1, argv[0]);        
            VOS_StrCpy(&name1[strlen(name1)], name);
            len = strlen(name1);
            if(len>=H248_USER_TID_NAME_SIZE)
                VOS_MemCpy(user_tid_config.user_tid_name, name1, H248_USER_TID_NAME_SIZE);
            else
                VOS_MemCpy(&(user_tid_config.user_tid_name[H248_USER_TID_NAME_SIZE-len]), name1, len);
            lRet |= OnuMgt_SetH248UserTidConfig(PonPortIdx, OnuIdx, 2, &user_tid_config);
        }
        else
        {
            lRet = VOS_ERROR;
            vty_out(vty, "  %%Param Error!\r\n");
        }
        
    }
    else
    {
        int i = 1;
        char *p;
        data = VOS_AtoL(argv[1]);
        
        while(data)
        {   
            name[H248_USER_TID_NAME_SIZE-i] = data%10+'0';
            data /= 10;
            i++;   
        }
        VOS_StrCpy(name1, argv[0]);        
        ctc_voip_copy(&name1[strlen(name1)], name, H248_USER_TID_NAME_SIZE);
        len = strlen(name1);
        if(len>=H248_USER_TID_NAME_SIZE)
            VOS_MemCpy(user_tid_config.user_tid_name, name1, H248_USER_TID_NAME_SIZE);
        else
            VOS_MemCpy(&(user_tid_config.user_tid_name[H248_USER_TID_NAME_SIZE-len]), name1, len);
        lRet = OnuMgt_SetH248UserTidConfig(PonPortIdx, OnuIdx, 1, &user_tid_config);

        VOS_MemZero(name, H248_USER_TID_NAME_SIZE);
        VOS_MemZero(name1, 64);    
        VOS_MemZero(&user_tid_config, sizeof(CTC_STACK_h248_user_tid_config_t));
        
        if(VOS_AtoL(argv[1])+VOS_AtoL(argv[3])<VOS_AtoL(argv[2]))
        {
            data = VOS_AtoL(argv[1])+VOS_AtoL(argv[3]);
            i = 1;
            while(data)
            {   
                name[H248_USER_TID_NAME_SIZE-i] = data%10+'0';
                data /= 10;
                i++;   
            }
            VOS_StrCpy(name1, argv[0]);        
            ctc_voip_copy(&name1[strlen(name1)], name, H248_USER_TID_NAME_SIZE);
            len = strlen(name1);
            
            if(len>=H248_USER_TID_NAME_SIZE)
                VOS_MemCpy(user_tid_config.user_tid_name, name1, H248_USER_TID_NAME_SIZE);
            else
                VOS_MemCpy(&(user_tid_config.user_tid_name[H248_USER_TID_NAME_SIZE-len]), name1, len);
            lRet |= OnuMgt_SetH248UserTidConfig(PonPortIdx, OnuIdx, 2, &user_tid_config);
            
        }
        else
        {
            lRet = VOS_ERROR;
            vty_out(vty, "  %%Param Error!\r\n");
        }
    }
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%H248 User tid set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(ctc_h248_local_rtpprefix_config,
	ctc_h248_local_rtpprefix_config_cmd,
	"h248 local rtp <rtpprefix> <0-65535> <0-65535> <1-65535> [fix|unfix] <1-8>",
	"config H248\n" 
	"Media gateway config\n"
	"rtpname config\n"
	"rtpnameprefix config\n" 
	"start id\n"
	"end id\n"
	"id increment\n"
	"rtpnamesuffix fixed length\n"
	"rtpnamesuffix unfixed length\n"
	"rtpnamesuffix length config\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    char rtp_tid[H248_RTP_TID_DIGIT_SIZE]={0};
    char name1[H248_RTP_TID_DIGIT_SIZE]={0};
    int i = 1;
    int slot = 0, port = 0;
    ULONG data = 0;
    CTC_STACK_h248_rtp_tid_config_t h248_rtp_tid_info;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_rtp_tid_info, sizeof(CTC_STACK_h248_rtp_tid_config_t));

    length = strlen(argv[0]);
    if(length>H248_RTP_TID_PREFIX_SIZE)
        VOS_MemCpy(h248_rtp_tid_info.rtp_tid_prefix, argv[0], H248_RTP_TID_PREFIX_SIZE);
    else
        VOS_MemCpy(&h248_rtp_tid_info.rtp_tid_prefix[H248_RTP_TID_PREFIX_SIZE-length], argv[0], length);

    if(VOS_AtoL(argv[2])>VOS_AtoL(argv[1]))
        h248_rtp_tid_info.num_of_rtp_tid = (VOS_AtoL(argv[2])-VOS_AtoL(argv[1]))/VOS_AtoL(argv[3]);
    else
        return CMD_WARNING;

    data = VOS_AtoL(argv[1]);
    *(ULONG *)(&h248_rtp_tid_info.rtp_tid_digit_begin[4]) = VOS_AtoL(argv[1]);
    if(VOS_StrCmp("fix", argv[4])==0)
    {
        h248_rtp_tid_info.rtp_tid_mode = CTC_STACK_H248_RTP_TID_MODE_ALIGNED;
    }
    else
    {
        h248_rtp_tid_info.rtp_tid_mode = CTC_STACK_H248_RTP_TID_MODE_UNALIGNED;
    }
    h248_rtp_tid_info.rtp_tid_digit_length = VOS_AtoL(argv[5]);
    lRet = OnuMgt_SetH248RtpTidConfig(PonPortIdx, OnuIdx, &h248_rtp_tid_info);
    if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%H248 Rtp tid set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(show_ctc_h248_config_func,
    show_ctc_h248_config_cmd,
    "show h248 configuration",
    "Show running system information\n"
    "Config H248\n"
    "Voice configuration\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    char ipstr[64]={0};
    CTC_STACK_h248_param_config_t h248_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_param, sizeof(CTC_STACK_h248_param_config_t));
    
    lRet = OnuMgt_GetH248Config(PonPortIdx, OnuIdx, &h248_param);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice h248 config get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        vty_out(vty, "Voice H248 Parameter Config:\r\n");
        vty_out(vty, "  %-15s: %d\r\n", "MG Port", h248_param.mg_port);
        get_ipdotstring_from_long(ipstr, h248_param.mgcip);
        vty_out(vty, "  %-15s: %s\r\n", "MGC Ip", ipstr);
        vty_out(vty, "  %-15s: %d\r\n", "MGC Port", h248_param.mgccom_port_num);
        get_ipdotstring_from_long(ipstr, h248_param.back_mgcip);
        vty_out(vty, "  %-15s: %s\r\n", "Backup MGC Ip", ipstr);
        vty_out(vty, "  %-15s: %d\r\n", "Backup MGC Port", h248_param.back_mgccom_port_num);
        vty_out(vty, "  %-15s: %s\r\n", "Active MGC", h248_param.active_mgc == CTC_STACK_H248_ACTIVE_MGC_BACKUP?"Backup MGC":"MGC");
        if(h248_param.reg_mode == CTC_STACK_H248_REG_MODE_IP)       
            vty_out(vty, "  %-15s: %s\r\n", "Register Mode", "IP");
        else
        {
            vty_out(vty, "  %-15s: %s\r\n", "Register Mode", "Domain");
            ctc_voip_copy(ipstr, h248_param.mid, H248_MID_SIZE);            
            vty_out(vty, "  %-15s: %s\r\n", "Domain name", ipstr);            
        }
        vty_out(vty, "  %-15s: %s\r\n", "Heartbeat Mode", h248_param.heart_beat_mode == CTC_STACK_H248_HEART_BEAT_MODE_OFF?"Disable":"Enable");
        vty_out(vty, "  %-15s: %d\r\n", "Heartbeat Cycle", h248_param.heart_beat_cycle);
        vty_out(vty, "  %-15s: %d\r\n", "Heartbeat Count", h248_param.heart_beat_count);        
    }
	return CMD_SUCCESS;
}
DEFUN(show_ctc_h248_user_config_func,
    show_ctc_h248_user_config_cmd,
    "show h248 user configuration",
    "Show running system information\n"
    "Config H248\n"
    "Config User Information\n"
    "Voice configuration\n"
)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int slot=0, port=0;   
    char h248_user[128]={0};
    CTC_STACK_h248_param_config_t h248_param;
    CTC_STACK_h248_user_tid_config_array h248_user_tid_array;
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_user_tid_array, sizeof(CTC_STACK_h248_user_tid_config_array));
    lRet = OnuMgt_GetH248UserTidConfig(PonPortIdx, OnuIdx, port, &h248_user_tid_array);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice h248 config get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        int maxport = GetOnuVoipMaxPortNum(PonPortIdx, OnuIdx);
        int i = 0;
        for(i=0;i<maxport;i++)
        {
            int j = 0;            
            unsigned short port = h248_user_tid_array.h248_user_tid_array[i].management_object_index.port_number;
    		vty_out(vty, "Voice phone%d's H248 User Config:\r\n", port);
            ctc_voip_copy(h248_user, h248_user_tid_array.h248_user_tid_array[i].h248_user_tid_config.user_tid_name, H248_USER_TID_NAME_SIZE);
            vty_out(vty, "  %s: %s\r\n", "User Tid", h248_user);
        }
    }
	return CMD_SUCCESS;
}
DEFUN(show_ctc_h248_rtp_config_func,
    show_ctc_h248_rtp_config_cmd,
    "show h248 rtp configuration",
    "Show running system information\n"
    "Config H248\n"
    "Config rtp information"
    "Voice configuration\n"
)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    char h248_user[128]={0};
    CTC_STACK_h248_rtp_tid_info_t h248_rtp_tid_info;

	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportH248(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support h248!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&h248_rtp_tid_info, sizeof(CTC_STACK_h248_rtp_tid_info_t));
    lRet = OnuMgt_GetH248RtpTidConfig(PonPortIdx, OnuIdx, &h248_rtp_tid_info);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice h248 rtp tid config get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        vty_out(vty, "Voice H248 Rtp Tid Config:\r\n");
        
        vty_out(vty, "  %-15s: %d\r\n", "Rtp Tid Number", h248_rtp_tid_info.num_of_rtp_tid);
        ctc_voip_copy(h248_user, h248_rtp_tid_info.rtp_tid_name, H248_RTP_TID_NAME_SIZE);
        vty_out(vty, "  %-15s: %s\r\n", "Rtp Tid Name", h248_user);
        
    }
	return CMD_SUCCESS;
}
#endif

#if 1
DEFUN(config_ctc_sip_local_port_config_func,
    config_ctc_sip_local_port_config_cmd,
    "sip local port <1-65535>",
    "config sip\n"
    "Config for Local PORT\n" 
    "Local Port config\n" 
    "Local Port (1-65535)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_local_port;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)

    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.mg_port = VOS_AtoL(argv[0]);
    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip local port set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_server_address_config_func,
    config_ctc_sip_server_address_config_cmd,
    "sip server address <address>",
    "config sip\n"
    "Config for Server!\n"
	"address config\n"
	"ip address (xx.xx.xx.xx)\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_server_ip;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.server_ip = get_long_from_ipdotstring(argv[0]);

    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip server address set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_server_port_config_func,
    config_ctc_sip_server_port_config_cmd,
    "sip server port <1-65535>",
    "config sip\n"
    "Config for Server PORT\n" 
    "Server Port config\n" 
    "Server Port (1-65535)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_server_port;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.serv_com_port = VOS_AtoL(argv[0]);
    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip server port set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_server_backup_address_config_func,
    config_ctc_sip_server_backup_address_config_cmd,
    "sip server-backup address <address>",
    "config sip\n"
    "Config for Backup Server!\n"
	"address config\n"
	"ip address (xx.xx.xx.xx)\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_server_backup_ip;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.back_server_ip = get_long_from_ipdotstring(argv[0]);

    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip server-backup address set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_server_backup_port_config_func,
    config_ctc_sip_server_backup_port_config_cmd,
    "sip server-backup port <1-65535>",
    "config sip\n"
    "Config for Backup Server PORT\n" 
    "Server Port config\n" 
    "Server Port (1-65535)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_server_backup_port;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.back_serv_com_port = VOS_AtoL(argv[0]);
    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip server-backup port set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}
DEFUN(config_ctc_sip_reg_realm_config_func,
    config_ctc_sip_reg_realm_config_cmd,
    "sip register address <address>",
    "config sip\n"
    "Config for Registar IP\n"
	"address config\n"
	"ip address (xx.xx.xx.xx)\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_reg_server_ip;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.reg_server_ip = get_long_from_ipdotstring(argv[0]);

    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip register realm set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_reg_port_config_func,
    config_ctc_sip_reg_port_config_cmd,
    "sip register port <1-65535>",
    "config sip\n"
    "Config for Registar PORT\n"
    "Register Port config\n"
    "Register Port (1-65535)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_reg_server_port;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.reg_serv_com_port = VOS_AtoL(argv[0]);
    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip register port set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_reg_backup_realm_config_func,
    config_ctc_sip_reg_backup_realm_config_cmd,
    "sip register-backup address <address>",
    "config sip\n"
    "Config for Backup Registar IP\n"
	"address config\n"
	"ip address (xx.xx.xx.xx)\n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_reg_server_backup_ip;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.back_reg_server_ip = get_long_from_ipdotstring(argv[0]);

    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip register-backup realm set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_reg_backup_port_config_func,
    config_ctc_sip_reg_backup_port_config_cmd,
    "sip register-backup port <1-65535>",
    "config sip\n"
    "Config for Backup Registar PORT\n"
    "Register Port config\n"
    "Register Port (1-65535)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_reg_server_backup_port;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.back_reg_serv_com_port = VOS_AtoL(argv[0]);
    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip register-backup port set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_outbound_proxy_address_config_func,
    config_ctc_sip_outbound_proxy_address_config_cmd,
    "sip outboundProxy address <adress>",
    "config sip\n"
    "Config for Outbound Proxy!\n"
    "Config for Outbound Proxy IP\n"
    "Outbound Proxy address config(xx.xx.xx.xx)\n" 
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_outbound_server_ip;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.outbound_server_ip = get_long_from_ipdotstring(argv[0]);

    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip outboundProxy address set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_outbound_proxy_port_config_func,
    config_ctc_sip_outbound_proxy_port_config_cmd,
    "sip outboundProxy port <1-65535>",
    "config sip\n"
    "Config for Outbound Proxy PORT\n"
    "Outbound Proxy config\n"
    "Outbound Proxy Port (1-65535)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_outbound_server_port;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.outbound_serv_com_port = VOS_AtoL(argv[0]);
    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%sip outboundProxy port set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_expires_config_func,
    config_ctc_sip_expires_config_cmd,
    "sip expires <60-3600>",
    "config sip\n"
    "Config for Expires\n" 
    "expires time(60-3600)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_reg_interval;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.reg_interval = VOS_AtoL(argv[0]);
    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip expires set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(ctc_sip_heartbeat_enable_config_func,
    ctc_sip_heartbeat_enable_config_cmd,
    "sip heartbeat [enable|disable]",
    "config sip\n"
    "heartbeat config\n" 
    "enable\n"
    "disable\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_heartbeat_enable;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    
    sip_param.heart_beat_switch =  VOS_StrCmp(argv[0], "enable")?CTC_STACK_SIP_HEART_BEAT_OFF:CTC_STACK_SIP_HEART_BEAT_ON;

    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip heartbeat enable set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;

}

DEFUN(ctc_sip_heartbeatimer_config,
	ctc_sip_heartbeatimer_config_cmd, 
	"sip heartbeat time <0-3600>", 
    "config sip\n"
	"heartbeat config\n"
	"heartbeat timer config\n" 
	"Heartbeat timer \n")
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = sip_config_heartbeat_time;
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    sip_param.heart_beat_cycle = VOS_AtoL(argv[0]);
    lRet = OnuMgt_SetSipConfig(PonPortIdx, OnuIdx, code, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip heartbeat time set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_digitmap_config_func,
    config_ctc_sip_digitmap_config_cmd,
    "sip dial-plan <dialplan>",
    "config sip\n"
    "Dial plan configuration\n"
    "Dial plan string(1-1024)\n" 
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    CTC_STACK_SIP_digit_map_t sip_digit_map;
    char digit_map[1024] = {0}; 
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_digit_map, sizeof(CTC_STACK_SIP_digit_map_t));
    sip_digit_map.digital_map_length = 1024;
    if(strlen(argv[0])>=1023)
        VOS_MemCpy(digit_map, argv[0], 1023);
    else
        VOS_StrCpy(digit_map, argv[0]);
    sip_digit_map.digital_map = digit_map;
    lRet = OnuMgt_SetSipDigitMap(PonPortIdx, OnuIdx, &sip_digit_map);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip digit-map set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(undo_ctc_sip_digitmap_config_func,
    undo_ctc_sip_digitmap_config_cmd,
    "undo sip dial-plan",
    "Negate a command or set its defaults\n"    
    "config sip\n"
    "Dial plan configuration\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    CTC_STACK_SIP_digit_map_t sip_digit_map;
    char digit_map[1024] = {"[*#]x[0-9*].#|**xx|*#x[0-9*].#|#*x[0-9*].#|#300##|#500##|[*#]96#xx.t|[*#]95#xx.t|*99*xx.#xx.t|*66*x[0-9*].#x[0-9*].t|##|010xxxxxxxx|02xxxxxxxxx|0[3-9]xxxxxxxxx|0311xxxxxxxx|037[179]xxxxxxxx|04[135]1xxxxxxxx|051[0-9]xxxxxxxx|052[37]xxxxxxxx|053[12]xxxxxxxx|057[1345679]xxxxxxxx|059[15]xxxxxxxx|0731xxxxxxxx|075[457]xxxxxxxx|076[09]xxxxxxxx|0898xxxxxxxx|00xxx.t|[2-8][1-9]xxxxx|[2-8][1-9]xxxxxx|1[358]xxxxxxxxx|01[358]xxxxxxxxx|11[02479]|12[0268]|11[13568]x.t|125xx|12[13479]x.t|100[015678]x|100[2349]x.t|10[1-9]xx.t|14xx.t|1[79]xx.t|160|168xxxxx|16[1-79]x.t|[48]00xxxxxxx|[48]0[1-9]x.t|[23567]0xx.t|955xx|95[0-46-9]xxx.t|9[0-46-9]xxxx.t|xxxxxxxx"}; 
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_digit_map, sizeof(CTC_STACK_SIP_digit_map_t));
    sip_digit_map.digital_map_length = 1024;
    sip_digit_map.digital_map = &digit_map;
    lRet = OnuMgt_SetSipDigitMap(PonPortIdx, OnuIdx, &sip_digit_map);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip digit-map set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_display_name_config_func,
    config_ctc_sip_display_name_config_cmd,
    "sip [phone1|phone2|phoneall] displayname <name>",
    "config sip\n"
    "SIP phone1 Config\n"
    "SIP phone2 Config\n"
    "SIP both phone1 and phone2 config\n" 
    "User DisplayName config\n" 
    "Display Name (1-64)\n"
    	)
{
    return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_user_name_config_func,
    config_ctc_sip_user_name_config_cmd,
    "sip [phone1|phone2|phoneall] username <name>",
    "config sip\n"
    "SIP phone1 Config\n"
    "SIP phone2 Config\n"
    "SIP both phone1 and phone2 Config\n"
    "UserName config\n"
    "User Name (1-32)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    ULONG length = 0;
    int slot = 0, port = 0;
    int code = sip_user_config_username;
    CTC_STACK_sip_user_param_config_t sip_user_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    if(VOS_StrCmp(argv[0], "phone1") == 0)
        port = 1;
    else if(VOS_StrCmp(argv[0], "phone2") == 0)
        port = 2;
    else
        port = 0;
    
    VOS_MemZero(&sip_user_param, sizeof(CTC_STACK_sip_user_param_config_t));
    length = strlen(argv[1]);
    if(length<SIP_USER_NAME_SIZE)
        VOS_MemCpy(&sip_user_param.user_name[SIP_USER_NAME_SIZE-length], argv[1], length);
    else
        VOS_MemCpy(sip_user_param.user_name, argv[1], SIP_USER_NAME_SIZE);
    if(port)
        lRet = OnuMgt_SetSipUserConfig(PonPortIdx, OnuIdx, port, code, &sip_user_param);   
    else
    {
        int maxport = GetOnuVoipMaxPortNum(PonPortIdx, OnuIdx);
        lRet = 0;
        for(port=1;port<=maxport;port++)
        {
            lRet |= OnuMgt_SetSipUserConfig(PonPortIdx, OnuIdx, port, code, &sip_user_param);   
        }
    }
    if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip username config set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_password_config_func,
    config_ctc_sip_password_config_cmd,
    "sip [phone1|phone2|phoneall] password <pass>",
    "config sip\n"
    "SIP phone1 Config\n"
    "SIP phone2 Config\n"
    "SIP both phone1 and phone2 Config\n" 
    "PassWord config\n" 
    "User passwords (1-16)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    ULONG length = 0;    
    int slot = 0, port = 0;
    int code = sip_user_config_password;
    CTC_STACK_sip_user_param_config_t sip_user_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    if(VOS_StrCmp(argv[0], "phone1") == 0)
        port = 1;
    else if(VOS_StrCmp(argv[0], "phone2") == 0)
        port = 2;
    else
        port = 0;
    
    VOS_MemZero(&sip_user_param, sizeof(CTC_STACK_sip_user_param_config_t));
    length = strlen(argv[1]);
    if(length<SIP_PASSWD_SIZE)
        VOS_MemCpy(&sip_user_param.passwd[SIP_PASSWD_SIZE-length], argv[1], length);
    else
        VOS_MemCpy(sip_user_param.passwd, argv[1], SIP_PASSWD_SIZE);
    
    if(port)
        lRet = OnuMgt_SetSipUserConfig(PonPortIdx, OnuIdx, port, code, &sip_user_param);   
    else
    {
        int maxport = GetOnuVoipMaxPortNum(PonPortIdx, OnuIdx);
        lRet = 0;
        for(port=1;port<=maxport;port++)
        {
            lRet |= OnuMgt_SetSipUserConfig(PonPortIdx, OnuIdx, port, code, &sip_user_param);   
        }
    }
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip password config set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(config_ctc_sip_account_config_func,
    config_ctc_sip_account_config_cmd,
    "sip [phone1|phone2|phoneall] account <account>",
    "config sip\n"
    "SIP phone1 Config\n"
    "SIP phone2 Config\n"
    "SIP both phone1 and phone2 Config\n" 
    "User Account config\n"
    "User Account (1-16)\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet = 0;
    int slot = 0, port = 0;
    ULONG length = 0;
    int code = sip_user_config_account;
    CTC_STACK_sip_user_param_config_t sip_user_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    if(VOS_StrCmp(argv[0], "phone1") == 0)
        port = 1;
    else if(VOS_StrCmp(argv[0], "phone2") == 0)
        port = 2;
    else
        port = 0;
    
    VOS_MemZero(&sip_user_param, sizeof(CTC_STACK_sip_user_param_config_t));
    length = strlen(argv[1]);
    if(length<SIP_PORT_NUM_SIZE)
        VOS_MemCpy(&sip_user_param.sip_port_num[SIP_PORT_NUM_SIZE-length], argv[1], length);
    else
        VOS_MemCpy(sip_user_param.sip_port_num, argv[1], SIP_PORT_NUM_SIZE);
    
    if(port)
        lRet = OnuMgt_SetSipUserConfig(PonPortIdx, OnuIdx, port, code, &sip_user_param);   
    else
    {
        int maxport = GetOnuVoipMaxPortNum(PonPortIdx, OnuIdx);
        lRet = 0;
        for(port=1;port<=maxport;port++)
        {
            lRet |= OnuMgt_SetSipUserConfig(PonPortIdx, OnuIdx, port, code, &sip_user_param);   
        }
    }
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip account config set failed!\r\n");	
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}

DEFUN(config_ctc_show_sip_config_func,
    config_ctc_show_sip_config_cmd,
    "show sip [phone1|phone2|phoneall]",
    "Show running system information\n"
    "config sip\n"
    "Show SIP configuration for phone1\n"
    "Show SIP configuration for phone2\n" 
    "Show SIP configuration for both phone1 and phone2\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	ULONG ulRet;
    int slot = 0, port = 0;
    char temp_str[40]={0};
    CTC_STACK_sip_user_param_config_array sip_user_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    if(VOS_StrCmp(argv[0], "phone1") == 0)
        port = 1;
    else if(VOS_StrCmp(argv[0], "phone2") == 0)
        port = 2;
    else
        port = 0;
    
    VOS_MemZero(&sip_user_param, sizeof(CTC_STACK_sip_user_param_config_t));    
    lRet = OnuMgt_GetSipUserConfig(PonPortIdx, OnuIdx, port, &sip_user_param);   
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Sip User config get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        int i = 0;
        if(port)
        {
            unsigned short port = sip_user_param.sip_user_param_array[i].management_object_index.port_number;
    		vty_out(vty, "Voice phone%d's SIP User Config:\r\n", port);
            ctc_voip_copy(temp_str, sip_user_param.sip_user_param_array[i].sip_user_param_config.sip_port_num, SIP_PORT_NUM_SIZE);
            vty_out(vty, "  %-17s: %s\r\n", "User Phone Number", temp_str);
            ctc_voip_copy(temp_str, sip_user_param.sip_user_param_array[i].sip_user_param_config.user_name, SIP_USER_NAME_SIZE);           
            vty_out(vty, "  %-17s: %s\r\n", "User Name", temp_str);
            ctc_voip_copy(temp_str, sip_user_param.sip_user_param_array[i].sip_user_param_config.passwd, SIP_PASSWD_SIZE);            
            vty_out(vty, "  %-17s: %s\r\n", "User Password", temp_str);
        }
        else
        {
            int maxport = GetOnuVoipMaxPortNum(PonPortIdx, OnuIdx);
            
            for(i=0;i<maxport;i++)
            {
                int j = 0;
                unsigned short port = sip_user_param.sip_user_param_array[i].management_object_index.port_number;
        		vty_out(vty, "Voice phone%d's SIP User Config:\r\n", port);
                ctc_voip_copy(temp_str, sip_user_param.sip_user_param_array[i].sip_user_param_config.sip_port_num, SIP_PORT_NUM_SIZE);
                vty_out(vty, "  %-17s: %s\r\n", "User Phone Number", temp_str);
                ctc_voip_copy(temp_str, sip_user_param.sip_user_param_array[i].sip_user_param_config.user_name, SIP_USER_NAME_SIZE);           
                vty_out(vty, "  %-17s: %s\r\n", "User Name", temp_str);
                ctc_voip_copy(temp_str, sip_user_param.sip_user_param_array[i].sip_user_param_config.passwd, SIP_PASSWD_SIZE);            
                vty_out(vty, "  %-17s: %s\r\n", "User Password", temp_str);
            }
        }
    }
    return CMD_SUCCESS;
}
DEFUN(show_ctc_sip_config_func,
    show_ctc_sip_config_cmd,
    "show sip configuration",
    "Show running system information\n"
    "config sip\n"
    "Voice configuration\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    char ipstr[33]={0};    
    CTC_STACK_sip_param_config_t sip_param;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    if(IsSupportSip(PonPortIdx, OnuIdx)!=VOS_OK)
    {
        vty_out(vty, "  %%Onu do not support Sip!\r\n");
		return CMD_WARNING;
    }
    
    VOS_MemZero(&sip_param, sizeof(CTC_STACK_sip_param_config_t));
    
    lRet = OnuMgt_GetSipConfig(PonPortIdx, OnuIdx, &sip_param);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%voice sip config get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        vty_out(vty, "Voice SIP Parameter Config:\r\n");
        vty_out(vty, "  %-31s: %d\r\n", "MG Port", sip_param.mg_port);
        get_ipdotstring_from_long(ipstr, sip_param.server_ip);
        vty_out(vty, "  %-31s: %s\r\n", "Sip Proxy Server Ip", ipstr);
        vty_out(vty, "  %-31s: %d\r\n", "Sip Proxy Server Port", sip_param.serv_com_port);
        get_ipdotstring_from_long(ipstr, sip_param.back_server_ip);
        vty_out(vty, "  %-31s: %s\r\n", "Backup Sip Proxy Server Ip", ipstr);
        vty_out(vty, "  %-31s: %d\r\n", "Backup Sip Proxy Server Port", sip_param.back_serv_com_port);
        vty_out(vty, "  %-31s: %d\r\n", "Active Sip Proxy Server", sip_param.active_proxy_server);
        get_ipdotstring_from_long(ipstr, sip_param.reg_server_ip);
        vty_out(vty, "  %-31s: %s\r\n", "Sip Register Server IP", ipstr);
        vty_out(vty, "  %-31s: %d\r\n", "Sip Register Server Port", sip_param.reg_serv_com_port);
        get_ipdotstring_from_long(ipstr, sip_param.back_reg_server_ip);
        vty_out(vty, "  %-31s: %s\r\n", "Backup Sip Register Server Ip", ipstr);
        vty_out(vty, "  %-30s: %d\r\n", "Backup Sip Register Server Port", sip_param.back_reg_serv_com_port);
        get_ipdotstring_from_long(ipstr, sip_param.outbound_server_ip);
        vty_out(vty, "  %-31s: %s\r\n", "OutBound Server IP", ipstr);
        vty_out(vty, "  %-31s: %d\r\n", "OutBound Server Port", sip_param.outbound_serv_com_port);
        vty_out(vty, "  %-31s: %d\r\n", "Sip Register Interval", sip_param.reg_interval);
        vty_out(vty, "  %-31s: %s\r\n", "Heartbeat Switch", sip_param.heart_beat_switch == CTC_STACK_SIP_HEART_BEAT_OFF?"Disable":"Enable");
        vty_out(vty, "  %-31s: %d\r\n", "Heartbeat Cycle", sip_param.heart_beat_cycle);
        vty_out(vty, "  %-31s: %d\r\n", "Heartbeat Count", sip_param.heart_beat_count);        
    }
	return CMD_SUCCESS;
}
#endif

#if 1
DEFUN(
    ctc_onu_voip_fax_mode,
	ctc_onu_voip_fax_mode_cmd, 
	"voice fax [t38|pass-through]", 
	"Voice module configuration\n" 
	"Fax mode \n" 
	"T.38 relay\n"
	"Pass through in band\n"
)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = voice_fax_config_t38_enable;
    CTC_STACK_voip_fax_config_t voip_fax;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&voip_fax, sizeof(CTC_STACK_voip_fax_config_t));
    if(VOS_StrCmp(argv[0], "t38") == 0)        
        voip_fax.t38_enable = CTC_STACK_VOIP_FAX_T38;
    else
        voip_fax.t38_enable = CTC_STACK_VOIP_FAX_TRANSPARENT;
        
    lRet = OnuMgt_SetVoipFaxConfig(PonPortIdx, OnuIdx, code, &voip_fax);        
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Voice fax enable set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
DEFUN(
    ctc_onu_voip_fax_control,
	ctc_onu_voip_fax_control_cmd, 
	"voice fax control [negotiated|auto-vbd]", 
	"Voice module configuration\n" 
	"Fax mode\n" 
	"Fax Control\n" 
	"Negotiated\n"
	"Auto VBD\n"
)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int code = voice_fax_config_control;
    CTC_STACK_voip_fax_config_t voip_fax;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&voip_fax, sizeof(CTC_STACK_voip_fax_config_t));
    if(VOS_StrCmp(argv[0], "negotiated") == 0)        
        voip_fax.fax_control = CTC_STACK_VOIP_FAX_CONTROL_NEGOTIATED;
    else
        voip_fax.fax_control = CTC_STACK_VOIP_FAX_CONTROL_AUTO_VBD;
        
    lRet = OnuMgt_SetVoipFaxConfig(PonPortIdx, OnuIdx, code, &voip_fax);        
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Voice fax control set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(
    show_ctc_onu_voip_fax_mode,
	show_ctc_onu_voip_fax_mode_cmd, 
	"show voice fax", 
    "Show running system information\n"	
	"Voice module configuration\n" 
	"Fax mode \n" 
)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    CTC_STACK_voip_fax_config_t voip_fax;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&voip_fax, sizeof(CTC_STACK_voip_fax_config_t));
    lRet = OnuMgt_GetVoipFaxConfig(PonPortIdx, OnuIdx, &voip_fax);        
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Voice fax get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        vty_out(vty, "Voice fax Configuration:\r\n");
        vty_out(vty, "  %-23s: %s\r\n", "Voice T38 Enable", voip_fax.t38_enable == CTC_STACK_VOIP_FAX_T38?"T38":"Transparent");
        vty_out(vty, "  %-23s: %s\r\n", "Voice Fax/Modem Control", voip_fax.fax_control == CTC_STACK_VOIP_FAX_CONTROL_NEGOTIATED?"Negotiated":"Auto VBD");
    }
	return CMD_SUCCESS;
}

DEFUN(config_ctc_iad_status_func,
    config_ctc_iad_status_cmd,
    "voice iad {[register|deregister|reset]}*1",
    "Config voice module information\n"    
    "Config IAD\n"
    "Iad Module re-register\n"
    "Iad Module deregister\n"
    "Iad Module reset\n"    
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    char ipstr[33]={0};    
    CTC_STACK_operation_type_t iad_oper_status;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&iad_oper_status, sizeof(CTC_STACK_operation_type_t));
    if(VOS_StrCmp(argv[0], "register") == 0)
        iad_oper_status = CTC_STACK_OPERATION_TYPE_REGISTER;
    else if(VOS_StrCmp(argv[0], "deregister") == 0)
        iad_oper_status = CTC_STACK_OPERATION_TYPE_DEREGISTER;
    else 
        iad_oper_status = CTC_STACK_OPERATION_TYPE_RESET;
    
    lRet = OnuMgt_SetVoipIadOperation(PonPortIdx, OnuIdx, iad_oper_status);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Voice IAD Set failed!\r\n");	
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(show_ctc_iad_status_func,
    show_ctc_iad_status_cmd,
    "show voice iad status",
    "Show running system information\n"
    "Config voice module information\n"    
    "Config IAD\n"
    "Work Status\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    char ipstr[33]={0};    
    CTC_STACK_voip_iad_oper_status_t iad_oper_status;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
    
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&iad_oper_status, sizeof(CTC_STACK_voip_iad_oper_status_t));
    
    lRet = OnuMgt_GetVoipIadOperStatus(PonPortIdx, OnuIdx, &iad_oper_status);    
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Voice IAD status get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        if(CTC_STACK_VOIP_IAD_OPER_STATUS_REGISTERING == iad_oper_status)
            vty_out(vty, "Voice IAD status is %s!\r\n", "REGISTERING");
        else if(CTC_STACK_VOIP_IAD_OPER_STATUS_REGISTERED == iad_oper_status)
            vty_out(vty, "Voice IAD status is %s!\r\n", "REGISTERED");
        else if(CTC_STACK_VOIP_IAD_OPER_STATUS_FAULT == iad_oper_status)
            vty_out(vty, "Voice IAD status is %s!\r\n", "FAULT");
        else if(CTC_STACK_VOIP_IAD_OPER_STATUS_DEREGISTER == iad_oper_status)
            vty_out(vty, "Voice IAD status is %s!\r\n", "DEREGISTER");
        else if(CTC_STACK_VOIP_IAD_OPER_STATUS_REBOOT == iad_oper_status)
            vty_out(vty, "Voice IAD status is %s!\r\n", "REBOOT");
        else
            vty_out(vty, "Voice IAD status is %s!\r\n", "NULL");
    }
	return CMD_SUCCESS;
}
DEFUN(show_ctc_iad_config_func,
    show_ctc_iad_config_cmd,
    "show voice iad configuration",
    "Show running system information\n"
    "Config voice module information\n"    
    "Config IAD\n"
    "Voice configuration\n"
    	)
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    char ipstr[33]={0};    
    CTC_STACK_voip_iad_info_t iad_info;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&iad_info, sizeof(CTC_STACK_voip_iad_info_t));
    lRet = OnuMgt_GetIADInfo(PonPortIdx, OnuIdx, &iad_info);   
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Voice IAD config get failed!\r\n");	
		return CMD_WARNING;
	}
    else
    {
        vty_out(vty, "Voice IAD Configuration:\r\n");
        vty_out(vty, "  %-20s: %02x%02x.%02x%02x.%02x%02x\r\n", "Mac address",iad_info.mac_address[0], iad_info.mac_address[1],
            iad_info.mac_address[2], iad_info.mac_address[3], iad_info.mac_address[4], iad_info.mac_address[5]);
        vty_out(vty, "  %-20s: %s\r\n", "Protocol Supported", iad_info.voip_protocol==CTC_STACK_VOIP_PROTOCOL_H248?"H248":"SIP");
        ctc_voip_copy(ipstr, iad_info.sw_version, VOIP_SW_VERSION_SIZE);
        vty_out(vty, "  %-20s: %s\r\n", "IAD Software Version", ipstr);
        vty_out(vty, "  %-20s: %c%c%c%c-%c%c-%c%c %c%c:%c%c:%c%c\r\n", "Iad software time", iad_info.sw_time[18], iad_info.sw_time[19], iad_info.sw_time[20], iad_info.sw_time[21], 
            iad_info.sw_time[22], iad_info.sw_time[23], iad_info.sw_time[24], iad_info.sw_time[25], iad_info.sw_time[26], 
            iad_info.sw_time[27], iad_info.sw_time[28], iad_info.sw_time[29], iad_info.sw_time[30], iad_info.sw_time[31]);
        vty_out(vty, "  %-20s: %d\r\n", "Voip User Count", iad_info.user_count);
    }
	return CMD_SUCCESS;
}

DEFUN(show_ctc_posts_status_config,
	show_ctc_posts_status_cmd,
	"show voice [phone1|phone2|phoneall]",
    "Show running system information\n"
    "Config voice module information\n"    
    "SIP phone1 Config\n"
    "SIP phone2 Config\n"
    "SIP both phone1 and phone2 Config\n" 
    )
{
	LONG lRet;
	short int PonPortIdx =0;
	short int OnuIdx;
	ULONG   ulIfIndex = 0;
	ULONG   ulSlot, ulPort, ulOnuid;
	USHORT length;
	ULONG ulRet;
    int slot = 0, port = 0;
    int status = 0;    
    CTC_STACK_voip_pots_status_array pots_status_array;
    
	ulIfIndex =(ULONG) vty->index;
	ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	if( ulRet !=VOS_OK )
		return (CMD_WARNING );

	PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
	OnuIdx = ulOnuid-1;
	CHECK_ONU_RANGE;
	
	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
    VOS_MemZero(&pots_status_array, sizeof(CTC_STACK_voip_pots_status_array));
    if(VOS_StrCmp(argv[0], "phone1") == 0)
        port = 1;
    else if(VOS_StrCmp(argv[0], "phone2") == 0)
        port = 2;
    else
        port = 0;
    
    lRet = OnuMgt_GetVoipPortStatus(PonPortIdx, OnuIdx, port, &pots_status_array);
	if(lRet != VOS_OK)
	{
		vty_out(vty, "  %%Voice %s's Status get failed!\r\n", argv[0]);	
		return CMD_WARNING;
	}
    else
    {
        int i = 0;
        for(i=0;i<pots_status_array.number_of_entries;i++)
        {
            int j = 0;
            unsigned short port = pots_status_array.pots_status_array[i].management_object_index.port_number;
            status = pots_status_array.pots_status_array[i].pots_status.codec_mode;
            if(status>=0 && status <=5)
            {
        		vty_out(vty, "Voice phone%d's Status:\r\n", port);
                status = pots_status_array.pots_status_array[i].pots_status.port_status;
                vty_out(vty, "  %-22s: %s\r\n", "IAD Port Stauts", ctconu_iad_status_str[status]);
                status = pots_status_array.pots_status_array[i].pots_status.service_status;
                vty_out(vty, "  %-22s: %s\r\n", "IAD Port Service State", ctconu_iad_servive_str[status]);
                status = pots_status_array.pots_status_array[i].pots_status.codec_mode;
                vty_out(vty, "  %-22s: %s\r\n", "IAD Port Codec Mode", ctconu_iad_codec_str[status]);
            }
        }
    }
	return CMD_SUCCESS;
}
DEFUN(save_ctc_voip_config,
	save_ctc_voip_config_cmd,
	"mgt voip save",
    "Show or config onu mgt information\n"
    "Config voice module information\n"    
    "Save configuration\n"
    )
{
    LONG lRet;
    CHAR pBuff[256] = {0};
    short int PonPortIdx =0;
    short int OnuIdx;
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    USHORT length = 0;
    ULONG ulRet;
    char *stPayload=NULL;
    int OnuType;

    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return CMD_SUCCESS;

    ulIfIndex =(ULONG) vty->index;
    ulRet = PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
    if( ulRet !=VOS_OK )
        return (CMD_WARNING );

    PonPortIdx = GetPonPortIdxBySlot((short int)ulSlot, (short int)ulPort);
    OnuIdx = ulOnuid-1;
    CHECK_ONU_RANGE;

	CHECK_ONU_IS_ONLINE(PonPortIdx, OnuIdx)
    CHECK_IS_SUPPORT_VOIP(PonPortIdx, OnuIdx)
    
	length = VOS_Sprintf( pBuff, "mgt voip save\r\n");
    if(OnuMgt_CliCall(PonPortIdx, OnuIdx, pBuff, length, &stPayload, &length) == VOS_OK && !length)
    {
        vty_out(vty, "Voip config data save success!\r\n");
        /*vty_out( vty, "  %% %s OK!\r\n", hint_str );*/   /* Œ Ã‚µ•9302 */
    }
    else
    {
        vty_out(vty, "  %% Voip config data save failed!\r\n");
        return CMD_WARNING;
    }
    vty_out(vty, "\r\n");

    return CMD_SUCCESS;
}
#endif

LONG  CTC_VoipCmd_Init(enum node_type  node)
{    
	install_element (node, &config_ctc_voice_ip_mode_cmd);
	install_element (node, &config_ctc_voice_ipaddr_cmd);
	install_element (node, &config_ctc_voice_ipaddr1_cmd);
	install_element (node, &undo_ctc_voice_ipaddr_cmd);
	install_element (node, &show_ctc_voice_ip_cmd);
	install_element (node, &config_ctc_static_voice_ipgateway_cmd);
	install_element (node, &undo_ctc_static_voice_ipgateway_cmd);
	install_element (node, &config_ctc_voice_pppoe_mode_cmd);
	install_element (node, &config_ctc_voice_pppoe_username_cmd);
	install_element (node, &config_ctc_voice_pppoe_pwd_cmd);
	install_element (node, &config_ctc_voice_vlan_cmd);
	install_element (node, &show_ctc_voice_config_cmd);

	install_element (node, &ctc_h248_local_port_config_cmd);
	install_element (node, &ctc_h248_mgc_address_config_cmd);
	install_element (node, &ctc_h248_mgc_port_config_cmd);
	install_element (node, &ctc_h248_mgcbackup_address_cmd);
	install_element (node, &ctc_h248_mgcbackup_port_cmd);
	install_element (node, &ctc_h248_registermode_config_cmd);
	install_element (node, &ctc_h248_local_domain_config_cmd);
	install_element (node, &ctc_h248_mgheartbeat_cmd);
	install_element (node, &ctc_h248_heartbeatimer_config_cmd);
	install_element (node, &ctc_h248_local_name_config_cmd);
	install_element (node, &ctc_h248_local_rtpprefix_config_cmd);
	install_element (node, &show_ctc_h248_config_cmd);
	install_element (node, &show_ctc_h248_user_config_cmd);
	install_element (node, &show_ctc_h248_rtp_config_cmd);

	install_element (node, &config_ctc_sip_local_port_config_cmd);
	install_element (node, &config_ctc_sip_server_address_config_cmd);
	install_element (node, &config_ctc_sip_server_port_config_cmd);
	install_element (node, &config_ctc_sip_server_backup_address_config_cmd);
	install_element (node, &config_ctc_sip_server_backup_port_config_cmd);
	install_element (node, &config_ctc_sip_reg_realm_config_cmd);
	install_element (node, &config_ctc_sip_reg_port_config_cmd);
	install_element (node, &config_ctc_sip_reg_backup_realm_config_cmd);
	install_element (node, &config_ctc_sip_reg_backup_port_config_cmd);
	install_element (node, &config_ctc_sip_outbound_proxy_address_config_cmd);
	install_element (node, &config_ctc_sip_outbound_proxy_port_config_cmd);
	install_element (node, &config_ctc_sip_expires_config_cmd);
	install_element (node, &ctc_sip_heartbeat_enable_config_cmd);
	install_element (node, &ctc_sip_heartbeatimer_config_cmd);    
	install_element (node, &config_ctc_sip_digitmap_config_cmd);
	install_element (node, &undo_ctc_sip_digitmap_config_cmd);    
	install_element (node, &config_ctc_sip_display_name_config_cmd);
	install_element (node, &config_ctc_sip_user_name_config_cmd);
	install_element (node, &config_ctc_sip_password_config_cmd);
	install_element (node, &config_ctc_sip_account_config_cmd);
	install_element (node, &config_ctc_show_sip_config_cmd);
	install_element (node, &show_ctc_sip_config_cmd);

	install_element (node, &ctc_onu_voip_fax_mode_cmd);    
	install_element (node, &ctc_onu_voip_fax_control_cmd);     
	install_element (node, &show_ctc_onu_voip_fax_mode_cmd);
	install_element (node, &config_ctc_iad_status_cmd);
	install_element (node, &show_ctc_iad_status_cmd);
	install_element (node, &show_ctc_iad_config_cmd);
	install_element (node, &show_ctc_posts_status_cmd);
	install_element (node, &save_ctc_voip_config_cmd);
    
    return VOS_OK;
}

#ifdef	__cplusplus
}
#endif/* __cplusplus */
