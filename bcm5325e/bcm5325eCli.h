#ifndef BCM5325ECLI_H
#define BCM5325ECLI_H

#ifdef __cplusplus
extern "C" {
#endif

/*typedef unsigned char mac_address_t [6];*/
/****************************************
* BrdChannelBcm5325eInit
* description: 初始化bcm5325e vlan功能,设置所有pon卡
*	mac地址,设置所有槽位对应的bcm5325e端口隔离
*
*
*
******************************************/
int BrdChannelBcm5325eInit(void);


/******************************************
*BrdChannelMasterMacSet
* description: 该函数用于在板间通信的bcm5325e中增加
*	主控卡本版的mac地址,对应bcm5325e的端口为bcm port=5
*	1)当slot 4主控板时,输入本版主控板的参数为:ulSlot = 4,mac为本版
*	主控板的mac地址;
*	2)当slot 3为主控板时,输入的本版主控板的参数为:ulSlot = 3,mac为
*	本版主控板的mac地址;
*
* input :
*	ulSlot - #define BCM_BRD_MASTER_SLOT		3
*			#define BCM_BRD_MASTER_BACK_SLOT	4
*	mac	- mac地址
********************************************/
int BrdChannelMasterMacSet(ULONG ulSlot, mac_address_t mac);

/******************************************
*BrdChannelMasterBackMacSet
* description: 该函数用于在板间通信的bcm5325e中增加
*	对方主控卡本版的mac地址,对应bcm5325e的端口为bcm port=BCM_MASTER_BACK_BROAD_PORT
*	1)当slot 4主控板时,输入对方主控板的参数为:ulSlot = 3,mac为对方
*	主控板的mac地址;
*	2)当slot 3为主控板时,输入的对方主控板的参数为:ulSlot = 4,mac为
*	对方主控板的mac地址;
*
* input :
*	ulSlot - #define BCM_BRD_MASTER_SLOT		3
*			#define BCM_BRD_MASTER_BACK_SLOT	4
*	mac	- mac地址
*
********************************************/
int BrdChannelMasterBackMacSet(ULONG ulSlot, mac_address_t mac);

LONG BCM5325E_DEBUG_CommandInstall(void);

#ifdef __cplusplus
}
#endif

#endif	/*_BCM5325EHEADER_H_*/	
