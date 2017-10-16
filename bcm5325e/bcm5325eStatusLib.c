/**************************************************************************************
 *
 *  bcm5325eStatusLib.c 
 *
 *  Copyright (c) 2005-2009 GW Technologies Co., LTD.
 *  All rights reserved.
 *  
 *  modification history:
 *  ____________________
 *  2006-5-8 written by yubo
 *
 *  DESCRIPTION:
 *  bcm5325e status register Lib.  
 *        
 *
 **************************************************************************************/
#include "bcm5325eHeader.h"
#define ERROR (-1)
#define OK 0
int readLinkStatus(BYTE *portStatusP)
{
	WORD temp ;
	if(BCM5325EWordRegRead(LINKSTATUSSUMREG , &temp) == ERROR)
		return ERROR ;
	portStatusP[0] = (BYTE)(temp & 0x01) ;
	portStatusP[1] = (BYTE)((temp & 0x02) >> 1) ;	
	portStatusP[2] = (BYTE)((temp & 0x04) >> 2) ;
	portStatusP[3] = (BYTE)((temp & 0x08) >> 3) ;
	portStatusP[4] = (BYTE)((temp & 0x10) >> 4) ;
	portStatusP[5] = (BYTE)((temp & 0x100) >> 8) ;
	return OK ;
}

int readLinkStatusEvent(BYTE *portEventP)
{
	WORD temp ;
	if(BCM5325EWordRegRead(LINKSTATUSCHANGEREG , &temp) == ERROR)
		return ERROR ;
	portEventP[0] = (BYTE)(temp & 0x01) ;
	portEventP[1] = (BYTE)((temp & 0x02) >> 1) ;	
	portEventP[2] = (BYTE)((temp & 0x04) >> 2) ;
	portEventP[3] = (BYTE)((temp & 0x08) >> 3) ;
	portEventP[4] = (BYTE)((temp & 0x10) >> 4) ;
	portEventP[5] = (BYTE)((temp & 0x100) >> 8) ;
	return OK ;
}

int readLinkSpeed(BYTE *portSpeedP)
{
	WORD temp ;
	if(BCM5325EWordRegRead(PORTSPEEDSUMREG , &temp) == ERROR)
		return ERROR ;
	portSpeedP[0] = (BYTE)(temp & 0x01) ;
	portSpeedP[1] = (BYTE)((temp & 0x02) >> 1) ;	
	portSpeedP[2] = (BYTE)((temp & 0x04) >> 2) ;
	portSpeedP[3] = (BYTE)((temp & 0x08) >> 3) ;
	portSpeedP[4] = (BYTE)((temp & 0x10) >> 4) ;
	portSpeedP[5] = (BYTE)((temp & 0x100) >> 8) ;
	return OK ;
}

int readDuplexStatus(BYTE *duplexStatusP)
{
		WORD temp ;
	if(BCM5325EWordRegRead(DUPLEXSTATUSSUMREG , &temp) == ERROR)
		return ERROR ;
	duplexStatusP[0] = (BYTE)(temp & 0x01) ;
	duplexStatusP[1] = (BYTE)((temp & 0x02) >> 1) ;	
	duplexStatusP[2] = (BYTE)((temp & 0x04) >> 2) ;
	duplexStatusP[3] = (BYTE)((temp & 0x08) >> 3) ;
	duplexStatusP[4] = (BYTE)((temp & 0x10) >> 4) ;
	duplexStatusP[5] = (BYTE)((temp & 0x100) >> 8) ;
	return OK ;
}

int readPauseStatus(BYTE *pauseStatusP)
{
		WORD temp ;
	if(BCM5325EWordRegRead(PAUSESTATUSSUMREG , &temp) == ERROR)
		return ERROR ;
	pauseStatusP[0] = (BYTE)(temp & 0x01) ;
	pauseStatusP[1] = (BYTE)((temp & 0x02) >> 1) ;	
	pauseStatusP[2] = (BYTE)((temp & 0x04) >> 2) ;
	pauseStatusP[3] = (BYTE)((temp & 0x08) >> 3) ;
	pauseStatusP[4] = (BYTE)((temp & 0x10) >> 4) ;
	pauseStatusP[5] = (BYTE)((temp & 0x100) >> 8) ;
	return OK ;
}

int readBITSStatus(BYTE *bitsStatusP)
{
	BYTE temp ;
	if(BCM5325EByteRegRead(PAUSESTATUSSUMREG , &temp) == ERROR)
		return ERROR ;
	*bitsStatusP = (temp & 0x02) >> 1 ;
	return OK ;
}

