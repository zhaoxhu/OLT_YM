
#include "OltGeneral.h"
#if( EPON_MODULE_TDM_SERVICE == EPON_MODULE_YES )
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "gwEponSys.h"

#include "onu/ExtBoardType.h"
#include "E1_MIB.h"
#include "mn_e1.h"
#include "e1_oam.h"
#include "e1_apis.h"
#include"V2R1_product.h"
#include "Tdm_apis.h"
#include "Tdm_comm.h"


/* 查看两端索引是否已经用过 */
STATUS checkE1LinkIsExist(struct vty *vty, UCHAR tdmE1Index, ULONG onuDevId, UCHAR onuE1SlotId, UCHAR  onuE1Id)
{
	AllE1Info allE1Info;
	ULONG idxs[3], i, j;

	if ( tdmE1Index >= (MAX_E1_PER_FPGA * TDM_FPGA_MAX) )
	{
		E1_ERROR_INFO_PRINT(("checkE1LinkIsExist()::tdmE1Index=%d  error! \r\n", tdmE1Index));
		return VOS_ERROR;
	}

	memset((char *)&allE1Info, 0, sizeof(AllE1Info));

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	if ( VOS_OK != tdm_getAllE1Info(idxs, 0xFF, &allE1Info) )
	{
		E1_ERROR_INFO_PRINT(("checkE1LinkIsExist()::onuE1SlotId=%d  error! \r\n", onuE1SlotId));
		return VOS_ERROR;
	}

	for (i = 0; i < TDM_FPGA_MAX; i++)
	{
		for (j = 0; j < allE1Info.fpgaE1Info[i].fpgaValidE1Count; j++)
		{
			if ( ( (tdmE1Index / MAX_E1_PER_FPGA) == i) && ( (tdmE1Index % MAX_E1_PER_FPGA) == allE1Info.fpgaE1Info[i].eachE1Info[j].e1Index ) && (onuDevId == allE1Info.fpgaE1Info[i].eachE1Info[j].onuDevId) 
				&& (onuE1SlotId == allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1SlotId) && (onuE1Id == allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1Id) )
			{
				/* 链接已经存在 */
				if (NULL != vty)
				{
					vty_out(vty,"\r\nThe E1 Link has already been existed:%d/%d<-->%d/%d/%d  %d/%d\r\n", get_gfa_e1_slotno(), tdmE1Index + 1, GET_PONSLOT(onuDevId)/*onuDevId / 10000*/, GET_PONPORT(onuDevId)/*(onuDevId % 10000) / 1000*/, GET_ONUID(onuDevId)/*onuDevId % 1000*/, onuE1SlotId, onuE1Id + 1);
				} 
				else
				{
					sys_console_printf("\r\nThe E1 Link has already been existed:%d/%d<-->%d/%d/%d  %d/%d\r\n", get_gfa_e1_slotno(), tdmE1Index + 1, GET_PONSLOT(onuDevId)/*onuDevId / 10000*/, GET_PONPORT(onuDevId)/*(onuDevId % 10000) / 1000*/, GET_ONUID(onuDevId)/*onuDevId % 1000*/, onuE1SlotId, onuE1Id + 1);
				}

				return VOS_ERROR;
			}
			else if ( ( (tdmE1Index / MAX_E1_PER_FPGA) == i) && ( (tdmE1Index % MAX_E1_PER_FPGA) == allE1Info.fpgaE1Info[i].eachE1Info[j].e1Index ) )
			{
				/* olt侧索引已被使用 */
				if (NULL != vty)
				{
					vty_out(vty,"\r\nOlt E1 Link has already been existed:%d/%d<-->%d/%d/%d  %d/%d\r\n", get_gfa_e1_slotno(), tdmE1Index + 1, GET_PONSLOT(onuDevId)/*onuDevId / 10000*/, GET_PONPORT(onuDevId)/*(onuDevId % 10000) / 1000*/, GET_ONUID(onuDevId)/*onuDevId % 1000*/, onuE1SlotId, onuE1Id + 1);
				} 
				else
				{
					sys_console_printf("\r\nOlt E1 Link has already been existed:%d/%d<-->%d/%d/%d  %d/%d\r\n", get_gfa_e1_slotno(), tdmE1Index + 1, GET_PONSLOT(onuDevId)/*onuDevId / 10000*/, GET_PONPORT(onuDevId)/*(onuDevId % 10000) / 1000*/, GET_ONUID(onuDevId)/*onuDevId % 1000*/, onuE1SlotId, onuE1Id + 1);
				}

				return VOS_ERROR;
			}
			else if ( (onuDevId == allE1Info.fpgaE1Info[i].eachE1Info[j].onuDevId) 
				&& (onuE1SlotId == allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1SlotId) 
				&& (onuE1Id == allE1Info.fpgaE1Info[i].eachE1Info[j].onuE1Id) )
			{
				/* onu侧索引已被使用 */
				if (NULL != vty)
				{
					vty_out(vty,"\r\nOnu E1 Link has already been existed:%d/%d<-->%d/%d/%d  %d/%d\r\n", get_gfa_e1_slotno(), tdmE1Index + 1, GET_PONSLOT(onuDevId)/*onuDevId / 10000*/, GET_PONPORT(onuDevId)/*(onuDevId % 10000) / 1000*/, GET_ONUID(onuDevId)/*onuDevId % 1000*/, onuE1SlotId, onuE1Id + 1);
				} 
				else
				{
					sys_console_printf("\r\nOnu E1 Link has already been existed:%d/%d<-->%d/%d/%d  %d/%d\r\n", get_gfa_e1_slotno(), tdmE1Index + 1, GET_PONSLOT(onuDevId)/*onuDevId / 10000*/, GET_PONPORT(onuDevId)/*(onuDevId % 10000) / 1000*/, GET_ONUID(onuDevId)/*onuDevId % 1000*/, onuE1SlotId, onuE1Id + 1);
				}

				return VOS_ERROR;
			}
		}
	}

	return VOS_OK;
}

STATUS checkE1LinkIsNotExist(struct vty *vty, UCHAR tdmE1Index)
{
	AllE1Info allE1Info;
	ULONG idxs[3], i, j;

	if ( tdmE1Index >= (MAX_E1_PER_FPGA * TDM_FPGA_MAX) )
	{
		E1_ERROR_INFO_PRINT(("checkE1LinkIsExist()::tdmE1Index=%d  error! \r\n", tdmE1Index));
		return VOS_ERROR;
	}

	memset((char *)&allE1Info, 0, sizeof(AllE1Info));

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	if ( VOS_OK != tdm_getAllE1Info(idxs, 0xFF, &allE1Info) )
	{
		E1_ERROR_INFO_PRINT(("checkE1LinkIsExist()::tdm_getAllE1Info()  error! \r\n"));
		return VOS_ERROR;
	}

	for (i = 0; i < TDM_FPGA_MAX; i++)
	{
		for (j = 0; j < allE1Info.fpgaE1Info[i].fpgaValidE1Count; j++)
		{
			if ( ( (tdmE1Index / MAX_E1_PER_FPGA) == i) && ( (tdmE1Index % MAX_E1_PER_FPGA) == allE1Info.fpgaE1Info[i].eachE1Info[j].e1Index ) )
			{
				return VOS_OK;
			}
		}
	}

	if (NULL != vty)
	{
		vty_out(vty,"\r\n%% olt e1 index%d is not exist\r\n", tdmE1Index + 1);
	} 
	else
	{
		sys_console_printf("\r\n%% olt e1 index%d is not exist\r\n", tdmE1Index + 1);
	}
	
	return VOS_ERROR;
}


STATUS AddE1Link( UCHAR tdmE1Index, ULONG onuDevId, UCHAR onuE1SlotId, UCHAR  onuE1Id, UCHAR *eponE1Description )
{
	ULONG idxs[3];
	OAM_OnuE1Link oam_OnuE1Link;
	UCHAR mac[6];
	e1VlanTable_row_entry e1VlanTable_row_entry_tmp;
	short int vlan_tag=0;

	if ( tdmE1Index >= (MAX_E1_PER_FPGA * TDM_FPGA_MAX) )
	{
		E1_ERROR_INFO_PRINT(("AddE1Link()::tdmE1Index=%d  error! \r\n", tdmE1Index));
		return VOS_ERROR;
	}

	if (VOS_OK != checkOnuE1IsSupportE1(onuDevId, onuE1SlotId, onuE1Id))
	{
		E1_ERROR_INFO_PRINT(("AddE1Link()::checkOnuE1IsSupportE1()  error! \r\n"));
		return VOS_ERROR;
	}

	if ( ( NULL != eponE1Description ) && ( strlen(eponE1Description) > (E1_DESCRIPTION_MAX + 1) ) )
	{
		E1_ERROR_INFO_PRINT(("AddE1Link()::strlen(eponE1Description)=%d  error! \r\n", strlen(eponE1Description)));
		return VOS_ERROR;
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = tdmE1Index;

	/* 配置TDM上的链接数据 */
	if ( VOS_OK != tdm_e1LinkTable_rowset(idxs, onuDevId, onuE1SlotId, onuE1Id, MIB_E1_ENABLE, eponE1Description) )
	{
		E1_ERROR_INFO_PRINT(("AddE1Link()::tdm_e1LinkTable_set()  MIB_E1_ENABLE=%d   error! \r\n", MIB_E1_ENABLE));
		return VOS_ERROR;
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = tdmE1Index / MAX_E1_PER_FPGA;

	if ( VOS_OK != tdm_e1VlanTable_get( idxs, &e1VlanTable_row_entry_tmp ) )
	{
		E1_ERROR_INFO_PRINT(("AddE1Link()::tdm_e1VlanTable_get()   error! \r\n"));
		return VOS_ERROR;
	}

	/* 准备发送OAM */
	memset( (UCHAR *)&oam_OnuE1Link, 0, sizeof(OAM_OnuE1Link) );
	oam_OnuE1Link.MsgType = SET_ONU_E1_LINK_REQ;
	oam_OnuE1Link.E1PortTotalCount = 1;
	oam_OnuE1Link.oam_OnuE1Link[0].E1PortIdx = onuE1Id + 1;
	oam_OnuE1Link.oam_OnuE1Link[0].E1SlotIdx = onuE1SlotId;
	oam_OnuE1Link.oam_OnuE1Link[0].E1Enable  = OAM_E1_ENABLE;
	vlan_tag = e1VlanTable_row_entry_tmp.eponVlanPri;
	vlan_tag = (vlan_tag << 13 ) + e1VlanTable_row_entry_tmp.eponVlanId;
	oam_OnuE1Link.oam_OnuE1Link[0].VlanTag = vlan_tag;
	if(e1VlanTable_row_entry_tmp.eponVlanEnable == V2R1_ENABLE)
		{
		oam_OnuE1Link.oam_OnuE1Link[0].VlanEable = OAM_E1_VLAN_ENABLE;		
		}
	else{
		oam_OnuE1Link.oam_OnuE1Link[0].VlanEable = OAM_E1_VLAN_DISABLE;
		oam_OnuE1Link.oam_OnuE1Link[0].VlanTag = V2R1_DEFAULT_VLAN;
		}

	memcpy(mac, E1MAC, 6);
	mac[4] += tdmE1Index/* 0-23 */ / MAX_E1_PER_FPGA;
	mac[5] += tdmE1Index % MAX_E1_PER_FPGA;
	memcpy(oam_OnuE1Link.oam_OnuE1Link[0].DesMac, mac, 6);

	mac[5] += 0x80;
	memcpy(oam_OnuE1Link.oam_OnuE1Link[0].SrcMac, mac, 6);

	if ( VOS_OK != SetOnuE1Link(onuDevId, &oam_OnuE1Link) )
	{
		E1_ERROR_INFO_PRINT(("AddE1Link()::SetOnuE1Link()   error! \r\n"));
		/*return VOS_ERROR;*/
	}

	if(e1VlanTable_row_entry_tmp.eponVlanEnable == V2R1_ENABLE)
		AddOnuE1LinkMacToSw(get_gfa_e1_slotno(), (tdmE1Index / MAX_E1_PER_FPGA) + 1, (tdmE1Index % MAX_E1_PER_FPGA)+1, onuDevId, e1VlanTable_row_entry_tmp.eponVlanId);
	else 
		AddOnuE1LinkMacToSw(get_gfa_e1_slotno(), (tdmE1Index / MAX_E1_PER_FPGA) + 1, (tdmE1Index % MAX_E1_PER_FPGA)+1, onuDevId, V2R1_DEFAULT_VLAN);

	return VOS_OK;
}

STATUS DelE1Link( UCHAR tdmE1Index )
{
	ULONG idxs[3];
	UCHAR mac[6];
	OAM_OnuE1Link oam_OnuE1Link;
	e1LinkTable_row_entry e1LinkTable_row_entry_tmp;
	e1VlanTable_row_entry e1VlanTable_row_entry_tmp;
	e1PortTable_t e1PortTable;

	if ( tdmE1Index >= (MAX_E1_PER_FPGA * TDM_FPGA_MAX) )
	{
		E1_ERROR_INFO_PRINT(("DelE1Link()::tdmE1Index=%d  error! \r\n", tdmE1Index));
		return VOS_ERROR;
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = tdmE1Index;

	if ( VOS_OK != tdm_e1LinkTable_get( idxs, &e1LinkTable_row_entry_tmp ) )
	{
		E1_ERROR_INFO_PRINT(("DelE1Link()::tdm_e1LinkTable_get()  error! \r\n"));
		return VOS_ERROR;
	}

	/* TODO:清除环回 */
	if ( VOS_OK != sw_e1PortTable_get(idxs, &e1PortTable) )
	{
		E1_ERROR_INFO_PRINT(("DelE1Link()::sw_e1PortTable_get()   error! \r\n"));
	}

	e1PortTable.eponE1Loop &= TDM_E1_NO_LOOP;

	if ( VOS_OK != e1PortTable_set(LEAF_eponE1PortLoop, idxs, (USHORT)e1PortTable.eponE1Loop) )
	{
		E1_ERROR_INFO_PRINT(("DelE1Link()::e1PortTable_set()   error! \r\n"));
	}

	idxs[0] = e1LinkTable_row_entry_tmp.onuDevId;
	idxs[1] = e1LinkTable_row_entry_tmp.onuE1SlotId;
	idxs[2] = e1LinkTable_row_entry_tmp.onuE1Id;

	if ( VOS_OK != sw_e1PortTable_get(idxs, &e1PortTable) )
	{
		E1_ERROR_INFO_PRINT(("DelE1Link()::sw_e1PortTable_get()   error! \r\n"));
	}

	e1PortTable.eponE1Loop &= ONU_E1_NO_LOOP;

	if ( VOS_OK != e1PortTable_set(LEAF_eponE1PortLoop, idxs, (USHORT)e1PortTable.eponE1Loop) )
	{
		E1_ERROR_INFO_PRINT(("DelE1Link()::e1PortTable_set()   error! \r\n"));
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = tdmE1Index;

	/* TDM侧 */
	if ( VOS_OK != tdm_e1LinkTable_rowset( idxs, 0, 0, 0, MIB_E1_DISABLE, "" ) )
	{
		E1_ERROR_INFO_PRINT(("DelE1Link()::tdm_e1LinkTable_rowset()   error! \r\n"));
		return VOS_ERROR;
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = tdmE1Index / MAX_E1_PER_FPGA;

	if ( VOS_OK != tdm_e1VlanTable_get( idxs, &e1VlanTable_row_entry_tmp ) )
	{
		E1_ERROR_INFO_PRINT(("DelE1Link()::tdm_e1VlanTable_get()   error! \r\n"));
		return VOS_ERROR;
	}

	/* 准备发送OAM */
	memset( (UCHAR *)&oam_OnuE1Link, 0, sizeof(OAM_OnuE1Link) );
	oam_OnuE1Link.MsgType = SET_ONU_E1_LINK_REQ;
	oam_OnuE1Link.E1PortTotalCount = 1;
	oam_OnuE1Link.oam_OnuE1Link[0].E1Enable  = OAM_E1_DISABLE;
	oam_OnuE1Link.oam_OnuE1Link[0].E1PortIdx = e1LinkTable_row_entry_tmp.onuE1Id + 1;
	oam_OnuE1Link.oam_OnuE1Link[0].E1SlotIdx = e1LinkTable_row_entry_tmp.onuE1SlotId;
	if (MIB_E1_VLAN_DISABLE == e1VlanTable_row_entry_tmp.eponVlanEnable)
	{
		oam_OnuE1Link.oam_OnuE1Link[0].VlanEable = OAM_E1_VLAN_DISABLE;
	} 
	else
	{
		oam_OnuE1Link.oam_OnuE1Link[0].VlanEable = OAM_E1_VLAN_ENABLE;
	}
	/*oam_OnuE1Link.oam_OnuE1Link[0].VlanEable = e1VlanTable_row_entry_tmp.eponVlanEnable;*/
	oam_OnuE1Link.oam_OnuE1Link[0].VlanTag   = e1VlanTable_row_entry_tmp.eponVlanId | (e1VlanTable_row_entry_tmp.eponVlanPri << 13);

	memcpy(mac, E1MAC, 6);
	mac[4] += tdmE1Index/* 0-23 */ / MAX_E1_PER_FPGA;
	mac[5] += tdmE1Index % MAX_E1_PER_FPGA;
	memcpy(oam_OnuE1Link.oam_OnuE1Link[0].DesMac, mac, 6);

	mac[5] += 0x80;
	memcpy(oam_OnuE1Link.oam_OnuE1Link[0].SrcMac, mac, 6);

	if ( VOS_OK != SetOnuE1Link(e1LinkTable_row_entry_tmp.onuDevId, &oam_OnuE1Link) )
	{
		E1_ERROR_INFO_PRINT(("DelE1Link()::SetOnuE1Link()   error! \r\n"));
		/*return VOS_ERROR;*/
	}

	DelOnuE1LinkMacFromSw(get_gfa_e1_slotno(), (tdmE1Index / MAX_E1_PER_FPGA) + 1, (tdmE1Index % MAX_E1_PER_FPGA)+1, e1LinkTable_row_entry_tmp.onuDevId);

	return VOS_OK;
}
#if 0
/* 查找同一个fpga上链接的E1索引 ，*onuE1Count返回链接的E1个数*/
static STATUS SearchOnuDevIdxBySgIdx(unsigned char SgIdx, OnuE1Index *pOnuE1Index, UCHAR *onuE1Count)
{
	UCHAR i ,j;
	ULONG idxs[3];
	e1LinkTable_row_entry Entry;

	if (SgIdx >= TDM_FPGA_MAX)
	{
		E1_ERROR_INFO_PRINT(("SearchOnuDevIdxBySgIdx()::SgIdx=%d  error! \r\n", SgIdx));
		return VOS_ERROR;
	}

	if ( (NULL == pOnuE1Index) || (NULL == onuE1Count) )
	{
		E1_ERROR_INFO_PRINT(("SearchOnuDevIdxBySgIdx()::pOnuE1Index=NULL  or onuE1Count=NULL   error! \r\n"));
		return VOS_ERROR;
	}

	*onuE1Count = 0;

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = SgIdx * MAX_E1_PER_FPGA;

	for (i = 0; i < MAX_E1_PER_FPGA; i++)
	{
		if ( VOS_OK != tdm_e1LinkTable_get( idxs, &Entry ) )
		{
			E1_ERROR_INFO_PRINT(("SearchOnuDevIdxBySgIdx()::tdm_e1VlanTable_get()   error! \r\n"));
			return VOS_ERROR;
		}

		idxs[2] ++;

		if (MIB_E1_DISABLE == Entry.eponE1LocalEnable)
		{
			continue;
		}

              /* 检查是否有重复 */
		for (j = 0; j < *onuE1Count; j++)
		{
			if ( (pOnuE1Index[j].onuDevId == Entry.onuDevId) && (pOnuE1Index[j].onuE1SlotId == Entry.onuE1SlotId) && (pOnuE1Index[j].onuE1Id == Entry.onuE1Id) )
			{
				break;
			}
		}

		if (j == *onuE1Count)
		{
			pOnuE1Index[j].onuDevId    = Entry.onuDevId;
			pOnuE1Index[j].onuE1SlotId = Entry.onuE1SlotId;
			pOnuE1Index[j].onuE1Id     = Entry.onuE1Id;
		}

		(*onuE1Count)++;
	}

	return VOS_OK;
}
#endif
STATUS GetE1VlanEnable(unsigned char SgIdx, unsigned char *enableFlag, unsigned char *Priority, unsigned short *VlanId)
{
	ULONG idxs[3];
	e1VlanTable_row_entry e1VlanTable_row_entry_tmp;

	if (SgIdx >= TDM_FPGA_MAX)
	{
		E1_ERROR_INFO_PRINT(("GetE1VlanEnable()::SgIdx=%d  error! \r\n", SgIdx));
		return VOS_ERROR;
	}

	if ( (NULL == enableFlag) || (NULL == Priority) || (NULL == VlanId) )
	{
		E1_ERROR_INFO_PRINT(("GetE1VlanEnable()::param is NULL  error! \r\n", SgIdx));
		return VOS_ERROR;
	}

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	idxs[2] = SgIdx;

	/* 设置TDM侧 */
	if ( VOS_OK != tdm_e1VlanTable_get( idxs, &e1VlanTable_row_entry_tmp ) )
	{
		E1_ERROR_INFO_PRINT(("GetE1VlanEnable()::tdm_e1VlanTable_get()    error! \r\n"));
		return VOS_ERROR;
	}

	*enableFlag = e1VlanTable_row_entry_tmp.eponVlanEnable;
	*Priority   = e1VlanTable_row_entry_tmp.eponVlanPri;
	*VlanId     = e1VlanTable_row_entry_tmp.eponVlanId;

	return VOS_OK;
}

STATUS GetOamOnuE1Info(ULONG onuIndex, OAM_OnuE1Info *pOAM_OnuE1Info)
{
	onuE1Info onue1info;
	ULONG idxs[3], i, sw_idx[3];
	UCHAR mac[6];
	e1PortTable_t e1PortTable;

	if (NULL == pOAM_OnuE1Info)
	{
		E1_ERROR_INFO_PRINT(("GetOamOnuE1Info()::pOAM_OnuE1Info=NULL    error! \r\n"));
		return VOS_ERROR;
	}

	if (VOS_OK != onuDevIdxIsSupportE1(onuIndex))
	{
		E1_ERROR_INFO_PRINT(("GetOamOnuE1Info()::onuDevIdxIsSupportE1()    error! \r\n"));
		return VOS_ERROR;
	}

	memset((char *)pOAM_OnuE1Info, 0, sizeof(OAM_OnuE1Info));
	memset((char *)&onue1info, 0, sizeof(onuE1Info));

	pOAM_OnuE1Info->MsgType = SET_ONU_E1_ALL_REQ;

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	sw_idx[0] = onuIndex;

	if ( VOS_OK != tdm_getOnuE1Info( idxs, onuIndex, &onue1info ) )
	{
		E1_ERROR_INFO_PRINT(("GetOamOnuE1Info()::tdm_getOnuE1Info()    error! \r\n"));
		return VOS_ERROR;
	}

	for (i = 0; i < onue1info.onuValidE1Count; i++)
	{
		pOAM_OnuE1Info->E1PortTotalCount++;

		pOAM_OnuE1Info->oam_OnuE1Info[i].E1SlotIdx    = onue1info.onuEachE1Info[i].onuE1Slot;
		pOAM_OnuE1Info->oam_OnuE1Info[i].E1PortIdx    = onue1info.onuEachE1Info[i].onuE1Index + 1;
		
		if (MIB_E1_DISABLE == onue1info.onuEachE1Info[i].tdmE1Enable)
		{
			pOAM_OnuE1Info->oam_OnuE1Info[i].E1Enable = OAM_E1_DISABLE;
		} 
		else
		{
			pOAM_OnuE1Info->oam_OnuE1Info[i].E1Enable = OAM_E1_ENABLE;
		}
		E1_ERROR_INFO_PRINT(("count=%d    tdmE1Enable=%d     oamE1Enable=%d\r\n", i, onue1info.onuEachE1Info[i].tdmE1Enable, pOAM_OnuE1Info->oam_OnuE1Info[i].E1Enable));
		
		if (MIB_E1_VLAN_DISABLE == onue1info.onuEachE1Info[i].tdmVlanEnable)
		{
			pOAM_OnuE1Info->oam_OnuE1Info[i].VlanEable = OAM_E1_VLAN_DISABLE;
		} 
		else
		{
			pOAM_OnuE1Info->oam_OnuE1Info[i].VlanEable = OAM_E1_VLAN_ENABLE;
		}
		pOAM_OnuE1Info->oam_OnuE1Info[i].VlanTag      = onue1info.onuEachE1Info[i].tdmVlanTag;
		pOAM_OnuE1Info->oam_OnuE1Info[i].ClockControl = onue1info.onuEachE1Info[i].tdmE1TxClk;
		/*pOAM_OnuE1Info->oam_OnuE1Info[i].LoopControl  = onue1info.onuEachE1Info[i].tdmE1Loop;
		pOAM_OnuE1Info->oam_OnuE1Info[i].AlarmStat    = onue1info.onuEachE1Info[i].tdmAlarmStat;*/
		
		sw_idx[1] = onue1info.onuEachE1Info[i].onuE1Slot;
		sw_idx[2] = onue1info.onuEachE1Info[i].onuE1Index;

		if (VOS_OK != sw_e1PortTable_get( sw_idx, &e1PortTable ))
		{
			E1_ERROR_INFO_PRINT(("GetOamOnuE1Info()::onuDevIdx=%d slot=%d port=%d     error!\r\n", sw_idx[0], sw_idx[1], sw_idx[2]));
			pOAM_OnuE1Info->oam_OnuE1Info[i].AlarmMask = 0;
		}
		else
		{
			pOAM_OnuE1Info->oam_OnuE1Info[i].AlarmMask = e1PortTable.eponE1PortAlarmMask/* ONU侧的mask，不是TDM侧mask */;
		}

		memcpy(mac, E1MAC, 6);
		mac[4] += onue1info.onuEachE1Info[i].tdmSgIfId/*0-2*/;
		mac[5] += onue1info.onuEachE1Info[i].tdmE1Id;
		memcpy(pOAM_OnuE1Info->oam_OnuE1Info[i].DesMac, mac, 6);

		mac[5] += 0x80;
		memcpy(pOAM_OnuE1Info->oam_OnuE1Info[i].SrcMac, mac, 6);
	}

	return VOS_OK;
}

STATUS GetOamOnuBoardE1Info(ULONG onuIndex, UCHAR slotIdx, OAM_OnuE1Info *pOAM_OnuE1Info)
{
	onuE1Info onue1info;
	ULONG idxs[3], i, sw_idx[3];
	UCHAR mac[6];
	e1PortTable_t e1PortTable;

	if (NULL == pOAM_OnuE1Info)
	{
		E1_ERROR_INFO_PRINT(("GetOamOnuE1Info()::pOAM_OnuE1Info=NULL    error! \r\n"));
		return VOS_ERROR;
	}

	if (VOS_OK != checkOnuE1IsSupportE1(onuIndex, slotIdx, 0))
	{
		E1_ERROR_INFO_PRINT(("GetOamOnuE1Info()::checkOnuE1IsSupportE1()    error! \r\n"));
		return VOS_ERROR;
	}

	memset((char *)pOAM_OnuE1Info, 0, sizeof(OAM_OnuE1Info));
	memset((char *)&onue1info, 0, sizeof(onuE1Info));

	pOAM_OnuE1Info->MsgType = SET_ONU_E1_ALL_REQ;

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();
	sw_idx[0] = onuIndex;

	if ( VOS_OK != tdm_getOnuE1Info( idxs, onuIndex, &onue1info ) )
	{
		E1_ERROR_INFO_PRINT(("GetOamOnuE1Info()::tdm_getOnuE1Info()    error! \r\n"));
		return VOS_ERROR;
	}

	for (i = 0; i < onue1info.onuValidE1Count; i++)
	{
		if (onue1info.onuEachE1Info[i].onuE1Slot != slotIdx)
		{
			continue;
		}

		pOAM_OnuE1Info->E1PortTotalCount++;

		pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].E1SlotIdx    = onue1info.onuEachE1Info[i].onuE1Slot;
		pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].E1PortIdx    = onue1info.onuEachE1Info[i].onuE1Index + 1;

		if (MIB_E1_DISABLE == onue1info.onuEachE1Info[i].tdmE1Enable)
		{
			pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].E1Enable = OAM_E1_DISABLE;
		} 
		else
		{
			pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].E1Enable = OAM_E1_ENABLE;
		}
		E1_ERROR_INFO_PRINT(("count=%d    tdmE1Enable=%d     oamE1Enable=%d\r\n", i, onue1info.onuEachE1Info[i].tdmE1Enable, pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].E1Enable));

		if (MIB_E1_VLAN_DISABLE == onue1info.onuEachE1Info[i].tdmVlanEnable)
		{
			pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].VlanEable = OAM_E1_VLAN_DISABLE;
		} 
		else
		{
			pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].VlanEable = OAM_E1_VLAN_ENABLE;
		}
		pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].VlanTag      = onue1info.onuEachE1Info[i].tdmVlanTag;
		pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].ClockControl = onue1info.onuEachE1Info[i].tdmE1TxClk;
		/*pOAM_OnuE1Info->oam_OnuE1Info[i].LoopControl  = onue1info.onuEachE1Info[i].tdmE1Loop;
		pOAM_OnuE1Info->oam_OnuE1Info[i].AlarmStat    = onue1info.onuEachE1Info[i].tdmAlarmStat;*/
		sw_idx[1] = onue1info.onuEachE1Info[i].onuE1Slot;
		sw_idx[2] = onue1info.onuEachE1Info[i].onuE1Index;
		if (VOS_OK != sw_e1PortTable_get( sw_idx, &e1PortTable ))
		{
			E1_ERROR_INFO_PRINT(("GetOamOnuBoardE1Info()::onuDevIdx=%d slot=%d port=%d     error!\r\n", sw_idx[0], sw_idx[1], sw_idx[2]));
			pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].AlarmMask = 0;
		}
		else
		{
			pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].AlarmMask = e1PortTable.eponE1PortAlarmMask/* ONU侧的mask，不是TDM侧mask */;
		}

		memcpy(mac, E1MAC, 6);
		mac[4] += onue1info.onuEachE1Info[i].tdmSgIfId/*0-2*/;
		mac[5] += onue1info.onuEachE1Info[i].tdmE1Id;
		memcpy(pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].DesMac, mac, 6);

		mac[5] += 0x80;
		memcpy(pOAM_OnuE1Info->oam_OnuE1Info[pOAM_OnuE1Info->E1PortTotalCount - 1].SrcMac, mac, 6);
	}

	return VOS_OK;
}

STATUS GetOnuE1Info(ULONG onuIndex, OnuE1Info *pOnuE1Info)
{
	onuE1Info onue1info;
	ULONG idxs[3], i;
	e1PortTable_t e1PortTable;

	if (NULL == pOnuE1Info)
	{
		E1_ERROR_INFO_PRINT(("GetOnuE1Info()::pOnuE1Info=NULL    error! \r\n"));
		return VOS_ERROR;
	}

	if (VOS_OK != onuDevIdxIsSupportE1(onuIndex))
	{
		E1_ERROR_INFO_PRINT(("GetOnuE1Info()::onuDevIdxIsSupportE1()    error! \r\n"));
		return VOS_ERROR;
	}

	memset((char *)pOnuE1Info, 0, sizeof(OnuE1Info));
	memset((char *)&onue1info, 0, sizeof(onuE1Info));

	idxs[0] = 1;
	idxs[1] = get_gfa_e1_slotno();

	if ( VOS_OK != tdm_getOnuE1Info( idxs, onuIndex, &onue1info ) )
	{
		E1_ERROR_INFO_PRINT(("GetOamOnuE1Info()::tdm_getOnuE1Info()    error! \r\n"));
		return VOS_ERROR;
	}

	idxs[0] = onuIndex;

	for (i = 0; i < onue1info.onuValidE1Count; i++)
	{
		idxs[1] = onue1info.onuEachE1Info[i].onuE1Slot;
		idxs[2] = onue1info.onuEachE1Info[i].onuE1Index;

		if ( VOS_OK != sw_e1PortTable_get( idxs, &e1PortTable ) )
		{
			E1_ERROR_INFO_PRINT(("GetOamOnuE1Info()::sw_e1PortTable_get()    error! \r\n"));
			continue;
		}

		memcpy( (char *)&(pOnuE1Info->onuEachE1Info[i].onuE1Slot), (char *)&(onue1info.onuEachE1Info[i].onuE1Slot), sizeof(OnuEachE1Info) );
		memcpy( (char *)&(pOnuE1Info->onuEachE1Info[i].onuE1PortAlarmStatus), (char *)&(e1PortTable.eponE1PortAlarmStatus), sizeof(e1PortTable_t) );

		pOnuE1Info->onuValidE1Count++;
	}

	return VOS_OK;
}



extern int MAX_E1_PORT_NUM;
uchar_t tdmService_Alarm_status[9][4][65]/* ={{{0}}}*/;	/*0-clear, 1-alarm*//* 20100201 */
ushort_t   tdm_MaskBase = 0;

int get_e1_port_alarm_status( UCHAR e1port, UCHAR *pStatus )
{
	if( (pStatus != NULL) && (e1port <= MAX_E1_PORT_NUM) )
	{
		*pStatus = (e1_Alarm_status(e1port) & (~e1_Alarm_Mask(e1port)));
		return VOS_OK;
	}
	return VOS_ERROR;
}
int get_e1_cluster_alarm_status( UCHAR e1clu, UCHAR *pStatus )
{
	int e1port;
	if( (pStatus != NULL) && (e1clu <= 3) )
	{
		if( e1clu == 0 )
			e1clu = 1;
		
		*pStatus = 0;
		for( e1port=(e1clu-1)*8+1; e1port<e1clu*8+1; e1port++ )
		{
			*pStatus |= (e1_Alarm_status(e1port) & (~e1_Alarm_Mask(e1port)));
		}
		return VOS_OK;
	}
	return VOS_ERROR;
}

extern char *alarm_e1_mask_to_str( UCHAR mask, char *str );
void showTdmAlarmStatus( struct vty * vty )	/* added by xieshl 20080311 */
{
	char str[32];
	int i = 0;
	for( i=1; i<=MAX_E1_PORT_NUM; i++ )
	{
		if( (e1_Alarm_status(i) & E1_ALM_ALL) != 0 )
		{
			vty_out( vty, "\r\nE1 %d alarm: %s", i, alarm_e1_mask_to_str(e1_Alarm_status(i), str) );
		}
	}
}

STATUS tdmAlarmInit()
{
	VOS_MemZero( (uchar_t *)&tdmService_Alarm_status[0][0][0], sizeof(tdmService_Alarm_status) );
	tdm_MaskBase = 0;/*add by shixh@20080202*/
	return VOS_OK;
}

/* 功能: 指定ONU语音业务中断告警状态更新上报，一般用于ONU离线或修改告警屏蔽时，
    对应告警的ONU重新产生告警恢复并通知网管 */
    /*modified by shixh@20080724*/
STATUS onuTdmServiceAlarmStatus_update( ULONG onuDevIdx )
{
	USHORT pon_slotno, pon_portno, onuno;
	pon_slotno = GET_PONSLOT(onuDevIdx)/*onuDevIdx / 10000*/;
	pon_portno = GET_PONPORT(onuDevIdx)/*(onuDevIdx % 10000) / 1000*/;
	onuno = GET_ONUID(onuDevIdx)/*onuDevIdx % 1000*/;

	if( (pon_slotno == 0) || (pon_slotno > PONCARD_LAST) ||(pon_portno == 0) || (pon_portno > PONPORTPERCARD) || (onuno == 0) || (onuno > 64) )
		return VOS_ERROR;
	
	if(SlotCardIsPonBoard(pon_slotno)!=ROK)
					return VOS_ERROR; 
		
	if( tdmService_Alarm_status[pon_slotno][pon_portno][onuno] == 1 )
	{
		tdmServiceAbortAlarmClear_EventReport( onuDevIdx );
		tdmService_Alarm_status[pon_slotno][pon_portno][onuno] = 0;
	}
	return VOS_OK;
}

/* 功能: 指定PON下所有ONU语音业务中断告警状态更新上报，一般用于GFA-PON板热拔插或修改
    告警屏蔽时，对应告警的ONU重新产生告警恢复并通知网管 */
STATUS tdmServiceAlarmStatus_update( ULONG pon_slotno, ULONG pon_portno )
{
	USHORT onuno, PonPortIdx;
	
	for( onuno=1; onuno<=64; onuno++ )
	{
		if( tdmService_Alarm_status[pon_slotno][pon_portno][onuno] == 1 )
		{
			PonPortIdx = GetPonPortIdxBySlot(pon_slotno-1, pon_portno-1 );
	
			if( ThisIsValidOnu(PonPortIdx, onuno-1) == ROK )
			{
				tdmServiceAbortAlarmClear_EventReport( MAKEDEVID(pon_slotno,pon_portno,onuno)/*pon_slotno *10000 + pon_portno *1000 + onuno */);
			}
			tdmService_Alarm_status[pon_slotno][pon_portno][onuno] = 0;
		}
	}
	return VOS_OK;
}

/* 功能: 指定OLT E1端口告警状态更新上报，一般用于GFA-SIG板热拔插或修改
    告警屏蔽时，对应告警的E1 重新产生告警恢复并通知网管 */
STATUS tdmPortAlarmStatus_update( ULONG tdm_slotno, ULONG  e1_portno )
{
	UCHAR i, alm_bit;
	ULONG trapId;
	for( i=0; i<8; i++ )
	{
		alm_bit = (1 << i);
		if( e1_Alarm_status(e1_portno) & alm_bit )
		{
			trapId = 0;
			switch( alm_bit )
			{
				case E1_ALM_LOS:
					trapId = trap_e1LosAlarmClear;
					break;
				case E1_ALM_LOF:
					trapId = trap_e1LofAlarmClear;
					break;
				case E1_ALM_AIS:
					trapId = trap_e1AisAlarmClear;
					break;
				case E1_ALM_RAI:
					trapId = trap_e1RaiAlarmClear;
					break;
				case E1_ALM_SMF:
					trapId = trap_e1SmfAlarmClear;
					break;
				case E1_ALM_LOFSMF:
					trapId = trap_e1LomfAlarmClear;
					break;
				case E1_ALM_CRC3:
					trapId = trap_e1Crc3AlarmClear;
					break;
				case E1_ALM_CRC6:
					trapId = trap_e1Crc6AlarmClear;
					break;
				default:
					break;
			}
			if( trapId != 0 )
				e1AlarmClearEventReport( 1, tdm_slotno, e1_portno, trapId, alm_bit );
		}
	}
	e1_Alarm_status(e1_portno) = 0;	

	return VOS_OK;
}

/* 功能: 指定槽位相关的所有语音业务告警状态更新上报，一般用于GFA-SIG/GFA-EPON板
    热拔插或修改告警屏蔽时，重新产生告警恢复并通知网管 */
/*modified by shixh@20080724*/
STATUS tdmBoardAlarmStatus_update( ULONG slotno )
{
	ULONG pon_slotno, pon_portno, tdm_slotno, e1_portno;
	
	if(SlotCardIsPonBoard(slotno)==ROK)
		{
			/* begin: added by jianght 20090327  */
			oltPonBoardPull(slotno);
			/* end: added by jianght 20090327 */

			pon_slotno = slotno;
			for( pon_portno=1; pon_portno<=PONPORTPERCARD; pon_portno++ )
			tdmServiceAlarmStatus_update( pon_slotno, pon_portno );
		}
	else if(SlotCardIsTdmSgBoard(slotno)==ROK)
		{
			tdm_slotno = get_gfa_tdm_slotno();
			if( tdm_slotno == slotno )
			{
				for( pon_slotno=PONCARD_FIRST; pon_slotno<=PONCARD_LAST; pon_slotno++ )
				{
					if( SlotCardIsPonBoard(pon_slotno) != ROK)
					continue;
					for( pon_portno=1; pon_portno<=PONPORTPERCARD; pon_portno++ )
					tdmServiceAlarmStatus_update( pon_slotno, pon_portno );
				}
			}
			else
				{
					VOS_ASSERT(0);
				}
			
			for( e1_portno=1; e1_portno<=MAX_E1_PORT_NUM; e1_portno++ )
			{
				tdmPortAlarmStatus_update( tdm_slotno, e1_portno );
			}
		}
	else if(SlotCardIsTdmE1Board(slotno) == ROK)
	{
		tdm_slotno = get_gfa_tdm_slotno();

		for( e1_portno=1; e1_portno<=MAX_E1_PORT_NUM; e1_portno++ )
		{
			tdmPortAlarmStatus_update( tdm_slotno, e1_portno );
		}

		oltE1BoardPull(slotno);
	}
	return VOS_OK;
}

/*added by wangysh at 2007-12-14*/

STATUS GetTdmMaskBase(ulong * pvar)
{
	STATUS func_ret = VOS_ERROR;
	*pvar = tdm_MaskBase;
	func_ret = VOS_OK;
	return func_ret;
}

/* modified by xieshl 20080408, #6484 */
/* mask_type=1设置E1告警屏蔽，mask_type=2设置语音ONU告警屏蔽，mask_type=3全部告警屏蔽 */
STATUS SetTdmMaskBase( ushort mask_type, ushort value)
{
	int i = 0;
	ulong idxs[3]={0};
	/*sys_console_printf("tdm_MaskBase=%x\r\n", tdm_MaskBase);*/
	if( mask_type & 1 )
	{
		UCHAR e1_m = ((value  & TDM_BASE_ALM_E1) >> 8);
		for( i=1; i<=MAX_E1_PORT_NUM; i++ )
		{
			idxs[0]=OLT_DEV_ID;
			idxs[1]=0;
			idxs[2]=i;
			
			/*sys_console_printf("\r\n set value=%x e1_m=%x m=%x\r\n", value, e1_m, e1_Alarm_Mask[i] );*/
			if( e1_m != e1_Alarm_Mask(i) )
			{
				if( SetE1AlarmMask(idxs, e1_m) == VOS_ERROR )
					break;
				/*VOS_TaskDelay(10);*/
			}
		}
		/*sys_console_printf("\r\n%d set value=%x add=%x or=%x\r\n",i, value, 
			( ~(value & TDM_BASE_ALM_E1) ), (value & TDM_BASE_ALM_E1) );*/
		if( i > MAX_E1_PORT_NUM )
		{
			tdm_MaskBase &= (~TDM_BASE_ALM_E1);
			tdm_MaskBase |= (value & TDM_BASE_ALM_E1);
			/*return VOS_OK;*/
		}
		else
		{
			sys_console_printf("\r\n SetTdmMaskBase value=0x%x err\r\n");
		}
	}
	if( mask_type & 2 )
	{
		tdm_MaskBase &= (~TDM_BASE_ALM_OOS);
		tdm_MaskBase |= (value &TDM_BASE_ALM_OOS);
		return VOS_OK;
	}
	return VOS_ERROR;
}

STATUS GetE1AlarmMask(ulong idx, uchar * pvalue)
{
	if( (idx != 0) && (idx <= MAX_E1_PORT_NUM) && (pvalue != NULL) )
	{
		*pvalue = e1_Alarm_Mask(idx);
		return VOS_OK;
	}
	return VOS_ERROR;
}

STATUS GetE1AlarmMask_tdm(ulong idx, uchar * pvalue)/*modified by shixh@20080220*/
{
	e1porttable_row_entry   pEntry;
	ULONG idxs[3];
	 idxs[0]=1;
	 idxs[1]=get_gfa_sg_slotno();
	 idxs[2]=idx;
	 
 
	if( (idx != 0) && (idx <= MAX_E1_PORT_NUM) && (pvalue != NULL) )
	{
	if(tdm_e1portTable_get(idxs, &pEntry)!=VOS_OK)
		*pvalue=pEntry.almmask;
		return VOS_OK;
	}
	return VOS_ERROR;
}

STATUS SetE1AlarmMask_sw(ulong idx,uchar value)/*设置本地*/
{
	if( (idx != 0) && (idx <= MAX_E1_PORT_NUM) )
	{
		e1_Alarm_Mask(idx)  = value;
		return VOS_OK;
	}
	return VOS_ERROR;
}

STATUS SetE1AlarmMask_tdm(ulong idx,uchar value)/*设置TDM 板*/
{
/* modified by chenfj 2009-4-8*/
	if(get_gfa_sg_slotno())
	{
		ULONG idxs[3];
		idxs[0]=1;
		idxs[1]=get_gfa_sg_slotno();
		idxs[2]=idx;

		if( (idx != 0) && (idx <= MAX_E1_PORT_NUM) )
		{
			return tdm_e1portTable_set( 7, idxs, value );
		}
		return VOS_ERROR;
	}
	else if(get_gfa_e1_slotno())
	{
		ULONG idxs[3];
		idxs[0]=1;
		idxs[1]=get_gfa_e1_slotno();
		idxs[2]=idx;

		if( (idx != 0) && (idx <= MAX_E1_PORT_NUM) )
		{
			return e1PortTable_set( 2, idxs, value );
		}
		return VOS_ERROR;
	}
	else return VOS_ERROR;
}

STATUS SetE1AlarmMask(ulong idx[3],uchar value) /*同时设置本地和TDM板*/
{
	if(idx[0]==OLT_DEV_ID)
	{
		if( SetE1AlarmMask_tdm( idx[2], value) != VOS_OK )
			return VOS_ERROR;

		e1_Alarm_Mask(idx[2])  = value;
	
		return VOS_OK;
	}
	else/*输入参数是onu的情况*/
	{
		return VOS_OK;
	}
	return(VOS_ERROR);
}
/*end*/


/* 功能:   获取e1告警级别
   输入参数:devIdx－OLT设备索引，取值为1
   			     level -获取的告警级别
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
extern LONG getAlarmLevel( ULONG idx, ULONG *level );
/*add by shixh@20090325*/
extern  e1PortTable_t  tdmE1PortTable[24];
extern ULONG get_gfa_tdm_slotno();
STATUS  GetE1AlarmLevel_E1(ULONG  devIdx,ULONG  *level)
{
	int i,k=0;
	ULONG alarm_level=5;
	ULONG temp;
	
	UCHAR e1_alarmIdList[] = { 0,
		trap_e1LosAlarm,		       
	       trap_e1LofAlarm,
	       trap_e1AisAlarm,
	       trap_e1RaiAlarm,				
	       trap_e1SmfAlarm,			
	       trap_e1LomfAlarm,			
	       trap_e1Crc3Alarm,			
	       trap_e1Crc6Alarm,
	       0
	};
	if(level==NULL)
		return VOS_ERROR;
	if( devIdx == 1 )
	{
		for( i=1; i<=MAX_E1_PORT_NUM; i++ )
		{
			if( tdmE1PortTable[i].eponE1PortAlarmStatus!= 0 )
			{
				for( k=1; k<8; k++ )
				{
					if( (tdmE1PortTable[i].eponE1PortAlarmStatus& (~e1_Alarm_Mask(i))) & (1 << (8-k)) )
					{
						temp = 5;
						getAlarmLevel( e1_alarmIdList[k], &temp );
						if( temp < alarm_level )
							alarm_level = temp;
						if(alarm_level==1)
						{
							*level=alarm_level;
							return VOS_OK;
						}
					}
				}
			}
		}
	}
	*level=alarm_level;
	return VOS_OK;

}
STATUS  GetE1AlarmLevel(ULONG  devIdx,ULONG  *level)
{	/*  modified by chenfj 2009-4-8 */
	ULONG  slotno;  
	if(devIdx==1)
	{
		slotno=get_gfa_tdm_slotno();
		
		if(SlotCardIsTdmSgBoard(slotno)==ROK)
			return(GetE1AlarmLevel_sig(devIdx,level));
		else if(SlotCardIsTdmE1Board(slotno)==ROK)
			return(GetE1AlarmLevel_E1(devIdx,level));
		return(VOS_ERROR);
	}
	else
		return(GetE1AlarmLevel_ONU_E1(devIdx,level));

}
STATUS  GetE1AlarmLevel_ONU_E1(ULONG  devIdx,ULONG  *level)
{
	onu_ext_brd_table_t    BrdInfo_buf;
	int i,k=0;
	ULONG alarm_level=5;
	ULONG temp;
	
	UCHAR e1_alarmIdList[] = { 0,
		trap_e1LosAlarm,		       
	       trap_e1LofAlarm,
	       trap_e1AisAlarm,
	       trap_e1RaiAlarm,				
	       trap_e1SmfAlarm,			
	       trap_e1LomfAlarm,			
	       trap_e1Crc3Alarm,			
	       trap_e1Crc6Alarm,
	       0
	};
	
	if(level==NULL)
		return VOS_ERROR;
	if( devIdx == 1 )
	{
		for( i=1; i<=MAX_E1_PORT_NUM; i++ )
		{
			if( BrdInfo_buf.onuE1PortTable[i].eponE1PortAlarmStatus!= 0 )
			{
				for( k=1; k<8; k++ )
				{
					if( ( BrdInfo_buf.onuE1PortTable[i].eponE1PortAlarmStatus& (~e1_Alarm_Mask(i))) & (1 << (8-k)) )
					{
						temp = 5;
						getAlarmLevel( e1_alarmIdList[k], &temp );
						if( temp < alarm_level )
							alarm_level = temp;
						if(alarm_level==1)
						{
							*level=alarm_level;
							return VOS_OK;
						}
					}
				}
			}
		}
	}
	*level=alarm_level;
	return VOS_OK;

}
STATUS  GetE1AlarmLevel_sig(ULONG  devIdx,ULONG  *level)
{
	int i,k=0;
	ULONG alarm_level=5;
	ULONG temp;
	
#if 0	/* modified by xieshl 20090212 这样处理是错误的，会导致设备告警状态误报 */
	if(level==NULL)
		return VOS_ERROR;
  
	for( i=1; i<=MAX_E1_PORT_NUM; i++ )
	{
		if( e1_Alarm_status[i] != 0 )
		{
			for( k=1; k<8; k++ )
			{
				if( e1_Alarm_status[i] & (1 << (8-k)) )
				{
					temp = 5;
					getAlarmLevel( k, &temp );

					if( temp < alarm_level )
						alarm_level = temp;
					if(alarm_level==1)
					{
						*level=alarm_level;
						return VOS_OK;
					}
				}
			}
		}
	}
#else
	UCHAR e1_alarmIdList[] = { 0,
		trap_e1LosAlarm,		       
	       trap_e1LofAlarm,
	       trap_e1AisAlarm,
	       trap_e1RaiAlarm,				
	       trap_e1SmfAlarm,			
	       trap_e1LomfAlarm,			
	       trap_e1Crc3Alarm,			
	       trap_e1Crc6Alarm,
	       0
	};

	if(level==NULL)
		return VOS_ERROR;
	if( devIdx == 1 )
	{
		for( i=1; i<=MAX_E1_PORT_NUM; i++ )
		{
			if( e1_Alarm_status(i) != 0 )
			{
				for( k=1; k<8; k++ )
				{
					/*if( e1_Alarm_status[i] & (1 << (8-k)) )*/
					if( (e1_Alarm_status(i) & (~e1_Alarm_Mask(i))) & (1 << (8-k)) )
					{
						temp = 5;
						getAlarmLevel( e1_alarmIdList[k], &temp );

						if( temp < alarm_level )
							alarm_level = temp;
						if(alarm_level==1)
						{
							*level=alarm_level;
							return VOS_OK;
						}
					}
				}
			}
		}
	}
#endif
	*level=alarm_level;
	return VOS_OK;
   
}

/* 功能:   获取tdm相连onu离线的告警级别
   输入参数:devIdx－OLT设备索引
   			     level -获取的告警级别
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
STATUS GetTdmServiceAbortAlarmLevel(ULONG  devIdx ,ULONG *level)
{
    uchar_t slot, pon, onu;
    if( devIdx != 1 )
 	{
 		/*slot = devIdx / 10000;
		pon = (devIdx % 10000) / 1000;
		onu = devIdx % 1000;*/
              slot=GET_PONSLOT(devIdx);
              pon=GET_PONPORT(devIdx);
              onu=GET_ONUID(devIdx);
              

		if( SYS_SLOTNO_IS_ILLEGAL(slot) || (pon == 0) || (pon > 4) || (onu == 0) || (onu > 64) )
		{
			/*VOS_ASSERT(0);*/
			return VOS_ERROR;
		}
		if( __SYS_MODULE_TYPE__(slot) != MODULE_E_GFA_EPON )
		{
			/*VOS_ASSERT(0);*/
			return VOS_ERROR;
		}
		
 		if(tdmService_Alarm_status[slot][pon][onu] != 0)
 		{
			getAlarmLevel( trap_tdmServiceAbortAlarm, level );
 		}
 	}
	return  VOS_OK;
}

/* 功能:  设置E1的告警级别
   输入参数:E1_alarm:E1_ALM_LOS (0x0080)	e1信号丢失	
                                         E1_ALM_LOF (0x0040)   e1桢丢失
                                         E1_ALM_AIS(0x0020)  e1本端告警指示
                                         E1_ALM_RAI(0x0010)  e1对端告警指示
                                         E1_ALM_SMF(0x0008)  e1信令复桢失步告警
                                         E1_ALM_LOFSMF(0x0004)  e1 crc复桢失步告警
                                         E1_ALM_CRC3(0x0002)  e1 crc-3误码
                                         E1_ALM_CRC6(0x0001)  e1 crc-6误码
                                         tdmService_Alarm(0x0100)tdm相连onu离线的告警
			     level -设置的告警级别
   返回值:  成功－VOS_OK，错误－VOS_ERROR */	
/*STATUS SetE1AlarmLevel(ULONG E1_alarm,ULONG  level)
{
       if(E1_alarm & E1_ALM_LOS)
		e1_Alarm_Level[0]=level;
	if(E1_alarm & E1_ALM_LOF)
		e1_Alarm_Level[1]=level;
	if(E1_alarm & E1_ALM_AIS)
		e1_Alarm_Level[2]=level;
	if(E1_alarm & E1_ALM_RAI)
		e1_Alarm_Level[3]=level;
	if(E1_alarm & E1_ALM_SMF)
		e1_Alarm_Level[4]=level;
	if(E1_alarm & E1_ALM_LOFSMF)
		e1_Alarm_Level[5]=level;
	if(E1_alarm & E1_ALM_CRC3)


		e1_Alarm_Level[6]=level;
	if(E1_alarm & E1_ALM_CRC6)
		e1_Alarm_Level[7]=level;
	if(E1_alarm & TDM_ALM_OOS)
		e1_TdmServiceAbortAlarm_level=level;
	
	return  VOS_OK;
}*/

LONG getOnuTdmServiceAlarmStatus( ULONG devIdx, ULONG *pStatus )
{
	if( devIdx != 1 )
	{
		ULONG slot = GET_PONSLOT(devIdx)/*devIdx / 10000*/;
		ULONG pon = GET_PONPORT(devIdx)/*(devIdx % 10000) /1000*/;
		ULONG onu = GET_ONUID(devIdx)/*devIdx % 1000*/;
		if( SYS_SLOTNO_IS_ILLEGAL(slot) || (pon == 0) || (pon > PONPORTPERCARD) || (onu == 0) || (onu > MAXONUPERPON) )
		{
			return VOS_ERROR;
		}
		if( SlotCardIsPonBoard(slot) ==ROK )
		{
			*pStatus = tdmService_Alarm_status[slot][pon][onu];
			return VOS_OK;
		}
 	}
	return VOS_ERROR;
}

LONG setOnuTdmServiceAlarmStatus( ULONG devIdx, ULONG status )
{
	if( devIdx != 1 )
	{
		ULONG slot = GET_PONSLOT(devIdx)/*devIdx / 10000*/;
		ULONG pon = GET_PONPORT(devIdx)/*(devIdx % 10000) /1000*/;
		ULONG onu = GET_ONUID(devIdx)/*devIdx % 1000*/;
		if( SYS_SLOTNO_IS_ILLEGAL(slot) || (pon == 0) || (pon > PONPORTPERCARD) || (onu == 0) || (onu > MAXONUPERPON) )
		{
			return VOS_ERROR;
		}
		if( SlotCardIsPonBoard(slot) ==ROK )
		{
			if( status != tdmService_Alarm_status[slot][pon][onu] )
			{
				tdmService_Alarm_status[slot][pon][onu] = status;
				return VOS_OK;
			}
		}
 	}
	return VOS_ERROR;
}

#endif

