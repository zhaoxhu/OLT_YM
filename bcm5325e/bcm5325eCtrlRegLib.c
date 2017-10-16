/**************************************************************************************
 *
 *  bcm5325eVlanLib.c 
 *
 *  Copyright (c) 2005-2009 GW Technologies Co., LTD.
 *  All rights reserved.
 *  
 *  modification history:
 *  ____________________
 *  2006-5-8 written by yubo
 *
 *  DESCRIPTION:
 *  bcm5325e vlan Lib.  
 *        
 *
 **************************************************************************************/
#include "bcm5325eHeader.h"

#define ERROR (-1)
#define OK 0

/* added by xieshl 20101025, Œ Ã‚µ•10756 */
int set_bcm5325e_port_enable( BYTE port, BYTE status )
{
	int ret;
	BYTE stpStatus, txStatus,rxStatus;
	
	VOS_TaskLock();  
	regPageCs(0);
	readPortCtrlReg( port, &stpStatus , &txStatus , &rxStatus );
	ret = setPortCtrlReg( port, stpStatus, status, status );	/* status : 0-enable, 1-disable */
	VOS_TaskUnlock();
	return ret;
}

/* add by tianzhy for disable port 1-4 if the SW board is SLAVE*/
int disablePonPort(void)
{
      int i;
      /*BYTE stpStatus, txStatus,rxStatus;*/
	  
      for(i=1; i<5; i++)
      {
           /*regPageCs(0);
           readPortCtrlReg( i, &stpStatus , &txStatus , &rxStatus);
	    setPortCtrlReg(i, stpStatus, 1, 1);*/ /* 1 is to disable */
	    set_bcm5325e_port_enable( i, 1 );
      }
      return OK;
}

int enablePonPort(void)
{
      int i;
      /*BYTE stpStatus, txStatus,rxStatus;*/
	  
      for(i=1; i<5; i++)
      {
           /*regPageCs(0);
           readPortCtrlReg( i, &stpStatus , &txStatus , &rxStatus);
	    setPortCtrlReg(i, stpStatus, 0, 0);*/
	    set_bcm5325e_port_enable( i, 0 );
      }
      return OK;
}	

int readPortCtrlReg(BYTE portId , BYTE *stpStatusP , BYTE *txStatusP , BYTE *rxStatusP)
{
	BYTE addr ;
	BYTE status = 0 ;
	
	addr = PORTCTRLREG_BASE + portId ;
	BCM5325EByteRegRead(addr , &status) ;
	*stpStatusP = (status & 0xE0) >> 5 ;
	*txStatusP = (status & 0x02) >> 1 ;
	*rxStatusP = (status & 0x01) ;
	return OK ;
}

int setPortCtrlReg(BYTE portId , BYTE stpStatus , BYTE txStatus , BYTE rxStatus)
{
	BYTE addr ;
	BYTE status = 0 ;
	
	addr = PORTCTRLREG_BASE + portId ;
	status =(stpStatus << 5) | (txStatus << 1) | (rxStatus) ; 
	BCM5325EByteRegWrite(addr , status) ;
	return OK ;
}



int readMIIPortCtrlReg(BYTE *stpStatusP /*, BYTE *rxUcstP , 
			BYTE *rxMcstP , BYTE *rxBcstP */, BYTE *txStatusP , BYTE *rxStatusP)
{
	BYTE addr ;
	BYTE status = 0 ;
	
	addr = MIIPORTCTRLREG ;
	BCM5325EByteRegRead(addr , &status) ; 
	*stpStatusP = (status & 0xE0) >> 5 ;
/*
	*rxUcstP = (status & 0x10) >> 4 ; 
	*rxMcstP = (status & 0x08) >> 3 ; 
	*rxBcstP = (status & 0x04) >> 2 ; 
*/
	*txStatusP = (status & 0x02) >> 1 ; 
	*rxStatusP = (status & 0x01) ; 
	return OK ;
}

int setMIIPortCtrlReg(BYTE stpStatus /*, BYTE rxUcst , 
			BYTE rxMcst , BYTE rxBcst */, BYTE txStatus , BYTE rxStatus)
{
	BYTE addr ;
	BYTE status = 0 ;
	
	addr = MIIPORTCTRLREG ;
	status = (stpStatus << 5) |/* (rxUcst << 4) | (rxMcst << 3) | (rxBcst << 2)
			|*/ (txStatus << 1) | rxStatus ; 
	BCM5325EByteRegWrite(addr , status) ; 
	return OK ;
}


int setSwitchModeReg(BYTE noBlkCd , BYTE retryLimt , BYTE fwd , BYTE manage)
{
	BYTE addr ;
	BYTE status = 0 ;
	
	addr = SWITCHMODEREG ;
	status = (noBlkCd << 3) | (retryLimt << 2) | (fwd << 1) | manage ; 
	BCM5325EByteRegWrite(addr , status) ; 
	return OK ; 
}

int readSwitchModeReg(BYTE *noBlkCdP , BYTE *retryLimtP , BYTE *fwdP , BYTE *manageP)
{
	BYTE addr ;
	BYTE status = 0 ;
	
	addr = SWITCHMODEREG ;
	BCM5325EByteRegRead(addr , &status) ; 
	*noBlkCdP = (status & 0x08) >> 3 ;
	*retryLimtP = (status & 0x04) >> 2 ; 
	*fwdP = (status & 0x02) >> 1 ; 
	*manageP = (status & 0x01) ; 
	return OK ;
}

int readMII1PortOverrideReg(BYTE *swOverrideP , BYTE *rvModeP , BYTE *flwCtrlP , 
			BYTE *speedP , BYTE *fdxP , BYTE *linkP)
{
	BYTE temp ;
	BYTE addr ;
	
	addr = MII1PORTSVERRIDEREG ;
	if(BCM5325EByteRegRead(addr , &temp) == ERROR)
		return ERROR ; 
	*swOverrideP = (temp & 0x80) >> 7 ;
	*rvModeP = (temp & 0x10) >> 4 ;
	*flwCtrlP = (temp & 0x08) >> 3 ; 
	*speedP = (temp & 0x04) >> 2 ;
	*fdxP = (temp & 0x02) >> 1 ;
	*linkP = temp & 0x01 ;
	return OK ;
}

int setMII1PortOverrideReg(BYTE swOverride , BYTE flwCtrl , BYTE speed , BYTE fdx)
{
	BYTE temp = 0 , addr ;
	
	addr = MII1PORTSVERRIDEREG ;
	if(BCM5325EByteRegRead(addr , &temp) == ERROR)
		return ERROR ;   
	temp |= (swOverride << 7)|(0x01 << 4)|(flwCtrl << 3)
		|(speed << 2)|(fdx << 1) ;
	if(BCM5325EByteRegWrite(addr , temp) == ERROR) 
	  	return ERROR ;
	return OK ;
}


void initPortCtlrl(void)
{
	BYTE portId ;
	BYTE stpStatus = 0 , txStatus = 0 , rxStatus = 0 ;
	BYTE noBlkCd = 0 , retryLimt = 0 , manage = 0 , fwd = 0 ;
	BYTE swOverride = 0 , rvMode = 0 , flwCtrl = 0 , speed = 0 ;
	BYTE fdx = 0 , link = 0 ; 

	/*set page CS */
	regPageCs(PAGE_PORTCS) ;	/*manage register*/
	/*read port ctrl 0x00-0x04*/
	for(portId = 0 ; portId < MAXPORTNUM ; portId++)
	{
		readPortCtrlReg(portId , &stpStatus , &txStatus , &rxStatus) ;
	}
	/*read MII port status 0x08*/
	readMIIPortCtrlReg(&stpStatus , &txStatus , &rxStatus) ;
	/*read switch mode 0x0B*/
	readSwitchModeReg(&noBlkCd , &retryLimt , &fwd , &manage) ;
	/*set MII1 port 0x0E*/
	setMII1PortOverrideReg(0x01 , 0 , 1 , 1) ;
	readMII1PortOverrideReg(&swOverride , &rvMode , &flwCtrl , 
			&speed , &fdx , &link) ;
}



