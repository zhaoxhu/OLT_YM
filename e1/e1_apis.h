

#ifndef _E1_APIS_H_
#define _E1_APIS_H_

#define E1_OOS                  0x52
#define E1_OOS_RECOV            0x53

#ifndef	PACKED
#define	PACKED	__attribute__((packed))
#endif


enum
{
	E1LINKTABLE_INDEX = 10,
	E1PORTTABLE_INDEX_new,
	E1VLANTABLE_INDEX
};

/*E1管理API返回值定义*/
enum{
	E1_VM_ERR = -1,
	E1_VM_OK,
	E1_VM_VID_EXIST
};

#define E1_DESCRIPTION_MAX      63/* 还有1字节存放'\0' */

typedef struct
{
	UCHAR  devIdx;/*OLT的物理索引号，默认就是1*/
	UCHAR  brdIdx;/* tdm板的槽位号 */
	UCHAR  e1LinkIdx;/* e1表的索引号 */

	ULONG  onuDevId;
	UCHAR  onuE1SlotId;
	UCHAR  onuE1Id;

	UCHAR  eponE1LocalEnable;
	UCHAR  eponE1Description[E1_DESCRIPTION_MAX + 1];
}PACKED e1LinkTable_row_entry;

typedef struct
{
	UCHAR  devIdx;/*OLT的物理索引号，默认就是1*/
	UCHAR  brdIdx;/* tdm板的槽位号 */
	UCHAR  e1portIdx;/* e1表的索引号 */

	USHORT eponE1PortAlarmStatus;
	USHORT eponE1PortAlarmMask;
	UCHAR  eponE1Loop;
	UCHAR  eponE1TxClock;
}PACKED e1PortTable_row_entry;

typedef struct
{
	UCHAR  devIdx;/*OLT的物理索引号，默认就是1*/
	UCHAR  brdIdx;/* tdm板的槽位号 */
	UCHAR  vlanIdx;/* vlan表的索引号 */

	UCHAR  eponVlanEnable;
	UCHAR  eponVlanPri;
	USHORT eponVlanId;
}PACKED e1VlanTable_row_entry;



extern STATUS tdm_e1LinkTable_get( ULONG *idxs, e1LinkTable_row_entry *pEntry );
extern STATUS tdm_e1LinkTable_getNext( ULONG *idxs, e1LinkTable_row_entry *pEntry );
extern STATUS tdm_e1LinkTable_set( UCHAR leafIdx, ULONG *idxs, ULONG setval , UCHAR *description );
extern STATUS tdm_e1LinkTable_rowset( ULONG  *idxs,
									  ULONG  onuDevId,
									  UCHAR  onuE1SlotId,
									  UCHAR  onuE1Id,
									  UCHAR  eponE1LocalEnable,
									  UCHAR  *eponE1Description );


extern STATUS tdm_e1PortTable_get( ULONG *idxs, e1PortTable_row_entry *pEntry );
extern STATUS tdm_e1PortTable_getNext( ULONG *idxs, e1PortTable_row_entry *pEntry );
extern STATUS tdm_e1PortTable_set( UCHAR leafIdx, ULONG *idxs, USHORT setval );
extern STATUS tdm_e1PortTable_rowset( ULONG  *idxs,
									  USHORT  eponE1PortAlarmMask,
									  UCHAR  eponE1Loop,
									  UCHAR  eponE1TxClock );


extern STATUS tdm_e1VlanTable_get( ULONG *idxs, e1VlanTable_row_entry *pEntry );
extern STATUS tdm_e1VlanTable_getNext( ULONG *idxs, e1VlanTable_row_entry *pEntry );
extern STATUS tdm_e1VlanTable_set( UCHAR leafIdx, ULONG *idxs, USHORT setval );
extern STATUS tdm_e1VlanTable_rowset( ULONG  *idxs,
									  UCHAR  eponVlanEnable,
									  UCHAR  eponVlanPri,
									  USHORT eponVlanId );

extern void init_e1_process_function(void);

/* 系统支持的最大信令网关接口数 */
#define MAX_SGIF_ID			3
#define MAX_SGIF_E1			8

/* 特殊处理消息 */
#define SUBMSG_ONU_E1_REQUEST             0x50
#define SUBMSG_ALL_E1_REQUEST             0x51
#define SUBMSG_MAX_FRAME_GAP_REQUEST  0x52
#define SUBMSG_SET_ETH2E1_BUF_REQUEST  0x53

#define SUBMSG_ONU_E1_RESPONSE            0x60
#define SUBMSG_ALL_E1_RESPONSE            0x61
#define SUBMSG_MAX_FRAME_GAP_RESPONSE  0x62
#define SUBMSG_SET_ETH2E1_BUF_RESPONSE  0x63

/* 一个ONU配置的E1信息结构体 */
typedef struct
{
	UCHAR  onuE1Slot;
	UCHAR  onuE1Index;
	UCHAR  tdmE1Enable;
	UCHAR  tdmSgIfId;/*0-2*/
	UCHAR  tdmE1Id;/*0-7*/
	UCHAR  tdmVlanEnable;
	USHORT tdmVlanTag;
	USHORT tdmAlarmStat;
	USHORT tdmAlarmMask;
	UCHAR  tdmE1Loop;
	UCHAR  tdmE1TxClk;
}PACKED OnuEachE1Info;

typedef struct
{
	UCHAR  onuValidE1Count;/* 说明onuEachE1Info[]有效项 */
	OnuEachE1Info onuEachE1Info[MAX_SGIF_E1 * MAX_SGIF_ID];
}PACKED onuE1Info;

extern STATUS tdm_getOnuE1Info( ULONG *idxs, ULONG onuIndex, onuE1Info *pOnuE1Info );

/* 查询所有配置过的E1信息 47*/
typedef struct
{
	UCHAR  e1Index;/* 0-7 */

	ULONG  onuDevId;
	UCHAR  onuE1SlotId;
	UCHAR  onuE1Id;
	UCHAR  eponE1LocalEnable;
	/*UCHAR  eponE1Description[E1_DESCRIPTION_MAX + 1];*/
	USHORT eponE1PortAlarmStatus;
	USHORT eponE1PortAlarmMask;
	UCHAR  eponE1Loop;
	UCHAR  eponE1TxClock;
}PACKED EachE1Info;/* 46 */

typedef struct
{
	UCHAR  fpgaIndex;
	UCHAR  eponVlanEnable;
	UCHAR  eponVlanPri;
	USHORT eponVlanId;
	UCHAR  fpgaValidE1Count;/* 说明eachE1Info[]有效项 */
	EachE1Info eachE1Info[MAX_SGIF_E1];
}PACKED FpgaE1Info;/* 374  */

typedef struct
{
	UCHAR      type;/* 查询类别若为0-2, 说明返回一片fpga上的配置过的E1信息；若为0xFF，则返回3片fpga上配置过的E1信息 */
	FpgaE1Info fpgaE1Info[MAX_SGIF_ID];
}PACKED AllE1Info;/* 1123 */

extern STATUS tdm_getAllE1Info( ULONG *idxs, UCHAR type, AllE1Info *pAllE1Info );

extern STATUS tdm_getMaxFrameGap(ULONG *idxs, char *maxFrmGapArray);
extern STATUS tdm_setEth2E1TxBuffer(ULONG *idxs, char *eth2E1TxBufRegArray);

#endif














