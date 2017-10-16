#ifndef BCM5325ECLI_H
#define BCM5325ECLI_H

#ifdef __cplusplus
extern "C" {
#endif

/*typedef unsigned char mac_address_t [6];*/
/****************************************
* BrdChannelBcm5325eInit
* description: ��ʼ��bcm5325e vlan����,��������pon��
*	mac��ַ,�������в�λ��Ӧ��bcm5325e�˿ڸ���
*
*
*
******************************************/
int BrdChannelBcm5325eInit(void);


/******************************************
*BrdChannelMasterMacSet
* description: �ú��������ڰ��ͨ�ŵ�bcm5325e������
*	���ؿ������mac��ַ,��Ӧbcm5325e�Ķ˿�Ϊbcm port=5
*	1)��slot 4���ذ�ʱ,���뱾�����ذ�Ĳ���Ϊ:ulSlot = 4,macΪ����
*	���ذ��mac��ַ;
*	2)��slot 3Ϊ���ذ�ʱ,����ı������ذ�Ĳ���Ϊ:ulSlot = 3,macΪ
*	�������ذ��mac��ַ;
*
* input :
*	ulSlot - #define BCM_BRD_MASTER_SLOT		3
*			#define BCM_BRD_MASTER_BACK_SLOT	4
*	mac	- mac��ַ
********************************************/
int BrdChannelMasterMacSet(ULONG ulSlot, mac_address_t mac);

/******************************************
*BrdChannelMasterBackMacSet
* description: �ú��������ڰ��ͨ�ŵ�bcm5325e������
*	�Է����ؿ������mac��ַ,��Ӧbcm5325e�Ķ˿�Ϊbcm port=BCM_MASTER_BACK_BROAD_PORT
*	1)��slot 4���ذ�ʱ,����Է����ذ�Ĳ���Ϊ:ulSlot = 3,macΪ�Է�
*	���ذ��mac��ַ;
*	2)��slot 3Ϊ���ذ�ʱ,����ĶԷ����ذ�Ĳ���Ϊ:ulSlot = 4,macΪ
*	�Է����ذ��mac��ַ;
*
* input :
*	ulSlot - #define BCM_BRD_MASTER_SLOT		3
*			#define BCM_BRD_MASTER_BACK_SLOT	4
*	mac	- mac��ַ
*
********************************************/
int BrdChannelMasterBackMacSet(ULONG ulSlot, mac_address_t mac);

LONG BCM5325E_DEBUG_CommandInstall(void);

#ifdef __cplusplus
}
#endif

#endif	/*_BCM5325EHEADER_H_*/	
