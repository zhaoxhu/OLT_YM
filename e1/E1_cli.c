
#include "syscfg.h"

#include "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include  "V2R1_product.h"

#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
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
#include "mn_oam.h"

#include "ifm/ifm_debug.h"

#include "interface/interface_task.h"
#include "sys/console/sys_console.h"
#include "sys/main/sys_main.h"
#include "cpi/ctss/ctss_ifm.h"
#include "manage/snmp/sn_tc.h"

#include  "GwttOam/OAM_gw.h"

#include "../superset/platform/sys/main/Sys_main.h" 
#include "../superset/cpi/typesdb/Typesdb_module.h"

#include "Tdm_comm.h"
#include "Tdm_apis.h"
#include "tdm/mn_tdm.h"
#include "gwEponSys.h"
#include "onu/ExtBoardType.h"

#include "e1_apis.h"
#include "e1_oam.h"
#include "E1_MIB.h"
#include "E1DataSave.h"



#define E1_NODE ATM_SUBIF_NODE

extern ULONG IFM_PON_CREATE_INDEX( ULONG ulSlot, ULONG ulPort , ULONG ulOnuId, ULONG ulOnuFeId);
extern LONG PON_ParseSlotPortOnu( CHAR * szName, ULONG * pulSlot, ULONG * pulPort , ULONG * pulOnuid);

extern int SetTdmE1LinkVlanEnable(unsigned char TdmSlot,unsigned char fpgaIdx, unsigned char Priority, unsigned short int E1LinkVid);
extern int SetTdmE1LinkVlanDisable(unsigned char TdmSlot,unsigned char fpgaIdx);


/* begin: 参数解析和检查 */
/*#define ERROR -1*/
#define OK     0

#define TDM_E1_MAX_SLOT_INDEX   16
#define TDM_E1_MAX_PORT_INDEX   24
#define TDM_E1_MAX_REPEAT_E1_PORT  100

typedef struct
{
	unsigned long slotIdx;
	unsigned long portIdx[TDM_E1_MAX_PORT_INDEX];
}__attribute__((packed)) E1SlotPort;


int parseSeriesE1Port(unsigned long *portIdx, unsigned long min, unsigned long max)
{
	unsigned char i, flag/* 0无重复，1有重复 */;

	if (NULL == portIdx)
	{
		/*printf("parseSeriesE1Port()::portIdx=NULL     error! \r\n");*/
		return VOS_ERROR;
	}

	if (min >=  max)
	{
		/*printf("parseSeriesE1Port():: min=%ld, max=%ld     error! \r\n", min, max);*/
		return VOS_ERROR;
	}

	for (; min <= max; min++)
	{
		flag = 0;

		for (i = 0; portIdx[i] != 0xFFFFFFFF; i++)
		{
			if (min == portIdx[i])
			{
				flag = 1;
				break;
			}
		}

		if (1 == flag)/*有重复*/
		{
			continue;
		} 
		else/*无重复*/
		{
			portIdx[i] = min;
		}
	}

	return VOS_OK;
}

/* 不包含E1重复端口号的处理 */
int getE1Port(unsigned long *portIdx, char *str)
{
	unsigned long strLen, i;
	char num[5] = {0}, numOffset = 0, portIdxOffset = 0;

	if (NULL == str || NULL == portIdx)
	{
		/*printf("getE1Port()::str=NULL or portIdx=NULL\r\n");*/
		return VOS_ERROR;
	}

	strLen = (unsigned long)strlen(str);

	for (i = 0; i < strLen; i++)
	{
		if (numOffset > 2)
		{
			/*printf("getE1Port():: num:%ld   too large\r\n", atol(num));*/
			return VOS_ERROR;
		}

		if ( (str[i] >= '0') && (str[i] <= '9') )
		{
			strncpy( &num[(UCHAR)numOffset++], (CHAR *)&str[i], 1);
		} 
		else if (str[i] == ',' || str[i] == '-')
		{
			if (0 == i)
			{
				continue;
			} 
			else
			{
				if (portIdxOffset >= TDM_E1_MAX_REPEAT_E1_PORT)
				{
					/*printf("getE1Port()::portIdxOffset=%d    error!\r\n", portIdxOffset);*/
					return VOS_ERROR;
				}

				portIdx[(UCHAR)portIdxOffset++] = atol(num);
				numOffset = 0;
				memset(num, 0, sizeof(num));
				continue;
			}
		}
		else
		{
			return VOS_ERROR;
		}
	}

	if (strlen(num) != 0)
	{
		portIdx[(UCHAR)portIdxOffset] = atol(num);
	}

	return VOS_OK;
}

int parseE1PortList(unsigned long *portIdx, char *str)
{
	unsigned long strLen, i, j;
	unsigned long allE1Port[TDM_E1_MAX_REPEAT_E1_PORT]/*含有重复的e1 port*/, allE1PortOffset = 0;
	unsigned long portIdxOffset = 0;
	char flag = 0/* 0无重复，1有重复 */;

	if (NULL == str || NULL == portIdx)
	{
		/*printf("parseE1PortList()::str=NULL or portIdx=NULL\r\n");*/
		return VOS_ERROR;
	}

	memset((char *)allE1Port, 0xFF, sizeof(allE1Port));

	if (VOS_OK != getE1Port(allE1Port, str))
	{
		/*printf("parseE1PortList()::getE1Port()    error!\r\n");*/
		return VOS_ERROR;
	}

	strLen = (unsigned long)strlen(str);

	for (i = 0; i < strLen; )/* 总循环 */
	{
		if ('-' == str[i])
		{
			if (i == 0)
			{
				/*printf("parseE1PortList():: str[0]='-'    error!\r\n");*/
				return VOS_ERROR;
			}

			if (allE1PortOffset == 0)
			{
				/*printf("parseE1PortList():: allE1PortOffset=0    error!\r\n");*/
				return VOS_ERROR;
			}

			if ( (str[i-1] < '0') || (str[i-1] > '9') || (str[i+1] < '0') || (str[i+1] > '9') )
			{
				/*printf("parseE1PortList():: str[%d]=0x%02x   str[%d]=0x%02x   error!\r\n", i-1, str[i-1], i+1, str[i+1]);*/
				return VOS_ERROR;
			}

			if (VOS_OK != parseSeriesE1Port(portIdx, allE1Port[allE1PortOffset-1], allE1Port[allE1PortOffset]))
			{
				/*printf("parseE1PortList()::parseSeriesE1Port()    error!\r\n");*/
				return VOS_ERROR;
			}

			/*重新定位portIdx偏移portIdxOffset*/
			for (j = 0; j < (TDM_E1_MAX_PORT_INDEX + 1); j++)
			{
				if (portIdx[j] == 0xFFFFFFFF)
				{
					portIdxOffset = j;
					break;
				}
			}

			i++;
		} 
		else if (',' == str[i])
		{
			i++;
		}
		else if ( (str[i] >= '0') && (str[i] <= '9') )
		{
			if (allE1Port[allE1PortOffset] >= 10)/* 两位数 */
			{
				i += 2;
			} 
			else/* 一位数 */
			{
				i++;
			}
			/*重新定位portIdx偏移portIdxOffset*/
			for (j = 0; j < (TDM_E1_MAX_PORT_INDEX + 1); j++)
			{
				if (portIdx[j] == 0xFFFFFFFF)
				{
					portIdxOffset = j;
					break;
				}
			}

			/* 检查是否有重复端口号 */
			for (j = 0; j < portIdxOffset; j++)
			{
				if (portIdx[j] == allE1Port[allE1PortOffset])
				{
					flag = 1;
					break;
				}
			}

			if (1 == flag)/* 有重复 */
			{
				flag = 0;
			} 
			else/* 无重复 */
			{
				portIdx[portIdxOffset] = allE1Port[allE1PortOffset];
				portIdxOffset++;
			}

			allE1PortOffset++;
		}
		else
		{
			/*printf("parseE1PortList():: char=0x%02x    error!\r\n", str[i]);*/
			return VOS_ERROR;
		}
	}

	return VOS_OK;
}

/* pE1SlotPort索引从0开始 */
int parseE1SlotPortList(char *str, E1SlotPort *pE1SlotPort)
{
	unsigned char i, j;
	unsigned long strLen, slotIdx;
	char num[3], portlist[64];
	char flag/* 0无重复，1有重复 */;

	if (NULL == str || NULL == pE1SlotPort)
	{
		/*printf("parseE1SlotPortList()::str=NULL or pE1SlotPort=NULL\r\n");*/
		return VOS_ERROR;
	}

	strLen = (unsigned long)strlen(str);

	for (i = 0; i < strLen; i++)
	{
		if (str[i] == '/')
		{
			memset(num, 0, sizeof(num));
			num[0] = str[i - 1];
			slotIdx = atol(num);

			for (j = i + 1; j < strLen; j++)
			{
				if (str[j+1] == 0)
				{
					memset(portlist, 0, sizeof(portlist));
					strncpy(portlist, &str[i + 1], j - i);
					break;
				}
				else if (str[j+1] == '/')
				{
					memset(portlist, 0, sizeof(portlist));
					strncpy(portlist, &str[i + 1], j - i - 2/* 减一个数字和逗号 */);
					break;
				}
			}

			flag = 0;

			for (j = 0; j < TDM_E1_MAX_SLOT_INDEX && pE1SlotPort[j].slotIdx != 0xFFFFFFFF; j++)
			{
				if (pE1SlotPort[j].slotIdx == slotIdx)
				{
					flag = 1;

					if (VOS_OK != parseE1PortList(pE1SlotPort[j].portIdx, portlist))
					{
						/*printf("parseE1SlotPortList()::parseE1PortList()    error!\r\n");*/
						return VOS_ERROR;
					}
					break;
				}
			}

			if (flag == 0)/* 无重复 */
			{
				pE1SlotPort[j].slotIdx = slotIdx;

				if (VOS_OK != parseE1PortList(pE1SlotPort[j].portIdx, portlist))
				{
					/*printf("parseE1SlotPortList()::parseE1PortList()    error!\r\n");*/
					return VOS_ERROR;
				}
			}
		}
	}

	return VOS_OK;
}

/* 检查输入参数的合法性 */
enum match_type checkE1SlotPortList(char *str)
{
	unsigned char i, j;
	unsigned long strLen;
	char portlist[64];
	unsigned long allE1Port[TDM_E1_MAX_REPEAT_E1_PORT]/*含有重复的e1 port*/;

	if (NULL == str)
	{
		/*printf("checkE1SlotPortList()::str=NULL    error!\r\n");*/
		return no_match;
	}

	/* 前3字节检测 */
	if ( (str[0] < '0') || (str[0] > '9') )
	{
		/*printf("checkE1SlotPortList()::str[0]=%c    error!\r\n", str[0]);*/
		return no_match;
	}

	if ( str[1] != '/' )
	{
		/*printf("checkE1SlotPortList()::str[1]=%c    error!\r\n", str[1]);*/
		return no_match;
	}

	if ( (str[2] < '0') || (str[2] > '9') )
	{
		/*printf("checkE1SlotPortList()::str[2]=%c    error!\r\n", str[2]);*/
		return no_match;
	}

	strLen = (unsigned long)strlen(str);

	if (strLen > 32)
	{
		/*printf("checkE1SlotPortList()::strLen=%ld  too large  error!\r\n", strLen);*/
		return no_match;
	}

	/* 检查字符串 */
	for (i = 0; i < strLen; i++)
	{
		if ( ( (str[i] < '0') || (str[i] > '9') ) && (str[i] != ',') && (str[i] != '/') && (str[i] != '-') )
		{
			/*printf("checkE1SlotPortList()::str[%d]=%c    error!\r\n", i, str[i]);*/
			return no_match;
		}
	}

	/* 检查','，'-'*/
	for (i = 3; i < strLen; i++)
	{
		if ( (str[i] == ',') || (str[i] == '-') )
		{
			if ( (str[i - 1] < '0') || (str[i - 1] > '9') )
			{
				/*printf("checkE1SlotPortList()::str[%d]=%c    error!\r\n", i - 1, str[i - 1]);*/
				return no_match;
			}

			if ( (str[i + 1] < '0') || (str[i + 1] > '9') )
			{
				/*printf("checkE1SlotPortList()::str[%d]=%c    error!\r\n", i + 1, str[i + 1]);*/
				return no_match;
			}
		}
	}

	/* 检查槽位号, 从第2个slot index开始查 */
	for (i = 4; i < strLen; i++)
	{
		if (str[i] == '/')
		{
			if ( (str[i - 1] < '0') || (str[i - 1] > '9') )
			{
				/*printf("checkE1SlotPortList()::str[%d]=%c    error!\r\n", i - 1, str[i - 1]);*/
				return no_match;
			}

			if (str[i - 2] != ',')
			{
				/*printf("checkE1SlotPortList()::str[%d]=%c    error!\r\n", i - 2, str[i - 2]);*/
				return no_match;
			}

			if ( (str[i + 1] < '0') || (str[i + 1] > '9') )
			{
				/*printf("checkE1SlotPortList()::str[%d]=%c    error!\r\n", i + 1, str[i + 1]);*/
				return no_match;
			}
		}
	}

	/* 检查E1端口号 */
	for (i = 0; i < strLen; i++)
	{
		if (str[i] == '/')
		{
			for (j = i + 1; j < strLen; j++)
			{
				if (str[j+1] == 0)
				{
					memset(portlist, 0, sizeof(portlist));
					strncpy(portlist, &str[i + 1], j - i);
					break;
				}
				else if (str[j+1] == '/')
				{
					memset(portlist, 0, sizeof(portlist));
					strncpy(portlist, &str[i + 1], j - i - 2/* 减一个数字和逗号 */);
					break;
				}
			}

			memset((char *)allE1Port, 0xFF, sizeof(allE1Port));

			if (VOS_OK != getE1Port(allE1Port, portlist))
			{
				/*printf("checkE1SlotPortList()::getE1Port()    error!\r\n");*/
				return no_match;
			}

			for (j = 0; j < TDM_E1_MAX_REPEAT_E1_PORT && allE1Port[j] != 0xFFFFFFFF; j++)
			{
				if ( (allE1Port[j] < 1) || ( allE1Port[j] > TDM_E1_MAX_PORT_INDEX) )
				{
					/*printf("checkE1SlotPortList()::num=%ld    error!\r\n", allE1Port[j]);*/
					return no_match;
				}
			}
		}
	}

	return exact_match;
}

/*  chenfj 
     这个函数对CLI 参数<slot/port/onuid> 的检查不到位，不建议采用
     建议沿用之前对这个类型参数的解析，处理。如下:
     1 先将参数字符串解析成三个整形数:PON_ParseSlotPortOnu()
     2 再检查slot, port, onuid 的范围: PonCardSlotPortCheckByVty()
     */
#if 0
enum match_type checkOnuIndex(char *str)
{
	unsigned long strLen, i;

	strLen = (unsigned long)strlen(str);

	for (i = 0; i < strLen; i++)
	{
		if ('/' == str[i])
		{
			break;
		}
	}

	/* 检查PON口索引是否在1-4范围 */
	if ( (str[i + 1] < '1') || (str[i + 1] > '4') )
	{
		return no_match;
	}

	return exact_match;
}
#endif
/* 判断两个<slot/e1portlist>的E1端口是否数目相等，重复的算同一个 */

/* end: 参数解析和检查 */

CMD_NOTIFY_REFISTER_S stCMD_E1_Port_List_Check =
{
	"<slot/e1portlist>",
	checkE1SlotPortList,
	0
};

CMD_NOTIFY_REFISTER_S stCMD_Fpga_Port_List_Check =
{
	"<slot/fpgaportlist>",
	checkE1SlotPortList,
	0
};

/*CMD_NOTIFY_REFISTER_S stCMD_Onu_Index_Check =
{
	"<slot/port/onuid>",
	checkOnuIndex,
	0
};*/		/* removed by xieshl 20090624, 重复定义，见onu_cli.c  */

/* 返回是否两端E1链接个数相等 */
static BOOL checkE1LinkPortList(E1SlotPort *oltE1PortList, E1SlotPort *onuE1PortList)
{
	ULONG i, j, oltE1PortCount = 0, onuE1PortCount = 0;

	if ( (NULL == oltE1PortList) || (NULL == onuE1PortList) )
	{
		E1_ERROR_INFO_PRINT(("checkE1LinkPortList()::oltE1PortList=NULL or onuE1PortList=NULL    error! \r\n"));
		return FALSE;
	}

	for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && oltE1PortList[i].slotIdx != 0xFFFFFFFF; i++)
	{
		for (j = 0; j < TDM_E1_MAX_PORT_INDEX && oltE1PortList[i].portIdx[j] != 0xFFFFFFFF; j++)
		{
			oltE1PortCount++;
		}
	}

	for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && onuE1PortList[i].slotIdx != 0xFFFFFFFF; i++)
	{
		for (j = 0; j < TDM_E1_MAX_PORT_INDEX && onuE1PortList[i].portIdx[j] != 0xFFFFFFFF; j++)
		{
			onuE1PortCount++;
		}
	}

	if (oltE1PortCount == onuE1PortCount)
	{
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}

static char* alarm_e1_mask_to_str( UCHAR mask, char *str )
{
	if( str == NULL )
	{
		return str;
	}

	if( !(mask & E1_ALM_LOS) && !(mask & E1_ALM_AIS) )
	{
		VOS_StrCpy( str, "NO");
		return str;
	}

	str[0] = 0;

	if( mask & E1_ALM_LOS )
	{
		VOS_StrCat( str,"LOS|");
	}
	if( mask & E1_ALM_AIS)
	{
		VOS_StrCat(str, "AIS|");
	}

	if( VOS_StrLen(str) > 3 )
	{
		str[VOS_StrLen(str) - 1] = 0;
	}

	return str;
}

static UCHAR alarm_mask_e1_parase( int argc, char** argv)
{
	UCHAR mask = 0;

	if( VOS_MemCmp(argv[argc - 1], "all", 3) == 0 )
	{
		mask |= E1_ALM_LOS;
		mask |= E1_ALM_AIS;
	}
	else if( VOS_StriCmp(argv[argc - 1], "los") == 0 )
	{
		mask |= E1_ALM_LOS;
	}
	else if( VOS_StriCmp(argv[argc - 1], "ais") == 0 )
	{
		mask |= E1_ALM_AIS;
	}
	else
	{
		E1_ERROR_INFO_PRINT(("alarm_mask_e1_parase()::argv[%d]=%s  error! \r\n", argc - 1, argv[argc - 1]));
		return 0xFF;
	}

	return mask;
}
#if INTO_E1_NODE
#endif
DEFUN  (
		into_epon_e1_node,
		into_epon_e1_node_cmd,
		"tdm e1",
		"Select tdm-e1 board to config\n"
		"enter tdm-e1 board cli node\n"
		)
{
	ULONG  	e1Slot = 0;
	CHAR    prompt[64] = { 0 };
	ULONG   ulIFIndex = 0;
	CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };

	VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
	VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%s", "tdm", "-e1" );

	e1Slot = get_gfa_e1_slotno();
	if (0 == e1Slot)
	{
		vty_out(vty," %% there is no TDM-E1 board.\r\n");
		return( CMD_WARNING );
	}
#if 0
	/* 1 板在位检查*/
	if( __SYS_MODULE_TYPE__(e1Slot) == MODULE_TYPE_NULL )
	{
		vty_out(vty," %% slot %d is not inserted\r\n", e1Slot);
		return( CMD_WARNING );
	}
	/* 2  板类型检查*/
	if(SlotCardIsTdmE1Board(e1Slot) != ROK )
	{
		vty_out(vty," %% slot %d is not tdm-e1 card\r\n", e1Slot);
		return( CMD_WARNING );
	}
	/* tdm fpga is inserted  */
	for (i = 0; i < TDM_FPGA_MAX; i++)
	{
		if(getTdmChipInserted((unsigned char)(e1Slot-1), i) != TDM_FPGA_EXIST)
		{
			vty_out(vty,"  %% %s%d/%d is not inserted\r\n", GetGFA6xxxTdmNameString(),e1Slot, i+1);
			return(CMD_WARNING);
		}
	}
#endif
	ulIFIndex = IFM_PON_CREATE_INDEX( e1Slot, 0/*TDM_Port*/, 0, 0);
	if ( ulIFIndex == 0 )
	{
		vty_out( vty, "  %% Can not find interface %s\r\n", ifName );
		return CMD_WARNING;
	}

	vty->node = E1_NODE;
	vty->index = ( VOID * ) ulIFIndex;

	VOS_StrCpy( prompt, "%s(epon-" );
	VOS_StrCat( prompt, ifName );
	VOS_StrCat( prompt, ")#" );
	vty_set_prompt( vty, prompt );
	
	return( CMD_SUCCESS );
}

/* 取得告警字符串 */
static STATUS getAlarmStr(USHORT alarm, char *str)
{
	if (NULL == str)
	{
		return VOS_ERROR;
	}

	/* 屏蔽OOS*/
	alarm &= ~0x0100;

	switch (alarm)
	{
	case 0x0080:
		VOS_StrCpy(str, "LOS          ");
		break;
	case 0x0020:
		VOS_StrCat(str, "AIS           ");
		break;
	case 0x00A0:
		VOS_StrCpy(str, "LOS|AIS      ");
		break;
	case 0:
		VOS_StrCpy(str, "NO           ");
		break;
	default:
		VOS_StrCpy(str, "ERROR        ");
		E1_ERROR_INFO_PRINT(("getOltLoopBackStr()::alarm=0x%04x    error!\r\n", alarm));
		return VOS_ERROR;
	}

	return VOS_OK;
}

/* 取得OLT环回字符串 */
static STATUS getOltLoopBackStr(UCHAR loop, char *str)
{
	if (NULL == str)
	{
		return VOS_ERROR;
	}

	switch (loop & (~TDM_E1_NO_LOOP))
	{
	case TDM_E1_CIRCUIT_LOOP:
		VOS_StrCpy(str, "CIRCUIT");
		break;
	case TDM_E1_SYSTEM_LOOP:
		VOS_StrCpy(str, "SYSTEM");
		break;
	case TDM_E1_ALL_LOOP:
		VOS_StrCpy(str, "ALL");
		break;
	case 0:
		VOS_StrCpy(str, "NO");
		break;
	default:
		E1_ERROR_INFO_PRINT(("getOltLoopBackStr()::loop=0x%02x    error!\r\n", loop));
		return VOS_ERROR;
	}

	return VOS_OK;
}
/* 取得ONU环回字符串 */
static STATUS getOnuLoopBackStr(UCHAR loop, char *str)
{
	if (NULL == str)
	{
		return VOS_ERROR;
	}

	switch (loop & (~ONU_E1_NO_LOOP))
	{
	case ONU_E1_CIRCUIT_LOOP:
		VOS_StrCpy(str, "CIRCUIT");
		break;
	case ONU_E1_SYSTEM_LOOP:
		VOS_StrCpy(str, "SYSTEM");
		break;
	case ONU_E1_ALL_LOOP:
		VOS_StrCpy(str, "ALL");
		break;
	case 0:
		VOS_StrCpy(str, "NO");
		break;
	default:
		E1_ERROR_INFO_PRINT(("getOnuLoopBackStr()::loop=0x%02x    error!\r\n", loop));
		return VOS_ERROR;
	}

	return VOS_OK;
}
/* 取得时钟字符串 */
static STATUS getTxClockStr(UCHAR clk, char *str)
{
	if (NULL == str)
	{
		return VOS_ERROR;
	}

	switch (clk)
	{
	case E1_TX_CLOCK_AUTO:
		VOS_StrCpy(str, "AUTO   ");
		break;
	case E1_TX_CLOCK_PICK:
		VOS_StrCpy(str, "DISTILL");
		break;
	case E1_TX_CLOCK_SPEC:
		VOS_StrCpy(str, "2M     ");
		break;
	case E1_TX_CLOCK_CRYS:
		VOS_StrCpy(str, "LOCAL  ");
		break;
	default:
		VOS_StrCpy(str, "ERROR  ");
		E1_ERROR_INFO_PRINT(("getTxClockStr()::clk=0x%02x    error!\r\n", clk));
		return VOS_ERROR;
	}

	return VOS_OK;
}

static STATUS checkFpgaPortList(struct vty *vty, E1SlotPort *pFpgaPort)
{
	ULONG i, j;

	if (NULL == pFpgaPort)
	{
		E1_ERROR_INFO_PRINT(("checkFpgaPortList()::pFpgaPort=NULL  error! \r\n"));
		return VOS_ERROR;
	}

	for (i = 0; pFpgaPort[i].slotIdx != 0xFFFFFFFF && i < TDM_E1_MAX_SLOT_INDEX; i++)
	{
		if (get_gfa_e1_slotno() != pFpgaPort[i].slotIdx)
		{
			vty_out( vty, "input slot no:%d is error.  (E1 Board Slot is %d) .\r\n", pFpgaPort[i].slotIdx, get_gfa_e1_slotno() );
			return VOS_ERROR;
		}
		else
		{
			for (j = 0; pFpgaPort[i].portIdx[j] != 0xFFFFFFFF && j < TDM_E1_MAX_PORT_INDEX; j++)
			{
				if(TdmPortRangeCheck(pFpgaPort[i].portIdx[j] ) != ROK)
				/*if ( (pFpgaPort[i].portIdx[j] == 0) || (pFpgaPort[i].portIdx[j] > TDM_FPGA_MAX) )*/
				{
					vty_out( vty, " %% %s fpga%d/%d index error \r\n", GetGFA6xxxTdmNameString(), pFpgaPort[i].slotIdx, pFpgaPort[i].portIdx[j] );
					return VOS_ERROR;
				}

				if(getTdmChipInserted((unsigned char)(pFpgaPort[i].slotIdx - 1), pFpgaPort[i].portIdx[j] - 1) != TDM_FPGA_EXIST)
				{
					vty_out( vty, "  %% %s fpga%d/%d is not inserted\r\n", GetGFA6xxxTdmNameString(), pFpgaPort[i].slotIdx, pFpgaPort[i].portIdx[j] );
					return VOS_ERROR;
				}
			}
		}
	}

	return VOS_OK;
}

static STATUS checkTdmSlotPortList(struct vty *vty, E1SlotPort *pE1SlotPort)
{
	ULONG i, j;

	if (NULL == pE1SlotPort)
	{
		E1_ERROR_INFO_PRINT(("checkTdmSlotPortList()::pE1SlotPort=NULL  error! \r\n"));
		return VOS_ERROR;
	}

	for (i = 0; pE1SlotPort[i].slotIdx != 0xFFFFFFFF && i < TDM_E1_MAX_SLOT_INDEX; i++)
	{
		if (get_gfa_e1_slotno() != pE1SlotPort[i].slotIdx)
		{
			vty_out( vty, "input slot no:%d is error.  (E1 Board Slot is %d) .\r\n", pE1SlotPort[i].slotIdx, get_gfa_e1_slotno() );
			/*continue;  解决CLI输入E1板卡任意槽位号都可以的问题*/
			return VOS_ERROR;
		}
		else
		{
			for (j = 0; pE1SlotPort[i].portIdx[j] != 0xFFFFFFFF && j < TDM_E1_MAX_PORT_INDEX; j++)
			{
				if(TdmE1NumRangeCheck(pE1SlotPort[i].portIdx[j]) != ROK)
				/*if ( (pE1SlotPort[i].portIdx[j] == 0) || (pE1SlotPort[i].portIdx[j] > (MAX_E1_PER_FPGA * TDM_FPGA_MAX)) )*/
				{
					vty_out( vty, "%% tdm e1 port %d  is error. \r\n", pE1SlotPort[i].portIdx[j]);
					return VOS_ERROR;
				}

				if(getTdmChipInserted((unsigned char)(pE1SlotPort[i].slotIdx - 1), ((pE1SlotPort[i].portIdx[j]-1)/ MAX_E1_PER_FPGA)) != TDM_FPGA_EXIST)
				{
					vty_out( vty, "  %% %s fpga%d/%d is not inserted\r\n", GetGFA6xxxTdmNameString(), pE1SlotPort[i].slotIdx, (pE1SlotPort[i].portIdx[j] / MAX_E1_PER_FPGA) );
					return VOS_ERROR;
				}
			}
		}
	}

	return VOS_OK;
}

static STATUS checkOnuSlotPortList(E1SlotPort *pE1SlotPort)
{
	ULONG i, j;

	if (NULL == pE1SlotPort)
	{
		E1_ERROR_INFO_PRINT(("checkOnuSlotPortList()::pE1SlotPort=NULL  error! \r\n"));
		return VOS_ERROR;
	}

	for (i = 0; pE1SlotPort[i].slotIdx != 0xFFFFFFFF && i < TDM_E1_MAX_SLOT_INDEX; i++)
	{
		if ( (pE1SlotPort[i].slotIdx == 0) || (pE1SlotPort[i].slotIdx > MAX_ONU_BRD_NUM) )
		{
			E1_ERROR_INFO_PRINT(("checkOnuSlotPortList()::onu e1 board slot index=%d  error! \r\n", pE1SlotPort[i].slotIdx));
			return VOS_ERROR;
		}
		else
		{
			for (j = 0; pE1SlotPort[i].portIdx[j] != 0xFFFFFFFF && j < TDM_E1_MAX_PORT_INDEX; j++)
			{
				if ( ( pE1SlotPort[i].portIdx[j] > MAX_ONU_BOARD_E1 ) || ( 0 == pE1SlotPort[i].portIdx[j] ) )
				{
					E1_ERROR_INFO_PRINT(("checkOnuSlotPortList()::onu e1 port index=%d  error! \r\n", pE1SlotPort[i].portIdx[j]));
					return VOS_ERROR;
				}
			}
		}
	}

	return VOS_OK;
}

#if E1_PORT
#endif
DEFUN  (
		show_olt_e1_port,
		show_olt_e1_port_cmd,
		"show olt-e1-port {<slot/e1portlist>}*1",
		"show e1-port data\n"
		"show olt-e1-port data\n"
		"select olt e1 slot port\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i;
	char str[16], line[80];

	/*if (argc > 1)
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}*/

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	vty_out(vty, "\r\nIndex    AlarmStat    AlarmMask    TxClock\r\n");

	if (0 == argc)
	{
		for (i = 0; i < MAX_E1_PORT_NUM; i++)
		{
			idxs[2] = i;

			if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
			{
				vty_out(vty,"Get E1 Port:%d Failed.   \r\n", i + 1);
				return CMD_WARNING;
			}
			else
			{
				memset(line, 0, sizeof(line));

				memset(str, 0, sizeof(str));
				getAlarmStr(e1PortTable.eponE1PortAlarmStatus, str);
				strcat(line, str);

				memset(str, 0, sizeof(str));
				getAlarmStr(e1PortTable.eponE1PortAlarmMask, str);
				strcat(line, str);

				memset(str, 0, sizeof(str));
				getTxClockStr(e1PortTable.eponE1TxClock, str);
				strcat(line, str);

				vty_out(vty, "%02d       %s\r\n", i + 1, line);
			}
		}
	} 
	else if (1 == argc)
	{
		memset( (char *)e1SlotPort, 0xFF, sizeof(e1SlotPort) );

		if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}

		if (VOS_OK != checkTdmSlotPortList(vty, e1SlotPort))
		{
			/*vty_out(vty,"\r\n parameter error\r\n");*/
			return CMD_WARNING;
		}

		for (i = 0; i < MAX_E1_PORT_NUM && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
		{
			idxs[2] = e1SlotPort[0].portIdx[i] - 1;

			if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
			{
				vty_out(vty,"Get E1 Port:%d Failed.   \r\n", e1SlotPort[0].portIdx[i]);
				return CMD_WARNING;
			}
			else
			{
				memset(line, 0, sizeof(line));

				memset(str, 0, sizeof(str));
				getAlarmStr(e1PortTable.eponE1PortAlarmStatus, str);
				strcat(line, str);

				memset(str, 0, sizeof(str));
				getAlarmStr(e1PortTable.eponE1PortAlarmMask, str);
				strcat(line, str);

				memset(str, 0, sizeof(str));
				getTxClockStr(e1PortTable.eponE1TxClock, str);
				strcat(line, str);

				vty_out(vty, "%02d       %s\r\n", e1SlotPort[0].portIdx[i], line);
			}
		}
	}

	vty_out(vty, "\r\n");
	return( CMD_SUCCESS );
}

DEFUN  (
		show_onu_e1_port,
		show_onu_e1_port_cmd,
		"show onu-e1-port {<slot/port/onuid> <slot/e1portlist>}*1",
		"show e1-port data\n"
		"show onu-e1-port data\n"
		"input the onu slot/port/onuid\n"
		"select onu e1 slot:2-5   port:1-4\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i, j;
	ULONG PON_slot, PON_port, PON_onuid;
	char str[16], line[80];
	ULONG PonPortId, onuId;
    SHORT PonPortIndex;

	if (argc > 2 || argc == 1)
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	vty_out(vty, "\r\nIndex      AlarmStat    AlarmMask    TxClock    Impedance\r\n");

	if (0 == argc)
	{
		idxs[0] = 1;
		idxs[1] = TDMCARD_LAST;
		idxs[2] = MAX_E1_PER_FPGA * TDM_FPGA_MAX - 1;

		while ( VOS_OK == sw_e1PortTable_getNext( idxs, &e1PortTable ) )
		{
			memset(line, 0, sizeof(line));

			memset(str, 0, sizeof(str));
			getAlarmStr(e1PortTable.eponE1PortAlarmStatus, str);
			strcat(line, str);

			memset(str, 0, sizeof(str));
			getAlarmStr(e1PortTable.eponE1PortAlarmMask, str);
			strcat(line, str);

			memset(str, 0, sizeof(str));
			getTxClockStr(e1PortTable.eponE1TxClock, str);
			strcat(line, str);

			vty_out(vty, "%d/%d/%d %d/%d  %s    ", GET_PONSLOT(idxs[0])/*idxs[0] / 10000*/, GET_PONPORT(idxs[0] )/*(idxs[0] % 10000) / 1000*/, GET_ONUID(idxs[0])/*idxs[0] % 1000*/, idxs[1], idxs[2] + 1, line);

			if ( -1 == parseOnuIndexFromDevIdx(idxs[0], &PonPortId, &onuId) )
			{
				E1_ERROR_INFO_PRINT(("call  parseOnuIndexFromDevIdx()  error! \r\n"));
			}

			if (OAM_ONU_SLOT_GT_4E1_120ohm == ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].brdType)
			{
				vty_out(vty, "120 ohm\r\n");
			} 
			else if (OAM_ONU_SLOT_GT_4E1_75ohm == ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].brdType)
			{
				vty_out(vty, "75 ohm\r\n");
			}
			else
			{
				vty_out(vty, "unknown\r\n");
			}
		}
	} 
	else if (2 == argc)
	{
		if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}

        PonPortIndex = GetPonPortIdxBySlot( (short int)PON_slot, (short int)PON_port );
		if (VOS_ERROR == PonPortIndex)
		{
			vty_out( vty, "  %% Parameter is error.\r\n" );
			return CMD_WARNING;
		}

		if( ThisIsValidOnu( PonPortIndex, (PON_onuid-1) ) != ROK )
		{
			vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
			return( CMD_WARNING );
		}

		/*idxs[0] = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
              idxs[0]=MAKEDEVID(PON_slot,PON_port,PON_onuid);

		if ( -1 == parseOnuIndexFromDevIdx(idxs[0], &PonPortId, &onuId) )
		{
			vty_out(vty,"\r\n parse onu devIndex error\r\n");
			return CMD_WARNING;
		}

		memset( (char *)e1SlotPort, 0xFF, sizeof(e1SlotPort) );

		if (VOS_OK != parseE1SlotPortList(argv[1], e1SlotPort))
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}

		if (VOS_OK != checkOnuSlotPortList(e1SlotPort))
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}

		for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && e1SlotPort[i].slotIdx != 0xFFFFFFFF; i++)
		{
			idxs[1] = e1SlotPort[i].slotIdx;

			for (j = 0; j < TDM_E1_MAX_PORT_INDEX && e1SlotPort[i].portIdx[j] != 0xFFFFFFFF; j++)
			{
				idxs[2] = e1SlotPort[i].portIdx[j] - 1;

				if ( VOS_OK != checkOnuE1IsSupportE1(idxs[0], idxs[1], idxs[2]) )
				{
					vty_out(vty,"Onu:%d/%d/%d    Port:%d/%d  is not suppot E1.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
					return CMD_WARNING;
				}

				if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
				{
					vty_out(vty,"Get E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
					return CMD_WARNING;
				}
				else
				{
					memset(line, 0, sizeof(line));

					memset(str, 0, sizeof(str));
					getAlarmStr(e1PortTable.eponE1PortAlarmStatus, str);
					strcat(line, str);

					memset(str, 0, sizeof(str));
					getAlarmStr(e1PortTable.eponE1PortAlarmMask, str);
					strcat(line, str);

					memset(str, 0, sizeof(str));
					getTxClockStr(e1PortTable.eponE1TxClock, str);
					strcat(line, str);

					vty_out(vty, "%d/%d/%d %d/%d  %s    ", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1, line);

					if ( -1 == parseOnuIndexFromDevIdx(idxs[0], &PonPortId, &onuId) )
					{
						E1_ERROR_INFO_PRINT(("call  parseOnuIndexFromDevIdx()  error! \r\n"));
					}

					if (OAM_ONU_SLOT_GT_4E1_120ohm == ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].brdType)
					{
						vty_out(vty, "120 ohm\r\n");
					} 
					else if (OAM_ONU_SLOT_GT_4E1_75ohm == ExtBrdMgmtTable[PonPortId][onuId + 1].BrdMgmtTable[idxs[1]].brdType)
					{
						vty_out(vty, "75 ohm\r\n");
					}
					else
					{
						vty_out(vty, "unknown\r\n");
					}
				}
			}
		}
	}

	vty_out(vty, "\r\n");
	return( CMD_SUCCESS );
}

DEFUN  (
		loop_olt_e1_port,
		loop_olt_e1_port_cmd,
		"olt-e1-port loopback <slot/e1portlist> {[circuit|system]}*1",
		"loop e1-port\n"
		"loop olt-e1-port\n"
		"select olt e1 slot port\n"
		"circuit loopback\n"
		"system loopback\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	USHORT swLoopValue = 0;
	ULONG idxs[3], i;

	if ( (0 == argc) || (2 < argc) )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkTdmSlotPortList(vty,e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	idxs[0] = 1;
	idxs[1] = e1SlotPort[0].slotIdx;

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		idxs[2] = e1SlotPort[0].portIdx[i] - 1;

		if (1 == argc)
		{
			swLoopValue = TDM_E1_ALL_LOOP;
		}
		else if (2 == argc)
		{
			if(!VOS_StrCmp(argv[1],"circuit"))
			{
				swLoopValue = TDM_E1_CIRCUIT_LOOP;
			}
			else if(!VOS_StrCmp(argv[1],"system"))
			{
				swLoopValue = TDM_E1_SYSTEM_LOOP;
			}
			else
			{
				vty_out(vty,"%% Parameter is error.\r\n");
				continue;
			}
		}

		if( !checkLoopBack(idxs, swLoopValue) )
		{
			vty_out(vty,"e1 link has not been set! or an other system loop back has been set!.\r\n");
			return CMD_WARNING;
		}

		if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortLoop, idxs, swLoopValue ) )
		{
			vty_out(vty,"Set E1 Port:%d Failed.   \r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		no_loop_olt_e1_port,
		no_loop_olt_e1_port_cmd,
		"undo olt-e1-port loopback <slot/e1portlist>",
		"undo loop e1-port\n"
		"undo loop olt-e1-port\n"
		"undo loop olt-e1-port\n"
		"select olt e1 slot   port:1-24\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	ULONG idxs[3], i;

	if ( 1 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty, "\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkTdmSlotPortList(vty, e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	idxs[0] = 1;
	idxs[1] = e1SlotPort[0].slotIdx;

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		idxs[2] = e1SlotPort[0].portIdx[i] - 1;

		if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortLoop, idxs, 0 ) )
		{
			vty_out(vty,"Set E1 Port:%d Failed.   \r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		loop_onu_e1_port,
		loop_onu_e1_port_cmd,
		"onu-e1-port loopback <slot/port/onuid> <slot/e1portlist> {[circuit|system]}*1",
		"loop e1-port\n"
		"loop onu-e1-port\n"
		"input the onu slot/port/onuid\n"
		"select onu e1 slot:2-5   port:1-4\n"
		"circuit loopback\n"
		"system loopback\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	USHORT swLoopValue = 0;
	SHORT PonPortIndex = 0;
	ULONG idxs[3], i, j;
	ULONG PON_slot, PON_port, PON_onuid;

	if ( (2 != argc) && (3 != argc) )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

    PonPortIndex = GetPonPortIdxBySlot( (short int)PON_slot, (short int)PON_port );
	if (VOS_ERROR == PonPortIndex)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsValidOnu( PonPortIndex,(PON_onuid-1) ) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[1], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkOnuSlotPortList(e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	/*idxs[0] = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
        idxs[0]=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && e1SlotPort[i].slotIdx != 0xFFFFFFFF; i++)
	{
		idxs[1] = e1SlotPort[i].slotIdx;

		for (j = 0; j < TDM_E1_MAX_PORT_INDEX && e1SlotPort[i].portIdx[j] != 0xFFFFFFFF; j++)
		{
			idxs[2] = e1SlotPort[i].portIdx[j] - 1;

			if ( VOS_OK != checkOnuE1IsSupportE1(idxs[0], idxs[1], idxs[2]) )
			{
				vty_out(vty,"Onu:%d/%d/%d    Port:%d/%d  is not suppot E1.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}

			if (2 == argc)
			{
				swLoopValue = ONU_E1_ALL_LOOP;
			}
			else if (3 == argc)
			{
				if(!VOS_StrCmp(argv[2],"circuit"))
				{
					swLoopValue = ONU_E1_CIRCUIT_LOOP;
				}
				else if(!VOS_StrCmp(argv[2],"system"))
				{
					swLoopValue = ONU_E1_SYSTEM_LOOP;
				}
				else
				{
					vty_out(vty,"%% Parameter is error.\r\n");
					continue;
				}
			}

			if ( !checkLoopBack(idxs, swLoopValue) )
			{
				vty_out(vty,"e1 link has not been set! or an other system loop back has been set!.\r\n");
				return CMD_WARNING;
			}

			if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortLoop, idxs, swLoopValue ) )
			{
					vty_out(vty,"Set E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
			}
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		no_loop_onu_e1_port,
		no_loop_onu_e1_port_cmd,
		"undo onu-e1-port loopback <slot/port/onuid> <slot/e1portlist>",
		"undo loop e1-port\n"
		"undo loop onu-e1-port\n"
		"undo loop onu-e1-port\n"
		"input the onu slot/port/onuid\n"
		"select onu e1 slot:2-5   port:1-4\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	ULONG idxs[3], i, j;
	ULONG PON_slot, PON_port, PON_onuid;
    SHORT PonPortIndex;

	if (2 != argc)
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

    PonPortIndex = GetPonPortIdxBySlot( (short int)PON_slot, (short int)PON_port );
	if (VOS_ERROR == PonPortIndex)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsValidOnu( PonPortIndex,(PON_onuid-1) ) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[1], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkOnuSlotPortList(e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	/*idxs[0] = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
        idxs[0]=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && e1SlotPort[i].slotIdx != 0xFFFFFFFF; i++)
	{
		idxs[1] = e1SlotPort[i].slotIdx;

		for (j = 0; j < TDM_E1_MAX_PORT_INDEX && e1SlotPort[i].portIdx[j] != 0xFFFFFFFF; j++)
		{
			idxs[2] = e1SlotPort[i].portIdx[j] - 1;

			if ( VOS_OK != checkOnuE1IsSupportE1(idxs[0], idxs[1], idxs[2]) )
			{
				vty_out(vty,"Onu:%d/%d/%d    Port:%d/%d  is not suppot E1.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}

			if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortLoop, idxs, 0 ) )
			{
				vty_out(vty,"Set E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
			}
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		set_olt_e1_clock,
		set_olt_e1_clock_cmd,
		"olt-e1-port clock <slot/e1portlist> {[distill|2M|local]}*1",
		"set e1-port\n"
		"set olt-e1-port clock\n"
		"select olt e1 slot   port:1-24\n"
		"distill type\n"
		"2M type\n"
		"locall type\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	UCHAR eponE1TxClock = 0;
	ULONG idxs[3], i;

	if ( (1 != argc) && (2 != argc) )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkTdmSlotPortList(vty, e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	idxs[0] = 1;
	idxs[1] = e1SlotPort[0].slotIdx;

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		idxs[2] = e1SlotPort[0].portIdx[i] - 1;

		if (1 == argc)
		{
			eponE1TxClock = E1_TX_CLOCK_AUTO;
		}
		else if (2 == argc)
		{
			if(!VOS_StrCmp(argv[1],"distill"))
			{
				eponE1TxClock = E1_TX_CLOCK_PICK;
			}
			else if(!VOS_StrCmp(argv[1],"2m"))
			{
				eponE1TxClock = E1_TX_CLOCK_SPEC;
			}
			else if(!VOS_StrCmp(argv[1],"local"))
			{
				eponE1TxClock = E1_TX_CLOCK_CRYS;
			}
			else
			{
				vty_out(vty,"%% Parameter is error.\r\n");
				return CMD_WARNING;
			}
		}

		if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortTxClock, idxs, (USHORT)eponE1TxClock ) )
		{
			vty_out(vty,"Set Olt E1 Port:%d Tx Clock Failed.\r\n", idxs[2] + 1);
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		set_onu_e1_clock,
		set_onu_e1_clock_cmd,
		"onu-e1-port clock <slot/port/onuid> <slot/e1portlist> {[distill|2M|local]}*1",
		"set e1-port\n"
		"set onu-e1-port clock\n"
		"input the onu slot/port/onuid\n"
		"select onu e1 slot:2-5   port:1-4\n"
		"distill type\n"
		"2M type\n"
		"locall type\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	UCHAR eponE1TxClock = 0;
	ULONG idxs[3], i, j;
	ULONG PON_slot, PON_port, PON_onuid;
    SHORT PonPortIndex;

	if ( (2 != argc) && (3 != argc) )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

    PonPortIndex = GetPonPortIdxBySlot( (short int)PON_slot, (short int)PON_port );
	if (VOS_ERROR == PonPortIndex)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsValidOnu( PonPortIndex,(PON_onuid-1) ) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	/*idxs[0] = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
        idxs[0]=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[1], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkOnuSlotPortList(e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && e1SlotPort[i].slotIdx != 0xFFFFFFFF; i++)
	{
		idxs[1] = e1SlotPort[i].slotIdx;

		for (j = 0; j < TDM_E1_MAX_PORT_INDEX && e1SlotPort[i].portIdx[j] != 0xFFFFFFFF; j++)
		{
			idxs[2] = e1SlotPort[i].portIdx[j] - 1;

			if ( VOS_OK != checkOnuE1IsSupportE1(idxs[0], idxs[1], idxs[2]) )
			{
				vty_out(vty,"Onu:%d/%d/%d    Port:%d/%d  is not suppot E1.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}

			if (2 == argc)
			{
				eponE1TxClock = E1_TX_CLOCK_AUTO;
			}
			else if (3 == argc)
			{
				if(!VOS_StrCmp(argv[2],"distill"))
				{
					eponE1TxClock = E1_TX_CLOCK_PICK;
				}
				else if(!VOS_StrCmp(argv[2],"2m"))
				{
					eponE1TxClock = E1_TX_CLOCK_SPEC;
				}
				else if(!VOS_StrCmp(argv[2],"local"))
				{
					eponE1TxClock = E1_TX_CLOCK_CRYS;
				}
				else
				{
					vty_out(vty,"%% Parameter is error.\r\n");
					return CMD_WARNING;
				}
			}

			if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortTxClock, idxs, (USHORT)eponE1TxClock ) )
			{
				vty_out(vty,"Set Onu E1 Port:%d/%d/%d    %d/%d TxClock Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}
		}
	}

	return( CMD_SUCCESS );
}

#if ADD_E1_LINK
#endif
DEFUN  (
		add_e1_link,
		add_e1_link_cmd,
		"add e1-link <slot/e1portlist> onu-e1 <slot/port/onuid> <slot/e1portlist>",
		"add e1 link\n"
		"select olt e1\n"
		"select olt e1 slot:   port\n"
		"select onu e1\n"
		"input the onu slot/port/onuid\n"
		"select onu e1 slot:2-5   port:1-4\n"
		)
{
	E1SlotPort oltE1PortList[TDM_E1_MAX_SLOT_INDEX], onuE1PortList[TDM_E1_MAX_SLOT_INDEX];
	ULONG onuDevId, e1LinkCount, m, n;
	ULONG PON_slot, PON_port, PON_onuid;
    SHORT PonPortIndex;

	if ( 3 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != PON_ParseSlotPortOnu( argv[1], &PON_slot, &PON_port, &PON_onuid ))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

    PonPortIndex = GetPonPortIdxBySlot( (short int)PON_slot, (short int)PON_port );
	if (VOS_ERROR == PonPortIndex)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsValidOnu( PonPortIndex,(PON_onuid-1) ) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	/*onuDevId = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
        onuDevId=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	memset((char *)oltE1PortList, 0xFF, sizeof(oltE1PortList));
	memset((char *)onuE1PortList, 0xFF, sizeof(onuE1PortList));

	if (VOS_OK != parseE1SlotPortList(argv[0], oltE1PortList))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != parseE1SlotPortList(argv[2], onuE1PortList))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	/* 比较端口数目是否相当 */
	if ( !checkE1LinkPortList(oltE1PortList, onuE1PortList) )
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkTdmSlotPortList(vty, oltE1PortList))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != checkOnuSlotPortList(onuE1PortList))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	e1LinkCount = 0;

	for (m = 0; m < TDM_E1_MAX_SLOT_INDEX && onuE1PortList[m].slotIdx != 0xFFFFFFFF; m++)
	{
		for (n = 0; n < TDM_E1_MAX_PORT_INDEX && onuE1PortList[m].portIdx[n] != 0xFFFFFFFF; n++)
		{
			/* 检查索引是否已经存在 */
			if ( VOS_OK != checkE1LinkIsExist(vty, (UCHAR)(oltE1PortList[0].portIdx[e1LinkCount] - 1), onuDevId, (UCHAR)onuE1PortList[m].slotIdx, (UCHAR)(onuE1PortList[m].portIdx[n] - 1)) )
			{
				e1LinkCount++;
				return CMD_WARNING;
			}

			if ( VOS_OK != AddE1Link( (UCHAR)(oltE1PortList[0].portIdx[e1LinkCount] - 1), onuDevId, (UCHAR)onuE1PortList[m].slotIdx, (UCHAR)(onuE1PortList[m].portIdx[n] - 1), NULL ) )
			{
				vty_out(vty,"Add E1 Link Failed.   %d/%d<-->%d/%d/%d/ %d/%d\r\n", oltE1PortList[0].slotIdx, oltE1PortList[0].portIdx[e1LinkCount], PON_slot, PON_port, PON_onuid, onuE1PortList[m].slotIdx, onuE1PortList[m].portIdx[n]);
			}

			e1LinkCount++;
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		del_e1_link,
		del_e1_link_cmd,
		"delete e1-link <slot/e1portlist>",
		"delelt e1 link\n"
		"delelt e1 link\n"
		"select olt e1 slot port\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	ULONG i;

	if ( 1 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkTdmSlotPortList(vty, e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		/* 检查索引是否已经存在 */
		if ( VOS_OK != checkE1LinkIsNotExist(vty, (UCHAR)(e1SlotPort[0].portIdx[i] - 1) ) )
		{
			/*vty_out(vty,"\r\n call checkE1LinkIsNotExist() error!   \r\n");*/
			return CMD_WARNING;
		}

		if ( VOS_OK != DelE1Link(e1SlotPort[0].portIdx[i] - 1) )
		{
			vty_out(vty,"Delete E1 Link:%d Fail.\r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		show_e1_link,
		show_e1_link_cmd,
		"show e1-link {[active]}*1",
		"show e1 link\n"
		"show e1 link\n"
		"select show active e1 link or all the e1 link\n"
		)
{
	UCHAR oltE1PortIdx;
	ULONG idxs[3];
	e1LinkTable_row_entry Entry;

	if ( 1 < argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if ( 1 == argc )
	{
		if( VOS_StrCmp(argv[0],"active") )
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	vty_out(vty," \r\nOLT E1Port      ONU    E1Port\r\n");

	for (oltE1PortIdx = 0; oltE1PortIdx < (MAX_E1_PORT_NUM/*MAX_E1_PER_FPGA * TDM_FPGA_MAX*/); oltE1PortIdx++)
	{
		idxs[2] = oltE1PortIdx;

		if ( VOS_OK != tdm_e1LinkTable_get( idxs, &Entry ) )
		{
			vty_out(vty,"\r\n call tdm_e1LinkTable_get() error!   tdmE1Index=%ld\r\n", (oltE1PortIdx+1));
			return CMD_WARNING;
		}

		if (1 == argc)
		{
			if (MIB_E1_ENABLE == Entry.eponE1LocalEnable)
			{
				vty_out(vty, "%d/%02d            %d/%d/%d  %d/%d\r\n", get_gfa_e1_slotno(), oltE1PortIdx + 1, GET_PONSLOT(Entry.onuDevId)/*Entry.onuDevId / 10000*/, GET_PONPORT(Entry.onuDevId)/*(Entry.onuDevId % 10000) / 1000*/, GET_ONUID(Entry.onuDevId)/*Entry.onuDevId % 1000*/, Entry.onuE1SlotId, Entry.onuE1Id + 1);
				/*vty_out(vty, "Description:%s\r\n", Entry.eponE1Description);*/
			}
		} 
		else
		{
			vty_out(vty, "%d/%02d            %d/%d/%d  %d/%d\r\n", get_gfa_e1_slotno(), oltE1PortIdx + 1, GET_PONSLOT(Entry.onuDevId)/*Entry.onuDevId / 10000*/, GET_PONPORT(Entry.onuDevId)/*(Entry.onuDevId % 10000) / 1000*/, GET_ONUID(Entry.onuDevId)/*Entry.onuDevId % 1000*/, Entry.onuE1SlotId, Entry.onuE1Id + 1);
			/*vty_out(vty, "Description:%s\r\n", Entry.eponE1Description);*/
		}
	}

	return( CMD_SUCCESS );
}

#if E1_MASK
#endif
DEFUN  (
		_alarm_mask_olt_e1,
		_alarm_mask_olt_e1_cmd,
		"alarm-mask olt-e1-port <slot/e1portlist> [all|los|ais]",
		"alarm mask e1 port\n"
		"alarm mask olt e1 port\n"
		"select olt e1 slot port\n"
		"all alarm mask\n"
		"los alarm mask\n"
		"ais alarm mask\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i;
	UCHAR mask = 0;

	if ( 2 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	mask = alarm_mask_e1_parase(argc, argv);
	if (0xFF == mask)
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkTdmSlotPortList(vty,e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	idxs[0] = 1;
	idxs[1] = e1SlotPort[0].slotIdx;

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		idxs[2] = e1SlotPort[0].portIdx[i] - 1;

		if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
		{
			vty_out(vty,"Get E1 Port:%d Failed.   \r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}
		else
		{
			/*e1PortTable.eponE1PortAlarmMask &= 0xFF00; 清除原来的LOS AIS */
			e1PortTable.eponE1PortAlarmMask |= mask;

			if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortAlarmMask, idxs, e1PortTable.eponE1PortAlarmMask ) )
			{
				vty_out(vty,"Set E1 Port:%d Failed.   \r\n", e1SlotPort[0].portIdx[i]);
				return CMD_WARNING;
			}
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		_undo_alarm_mask_olt_e1,
		_undo_alarm_mask_olt_e1_cmd,
		"undo alarm-mask olt-e1-port <slot/e1portlist> [all|los|ais]",
		"undo alarm mask e1 port\n"
		"undo alarm mask olt e1 port\n"
		"undo alarm mask olt e1 port\n"
		"select olt e1 slot   port:1-24\n"
		"all alarm mask\n"
		"los alarm mask\n"
		"ais alarm mask\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i;
	UCHAR mask = 0;

	if ( 2 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	mask = alarm_mask_e1_parase(argc, argv);
	if (0xFF == mask)
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkTdmSlotPortList(vty, e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	idxs[0] = 1;
	idxs[1] = e1SlotPort[0].slotIdx;

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		idxs[2] = e1SlotPort[0].portIdx[i] - 1;

		if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
		{
			vty_out(vty,"Get E1 Port:%d Failed.   \r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}
		else
		{
			e1PortTable.eponE1PortAlarmMask &= ~mask;

			if ( VOS_OK != e1PortTable_set( LEAF_eponE1PortAlarmMask, idxs, e1PortTable.eponE1PortAlarmMask ) )
			{
				vty_out(vty,"Set E1 Port:%d Failed.   \r\n", e1SlotPort[0].portIdx[i]);
				return CMD_WARNING;
			}
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		_show_alarm_mask_olt_e1,
		_show_alarm_mask_olt_e1_cmd,
		"show alarm-mask olt-e1-port <slot/e1portlist>",
		"show alarm mask\n"
		"show alarm mask olt e1 port\n"
		"show alarm mask olt e1 port\n"
		"select olt e1 slot port\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i;
	char str[32];

	if ( 1 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkTdmSlotPortList(vty, e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	idxs[0] = 1;
	idxs[1] = e1SlotPort[0].slotIdx;

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		idxs[2] = e1SlotPort[0].portIdx[i] - 1;

		if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
		{
			vty_out(vty,"Get E1 Port:%d Failed.   \r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}
		else
		{
			vty_out( vty, "E1 port: %d/%d alarm mask:%s\r\n", e1SlotPort[0].slotIdx, idxs[2] + 1, alarm_e1_mask_to_str((UCHAR)e1PortTable.eponE1PortAlarmMask, str) );
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		alarm_mask_onu_e1,
		alarm_mask_onu_e1_cmd,
		"alarm-mask onu-e1-port <slot/port/onuid> <slot/e1portlist> [all|los|ais]",
		"alarm mask e1 port\n"
		"alarm mask onu e1 port\n"
		"input the onu slot/port/onuid\n"
		"select onu e1 slot:2-5   port:1-4\n"
		"all alarm mask\n"
		"los alarm mask\n"
		"ais alarm mask\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i, j;
	UCHAR mask = 0;
	ULONG PON_slot, PON_port, PON_onuid;
    SHORT PonPortIndex;

	if ( 3 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

    PonPortIndex = GetPonPortIdxBySlot( (short int)PON_slot, (short int)PON_port );
	if (VOS_ERROR == PonPortIndex)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsValidOnu( PonPortIndex,(PON_onuid-1) ) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	/*idxs[0] = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
        idxs[0]=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	mask = alarm_mask_e1_parase(argc, argv);
	if (0xFF == mask)
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[1], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkOnuSlotPortList(e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && e1SlotPort[i].slotIdx != 0xFFFFFFFF; i++)
	{
		idxs[1] = e1SlotPort[i].slotIdx;

		for (j = 0; j < TDM_E1_MAX_PORT_INDEX && e1SlotPort[i].portIdx[j] != 0xFFFFFFFF; j++)
		{
			idxs[2] = e1SlotPort[i].portIdx[j] - 1;

			if ( VOS_OK != checkOnuE1IsSupportE1(idxs[0], idxs[1], idxs[2]) )
			{
				vty_out(vty,"Onu:%d/%d/%d    Port:%d/%d  is not suppot E1.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}

			if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
			{
				vty_out(vty,"Get E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}
			else
			{
				/*e1PortTable.eponE1PortAlarmMask &= 0xFF00; 清楚原来的LOS,AIS */
				e1PortTable.eponE1PortAlarmMask |= mask;

				if ( VOS_OK != e1PortTable_set(LEAF_eponE1PortAlarmMask, idxs, e1PortTable.eponE1PortAlarmMask) )
				{
					vty_out(vty,"Set E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
					return CMD_WARNING;
				}
			}
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		undo_alarm_mask_onu_e1,
		undo_alarm_mask_onu_e1_cmd,
		"undo alarm-mask onu-e1-port <slot/port/onuid> <slot/e1portlist> [all|los|ais]",
		"undo alarm mask e1 port\n"
		"undo alarm mask onu e1 port\n"
		"undo alarm mask onu e1 port\n"
		"input the onu slot/port/onuid\n"
		"select onu e1 slot:2-5   port:1-4\n"
		"all alarm mask\n"
		"los alarm mask\n"
		"ais alarm mask\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i, j;
	UCHAR mask = 0;
	ULONG PON_slot, PON_port, PON_onuid;
    SHORT PonPortIndex;

	if ( 3 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

    PonPortIndex = GetPonPortIdxBySlot( (short int)PON_slot, (short int)PON_port );
	if (VOS_ERROR == PonPortIndex)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsValidOnu( PonPortIndex,(PON_onuid-1) ) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	/*idxs[0] = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
        idxs[0]=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	mask = alarm_mask_e1_parase(argc, argv);
	if (0xFF == mask)
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[1], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkOnuSlotPortList(e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && e1SlotPort[i].slotIdx != 0xFFFFFFFF; i++)
	{
		idxs[1] = e1SlotPort[i].slotIdx;

		for (j = 0; j < TDM_E1_MAX_PORT_INDEX && e1SlotPort[i].portIdx[j] != 0xFFFFFFFF; j++)
		{
			idxs[2] = e1SlotPort[i].portIdx[j] - 1;

			if ( VOS_OK != checkOnuE1IsSupportE1(idxs[0], idxs[1], idxs[2]) )
			{
				vty_out(vty,"Onu:%d/%d/%d    Port:%d/%d  is not suppot E1.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}

			if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
			{
				vty_out(vty,"Get E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}
			else
			{
				e1PortTable.eponE1PortAlarmMask &= ~mask;

				if ( VOS_OK != e1PortTable_set(LEAF_eponE1PortAlarmMask, idxs, e1PortTable.eponE1PortAlarmMask) )
				{
					vty_out(vty,"Set E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
					return CMD_WARNING;
				}
			}
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		show_alarm_mask_onu_e1,
		show_alarm_mask_onu_e1_cmd,
		"show alarm-mask onu-e1-port <slot/port/onuid> <slot/e1portlist>",
		"show alarm mask\n"
		"show alarm mask onu e1 port\n"
		"show alarm mask onu e1 port\n"
		"input the onu slot/port/onuid\n"
		"select onu e1 slot:2-5   port:1-4\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i, j;
	char str[32];
	ULONG PON_slot, PON_port, PON_onuid;
    SHORT PonPortIndex;

	if ( 2 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

    PonPortIndex = GetPonPortIdxBySlot( (short int)PON_slot, (short int)PON_port );
	if (VOS_ERROR == PonPortIndex)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsValidOnu( PonPortIndex,(PON_onuid-1) ) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	/*idxs[0] = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
        idxs[0]=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[1], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkOnuSlotPortList(e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && e1SlotPort[i].slotIdx != 0xFFFFFFFF; i++)
	{
		idxs[1] = e1SlotPort[i].slotIdx;

		for (j = 0; j < TDM_E1_MAX_PORT_INDEX && e1SlotPort[i].portIdx[j] != 0xFFFFFFFF; j++)
		{
			idxs[2] = e1SlotPort[i].portIdx[j] - 1;

			if ( VOS_OK != checkOnuE1IsSupportE1(idxs[0], idxs[1], idxs[2]) )
			{
				vty_out(vty,"Onu:%d/%d/%d    Port:%d/%d  is not suppot E1.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}

			if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
			{
				vty_out(vty,"Get E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}
			else
			{
				vty_out( vty, "Onu: %d/%d/%d  E1 port:%d/%d alarm mask:%s\r\n\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1, alarm_e1_mask_to_str((UCHAR)e1PortTable.eponE1PortAlarmMask, str) );
			}
		}
	}

	return( CMD_SUCCESS );
}

#if E1_VLAN
#endif
DEFUN  (
		e1_vlan,
		e1_vlan_cmd,
		"e1-vlan <slot/fpgaportlist> vlan-id <1-4094> priority <0-7>",
		"set e1 vlan\n"
		"select olt e1 slot   port:1-3\n"
		"set e1 vlan id\n"
		"set e1 vlan id:1-4094\n"
		"set e1 vlan priority:0-7\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	UCHAR i;

	if ( 3 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkFpgaPortList(vty, e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		if ( (e1SlotPort[0].portIdx[i] - 1) >= TDM_FPGA_MAX )
		{
			vty_out(vty,"\r\n SigPort=%ld      error!\r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}

		if ( -1 == SetTdmE1LinkVlanEnable( get_gfa_e1_slotno(),e1SlotPort[0].portIdx[i], (unsigned char)VOS_AtoL(argv[2]), (unsigned short)VOS_AtoL(argv[1]) ) )
		{
			vty_out(vty,"Set Fpga%d Vlan Failed.\r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		undo_e1_vlan,
		undo_e1_vlan_cmd,
		"undo e1-vlan <slot/fpgaportlist>",
		"undo e1 vlan\n"
		"undo e1 vlan\n"
		"select olt e1 slot   port:1-3\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	UCHAR i;

	if ( 1 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkFpgaPortList(vty, e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		if ( (e1SlotPort[0].portIdx[i] - 1) >= TDM_FPGA_MAX )
		{
			vty_out(vty,"\r\n SigPort=%ld      error!\r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}

		if (0 != SetTdmE1LinkVlanDisable(get_gfa_e1_slotno(), e1SlotPort[0].portIdx[i]))
		{
			vty_out(vty,"Disable Fpga%d Vlan Failed.\r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		show_e1_vlan,
		show_e1_vlan_cmd,
		"show e1-vlan",
		"show e1 vlan\n"
		"show e1 vlan\n"
		)
{
	UCHAR i, enableFlag, Priority;
	USHORT VlanId;

	/*if ( 0 != argc )
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}*/

	for (i = 0; i < TDM_FPGA_MAX; i++)
	{
		if (VOS_OK != GetE1VlanEnable(i, &enableFlag, &Priority, &VlanId))
		{
			vty_out(vty,"\r\n call GetE1VlanEnable() error!   SigPort=%ld\r\n", i);
			return CMD_WARNING;
		}

		if (MIB_E1_VLAN_DISABLE == enableFlag)
		{
			vty_out(vty, "FpgaId:%d  Disable  VlanId:%d   VlanPri:%d\r\n", i + 1, VlanId, Priority);
		} 
		else
		{
			vty_out(vty, "FpgaId:%d  Enable  VlanId:%d   VlanPri:%d\r\n", i + 1, VlanId, Priority);
		}
	}

	return( CMD_SUCCESS );
}

#ifdef  _GFA6xxx_ONU_E1LINK_OAM_DEBUG 

/*以下命令用于查询ONU上E1 连接配置*/
	QDEFUN (
		   onu_e1link_debug_show,
		   onu_e1link_debug_show_cmd,
		   "debug onu e1-link <slot/port/onuid>",
		   "show onu e1-link information\n"
		   "show onu e1-link information\n"
		   "show onu e1-linkn\n"
		   "input the slot/port/onuid\n",
		   &eventQueId

)
{
	LONG lRet;
	ULONG portid,slotid,onuid,onuDevIdx;
	ULONG tdmslot,sgidx,onu;
	ULONG ulIfIndex = 0;
	unsigned char TdmSlot1=0, SgIdx1=0;
	unsigned short int  OnuLogicId=0;
	int ret;
	short int PonPortIdx = 0;
	VoiceVlanEnable VoiceVlan;

	lRet = PON_ParseSlotPortOnu( argv[0], &slotid, &portid, &onuid );
	if( lRet != VOS_OK )
		return CMD_WARNING;

	if(PonCardSlotPortCheckWhenRunningByVty(slotid, portid, vty) != ROK)
		return(CMD_WARNING);
	
	/*onuDevIdx =slotid*10000+portid*1000+onuid;*/
        onuDevIdx=MAKEDEVID(slotid,portid,onuid);
        
	PonPortIdx = GetPonPortIdxBySlot(slotid, portid);
	if(ThisIsValidOnu(PonPortIdx,(onuid-1)) != ROK )
	{
		vty_out(vty, " %% onu %d/%d/%d not exist\r\n", slotid, portid, onuid );
		return( CMD_WARNING );
	}

	ulIfIndex = ( ULONG ) ( vty->index ) ;	
	PON_GetSlotPortOnu( ulIfIndex, (ULONG *)&tdmslot, (ULONG *)&sgidx, (ULONG *)&onu);

	ret = GetOnuBelongToSG(onuDevIdx, &TdmSlot1, &SgIdx1, &OnuLogicId);
	if( ret == TDM_VM_NOT_SUPPORT_VOICE )
		{
		vty_out(vty,"\r\n onu %d/%d/%d not support voice service\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
		}
	else if( ret == TDM_VM_NOT_IN_ANY_SG )
		{
		vty_out(vty,"\r\n onu %d/%d/%d is not configed to any %s\r\n",slotid, portid,onuid, GetGFA6xxxTdmNameString());
		return( CMD_WARNING );
		}
	if((TdmSlot1 != tdmslot ) ||(SgIdx1 != sgidx ))
		{
		vty_out(vty,"\r\n onu %d/%d/%d is belonged to another %s(%d/%d)\r\n",slotid, portid,onuid, GetGFA6xxxTdmNameString(),TdmSlot1, SgIdx1);
		return( CMD_WARNING );
		}

	if( GetOnuOperStatus(PonPortIdx, onuid-1) != ONU_OPER_STATUS_UP )
	{
		vty_out(vty,"\r\n onu %d/%d/%d is off-line\r\n",slotid, portid,onuid );
		return( CMD_WARNING );
	}
	ret = GetOnuVoiceVlan( onuDevIdx, &VoiceVlan);
	if( ret != VOS_OK )
		{
		vty_out(vty," %% Execute err\r\n");
		return( CMD_WARNING );
		}
	vty_out(vty," onu %d/%d/%d voice vlan information\r\n",slotid, portid,onuid );
	if(VoiceVlan.Enable == V2R1_ENABLE )
		vty_out(vty,"voice vlan enabled\r\n");
	else vty_out(vty,"voice vlan disabled\r\n");
	vty_out(vty,"voice vlan priority:%d\r\n", VoiceVlan.Priority );
	vty_out(vty,"voice vlan id:%d\r\n",VoiceVlan.VlanId );

	return( CMD_SUCCESS );
	
}
#endif

#if ONU_E1_QUERY
#endif
DEFUN  (
		query_onu_e1_all_info,
		query_onu_e1_all_info_cmd,
		"query-onu-e1-all-info <slot/port/onuid>",
		"query onu e1 all info\n"
		"input the onu slot/port/onuid\n"
		)
{
	UCHAR i, j;
    SHORT PonPortIdx;
	OAM_OnuE1Info oam_OnuE1Info;
	ULONG PON_slot, PON_port, PON_onuid;

	if ( 1 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

    PonPortIdx = GetPonPortIdxBySlot( (short int)PON_slot, (short  int)PON_port );
	if (VOS_ERROR == PonPortIdx)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsValidOnu( PonPortIdx,(PON_onuid-1) ) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	memset((char *)&oam_OnuE1Info, 0, sizeof(OAM_OnuE1Info));

	if (VOS_OK != GetOnuE1All(MAKEDEVID(PON_slot,PON_port,PON_onuid)/*PON_slot * 10000 + PON_port * 1000 + PON_onuid*/, &oam_OnuE1Info))
	{
		vty_out(vty,"\r\n call GetOnuE1All() error!   \r\n");
		return CMD_WARNING;
	}

	E1_ERROR_INFO_PRINT(("E1PortTotalCount=%d\r\n", oam_OnuE1Info.E1PortTotalCount));
	for (i = 0; i < oam_OnuE1Info.E1PortTotalCount; i++)
	{
		if (OAM_E1_DISABLE == oam_OnuE1Info.oam_OnuE1Info[i].E1Enable)
		{
			vty_out(vty,"\r\nOnu %d/%d/%d  E1 Port:%d/%d is Disable:\r\n", PON_slot, PON_port, PON_onuid, oam_OnuE1Info.oam_OnuE1Info[i].E1SlotIdx, oam_OnuE1Info.oam_OnuE1Info[i].E1PortIdx);
		} 
		else
		{
			vty_out(vty,"\r\nOnu %d/%d/%d  E1 Port:%d/%d is Enable:\r\n", PON_slot, PON_port, PON_onuid, oam_OnuE1Info.oam_OnuE1Info[i].E1SlotIdx, oam_OnuE1Info.oam_OnuE1Info[i].E1PortIdx);
		}

		vty_out(vty,"DesMac: ");
		for (j = 0; j < 6; j++)
		{
			vty_out(vty,"%02x ", oam_OnuE1Info.oam_OnuE1Info[i].DesMac[j]);
		}
		vty_out(vty,"\r\n");

		vty_out(vty,"SrcMac: ");
		for (j = 0; j < 6; j++)
		{
			vty_out(vty,"%02x ", oam_OnuE1Info.oam_OnuE1Info[i].SrcMac[j]);
		}
		vty_out(vty,"\r\n");

		if (OAM_E1_VLAN_DISABLE == oam_OnuE1Info.oam_OnuE1Info[i].VlanEable)
		{
			vty_out(vty,"Vlan is Disable:\r\n");
		} 
		else
		{
			vty_out(vty,"Vlan is Enable:\r\n");
		}

		vty_out(vty,"Vlan Id:%d    Vlan Pri:%d\r\n", oam_OnuE1Info.oam_OnuE1Info[i].VlanTag & 0x0FFF, oam_OnuE1Info.oam_OnuE1Info[i].VlanTag >> 13);

		vty_out(vty,"TxClock:");
		if (0 == oam_OnuE1Info.oam_OnuE1Info[i].ClockControl)
		{
			vty_out(vty,"Auto\r\n");
		} 
		else if (1 == oam_OnuE1Info.oam_OnuE1Info[i].ClockControl)
		{
			vty_out(vty,"Distill\r\n");
		}
		else
		{
			vty_out(vty,"Error!\r\n");
		}

		if (0x0080 == oam_OnuE1Info.oam_OnuE1Info[i].AlarmMask)
		{
			vty_out(vty,"Alarm Mask:LOS\r\n");
		}
		else if (0x0000 == oam_OnuE1Info.oam_OnuE1Info[i].AlarmMask)
		{
			vty_out(vty,"Alarm Mask:NO\r\n");
		}
		else
		{
			vty_out(vty,"Alarm Mask:0x%04x error!\r\n", oam_OnuE1Info.oam_OnuE1Info[i].AlarmMask);
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		e1_debug,
		e1_debug_cmd,
		"e1-debug [ErrorInfo|BoardTx|BoardRx|OamTx|OamRx] [Disable|Enable]",
		"e1 debug\n"
		"e1 function error info\n"
		"sw send board msg to tdm\n"
		"sw receive board msg from tdm\n"
		"sw send e1 oam\n"
		"sw receive e1 oam\n"
		"turn on the debug\n"
		"turn off the debug\n"
		)
{
	if ( 2 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (!VOS_StrCmp(argv[0],"errorinfo"))
	{
		if (!VOS_StrCmp(argv[1],"disable"))
		{
			debugE1 &= ~E1_ERROR_INFO;
		} 
		else if (!VOS_StrCmp(argv[1],"enable"))
		{
			debugE1 |= E1_ERROR_INFO;
		}
		else
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}
	} 
	else if (!VOS_StrCmp(argv[0],"boardtx"))
	{
		if (!VOS_StrCmp(argv[1],"disable"))
		{
			debugE1 &= ~E1_TX_BOARD_MSG;
		} 
		else if (!VOS_StrCmp(argv[1],"enable"))
		{
			debugE1 |= E1_TX_BOARD_MSG;
		}
		else
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}
	}
	else if (!VOS_StrCmp(argv[0],"boardrx"))
	{
		if (!VOS_StrCmp(argv[1],"disable"))
		{
			debugE1 &= ~E1_RX_BOARD_MSG;
		} 
		else if (!VOS_StrCmp(argv[1],"enable"))
		{
			debugE1 |= E1_RX_BOARD_MSG;
		}
		else
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}
	}
	else if (!VOS_StrCmp(argv[0],"oamtx"))
	{
		if (!VOS_StrCmp(argv[1],"disable"))
		{
			debugE1 &= ~E1_TX_OAM_MSG;
		} 
		else if (!VOS_StrCmp(argv[1],"enable"))
		{
			debugE1 |= E1_TX_OAM_MSG;
		}
		else
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}
	}
	else if (!VOS_StrCmp(argv[0],"oamrx"))
	{
		if (!VOS_StrCmp(argv[1],"disable"))
		{
			debugE1 &= ~E1_RX_OAM_MSG;
		} 
		else if (!VOS_StrCmp(argv[1],"enable"))
		{
			debugE1 |= E1_RX_OAM_MSG;
		}
		else
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	return( CMD_SUCCESS );
}

/* 此命令显示SW保存的E1 Port数据 */
DEFUN  (
		show_e1_port,
		show_e1_port_cmd,
		"show e1-port {<slot/port/onuid> <slot/e1portlist>}*1",
		"show olt or onu e1 port info\n"
		"show olt or onu e1 port info\n"
		"onu DevIndex\n"
		"onu E1 slotNo and PortNo\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i, j;
	ULONG PON_slot, PON_port, PON_onuid;

	if ( 2 < argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (2 == argc)/* 查询一个onu中，一个E1板卡上的多个E1 Port */
	{
		if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}

		if (VOS_ERROR == GetPonPortIdxBySlot( (short int)PON_slot, (short  int)PON_port ))
		{
			vty_out( vty, "  %% Parameter is error.\r\n" );
			return CMD_WARNING;
		}

		memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

		if (VOS_OK != parseE1SlotPortList(argv[1], e1SlotPort))
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}

		if (VOS_OK != checkOnuSlotPortList(e1SlotPort))
		{
			vty_out(vty,"\r\n parameter error\r\n");
			return CMD_WARNING;
		}

		/*idxs[0] = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
              idxs[0] =MAKEDEVID(PON_slot,PON_port,PON_onuid);

		/*vty_out(vty,"****************************OltE1PortTable************************\r\n");*/
		vty_out(vty," OltE1PortTable\r\n");
		vty_out(vty,"Idx        AlarmStat       AlarmMask         Loop           TxClock\r\n");
		/*vty_out(vty,"-------------------------------------------------------------------\r\n");*/

		for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && e1SlotPort[i].slotIdx != 0xFFFFFFFF; i++)
		{
			idxs[1] = e1SlotPort[i].slotIdx;

			for (j = 0; j < TDM_E1_MAX_PORT_INDEX && e1SlotPort[i].portIdx[j] != 0xFFFFFFFF; j++)
			{
				idxs[2] = e1SlotPort[i].portIdx[j] - 1;

				if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
				{
					vty_out(vty,"Get E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
					return CMD_WARNING;
				}
				else
				{
					vty_out(vty, "%05d      0x%04x          0x%04x            0x%02x           %d\r\n", idxs[2], 
						e1PortTable.eponE1PortAlarmStatus, e1PortTable.eponE1PortAlarmMask, e1PortTable.eponE1Loop, 
						e1PortTable.eponE1TxClock);
				}
			}
		}
	} 
	else/* 查询一OLT上24个E1 Port */
	{
		vty_out(vty," OltE1PortTable\r\n");
		/*vty_out(vty,"****************************OltE1PortTable************************\r\n");*/
		vty_out(vty,"Idx        AlarmStat       AlarmMask         Loop           TxClock\r\n");
		/*vty_out(vty,"-------------------------------------------------------------------\r\n");*/

		idxs[0] = 1;
		idxs[1] = get_gfa_e1_slotno();

		for (i = 0; i < MAX_E1_PORT_NUM; i++)
		{
			idxs[2] = i;

			if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
			{
				vty_out(vty,"\r\n call sw_e1PortTable_get() error!   index=%d\r\n", i);
				return CMD_WARNING;
			}
			else
			{
				vty_out(vty, "%05d      0x%04x          0x%04x            0x%02x           %d\r\n", i, 
					e1PortTable.eponE1PortAlarmStatus, e1PortTable.eponE1PortAlarmMask, e1PortTable.eponE1Loop, 
					e1PortTable.eponE1TxClock);
			}
		}
	}

	return( CMD_SUCCESS );
}

#if SHOW_E1_LOOP
#endif

DEFUN  (
		show_onu_e1_loop,
		show_onu_e1_loop_cmd,
		"show onu-e1-port loopback <slot/port/onuid> <slot/e1portlist>",
		"show onu e1 port\n"
		"show onu e1 port\n"
		"show onu e1 port loopback\n"
		"onu DevIndex\n"
		"onu E1 slotNo and PortNo\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i, j;
	ULONG PON_slot, PON_port, PON_onuid;
	char str[16];
    SHORT PonPortIdx;

	if ( 2 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	if (VOS_OK != PON_ParseSlotPortOnu( argv[0], &PON_slot, &PON_port, &PON_onuid ))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

    PonPortIdx = GetPonPortIdxBySlot( (short int)PON_slot, (short  int)PON_port );
	if (VOS_ERROR == PonPortIdx)
	{
		vty_out( vty, "  %% Parameter is error.\r\n" );
		return CMD_WARNING;
	}

	if( ThisIsValidOnu( PonPortIdx,(PON_onuid-1) ) != ROK )
	{
		vty_out(vty," %% onu %d/%d/%d not exist\r\n", PON_slot, PON_port, PON_onuid );
		return( CMD_WARNING );
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[1], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkOnuSlotPortList(e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	/*idxs[0] = PON_slot * 10000 + PON_port * 1000 + PON_onuid;*/
        idxs[0]=MAKEDEVID(PON_slot,PON_port,PON_onuid);

	for (i = 0; i < TDM_E1_MAX_SLOT_INDEX && e1SlotPort[i].slotIdx != 0xFFFFFFFF; i++)
	{
		idxs[1] = e1SlotPort[i].slotIdx;

		for (j = 0; j < TDM_E1_MAX_PORT_INDEX && e1SlotPort[i].portIdx[j] != 0xFFFFFFFF; j++)
		{
			idxs[2] = e1SlotPort[i].portIdx[j] - 1;

			if ( VOS_OK != checkOnuE1IsSupportE1(idxs[0], idxs[1], idxs[2]) )
			{
				vty_out(vty,"Onu:%d/%d/%d    Port:%d/%d  is not suppot E1.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}

			if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
			{
				vty_out(vty,"Get E1 Port:%d/%d/%d    %d/%d  Failed.\r\n", PON_slot, PON_port, PON_onuid, idxs[1], idxs[2] + 1);
				return CMD_WARNING;
			}
			else
			{
				memset(str, 0, sizeof(str));
				getOnuLoopBackStr(e1PortTable.eponE1Loop, str);
				vty_out(vty, "%d/%d/%d  %d/%d   LoopBack:%s\r\n", PON_slot, PON_port, PON_onuid, e1SlotPort[i].slotIdx, e1SlotPort[i].portIdx[j], str);
			}
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		show_olt_e1_loop,
		show_olt_e1_loop_cmd,
		"show olt-e1-port loopback <slot/e1portlist>",
		"show olt e1 port\n"
		"show olt e1 port\n"
		"show olt e1 port loopback\n"
		"olt E1 slotNo and PortNo\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	e1PortTable_t e1PortTable;
	ULONG idxs[3], i;
	char str[16];

	if ( 1 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if ( VOS_OK != checkTdmSlotPortList(vty, e1SlotPort) )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	for (i = 0; i < MAX_E1_PORT_NUM && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		idxs[2] = e1SlotPort[0].portIdx[i] - 1;

		if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
		{
			vty_out(vty,"\r\n get e1 port error!   index=%d\r\n", e1SlotPort[0].portIdx[i]);
			return CMD_WARNING;
		}
		else
		{
			memset(str, 0, sizeof(str));
			getOltLoopBackStr(e1PortTable.eponE1Loop, str);
			vty_out(vty, "%02d    LoopBack:%s\r\n", e1SlotPort[0].portIdx[i], str);
		}
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		show_max_frm_gap,
		show_max_frm_gap_cmd,
		"show max-frm-gap",
		"show max frame gap\n"
		"show max frame gap\n"
		)
{
	ULONG idxs[3], i;
	UCHAR maxFrmGapArray[8 * 3 + 1];

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	if ( VOS_OK != tdm_getMaxFrameGap(idxs, maxFrmGapArray) )
	{
		vty_out(vty,"Get e1 max frame gap failed!\r\n");
		return CMD_WARNING;
	}

	for (i = 0; i < MAX_E1_PORT_NUM; i++)
	{
		vty_out(vty,"Idx:%02d   RegValue:0x%02x\r\n", i + 1, maxFrmGapArray[i]);
	}

	return( CMD_SUCCESS );
}

DEFUN  (
		setEth2E1Buf,
		setEth2E1Buf_cmd,
		"set eth2e1-buffer <slot/e1portlist> <4-15>",
		"set eth to e1 tx buffer\n"
		"set eth to e1 tx buffer\n"
		"select olt e1 slot port\n"
		"the value of tx buffer:4-15(ms)\n"
		)
{
	E1SlotPort e1SlotPort[TDM_E1_MAX_SLOT_INDEX];
	UCHAR eth2E1TxBufRegArray[8 * 3 + 1], tmp = 0;
	ULONG idxs[3], i;

	if ( 2 != argc )
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	memset((char *)e1SlotPort, 0xFF, sizeof(e1SlotPort));

	if (VOS_OK != parseE1SlotPortList(argv[0], e1SlotPort))
	{
		vty_out(vty,"\r\n parameter error\r\n");
		return CMD_WARNING;
	}

	if (VOS_OK != checkTdmSlotPortList(vty,e1SlotPort))
	{
		/*vty_out(vty,"\r\n parameter error\r\n");*/
		return CMD_WARNING;
	}

	tmp = (unsigned char)VOS_AtoL(argv[1]);

	if ( tmp < 4 || tmp > 15 )
	{
		vty_out(vty,"\r\n the input param:%d is out of range!\r\n", tmp);
		return CMD_WARNING;
	}

	memset(eth2E1TxBufRegArray, 0, sizeof(eth2E1TxBufRegArray));

	idxs[0] = 1;
	idxs[1] = e1SlotPort[0].slotIdx;

	for (i = 0; i < TDM_E1_MAX_PORT_INDEX && e1SlotPort[0].portIdx[i] != 0xFFFFFFFF; i++)
	{
		idxs[2] = e1SlotPort[0].portIdx[i] - 1;
		eth2E1TxBufRegArray[idxs[2]] = tmp;
	}

	if (VOS_OK != tdm_setEth2E1TxBuffer(idxs, eth2E1TxBufRegArray))
	{
		vty_out(vty,"\r\n Send the msg to TDM board failed!\r\n");
		return CMD_WARNING;
	}

	return( CMD_SUCCESS );
}

int e1_init_func()
{
	return VOS_OK;
}

int e1_showrun( struct vty * vty )
{    
	return VOS_OK;
}


int e1_config_write ( struct vty * vty )
{
	return VOS_OK;
}

struct cmd_node e1_port_node =
{
	E1_NODE,
	NULL,
	1
};

LONG e1_node_install()
{
	install_node( &e1_port_node, e1_config_write);
	e1_port_node.prompt = ( CHAR * ) VOS_Malloc( 64, MODULE_RPU_PON);

	if ( !e1_port_node.prompt )
	{
		ASSERT( 0 );
		return -IFM_E_NOMEM;
	}

	install_default( E1_NODE );
	return VOS_OK;
}

LONG e1_module_init()
{
	struct cl_cmd_module *e1_module = NULL;

	e1_module = ( struct cl_cmd_module *)VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_RPU_PON);

	if (!e1_module)
	{
		ASSERT( 0 );
		return -IFM_E_NOMEM;
	}

	VOS_MemZero((char *)e1_module, sizeof(struct cl_cmd_module));

	e1_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_RPU_PON );
	if ( !e1_module->module_name )
	{
		ASSERT( 0 );
		VOS_Free( e1_module );
		return -IFM_E_NOMEM;
	}

	VOS_StrCpy( e1_module->module_name, "tdm-E1" );

	e1_module->init_func = e1_init_func;
	e1_module->showrun_func = e1_showrun;
	e1_module->next = NULL;
	e1_module->prev = NULL;

	if ( cmd_rugular_register( &stCMD_E1_Port_List_Check ) == no_match )
	{
		ASSERT( 0 );
	}

	if ( cmd_rugular_register( &stCMD_Fpga_Port_List_Check ) == no_match )
	{
		ASSERT( 0 );
	}

	/*if ( cmd_rugular_register( &stCMD_Onu_Index_Check ) == no_match )
	{
		ASSERT( 0 );
	}*/

	cl_install_module( e1_module );

	return VOS_OK;
}


extern void e1_debug_CommandInstall(void);

void e1_CommandInstall(void)
{
	install_element(CONFIG_NODE, &into_epon_e1_node_cmd);
	install_element(E1_NODE,     &show_olt_e1_port_cmd);
	install_element(E1_NODE,     &show_onu_e1_port_cmd);
	install_element(E1_NODE,     &loop_olt_e1_port_cmd);
	install_element(E1_NODE,     &no_loop_olt_e1_port_cmd);
	install_element(E1_NODE,     &loop_onu_e1_port_cmd);
	install_element(E1_NODE,     &no_loop_onu_e1_port_cmd);
	install_element(DEBUG_HIDDEN_NODE,     &set_olt_e1_clock_cmd);
	install_element(DEBUG_HIDDEN_NODE,     &set_onu_e1_clock_cmd);
	install_element(E1_NODE,     &add_e1_link_cmd);
	install_element(E1_NODE,     &del_e1_link_cmd);
	install_element(E1_NODE,     &show_e1_link_cmd);
	install_element(E1_NODE,     &_alarm_mask_olt_e1_cmd);
	install_element(E1_NODE,     &_undo_alarm_mask_olt_e1_cmd);
	install_element(E1_NODE,     &_show_alarm_mask_olt_e1_cmd);
	install_element(E1_NODE,     &alarm_mask_onu_e1_cmd);
	install_element(E1_NODE,     &undo_alarm_mask_onu_e1_cmd);
	install_element(E1_NODE,     &show_alarm_mask_onu_e1_cmd);
	install_element(E1_NODE,     &e1_vlan_cmd);
	install_element(E1_NODE,     &undo_e1_vlan_cmd);
	install_element(E1_NODE,     &show_e1_vlan_cmd);
	install_element(E1_NODE,     &query_onu_e1_all_info_cmd);
	install_element(E1_NODE,     &show_onu_e1_loop_cmd);
	install_element(E1_NODE,     &show_olt_e1_loop_cmd);

	install_element(DEBUG_HIDDEN_NODE, &e1_debug_cmd);
	install_element(DEBUG_HIDDEN_NODE, &show_e1_port_cmd);
	install_element(DEBUG_HIDDEN_NODE, &show_max_frm_gap_cmd);
	install_element(DEBUG_HIDDEN_NODE, &setEth2E1Buf_cmd);

	/* 正式发布去除e1_debug_CommandInstall() */
    /*e1_debug_CommandInstall();*/
	e1_DataSave_CommandInstall();
}

LONG E1_CliInit()
{
	e1_node_install();
	e1_module_init();
	e1_CommandInstall();

	return VOS_OK;
}

#endif	/* EPON_MODULE_TDM_SERVICE */

