/**************************************************************
*
*   IncludeFromCmc.h -- 
*
*  
*    Copyright (c)  2012.10 , GWD Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   ----------|--- --------|---------------------|------------
*	1.00	        | 02/18/2013  | Creation				| liwei056
*
***************************************************************/
#ifndef  __INCLUDEFROMCMC_H
#define __INCLUDEFROMCMC_H


#ifndef PACK
#define PACK __attribute__((packed))
#endif


#define CMC_MAX_VERSION_LENGTH     128

#define CMC_MIN_TLV_FILE_LENGTH    16
#define CMC_MAX_TLV_FILE_LENGTH    65535

#define CMC_MIN_CLS_FILE_LENGTH    3
#define CMC_MAX_CLS_FILE_LENGTH    255

#define CMC_MAC_LEN                6

#define CMC_MAX_DS_CH              16
#define CMC_MAX_US_CH              4

#define CMC_CFG_MAX_CM_DEFAULT                     200

#define CMC_CFG_DOWN_CHANNEL_ANNEX_A                 0
#define CMC_CFG_DOWN_CHANNEL_ANNEX_B                 1
#define CMC_CFG_DOWN_CHANNEL_ANNEX_DEFAULT           CMC_CFG_DOWN_CHANNEL_ANNEX_A

#define CMC_CFG_UP_CHANNEL_TYPE_SCDMA                3
#define CMC_CFG_UP_CHANNEL_TYPE_ATDMA                2
#define CMC_CFG_UP_CHANNEL_TYPE_DEFAULT              CMC_CFG_UP_CHANNEL_TYPE_SCDMA

#define CMC_CFG_UP_CHANNEL_PROFILE_DEFAULT           0

#define CMC_CFG_DOWN_CHANNEL_INTERLEAVER_DEFAULT     0

#define CMC_CFG_DOWN_CHANNEL_MODULATION_64QAM        0
#define CMC_CFG_DOWN_CHANNEL_MODULATION_256QAM       1
#define CMC_CFG_DOWN_CHANNEL_MODULATION_1024QAM      2
#define CMC_CFG_DOWN_CHANNEL_MODULATION_DEFAULT      CMC_CFG_DOWN_CHANNEL_MODULATION_64QAM

#define CMC_CFG_DOWN_CHANNEL_FREQ_BASE_DEFAULT   1600000
#define CMC_CFG_DOWN_CHANNEL_FREQ_STEP_DEFAULT   1600000
#define CMC_CFG_DOWN_CHANNEL_FREQ_DIR_INCREASE   1
#define CMC_CFG_DOWN_CHANNEL_FREQ_DIR_DECREASE   2
#define CMC_CFG_DOWN_CHANNEL_FREQ_DIR_DEFAULT    CMC_CFG_DOWN_CHANNEL_FREQ_DIR_INCREASE
#define CMC_CFG_DOWN_CHANNEL_FREQ_DEFAULT(channel_idx)  (CMC_CFG_DOWN_CHANNEL_FREQ_BASE_DEFAULT + CMC_CFG_DOWN_CHANNEL_FREQ_STEP_DEFAULT * (channel_idx))

#define CMC_CFG_UP_CHANNEL_FREQ_BASE_DEFAULT   1600000
#define CMC_CFG_UP_CHANNEL_FREQ_STEP_DEFAULT   1600000
#define CMC_CFG_UP_CHANNEL_FREQ_DIR_INCREASE   1
#define CMC_CFG_UP_CHANNEL_FREQ_DIR_DECREASE   2
#define CMC_CFG_UP_CHANNEL_FREQ_DIR_DEFAULT    CMC_CFG_UP_CHANNEL_FREQ_DIR_INCREASE
#define CMC_CFG_UP_CHANNEL_FREQ_WIDTH_DEFAULT  6400000
#define CMC_CFG_UP_CHANNEL_FREQ_DEFAULT(channel_idx)  (CMC_CFG_UP_CHANNEL_FREQ_BASE_DEFAULT + CMC_CFG_UP_CHANNEL_FREQ_STEP_DEFAULT * (channel_idx))

#define CMC_CFG_UP_CHANNEL_POWER_LEVEL_DEFAULT     100
#define CMC_CFG_DOWN_CHANNEL_POWER_LEVEL_DEFAULT   500


#define CMC_CFG_GROUP_DYNAMIC_METHOD_DISABLED    0
#define CMC_CFG_GROUP_DYNAMIC_METHOD_DYNAMIC     1
#define CMC_CFG_GROUP_DYNAMIC_METHOD_STATIC      2
#define CMC_CFG_GROUP_DYNAMIC_METHOD_DEFAULT     CMC_CFG_GROUP_DYNAMIC_METHOD_DYNAMIC

#define CMC_CFG_GROUP_DYNAMIC_PERIOD_DEFAULT               30
#define CMC_CFG_GROUP_DYNAMIC_WEIGHT_PERIOD_DEFAULT        1
#define CMC_CFG_GROUP_DYNAMIC_OVERLOAD_THRESHOLD_DEFAULT   100
#define CMC_CFG_GROUP_DYNAMIC_DIFFERENCE_THRESHOLD_DEFAULT 100
#define CMC_CFG_GROUP_DYNAMIC_MOVE_MAX_DEFAULT             100
#define CMC_CFG_GROUP_DYNAMIC_HOLDTIME_MIN_DEFAULT         60

#define CMC_CFG_GROUP_DYNAMIC_RANGE_OVERRIDE_DISABLED     2
#define CMC_CFG_GROUP_DYNAMIC_RANGE_OVERRIDE_ENABLED      1
#define CMC_CFG_GROUP_DYNAMIC_RANGE_OVERRIDE_DEFAULT      CMC_CFG_GROUP_DYNAMIC_RANGE_OVERRIDE_ENABLED

#define CMC_CFG_GROUP_DYNAMIC_ATDMA_DCC_INIT_TECH_DEFAULT 1
#define CMC_CFG_GROUP_DYNAMIC_ATDMA_DBC_INIT_TECH_DEFAULT 1
#define CMC_CFG_GROUP_DYNAMIC_SCDMA_DCC_INIT_TECH_DEFAULT 1
#define CMC_CFG_GROUP_DYNAMIC_SCDMA_DBC_INIT_TECH_DEFAULT 1


#define CMC_CFG_SVLAN_ID_NONE            0
#define CMC_CFG_SVLAN_ID_ALL             4095
#define CMC_CFG_SVLAN_ID_UNKNOWN         CMC_CFG_SVLAN_ID_ALL
#define CMC_CFG_SVLAN_ID_DEFAULT         0


#define CMC_CHANNELID_ALL  0


enum
{
    CMC_EVENT_CMC_UNKNOWN = 0,
    CMC_EVENT_CMC_ARRIVAL,
    CMC_EVENT_CMC_READY,
    CMC_EVENT_CMC_DEPART,
    CMC_EVENT_CM_ARRIVAL,
    CMC_EVENT_CM_READY,
    CMC_EVENT_CM_DEPART,
    CMC_EVENT_CODE_MAX
};


typedef struct MacAddressT
{
    U8 addr[CMC_MAC_LEN];
} MacAddressT;


int CMCCTRL_DumpAllCmc(char *dump_buf, unsigned short *dump_len);
int CMCCTRL_DumpCmc(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);

int CMCOAM_System_DumpAlarmList(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_System_DumpLogData(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);

int CMCCTRL_RegisterCmc(unsigned char cmcMac[CMC_MAC_LEN], unsigned char *vendorData, int vendorDataLength);
int CMCCTRL_UnRegisterCmc(unsigned char cmcMac[CMC_MAC_LEN]);

int CMCOAM_System_GetSoftwareVersion(char *pVersion, unsigned char *pLength, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_System_GetNumOfMulticasts(unsigned short *pMulNum, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_System_GetNumOfCm(unsigned short *pCmNum, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_System_SetNumOfCm(unsigned short cmNum, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_System_ResetCmcBoard(unsigned char cmcMac[CMC_MAC_LEN]);

int CMCOAM_Channel_DumpDownstreamChannel(unsigned char cmcMac[CMC_MAC_LEN], unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Channel_GetDownstreamSettingsEnabled(unsigned char channel_id, unsigned char enable[CMC_MAX_DS_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetDownstreamSettingsEnabled(unsigned char channel_id, unsigned char enable, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetDownstreamSettingsAnnexMode(unsigned char channel_id, unsigned char annex_mode[CMC_MAX_DS_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetDownstreamSettingsAnnexMode(unsigned char channel_id, unsigned char annex_mode, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetDownstreamSettingsFreq(unsigned char channel_id, unsigned long freq[CMC_MAX_DS_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetDownstreamSettingsFreq(unsigned char channel_id, unsigned long freq, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_AutoAssignDownstreamSettingsFreq(unsigned long freq, unsigned long step, unsigned char mode, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetDownstreamSettingsModulation(unsigned char channel_id, unsigned char modulation[CMC_MAX_DS_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetDownstreamSettingsModulation(unsigned char channel_id, unsigned char modulation, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetDownstreamSettingsPowerLevel(unsigned char channel_id, short int power[CMC_MAX_DS_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetDownstreamSettingsPowerLevel(unsigned char channel_id, short int power, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetDownstreamSettingsInterleaver(unsigned char channel_id, unsigned char interleaver[CMC_MAX_DS_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetDownstreamSettingsInterleaver(unsigned char channel_id, unsigned char interleaver, unsigned char cmcMac[CMC_MAC_LEN]);

int CMCOAM_Channel_DumpUpstreamChannel(unsigned char cmcMac[CMC_MAC_LEN], unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Channel_GetUpstreamSettingsEnabled(unsigned char channel_id, unsigned char enable[CMC_MAX_US_CH],unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetUpstreamSettingsEnabled(unsigned char channel_id, unsigned char enable,unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetUpstreamSettingsFreq(unsigned char channel_id, unsigned long freq[CMC_MAX_US_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetUpstreamSettingsFreq(unsigned char channel_id, unsigned long freq, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_AutoAssignUpstreamSettingsFreq(unsigned long freq, unsigned long step, unsigned char mode, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetUpstreamSettingsChannelWidth(unsigned char channel_id, unsigned long channel_width[CMC_MAX_US_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetUpstreamSettingsChannelWidth(unsigned char channel_id, unsigned long channel_width, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetUpstreamSettingsChannelProfile(unsigned char channel_id, unsigned char channel_profile[CMC_MAX_US_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetUpstreamSettingsChannelProfile(unsigned char channel_id, unsigned char channel_profile, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetUpstreamSettingsD30Mode(unsigned char channel_id, unsigned char mode[CMC_MAX_US_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetUpstreamSettingsD30Mode(unsigned char channel_id, unsigned char mode, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_DumpUpstreamInputPower(unsigned char cmcMac[CMC_MAC_LEN], unsigned char channel_id, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Channel_GetUpstreamInputPowerLevel(unsigned char channel_id, short int power[CMC_MAX_US_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetUpstreamInputPower(unsigned char channel_id, short int power, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_GetUpstreamChannelType(unsigned char channel_id, unsigned char channel_type[CMC_MAX_US_CH], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_SetUpstreamChannelType(unsigned char channel_id, unsigned char channel_type, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Channel_DumpUpstreamChannelSignalQuality(unsigned char cmcMac[CMC_MAC_LEN], unsigned char channel_id, char *dump_buf, unsigned short *dump_len);

int CMCOAM_Channel_DumpCmcAllInterfaces(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_Channel_DumpCmcMacStatistics(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_Channel_DumpCmcInterfaceStatistics(unsigned char cmcMac[CMC_MAC_LEN], unsigned short channel_index, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Channel_DumpCmcInterfaceUtilization(unsigned char cmcMac[CMC_MAC_LEN], unsigned short channel_index, char *dump_buf, unsigned short *dump_len);


int CMCOAM_Group_DumpLoadBalancingDynConfig(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_Group_SetLoadBalancingDynMethod(unsigned char cmcMac[CMC_MAC_LEN], unsigned char method);
int CMCOAM_Group_SetLoadBalancingDynPeriod(unsigned char cmcMac[CMC_MAC_LEN], unsigned long period);
int CMCOAM_Group_SetLoadBalancingDynWeightedAveragePeriod(unsigned char cmcMac[CMC_MAC_LEN], unsigned long period);
int CMCOAM_Group_SetLoadBalancingDynOverloadThresold(unsigned char cmcMac[CMC_MAC_LEN], unsigned char percent);
int CMCOAM_Group_SetLoadBalancingDynDifferenceThresold(unsigned char cmcMac[CMC_MAC_LEN], unsigned char percent);
int CMCOAM_Group_SetLoadBalancingDynMaxMoveNumber(unsigned char cmcMac[CMC_MAC_LEN], unsigned long move_max);
int CMCOAM_Group_SetLoadBalancingDynMinHoldTime(unsigned char cmcMac[CMC_MAC_LEN], unsigned long hold_time);
int CMCOAM_Group_SetLoadBalancingDynRangeOverrideMode(unsigned char cmcMac[CMC_MAC_LEN], unsigned char range_mode);
int CMCOAM_Group_SetLoadBalancingDynAtdmaDccInitTech(unsigned char cmcMac[CMC_MAC_LEN], unsigned char tech_id);
int CMCOAM_Group_SetLoadBalancingDynScdmaDccInitTech(unsigned char cmcMac[CMC_MAC_LEN], unsigned char tech_id);
int CMCOAM_Group_SetLoadBalancingDynAtdmaDbcInitTech(unsigned char cmcMac[CMC_MAC_LEN], unsigned char tech_id);
int CMCOAM_Group_SetLoadBalancingDynScdmaDbcInitTech(unsigned char cmcMac[CMC_MAC_LEN], unsigned char tech_id);

int CMCOAM_Group_DumpAllLoadBalancingGrp(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_Group_DumpFullLoadBalancingGrpSettings(unsigned char cmcMac[CMC_MAC_LEN], unsigned char grp_id, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Group_DumpLoadBalancingGrpDownsteam(unsigned char cmcMac[CMC_MAC_LEN], unsigned char grp_id, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Group_DumpLoadBalancingGrpUpsteam(unsigned char cmcMac[CMC_MAC_LEN], unsigned char grp_id, char *dump_buf, unsigned short *dump_len);

int CMCOAM_Group_CreateDestroyLoadBalancingGrp(unsigned char grp_id, unsigned char method, unsigned char option, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Group_AddRemoveDownChannelsToFromLoadBalancingGrp(unsigned char grp_id, unsigned char *ch_id, unsigned char num_of_ch, unsigned char option, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Group_AddRemoveUpChannelsToFromLoadBalancingGrp(unsigned char grp_id, unsigned char *ch_id, unsigned char num_of_ch, unsigned char option, unsigned char cmcMac[CMC_MAC_LEN]);

int CMCOAM_Group_AddRemoveCnuToFromLoadBalancingGrp(unsigned char grp_id, unsigned char option, unsigned char mac_start[CMC_MAC_LEN], unsigned char mac_end[CMC_MAC_LEN], unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Group_SetExcludeCnusFromLoadBalancingGrp(unsigned char option, unsigned char mac_start[CMC_MAC_LEN], unsigned char mac_end[CMC_MAC_LEN], unsigned char cmcMac[CMC_MAC_LEN]);

int CMCOAM_Group_DumpLoadBalancingGrpCnuConfig(unsigned char cmcMac[CMC_MAC_LEN], unsigned char grp_id, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Group_DumpLoadBalancingGrpActivedCnu(unsigned char cmcMac[CMC_MAC_LEN], unsigned char grp_id, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Group_DumpLoadBalancingGrpExcludeCnusConfiguration(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_Group_DumpLoadBalancingGrpExcludeActiveCnus(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);

int CMCOAM_Cm_DumpCnuList(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_Cm_DumpCnuStatus(unsigned char cmcMac[CMC_MAC_LEN], MacAddressT cmMac, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Cm_DumpCableModemHistory(unsigned char cmcMac[CMC_MAC_LEN], MacAddressT *cmMac, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Cm_ClearCableModem(MacAddressT cmMac, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Cm_ClearCableModemHistory(unsigned char cmcMac[CMC_MAC_LEN]);

int CMCOAM_Cm_DumpCnuDownstream(unsigned char cmcMac[CMC_MAC_LEN], MacAddressT cmMac, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Cm_DumpCnuUpstream(unsigned char cmcMac[CMC_MAC_LEN], MacAddressT cmMac, char *dump_buf, unsigned short *dump_len);
int CMCOAM_Cm_SetCnuDownstream(MacAddressT cmMac, unsigned char* channel_lst, unsigned char num_of_channel, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_Cm_SetCnuUpstream(MacAddressT cmMac, unsigned char* channel_lst, unsigned char num_of_channel, unsigned char cmcMac[CMC_MAC_LEN]);

int CMCCTRL_Qos_DumpCmClassifier(unsigned char cmcMac[CMC_MAC_LEN], MacAddressT cmMac, char *dump_buf, unsigned short *dump_len);
int CMCCTRL_Qos_DumpCmServiceFlow(unsigned char cmcMac[CMC_MAC_LEN], MacAddressT cmMac, char *dump_buf, unsigned short *dump_len);

int CMCOAM_Qos_DumpPktClassifierConfig(unsigned long *sf_id, unsigned short num_of_sf, unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_Qos_DumpServiceFlowConfig(unsigned long *sf_id, unsigned short num_of_sf, unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_Qos_DumpServiceFlowStatistics(unsigned long *sf_id, unsigned short num_of_sf, unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);

int CMCOAM_Qos_DumpDsBondingGroup(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);
int CMCOAM_Qos_DumpUsBondingGroup(unsigned char cmcMac[CMC_MAC_LEN], char *dump_buf, unsigned short *dump_len);

int CMCCTRL_Dsx_CreateServiceFlow(MacAddressT cmMac, unsigned char cos, const unsigned char *tlv_data, int tlv_len, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCCTRL_Dsx_ChangeServiceFlow(MacAddressT cmMac, unsigned long usfid, unsigned long dsfid, const unsigned char *tlv_data, int tlv_len, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCCTRL_Dsx_DestroyServiceFlow(MacAddressT cmMac, unsigned long us_sfid, unsigned long ds_sfid, unsigned char cmcMac[CMC_MAC_LEN]);

int CMCCTRL_Qos_SetServiceFlowClassName(const unsigned char *pName, unsigned char action, const unsigned char *tlv_data, int tlv_len, unsigned char cmcMac[CMC_MAC_LEN]);


typedef struct
{
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char wday;
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
}BASE_DateTimeT;

int CMCOAM_System_GetTime(unsigned char cmcMac[CMC_MAC_LEN], BASE_DateTimeT *pDateTime);
int CMCOAM_System_SetTime(unsigned char cmcMac[CMC_MAC_LEN], const BASE_DateTimeT * pDateTime);
int CMCOAM_System_LocalTime(unsigned char cmcMac[CMC_MAC_LEN]);


typedef enum 
{
	kTPID = 0x1,
	kMinMapTime,
	kMaxMapTime,
	kInitRngPeriod,
	kPeriodicRngPeriod,
	kRngBackoffStart,
	kRngBackoffEnd,
	kDataBackoffStart,
	kDataBackoffEnd,
	kMapLeadTimeAdjustment,
	MAX_SUBTYPE_NUM,
	kIlleagalSubType = 0xFF
}subTypeCodeT;

int CMCOAM_System_SetCustomConfiguration(unsigned char subtype, const void *pValue, unsigned short len, unsigned char cmcMac[CMC_MAC_LEN]);
int CMCOAM_System_DumpCmcCustomCfg(unsigned char cmcMac[CMC_MAC_LEN], unsigned char subtype, char *dump_buf, unsigned short *dump_len);

#endif 

