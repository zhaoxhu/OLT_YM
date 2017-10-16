#ifndef _BCM5325EARLLIB_H_
#define _BCM5325EARLLIB_H_

#define	PAGE_ARLACCESS	0x05

#define	ARLRWCTRLREG		0x00
#define	MACINDEXREG		0x02
#define VIDINDEXREG		0x08
#define	ARLENTRY0REG		0x10
#define	ARLENTRY1REG		0x18
#define	ARLSEARCHCTRLREG	0x20
#define	ARLSEARCHRESULTREG	0x24
#define	ARLSEARCHRESULTEXTERNREG  0x2C
#define	VIDENTRY0		0x30
#define	VIDENTRY1		0x32

#define	ENABLE_8021Q		1
#define	DISABLE_8021Q		0

#define	GOBLEARLCTRLREG		0x00
#define	BPDUMUTLIADDRREG	0x04
#define MUTLIADDR1REG		0x10
#define	MUTLIVECTOR1REG		0x16	
#define MUTLIADDR2REG		0x20
#define	MUTLIVECTOR2REG		0x26	


#define	MACADDRLEN			6
#define	TYPE_UNICAST		1
#define	TYPE_MULTICAST	2


typedef struct
{
	BYTE	valid ;
	BYTE	staticFlag ;
	BYTE	age ;
	BYTE	priority ;
	BYTE	port ;
	BYTE	macAddr[MACADDRLEN] ;
	unsigned short vlan;
}ArlEntry_t ;

extern int writeARLEntry(ArlEntry_t *arlP , BYTE type) ;
/*extern int getBcm5325EStaticMacEntry(BYTE *macP , BYTE vid , ArlEntry_t *entry , BYTE type) ;*//* removed by  xieshl 20080328 */



int readARLEntry(BYTE *macP , BYTE vid , void *arlP);

int writeARLEntry(ArlEntry_t *arlP , BYTE type) ;

int setGobleArlReg(BYTE mport , BYTE hash);

int setMacReg(BYTE addr , BYTE *macP);

int setVectorReg(BYTE addr , BYTE vector);

/*int getBcm5325EStaticMacEntry(BYTE *macP , BYTE vid , ArlEntry_t *entry , BYTE type);*//* removed by  xieshl 20080328 */

/*int Bcm5325eArlEntryShow(void);*/

int Bcm5325eV2r1StaticMacSet(void);

int Bcm5325eArlEntryRd(ArlEntry_t *entry);

int Bcm5325eArlEntryGet(BYTE arlEntryaddr, ArlEntry_t *entry);

int ARLEntryWrt(ArlEntry_t *arlP , BYTE type);

#endif	/*_BCM5325EARLLIB_H_*/	