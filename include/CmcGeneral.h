/**************************************************************
*
*    CmcGeneral.h -- Cmc management high level Application functions General header
*
*  
*    Copyright (c)  2013.3 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version   |      Date	      |    Change		    |    Author	  
*   --------|--- --------|------------------|------------
*	1.00	     | 13/03/2013 |     Creation		    | liwei056
*
*
***************************************************************/
#ifndef _CMCGERNRAL_H
#define  _CMCGERNRAL_H

#include  "OltGeneral.h"
#include  "OnuGeneral.h"
#include  "PonGeneral.h"
#include  "V2R1General.h"
#include  "V2R1_product.h"
#include  "qos/qos_include.h"


typedef enum
{
    cmc_mgmt_id = 0,

    cmc_mgmt_register,

    cmc_mgmt_unregister,

    cmc_mgmt_select,

    cmc_mgmt_last
}cmc_mgmt_type;

typedef enum
{
    cmc_sf_id = 0,

    cmc_sf_mac,

    cmc_sf_cos,

    cmc_sf_usid,

    cmc_sf_dsid,

    cmc_sf_tlv,

    cmc_sf_status,

    cmc_sf_last
}cmc_sf_type;

typedef enum
{
    cmc_sf_op_idle = 0,

    cmc_sf_op_create,

    cmc_sf_op_change,

    cmc_sf_op_delete,

    cmc_sf_op_last
}cmc_sf_op_status;


STATUS getBrcmCmcControllerMgmtVar(ULONG cmc_id,ULONG typeId,MIB_VALUE *retVar);
STATUS setBrcmCmcControllerMgmtVar(ULONG cmc_id,ULONG typeId,MIB_VALUE *setVar);

STATUS getBrcmCmcControllerServiceFlowVar(ULONG sf_id,ULONG typeId,MIB_VALUE *retVar);
STATUS setBrcmCmcControllerServiceFlowVar(ULONG sf_id,ULONG typeId,MIB_VALUE *setVar);

int notify_cmc_event(int CmcEventCode, short int PonPortIdx, short int OnuIdx, unsigned char CmcMac[6], unsigned char CmMac[6]);


int ShowCmcMacLearningByVty( short int PonPortIdx, short int OnuIdx, mac_address_t cmc_mac, struct vty *vty );
int ShowCmcMacLearningCounterByVty( short int PonPortIdx, short int OnuIdx, mac_address_t cmc_mac, struct vty *vty );

int ShowCmMacLearningByVty( short int PonPortIdx, short int OnuIdx, mac_address_t cmc_mac, mac_address_t cm_mac, struct vty *vty );
int ShowCmMacLearningCounterByVty( short int PonPortIdx, short int OnuIdx, mac_address_t cmc_mac, mac_address_t cm_mac, struct vty *vty );

int SetCmcSVlanID(short int PonPortIdx, int iVlanId);
int GetCmcSVlanID(int *iVlanId);
int ResumeCmcSVlanID(short int PonPortIdx);


int cmc_init();


#endif /* _CMCGERNRAL_H */
