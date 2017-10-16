
#ifndef _E1_OAM_H_
#define _E1_OAM_H_


#define OAM_E1_ENABLE			    1
#define OAM_E1_DISABLE		        0
#define OAM_E1_VLAN_ENABLE			1
#define OAM_E1_VLAN_DISABLE			0

#define E1_CIRCUIT_LOOP         1 /* 线路环回 */
#define E1_SYSTEM_LOOP          2 /* 系统环回 */
#define E1_ALL_LOOP             3 /* 双向环回 */
#define E1_NO_LOOP              4 /* 不环回 */

#define MAX_ONU_E1    4 * 5 /* 每块E1板最多4路E1 */


#define  V2R1_E1_OK            0
#define  V2R1_E1_ERROR       (-1)
#define  V2R1_E1_ONU_ONLINE    1
#define  V2R1_E1_ONU_OFFLINE   2

#define  V2R1_E1_ONU_COMM_SUCCESS  1
#define  V2R1_E1_ONU_COMM_FAIL  2


/* 配置ONU E1连接 */
#define  SET_ONU_E1_LINK_REQ   120
#define  SET_ONU_E1_LINK_RSP   120

/* 配置ONU E1 Vlan */
#define  SET_ONU_E1_VLAN_REQ   121
#define  SET_ONU_E1_VLAN_RSP   121

/* 配置ONU E1环回 */
#define  SET_ONU_E1_LOOP_REQ   122
#define  SET_ONU_E1_LOOP_RSP   122

/* 配置ONU E1时钟 */
#define  SET_ONU_E1_CLOCK_REQ   123
#define  SET_ONU_E1_CLOCK_RSP   123

/* 配置ONU E1告警屏蔽 */
#define  SET_ONU_E1_ALARMMASK_REQ   124
#define  SET_ONU_E1_ALARMMASK_RSP   124

/* 配置ONU E1所有信息 */
#define  SET_ONU_E1_ALL_REQ   125
#define  SET_ONU_E1_ALL_RSP   125

/* 取得ONU E1所有信息 */
#define  GET_ONU_E1_ALL_REQ   126
#define  GET_ONU_E1_ALL_RSP   126



/* ONU OAM */

typedef struct {
	
	unsigned char  reserve3;/* Byte 3 */
	unsigned char  reserve2;
	unsigned char  E1SlotIdx;
	unsigned char  E1PortIdx;/* Byte 0 */

	unsigned char  E1Enable;
	unsigned char  DesMac[6];
	unsigned char  SrcMac[6];
	unsigned char  VlanEable;
	unsigned short int VlanTag;
}__attribute__((packed)) OAM_OnuE1Link_t;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned char E1PortTotalCount;/* 一次配置E1业务中需要操作的E1端口数 */
	OAM_OnuE1Link_t oam_OnuE1Link[MAX_ONU_E1];
}__attribute__((packed)) OAM_OnuE1Link;


typedef struct {
	unsigned char  reserve3;/* Byte 3 */
	unsigned char  reserve2;
	unsigned char  E1SlotIdx;
	unsigned char  E1PortIdx;/* Byte 0 */

	unsigned char  VlanEable;
	unsigned short int VlanTag;
}__attribute__((packed)) OAM_OnuE1Vlan_t;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned char E1PortTotalCount;/* 一次配置E1业务中需要操作的E1端口数 */
	OAM_OnuE1Vlan_t oam_OnuE1Vlan[MAX_ONU_E1];
}__attribute__((packed)) OAM_OnuE1Vlan;


typedef struct {
	unsigned char MsgType;
	unsigned char Result;

	unsigned char  E1PortIdx;/* Byte 0 */
	unsigned char  E1SlotIdx;
	unsigned char  none2;
	unsigned char  none3;/* Byte 3 */

	unsigned char  LoopControl;
}__attribute__((packed)) OAM_OnuE1Loop;


typedef struct {
	unsigned char  reserve3;/* Byte 3 */
	unsigned char  reserve2;
	unsigned char  E1SlotIdx;
	unsigned char  E1PortIdx;/* Byte 0 */

	unsigned char  ClockControl;/* 0-3 */
}__attribute__((packed)) OAM_OnuE1Clock_t;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned char E1PortTotalCount;/* 一次配置E1业务中需要操作的E1端口数 */
	OAM_OnuE1Clock_t oam_OnuE1Clock[MAX_ONU_E1];
}__attribute__((packed)) OAM_OnuE1Clock;


typedef struct {
	unsigned char  reserve3;/* Byte 3 */
	unsigned char  reserve2;
	unsigned char  E1SlotIdx;
	unsigned char  E1PortIdx;/* Byte 0 */

	unsigned short AlarmMask;
}__attribute__((packed)) OAM_OnuE1AlarmMask_t;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned char E1PortTotalCount;/* 一次配置E1业务中需要操作的E1端口数 */
	OAM_OnuE1AlarmMask_t oam_OnuE1AlarmMask[MAX_ONU_E1];
}__attribute__((packed)) OAM_OnuE1AlarmMask;


typedef struct {
	unsigned char  reserve3;/* Byte 3 */
	unsigned char  reserve2;
	unsigned char  E1SlotIdx;
	unsigned char  E1PortIdx;/* Byte 0 */

	unsigned char  E1Enable;
	unsigned char  DesMac[6];
	unsigned char  SrcMac[6];
	unsigned char  VlanEable;
	unsigned short int VlanTag;
	unsigned char  ClockControl;/* 0-3 */
	unsigned char  LoopControl;
	unsigned short int AlarmStat;
	unsigned short int AlarmMask;
}__attribute__((packed)) OAM_OnuE1Info_t;

typedef struct {
	unsigned char MsgType;
	unsigned char Result;
	unsigned char E1PortTotalCount;/* 一次配置E1业务中需要操作的E1端口数 */
	OAM_OnuE1Info_t oam_OnuE1Info[MAX_ONU_E1];
}__attribute__((packed)) OAM_OnuE1Info;


extern STATUS SetOnuE1Link(unsigned long onuDevIdx, OAM_OnuE1Link *pOAM_OnuE1Link);
extern STATUS SetOnuE1Vlan(unsigned long onuDevIdx, OAM_OnuE1Vlan *pOAM_OnuE1Vlan);
extern STATUS SetOnuE1Loop(unsigned long onuDevIdx, OAM_OnuE1Loop *pOAM_OnuE1Loop);
extern STATUS SetOnuE1Clock(unsigned long onuDevIdx, OAM_OnuE1Clock *pOAM_OnuE1Clock);
extern STATUS SetOnuE1AlarmMask(unsigned long onuDevIdx, OAM_OnuE1AlarmMask *pOAM_OnuE1AlarmMask);
extern STATUS SetOnuE1All(unsigned long onuDevIdx, OAM_OnuE1Info *pOAM_OnuE1Info);/* 用于重新注册 */
extern STATUS GetOnuE1All(unsigned long onuDevIdx, OAM_OnuE1Info *pOAM_OnuE1Info);

#endif
