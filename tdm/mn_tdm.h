#ifndef __INCmn_tdmh
#define __INCmn_tdmh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


enum{
	LEAF_EPONE1PORTDEVINDEX=1,		
	LEAF_EPONE1PORTBRDINDEX,	
	LEAF_EPONE1PORTPORTINDEX,	
	LEAF_EPONE1PORTE1CLUSTERINDEX,
	LEAF_EPONE1PORTFUNCTION,
	LEAF_EPONE1PORTALARMSTATUS,
	LEAF_EPONE1PORTALARMMASK,	
	LEAF_EPONE1PORTCRCENABLE	,
	LEAF_EPONE1PORTLOOPBACKCTRL,
	LEAF_EPONE1PORTCODE,
	LEAF_EPONE1PORTIMPEDANCE,
	LEAF_EPONTDME1_MAX
};

enum {
	LEAF_EPONTDMONUSGIFINDEX = 1,
	LEAF_EPONTDMONUDEVINDEX,
	LEAF_EPONTDMONULOGICALONUINDEX,
	LEAF_EPONTDMONUSERVICESTATUS,
	LEAF_EPONTDMONUROWSTATUS,
	LEAF_EPONTDMONUPOTSLOOPBACKCTRL,
	LEAF_EPONTDMONULINKEDPONBOARDINDEX,
	LEAF_EPONTDMONULINKEDPONPORTINDEX,
	LEAF_EPONTDMONULINKEDONUINDEX,
	LEAF_EPONTDMONUNAME,
	LEAF_EPONTDMONU_MAX
};

enum{
	LEAF_EPONPOTSLINKSGIFINDEX=1,
	LEAF_EPONPOTSLINKSGPORTINDEX,
	LEAF_EPONPOTSLINKONUDEV,
	LEAF_EPONPOTSLINKONUBOARD,
	LEAF_EPONPOTSLINKONUPOTS,
	LEAF_EPONPOTSLINKPHONECODE,
	LEAF_EPONPOTSLINKDESC,
	LEAF_EPONPOTSLINKROWSTATUS,
	LEAF_EPONPOTSLINKPONBOARDINDEX,
	LEAF_EPONPOTSLINKPONPORTINDEX,
	LEAF_EPONPOTSLINKONUINDEX,
	LEAF_EPONPOTSLINKONUNAME,
	LEAF_EPONPOTSLINK_MAX

};

enum{
	LEAF_EPONSGIFINDEX	= 1,
	LEAF_EPONSGE1CLUSTERINDEX,
	LEAF_EPONSGHDLCMASTERE1 = 3,
	LEAF_EPONSGHDLCSLAVEE1,
	LEAF_EPONSGHDLCE1CLK,
	LEAF_EPONSGVLANENABLE,
	LEAF_EPONSGVLANID,
	LEAF_EPONSGVLANPRI,
	LEAF_EPONSG_MAX
};
extern int deleteOnuPotsLinkFromSgByLogicPort( ULONG sgIndex, ULONG logicPort );

/*语音管理API声明*/


/*******************************************************************************************
函数名：SetTdmSGVoiceVlanEnable
功能：	使能TDM上语音VLAN
输入：	TdmSlot -- TDM板所在的槽位，槽位号的编排按GFA6700产品规定
		SgIdx -- 信令网关索引，取值 1－3
		Priority -- 语音VLAN优先级
		VlanId -- 语音VLANID
输出：
返回值：
*******************************************************************************************/
extern int SetTdmSGVoiceVlanEnable(unsigned char TdmSlot,unsigned char SgIdx, unsigned char Priority, unsigned short int VlanId);
extern int SetTdmSGVoiceVlanDisable(unsigned char TdmSlot,unsigned char SgIdx);
extern int GetTdmSGVoiceVlanEnable(unsigned char TdmSlot,unsigned char SgIdx, unsigned char *enableFlag, unsigned char *Priority, unsigned short int *VlanId);

extern int SetSWVoiceVlanEnable(unsigned char TdmSlot ,unsigned char SgIdx, unsigned short int Vid, unsigned char priority);
extern int SetSWVoiceVlanDisable(unsigned char TdmSlot ,unsigned char SgIdx, unsigned short int Vid);
extern int GetSWVoiceVlanEnable(unsigned char TdmSlot ,unsigned char SgIdx, unsigned char *enableFlag, unsigned char *Priority, unsigned short int *VlanId, 
							 unsigned char *PortNum, unsigned char *PortList);
extern int SetOnuVoiceVlanEnable(unsigned long OnuDeviceIdx,unsigned short int vid, unsigned char priority);
extern int SetOnuVoiceVlanDisable(unsigned long OnuDeviceIdx);
extern int GetOnuVoiceVlanEnable(unsigned long OnuDeviceIdx, unsigned char *EnableFlag , unsigned char *Priority, unsigned short int *VlanId );							

/*语音ONU配置管理API*/
extern int AddOnuToSG(unsigned char TdmSlot, unsigned char SgIdx, unsigned long OnuDeviceIdx, unsigned short int *logicOnu);
/*int DeleteOnuFromSG( unsigned long OnuDeviceIdx );*/
extern int DeleteAllOnuFromSG(unsigned char TdmSlot ,unsigned char SgIdx);
extern int GetOnuBelongToSG(unsigned long OnuDeviceIdx, unsigned char *TdmSlot ,unsigned char *SgIdx,unsigned short int *LogicOnuId);
extern int GetAllOnuBelongToSG(unsigned char TdmSlot,unsigned char SgIdx, unsigned long*OnuNumber, unsigned long *OnuDeviceIdx);

/*POTS端口语音连接设置API*/
extern int AddOnuPotsLinkToSg(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort, unsigned long OnuDeviceIdx, 
						unsigned char Potsboard, unsigned char PotsPort);
extern int DeleteOnuPotsLinkFromSg(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort);
extern int GetOnuPotsLinkByDevIdx(unsigned long OnuDeviceIdx, unsigned char PotsBoard, unsigned char PotsPort, BOOL *EnableFlag, unsigned char *TdmSlot, 
						unsigned char *SgIdx, unsigned short int *LogicPort);
extern int GetOnuPotsLinkBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort, 
							BOOL *EnableFlag, unsigned long *OnuDeviceIdx, unsigned char *PotsBoard, unsigned char *PotsPort);
extern int SetOnuPotsLinkPhoneNumByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *PhonuNumber);
extern int SetOnuPotsLinkPhoneNumBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char *PhonuNumber);

extern int GetOnuPotsLinkPhoneNumByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *PhonuNumber);
extern int GetOnuPotsLinkPhoneNumBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char *PhonuNumber);
extern int SetOnuPotsLinkDescByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *Description );
extern int SetOnuPotsLinkDescBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char * Description );
extern int GetOnuPotsLinkDescBySgLogicPort(unsigned char TdmSlot, unsigned char SgIdx, unsigned short int LogicPort,unsigned char * Description);
extern int GetOnuPotsLinkDescByDevIdx(unsigned long OnuDeviceIdx, unsigned char Potsboard, unsigned char PotsPort,unsigned char *Description );

/* added by xieshl 20080407 */
extern int mn_tdmOnuVoiceLoopbackStart( ULONG *idxs );
extern int mn_tdmOnuVoiceLoopbackStop( ULONG *idxs );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCmn_tdmh */
