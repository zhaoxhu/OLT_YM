/**************************************************************************************
 *
 *  bcm5325eManageLib.c 
 *
 *  Copyright (c) 2005-2009 GW Technologies Co., LTD.
 *  All rights reserved.
 *  
 *  modification history:
 *  ____________________
 *  2006-5-8 written by yubo
 *
 *  DESCRIPTION:
 *  bcm5325e manage function Lib.  
 *        
 *
 **************************************************************************************/
#include "bcm5325eHeader.h"

#define ERROR (-1)
#define OK 0
int readManagePortStatus(BYTE *statusP)
{
	BYTE temp ;
	if(BCM5325EByteRegRead(GOBLEMANAGEREG , &temp) == ERROR) 
	  	return ERROR ;
	if((temp & 0xC0) == 0x80)
		*statusP = INBANDMANAGE ;
	else if(((temp & 0xC0) == 0x40) || ((temp & 0xC0) == 0))
		*statusP = NOMANAGEPORT ;
	else
		return ERROR ;
	return OK ;
}

int setManagePortStatus(BYTE status)
{
	BYTE temp ;
	if(BCM5325EByteRegRead(GOBLEMANAGEREG , &temp) == ERROR) 
	  	return ERROR ;
/*
	if(status == INBANDMANAGE)
	(
		temp &= 0x3F ;
		temp |= 0x80 ;
	}
	else if(status == NOMANAGEPORT)
	{
		temp &= 0x3F ;
	}	
	else
	{
		return ERROR ;
	}
	if(BCM5325EByteRegWrite(GOBLEMANAGEREG , temp) == ERROR) 
	  	return ERROR ;
*/	
	return OK ;
}

int readIGMPStatus(BYTE *statusP)
{
	BYTE temp ;
	if(BCM5325EByteRegRead(GOBLEMANAGEREG , &temp) == ERROR) 
	  	return ERROR ;
	if((temp & 0x08) == 0x08)
		*statusP = ENABLEIGMP ;
	else if((temp & 0x08) == 0x00)
		*statusP = DISABLEIGMP ;
	return OK ;	
}

int setIGMPStatus(BYTE status)
{
	BYTE temp ;
	if(BCM5325EByteRegRead(GOBLEMANAGEREG , &temp) == ERROR) 
	  	return ERROR ;
/*
	if(status == ENABLEIGMP)
	(
		temp |= 0x08 ;
	}
	else if(status == DISABLEIGMP)
	{
		temp &= 0xF7 ;
	}	
	else
		return ERROR ;
	if(BCM5325EByteRegWrite(GOBLEMANAGEREG , temp) == ERROR) 
	  	return ERROR ;
*/	
	return OK ;	
}

int readRcvBPDUStatus(BYTE *statusP)
{
	BYTE temp ;
	if(BCM5325EByteRegRead(GOBLEMANAGEREG , &temp) == ERROR) 
	  	return ERROR ;
	if((temp & 0x02) == 0x02)
		*statusP = ENABLERCVBPDU ;
	else if((temp & 0x02) == 0x00)
		*statusP = DISABLERCVBPDU ;
	return OK ;		
}

int setRcvBPDUStatus(BYTE status)
{
	BYTE temp ;
	if(BCM5325EByteRegRead(GOBLEMANAGEREG , &temp) == ERROR) 
	  	return ERROR ;
/*
	if(status == ENABLERCVBPDU)
	(
		temp |= 0x02 ;
	}
	else if(status == DISABLERCVBPDU)
	{
		temp &= 0xFD ;
	}	
	else
		return ERROR ;
*/
	if(BCM5325EByteRegWrite(GOBLEMANAGEREG , temp) == ERROR) 
	  	return ERROR ;	
	return OK ;	
}

int resetMIB()
{
	BYTE temp ;
	if(BCM5325EByteRegRead(GOBLEMANAGEREG , &temp) == ERROR) 
	  	return ERROR ;
	temp |= 0x01 ;	
	if(BCM5325EByteRegWrite(GOBLEMANAGEREG , temp) == ERROR) 
	  	return ERROR ;	
	if(BCM5325EByteRegRead(GOBLEMANAGEREG , &temp) == ERROR) 
	  	return ERROR ;
	temp &= 0xFE ;	
	if(BCM5325EByteRegWrite(GOBLEMANAGEREG , temp) == ERROR) 
	  	return ERROR ;
	return OK ;
}

int readVersionID(BYTE *versionIdP)
{
	BYTE temp ;
	if(BCM5325EByteRegRead(VERSIONIDREG , &temp) == ERROR) 
	  	return ERROR ;
	*versionIdP = temp & 0x03 ;
	return OK ;
}

int readAgeTime(DWORD *ageTimeP)
{
	DWORD temp ;
	if(BCM5325EDwordRegRead(AGETIMECTRLREG , &temp) == ERROR) 
	  	return ERROR ;
	*ageTimeP = temp & MAXAGETIME ;
	return OK ;
}

int setAgeTime(DWORD ageTime)
{
	if(ageTime > MAXAGETIME)
		return ERROR ;
	if(BCM5325EDwordRegWrite(AGETIMECTRLREG , ageTime) == ERROR) 
	  	return ERROR ;
	return OK ;
}



/* ======================================= */
/*added by wutw*/
#define BCM_PORT_1_MIB_PAGE	0x20
#define BCM_PORT_2_MIB_PAGE	0x21
#define BCM_PORT_3_MIB_PAGE	0x22
#define BCM_PORT_4_MIB_PAGE	0x23
#define BCM_PORT_5_MIB_PAGE	0x24
#define BCM_PORT_MII_MIB_PAGE	0x28

#define BCD_MIB_RXUNICASETPKTS	0x06
#define BCD_MIB_TXGOODPKTS		0x00
#define BCD_MIB_TXUNICASETPKTS	0x02
#define BCD_MIB_RXGOODPKTS		0x04

#define BCM_0						0

int bcm5325eTempinit(void)
{
	/*初始化mib 为group-0 */
	BYTE value = 0;
	regPageCs((BYTE)0x02);

	
	if (OK != BCM5325EByteRegRead((BYTE)0x04 , &value))
		{
		/*printf(" 1. Rd 0x04 Error!\r\n");*/
		return ERROR;
		}	
	value &= 0xc0;
	if (OK != BCM5325EByteRegWrite((BYTE)0x04 , value))
		{
		/*printf(" Wt 0x04 Error!\r\n");*/
		return ERROR;
		}	
	value = 0;

	/*初始化mib 为group-1*/
	/*value = 0;
	regPageCs((BYTE)0x02);
	BCM5325EByteRegRead((BYTE)0x04 , &value);
	value &= 0xc0;
	value |= 0x3f;
	BCM5325EByteRegWrite((BYTE)0x04 , value);
	*/ 
	
	if (OK != BCM5325EByteRegRead((BYTE)0x04 , &value))
		{
		/*printf(" 2. Rd 0x04 Error!\r\n");*/
		return ERROR;
		}
	/*printf(" reg 0x04 : 0x%x\r\n",value);*/
	return OK;
}

#if 0	/* removed by xieshl 20100201 */
int portmib(BYTE portid)
{

	BYTE addr = 0;
	WORD dataP = 0;
	switch(portid)
		{
		case 0:
			addr = BCM_PORT_1_MIB_PAGE;
			break;
		case 1:
			addr = BCM_PORT_2_MIB_PAGE;
			break;
		case 2:
			addr = BCM_PORT_3_MIB_PAGE;
			break;
		case 3:
			addr = BCM_PORT_4_MIB_PAGE;
			break;
		case 4:
			addr = BCM_PORT_5_MIB_PAGE;
			break;
		case 5:
			addr = BCM_PORT_MII_MIB_PAGE;
			break;
			
		}
	regPageCs((BYTE)addr);	
	printf("-------------------------------\r\n");
	if (portid == 5)
		printf(" MII port(%d) mib :\r\n",portid);
	else
		printf(" port %d mib :\r\n",portid);
	
	BCM5325EWordRegRead((BYTE)BCD_MIB_TXGOODPKTS , &dataP);
	printf(" TXGOODPKTS: %d\r\n",dataP);
	dataP= 0;

	BCM5325EWordRegRead((BYTE)BCD_MIB_TXUNICASETPKTS , &dataP);
	printf(" TXUNICASETPKTS: %d\r\n",dataP);
	dataP= 0;

	BCM5325EWordRegRead((BYTE)BCD_MIB_RXGOODPKTS , &dataP);
	printf(" RXGOODPKTS: %d\r\n",dataP);
	dataP= 0;

	BCM5325EWordRegRead((BYTE)BCD_MIB_RXUNICASETPKTS , &dataP);
	printf(" RXUNICASETPKTS: %d\r\n",dataP);
	dataP= 0;
	return 0;
	}

int showmib(void)
{
	BYTE port = 0;

	for (port = 0; port < 6; port ++)
		portmib(port);

	return 0;
}


int portctrlMod(BYTE port)
{
	BYTE temp = 0;
	regPageCs((BYTE)0x0);

		if(BCM5325EByteRegRead(port , &temp) == ERROR)
			return ERROR ; 	
		printf(" 1. page 0x0  reg %d value : 0x%x \r\n",port, temp);	
		temp |= 0x23;
		if (OK != BCM5325EByteRegWrite((BYTE)port , temp))
			{
			printf(" Wt 0x04 Error!\r\n");
			return ERROR;
			}
		if(BCM5325EByteRegRead(port , &temp) == ERROR)
			return ERROR ; 	
		printf(" 2. page 0x0  reg %d value : 0x%x \r\n",port, temp);			
	return OK;
}
#endif

/*int portCtrlinit(void)
{
	BYTE port = 0;
	for(port = 0x0; port < 0x4; port++)
		{	
		if (OK != portctrlMod(port))
			return ERROR;
	}
	return OK;
}*/


/*int bcm5325ePortMibShow()
{
	BYTE addr = 0;
	WORD dataP = 0;
	BYTE portid = 0;
	for(portid = 0;portid<6; portid++)
	switch(portid)
		{
		case 0:
			addr = BCM_PORT_1_MIB_PAGE;
			break;
		case 1:
			addr = BCM_PORT_2_MIB_PAGE;
			break;
		case 2:
			addr = BCM_PORT_3_MIB_PAGE;
			break;
		case 3:
			addr = BCM_PORT_4_MIB_PAGE;
			break;
		case 4:
			addr = BCM_PORT_5_MIB_PAGE;
			break;
		case 5:
			addr = BCM_PORT_MII_MIB_PAGE;
			break;
			
		}
	regPageCs((BYTE)addr);	
	printf("-------------------------------\r\n");
	if (portid == 5)
		printf(" slot %d(MII  %d) mib :\r\n",portid, portid+3);
	else
		printf(" slot %d(port %d) mib :\r\n",portid, portid+3);
	
	BCM5325EWordRegRead((BYTE)BCD_MIB_TXGOODPKTS , &dataP);
	printf(" TXGOODPKTS: %d\r\n",dataP);
	dataP= 0;

	BCM5325EWordRegRead((BYTE)BCD_MIB_TXUNICASETPKTS , &dataP);
	printf(" TXUNICASETPKTS: %d\r\n",dataP);
	dataP= 0;

	BCM5325EWordRegRead((BYTE)BCD_MIB_RXGOODPKTS , &dataP);
	printf(" RXGOODPKTS: %d\r\n",dataP);
	dataP= 0;

	BCM5325EWordRegRead((BYTE)BCD_MIB_RXUNICASETPKTS , &dataP);
	printf(" RXUNICASETPKTS: %d\r\n",dataP);
	dataP= 0;
	return 0;
	}*/



