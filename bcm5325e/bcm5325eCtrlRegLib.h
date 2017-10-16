#ifndef _BCM5325ECTRLREGLIB_H_
#define	_BCM5325ECTRLREGLIB_H_

#define	 MAXPORTNUM		5
#define	 PAGE_PORTCS		0


#define	PORTCTRLREG_BASE	0x00
#define MIIPORTCTRLREG	 	0x08
#define	SWITCHMODEREG	 	0x0B
#define MII1PORTSVERRIDEREG	0x0E


int readPortCtrlReg(BYTE portId , BYTE *stpStatusP , BYTE *txStatusP , BYTE *rxStatusP) ;
int setPortCtrlReg(BYTE portId , BYTE stpStatus , BYTE txStatus , BYTE rxStatus) ;
int readMIIPortCtrlReg(BYTE *stpStatusP /* , BYTE *rxUcstP , 
			BYTE *rxMcstP , BYTE *rxBcstP */, BYTE *txStatusP , BYTE *rxStatusP) ;
int setMIIPortCtrlReg(BYTE stpStatus , /*BYTE rxUcst , 
			BYTE rxMcst , BYTE rxBcst , */BYTE txStatus , BYTE rxStatus) ;
int setSwitchModeReg(BYTE noBlkCd , BYTE retryLimt , BYTE fwd , BYTE manage) ;
int readSwitchModeReg(BYTE *noBlkCdP , BYTE *retryLimtP , BYTE *fwdP , BYTE *manageP) ;
int setII1PortOverrideReg(BYTE swOverride , BYTE rvMode , BYTE flwCtrl , 
			BYTE speed , BYTE fdx , BYTE link) ;
int readMII1PortOverrideReg(BYTE *swOverrideP , BYTE *rvModeP , BYTE *flwCtrlP , 
			BYTE *speedP , BYTE *fdxP , BYTE *linkP) ;

#endif	/*_BCM5325ECTRLREGLIB_H_*/