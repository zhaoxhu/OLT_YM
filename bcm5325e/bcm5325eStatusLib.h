#ifndef _BCM5325ESTATUSLIB_H_
#define	_BCM5325ESTATUSLIB_H_

#define	LINKSTATUSSUMREG	0x00
#define	LINKSTATUSCHANGEREG	0x02
#define	PORTSPEEDSUMREG		0x04
#define	DUPLEXSTATUSSUMREG	0x06
#define	PAUSESTATUSSUMREG	0x08
#define	BITSSTSTUSREG		0x46

int readLinkStatus(BYTE *portStatusP) ;
int readLinkStatusEvent(BYTE *portEventP) ;
int readLinkSpeed(BYTE *portSpeedP) ;
int readDuplexStatus(BYTE *duplexStatusP) ;
int readPauseStatus(BYTE *pauseStatusP) ;
int readBITSStatus(BYTE *bitsStatusP) ;

#endif	/*_BCM5325ESTATUSLIB_H_*/