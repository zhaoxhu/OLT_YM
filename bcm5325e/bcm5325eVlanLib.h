#ifndef _BCM5325EVLANLIB_H_
#define	_BCM5325EVLANLIB_H_
#include "bcm5325eHeader.h"
/*************PORT_BASED VLAN********/

#define PAGE_PORTBASEVLAN	0x31

#define	PORTBASEVLAN_BASE	0x00
#define	VLANCTRLREG_PORT_0	0x00
#define	VLANCTRLREG_PORT_1	0x02
#define	VLANCTRLREG_PORT_2	0x04
#define	VLANCTRLREG_PORT_3	0x06
#define	VLANCTRLREG_PORT_4	0x08
#define	VLANCTRLREG_PORT_MII	0x10
#define	VLANCTRLREG_PORT_SERIAL	0x12

#define	MAXPORTNUMBER		6

#define	MII_PORTID		5



/******************802.1q VLAN*********/
#define	ENABLE_8021Q		1
#define	DISABLE_8021Q		0

#define	VLANLEARN_SVL		0
#define	VLANLEARN_IVL		3

#define	FRAME_CTRL_FWD		0
#define	FRAME_CTRL_REPRI	1
#define	FRAME_CTRL_REVID	2
#define	FRAME_CTRL_REALL	3


typedef struct {
	WORD vid;
	BYTE portlist;
	BYTE untagMap;
	BYTE valid;
	} VLAN8021QENTRY;

int setPortBaseVlan(BYTE portAddr , WORD vlanMember) ;

int readPortBaseVlan(BYTE portAddr , WORD *vlanMemberP) ;

/*int initPortVlan(void) ;*/

int  setPortPvid(BYTE portId , WORD pvid);

int readPortPvid(BYTE portId , WORD *pvidP);

int miiPortInit(void );

int vlanEntryWrt( VLAN8021QENTRY *entry);

int vlanDefaultInit(void);

int vlan8021QCreate( WORD vid );

int vlan8021QRemove( WORD vid );

int vlan801QEntryGet(WORD vid, VLAN8021QENTRY *entry);

int vlan8021QPortAdd( WORD vid, BYTE port, BYTE untagMap);

int vlan8021QPortDel( WORD vid, BYTE port);

/*int vlanInit(void);*/
int vlan8021QVlanInit(void);
/*int vlan8021QShow(void);*/
int bcm5325eVlanConfig(int Product_type);


#endif	/*_BCM5325EVLANLIB_H_*/
