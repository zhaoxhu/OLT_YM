/*--------------------------------------------------------------------*\
 *  File name:  access_id_main.c 
 *  Author:     wugang
 *  Version:    V1.0
 *  Date:       2009-06-04
 *  Description: ONU线路接入标识，包括PPPOE RELAY和DHCP RELAY(暂未实现)两部分功能
\*--------------------------------------------------------------------*/

#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"
#include "V2R1General.h"
#include "vos_io.h"
#include "sys/devsm/devsm_cli.h"

#include "access_id.h"

char Access_id_module_state = ACCESS_ID_MODULE_FREE;/*定义ONU用户线路接入标识模块是否正被占用*/

char g_PPPOE_relay = PPPOE_RELAY_DISABLE;/*定义pppoe relay 是否使能*/
char g_PPPOE_relay_mode = PPPOE_RELAY_DSL_RORUM_MODE;/*定义pppoe relay模式*/
char PPPOE_relay_maual_string[35];
char *PPPOE_Relay_Maual_String_head = PPPOE_relay_maual_string;
int PPPOE_Relay_Maual_String_Len = 0;

char g_DHCP_relay = DHCP_RELAY_DISABLE;/*dhcp relay是否是能*/
char g_DHCP_relay_mode = DHCP_RELAY_CTC_MODE;/*dhcp relay模式*/

int ResumePPPoEDelayConfig(short int PonPortIdx)
{
    if ('\0' != *PPPOE_Relay_Maual_String_head)
    {
        OLT_SetPPPoERelayParams(PonPortIdx, PPPOE_RELAY_PARAMID_STRHEAD, PPPOE_Relay_Maual_String_head, PPPOE_Relay_Maual_String_Len);
    }

    if (PPPOE_RELAY_ENABLE == g_PPPOE_relay)
    {
        OLT_SetPPPoERelayMode(PonPortIdx, PPPOE_RELAY_ENABLE, g_PPPOE_relay_mode);
    }

    return 0;
}

int ResumeDhcpDelayConfig(short int PonPortIdx)
{
    if (DHCP_RELAY_ENABLE == g_DHCP_relay)
    {
        OLT_SetDhcpRelayMode(PonPortIdx, DHCP_RELAY_ENABLE, g_DHCP_relay_mode);
    }

    return 0;
}

/*******************************************************************************
* PPPOE_RELAY_Oam_Pkt
*
* DESCRIPTION:
*       编译PPPOE RELAY buffer
*       把信息封装成GWD定义的RELAY OAM
*       如果发送超时或ONU的返回值错误，则重新发送，最多重发次数由宏控制
*
* INPUTS:
*       PonPortIdx - PON口索引.
*       OnuIdx - ONU索引.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       VOS_OK   - send success
*       VOS_FAIL - send error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
STATUS PPPOE_RELAY_Oam_Pkt(ULONG PonPortIdx, ULONG OnuIdx)
{
    ACCESS_ID_OAM_s pppoe_relay_oam_t;
    ACCESS_ID_OAM_s *PPPOE_Relay_Oam = &pppoe_relay_oam_t;
    ACCESS_ID_OAM_s *OAM_Relay_Recv = NULL;
    int lret;/*发送报文函数的返回值*/
    int str_length = 0;
    UCHAR *pMacAddr;
    unsigned char *temp_str = NULL;/*移动的指针*/
    short int ulslot = 0;
    short int ulport = 0;
    ULONG ONU_id = 0;
    char temp_len;
    int count;/*对错误返回的报文重发次数*/
    char liv_RecvMsgFromOnu[EUQ_MAX_OAM_PDU];
    short int liv_recBuflen = 0;
    temp_str = PPPOE_Relay_Oam->relay_str;

    VOS_MemSet( liv_RecvMsgFromOnu, 0,  EUQ_MAX_OAM_PDU );
  
    /*获取OLT的MAC地址*/
    /*funReadMacAddFromNvram( &macStructure );*/	/* modified by xieshl 20091216, 防止读eeprom */
	pMacAddr = SYS_PRODUCT_BASEMAC;

    /*设置OAM_TYPE值*/
    PPPOE_Relay_Oam->msg_type = ACCESS_ID_OAM;

    /*设置结果，用0填充*/
    PPPOE_Relay_Oam->result = 0;
  
    /*检查RELAY种类*/
    PPPOE_Relay_Oam->relay_type = RELAY_TYPE_PPPOE;

    /*公用部分*/
    ulslot = GetCardIdxByPonChip( (short int)PonPortIdx );
    ulport = GetPonPortByPonChip( (short int)PonPortIdx );
    ONU_id = OnuIdx + 1;

    if ( PPPOE_RELAY_DISABLE == g_PPPOE_relay)
    {
        PPPOE_Relay_Oam->relay_mode = PPPOE_RELAY_DISABLE;
    }
    else
    {
        PPPOE_Relay_Oam->relay_mode = g_PPPOE_relay_mode;
    }

    if ( PPPOE_RELAY_DISABLE == g_PPPOE_relay)/*如果关闭relay，则发空string*/
    {
        *temp_str = '\0';
        temp_str ++;
    }
    else
    {
        if('\0' != *PPPOE_Relay_Maual_String_head)/*手动设置*/
        {
            /*获取字符串指针和长度*/
            VOS_MemCpy(temp_str, PPPOE_Relay_Maual_String_head, PPPOE_Relay_Maual_String_Len);
            temp_str += PPPOE_Relay_Maual_String_Len;      
        }
        else
        {
            if(PPPOE_RELAY_FPT_PRIVATE_MODE == g_PPPOE_relay_mode)
            {
                char *epon_tech = " PON ";/*空格是要求的*/
        		char oltName[MAXDEVICENAMELEN];
        		ULONG nameLen = 0;
            	GetOltDeviceName( oltName, &nameLen);
            	if( nameLen >20 )
            	{
            		nameLen = 20;
            	}
                oltName[nameLen] = '\0';
                /*sys_console_printf("\r\n oltName = %s \r\n", oltName);*/
                    /*device name + mac地址*/
                VOS_Sprintf( temp_str,"%s", oltName);
                temp_str += nameLen;

                /*epon技术标识*/
                VOS_MemCpy( temp_str, epon_tech, 5);
                temp_str += 5;
                
            }
            else
            {
                char *epon_tech = " epon ";/*空格是要求的*/
        		char oltName[MAXDEVICENAMELEN];
        		ULONG nameLen = 0;
            	GetOltDeviceName( oltName, &nameLen);
            	if( nameLen >20 )
            	{
            		nameLen = 20;
            	}
                oltName[nameLen] = '\0';
                /*sys_console_printf("\r\n oltName = %s \r\n", oltName);*/
                    /*device name + mac地址*/
                VOS_Sprintf( temp_str,"<%s>%02x%02x%02x%02x%02x%02x", oltName,
                		pMacAddr[0], pMacAddr[1], pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5] );
                temp_str += (nameLen + 2 + 12);

                /*epon技术标识*/
                VOS_MemCpy( temp_str, epon_tech, 6 );
                temp_str += 6;
            }
        }
        
        temp_len = VOS_Sprintf(temp_str,"%d/%d/%d",ulslot,ulport,ONU_id);
        temp_str += temp_len;
    }

    str_length = temp_str - (unsigned char *)PPPOE_Relay_Oam;

    for(count = 0; count < MAX_RETRY_COUNT; count ++)
    {
        /*modified by luh 2012-9-17*/        
        lret = Onu_Access_Msg_Oam_pkt(PonPortIdx, OnuIdx, ACCESS_ID_OAM, (unsigned char *)(&PPPOE_Relay_Oam->result), str_length-1, liv_RecvMsgFromOnu, &liv_recBuflen);

        /*对返回值进行检测*/

        if( lret != VOS_OK )
        {
            continue;/*再发一次*/
        }
    
        OAM_Relay_Recv = (ACCESS_ID_OAM_s *)/*RecvMsgFromOnu*/liv_RecvMsgFromOnu;

        if( OAM_Relay_Recv->msg_type != ACCESS_ID_OAM )
        {
            sys_console_printf("\r\n  received wrong OAM message type from %d/%d/%d !\r\n",ulslot,ulport,ONU_id);
            return VOS_ERROR;
        }
			
        if( OAM_Relay_Recv->result != ONU_ACCESS_ID_SUCCESS )
        {
            sys_console_printf("\r\n  send pppoe relay relay circuit id to onu %d/%d/%d failed!\r\n",ulslot,ulport,ONU_id);
			return VOS_ERROR;
        }     
        return VOS_OK;
    }
    if (lret != VOS_OK)
    {
        sys_console_printf("\r\n  send pppoe relay circuit id value to onu %d/%d/%d failed!\r\n",ulslot,ulport,ONU_id);
        return VOS_ERROR;
    }
    return VOS_OK;
}

/*******************************************************************************
* PPPOE_RELAY_Oam_Pkt
*
* DESCRIPTION:
*       编译PPPOE RELAY buffer
*       把信息封装成GWD定义的RELAY OAM
*       如果发送超时或ONU的返回值错误，则重新发送，最多重发次数由宏控制
*
* INPUTS:
*       PonPortIdx - PON口索引.
*       OnuIdx - ONU索引.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       VOS_OK   - send success
*       VOS_FAIL - send error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
STATUS DHCP_RELAY_Oam_Pkt(ULONG PonPortIdx, ULONG OnuIdx)
{
    ACCESS_ID_OAM_s dhcp_relay_oam_t;
    ACCESS_ID_OAM_s *DHCP_Relay_Oam = &dhcp_relay_oam_t;
    ACCESS_ID_OAM_s *OAM_Relay_Recv = NULL;
    int lret;/*发送报文函数的返回值*/
    int str_length = 0;
    UCHAR * pMacAddr;
    unsigned char *temp_str = NULL;/*移动的指针*/
    short int ulslot = 0;
    short int ulport = 0;
    ULONG ONU_id = 0;
    char temp_len;
    int count;/*对错误返回的报文重发次数*/
    char liv_RecvMsgFromOnu[EUQ_MAX_OAM_PDU];
    short int liv_RecvMsgLen = 0;
    temp_str = DHCP_Relay_Oam->relay_str;
  
    /*获取OLT的MAC地址*/
    /*funReadMacAddFromNvram( &macStructure );*/	/* modified by xieshl 20091216, 防止读eeprom */
	pMacAddr = SYS_PRODUCT_BASEMAC;
	
    VOS_MemSet( liv_RecvMsgFromOnu, 0,  EUQ_MAX_OAM_PDU);

    /*设置OAM_TYPE值*/
    DHCP_Relay_Oam->msg_type = ACCESS_ID_OAM;

    /*设置结果，用0填充*/
    DHCP_Relay_Oam->result = 0;
  
    /*检查RELAY种类*/
    DHCP_Relay_Oam->relay_type = RELAY_TYPE_DHCP;

    /*公用部分*/
    ulslot = GetCardIdxByPonChip( (short int)PonPortIdx );
    ulport = GetPonPortByPonChip( (short int)PonPortIdx );
    ONU_id = OnuIdx + 1;

    if ( DHCP_RELAY_DISABLE == g_DHCP_relay)
    {
        DHCP_Relay_Oam->relay_mode = DHCP_RELAY_DISABLE;
    }
    else
    {
        DHCP_Relay_Oam->relay_mode = g_DHCP_relay_mode;
    }

    if ( PPPOE_RELAY_DISABLE == g_DHCP_relay)/*如果关闭relay，则发空string*/
    {
        *temp_str = '\0';
        temp_str ++;
    }
    else
    {
    	/*{eth|trk} NAS_slot/NAS_subslot/NAS_port:SVLAN.CVLAN AccessNodeIdentifier/ANI_rack/ANI_frame/ANI_slot/ANI_subslot/ANI_port/ */
        temp_len = VOS_Sprintf(temp_str,"0 0/0/0:0.0 %02x%02x%02x%02x%02x%02x/0/0/",
			pMacAddr[0], pMacAddr[1], pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]);
		temp_str += temp_len;
			
        
        temp_len = VOS_Sprintf(temp_str,"%d/0/%d/",ulslot,ulport);
        temp_str += temp_len;
    }

    str_length = temp_str - (unsigned char *)DHCP_Relay_Oam;

    for(count = 0; count < MAX_RETRY_COUNT; count ++)
    {
        /*modified by luh 2012-9-17*/
        lret = Onu_Access_Msg_Oam_pkt(PonPortIdx, OnuIdx, ACCESS_ID_OAM, (unsigned char *)(&DHCP_Relay_Oam->result), str_length-1, liv_RecvMsgFromOnu, &liv_RecvMsgLen);

        /*对返回值进行检测*/

        if( lret != VOS_OK )
        {
            continue;/*再发一次*/
        }
    
        OAM_Relay_Recv = (ACCESS_ID_OAM_s *)/*RecvMsgFromOnu*/liv_RecvMsgFromOnu;

        if( OAM_Relay_Recv->msg_type != ACCESS_ID_OAM )
        {
            sys_console_printf("\r\n  received wrong OAM message type from %d/%d/%d !\r\n",ulslot,ulport,ONU_id);
            return VOS_ERROR;
        }
			
        if( OAM_Relay_Recv->result != ONU_ACCESS_ID_SUCCESS )
        {
            sys_console_printf("\r\n  send dhcp relay relay circuit id to onu %d/%d/%d failed!\r\n",ulslot,ulport,ONU_id);
			return VOS_ERROR;
        }     
        return VOS_OK;
    }
    if (lret != VOS_OK)
    {
        sys_console_printf("\r\n  send dhcp relay circuit id value to onu %d/%d/%d failed!\r\n",ulslot,ulport,ONU_id);
        return VOS_ERROR;
    }
    return VOS_OK;
}



/*******************************************************************************
* Onu_Access_Msg_Oam_pkt
*
* DESCRIPTION:
*       接入线路信息 OAM报文发送
*       进行参数检查，并发送OAM帧
*
* INPUTS:
*       PonPortIdx  - PON口索引.
*       OnuIdx - ONU索引.
*       MsgType - OAM massage type
*       pBuf - 指向buffer的指针
*       length - buffer长度
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       VOS_OK   - 不需要调用者再发一次的返回值
*       VOS_FAIL - 需要调用者再发一次的返回值
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
STATUS Onu_Access_Msg_Oam_pkt(ULONG PonPortIdx, ULONG OnuIdx, char MsgType, unsigned char *pBuf, int length, unsigned char *pRxBuf, short int*RxBufLen)
{
    int ulslot,ulport,ONU_id;
    int status;

    /*参数检查*/
    if (MsgType != ACCESS_ID_OAM)
    {
        /*sys_console_printf(("Onu_Access_Msg_Oam_pkt():msg_type=wrong!\r\n"));*/
        return VOS_ERROR;    
    }

    if (NULL == pBuf)
    {
		/*sys_console_printf(("Onu_Access_Msg_Oam_pkt():pBuf=NULL!\r\n"));*/
		return VOS_ERROR;
	}

    /*ONU是否在线*/
	/*if (GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )*/
	status = GetOnuOperStatus_1(PonPortIdx, OnuIdx);
	if( (status != ONU_OPER_STATUS_UP) && (status != ONU_OPER_STATUS_DORMANT) )
	{
             ulslot = GetCardIdxByPonChip( (short int)PonPortIdx );
             ulport = GetPonPortByPonChip( (short int)PonPortIdx );
             ONU_id = OnuIdx + 1;

		/*sys_console_printf("onu:%d/%d/%d is not online!\r\n", ulslot, ulport,ONU_id);*/
		return VOS_ERROR;
	}

    /*发送OAM报文*/
    /*modified by luh 2012-9-17*/
    return Oam_Session_Send(PonPortIdx, OnuIdx, MsgType, 0, 0, 0, NULL, pBuf, length, pRxBuf, RxBufLen);
}


/* */
/*******************************************************************************
* onuIdx_Is_Support_Relay
*
* DESCRIPTION:
*      判断ONU设备支持何种pppoe relay并调用发送者
*
* INPUTS:
*       PonPortIdx  - PON口索引.
*       OnuIdx - ONU索引.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       VOS_OK   
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
STATUS onuIdx_Is_Support_Relay(ULONG PonPortIdx, ULONG OnuIdx, ULONG Relay_type)
{
#if 0
    int onuType;
    int OnuEntry;

    OnuEntry = PonPortIdx * MAXONUPERPON + OnuIdx;
    onuType = OnuMgmtTable[OnuEntry].DeviceInfo.type;
#endif

    if ( RELAY_TYPE_PPPOE == Relay_type )
    {
#if 0
        if ((onuType == V2R1_ONU_GT811_A)||(onuType == V2R1_ONU_GT812_A)||
                (onuType == V2R1_ONU_GT812_B)||(onuType == V2R1_ONU_GT815)||
                (onuType == V2R1_ONU_GT861)||(onuType == V2R1_ONU_GT863)||(onuType == V2R1_ONU_GT866))
#endif
            PPPOE_RELAY_Oam_Pkt( PonPortIdx, OnuIdx);    
    }
    
	if ( RELAY_TYPE_DHCP == Relay_type)
	{
#if 0
		if ((onuType == V2R1_ONU_GT811_A)||(onuType == V2R1_ONU_GT812_A)||
                (onuType == V2R1_ONU_GT812_B)||(onuType == V2R1_ONU_GT815)||
                (onuType == V2R1_ONU_GT861)||(onuType == V2R1_ONU_GT863)||(onuType == V2R1_ONU_GT866))
#endif
            DHCP_RELAY_Oam_Pkt( PonPortIdx, OnuIdx);
	}
    
    return VOS_OK;
}

