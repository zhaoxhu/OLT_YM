#ifdef	__cplusplus
extern "C"
{
#endif

#include  "OltGeneral.h"
#if( EPON_MODULE_PON_SFP_CHECK == EPON_MODULE_YES )
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include "bmsp/product_info/bms_product_info.h"
/*
#include  "gwEponMibData.h"

#include  "PonEventHandler.h"
#include "Math.h"
*/

/*add by yanjy2016-12*/
int checkSfpTypeEponOrGpon(short int PonPortIdx);
extern int GetPonPortSfpState(short int PonPortIdx);

/*add by yanjy2016-12*/

_XCVR_DATA_ XcvrInfoArr[] = {
	{ 0,  "Identifier", 				1, 0 },  		/*  0  */
	{ 1,  "Extended Identifier", 		1, 0 },
	{ 2,  "Connector", 				1, 0 },
	{ 3,  "Transceiver", 			8, 0 },
	{ 11, "Encoding", 				1, 0 },
	{ 12, "BR, Nominal", 			1, 0 },		/*  5  */
	{ 13, "Reserved", 				1, 0 },
	{ 14, "Length (9um)-KM",		1, 0 },
	{ 15, "Length (9um)-100M",		1, 0 },
	{ 16, "Length (50um)-10M",		1, 0 },
	{ 17, "Length (62.5um)-10m",	1, 0 },		/*  10  */
	{ 18, "Length (Copper)",		1, 0 },
	{ 19, "Reserved", 				1, 0 },
	{ 20, "Vendor name", 			16, 1 },
	{ 36, "Reserved", 				1, 0 },
	{ 37, "Vendor OUI", 			3, 0 },			/*  15  */
	{ 40, "Vendor PN", 				16, 1 },
	{ 56, "Vendor Revision Number",	4, 1 },
	{ 60, "Wavelength",			2, 0 },
	{ 62, "Reserved", 				1, 0 },
	{ 63, "CC-base", 				1, 0 },		/*  20  */
	{ 64, "Transceiver Options",		2, 0 },
	{ 66, "BR, max",				1, 0 },
	{ 67, "BR, min",				1, 0 },
	{ 68, "Vendor SN",				16, 1 },
	{ 84, "Date code",				8, 1 },		/* 25  */
	{ 92, "Diagnostic Monitoring Type",1, 0 },
	{ 93, "Enhanced Options",		1, 0 },
	{ 94, "SFF-8472 Compliance",	1, 0 },
	{ 95, "CC_EXT",				1, 0 },
	{ 96, NULL,					1, 0 }		/*  30  */
};

_XCVR_DATA_ XcvrDiagArr[] = {
	{ 0,  "Temp High Alarm",		2, 0 },   		/* 0 */
	{ 2,  "Temp Low Alarm", 		2, 0 },
	{ 4,  "Temp High Warning", 		2, 0 },
	{ 6,  "Temp Low Warning",		2, 0 },
	{ 8,  "Voltage High Alarm",		2, 0 },
	{ 10, "Voltage Low Alarm", 		2, 0 },		/* 5 */
	{ 12, "Voltage High Warning",	2, 0 },
	{ 14, "Voltage Low Warning",		2, 0 },
	{ 16, "Bias High Alarm",			2, 0 },
	{ 18, "Bias Low Alarm",			2, 0 },
	{ 20, "Bias High Warning",		2, 0 },		/* 10 */
	{ 22, "Bias Low Warning",		2, 0 },
	{ 24, "TX Power High Alarm",	2, 0 },
	{ 26, "TX Power Low Alarm", 	2, 0 },
	{ 28, "TX Power High Warning",	2, 0 },
	{ 30, "TX Power Low Warning", 	2, 0 },		/* 15 */
	{ 32, "RX Power High Alarm",	2, 0 },
	{ 34, "RX Power Low Alarm",		2, 0 },
	{ 36, "RX Power High Warning",	2, 0 },
	{ 38, "RX Power Low Warning", 	2, 0 }, 
	/*{ 40, "Reserved",			16, 0 },*/
	{ 56, "Rx_PWR(4)",			4, 0 },		/* 20 */
	{ 60, "Rx_PWR(3)",			4, 0 },
	{ 64, "Rx_PWR(2)",			4, 0 },
	{ 68, "Rx_PWR(1)",			4, 0 },
	{ 72, "Rx_PWR(0)",			4, 0 },
	{ 76, "Tx_I(Slope)",			2, 0 },			/* 25 */
	{ 78, "Tx_I(Offset)",			2, 0 },
	{ 80, "Tx_PWR(Slope)",			2, 0 },
	{ 82, "Tx_PWR(Offset)",			2, 0 },
	{ 84, "T (Slope)",				2, 0 },
	{ 86, "T (Offset)",				2, 0 },		/* 30 */
	{ 88, "V (Slope)",				2, 0 },
	{ 90, "V (Offset)",				2, 0 },
	/*{ 92, "Reserved",			3, 0 },*/
	{ 95, "Checksum",				1, 0 },
	{ 96, "Temperature",			2, 0 },
	{ 98, "Working-voltage",		2, 0 },			/* 35 */
	{100, "Bias-current",			2, 0 },
	{102, "TX Optical-power",		2, 0 },
	{104, "RX Optical-Power",		2, 0 },
	/*{106, "Reserved",			4, 0 },*/
	{110, "Optional Status/Control",1, 0 },
	{111, NULL,					1, 0 }	/* 40 */
};

unsigned char *XGBASE_PRX_PN[] = {
	"LTH4301-PC+",
	NULL
};

unsigned char *XGBASE_PR_PN[] = {
	"LTH5302-PC+",
	NULL
};
LONG CheckSlotPonSFPIsOK( short int PonPortIdx );
#undef PON_SFP_EEPROM_PERSISTENCE
#ifdef PON_SFP_EEPROM_PERSISTENCE
UCHAR pon_sfp_online_flag[SYS_MAX_PON_PORTNUM];
UCHAR pon_sfp_auth_flag[SYS_MAX_PON_PORTNUM];

STATUS ponSfpInit()
{
	VOS_MemZero( pon_sfp_online_flag, sizeof(pon_sfp_online_flag) );
	VOS_MemZero( pon_sfp_auth_flag, sizeof(pon_sfp_auth_flag) );
	return (ROK);
}

int ponSfp_SlotPull( ULONG slotno )
{
	int i;
	short int PonPortIdx;

	for( i=1; i<=MAX_PONPORT_PER_BOARD; i++ )
	{
		PonPortIdx = GetPonPortIdxBySlot( slotno, i );
		if( (PonPortIdx >= 0) && (PonPortIdx < MAXPON) )
		{
			pon_sfp_online_flag[PonPortIdx] = 0;
			pon_sfp_auth_flag[PonPortIdx] = 0;
		}
	}
	return VOS_OK;
}
int ponSfp_SlotInsert( ULONG slotno )
{
	return VOS_OK;
}
#endif

/************************************************************************
	�ж�pon�忨��·����PON ��ģ���Ƿ�֧�ֹ⹦�ʲ���
	��Ҫ������:
		1  �µ�PON ���·
		2  ����RSSI ���ܵ��ض���ģ��
*************************************************************************/
/*
	1 -- ͨ��PON��eeprom�еĹ�ģ�����ͣ��жϴ�PON ���Ƿ�֧�ֽ���ONU �⹦�ʲ���RSSI
		 �ṩ��������һ�µ�API����������ͬ
*/
extern unsigned long getSfpManufacturerId( unsigned char slotno,  unsigned char ponno );

ULONG getPonSfpTypeFromProductE2( short int PonPortIdx )
{
	int slotno, portno;

	CHECK_PON_RANGE
	slotno = GetCardIdxByPonChip(PonPortIdx);
	portno = GetPonPortByPonChip(PonPortIdx);

	return getSfpManufacturerId(slotno, portno);
}

extern UCHAR Sfp_Pon_Online_Flag[ ];

int ponSfp_IsSupportRSSI(int slot, int port, struct vty *vty)
{
	/*int SFPType=spfManufacturer_null;*/
	PON_gpio_line_io_t  direction;
	bool value = 0;
	unsigned int PonPortIdx;
	int lRet = 0;
	
	if(PonCardSlotPortCheckByVty(slot, port, vty) != ROK)
		return(RERROR);
	if(SlotCardIsPonBoard(slot) != ROK)
	{
		if(vty != NULL )
		{
			vty_out( vty, "  %% error. slot %d is not %s\r\n", slot, GetPonBoardNameString());
		}
		return(RERROR);
	}

	PonPortIdx = GetPonPortIdxBySlot(slot, port);

	if( !SYS_LOCAL_MODULE_TYPE_IS_SFP_FIXED_PON )
	{
#if 0
	    /*for 10G EPON of PMC8411 by jinhl @2012-11-12*/
		if(OLT_GpioAccess(PonPortIdx, PON_GPIO_LINE_3, PON_GPIO_LINE_INPUT, 0, &direction, &value ) == 0)
#else
      		if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
		{
			lRet = GetGponSfpOnlineState(PonPortIdx, &value);
		}
		else
		{
         	 	lRet = OLT_ReadGpio(PonPortIdx, OLT_GPIO_SFP_LOSS, &value);
		}
#endif
		if(lRet == 0)
		{
			if(1 == value)
			{
				if( 1 == Sfp_Pon_Online_Flag[PonPortIdx] )
				{
					ClearOpticalPowerAlarmWhenPonPortDown( PonPortIdx ) ;
				}
				
				/*Sfp_Pon_Online_Flag[ PonPortIdx ] = 0 ;*/
				
				return VOS_ERROR; /* �����ù�ģ�����ڲ���λ�� */
			}
			else
			{
				Sfp_Pon_Online_Flag[ PonPortIdx ] = 1;
			}
		}
	}
	
	return ROK;
}

/*  
	2 -- PON �ڹ�ģ���Ƿ���λ
	        Ŀǰ���Թ�ģ���ڲ��Ĵ�����vendor name ��Ϊ��ʶ��֧�ֺ��Ź�ģ�飬�����µ�
	        ��ģ�飬����չ�˺�����
	        �Ժ�Ӳ��֧�ֺ󣬿�ͨ����PON оƬGPIO �ܽ�״̬����֪��ģ���Ƿ���λ
	        �ṩ��������һ�µ�API����������ͬ
*/
int ponSfp_IsOnline(short int PonPortIdx)
{
	short int data;
	short int  ulDevAddr;
	short int gwRet;
	int status;
	_XCVR_DATA_  *pXcvrArr;
#ifdef PON_SFP_EEPROM_PERSISTENCE
	int i;
	char *pHisense = SFP_TYPE_Vendor_Name_Hisense;
#endif
	ULONG portno, slotno;
	ULONG SFPType;
	int ret = 0;

	CHECK_PON_RANGE

	status = GetPonchipWorkingStatus( PonPortIdx );
	if( (status != PONCHIP_UP ) && (status != PONCHIP_TESTING))
		return (RERROR);
	
#ifdef PON_SFP_EEPROM_PERSISTENCE
	if( pon_sfp_online_flag[PonPortIdx] == 0 )
	{
		int len = VOS_StrLen(pHisense);
		ulDevAddr = A0H_1010000X;
		pXcvrArr = &XcvrInfoArr[13];
		
		if( len > pXcvrArr->cLen )
			len = pXcvrArr->cLen;
		
		for(i=0; i<len; i++)
		{
		    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			gwRet = OLT_ReadI2CRegister(PonPortIdx,ulDevAddr, pXcvrArr->cAddr+i, &data);
			if(PAS_EXIT_OK  != gwRet)
			{
				/*pon_sfp_online_flag[PonPortIdx] = 2;*/
				break;
			}
			else
			{
				if( pHisense[i] != *(((unsigned char *)&data)+1) )
				{
					pon_sfp_online_flag[PonPortIdx] = 2;
					break;
				}
			}
		}
	}
	if( pon_sfp_online_flag[PonPortIdx] == 1 )
		return(ROK);
	return (RERROR);
#else
	status = ROK;
	slotno  = (short int )GetCardIdxByPonChip(PonPortIdx);
	portno = GetPonPortByPonChip(PonPortIdx);
	SFPType = getSfpManufacturerId(slotno, portno);

	if( SFPType == spfManufacturer_GWD ) 
	{
        bool bSFPIsLoose;

	if(SYS_LOCAL_MODULE_TYPE_IS_GPON)
	{
		ret = GetGponSfpOnlineState(PonPortIdx, &bSFPIsLoose);
	}
	else
	{
     	 	ret = OLT_ReadGpio(PonPortIdx, OLT_GPIO_SFP_LOSS, &bSFPIsLoose);
	}
		
        if ( 0 == ret )
        {
            if ( bSFPIsLoose )
            {
    			status = RERROR;
            }
        }
        else
        {
    		ulDevAddr = A0H_1010000X;
    		pXcvrArr = &XcvrInfoArr[13];
#if 0		
    		for(i=0; i<3/*pXcvrArr->cLen*/; i++)	/* ��ʱֻ�ж�ǰ3���ַ� */
    		{
    			gwRet = PAS_read_i2c_register(PonPortIdx,ulDevAddr, pXcvrArr->cAddr+i, &data);
    			if(PAS_EXIT_OK  != gwRet)
    			{
    				status = RERROR;
    				break;
    			}
    			else
    			{
    				if( pHisense[i] != *(((unsigned char *)&data)+1) )
    				{
    					status = RERROR;
    					break;
    				}
    			}
    		}
#else
            /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
    		gwRet = OLT_ReadI2CRegister(PonPortIdx,ulDevAddr, pXcvrArr->cAddr, &data);
    		if(PAS_EXIT_OK  != gwRet)
    			status = RERROR;
#endif
        }
	}
	else
		status = RERROR;
	
	return status;
#endif
}

/*  
	3 -- PON �ڹ�ģ������ʶ��: �Ƿ�ΪGWD ����
		  �ṩ��������һ�µ�API����������ͬ
*/
#ifdef PON_SFP_EEPROM_PERSISTENCE
int  ponSfp_readVendorOUI( short int PonPortIdx, char *SFPOUIType)
{
	short int i2c_device, data;
	short int gwRet;
	int i;

	if(SFPOUIType == NULL)
		return(RERROR);
	VOS_MemSet(SFPOUIType, 0, SFPOUITYPELEN);
	
	i2c_device = A2H_1010001X;
	for(i=0; i<SFPOUITYPELEN; i++)
	{
	    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		gwRet = OLT_ReadI2CRegister(PonPortIdx,i2c_device, SFPOUITYPEBASEADDR+i, &data);
		if(PAS_EXIT_OK  != gwRet)
		{
			SFPOUIType[i] = 0x20;
			break;
		}
		else 
			SFPOUIType[i] = *(((unsigned char *)&data)+1);
	}
	SFPOUIType[i] = '\0';

	return(ROK);	
}
#else
/* ȡSFP ��ģ����Vendor OUI */
int  ponSfp_readVendorID( short int PonPortIdx, char *sfpType)
{
	short int gwRet;
	char data[2];
	int i;

	if(sfpType == NULL)
		return(RERROR);
	
	for(i=0; i<10/*SFP_TYPE_Vendor_len*/; i++)
	{
	    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
		gwRet = OLT_ReadI2CRegister(PonPortIdx, A0H_1010000X, 0x60+i, (short int *)data);
		if(PAS_EXIT_OK  != gwRet)
			break;
		else
		{
			if( (data[1] == 0x20) || (data[1] == 0) ||(data[1] == 0xff) )/*���ⵥ: 16974*/
				break;
			sfpType[i] = data[1];
		}
		if( i == (SFPOUITYPELEN - 1) )
		{
			if( VOS_MemCmp(sfpType, SFPOUITYPE_GWD, SFPOUITYPELEN) == 0 )
			{
				i++;
				break;
			}
		}
	}

	/*if( i )*/
	{
		if( i == 10 /*SFP_TYPE_Vendor_len-1*/)
			sfpType[9] = '\0';
		else
			sfpType[i] = '\0';
	}
	/*else
	{
		if( ponSFPType_read(PonPortIdx, sfpType) != ROK )
			VOS_StrCpy( sfpType, "Unknown" );
	}*/
	return(ROK);	
}
#endif
int pon_SFP_MonitorType_read(short int PonPortIdx, char *MonType)
{
         short int DevAddr, data;
         short int ret;
         _XCVR_DATA_ *pXcvrArr;
        
         DevAddr = A0H_1010000X;       
         pXcvrArr = &XcvrInfoArr[26];
       
         if(MonType == NULL)
                   return(RERROR);
         data=0;
		 /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
         ret = OLT_ReadI2CRegister(PonPortIdx,DevAddr, pXcvrArr->cAddr, &data);
         if((PAS_EXIT_OK  != ret)||((PAS_EXIT_OK  == ret)&&((*(&data))==0)))
         {
                   return VOS_ERROR;
         }
         else
         {
                   *MonType = *(((unsigned char *)&data)+1);
         }
         return(ROK);              
}

ULONG CHECK_SFP_IS_GWD_TYPE_ENABLE = 0;

int  ponSfp_IsGwdType(short int PonPortIdx, struct vty *vty)
{
	int ret = ROK;
#if 0
	short int gwRet, data;
	int i;
	char *sfpType = SFPOUITYPE_GWD;

	CHECK_PON_RANGE;

	/*if(ponSfp_readVendorID(PonPortIdx, SFPTypeString) != ROK)
		return(RERROR);
	if( VOS_StrnCmp(SFPTypeString, SFPOUITYPE_GWD, SFPOUITYPELEN) == 0)
		return(ROK);*/
	/* ��ģ�����ͼ���ģ���е�Vendor OUI ����Ԥ��ֵһ�£���ͨ�����*/	
	for(i=0; i<SFPOUITYPELEN; i++)
	{
		gwRet = PAS_read_i2c_register(PonPortIdx, A0H_1010000X, 0x60+i, &data);
		if(PAS_EXIT_OK  == gwRet)
		{
			if( sfpType[i] != *(((unsigned char *)&data)+1) )
				return VOS_ERROR;
		}
		else
			return VOS_ERROR;
	}
#else	/* modified by xieshl 20110708 */
	short int gwRet, data;
	int i, j, rd_err, rd_count = 2;
	char *sfpType = SFPOUITYPE_GWD;

	CHECK_PON_RANGE;

	if(!CHECK_SFP_IS_GWD_TYPE_ENABLE)
	{
		return ret;
	}

	/*if(ponSfp_readVendorID(PonPortIdx, SFPTypeString) != ROK)
		return(RERROR);
	if( VOS_StrnCmp(SFPTypeString, SFPOUITYPE_GWD, SFPOUITYPELEN) == 0)
		return(ROK);*/
	/* ��ģ�����ͼ���ģ���е�Vendor OUI ����Ԥ��ֵһ�£���ͨ�����*/	
	for(i=0; i<SFPOUITYPELEN; i++)
	{
		ret = RERROR;
		rd_err = 0;
		for( j=0; j<rd_count; j++ )
		{
		    /*for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
			gwRet = OLT_ReadI2CRegister(PonPortIdx, A0H_1010000X, 0x60+i, &data);
			if(PAS_EXIT_OK  == gwRet)
			{
				if( sfpType[i] == *(((unsigned char *)&data)+1) )
				{
					ret = ROK;
					break;
				}
			}
			else
				rd_err++;
		}
		if( ret == RERROR )
		{
			if( rd_err == rd_count )
				ret = ROK;
			break;
		}
	}

#endif
	return ret;
}

/* B--added by liwei056@2014-3-25 for BCM-PON's SFP CheckReport */
int  ponSfp_IsHisenseType(short int PonPortIdx)
{
	int ret;
    char szSfpTypeStr[10];

	CHECK_PON_RANGE;

    if ( ROK == (ret = ponSFPType_read(PonPortIdx, szSfpTypeStr)) )
    {
        if ( (0 != VOS_StrCmp(SFPOUITYPE_GWD, szSfpTypeStr))
            && (0 != VOS_StrCmp(SFPOUITYPE_HIS, szSfpTypeStr)) )
        {
            ret = RERROR;
        }
    }

	return ret;
}

int ponSfp_GetProductDate(short int PonPortIdx, unsigned char aucDate[4])
{
	int ret;
    int i, m, n;
	short int DevAddr, data;
    char aszDateStr[3];
	_XCVR_DATA_ *pXcvrArr;
	
	DevAddr  = A0H_1010000X;	
	pXcvrArr = &XcvrInfoArr[25];

	/* ��ģ���������*/
    VOS_MemZero(aucDate, 4);
    m = 0;
    n = 0;
	for(i=0; i<8; i++)
	{
		ret = OLT_ReadI2CRegister(PonPortIdx, DevAddr, (short int)(pXcvrArr->cAddr+i), &data);
		if ( 0 == ret )
		{
			if ( 0x20 != data )
            {
				aszDateStr[m++] = *(((char *)&data)+1);
                if ( 2 == m )
                {
                    m = 0;
                    aszDateStr[2] = '\0';
                    aucDate[n++]  = (unsigned char)VOS_AtoI(aszDateStr);
                }
            }         
			else 
			{
				break;
			}
		}
        else
        {
            break;
        }
	}

    return ret;
}
/* E--added by liwei056@2014-3-25 for BCM-PON's SFP CheckReport */
int test1 = 0;
/* added by xieshl 20110705, PON������ʱ�ȼ��һ�Σ����õȴ���ⶨʱ�����ⵥ11899 */
LONG CheckSlotPonSFPTypeCallback( short int PonPortIdx, int checkFlags )
{
    int status;
    static int flag = 0;
    int checkGwdSfpTimes, checkGwdConfirmTimes;
    long lCheckIsOK;
    long lCheckIsGWD;
    unsigned char aucDate[4];

	if( !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
		return VOS_OK;
	if( (PonPortIdx < 0) || (PonPortIdx >= MAXPON) )
		return VOS_ERROR;

	if ( checkFlags )/*������������õģ�1:pon��������0:��ʱ���*/
    {
        checkGwdSfpTimes = 2;
        checkGwdConfirmTimes = 0;
    }
    else
    {
        checkGwdSfpTimes = 1;
        checkGwdConfirmTimes = 3;
    }

    lCheckIsOK = CheckSlotPonSFPIsOK(PonPortIdx);
	status = GetPonchipWorkingStatus( PonPortIdx );
/*add by yanjy2016-12*/
/*���ȼ���ģ�������Ƿ�ƥ��*/
	/*if( (1 == GetPonPortSfpState( PonPortIdx )) && lCheckIsOK )
      {*/
      if (VOS_OK == check_sfp_online_state( PonPortIdx ))/*��λ*/
	{
		 if ( ponSfp_GetProductDate(PonPortIdx, aucDate) == ROK )/*��ȡ��ģ�����������*/
               {
                    if ( (aucDate[0] > 13) || ((aucDate[0] == 13) && (aucDate[1] >= 3)) )
                    {
                    	 if(VOS_OK ==  checkSfpTypeEponOrGpon(PonPortIdx))/*��ƥ��*/
					return VOS_OK;
                    }
               }	
	}
	else 
		return VOS_OK;
/*end: add by yanjy2016-12*/

	if( (getPonSfpTypeFromProductE2(PonPortIdx) != spfManufacturer_GWD)
        || ((status != PONCHIP_UP ) && (status != PONCHIP_TESTING)) )
	{  
	    lCheckIsGWD = TRUE;
    }
    else
    {
    	if ( ponSfp_IsOnline(PonPortIdx) == ROK )/*������GW��ģ�飬ֱ�ӷ��ص���-1�������һ������ڲ���λ*/
        {
	            do 
		     {
	                	if ( ROK == ponSfp_IsGwdType(PonPortIdx, NULL) )
	                	{
	        		    	lCheckIsGWD = TRUE;
	                	}
	                	else
	                	{
	        		    	lCheckIsGWD = FALSE;
	                	}
	            	} while( (FALSE == lCheckIsGWD) && (--checkGwdSfpTimes > 0) );
	        }
        	else
        	{
		    lCheckIsGWD = TRUE;
        	}
    }

    if ( lCheckIsOK && lCheckIsGWD )
    {

		 if(VOS_OK ==  checkSfpTypeEponOrGpon(PonPortIdx))/*����澯��Ҫ����ͬʱ����*/
		 {
			return VOS_OK;	
		 }
	    	 if( PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm != V2R1_DISABLE )
	    	 {
	    	 	/*StartupPonPort(PonPortIdx);/*undo shutdown �˿�*/
			PONTx_Enable( PonPortIdx, PONPORT_TX_SFPCHK );

			/*��Ӧ�Ĳ���pon�ڵķ�����в���*/	
			if(test1)	
			{
				if(1 == flag)
				{
					/*PONTx_Disable( PonPortIdx, PONPORT_TX_SFPCHK );*/
					PONTx_Enable( PonPortIdx, PONPORT_TX_SFPCHK );
					flag = 0;
				}	
			}
			
    		Trap_PonPortSFPTypeMismatch(PonPortIdx, V2R1_DISABLE);

    		PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_DISABLE;
    		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
    	}
    }
    else
    {
        if ( 0 < checkGwdConfirmTimes )
        {       
            if ( !lCheckIsGWD )
            {
                if ( PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter < checkGwdConfirmTimes )
                {
                    if ( ++PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter >= checkGwdConfirmTimes )
                    {
                        lCheckIsGWD = -1;
                    }
                }
            }
            else
            {
                if ( PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter > 0 )
                {
                    PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
                }
            }
        }
        else
        {
            if ( !lCheckIsGWD ) 
			lCheckIsGWD = -1;
        }

        if ( (lCheckIsGWD < 0) || (!lCheckIsOK) )
        {
        	if( PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm != V2R1_ENABLE )
        	{
        		/*���ⲿ�ִ������Σ�ֻ�Ǹ澯���رշ��⣬Ŀ�������������̵Ĺ�ģ��Ҳ����ʹ��*/
        		if(test1)
        		{
        			flag = 1;
				/*PONTx_Enable( PonPortIdx, PONPORT_TX_SFPCHK );*/
				PONTx_Disable( PonPortIdx, PONPORT_TX_SFPCHK);
        		}
			/*end:delete by yanjy*/

        		Trap_PonPortSFPTypeMismatch(PonPortIdx, V2R1_ENABLE);

        		PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_ENABLE;
        		PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
        	}
        }
    }

#if 0	/* modified by xieshl 20160520, ���ONU��ǰע�����⣬ֻ�ڸ澯ʱ���п��� */
    if ( lCheckIsGWD > 0 )
    {
		/*StartupPonPort(PonPortIdx);*/		    /* ��PON �˿�*/
		PONTx_Enable( PonPortIdx, PONPORT_TX_SFPCHK );
    }
    else if ( lCheckIsGWD < 0 )
    {
		/*ShutdownPonPort(PonPortIdx);*/		/* �ر�PON �˿�*/
		PONTx_Disable( PonPortIdx, PONPORT_TX_SFPCHK );
    }
    else
    {}
#endif
	return VOS_OK;
}
/*add by yanjy2016-12*/
/*����EPON/GPON��ģ�飬��ƥ���ϱ��澯*/
/*����:VOS_OK---��ƥ��
	    VOS_ERROR---ƥ��*/
int checkSfpTypeEponOrGpon(short int PonPortIdx)
{
	int ret;
	short int data;

	/*ponportidx��ǰ���Ѿ������*/
	if( SYS_LOCAL_MODULE_TYPE_IS_GPON )
	{
		ret = OLT_ReadI2CRegister(PonPortIdx, A0H_1010000X, 12, &data);
		
		if( PAS_EXIT_OK == ret )
		{
			 if( 0x000d == data )/*EPON��ģ��*/
			{
				if( PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm != V2R1_ENABLE )
        			{
					/*ShutdownPonPort(PonPortIdx);shutdown �˿�*/
					PONTx_Disable(PonPortIdx, PONPORT_TX_SFPCHK);

        				Trap_PonPortSFPTypeMismatch(PonPortIdx, V2R1_ENABLE);/*��ƥ��*/

        				PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_ENABLE;
        				PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
        			}
				return VOS_OK;
			}
			/*10GEPON*/
		}
	}
	else if( SYS_LOCAL_MODULE_TYPE_IS_EPON )
	{
		ret = OLT_ReadI2CRegister(PonPortIdx, A0H_1010000X, 12, &data);
		if( PAS_EXIT_OK == ret )
		{	
			 if( 0x19 == data )
			{	
				if( PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm != V2R1_ENABLE )
	        		{
					/*PONTx_Enable( PonPortIdx, PONPORT_TX_SFPCHK );*/
					/*ShutdownPonPort(PonPortIdx);shutdown �˿�*/
					PONTx_Disable(PonPortIdx, PONPORT_TX_SFPCHK);
					
        				Trap_PonPortSFPTypeMismatch(PonPortIdx, V2R1_ENABLE);

        				PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_ENABLE;
        				PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
	        		}
				return VOS_OK;	
			}		
		}
	}
		return VOS_ERROR;
}

/*add by yanjy2016-12*/


/*  4  ��ʱִ��PON ���Ƿ�֧�ֹ⹦�ʼ��*/
void CheckPonSFPTypeValid()
{
#if 0
	int status;
#endif
	short int PonPortIdx;

	if( PRODUCT_IS_HL_Series(SYS_PRODUCT_TYPE) )
	{		
		if(!SYS_LOCAL_MODULE_TYPE_IS_CPU_PON)
			return;
	}

	for( PonPortIdx = 0; PonPortIdx < MAXPON; PonPortIdx++ )
	{
        CheckSlotPonSFPTypeCallback(PonPortIdx, FALSE);
#if 0
		if(ponSfp_IsOnline(PonPortIdx) != ROK)
		{
			/* �Բ�֧��RSSI ���ܵ�PON �ڣ���֮ǰ�Ƿ�����RSSI ���ر�*/
			if(PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm != V2R1_DISABLE)
			{
				Trap_PonPortSFPTypeMismatch(PonPortIdx, V2R1_DISABLE);

				StartupPonPort(PonPortIdx);	/* ����PON �˿�*/

				PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_DISABLE;			
				PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
			}
			continue;
		}
#endif
#if 0
		/* ��ģ�����ͼ��*/
        status = ponSfp_IsGwdType(PonPortIdx, NULL);
		if( ROK != status )
		{
			if(PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm != V2R1_ENABLE)
			{
				if( PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter >= 2 )
				{
					Trap_PonPortSFPTypeMismatch(PonPortIdx, V2R1_ENABLE);

					/* modified by xieshl 2011120703�����ܹر�PON�ڣ�����һ��PON�ڻ�PON�帴λ������״̬�Ͳ����Զ�
					    �ָ��ˣ�Ӧ���ùط���ʹ�ܣ����ⵥ13757 */
					/*ShutdownPonPort(PonPortIdx);*/		/* �ر�PON �˿�*/
					PONTx_Disable( PonPortIdx );

					PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_ENABLE;
					PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
				}
				else
				{
					PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter++;
				}
			}
			else
			{
				if( OnRegOnuCounter(PonPortIdx) > 0 )
				{
					PONTx_Disable( PonPortIdx );
				}
			}
		}
		else		/* match */
		{  
			if(PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm != V2R1_DISABLE)
			{
				Trap_PonPortSFPTypeMismatch(PonPortIdx, V2R1_DISABLE);

				/*StartupPonPort(PonPortIdx);*/	/* ����PON �˿�*/
				PONTx_Enable( PonPortIdx );

				PonPortTable[PonPortIdx].PonPortmeteringInfo.SFPTypeMismatchAlarm = V2R1_DISABLE;			
				PonPortTable[PonPortIdx].PonPortmeteringInfo.sfp_type_invalid_counter = 0;
			}
		}
#endif
	}
    
	return;
}

/*int  test_pas_i2c_time( short int PonPortIdx, int n )
{
	short int i2c_device, data;
	short int gwRet;
	int i;
	int t1, t2;
	
	i2c_device = A2H_1010001X;
	t1 = VOS_GetTick();
	for(i=0; i<n; i++)
	{
		gwRet = PAS_read_i2c_register(PonPortIdx,i2c_device, SFPOUITYPEBASEADDR, &data);
	}
	t2 = VOS_GetTick();
	sys_console_printf("\r\n %% timelen=%d\r\n", t2-t1);

	return(ROK);	
}*/

#if 0
int debug_rssi_triger_time(short int PonPortIdx, bool calibrated, unsigned short   adc_measurement_start, unsigned short   adc_measurement_time  )
{
	PON_virtual_scope_i2c_params_t    i2c_params;
	i2c_params.i2c_device_id = A2H_1010001X;
	i2c_params.calibrated = calibrated; 
	i2c_params.adc_measurement_start = adc_measurement_start;
	i2c_params.adc_measurement_time = adc_measurement_time;
	return PAS_set_virtual_scope_i2c_params( PonPortIdx, i2c_params );
}


#endif

/* B--added by liwei056@2014-3-25 for BCM-PON's SFP CheckReport */
/*����ģ�����Ϣ
ֻ��:1.��ģ����λ
	2.�Ǻ��� or GW��ģ��
	3.������������13��3����ǰ
	���ش���
���������������ȷ*/
LONG CheckSlotPonSFPIsOK( short int PonPortIdx )
{
    if ( SYS_LOCAL_MODULE_TYPE_IS_TK_PONCARD_MANAGER && !SYS_LOCAL_MODULE_TYPE_IS_BCM_PONCARD_MANAGER )
    {/*pon��ִ��*/
        bool bSFPIsLoose;
        int iReadTimes = 2;/*������״ֵ̬*/
    
        while ( (iReadTimes-- > 0) && (0 == OLT_ReadGpio(PonPortIdx, OLT_GPIO_SFP_LOSS, &bSFPIsLoose)) )
        {
            if ( !bSFPIsLoose )/*��λ:0������λ:1*/
            {
            	if( ponSfp_IsHisenseType(PonPortIdx) == ROK )/*�Ǻ��ź�GW�Ĺ�ģ��*/
                {
                    unsigned char aucDate[4];
                    
                    if ( ponSfp_GetProductDate(PonPortIdx, aucDate) == ROK )/*��ȡ��ģ�����������*/
                    {
                        if ( (aucDate[0] < 13) || ((aucDate[0] == 13) && (aucDate[1] <= 3)) )
                        {
                            return FALSE;
                        }
                    }
                }
                else
                {
                    /* GPIO��ʶSFP��λ������ȴ�I2C���� */
                    if ( iReadTimes > 0 ) VOS_TaskDelay(20);
                }
            }
            else
            {
                break;
            }
        }
    }

    return TRUE;
}
/* E--added by liwei056@2014-3-25 for BCM-PON's SFP CheckReport */

/*add by yanjy2016-12*/
/*�жϹ�ģ�������EPON/GPON����������show olt information����*/
int  ponSfp_readType( short int PonPortIdx, char *sfpType)
{
	short int ret;
	short int  data;

	if(sfpType == NULL)
		return(RERROR);
	
	ret = OLT_ReadI2CRegister(PonPortIdx, A0H_1010000X, 12, &data);
	if(PAS_EXIT_OK  == ret)
	{
		if( 0x19 == data )	
			VOS_StrCpy(sfpType, "GPON");
		else if( 0x0d == data )
			VOS_StrCpy(sfpType, "EPON");
		else 
			VOS_StrCpy(sfpType, "EPON");
	}
	else
		return(RERROR);
	
	return(ROK);	
}
/*add by yanjy2016-12*/

/*added by wangjiah@2017-02-21 to get xfp type by product number*/
/** get 10GEPON xfp type symmetric(10G/10G) or asymmetric(10G/1G)
 *  params:
 *		PonPortIdx : index of pon port
 *		type :  type of 10g gepon xfp, can be SYM_10_10 or ASYM_10_1 or UNKNOWN
 *	return 
 *		ROK or Not ROK
 */
#define VENDER_PART_NUMBER_LENGTH 8 
unsigned char VENDOR_PART_NUMBER[VENDER_PART_NUMBER_LENGTH] = {'0'};
int ponSfp_getXfpType( short int PonPortIdx, int *type)
{
	short int ret = 0;
	short int data = 0;
	short int i = 0;
	_XCVR_DATA_ *pXcvrArr = &XcvrInfoArr[16];
	unsigned char **ppXgbase_prx_vendor_pn = XGBASE_PRX_PN;
	unsigned char **ppXgbase_pr_vendor_pn = XGBASE_PR_PN;
	
	if( NULL == ppXgbase_prx_vendor_pn || NULL == ppXgbase_pr_vendor_pn || NULL == type)
	{
		return RERROR;
	}

	if(PONCHIP_BCM55538 != GetPonChipTypeByPonPort(PonPortIdx))
	{
		return RERROR;
	}

	for(i = 0; i < VENDER_PART_NUMBER_LENGTH; i++)
	{
		ret = OLT_ReadI2CRegister(PonPortIdx, A0H_1010000X, pXcvrArr->cAddr + i,&data);
		if(ret != ROK){ return ret; }
		VENDOR_PART_NUMBER[i] = (unsigned char)data;
	}
	if(ROK != ret)
	{
		return ret;
	}
	VENDOR_PART_NUMBER[i] = '\0';

	while(*ppXgbase_pr_vendor_pn)
	{
		if(0 == VOS_StrnCmp(*ppXgbase_pr_vendor_pn, VENDOR_PART_NUMBER, 7))
		{
			*type = XFP_TYPE_SYM_10_10;
			return ROK;
		}
		ppXgbase_pr_vendor_pn++;
	}

	while(*ppXgbase_prx_vendor_pn)	
	{
		if(0 == VOS_StrnCmp(*ppXgbase_prx_vendor_pn, VENDOR_PART_NUMBER, 7))
		{
			*type = XFP_TYPE_ASYM_10_1;
			return ROK;
		}
		ppXgbase_prx_vendor_pn++;
	}
	*type = XFP_TYPE_UNKNOWN; 
	return ROK;
}
#endif	/* EPON_MODULE_PON_SFP_CHECK */

#ifdef	__cplusplus
}
#endif/* __cplusplus */

