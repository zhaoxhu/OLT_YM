/**************************************************************************************
 *
 *  bcm5325eARLLib.c 
 *
 *  Copyright (c) 2005-2009 GW Technologies Co., LTD.
 *  All rights reserved.
 *  
 *  modification history:
 *  ____________________
 *  2006-5-8 written by yubo
 *
 *  DESCRIPTION:
 *  bcm5325e ARL access Lib.  
 *        
 *
 **************************************************************************************/
#include "bcm5325eHeader.h"
#include "vxWorks.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "taskLib.h"
#include "V2R1General.h"
BYTE	gStatus8021q ;
/*BYTE    temp = 0 ;*/

extern int regSpiStatusIdle(void);
extern void spiWrite(BYTE address , BYTE *setData , BYTE nByte);
extern int	test_mii_mac_addresses_get (short int olt_id, UCHAR *host_mii_mac_addr);
extern int *GetPonChipMgmtPathMac(short int PonPortIdx);

typedef unsigned char mac_address_t [6];
int	test_mii_mac_addresses_get (short int olt_id, mac_address_t	host_mii_mac_addr)
{
	short int count = 0;
	short int temp = 0;
	char mac_6 = 0;
	int Product_type = GetOltType();

	if(host_mii_mac_addr == NULL) return 0;
	/*memcpy(host_mii_mac_addr, (unsigned char *)GetPonChipMgmtPathMac(olt_id), 6);*/
	

	if (olt_id == 20)
		{
		host_mii_mac_addr[0] = 0xff; /* Constant for PAS5001 */
		host_mii_mac_addr[1] = 0xff; /* Constant for PAS5001 */
		host_mii_mac_addr[2] = 0xff; /* Constant for PAS5001 */
		host_mii_mac_addr[3] = 0xff; /* Constant for PAS5001 */
		host_mii_mac_addr[4] = 0xff; /* Constant for PAS5001 */
		host_mii_mac_addr[5] = 0xff;
		return 0;
		}
	/* Mapping is done for Passave reference design board (65006B board). May be 
changed for other boards */
		/*{0x00, 0x00, 0x00,  0xec, 0xf0, 0xf4, 0xf8, 0xfc };*/
		host_mii_mac_addr[0] = 0x00; /* Constant for PAS5001 */
		host_mii_mac_addr[1] = 0x0C; /* Constant for PAS5001 */
		host_mii_mac_addr[2] = 0xD5; /* Constant for PAS5001 */
		host_mii_mac_addr[3] = 0x00; /* Constant for PAS5001 */
		host_mii_mac_addr[4] = 0x01; /* Constant for PAS5001 */
if(Product_type == V2R1_OLT_GFA6700)
{
	count = olt_id/4;
	temp = olt_id%4;
	switch(count)
		{
		case 0:
			mac_6 = 0xfc;
			break;
		case 1:
			mac_6 = 0xf8;
			break;
		case 2:
			mac_6 = 0xf4;
			break;
		case 3:
			mac_6 = 0xf0;
			break;
		case 4:
			mac_6 = 0xec;
			break;
		default:
			break;
		}

	host_mii_mac_addr[5] = (mac_6 + temp); 
}
else if(Product_type == V2R1_OLT_GFA6100)
{
	switch(olt_id)
		{
		case 0:
			temp = 0;
			break;
		case 1: 
			temp = 0x10;
			break;
		case 2:
			temp = 0x20;
			break;
		case 3:
			temp = 0x30;
			break;
		default:
			break;
		}

	host_mii_mac_addr[5] = temp; 

}
	return (0);

}


int readARLEntry(BYTE *macP , BYTE vid , void *arlP)
{
	BYTE temp , count = 0 , n ;
	WORD delay = 0 ;
	
	/*(1) set ARL CS*/
	regPageCs(PAGE_ARLACCESS) ;
	/*(2): set MAC Address*/
	for(n = 0 ; n < MACADDRLEN ; n++)
		if(BCM5325EByteRegWrite(MACINDEXREG + n , *(macP + n)) == ERROR)
			return ERROR ; 
	/*(3): set VID */
	if(gStatus8021q == ENABLE_8021Q)
	{
		temp |= (vid & 0x07) ;
		if(BCM5325EByteRegWrite(VIDINDEXREG , temp) == ERROR)
			return ERROR ; 	
	}
	/*(4): write ARL_R/W , and START/DONE bit*/
	temp = 0x81 ;
	if(BCM5325EByteRegWrite(ARLRWCTRLREG , temp) == ERROR)
		return ERROR ; 
	/*(5) waitting for operate completed */
	for(delay = 0 ; delay < 1000 ; delay++) ;
	if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
		return ERROR ; 
	while((temp & 0x80) != 0)
	{
		count ++ ;
		if(count > 10)			/*time out */
			return ERROR ;
		for(delay = 0 ; delay < 1000 ; delay++) ;
		if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
			return ERROR ; 
	}	
	/*(6): read Entry0 reg*/
		for(n = 0 ; n < 8 ; n++)
		if(BCM5325EByteRegRead(ARLENTRY0REG + n , ((char *)arlP + n)) == ERROR)
			return ERROR ; 	
	return OK ;
}

int writeARLEntry(ArlEntry_t *arlP , BYTE type)
{
	BYTE temp = 0 , count = 0 ;
	BYTE	data[8] = {0} ;
	WORD	delay ;
	
	/*(1) set ARL CS*/
	regPageCs(PAGE_ARLACCESS) ;
	/*(2) write Entry0 */
	memcpy(data , arlP->macAddr , MACADDRLEN) ;		/*write MAC address*/
	if(type == TYPE_UNICAST)
	{
		data[6] = arlP->port ;									/*write port*/
		data[7] = (arlP->valid << 7)|(arlP->staticFlag << 6)|(arlP->age << 5)|(arlP->priority << 3) ;  
	}
	else if(type == TYPE_MULTICAST)
	{
		data[6] = (arlP->port) & 0x3F ;									/*write port*/
		data[7] = (arlP->valid << 7)|(arlP->staticFlag << 6)|(arlP->age << 5) ;  
	}
	else
		return ERROR ;
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiWrite(ARLENTRY0REG , data , sizeof(data)) ;
	/*(3)write ARL_R/W , and START/DONE bit */
	temp = 0x80 ;
	if(BCM5325EByteRegWrite(ARLRWCTRLREG , temp) == ERROR)
		return ERROR ; 
	/*(4) waitting for operate completed */
	for(delay = 0 ; delay < 1000 ; delay++) ;
	if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
		return ERROR ; 
	while((temp & 0x80) != 0)
	{
		count ++ ;
		if(count > 10)			/*time out */
			return ERROR ;
		for(delay = 0 ; delay < 1000 ; delay++) ;
		if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
			return ERROR ; 
	}
	return OK ;
} 


/**************************GOBLE ARL REGISTER********************************/
int setGobleArlReg(BYTE mport , BYTE hash)
{
	BYTE temp = 0 ;
	temp |= (mport << 4) | hash ;
	if(BCM5325EByteRegWrite(GOBLEARLCTRLREG , temp) == ERROR)
		return ERROR ; 
	return OK ;
}

int setMacReg(BYTE addr , BYTE *macP)
{
	BYTE n ;
	for(n = 0 ; n < 6 ; n++)
		if(BCM5325EByteRegWrite(addr , *(macP + n)) == ERROR)
			return ERROR ;
	return OK ; 	
}
/*Bit0-4 , port0-4 , Bit5 MII*/
int setVectorReg(BYTE addr , BYTE vector)
{
	WORD temp = 0 ;
	temp |= (vector & 0x1F) ;
	if((vector & 0x20) == 0x20)
		temp |= 0x100 ;
	if(BCM5325EWordRegWrite(addr , vector) == ERROR)
		return ERROR ; 
	return OK ;	
}

/**********************************************************
*
*
*
*
*
*
*
**********************************************************/
#if 0	/* removed by  xieshl 20080328 */
int getBcm5325EStaticMacEntry(BYTE *macP , BYTE vid , ArlEntry_t *entry , BYTE type)
{
	BYTE data[8] = {0} ;
	
	readARLEntry(macP , vid , data) ;
	memcpy(entry->macAddr , data , MACADDRLEN) ;
	if(type == TYPE_UNICAST)
	{
		entry->port = (data[6] & 0x0F) ;
		entry->priority = (data[7] & 0x18) >> 3 ;	
		entry->age = (data[7] & 0x20) >> 5 ;	
		entry->staticFlag = (data[7] & 0x40) >> 6 ;
		entry->valid = (data[7] & 0x80) >> 7 ;
	}
	else if(type == TYPE_MULTICAST)
	{
		entry->port = (data[6] & 0x3F) ;
		entry->age = (data[7] & 0x20) >> 5 ;	
		entry->staticFlag = (data[7] & 0x40) >> 6 ;
		entry->valid = (data[7] & 0x80) >> 7 ; 
		entry->priority = 0 ;
	}
	else
		return ERROR ;
	return OK ;
}
#endif

#if 0 /* removed by xieshl 20100201 */
/*==============================================*/
/*added by wutw at august twentry four*/
int Bcm5325eArlEntryShow(void)
{
	int result = OK;
	BYTE addr = 0;
	BYTE datap = 0;
	BYTE bcm_port = 0;
	ArlEntry_t entry;
	BYTE i = 0;
	int count = 0;
	ULONG ulSlot = 0;
	
	memset(&entry, 0, sizeof(ArlEntry_t));
	result = regPageCs(PAGE_ARLACCESS);
	if (ERROR == result)
		{
			printf(" regPageCs page 0x%x Err!\r\n",PAGE_ARLACCESS);
			return result;
		}	
	result = BCM5325EByteRegRead(ARLSEARCHCTRLREG , &datap);
	if (ERROR == result)
		{
			printf(" 1. Rd addr 0x%x Err!\r\n",addr);
			return result;
		}
	datap |= 0x81;
	result = BCM5325EByteRegWrite(ARLSEARCHCTRLREG , datap);
	if (ERROR == result)
		{
			printf(" Wt addr 0x%x Err!\r\n",addr);
			return result;
		}	

	
	/*读search valid*/
	/*result = BCM5325EByteRegRead(ARLSEARCHCTRLREG , &datap);
	if (ERROR == result)
		{
			printf(" Rd addr 0x%x Err!\r\n",addr);
			return result;
		}*/
	printf("\r\n  serial       mac               slot(bcmport)   static   valid   vid \r\n");		
/*	printf("\r\n  serial       mac               portlist(0x)    static   valid     vid \r\n");
*/	printf("------------------------------------------------------------------------\r\n");

	while(OK == BCM5325EByteRegRead(ARLSEARCHCTRLREG , &datap))
		{
			if (datap & 0x1)
				{
					result = Bcm5325eArlEntryRd(&entry);
					if (result == ERROR)
						return result;
					printf("   %3d      ",count);
					for(i = 0; i < 5; i++)
						printf("%02x-",entry.macAddr[5-i]);
					printf("%02x",entry.macAddr[0]);
					
					if(0x8 == entry.port)
					{
						bcm_port = 5;
					}
					else 	
					{
						bcm_port = entry.port;					
					}
					CliBcm5325ePortConvertSlot( bcm_port, &ulSlot );
					printf("%12d(%d)", ulSlot, bcm_port);
					printf("%9d",entry.staticFlag);
					printf("%8d",entry.valid);
					printf("     %d\r\n",entry.vlan);
					count++;
				}
			if ((datap & 0x80) == 0)
				return result;
			else
				taskDelay(10);
		}
	printf("\r\n");
	return result;
	/*return (Bcm5325eArlEntryShow());*/
}
#endif
#if 0		/* removed by  xieshl 20080328 */
int arpshow()
{
	int result = 0;
	result = Bcm5325eArlEntryShow();
	return result;
}

int arpinit(void)
{
	return (Bcm5325eV2r1StaticMacSet());
}

#endif

int Bcm5325eV2r1StaticMacSet(void)
{
	int result = 0;
	short ponId = 0;
	unsigned char	host_mii_mac_addr[6];
	ArlEntry_t *arlP = NULL;
	int Product_type  = GetOltType();
	
	/*regPageCs(0x04) ;
	if(BCM5325EByteRegRead(0x0 , &temp) == ERROR)
		return ERROR ; 
	printf(" temp 0x%x\r\n",temp);*/
/*
	printf("  serial       mac               portlist(0x)    static   valid     vid \r\n");
	printf("-----------------------------------------------------------------------\t\n");
	*/
	arlP = (ArlEntry_t *)malloc(sizeof(ArlEntry_t));
	if (NULL == arlP)
		return -1;
	memset(arlP, 0, sizeof(ArlEntry_t));
	arlP->age = 0;
	arlP->priority = 0;
	arlP->staticFlag = 1;
	arlP->valid = 1;
	arlP->vlan = 1;
	/*设置cpu的mac地址*/
	#if 0
	arlP->port = 0x8;
	memcpy((arlP->macAddr), cpumac, 6);
	result = ARLEntryWrt(arlP , TYPE_UNICAST);
	/*result = ARLEntryWrt_0(arlP , TYPE_UNICAST);*/
	if (result != OK)
		return result;
	#endif
	/*设置pon芯片的mac地址*/
	if(Product_type == V2R1_OLT_GFA6700)
		{
		for(ponId = 0; ponId<20;ponId++)
			{
			/*int i = 0;*/
			test_mii_mac_addresses_get (ponId, host_mii_mac_addr);
			memcpy((arlP->macAddr), host_mii_mac_addr, 6);
			if(ponId>=0 && ponId<4)
			{
				arlP->port = 0x4;
				arlP->vlan = 4;
			}
			if(ponId>=4 && ponId<8)
			{
				arlP->port = 0x3;
				arlP->vlan = 3;
			}
			if(ponId>=8 && ponId<12)
			{
				arlP->port = 0x2;
				arlP->vlan = 2;
			}
			if(ponId>=12 && ponId<16)
			{
				arlP->port = 0x1;
				arlP->vlan = 7;
			}
			if(ponId>=16 && ponId<20)
			{
				arlP->port = 0x0;
				arlP->vlan = 8;
			}
			/*if(i/2 == 0)*/
				result = ARLEntryWrt(arlP , TYPE_UNICAST);
				/*result = ARLEntryWrt_0(arlP , TYPE_UNICAST);*/
			/*else
				result = ARLEntryWrt(arlP , TYPE_UNICAST);*/
				/*result = ARLEntryWrt_1(arlP , TYPE_UNICAST);*/
			/*if (result != OK)
				return result;*/
			}
		}
	else if(Product_type == V2R1_OLT_GFA6100)
		{
		for(ponId = 0; ponId < 4; ponId++)
			{
			test_mii_mac_addresses_get (ponId, (arlP->macAddr));
			/*memcpy((arlP->macAddr), host_mii_mac_addr, 6);*/
			if(ponId == 0)
				{
				arlP->port = 0x0;
				arlP->vlan =8;
				}
			else if(ponId == 1 )
				{
				arlP->port = 0x1;
				arlP->vlan = 7;
				}
			else if(ponId == 2 )
				{
				arlP->port = 0x2;
				arlP->vlan = 2;
				}
			else if(ponId == 3 )
				{
				arlP->port = 0x3;
				arlP->vlan = 3;
				}
			result = ARLEntryWrt(arlP , TYPE_UNICAST);
			}
		}
	
/* added by  xieshl 20080328 for GFA-SIG */
	arlP->macAddr[0] = 0x00;
	arlP->macAddr[1] = 0x01;
	arlP->macAddr[2] = 0x0b;
	arlP->macAddr[3] = 0x0f;
	arlP->macAddr[4] = 0x01;
	if(Product_type == V2R1_OLT_GFA6700)
		{
		/* slot 4 GFA-SIG */
		arlP->port = 0x0;
		arlP->vlan = 8;
		arlP->macAddr[5]=4;
		ARLEntryWrt(arlP , TYPE_UNICAST);
			
		/* slot 5 GFA-SIG */
		arlP->port = 0x1;
		arlP->vlan = 7;
		arlP->macAddr[5]=5;
		ARLEntryWrt(arlP , TYPE_UNICAST);

		/* slot 6 GFA-SIG */
		arlP->port = 0x2;
		arlP->vlan = 2;
		arlP->macAddr[5]=6;
		ARLEntryWrt(arlP , TYPE_UNICAST);

		/* slot 7 GFA-SIG */
		arlP->port = 0x3;
		arlP->vlan = 3;
		arlP->macAddr[5]=7;
		ARLEntryWrt(arlP , TYPE_UNICAST);

		/* slot 8 GFA-SIG */
		arlP->port = 0x4;
		arlP->vlan = 4;
		arlP->macAddr[5]=8;
		ARLEntryWrt(arlP , TYPE_UNICAST);
		}
	else if(Product_type == V2R1_OLT_GFA6100)
		{
		
		}
	
	free(arlP);
		

	return result;
}

#if 0	/* removed by  xieshl 20080328 */
int Bcm5325eV2r1StaticMacSet_Ivl(void)
{
	short ponId = 0;
	int result = 0;
	unsigned 	host_mii_mac_addr[6];
	ArlEntry_t *arlP = NULL;

	
	/*regPageCs(0x04) ;
	if(BCM5325EByteRegRead(0x0 , &temp) == ERROR)
		return ERROR ; 
	printf(" temp 0x%x\r\n",temp);*/
/*
	printf("  serial       mac               portlist(0x)    static   valid     vid \r\n");
	printf("-----------------------------------------------------------------------\t\n");
	*/
	arlP = (ArlEntry_t *)g_malloc(sizeof(ArlEntry_t));
	if (NULL == arlP)
		return -1;
	memset(arlP, 0, sizeof(ArlEntry_t));
	arlP->age = 0;
	arlP->priority = 0;
	arlP->staticFlag = 1;
	arlP->valid = 1;
	arlP->vlan = 1;
	/*设置cpu的mac地址*/
	#if 0
	arlP->port = 0x8;
	memcpy((arlP->macAddr), cpumac, 6);
	result = ARLEntryWrt(arlP , TYPE_UNICAST);
	/*result = ARLEntryWrt_0(arlP , TYPE_UNICAST);*/
	if (result != OK)
		return result;
	#endif
	/*设置pon芯片的mac地址*/

	for(ponId = 0; ponId<20;ponId++)
		{
		int i = 0;
		test_mii_mac_addresses_get (ponId, host_mii_mac_addr);
		memcpy((arlP->macAddr), host_mii_mac_addr, 6);
		if(ponId>=0 && ponId<4)
		{
			arlP->port = 0x4;
			arlP->vlan = 4;
		}
		if(ponId>=4 && ponId<8)
		{
			arlP->port = 0x3;
			arlP->vlan = 3;
		}
		if(ponId>=8 && ponId<12)
		{
			arlP->port = 0x2;
			arlP->vlan = 2;
		}
		if(ponId>=12 && ponId<16)
		{
			arlP->port = 0x1;
			arlP->vlan = 7;
		}
		if(ponId>=16 && ponId<20)
		{
			arlP->port = 0x0;
			arlP->vlan = 8;
		}
		if(i/2 == 0)
			result = ARLEntryWrt(arlP , TYPE_UNICAST);
			/*result = ARLEntryWrt_0(arlP , TYPE_UNICAST);*/
		else
			result = ARLEntryWrt(arlP , TYPE_UNICAST);
			/*result = ARLEntryWrt_1(arlP , TYPE_UNICAST);*/
		if (result != OK)
			return result;
		}

	return result;
}
#endif
/*wangdp*/
#if 0
int Bcm5325eArlEntryRd(ArlEntry_t *entry)
{
	int result = OK;
	BYTE addr = 0;
	BYTE datap = 0;
	unsigned short vid = 0;

	if (NULL == entry)
		return ERROR;

	
	for (addr = ARLSEARCHRESULTREG; addr <= ARLSEARCHRESULTEXTERNREG;addr ++)
		{
			result = BCM5325EByteRegRead(addr , &datap);
			if (result == ERROR)
				return result;
			switch(addr)
				{
					case ARLSEARCHRESULTREG:
						entry->macAddr[0] = datap;
						datap = 0;
						break;
					case ARLSEARCHRESULTREG + 1:
						entry->macAddr[1] = datap;
						datap = 0;
						break;
					case ARLSEARCHRESULTREG + 2:
						entry->macAddr[2] = datap;
						datap = 0;
						break;
					case ARLSEARCHRESULTREG + 3:
						entry->macAddr[3] = datap;
						datap = 0;
						break;
					case ARLSEARCHRESULTREG + 4:
						entry->macAddr[4] = datap;
						datap = 0;
						break;
					case ARLSEARCHRESULTREG + 5:
						entry->macAddr[5] = datap;
						datap = 0;
						break;
					case ARLSEARCHRESULTREG + 6:
						entry->port = datap & 0x0f;
						vid = (unsigned short)(((datap & 0xe0) >> 5) & 0x7);
						/*printf("  (ARLSEARCHRESULTREG + 6)dataP 0x%x\r\n",datap);
						printf("  (ARLSEARCHRESULTREG + 6)vid 0x%x\r\n",vid);*/
						datap = 0;
						break;
					case ARLSEARCHRESULTREG + 7:
						vid |= (unsigned short)((datap & 0x1) << 3);
						entry->vlan= vid;
						entry->age = (datap &0x20) >> 5;
						entry->staticFlag = (datap & 0x40) >> 6;
						entry->valid = (datap & 0x80) >> 7;
						
						/*printf("  (ARLSEARCHRESULTREG + 7)dataP 0x%x\r\n",datap);
						printf("  (ARLSEARCHRESULTREG + 7)vid 0x%x\r\n",vid);*/
						datap = 0;
						break;
					case ARLSEARCHRESULTREG + 8:
						entry->priority = datap & 0x3;
						datap = 0;
						break;
					default:
						return (-1);
						break;
				}
		}
	return result;
}
#endif
int Bcm5325eArlEntryGet(BYTE arlEntryaddr, ArlEntry_t *entry)
{
	int result = OK;
	BYTE addr = 0;
	/*BYTE datap = 0;*/
	BYTE datap[8] = {0};
	unsigned short vid = 0;;
	if (NULL == entry)
		return ERROR;
	
	addr = arlEntryaddr;
	result = BCM5325EEntryRead(addr, datap,8);
	if (result == ERROR)
		return result;
	entry->macAddr[0] = datap[0];
	entry->macAddr[1] = datap[1];
	entry->macAddr[2] = datap[2];
	entry->macAddr[3] = datap[3];
	entry->macAddr[4] = datap[4];
	entry->macAddr[5] = datap[5];
	entry->port = datap[6] & 0x0f;
	vid = ((datap[6] &0xe0) >> 5) && 0x7;
	vid |= (datap[7] & 0x1) << 3;
	entry->vlan= vid;
	entry->age = (datap[7] &0x20) >> 5;
	entry->staticFlag = (datap[7] & 0x40) >> 6;
	entry->valid = (datap[7] & 0x80) >> 7;
	/*for(i = 0;i<8;i++)
		printf("data[%d] 0x%x\r\n",i,datap[i]);*/
	#if 0
	if (arlEntryaddr == 0x10)
	for (addr = arlEntryaddr; addr < 8;addr ++)
		{
			result = BCM5325EByteRegRead(addr , &datap);
			if (result == ERROR)
				return result;
			switch(addr)
				{
					case 0x10:
						entry->macAddr[0] = datap;
						datap = 0;
						break;
					case 0x10 + 1:
						entry->macAddr[1] = datap;
						datap = 0;
						break;
					case 0x10 + 2:
						entry->macAddr[2] = datap;
						datap = 0;
						break;
					case 0x10 + 3:
						entry->macAddr[3] = datap;
						datap = 0;
						break;
					case 0x10 + 4:
						entry->macAddr[4] = datap;
						datap = 0;
						break;
					case 0x10 + 5:
						entry->macAddr[5] = datap;
						datap = 0;
						break;
					case 0x10 + 6:
						entry->port = datap & 0x0f;
						vid = ((datap &0xe0) >> 5) && 0x7;
						datap = 0;
						break;
					case 0x10 + 7:
						vid |= (datap & 0x1) << 3;
						entry->vlan= vid;
						entry->age = (datap &0x20) >> 5;
						entry->staticFlag = (datap & 0x40) >> 6;
						entry->valid = (datap & 0x80) >> 7;
						datap = 0;
						break;
					case 0x10 + 8:
						entry->priority = datap & 0x3;
						datap = 0;
						break;
					default:
						return (-1);
						break;
				}
	
		}
	if (arlEntryaddr == 0x10)
	for (addr = arlEntryaddr; addr < 8;addr ++)
		{
			result = BCM5325EByteRegRead(addr , &datap);
			if (result == ERROR)
				return result;
			switch(addr)
				{
					case 0x18:
						entry->macAddr[0] = datap;
						datap = 0;
						break;
					case 0x18 + 1:
						entry->macAddr[1] = datap;
						datap = 0;
						break;
					case 0x18 + 2:
						entry->macAddr[2] = datap;
						datap = 0;
						break;
					case 0x18 + 3:
						entry->macAddr[3] = datap;
						datap = 0;
						break;
					case 0x18 + 4:
						entry->macAddr[4] = datap;
						datap = 0;
						break;
					case 0x18 + 5:
						entry->macAddr[5] = datap;
						datap = 0;
						break;
					case 0x18 + 6:
						entry->port = datap & 0x0f;
						vid = ((datap &0xe0) >> 5) && 0x7;
						datap = 0;
						break;
					case 0x18 + 7:
						vid |= (datap & 0x1) << 3;
						entry->vlan= vid;
						entry->age = (datap &0x20) >> 5;
						entry->staticFlag = (datap & 0x40) >> 6;
						entry->valid = (datap & 0x80) >> 7;
						datap = 0;
						break;
					case 0x18 + 8:
						entry->priority = datap & 0x3;
						datap = 0;
						break;
					default:
						return (-1);
						break;
				}	
		}	
	#endif
	return result;
}


int ARLEntryWrt(ArlEntry_t *arlP , BYTE type)
{
	BYTE temp = 0 , count = 0 ;
	BYTE	data[8] = {0} ;
	WORD	delay ;
	BYTE	ArlEntryaddr = 0;
	BYTE	vidEntryaddr = 0;
	BYTE	bin_num = 0;
	BYTE	entry_status[2], static_bit[2];
	BYTE	i = 0;
	int		result = 0;
	ArlEntry_t entry;
	/*(1) set ARL CS*/
	
	/*(2) write Entry0 */
	/*memcpy(data , arlP->macAddr , MACADDRLEN) ;	*/	/*write MAC address*/
	for(i = 0;i<6;i++)
	{
		data[i] = arlP->macAddr[5-i];
	}
	if(type == TYPE_UNICAST)
	{
		data[6] = (arlP->port) & 0xf ;	/*write port*/
		data[7] = (arlP->valid << 7)|(arlP->staticFlag << 6)|(arlP->age << 5)|(arlP->priority << 3) ;  
	}
	else if(type == TYPE_MULTICAST)
	{
		data[6] = (arlP->port) & 0x3F ;									/*write port*/
		data[7] = (arlP->valid << 7)|(arlP->staticFlag << 6)|(arlP->age << 5) ;  
	}
	else
		return ERROR ;
	regPageCs(PAGE_ARLACCESS) ;
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
		
	/*write mac index table*/
	/*spiWrite(MACINDEXREG , arlP->macAddr , 6);*/
	spiWrite(MACINDEXREG , data , 6);
	/*write vid index table*/
	if(BCM5325EByteRegWrite(VIDINDEXREG , arlP->vlan) == ERROR)
		return ERROR ;
	
	/*(3)write ARL_R/W , and START/DONE bit */
	if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
		return ERROR ; 	
	temp |= 0x81 ;
	if(BCM5325EByteRegWrite(ARLRWCTRLREG , temp) == ERROR)
		return ERROR ; 
	/*(4) waitting for operate completed */
	for(delay = 0 ; delay < 1000 ; delay++)
	{}
	
	if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
		return ERROR ; 
	while((temp & 0x80) != 0)
	{
		count ++ ;
		if(count > 10)			/*time out */
			return ERROR ;
		for(delay = 0 ; delay < 1000 ; delay++) ;
		if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
			return ERROR ; 
	}
	
	memset(&entry, 0, sizeof(ArlEntry_t) );
	/*result = Bcm5325eArlEntryRd(&entry);
	if (result == ERROR)
		return result;*/
	
	for(bin_num = 0; bin_num<2;bin_num++)
	{
		
		ArlEntryaddr = (bin_num == 0 ? ARLENTRY0REG : ARLENTRY1REG);
		result = Bcm5325eArlEntryGet(ArlEntryaddr, &entry);
		if (result == ERROR)
			return result;
					/*printf("   %3d      ",count);
					for(i = 0; i < 5; i++)
						printf("%02x-",entry.macAddr[5-i]);
					printf("%02x",entry.macAddr[0]);
					printf("%12x",entry.port);
					printf("%12d",entry.staticFlag);
					printf("%8d",entry.valid);
					printf("     %d\r\n",entry.vlan);*/
					count++;
					
		if (entry.valid == 0)
		{
        	entry_status[bin_num] = 1/*L2_EMPTY*/;
        	continue;   /* next bin */
        }
		static_bit[bin_num] = entry.staticFlag ;
        if (memcmp(entry.macAddr, data, 6) != 0)
		{
            entry_status[bin_num] = 2/*L2_MAC_DIFF*/;    /* MAC not matched */
            continue;   /* next bin */
        }
		entry_status[bin_num] = 0/*L2_HIT*/;
	}

        if (entry_status[0] == 0/*L2_HIT*/) {
            /* overwrite */
            bin_num = 0;
        } else if (entry_status[1] == 0/*L2_HIT*/) {
            /* overwrite */
            bin_num = 1;
        } else if (entry_status[0] == 1/*L2_EMPTY*/) {
            /* write into entry 0 */
            bin_num = 0;
        } else if (entry_status[1] == 1/*L2_EMPTY*/) {
            /* write into entry 1 */
            bin_num = 1;
        } else if (!static_bit[0]) {
            bin_num = 0;
        } else if (!static_bit[1]) {
            bin_num = 1;
        } else {
            return (-1);
        }
	
	/*write ARL ENTRY0 REG*/
	/*for(ArlEntryaddr = ARLENTRY0REG; ArlEntryaddr<= 0x17;ArlEntryaddr++)
	{
		if(BCM5325EByteRegWrite(ArlEntryaddr , data[ArlEntryaddr-ARLENTRY0REG]) == ERROR)
			return ERROR ;
	}*/
	/*write ARL ENTRY1 REG*/
	/*for(ArlEntryaddr = ARLENTRY1REG; ArlEntryaddr<= 0x1F;ArlEntryaddr++)
	{
		if(BCM5325EByteRegWrite(ArlEntryaddr , data[ArlEntryaddr-ARLENTRY1REG]) == ERROR)
			return ERROR ;
	}*/
	ArlEntryaddr = (bin_num == 0 ? ARLENTRY0REG : ARLENTRY1REG);
	vidEntryaddr = (bin_num == 0 ? VIDENTRY0 : VIDENTRY1);
	spiWrite(ArlEntryaddr , data , sizeof(data));

	if(BCM5325EByteRegWrite(vidEntryaddr , arlP->vlan) == ERROR)
		return ERROR ;

	/*(3)write ARL_R/W , and START/DONE bit */
	temp = 0x80 ;
	if(BCM5325EByteRegWrite(ARLRWCTRLREG , temp) == ERROR)
		return ERROR ; 
	/*(4) waitting for operate completed */
	for(delay = 0 ; delay < 1000 ; delay++) ;
	if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
		return ERROR ; 
	while((temp & 0x80) != 0)
	{
		count ++ ;
		if(count > 10)			/*time out */
			return ERROR ;
		for(delay = 0 ; delay < 1000 ; delay++) ;
		if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
			return ERROR ; 
	}
	return OK ;
} 



int ARLEntryWrt_0(ArlEntry_t *arlP , BYTE type)
{
	BYTE temp = 0 , count = 0 ;
	BYTE	data[8] = {0} ;
	WORD	delay ;
	BYTE	i = 0;
	/*(1) set ARL CS*/
	regPageCs(PAGE_ARLACCESS) ;
	/*(2) write Entry0 */
	/*memcpy(data , arlP->macAddr , MACADDRLEN) ;	*/	/*write MAC address*/
	for(i = 0;i<6;i++)
		{
			data[i] = arlP->macAddr[5-i];
		}
	if(type == TYPE_UNICAST)
	{
		data[6] = arlP->port ;	/*write port*/
		data[7] = (arlP->valid << 7)|(arlP->staticFlag << 6)|(arlP->age << 5)|(arlP->priority << 3) ;  
	}
	else if(type == TYPE_MULTICAST)
	{
		data[6] = (arlP->port) & 0x3F ;									/*write port*/
		data[7] = (arlP->valid << 7)|(arlP->staticFlag << 6)|(arlP->age << 5) ;  
	}
	else
		return ERROR ;
	/*if(regSpiStatusIdle() != OK)		* SPI Idle *
		return ERROR ;*/
		
	/*write mac index table*/
	spiWrite(MACINDEXREG , data , 6);
	
	/*write vid index table*/
	if(BCM5325EByteRegWrite(VIDINDEXREG , arlP->vlan) == ERROR)
		return ERROR ;
	/*(3)write ARL_R/W , and START/DONE bit */
	#if 0
	temp = 0x81 ;
	if(BCM5325EByteRegWrite(ARLRWCTRLREG , temp) == ERROR)
		return ERROR ; 
	/*(4) waitting for operate completed */
	for(delay = 0 ; delay < 1000 ; delay++) ;
	if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
		return ERROR ; 
	while((temp & 0x80) != 0)
	{
		count ++ ;
		if(count > 10)			/*time out */
			return ERROR ;
		for(delay = 0 ; delay < 1000 ; delay++) ;
		if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
			return ERROR ; 
	}
	#endif
	/*write ARL ENTRY0 REG*/
	/*for(ArlEntryaddr = ARLENTRY0REG; ArlEntryaddr<= 0x17;ArlEntryaddr++)
	{
		if(BCM5325EByteRegWrite(ArlEntryaddr , data[ArlEntryaddr-ARLENTRY0REG]) == ERROR)
			return ERROR ;
	}*/
	/*write ARL ENTRY1 REG*/
	/*for(ArlEntryaddr = ARLENTRY1REG; ArlEntryaddr<= 0x1F;ArlEntryaddr++)
	{
		if(BCM5325EByteRegWrite(ArlEntryaddr , data[ArlEntryaddr-ARLENTRY1REG]) == ERROR)
			return ERROR ;
	}*/
	
	spiWrite(ARLENTRY0REG , data , sizeof(data));
	/*spiWrite(ARLENTRY1REG , data , sizeof(data));*/
	if(BCM5325EByteRegWrite(VIDENTRY0 , arlP->vlan) == ERROR)
		return ERROR ;

	/*(3)write ARL_R/W , and START/DONE bit */
	temp = 0x80 ;
	if(BCM5325EByteRegWrite(ARLRWCTRLREG , temp) == ERROR)
		return ERROR ; 
	/*(4) waitting for operate completed */
	for(delay = 0 ; delay < 1000 ; delay++) ;
	if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
		return ERROR ; 
	while((temp & 0x80) != 0)
	{
		count ++ ;
		if(count > 10)			/*time out */
			return ERROR ;
		for(delay = 0 ; delay < 1000 ; delay++) ;
		if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
			return ERROR ; 
	}
	return OK ;
} 

int ARLEntryWrt_1(ArlEntry_t *arlP , BYTE type)
{
	BYTE temp = 0 , count = 0 ;
	BYTE	data[8] = {0} ;
	WORD	delay ;
	BYTE	i = 0;
	/*(1) set ARL CS*/
	regPageCs(PAGE_ARLACCESS) ;
	/*(2) write Entry0 */
	/*memcpy(data , arlP->macAddr , MACADDRLEN) ;	*/	/*write MAC address*/
	for(i = 0;i<6;i++)
		{
			data[i] = arlP->macAddr[5-i];
		}
	if(type == TYPE_UNICAST)
	{
		data[6] = arlP->port ;	/*write port*/
		data[7] = (arlP->valid << 7)|(arlP->staticFlag << 6)|(arlP->age << 5)|(arlP->priority << 3) ;  
	}
	else if(type == TYPE_MULTICAST)
	{
		data[6] = (arlP->port) & 0x3F ;									/*write port*/
		data[7] = (arlP->valid << 7)|(arlP->staticFlag << 6)|(arlP->age << 5) ;  
	}
	else
		return ERROR ;
	
	/*if(regSpiStatusIdle() != OK)		* SPI Idle *
		return ERROR ;*/
	/*write mac index table*/
	spiWrite(MACINDEXREG , arlP->macAddr , 6);
	
	/*write vid index table*/
	if(BCM5325EByteRegWrite(VIDINDEXREG , arlP->vlan) == ERROR)
		return ERROR ;
	/*(3)write ARL_R/W , and START/DONE bit */
	
	temp = 0x81 ;
	if(BCM5325EByteRegWrite(ARLRWCTRLREG , temp) == ERROR)
		return ERROR ; 
	/*(4) waitting for operate completed */
	for(delay = 0 ; delay < 1000 ; delay++) ;
	if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
		return ERROR ; 
	while((temp & 0x80) != 0)
	{
		count ++ ;
		if(count > 10)			/*time out */
			return ERROR ;
		for(delay = 0 ; delay < 1000 ; delay++) ;
		if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
			return ERROR ; 
	}
	


	
	/*write ARL ENTRY0 REG*/
	/*for(ArlEntryaddr = ARLENTRY0REG; ArlEntryaddr<= 0x17;ArlEntryaddr++)
	{
		if(BCM5325EByteRegWrite(ArlEntryaddr , data[ArlEntryaddr-ARLENTRY0REG]) == ERROR)
			return ERROR ;
	}*/
	/*write ARL ENTRY1 REG*/
	/*for(ArlEntryaddr = ARLENTRY1REG; ArlEntryaddr<= 0x1F;ArlEntryaddr++)
	{
		if(BCM5325EByteRegWrite(ArlEntryaddr , data[ArlEntryaddr-ARLENTRY1REG]) == ERROR)
			return ERROR ;
	}*/
	
	spiWrite(ARLENTRY1REG , data , sizeof(data));
	/*spiWrite(ARLENTRY1REG , data , sizeof(data));*/
	if(BCM5325EByteRegWrite(VIDENTRY1 , arlP->vlan) == ERROR)
		return ERROR ;
	
	/*(3)write ARL_R/W , and START/DONE bit */
	temp = 0x80 ;
	if(BCM5325EByteRegWrite(ARLRWCTRLREG , temp) == ERROR)
		return ERROR ; 
	/*(4) waitting for operate completed */
	for(delay = 0 ; delay < 1000 ; delay++) ;
	if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
		return ERROR ; 
	while((temp & 0x80) != 0)
	{
		count ++ ;
		if(count > 10)			/*time out */
			return ERROR ;
		for(delay = 0 ; delay < 1000 ; delay++) ;
		if(BCM5325EByteRegRead(ARLRWCTRLREG , &temp) == ERROR)
			return ERROR ; 
	}
	return OK ;
} 



#if 0
int arpWrt(BYTE *mac, WORD vid, BYTE port, WORD status))
{
    u8_t bin_num, unit_idx;
    u8_t addr;
    u8_t entry_data[ARL_ENTRY_LEN];
    u8_t entry_status[2], static_bit[2];
    u8_t swap_mac[MAC_ADDRESS_LEN];   

	memcpy(swap_mac, mac, 6);
        for(bin_num = 0; bin_num < 2; bin_num++) {
            addr = (bin_num == 0 ? ARLENTRY0REG : ARLENTRY1REG);
            readreg(unit_idx, ARL_ACCESS_REG, addr, ARL_ENTRY_LEN, entry_data);

            if (!(entry_data[MAC_ADDRESS_LEN+1] & ARL_VALID_BIT)) {
                entry_status[bin_num] = L2_EMPTY;
                continue;   /* next bin */
            }

            /* bit[2] - static bit. (ARL entry register Bit[62] ) */
            static_bit[bin_num] =
                  (entry_data[MAC_ADDRESS_LEN+1] & ARL_STATIC_BIT);

            if (memcmp(entry_data, swap_mac, MAC_ADDRESS_LEN) != 0) {
                entry_status[bin_num] = L2_MAC_DIFF;    /* MAC not matched */
                continue;   /* next bin */
            }

            /* SUCCESS, Both MAC and VID is matched=>found */
            entry_status[bin_num] = L2_HIT;
        }


}
#endif

/* removed by xieshl 20100201 */
/*int modifiedvidreg(void)
{
	BYTE temp = 0;
	regPageCs(0x34) ;
	if(BCM5325EByteRegRead(0x0 , &temp) == ERROR)
		return ERROR ; 
	printf(" 1. page 0x34, reg : 0x0  value = 0x%x\r\n",temp);
	temp &= 0x9f;
	printf(" page 0x34, reg : 0x0  Wrt-vl = 0x%x\r\n",temp);
	if(BCM5325EByteRegWrite(0x0 , temp) == ERROR)
		return ERROR ; 	

	if(BCM5325EByteRegRead(0x0 , &temp) == ERROR)
		return ERROR ; 
	printf(" 2. page 0x34, reg : 0x0  value = 0x%x\r\n",temp);
	
	return 0;
}*/

