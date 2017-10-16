
#include "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_product.h"

#include "Tdm_comm.h"
#include "Tdm_apis.h"

#include "e1_oam.h"
#include "E1_MIB.h"


/* 打印OAM发送消息 */
extern void pktDataPrintf( uchar_t *pOctBuf, ulong_t bufLen );
static STATUS printTxOamMsg(char *buf, ULONG len)
{
	/*ULONG i;*/

	if (buf == NULL)
	{
		return VOS_ERROR;
	}
	if( debugE1 & (E1_ERROR_INFO | E1_TX_OAM_MSG) )
		sys_console_printf("\r\nTxLen=%d\r\n", len);
	if (debugE1 & E1_TX_OAM_MSG)
	{
		/*for (i = 0; i < len; i++)
		{
			if ( 0 == (i % 16) )
			{
				sys_console_printf(("\r\n"));
			}
			sys_console_printf("%02x ", buf[i]);
		}
		sys_console_printf("\r\n");*/
		pktDataPrintf( buf, len );	/* modified by xieshl 20100202 */
	}
	return VOS_OK;
}

/* 打印OAM接收消息 */
/*static*/ STATUS printRxOamMsg(char *buf, ULONG len)
{
	/*ULONG i;*/

	if (buf == NULL)
	{
		sys_console_printf(("printRxOamMsg()::buf=NULL\r\n"));
		return VOS_ERROR;
	}
	if( debugE1 & (E1_ERROR_INFO | E1_RX_OAM_MSG) )
		sys_console_printf("\r\nRxLen=%d\r\n", len);

	if (debugE1 & E1_RX_OAM_MSG)
	{
		/*for (i = 0; i < len; i++)
		{
			if ( 0 == (i % 16) )
			{
				sys_console_printf(("\r\n"));
			}
			sys_console_printf("%02x ", buf[i]);
		}
		sys_console_printf("\r\n");*/
		pktDataPrintf( buf, len );
	}
	return VOS_OK;
}


static void printMac(unsigned char *mac)
{
	/*unsigned char i;

	E1_ERROR_INFO_PRINT(("\r\n MAC:   "));

	for (i = 0; i < 6; i++)
	{
		E1_ERROR_INFO_PRINT(("0x%02x ", mac[i]));
	}

	E1_ERROR_INFO_PRINT(("\r\n"));*/
	sys_console_printf("\r\n MAC:%02x%02x.%02x%02x.%02x%02x\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static STATUS checkE1Mac(unsigned char *mac)
{
	unsigned char i;

	for (i = 0; i < 4; i++)
	{
		if (mac[i] != E1MAC[i])
		{
			return VOS_ERROR;
		}
	}

	if ( (mac[4] < 0x03) || (mac[4] > 0x05) )
	{
		return VOS_ERROR;
	}

	if ( ( mac[5] > 0x07 ) && ( (mac[5] < 0x80) ||(mac[5] > 0x87) ) )
	{
		return VOS_ERROR;
	}

	return VOS_OK;
}

static STATUS checkVlanTag(unsigned short VlanTag)
{
	unsigned short VlanId = 0;
	unsigned char  VlanPri = 0;

	VlanId  = VlanTag & 0x0FFF;
	VlanPri = (unsigned char)(VlanTag >> 13);

	if (VlanId > MAX_VLAN_ID)
	{
		E1_ERROR_INFO_PRINT(("checkVlanTag()::VlanId=%d\r\n", VlanId));
		return VOS_ERROR;
	}

	if (VlanPri > MAX_VLAN_PRI)
	{
		E1_ERROR_INFO_PRINT(("checkVlanTag()::VlanPri=%d\r\n", VlanPri));
		return VOS_ERROR;
	}

	return VOS_OK;
}

static STATUS checkOAM_OnuE1Link(OAM_OnuE1Link *pOAM_OnuE1Link)
{
	unsigned char i;

	if (NULL == pOAM_OnuE1Link)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::pOAM_OnuE1Link=NULL   error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	if (SET_ONU_E1_LINK_REQ != pOAM_OnuE1Link->MsgType)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::MsgType=0x%02x   error!!\r\n", pOAM_OnuE1Link->MsgType));
		return V2R1_E1_ERROR;
	}

	if ( (0 == pOAM_OnuE1Link->E1PortTotalCount) || (MAX_ONU_E1 < pOAM_OnuE1Link->E1PortTotalCount) )
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::E1PortTotalCount=%d   error!\r\n", pOAM_OnuE1Link->E1PortTotalCount));
		return V2R1_E1_ERROR;
	}

	for (i = 0; i < pOAM_OnuE1Link->E1PortTotalCount; i++)
	{
		if ( pOAM_OnuE1Link->oam_OnuE1Link[i].E1PortIdx > MAX_ONU_BOARD_E1 )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::Port E1:%d   E1PortIdx=%d    error!!\r\n", i, pOAM_OnuE1Link->oam_OnuE1Link[i].E1PortIdx));
			return V2R1_E1_ERROR;
		}

		if ( pOAM_OnuE1Link->oam_OnuE1Link[i].E1SlotIdx > MAX_ONU_E1_SLOT_ID )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::Port E1:%d   E1SlotIdx=%d    error!!\r\n", i, pOAM_OnuE1Link->oam_OnuE1Link[i].E1SlotIdx));
			return V2R1_E1_ERROR;
		}

		if ( (pOAM_OnuE1Link->oam_OnuE1Link[i].E1Enable != OAM_E1_ENABLE) && (pOAM_OnuE1Link->oam_OnuE1Link[i].E1Enable != OAM_E1_DISABLE) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::Port E1:%d   E1Enable=%d    error!!\r\n", i, pOAM_OnuE1Link->oam_OnuE1Link[i].E1Enable));
			return V2R1_E1_ERROR;
		}

		if ( VOS_ERROR == checkE1Mac(pOAM_OnuE1Link->oam_OnuE1Link[i].DesMac) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::Port E1:%d   DesMac   error!!\r\n", i));
			printMac(pOAM_OnuE1Link->oam_OnuE1Link[i].DesMac);
			return V2R1_E1_ERROR;
		}

		if ( VOS_ERROR == checkE1Mac(pOAM_OnuE1Link->oam_OnuE1Link[i].SrcMac) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::Port E1:%d   SrcMac   error!!\r\n", i));
			printMac(pOAM_OnuE1Link->oam_OnuE1Link[i].SrcMac);
			return V2R1_E1_ERROR;
		}

		if ( (pOAM_OnuE1Link->oam_OnuE1Link[i].VlanEable != OAM_E1_VLAN_ENABLE) && (pOAM_OnuE1Link->oam_OnuE1Link[i].VlanEable != OAM_E1_VLAN_DISABLE) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::Port E1:%d   VlanEable=%d    error!!\r\n", i, pOAM_OnuE1Link->oam_OnuE1Link[i].VlanEable));
			return V2R1_E1_ERROR;
		}

		if ( VOS_ERROR == checkVlanTag(pOAM_OnuE1Link->oam_OnuE1Link[i].VlanTag) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Link()::Port E1:%d   checkVlanTag()    error!!\r\n", i));
			return V2R1_E1_ERROR;
		}
	}

	return( V2R1_E1_OK );
}

static STATUS checkOAM_OnuE1Vlan(OAM_OnuE1Vlan *pOAM_OnuE1Vlan)
{
	unsigned char i;

	if (NULL == pOAM_OnuE1Vlan)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Vlan()::pOAM_OnuE1Vlan=NULL   error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	if (SET_ONU_E1_VLAN_REQ != pOAM_OnuE1Vlan->MsgType)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Vlan()::MsgType=0x%02x   error!!\r\n", pOAM_OnuE1Vlan->MsgType));
		return V2R1_E1_ERROR;
	}

	if ( (0 == pOAM_OnuE1Vlan->E1PortTotalCount) || (MAX_ONU_E1 < pOAM_OnuE1Vlan->E1PortTotalCount) )
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Vlan()::E1PortTotalCount=%d   error!!\r\n", pOAM_OnuE1Vlan->E1PortTotalCount));
		return V2R1_E1_ERROR;
	}

	for (i = 0; i < pOAM_OnuE1Vlan->E1PortTotalCount; i++)
	{
		if ( pOAM_OnuE1Vlan->oam_OnuE1Vlan[i].E1PortIdx > MAX_ONU_BOARD_E1 )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Vlan()::Port E1:%d   E1PortIdx=%d    error!!\r\n", i, pOAM_OnuE1Vlan->oam_OnuE1Vlan[i].E1PortIdx));
			return V2R1_E1_ERROR;
		}

		if ( pOAM_OnuE1Vlan->oam_OnuE1Vlan[i].E1SlotIdx > MAX_ONU_E1_SLOT_ID )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Vlan()::Port E1:%d   E1SlotIdx=%d    error!!\r\n", i, pOAM_OnuE1Vlan->oam_OnuE1Vlan[i].E1SlotIdx));
			return V2R1_E1_ERROR;
		}

		if ( (pOAM_OnuE1Vlan->oam_OnuE1Vlan[i].VlanEable != OAM_E1_VLAN_ENABLE) && (pOAM_OnuE1Vlan->oam_OnuE1Vlan[i].VlanEable != OAM_E1_VLAN_DISABLE) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Vlan()::Port E1:%d   VlanEable=%d    error!!\r\n", i, pOAM_OnuE1Vlan->oam_OnuE1Vlan[i].VlanEable));
			return V2R1_E1_ERROR;
		}

		if ( VOS_ERROR == checkVlanTag(pOAM_OnuE1Vlan->oam_OnuE1Vlan[i].VlanTag) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Vlan()::Port E1:%d   checkVlanTag()    error!!\r\n", i));
			return V2R1_E1_ERROR;
		}
	}

	return( V2R1_E1_OK );
}

static STATUS checkOAM_OnuE1Loop(OAM_OnuE1Loop *pOAM_OnuE1Loop)
{
	if (NULL == pOAM_OnuE1Loop)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Loop()::pOAM_OnuE1Vlan=NULL   error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	if (SET_ONU_E1_LOOP_REQ != pOAM_OnuE1Loop->MsgType)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Loop()::MsgType=0x%02x   error!!\r\n", pOAM_OnuE1Loop->MsgType));
		return V2R1_E1_ERROR;
	}

	if ( pOAM_OnuE1Loop->E1PortIdx > MAX_ONU_BOARD_E1 )
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Loop():: E1PortIdx=%d    error!!\r\n", pOAM_OnuE1Loop->E1PortIdx));
		return V2R1_E1_ERROR;
	}

	if ( pOAM_OnuE1Loop->E1SlotIdx > MAX_ONU_E1_SLOT_ID )
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Loop():: E1SlotIdx=%d    error!!\r\n", pOAM_OnuE1Loop->E1SlotIdx));
		return V2R1_E1_ERROR;
	}

	if ( E1_NO_LOOP < pOAM_OnuE1Loop->LoopControl )
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Loop()::LoopControl=%d     error!!\r\n", pOAM_OnuE1Loop->LoopControl));
		return V2R1_E1_ERROR;
	}

	return( V2R1_E1_OK );
}

static STATUS checkOAM_OnuE1Clock(OAM_OnuE1Clock *pOAM_OnuE1Clock)
{
	unsigned char i;

	if (NULL == pOAM_OnuE1Clock)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Clock()::pOAM_OnuE1Vlan=NULL   error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	if (SET_ONU_E1_CLOCK_REQ != pOAM_OnuE1Clock->MsgType)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Clock()::MsgType=0x%02x   error!!\r\n", pOAM_OnuE1Clock->MsgType));
		return V2R1_E1_ERROR;
	}

	if ( (0 == pOAM_OnuE1Clock->E1PortTotalCount) || (MAX_ONU_E1 < pOAM_OnuE1Clock->E1PortTotalCount) )
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Clock()::E1PortTotalCount=%d   error!\r\n", pOAM_OnuE1Clock->E1PortTotalCount));
		return V2R1_E1_ERROR;
	}

	for (i = 0; i < pOAM_OnuE1Clock->E1PortTotalCount; i++)
	{
		if ( pOAM_OnuE1Clock->oam_OnuE1Clock[i].E1PortIdx > MAX_ONU_BOARD_E1 )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Clock()::Port E1:%d   E1PortIdx=%d    error!!\r\n", i, pOAM_OnuE1Clock->oam_OnuE1Clock[i].E1PortIdx));
			return V2R1_E1_ERROR;
		}

		if ( pOAM_OnuE1Clock->oam_OnuE1Clock[i].E1SlotIdx > MAX_ONU_E1_SLOT_ID )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Clock()::Port E1:%d   E1SlotIdx=%d    error!!\r\n", i, pOAM_OnuE1Clock->oam_OnuE1Clock[i].E1SlotIdx));
			return V2R1_E1_ERROR;
		}

		if ( pOAM_OnuE1Clock->oam_OnuE1Clock[i].ClockControl > E1_TX_CLOCK_CRYS )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Clock()::Port E1:%d   VlanEable=%d    error!!\r\n", i, pOAM_OnuE1Clock->oam_OnuE1Clock[i].ClockControl));
			return V2R1_E1_ERROR;
		}
	}

	return( V2R1_E1_OK );
}

static STATUS checkOAM_OnuE1AlarmMask(OAM_OnuE1AlarmMask *pOAM_OnuE1AlarmMask)
{
	unsigned char i;

	if (NULL == pOAM_OnuE1AlarmMask)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1AlarmMask()::pOAM_OnuE1Vlan=NULL   error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	if (SET_ONU_E1_ALARMMASK_REQ != pOAM_OnuE1AlarmMask->MsgType)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1AlarmMask()::MsgType=0x%02x   error!!\r\n", pOAM_OnuE1AlarmMask->MsgType));
		return V2R1_E1_ERROR;
	}

	if ( (0 == pOAM_OnuE1AlarmMask->E1PortTotalCount) || (MAX_ONU_E1 < pOAM_OnuE1AlarmMask->E1PortTotalCount) )
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1AlarmMask()::E1PortTotalCount=%d   error!\r\n", pOAM_OnuE1AlarmMask->E1PortTotalCount));
		return V2R1_E1_ERROR;
	}

	for (i = 0; i < pOAM_OnuE1AlarmMask->E1PortTotalCount; i++)
	{
		if ( pOAM_OnuE1AlarmMask->oam_OnuE1AlarmMask[i].E1PortIdx > MAX_ONU_BOARD_E1 )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1AlarmMask()::Port E1:%d   E1PortIdx=%d    error!!\r\n", i, pOAM_OnuE1AlarmMask->oam_OnuE1AlarmMask[i].E1PortIdx));
			return V2R1_E1_ERROR;
		}

		if ( pOAM_OnuE1AlarmMask->oam_OnuE1AlarmMask[i].E1SlotIdx > MAX_ONU_E1_SLOT_ID )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1AlarmMask()::Port E1:%d   E1SlotIdx=%d    error!!\r\n", i, pOAM_OnuE1AlarmMask->oam_OnuE1AlarmMask[i].E1SlotIdx));
			return V2R1_E1_ERROR;
		}
	}

	return( V2R1_E1_OK );
}

static STATUS checkOAM_OnuE1Info(OAM_OnuE1Info *pOAM_OnuE1Info)
{
	unsigned char i;

	if (NULL == pOAM_OnuE1Info)
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::pOAM_OnuE1Link=NULL   error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	if ( (SET_ONU_E1_ALL_REQ != pOAM_OnuE1Info->MsgType) && (GET_ONU_E1_ALL_RSP != pOAM_OnuE1Info->MsgType) )
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::MsgType=0x%02x   error!!\r\n", pOAM_OnuE1Info->MsgType));
		return V2R1_E1_ERROR;
	}

	if ( (0 == pOAM_OnuE1Info->E1PortTotalCount) || (MAX_ONU_E1 < pOAM_OnuE1Info->E1PortTotalCount) )
	{
		E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::E1PortTotalCount=%d   error!\r\n", pOAM_OnuE1Info->E1PortTotalCount));
		return V2R1_E1_ERROR;
	}

	for (i = 0; i < pOAM_OnuE1Info->E1PortTotalCount; i++)
	{
		if ( pOAM_OnuE1Info->oam_OnuE1Info[i].E1PortIdx > MAX_ONU_BOARD_E1 )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::Port E1:%d   E1PortIdx=%d    error!!\r\n", i, pOAM_OnuE1Info->oam_OnuE1Info[i].E1PortIdx));
			return V2R1_E1_ERROR;
		}

		if ( pOAM_OnuE1Info->oam_OnuE1Info[i].E1SlotIdx > MAX_ONU_E1_SLOT_ID )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::Port E1:%d   E1SlotIdx=%d    error!!\r\n", i, pOAM_OnuE1Info->oam_OnuE1Info[i].E1SlotIdx));
			return V2R1_E1_ERROR;
		}

		if ( (pOAM_OnuE1Info->oam_OnuE1Info[i].E1Enable != OAM_E1_ENABLE) && (pOAM_OnuE1Info->oam_OnuE1Info[i].E1Enable != OAM_E1_DISABLE) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::Port E1:%d   E1Enable=%d    error!!\r\n", i, pOAM_OnuE1Info->oam_OnuE1Info[i].E1Enable));
			return V2R1_E1_ERROR;
		}
/*
		if ( VOS_ERROR == checkE1Mac(pOAM_OnuE1Info->oam_OnuE1Info[i].DesMac) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::Port E1:%d   DesMac   error!!\r\n", i));
			printMac(pOAM_OnuE1Info->oam_OnuE1Info[i].DesMac);
			return V2R1_E1_ERROR;
		}

		if ( VOS_ERROR == checkE1Mac(pOAM_OnuE1Info->oam_OnuE1Info[i].SrcMac) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::Port E1:%d   SrcMac   error!!\r\n", i));
			printMac(pOAM_OnuE1Info->oam_OnuE1Info[i].SrcMac);
			return V2R1_E1_ERROR;
		}
*/
		if ( (pOAM_OnuE1Info->oam_OnuE1Info[i].VlanEable != OAM_E1_VLAN_ENABLE) && (pOAM_OnuE1Info->oam_OnuE1Info[i].VlanEable != OAM_E1_VLAN_DISABLE) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::Port E1:%d   VlanEable=%d    error!!\r\n", i, pOAM_OnuE1Info->oam_OnuE1Info[i].VlanEable));
			return V2R1_E1_ERROR;
		}

		if ( VOS_ERROR == checkVlanTag(pOAM_OnuE1Info->oam_OnuE1Info[i].VlanTag) )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::Port E1:%d   checkVlanTag()    error!!\r\n", i));
			return V2R1_E1_ERROR;
		}

		if ( pOAM_OnuE1Info->oam_OnuE1Info[i].ClockControl > E1_TX_CLOCK_CRYS )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::Port E1:%d   VlanEable=%d    error!!\r\n", i, pOAM_OnuE1Info->oam_OnuE1Info[i].ClockControl));
			return V2R1_E1_ERROR;
		}

		if ( E1_NO_LOOP < pOAM_OnuE1Info->oam_OnuE1Info[i].LoopControl )
		{
			E1_ERROR_INFO_PRINT(("checkOAM_OnuE1Info()::LoopControl=%d     error!!\r\n", pOAM_OnuE1Info->oam_OnuE1Info[i].LoopControl));
			return V2R1_E1_ERROR;
		}
	}

	return( V2R1_E1_OK );
}

/* 检查ONU是否支持E1和ONU是否在线,然后发送OAM帧 */
static STATUS sendE1OamFrame(unsigned long onuDevIdx, char MsgType, unsigned char *pBuf, int length)
{
	ULONG PonPortIdx, OnuIdx;

	if ( (MsgType < SET_ONU_E1_LINK_REQ) || (MsgType > GET_ONU_E1_ALL_RSP) )
	{
		E1_ERROR_INFO_PRINT(("sendE1OamFrame()::MsgType=%d    error!!\r\n", MsgType));
		return V2R1_E1_ERROR;
	}

	if (NULL == pBuf)
	{
		E1_ERROR_INFO_PRINT(("sendE1OamFrame()::pBuf=NULL    error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	if (VOS_OK != onuDevIdxIsSupportE1(onuDevIdx))
	{
		E1_ERROR_INFO_PRINT(("sendE1OamFrame()::onuDevIdxIsSupportE1()    error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	if ( !onuIsOnLine(onuDevIdx) )
	{
		E1_ERROR_INFO_PRINT(("sendE1OamFrame()  onu:%d is not onLine! \r\n", onuDevIdx));
		return V2R1_E1_ERROR;
	}

	if ( -1 == parseOnuIndexFromDevIdx(onuDevIdx, &PonPortIdx, &OnuIdx) )
	{
		E1_ERROR_INFO_PRINT(("sendE1OamFrame()::parseOnuIndexFromDevIdx()  error! \r\n"));
		return V2R1_E1_ERROR;
	}
/*
	if( GetOnuOperStatus(PonPortIdx, OnuIdx) != ONU_OPER_STATUS_UP )
	{
		E1_ERROR_INFO_PRINT(("sendE1OamFrame()::GetOnuOperStatus()   error!!\r\n"));
		return V2R1_E1_ONU_OFFLINE;
	}
*/
	printTxOamMsg((char *)pBuf, (ULONG)length);

	return EQU_SendMsgToOnu(PonPortIdx, OnuIdx, MsgType, pBuf, length);
}

/************************************************************************/
/*                            E1 OAM API                                */
/************************************************************************/

STATUS SetOnuE1Link(unsigned long onuDevIdx, OAM_OnuE1Link *pOAM_OnuE1Link)
{
	int GetSemId;
	OAM_OnuE1Link *pOAM_OnuE1Link_Recv;
	int PduLength;

	if ( V2R1_E1_OK != checkOAM_OnuE1Link(pOAM_OnuE1Link) )
	{
		E1_ERROR_INFO_PRINT(("SetOnuE1Link()::checkOAM_OnuE1Link()  error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	PduLength = sizeof(OAM_OnuE1Link) - sizeof(OAM_OnuE1Link_t) * ( MAX_ONU_E1 - pOAM_OnuE1Link->E1PortTotalCount );
	GetSemId = sendE1OamFrame(onuDevIdx, SET_ONU_E1_LINK_REQ, (unsigned char *)pOAM_OnuE1Link, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_E1_ERROR )
	{
		E1_ERROR_INFO_PRINT(("\r\nSetOnuE1Link() but no RSP from onu:%d\r\n", onuDevIdx));
		return( V2R1_E1_ERROR );
	}
	else 
	{
		pOAM_OnuE1Link_Recv = (OAM_OnuE1Link *)RecvMsgFromOnu;

		printRxOamMsg( (char *)RecvMsgFromOnu, sizeof(OAM_OnuE1Link) );

		if( pOAM_OnuE1Link_Recv->MsgType == SET_ONU_E1_LINK_RSP )
		{
			if( pOAM_OnuE1Link_Recv->Result ==  V2R1_E1_ONU_COMM_SUCCESS )
			{
				return( V2R1_E1_OK );
			}
			else 
			{
				return( V2R1_E1_ERROR );
			}
		}
		else 
		{
			return( V2R1_E1_ERROR );
		}
	}
}

STATUS SetOnuE1Vlan(unsigned long onuDevIdx, OAM_OnuE1Vlan *pOAM_OnuE1Vlan)
{
	int GetSemId;
	OAM_OnuE1Vlan *pOAM_OnuE1Vlan_Recv;
	int PduLength;

	if ( V2R1_E1_OK != checkOAM_OnuE1Vlan(pOAM_OnuE1Vlan) )
	{
		E1_ERROR_INFO_PRINT(("SetOnuE1Vlan()::checkOAM_OnuE1Vlan()  error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	PduLength = sizeof(OAM_OnuE1Vlan) - sizeof(OAM_OnuE1Vlan_t) * ( MAX_ONU_E1 - pOAM_OnuE1Vlan->E1PortTotalCount );
	GetSemId = sendE1OamFrame(onuDevIdx, SET_ONU_E1_VLAN_REQ, (unsigned char *)pOAM_OnuE1Vlan, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_E1_ERROR )
	{
		E1_ERROR_INFO_PRINT(("\r\nSetOnuE1Vlan() but no RSP from onu:%d\r\n", onuDevIdx));
		return( V2R1_E1_ERROR );
	}
	else 
	{
		pOAM_OnuE1Vlan_Recv = (OAM_OnuE1Vlan *)RecvMsgFromOnu;

		printRxOamMsg( (char *)RecvMsgFromOnu, sizeof(OAM_OnuE1Vlan) );

		if( pOAM_OnuE1Vlan_Recv->MsgType == SET_ONU_E1_VLAN_RSP )
		{
			if( pOAM_OnuE1Vlan_Recv->Result ==  V2R1_E1_ONU_COMM_SUCCESS )
			{
				return( V2R1_E1_OK );
			}
			else 
			{
				return( V2R1_E1_ERROR );
			}
		}
		else 
		{
			return( V2R1_E1_ERROR );
		}
	}
}

STATUS SetOnuE1Loop(unsigned long onuDevIdx, OAM_OnuE1Loop *pOAM_OnuE1Loop)
{
	int GetSemId;
	OAM_OnuE1Loop *pOAM_OnuE1Loop_Recv;
	int PduLength;

	if ( V2R1_E1_OK != checkOAM_OnuE1Loop(pOAM_OnuE1Loop) )
	{
		E1_ERROR_INFO_PRINT(("SetOnuE1Loop()::checkOAM_OnuE1Loop()  error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	PduLength = sizeof(OAM_OnuE1Loop);
	GetSemId = sendE1OamFrame(onuDevIdx, SET_ONU_E1_LOOP_REQ, (unsigned char *)pOAM_OnuE1Loop, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_E1_ERROR )
	{
		E1_ERROR_INFO_PRINT(("\r\nSetOnuE1Loop() but no RSP from onu:%d\r\n", onuDevIdx));
		return( V2R1_E1_ERROR );
	}
	else 
	{
		pOAM_OnuE1Loop_Recv = (OAM_OnuE1Loop *)RecvMsgFromOnu;

		printRxOamMsg( (char *)RecvMsgFromOnu, sizeof(OAM_OnuE1Loop) );

		if( pOAM_OnuE1Loop_Recv->MsgType == SET_ONU_E1_LOOP_RSP )
		{
			if( pOAM_OnuE1Loop_Recv->Result ==  V2R1_E1_ONU_COMM_SUCCESS )
			{
				return( V2R1_E1_OK );
			}
			else 
			{
				return( V2R1_E1_ERROR );
			}
		}
		else 
		{
			return( V2R1_E1_ERROR );
		}
	}
}

STATUS SetOnuE1Clock(unsigned long onuDevIdx, OAM_OnuE1Clock *pOAM_OnuE1Clock)
{
	int GetSemId;
	OAM_OnuE1Clock *pOAM_OnuE1Clock_Recv;
	int PduLength;

	if ( V2R1_E1_OK != checkOAM_OnuE1Clock(pOAM_OnuE1Clock) )
	{
		E1_ERROR_INFO_PRINT(("SetOnuE1Clock()::checkOAM_OnuE1Clock()  error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	PduLength = sizeof(OAM_OnuE1Clock) - sizeof(OAM_OnuE1Clock_t) * ( MAX_ONU_E1 - pOAM_OnuE1Clock->E1PortTotalCount );
	GetSemId = sendE1OamFrame(onuDevIdx, SET_ONU_E1_CLOCK_REQ, (unsigned char *)pOAM_OnuE1Clock, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_E1_ERROR )
	{
		E1_ERROR_INFO_PRINT(("\r\n SetOnuE1Clock() but no RSP from onu:%d\r\n", onuDevIdx));
		return( V2R1_E1_ERROR );
	}
	else 
	{
		pOAM_OnuE1Clock_Recv = (OAM_OnuE1Clock *)RecvMsgFromOnu;

		printRxOamMsg( (char *)RecvMsgFromOnu, sizeof(OAM_OnuE1Clock) );

		if( pOAM_OnuE1Clock_Recv->MsgType == SET_ONU_E1_CLOCK_RSP )
		{
			if( pOAM_OnuE1Clock_Recv->Result ==  V2R1_E1_ONU_COMM_SUCCESS )
			{
				return( V2R1_E1_OK );
			}
			else 
			{
				return( V2R1_E1_ERROR );
			}
		}
		else 
		{
			return( V2R1_E1_ERROR );
		}
	}
}

STATUS SetOnuE1AlarmMask(unsigned long onuDevIdx, OAM_OnuE1AlarmMask *pOAM_OnuE1AlarmMask)
{
	int GetSemId;
	OAM_OnuE1AlarmMask *pOAM_OnuE1AlarmMask_Recv;
	int PduLength;

	if ( V2R1_E1_OK != checkOAM_OnuE1AlarmMask(pOAM_OnuE1AlarmMask) )
	{
		E1_ERROR_INFO_PRINT(("SetOnuE1AlarmMask()::checkOAM_OnuE1AlarmMask()  error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	PduLength = sizeof(OAM_OnuE1AlarmMask) - sizeof(OAM_OnuE1AlarmMask_t) * ( MAX_ONU_E1 - pOAM_OnuE1AlarmMask->E1PortTotalCount );
	GetSemId = sendE1OamFrame(onuDevIdx, SET_ONU_E1_ALARMMASK_REQ, (unsigned char *)pOAM_OnuE1AlarmMask, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_E1_ERROR )
	{
		E1_ERROR_INFO_PRINT(("\r\n SetOnuE1AlarmMask() but no RSP from onu:%d\r\n", onuDevIdx));
		return( V2R1_E1_ERROR );
	}
	else 
	{
		pOAM_OnuE1AlarmMask_Recv = (OAM_OnuE1AlarmMask *)RecvMsgFromOnu;

		printRxOamMsg( (char *)RecvMsgFromOnu, sizeof(OAM_OnuE1AlarmMask) );

		if( pOAM_OnuE1AlarmMask_Recv->MsgType == SET_ONU_E1_ALARMMASK_RSP )
		{
			if( pOAM_OnuE1AlarmMask_Recv->Result ==  V2R1_E1_ONU_COMM_SUCCESS )
			{
				return( V2R1_E1_OK );
			}
			else 
			{
				return( V2R1_E1_ERROR );
			}
		}
		else 
		{
			return( V2R1_E1_ERROR );
		}
	}
}

STATUS SetOnuE1All(unsigned long onuDevIdx, OAM_OnuE1Info *pOAM_OnuE1Info)
{
	int GetSemId;
	OAM_OnuE1Info *pOAM_OnuE1Info_Recv;
	int PduLength;

	if ( V2R1_E1_OK != checkOAM_OnuE1Info(pOAM_OnuE1Info) )
	{
		E1_ERROR_INFO_PRINT(("SetOnuE1All()::checkOAM_OnuE1Info()  error!!\r\n"));
		return V2R1_E1_ERROR;
	}

	PduLength = sizeof(OAM_OnuE1Info) - sizeof(OAM_OnuE1Info_t) * ( MAX_ONU_E1 - pOAM_OnuE1Info->E1PortTotalCount );
	GetSemId = sendE1OamFrame(onuDevIdx, SET_ONU_E1_ALL_REQ, (unsigned char *)pOAM_OnuE1Info, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_E1_ERROR )
	{
		E1_ERROR_INFO_PRINT(("\r\n SetOnuE1All() but no RSP from onu:%d\r\n", onuDevIdx));
		return( V2R1_E1_ERROR );
	}
	else 
	{
		pOAM_OnuE1Info_Recv = (OAM_OnuE1Info *)RecvMsgFromOnu;

		printRxOamMsg( (char *)RecvMsgFromOnu, sizeof(OAM_OnuE1Info) );

		if( pOAM_OnuE1Info_Recv->MsgType == SET_ONU_E1_ALL_RSP )
		{
			if( pOAM_OnuE1Info_Recv->Result ==  V2R1_E1_ONU_COMM_SUCCESS )
			{
				return( V2R1_E1_OK );
			}
			else 
			{
				return( V2R1_E1_ERROR );
			}
		}
		else 
		{
			return( V2R1_E1_ERROR );
		}
	}
}

STATUS GetOnuE1All(unsigned long onuDevIdx, OAM_OnuE1Info *pOAM_OnuE1Info)
{
	int GetSemId;
	OAM_OnuE1Info oam_OnuE1Info_Send;
	int PduLength;

       E1_ERROR_INFO_PRINT(("GetOnuE1All()::onuDevIdx=%ld\r\n", onuDevIdx));
       memset((UCHAR *)&oam_OnuE1Info_Send, 0, sizeof(OAM_OnuE1Info));
	oam_OnuE1Info_Send.MsgType = GET_ONU_E1_ALL_REQ;

	PduLength = 2;
	GetSemId = sendE1OamFrame(onuDevIdx, GET_ONU_E1_ALL_REQ, (unsigned char *)&oam_OnuE1Info_Send, PduLength);

	/* 等待并处理返回的响应消息*/
	if( GetSemId == V2R1_E1_ERROR )
	{
		E1_ERROR_INFO_PRINT(("\r\n GetOnuE1All() but no RSP from onu:%d\r\n", onuDevIdx));
		return( V2R1_E1_ERROR );
	}
	else 
	{
		memcpy((char *)pOAM_OnuE1Info, (char *)RecvMsgFromOnu, sizeof(OAM_OnuE1Info));

		printRxOamMsg( (char *)RecvMsgFromOnu, sizeof(OAM_OnuE1Info) );

		if( pOAM_OnuE1Info->MsgType == GET_ONU_E1_ALL_RSP )
		{
			if( pOAM_OnuE1Info->Result ==  V2R1_E1_ONU_COMM_SUCCESS )
			{
				if ( V2R1_E1_OK != checkOAM_OnuE1Info(pOAM_OnuE1Info) )
				{
					E1_ERROR_INFO_PRINT(("GetOnuE1All()::checkOAM_OnuE1Info()  error!!\r\n"));
					return V2R1_E1_ERROR;
				}
				return( V2R1_E1_OK );
			}
			else 
			{
			       E1_ERROR_INFO_PRINT(("GetOnuE1All()::Result=0x%02x   error!!\r\n", pOAM_OnuE1Info->Result));
				return( V2R1_E1_ERROR );
			}
		}
		else 
		{
		       E1_ERROR_INFO_PRINT(("GetOnuE1All()::MsgType=0x%02x   error!!\r\n", pOAM_OnuE1Info->MsgType));
			return( V2R1_E1_ERROR );
		}
	}
}

/* 
    此函数用于将TDM-E1板上保存的所有E1 连接
    及端口配置信息一同发送到ONU
*/
STATUS RestoreOnuE1LinkAll(unsigned long onuDevIdx)
{
	OAM_OnuE1Info pOAM_OnuE1Info;

	if((GetOamOnuE1Info(onuDevIdx, &pOAM_OnuE1Info) == VOS_OK) && ( pOAM_OnuE1Info.E1PortTotalCount != 0)
		&&( onuIsOnLine(onuDevIdx) == TRUE))
		SetOnuE1All(onuDevIdx,&pOAM_OnuE1Info);
		
	return(V2R1_E1_OK);
}

#endif
