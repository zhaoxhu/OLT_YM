#ifdef	__cplusplus
extern "C"
{
#endif

#include "syscfg.h"

#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
#include "cli/cli.h"
#include "cli/cl_cmd.h"
#include "cli/cl_mod.h"
#include "sys/console/sys_console.h"
#include "sys/main/sys_main.h"
#include "Vos_string.h"
#include "Vos_task.h"

#include  "OltGeneral.h"
#include "bcm5325eCli.h"
/*#include "../superset/platform/include/man/cli/cli.h"*/

/*#include "Vos_sem.h"*/
/*#include "vxWorks.h"*/
/*#include "string.h"
#include "taskLib.h"*/
/*#include "bcm5325eHeader.h"*/
#include "V2R1General.h"

#ifndef OK
#define OK 	0
#endif
#ifndef ERROR
#define ERROR (-1)
#endif
#define VLAN_MAX 	16
#define VLAN_VALID	1
#define VLAN_INVALID	0
#define	TYPE_UNICAST		1

#define	PAGE_ARLACCESS	0x05
#define	ARLSEARCHCTRLREG	0x20
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

#define BCM_BRD_MASTER_SLOT			3
#define BCM_BRD_MASTER_BACK_SLOT	4
#define BCM_MASTER_SELF_BROAD_PORT	5
#define BCM_MASTER_BACK_BROAD_PORT	4
#define BCM_MII_PORT				5

#define BCM_MASTER_3SLOT_VLAN		5
#define BCM_MASTER_4SLOT_VLAN		4

mac_address_t master_2Slot_mac = {0x00,0x05,0x3b,0xff,0x01,0x02};
mac_address_t master_3Slot_mac = {0x00,0x05,0x3b,0xff,0x01,0x03};
mac_address_t master_4Slot_mac = {0x00,0x05,0x3b,0xff,0x01,0x04};
/*
typedef struct
{
 unsigned char bMacAdd[6];

}Enet_MACAdd_Infor1;  
*/
typedef struct
{
	BYTE	valid ;
	BYTE	staticFlag ;
	BYTE	age ;
	BYTE	priority ;
	BYTE	port ;
	BYTE	macAddr[6] ;
	unsigned short vlan;
}ArlEntry_t ;
extern WORD vlanInfo[VLAN_MAX];
extern LONG GetMacAddr(CHAR * szStr, CHAR * pucMacAddr);
extern int vty_out (struct vty *, const char *, ...);
/*extern Enet_MACAdd_Infor1 *funReadMacAddFromNvram( Enet_MACAdd_Infor1 *);*/

extern int setPortBaseVlan(BYTE portAddr , WORD vlanMember) ;

extern int readPortBaseVlan(BYTE portAddr , WORD *vlanMemberP) ;

/*extern int initPortVlan(void) ;*/

extern int  setPortPvid(BYTE portId , WORD pvid);

extern int readPortPvid(BYTE portId , WORD *pvidP);

extern int vlanDefaultInit(void);

extern int vlan8021QCreate( WORD vid );

extern int vlan8021QRemove( WORD vid );

extern int vlan8021QPortAdd( WORD vid, BYTE port, BYTE untagMap);

extern int vlan8021QPortDel( WORD vid, BYTE port);

/*extern int Bcm5325eArlEntryShow(void);*/
extern int Bcm5325eV2r1StaticMacSet(void);
extern int vlan8021QVlanInit(void);
/*extern int vlan8021QShow(void);
extern int bcm5325eVlanConfig(void);*/

extern int miiPortInit(void);
extern int bcm5325eVlanConfig(int type);

extern int ARLEntryWrt(ArlEntry_t *arlP , BYTE type);

extern int regPageCs(BYTE pageAddr);
extern int BCM5325EWordRegRead(BYTE addr , WORD *dataP) ;
extern int BCM5325EByteRegRead(BYTE addr , BYTE *dataP) ;
extern int BCM5325EByteRegWrite(BYTE addr , BYTE value) ;
extern int BCM5325EWordRegWrite(BYTE addr , WORD value) ;
extern int BCM5325EDwordRegWrite(BYTE addr , DWORD value) ;
extern int BCM5325EEntryRead(BYTE addr , BYTE *dataP, BYTE dataLen) ;
extern int BCM5325EEntryWrt(BYTE addr , BYTE *dataP, BYTE dataLen);
extern int bcm5635eRegWrt(BYTE addr_page, BYTE addr_reg, BYTE value);
extern int Bcm5325eArlEntryRd(ArlEntry_t *entry);
	
int CliBcm5325eArpVtyShow(struct vty* vty);

int cliBcm5325ePvidVtyShow(struct vty* vty);

int cliBcm5325eStaticMacAdd(unsigned short vid, char *mac, unsigned char port);

int cliBcm5325eStaticMacDel(unsigned short vid, char *mac);

int cliBcm5325eVlanCreate(unsigned short vid);

int cliBcm5325eVlanRemove(unsigned short vid);

int cliBcm5325eVlanPortAdd(unsigned short vid, unsigned char port, unsigned char untagFlag);

int cliBcm5325eVlanPortDel(unsigned short vid, unsigned char port);

int cliBcm5325eVlanPvidSet(unsigned short pvid, unsigned char port);

int Clibcm5325eVlanVtyShow(struct vty* vty);

#if 0
#endif

int CliBcm5325ePortConvertSlot( unsigned char bcm_port, ULONG *ulSlot)
{
	if (ulSlot == NULL)
		return ERROR;
	if (bcm_port > 6)
		return ERROR;
	switch(bcm_port)
	{
		case 0:
			*ulSlot = 4;
			break;
		case 1:
			*ulSlot = 5;
			break;
		case 2:
			*ulSlot = 6;
			break;
		case 3:
			*ulSlot = 7;
			break;
		case 4:
			*ulSlot = 8;
			break;
		case 5:
			*ulSlot = 3;
		default:
			return ERROR;
	}
	return OK;
}

int CliBcm5325eSlotConvertPort( ULONG ulSlot, unsigned char *bcm_port )
{
	if (bcm_port == NULL)
		return ERROR;
	if (ulSlot > 8)
		return ERROR;
	if (ulSlot < 3)
		return ERROR;

	switch(ulSlot)
	{
		case 3:
			*bcm_port = 5;	
			break;
		case 4:
			*bcm_port = 0;
			break;
		case 5:
			*bcm_port = 1;
			break;
		case 6:
			*bcm_port = 2;
			break;
		case 7:
			*bcm_port = 3;
			break;
		case 8:
			*bcm_port = 4;
			break;
		default:
			return ERROR;
	}
	
	return OK;
}


int CliBcm5325eArpVtyShow(struct vty *vty)
{
	int result = OK;
	BYTE addr = 0;
	BYTE datap = 0;
	BYTE bcm_port = 0;
	ArlEntry_t entry;
	int i = 0;
	int count = 0;
	ULONG ulSlot = 0;
	
	memset(&entry, 0, sizeof(ArlEntry_t));
	result = regPageCs(PAGE_ARLACCESS);
	if (ERROR == result)
		{
			vty_out( vty," regPageCs page 0x%x Err!\r\n",PAGE_ARLACCESS);
			return result;
		}	
	result = BCM5325EByteRegRead(ARLSEARCHCTRLREG , &datap);
	if (ERROR == result)
		{
			vty_out( vty," 1. Rd addr 0x%x Err!\r\n",addr);
			return result;
		}
	datap |= 0x81;
	result = BCM5325EByteRegWrite(ARLSEARCHCTRLREG , datap);
	if (ERROR == result)
		{
			vty_out( vty," Wt addr 0x%x Err!\r\n",addr);
			return result;
		}	

	
	/*读search valid*/
	/*result = BCM5325EByteRegRead(ARLSEARCHCTRLREG , &datap);
	if (ERROR == result)
		{
			vty_out( vty," Rd addr 0x%x Err!\r\n",addr);
			return result;
		}*/
	vty_out( vty,"\r\n  serial       mac               slot(bcmport)   static   valid   vid \r\n");		
	/*vty_out( vty,"------------------------------------------------------------------------\r\n");*/

	while(OK == BCM5325EByteRegRead(ARLSEARCHCTRLREG , &datap))
	{
			if (datap & 0x1)
				{
					/*result = Bcm5325eArlEntryRd(&entry);*/
					if (result == ERROR)
						return result;
					vty_out( vty,"   %3d      ",count);
					vty_out( vty,"%02x%02x.%02x%02x.%02x%02x",entry.macAddr[0],entry.macAddr[1],entry.macAddr[2],entry.macAddr[3],entry.macAddr[4],entry.macAddr[5]);
					vty_out( vty,"%02x",entry.macAddr[0]);
					
					if(0x8 == entry.port)
					{
						bcm_port = 5;
					}
					else 	
					{
						bcm_port = entry.port;					
					}
					CliBcm5325ePortConvertSlot( bcm_port, &ulSlot );
					vty_out( vty,"%12d(%d)%9d%8d     %d\r\n", ulSlot, bcm_port,entry.staticFlag,entry.valid,entry.vlan);
					count++;
				}
			if ((datap & 0x80) == 0)
				return result;
			/*else
				VOS_TaskDelay(10);*/
			
		if( i > 128 )	/* modified by xieshl 20120406, 目前是死循环 */
			break;
		i++;
	}
	vty_out( vty,"\r\n");
	return result;
	/*return (Bcm5325eArlEntryShow());*/
}


int CliBcm5325eArpVtyShow_Svl(struct vty *vty)
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
			vty_out( vty," regPageCs page 0x%x Err!\r\n",PAGE_ARLACCESS);
			return result;
		}	
	result = BCM5325EByteRegRead(ARLSEARCHCTRLREG , &datap);
	if (ERROR == result)
		{
			vty_out( vty," 1. Rd addr 0x%x Err!\r\n",addr);
			return result;
		}
	datap |= 0x81;
	result = BCM5325EByteRegWrite(ARLSEARCHCTRLREG , datap);
	if (ERROR == result)
		{
			vty_out( vty," Wt addr 0x%x Err!\r\n",addr);
			return result;
		}	

	
	/*读search valid*/
	/*result = BCM5325EByteRegRead(ARLSEARCHCTRLREG , &datap);
	if (ERROR == result)
		{
			vty_out( vty," Rd addr 0x%x Err!\r\n",addr);
			return result;
		}*/
#if 0		
	vty_out( vty,"\r\n  serial       mac               slot(bcmport)   static   valid   vid \r\n");		
/*	vty_out( vty,"\r\n  serial       mac               portlist(0x)    static   valid     vid \r\n");
*/	vty_out( vty,"------------------------------------------------------------------------\r\n");
#endif
	vty_out( vty,"\r\n  serial       mac               slot(bcmport)   static   valid\r\n");		

	while(OK == BCM5325EByteRegRead(ARLSEARCHCTRLREG , &datap))
		{
			if (datap & 0x1)
				{
					/*result = Bcm5325eArlEntryRd(&entry);*/
					if (result == ERROR)
						return result;
					vty_out( vty,"   %3d      ",count);
					for(i = 0; i < 5; i++)
						vty_out( vty,"%02x-",entry.macAddr[5-i]);
					vty_out( vty,"%02x",entry.macAddr[0]);
					
					if(0x8 == entry.port)
					{
						bcm_port = 5;
					}
					else 	
					{
						bcm_port = entry.port;					
					}
					CliBcm5325ePortConvertSlot( bcm_port, &ulSlot );
					vty_out( vty,"%12d(%d)", ulSlot, bcm_port);
					vty_out( vty,"%9d",entry.staticFlag);
					vty_out( vty,"%8d",entry.valid);
					/*vty_out( vty,"     %d\r\n",entry.vlan);*/
					count++;
				}
			if ((datap & 0x80) == 0)
				return result;
			else
				VOS_TaskDelay(10);
		}
	vty_out( vty,"\r\n");
	return result;
	/*return (Bcm5325eArlEntryShow());*/
}



int cliBcm5325ePvidVtyShow(struct vty *vty)
{
	BYTE port = 0;
	WORD pvid = 0;
	BYTE bcm_port = 0;
	ULONG ulSlot = 0;
	
	vty_out( vty, "\r\n  slot(port) pvid\r\n");
	vty_out( vty, "------------------\r\n");
	for(port = 0; port<6; port++)
	{
		
		readPortPvid( port , &pvid );
		if ( 5 == port)
		{
			bcm_port = 5;
		}
		else
			bcm_port = port;
		CliBcm5325ePortConvertSlot( bcm_port, &ulSlot );
		vty_out( vty, "   %d(%d)      %d\r\n",ulSlot,bcm_port,pvid);
	}
	return OK;
}

int cliBcm5325eStaticMacAdd(unsigned short vid, char *mac, unsigned char port/*, struct vty *vty*/)
{
	ArlEntry_t arl;
	BYTE vFlag = 0;
	int i = 0;
	if (NULL == mac)
		return ERROR;
	if (vid >4095)
		return ERROR;
	if (port > 6)
		return ERROR;

	if (vid > 4095)
		return (-2);
	for(i = 0; i<VLAN_MAX; i++)
	{
		if(vid == vlanInfo[i])
		{
			vFlag = 1;
			break;
		}
	}
	if (vFlag == 0)
		return (-5);	
	
	memset(&arl, 0, sizeof(ArlEntry_t));
	memcpy((arl.macAddr), mac, 6);
	if (port == BCM_MII_PORT) 
		arl.port = 0x8;
	else
		arl.port = port;
	arl.vlan = vid;
	arl.staticFlag = 1;
	arl.valid = 1;
	if (OK != ARLEntryWrt( &arl , TYPE_UNICAST))
		return ERROR;
	return OK;
	
}

int cliBcm5325eStaticMacDel(unsigned short vid, char *mac)
{
	ArlEntry_t arl;
	int i = 0;
	int vFlag = 0;
	if (NULL == mac)
		return ERROR;
	if (vid > 4095)
		return (-2);
	for(i = 0; i<VLAN_MAX; i++)
	{
		if(vid == vlanInfo[i])
		{
			vFlag = 1;
			break;
		}
	}
	if (vFlag == 0)
		return (-5);		
	memset(&arl, 0, sizeof(ArlEntry_t));
	memcpy((arl.macAddr), mac, 6);
	arl.port = 0;
	arl.vlan = vid;
	arl.staticFlag = 1;
	arl.valid = 0;
	if (OK != ARLEntryWrt( &arl , TYPE_UNICAST))
		return ERROR;
	return OK;
	
}

int cliBcm5325eVlanCreate(unsigned short vid)
{
	int iRes = 0;
	if (vid > 4095)
		return ERROR;
	if (OK != (iRes = vlan8021QCreate(  vid )))
		return iRes;
	return OK;
}

int cliBcm5325eVlanRemove(unsigned short vid)
{
	int iRes = 0;
	if (vid > 4095)
		return ERROR;
	if (OK != (iRes = vlan8021QRemove(  vid )))
		return iRes;
	return OK;
}

int cliBcm5325eVlanPortAdd(unsigned short vid, unsigned char port, unsigned char untagFlag)
{
	int iRes = (-1);
	if (OK != (iRes = vlan8021QPortAdd(  vid,  port,  untagFlag)))
		return iRes;
	return OK;
}

int cliBcm5325eVlanPortDel(unsigned short vid, unsigned char port)
{
	int iRes = (-1);
	if (OK != (iRes = vlan8021QPortDel( vid, port)))
		return iRes;
	return OK;
}

int cliBcm5325eVlanPvidSet(unsigned short pvid, unsigned char port)
{
	int iRes = (-1); 
	if (OK != (iRes = setPortPvid( port,  pvid)))
		return iRes;
	return OK;
}



int cliBcm5325ePortMibShow(struct vty* vty)
{
	BYTE addr = 0;
	WORD dataP = 0;
	BYTE portid = 0;
	ULONG ulSlot = 0;
	for(portid = 0;portid<6; portid++)
	{
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
		default:
			break;
	}
	regPageCs((BYTE)addr);	
	vty_out( vty,"-------------------------------\r\n");
	if (portid == 5)
	{
		CliBcm5325ePortConvertSlot( portid, &ulSlot );
		vty_out( vty," slot %d(MII  %d) mib :\r\n",ulSlot, portid);
	}
	else
	{
		CliBcm5325ePortConvertSlot( portid, &ulSlot );
		vty_out( vty," slot %d(port %d) mib :\r\n",ulSlot, portid);
	}
	
	BCM5325EWordRegRead((BYTE)BCD_MIB_TXGOODPKTS , &dataP);
	vty_out( vty," TXGOODPKTS: %d\r\n",dataP);
	dataP= 0;

	BCM5325EWordRegRead((BYTE)BCD_MIB_TXUNICASETPKTS , &dataP);
	vty_out( vty," TXUNICASETPKTS: %d\r\n",dataP);
	dataP= 0;

	BCM5325EWordRegRead((BYTE)BCD_MIB_RXGOODPKTS , &dataP);
	vty_out( vty," RXGOODPKTS: %d\r\n",dataP);
	dataP= 0;

	BCM5325EWordRegRead((BYTE)BCD_MIB_RXUNICASETPKTS , &dataP);
	vty_out( vty," RXUNICASETPKTS: %d\r\n",dataP);
	dataP= 0;

	}
	return 0;
}
int Clibcm5325eVlanVtyShow(struct vty* vty)
{

	WORD rd_value = 0;
	WORD vid = 0;
	WORD tempvid = 0;
	WORD count = 0;
	BYTE data[4] = {0};
	ULONG ulSlot = 0;
	BYTE portlist = 0;
	BYTE ungtaglist = 0;
	BYTE valid = 0;
	BYTE i = 0;
	BYTE j = 0;
	

	vty_out( vty,"\r\n  vid  slot(bcm port)  tag    valid\r\n");
	vty_out( vty,"  ----------------------------------\r\n");	
	for(i = 0; i<VLAN_MAX; i++)
	{	
		if(0 == vlanInfo[i])
		{
			continue;
		}
		vid = vlanInfo[i];

   	 	data[0] = (BYTE) vid;
    	data[1] = (BYTE) (vid >> 8);
    /* RW_CTRL_ADDR_BIT: Bit[12], Read/Write Control, Read(0)/Write(1) */
    /* REQUEST_ADDR_BIT: Bit[13], Access Request, request(1) */
    	data[1] &= (~(BYTE)0x10);
    	data[1] |= (BYTE)0x20;	
		regPageCs(0x34) ;
		if (OK != BCM5325EEntryWrt(0x06, data, 2))
		{
			/*vty_out( vty,(" 1. Wt 0x%x Err!\r\n",ctrl_value);*/
			return ERROR;
		}		
		do
		{
			if (count>10)
				return ERROR;
			if( OK != BCM5325EWordRegRead(0x06, &rd_value))
				return ERROR;
			if ( (rd_value & 0x2000 )== 0 )
				break;
			for(j = 0;j<1000;j++)
				count++;
			
		}while(1);	

		if ( OK != BCM5325EEntryRead(0x0c, data, 4))
			return ERROR;
		portlist = (WORD)(data[0]&0x3f);
		{
			BYTE ti = ((data[0]&0xc0)>>6);
			BYTE pi = (BYTE)(((WORD)(data[1] & 0xf))<< 2);
			ungtaglist = ti+pi;
		}
			
		tempvid	= ((WORD)(data[1]&0xf0));
		tempvid |= ((WORD)(data[2]&0xf))<< 8;		
		tempvid |=  vid;
		valid = (data[2] & 0x10)>>4;
	
		if (vid != tempvid)
			return ERROR;
		if (VLAN_VALID != valid)
			return ERROR;
		

		vty_out( vty,"  %2d  ",vid);
		for(j=0,count=0;count<6;count++)
		{	
			if (portlist & (1 << count))
			{
				CliBcm5325ePortConvertSlot( count, &ulSlot );

				(j==0)?vty_out( vty,"%3d(%d)",ulSlot,count):vty_out( vty,"      %3d(%d)",ulSlot,count);
				j++;

				(ungtaglist & (1 << count))?vty_out( vty,"           U"):vty_out( vty,"           T");
				if(count != 5)
				{
					(valid == 1)?vty_out( vty,"    valid\r\n"):vty_out( vty,"   invalid\r\n");
				}
				else
				{
					(valid == 1)?vty_out( vty,"    valid"):vty_out( vty,"    invalid");	
				}
			}
		}
		vty_out( vty,"\r\n");
	}
	vty_out( vty,"\r\n");
	return OK;

	/*return (vlan8021QShow());*/
}


/****************************************
* BrdChannelBcm5325eInit
* description: 初始化bcm5325e vlan功能,设置所有pon卡
*	mac地址,设置所有槽位对应的bcm5325e端口隔离
*
*
*
******************************************/
extern int InitProductTypeVar();
extern int GetOltType();
int BrdChannelBcm5325eInit(void)
{
	int Product_type = GetOltType();
	ULONG ulSlot = 0;
    
	/*printf("product type = %d\r\n",	Product_type);*/
    
	/* Initialize the switch mii interface */
	if((Product_type == V2R1_OLT_GFA6700)||(Product_type == V2R1_OLT_GFA6100))
	{
		miiPortInit();
	}
	
	if((Product_type == V2R1_OLT_GFA6700)||(Product_type == V2R1_OLT_GFA6100))
	{
		/*设置bcm5325e的vlan功能(enable)于模式(svl)*/
		if ( OK != vlan8021QVlanInit( ))
			return ERROR;

		/*设置板间通道的端口隔离vlan*/
		if ( OK != bcm5325eVlanConfig(Product_type ))
				return ERROR;
	}

	ulSlot = SYS_LOCAL_MODULE_SLOTNO;

	if(Product_type == V2R1_OLT_GFA6700)
	{
		if ( BCM_BRD_MASTER_SLOT == ulSlot )
		{
			/*设置本版主控卡的静态mac地址*/
			if ( OK != BrdChannelMasterMacSet( BCM_BRD_MASTER_SLOT, master_3Slot_mac))
				return ERROR;
			/*设置对方主控卡的静态mac地址*/
			if ( OK != BrdChannelMasterBackMacSet( BCM_BRD_MASTER_BACK_SLOT, master_4Slot_mac))
			return ERROR;
		}
		else if ( BCM_BRD_MASTER_BACK_SLOT == ulSlot )
		{
			/*设置本版主控卡的静态mac地址*/
			if ( OK != BrdChannelMasterMacSet( BCM_BRD_MASTER_BACK_SLOT, master_4Slot_mac))
				return ERROR;
			/*设置对方主控卡的静态mac地址*/
			if ( OK != BrdChannelMasterBackMacSet( BCM_BRD_MASTER_SLOT, master_3Slot_mac))
				return ERROR;
		}
	}
	else if(Product_type == V2R1_OLT_GFA6100)
	{
		cliBcm5325eStaticMacAdd(BCM_MASTER_3SLOT_VLAN,master_2Slot_mac,BCM_MII_PORT);
	}

	/*设置所有pon板的静态mac地址*/
	if ( OK != Bcm5325eV2r1StaticMacSet())
			return ERROR;
	/*设置所有pon板的静态mac地址*/
	/*if ( OK != Bcm5325eV2r1StaticMacSet())
			return ERROR;*/
	
	return 0;
}

/******************************************
*BrdChannelMasterMacSet
* description: 该函数用于在板间通信的bcm5325e中增加
*	主控卡本版的mac地址,对应bcm5325e的端口为bcm port=5
*	1)当slot 4主控板时,输入本版主控板的参数为:ulSlot = 4,mac为本版
*	主控板的mac地址;
*	2)当slot 3为主控板时,输入的本版主控板的参数为:ulSlot = 3,mac为
*	本版主控板的mac地址;
*
* input :
*	ulSlot - #define BCM_BRD_MASTER_SLOT		3
*			#define BCM_BRD_MASTER_BACK_SLOT	4
*	mac	- mac地址
********************************************/
int BrdChannelMasterMacSet(ULONG ulSlot, mac_address_t mac)
{
	unsigned char defVid = BCM_MASTER_3SLOT_VLAN;
	if( (ulSlot != BCM_BRD_MASTER_SLOT) && (ulSlot != BCM_BRD_MASTER_BACK_SLOT))
		return ERROR;
	return (cliBcm5325eStaticMacAdd( defVid/*vid*/, mac, BCM_MII_PORT/*BCM_MASTER_SELF_BROAD_PORT,mii,port = 5*/));
}

/******************************************
*BrdChannelMasterBackMacSet
* description: 该函数用于在板间通信的bcm5325e中增加
*	对方主控卡本版的mac地址,对应bcm5325e的端口为bcm port=BCM_MASTER_BACK_BROAD_PORT
*	1)当slot 4主控板时,输入对方主控板的参数为:ulSlot = 3,mac为对方
*	主控板的mac地址;
*	2)当slot 3为主控板时,输入的对方主控板的参数为:ulSlot = 4,mac为
*	对方主控板的mac地址;
*
* input :
*	ulSlot - #define BCM_BRD_MASTER_SLOT		3
*			#define BCM_BRD_MASTER_BACK_SLOT	4
*	mac	- mac地址
*
********************************************/
int BrdChannelMasterBackMacSet(ULONG ulSlot, mac_address_t mac)
{
	unsigned char defVid = 8;/* BCM_MASTER_4SLOT_VLAN; I think the vlan should be 8  tianzhy  */
	if( (ulSlot != BCM_BRD_MASTER_SLOT) && (ulSlot != BCM_BRD_MASTER_BACK_SLOT))
		return ERROR;
	#if 0 /* tianzhy */
	return (cliBcm5325eStaticMacAdd( defVid/*vid*/, mac, BCM_MASTER_BACK_BROAD_PORT/*port = 4*/));
	#else
	return (cliBcm5325eStaticMacAdd( defVid/*vid*/, mac, 0));
       #endif
}

#if 0	/* removed by xieshl 20100201 */
/*for test.使用前需调用BrdChannelBcm5325eInit()函数*/
int testBcm5325eMacSet(void)
{
	mac_address_t master_mac = {0x00,0x00,0x02,0x03,0x04,0x05};
	mac_address_t back_mac = {0x00,0x00,0x09,0x09,0x09,0x09};
	BrdChannelMasterMacSet( BCM_BRD_MASTER_SLOT, master_mac );
	BrdChannelMasterBackMacSet( BCM_BRD_MASTER_BACK_SLOT, back_mac );
	return 0;
}

/*查看mac地址*/
int testCliBcm5325eArpShow(void)
{
	int result = OK;
	BYTE addr = 0;
	BYTE datap = 0;
	ArlEntry_t entry;
	BYTE i = 0;
	int count = 0;
	BYTE bcm_port = 0;
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
				VOS_TaskDelay(10);
		}
	printf("\r\n");
	return result;
	/*return (Bcm5325eArlEntryShow());*/
}
#endif

/*==================command=======================*/



/*added by wutw 2006/12/12*/
#if 0	/* removed by xieshl 20120406, 这个命令会挂起命令行，先去掉 */
DEFUN( bcm5325_arl_show,
           bcm5325_arl_show_cmd,
           "show bcm5325e mac",
           DescStringCommonShow
           "Show master broad 5325e chip information\n"
           "Show master broad 5325e chip mac\n" 
           )
{
      if ( VOS_OK != CliBcm5325eArpVtyShow( vty ))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}
#endif

DEFUN( bcm5325_vlan_show,
           bcm5325_vlan_show_cmd,
           "show bcm5325e vlan",
           DescStringCommonShow
           "Show master broad 5325e chip information\n"
           "Show master broad 5325e chip vlan\n" 
           )
{
      if ( VOS_OK != Clibcm5325eVlanVtyShow( vty ))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


DEFUN( bcm5325_pvid_show,
           bcm5325_pvid_show_cmd,
           "show bcm5325e pvid",
           DescStringCommonShow
           "Show master broad 5325e chip information\n"
           "Show master broad 5325e chip pvid\n" 
           )
{
      if ( VOS_OK != cliBcm5325ePvidVtyShow( vty ))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}

DEFUN( bcm5325_port_mib_show,
           bcm5325_port_mib_show_cmd,
           "show bcm5325e mib",
           DescStringCommonShow
           "Show master broad 5325e chip information\n"
           "Show master broad 5325e chip mib\n" 
           )
{
      if ( VOS_OK != cliBcm5325ePortMibShow( vty ))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}



DEFUN( bcm5325_vlan_create,
           bcm5325_vlan_create_cmd,
           "bcm5325e vlan <1-15>",
           "Config bcm5325e chip\n"
           "Config vlan\n"
           "Please input vid\n"
           )
{
	unsigned short vid;
	int iRes = (-1);
	vid = ( unsigned short)VOS_AtoL( argv[0] );
	iRes = cliBcm5325eVlanCreate( vid );
	if (iRes == (-4))
	{
		vty_out( vty, "  %% Vlan exist!\r\n");
		return VOS_OK;
	}
    if ( VOS_OK != iRes )
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


DEFUN( bcm5325_vlan_remove,
           bcm5325_vlan_remove_cmd,
           "undo bcm5325e vlan <1-15>",
           NO_STR
           "Config bcm5325e chip\n"
           "Config vlan\n"
           "Please input vid\n"
           )
{
	unsigned short vid;
	int iRes = (-1);
	vid = ( unsigned short ) VOS_AtoL( argv[ 0 ] );
	iRes = cliBcm5325eVlanRemove( vid );
	if ( (-5) == iRes)
	{
		vty_out( vty, "  %% Vlan no exist!\r\n");
		return VOS_OK;
	}
	if ( VOS_OK != iRes)
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


DEFUN( bcm5325_vlan_mem_add,
           bcm5325_vlan_mem_add_cmd,
           "bcm5325e vlan <1-15> mem-add <3-8> [tag|untag]",
           "Config bcm5325e chip\n"
           "Config vlan\n"
           "Please input vid\n"
           "Please input slotId\n"
           )
{
	unsigned short vid;
	unsigned char mem = 0;
	unsigned char untagFlag = 0;
	unsigned char bcm_port = 0;
	int iRes = (-1);
	vid = ( unsigned short ) VOS_AtoL( argv[ 0 ] );
	mem = ( unsigned short ) VOS_AtoL( argv[ 1 ] );
	if (!VOS_StrCmp((CHAR *)argv[2], "tag"))
		untagFlag = 0;
	else if (!VOS_StrCmp((CHAR *)argv[2], "untag"))
		untagFlag = 1;
	else
	{
		vty_out( vty, "  %% Parameter error!\r\n");
		return VOS_OK;
	}
	
	if (VOS_OK != CliBcm5325eSlotConvertPort( (ULONG)mem, &bcm_port ))
		return VOS_ERROR;
	
	iRes = cliBcm5325eVlanPortAdd( vid, bcm_port,untagFlag);
	if ( (-5) == iRes)
	{
		vty_out(vty, "  %% Vlan no exist!\r\n");
		return VOS_OK;
	}
	if ( VOS_OK != iRes)
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


DEFUN( bcm5325_vlan_mem_del,
           bcm5325_vlan_mem_del_cmd,
           "undo bcm5325e vlan <1-15> mem <3-8>",
           NO_STR
           "Config bcm5325e chip\n"
           "Config vlan\n"
           "Please input vid\n"
           "Delete vlan menber\n"
           "Please input slotId\n"
           )
{
	unsigned short vid = 0;
	unsigned char mem = 0;
	unsigned char bcm_port = 0;
	int iRes = (-1);
	vid = ( unsigned short ) VOS_AtoL( argv[ 0 ] );
	mem = ( unsigned short ) VOS_AtoL( argv[ 1 ] );

	if (VOS_OK != CliBcm5325eSlotConvertPort( (ULONG)mem, &bcm_port ))
		return VOS_ERROR;	
	iRes = cliBcm5325eVlanPortDel( vid, bcm_port );
	if ((-5) == iRes)
	{
		vty_out( vty,"  %% Vlan no exist.\r\n");
		return VOS_OK;
	}
      if ( VOS_OK != cliBcm5325eVlanPortDel( vid, bcm_port ))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


DEFUN( bcm5325_pvid_set,
           bcm5325_pvid_set_cmd,
           "bcm5325e pvid <1-15> mem <3-8>",
           "Config bcm5325e chip\n"
           "Config bcm5325e port pvid\n"
           "Please input pvid\n"
           "Config bcm5325e menber pvid\n"
           "Please input slotId\n"
           )
{
	unsigned short pvid = 0;
	unsigned char mem = 0;
	unsigned char bcm_port = 0;
	int iRes = 0;
	pvid = ( unsigned short ) VOS_AtoL( argv[ 0 ] );
	mem = ( unsigned char ) VOS_AtoL( argv[ 1 ] );
	if (VOS_OK != CliBcm5325eSlotConvertPort( (ULONG)mem, &bcm_port ))
		return VOS_ERROR;		
	iRes = cliBcm5325eVlanPvidSet( pvid, bcm_port );
	if ((-5) == iRes)
	{
		vty_out( vty, "  %% Vlan no exist!\r\n");
		return VOS_OK;
	}
    if ( VOS_OK != iRes )
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


DEFUN( bcm5325_mac_add,
           bcm5325_mac_add_cmd,
           "bcm5325e mac <H.H.H> vlan <1-15> mem <3-8>",
          /* "bcm5325e mac <H.H.H> mem <3-8>",*/
           "Config bcm5325e chip\n"
           "Add mac\n"
           "Please input mac\n"
           "VLAN associated with MAC address\n"
           "Please input vid\n"
           "MEM associated with MAC address\n"
           "Please input slotId\n"
           )
{
	unsigned short vid = 0;
	unsigned char mem = 0;
	CHAR MacAddr[6] = {0,0,0,0,0,0};
	unsigned char bcm_port = 0;
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}	
	vid = ( unsigned short ) VOS_AtoL( argv[ 1 ] );
	mem = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	/*vid = ( unsigned short ) 1;
	mem = ( unsigned char ) VOS_AtoL( argv[ 1 ] );	*/
	
	if (VOS_OK != CliBcm5325eSlotConvertPort( (ULONG)mem, &bcm_port ))
		return VOS_ERROR;	
    if ( VOS_OK != cliBcm5325eStaticMacAdd( vid, MacAddr, bcm_port))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


DEFUN( bcm5325_mac_del,
           bcm5325_mac_del_cmd,
           "undo bcm5325e mac <H.H.H> vlan <1-15>",
           NO_STR
           "Config bcm5325e chip\n"
           "Add mac\n"
           "Please input mac\n"
           "VLAN associated with MAC address\n"
           "Please input vid\n"
           )
{
	unsigned short vid = 0;
	CHAR MacAddr[6] = {0,0,0,0,0,0};
	
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}	
	vid = ( unsigned short ) VOS_AtoL( argv[ 1 ] );
    if ( VOS_OK != cliBcm5325eStaticMacDel( vid, MacAddr))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


#if CLI_BCM5325E_VLANMODE_IVL


DEFUN( bcm5325_mac_add,
           bcm5325_mac_add_cmd,
           "bcm5325e mac <H.H.H> vlan <1-15> mem <3-8>",
           "Config bcm5325e chip\n"
           "Add mac\n"
           "Please input mac\n"
           "VLAN associated with MAC address\n"
           "Please input vid\n"
           "MEM associated with MAC address\n"
           "Please input slotId\n"
           )
{
	unsigned short vid = 0;
	unsigned char mem = 0;
	CHAR MacAddr[6] = {0,0,0,0,0,0};
	unsigned char bcm_port = 0;
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}	
	vid = ( unsigned short ) VOS_AtoL( argv[ 1 ] );
	mem = ( unsigned char ) VOS_AtoL( argv[ 2 ] );
	
	if (VOS_OK != CliBcm5325eSlotConvertPort( (ULONG)mem, &bcm_port ))
		return VOS_ERROR;	
    if ( VOS_OK != cliBcm5325eStaticMacAdd( vid, MacAddr, bcm_port))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


DEFUN( bcm5325_mac_del,
           bcm5325_mac_del_cmd,
           "undo bcm5325e mac <H.H.H> vlan <1-15>",
           NO_STR
           "Config bcm5325e chip\n"
           "Add mac\n"
           "Please input mac\n"
           "VLAN associated with MAC address\n"
           "Please input vid\n"
           )
{
	unsigned short vid = 0;
	CHAR MacAddr[6] = {0,0,0,0,0,0};
	
	if ( GetMacAddr( ( CHAR* ) argv[ 0 ], MacAddr ) != VOS_OK )
	{
        vty_out( vty, "  %% Invalid MAC address.\r\n" );
        return CMD_WARNING;
	}	
	vid = ( unsigned short ) VOS_AtoL( argv[ 1 ] );
    if ( VOS_OK != cliBcm5325eStaticMacDel( vid, MacAddr))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}

#endif
/* added by xieshl 20071119 mdc/mdio调试命令 */
#if ( EPON_MODULE_EPON_5325E_MDIO == EPON_MODULE_YES )
extern int miim_cmdVlanShow( struct vty *vty );
extern int miim_cmdPvidShow( struct vty *vty );
DEFUN( miim_bcm5325_vlan_show,
           miim_bcm5325_vlan_show_cmd,
           "show miim vlan",
           DescStringCommonShow
           "Show pon broad 5325e chip info.\n"
           "Show pon broad 5325e chip vlan\n" 
           )
{
      if ( VOS_OK != miim_cmdVlanShow( vty ))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}


DEFUN( miim_bcm5325_pvid_show,
           miim_bcm5325_pvid_show_cmd,
           "show miim port",
           DescStringCommonShow
           "Show pon broad 5325e chip info.\n"
           "Show pon broad 5325e chip port(enable and vid)\n" 
           )
{
      if ( VOS_OK != miim_cmdPvidShow( vty ))
	  	vty_out( vty, "  %% Executing error.\r\n");
	  
      return VOS_OK;
}
#endif

LONG BCM5325E_DEBUG_CommandInstall(void)
{

	/*at by wutw at 2006/12/12*/
    /*install_element ( DEBUG_HIDDEN_NODE, &bcm5325_arl_show_cmd );*/
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_vlan_show_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_pvid_show_cmd ); 
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_port_mib_show_cmd );	

#if ( EPON_MODULE_EPON_5325E_MDIO == EPON_MODULE_YES )
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_vlan_create_cmd );	
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_vlan_remove_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_vlan_mem_add_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_vlan_mem_del_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_pvid_set_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_mac_add_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &bcm5325_mac_del_cmd );
   

    install_element ( DEBUG_HIDDEN_NODE, &miim_bcm5325_vlan_show_cmd );
    install_element ( DEBUG_HIDDEN_NODE, &miim_bcm5325_pvid_show_cmd ); 
#endif

	return OK;
	
}
#ifdef	__cplusplus
}
#endif/* __cplusplus */

