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
#include "stdio.h"
#include "string.h"
#include "bcm5325eHeader.h"
#include "V2R1General.h"

#define VLAN_RANGE_OUT_ERR		(-2)
#define PORTLIST_MIMATCH_ERR	(-3)
#define VLAN_EXIST_ERR			(-4)
#define VLAN_NOT_EXIST_ERR		(-5)


#define VLAN_VALID	1
#define VLAN_INVALID	0
#define VLAN_MAX	16

/*802.1q vlan read register */
#define	VLANREADREG	0x0C
/*802.1q vlan write register */
#define	VLANWRITEREG	0x08
WORD vlanInfo[VLAN_MAX] = {0};


/**************************************************************************************/
/**************************PORT-BASED VLAN*********************************************/
/*******************************************************************************
**                                                                              
** Function Name: setPortBaseVlan
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    set port-based vlan function 
**  INPUTS:         BYTE portAddr : the port of register address
**		    WORD vlanMember : vlan member
**  OUTPUTS:        	No                                                          
**  RETURN :   	    	success:  OK  ; fail : ERROR                                                          
*******************************************************************************/  
int setPortBaseVlan(BYTE portAddr , WORD vlanMember)
{
	BYTE addr ;
	if(portAddr != MII_PORTID)
		addr = PORTBASEVLAN_BASE + portAddr * 2 ;
	else
		addr = VLANCTRLREG_PORT_MII ;	
	if(BCM5325EWordRegWrite(addr , vlanMember) == ERROR)
		return ERROR ; 
	return OK ; 
}
/*******************************************************************************
**                                                                              
** Function Name: readPortBaseVlan
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    read port-based vlan function 
**  INPUTS:         BYTE portAddr : the port of register address
**  OUTPUTS:        WORD *vlanMemberP : pointer to vlan member	                                                        
**  RETURN :   	    	success:  OK  ; fail : ERROR                                                          
*******************************************************************************/  
int readPortBaseVlan(BYTE portAddr , WORD *vlanMemberP)
{
	BYTE addr ;
	WORD setData = 0 ;
	if(portAddr != MII_PORTID)
		addr = PORTBASEVLAN_BASE + portAddr * 2 ;
	else
		addr = VLANCTRLREG_PORT_MII ;	
	if(BCM5325EWordRegRead(addr , &setData) == ERROR)
		return ERROR ; 
	*vlanMemberP = setData ;
	return OK ; 
}
/*******************************************************************************
**                                                                              
** Function Name: initPortVlan
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    port-based initial function 
**  INPUTS:         No
**  OUTPUTS:        No	                                                        
**  RETURN :   	    	success:  OK  ; fail : ERROR                                                          
*******************************************************************************/ 
#if 0 /* removed by xieshl 20100201 */
int initPortVlan(void)
{
	BYTE portId ;
	WORD vlanNum ;
	WORD readNum = 0 ;


	/*(1) write page cs : 0x31*/
	regPageCs(PAGE_PORTBASEVLAN) ;
	/*(2) set port vlan*/
	for(portId = 0 ; portId < MAXPORTNUMBER ; portId++)
	{
		vlanNum = 0x0100 ;
		if(portId < (MAXPORTNUMBER - 1))	/*not MII port*/
		{
			vlanNum |= (0x01 << portId) ;	/* VLAN number:MII port & portn*/
			if(setPortBaseVlan(portId , vlanNum) == ERROR)
				return ERROR ;
			if (OK != readPortBaseVlan(portId , &readNum) )
				{
			printf(" Rd port %d Err!\r\n",portId);
				return (-1);
				}
			printf(" port %d vlan : 0x%x\r\n",portId,vlanNum);
		}
		else					/* MII port*/
		{
			
			vlanNum |= 0x1F ;		/*VLAN number: MII port & port0--port4*/
			if(setPortBaseVlan(portId , vlanNum) == ERROR)
				return ERROR ; 
			if (OK != readPortBaseVlan(portId , &readNum) )
				{
				printf(" 2. Rd port %d\r\n Err!\r\n",portId);
				return -1;
				}
			printf(" MII (%d) vlan : 0x%x\r\n",portId,vlanNum);
		}
	}
	return OK ;
}
#endif

/**************************END PORT-BASED VLAN*****************************************/




/*****************************802.1q VLAN**********************************************/
int enable8021qVlan(BYTE portAddr , BYTE vlanMode , BYTE frameCtrl)
{
		return 0;
}
/*****************************End 802.1q VLAN******************************************/



/*802.1q default port tag register */
#define DEFAULTPORTTAGREG_BASE	0x10
int  setPortPvid(BYTE portId , WORD pvid)
{
	BYTE addr ;
	BYTE i = 0;
	WORD vid = pvid;
	BYTE flag = 0;

	for(i = 0; i<VLAN_MAX; i++)
	{
		if(vid == vlanInfo[i])
		{
			flag = 1;
			break;	
		}
	}
	if ( 1 != flag )
		return VLAN_EXIST_ERR;
	
	regPageCs(0x34);
	addr = DEFAULTPORTTAGREG_BASE + (portId * 2) ;
	if (OK != BCM5325EWordRegWrite(addr , pvid))
		return ERROR;
	return OK;
}

int readPortPvid(BYTE portId , WORD *pvidP)
{
	BYTE addr ;
	regPageCs(0x34);
	addr = DEFAULTPORTTAGREG_BASE + (portId * 2) ;
	if (OK != BCM5325EWordRegRead(addr , pvidP) )
		return ERROR;
	return OK;
}

int miiPortInit(void)
{
	BYTE addr ;
	BYTE wMiiConfig = 0;
	
	regPageCs(0x00);
	addr = 0x0e;

	wMiiConfig = 0x97;
	if (OK != BCM5325EByteRegWrite(addr , wMiiConfig) )
		return ERROR;
	return OK;
}

int vlanEntryWrt( VLAN8021QENTRY *entry)
{

	int i = 0;
	WORD rd_value = 0;
	WORD tempVid = 0;
	BYTE data[4] = {0};

	data[0] = (entry->portlist & 0x3f);
	data[0] |= (entry->untagMap & 0x3)<<6;
	data[1] = (entry->untagMap & 0x3c)>>2;
	tempVid = (entry->vid & 0xff0)>> 4;
	data[1] |= (tempVid & 0xf)<< 4;
	data[2] = (tempVid & 0xf0)>> 4;
	data[2] |= ( entry->valid << 4);
	data[3] = 0;
	
	regPageCs(0x34) ;
	BCM5325EEntryWrt(0x08 , data, 4);
	/*ctrl_value |= (entry->vid && 0xfff);
	ctrl_value |= 0x1000; *write*
	ctrl_value |= 0x2000; *done*/
	data[0] = (entry->vid)&0xff;
	data[1] = (entry->vid & 0xf00)>>8;
	data[1] |= 0x10|0x20;

	if (OK != BCM5325EEntryWrt(0x06, data, 2))
	/*if (OK != BCM5325EWordRegWrite(0x06, ctrl_value))*/
	{
		/*printf(" 1. Wt 0x%x Err!\r\n",0x06);*/
		return ERROR;
	}		
	do
	{
		int j = 0;
		BCM5325EWordRegRead(0x06, &rd_value);
		if ( (rd_value & 0x2000 )== 0 )
			break;
		if (i > 10)
			return ERROR;
		else 
			for (j = 0; j<10;j++)
				rd_value &= 0;
			i++;
		
	}while(1);
	return OK;	
}

int vlan8021QVlanInit(void)
{
	/*SVL mode*/
	bcm5635eRegWrt( 0x34,0x0, 0x80);
	
	/*enable 8021Q*/
	bcm5635eRegWrt( 0x34,0x5, 0x20);
	vlanDefaultInit( );
	return OK;
}
/*added by wutongwu at august twenty four*/




int vlanDefaultInit(void)
{
	VLAN8021QENTRY entry;
	WORD defvid = 1;
	/*BYTE data[4] = {0};*/
	/*memset(&entry, 0, sizeof(VLAN8021QENTRY));
	entry.portlist = 0x3f;
	entry.untagMap = 0x3f;
	entry.valid = VLAN_VALID;
	entry.vid = defvid;
	vlanInfo[0] = defvid;
	if (OK != vlanEntryWrt( &entry ))
		return ERROR;*/
	memset(&entry, 0, sizeof(VLAN8021QENTRY));
	entry.portlist = 0x3f;
	entry.untagMap = 0x3f;
	entry.valid = VLAN_VALID;
	entry.vid = defvid;
	vlanInfo[0] = defvid;
	
	if (OK != vlanEntryWrt( &entry ))
		return ERROR;	


	return OK;	
}

/*for untagMap:
  bit0 - port 0 , setting 1 is value 
  bit1 - port 1 , setting 1 is value 
  bit2 - port 2 , setting 1 is value 
  bit3 - port 3 , setting 1 is value 
  bit4 - port 4 , setting 1 is value 
  bit5 - mii port
  for tagMap: 
  bit0 - port 0 , setting 0 is value 
  bit1 - port 1 , setting 0 is value 
  bit2 - port 2 , setting 0 is value 
  bit3 - port 3 , setting 0 is value 
  bit4 - port 4 , setting 0 is value 
  */


int vlan8021QCreate( WORD vid )
{

	BYTE i = 0;
	VLAN8021QENTRY entry;
	if (vid > 4095)
		return VLAN_RANGE_OUT_ERR;
	for(i = 0; i<VLAN_MAX; i++)
	{
		if(vid == vlanInfo[i])
			return VLAN_EXIST_ERR;
	}
	for(i = 0; i<VLAN_MAX; i++)
	{
		if(0 == vlanInfo[i])
		{
			vlanInfo[i] = vid;
			break;
		}
	}	

	memset(&entry, 0, sizeof(VLAN8021QENTRY));
	entry.portlist = 0;
	entry.untagMap = 0x3f;
	entry.valid = VLAN_VALID;
	entry.vid = vid;

	if (OK != vlanEntryWrt(&entry ))
		return ERROR;
	
	return OK;

}


int vlan8021QRemove( WORD vid )
{

	VLAN8021QENTRY entry;
	BYTE i = 0;
	BYTE vFlag = 0;
	if (vid > 4095)
		return VLAN_RANGE_OUT_ERR;
	for(i = 0; i<VLAN_MAX; i++)
	{
		if(vid == vlanInfo[i])
		{
			vlanInfo[i] = 0;
			vFlag = 1;
			break;
		}
	}
	if (vFlag == 0)
		return VLAN_NOT_EXIST_ERR;

	memset(&entry, 0, sizeof(VLAN8021QENTRY));
	entry.portlist = 0;
	entry.untagMap = 0x3f;
	entry.valid = VLAN_INVALID;
	entry.vid = vid;

	if (OK != vlanEntryWrt(&entry ))
		return ERROR;

	return OK;

}


int vlan801QEntryGet(WORD vid, VLAN8021QENTRY *entry)
{

	WORD rd_value = 0;
	/*WORD vid = 0;*/

	BYTE data[4] = {0};



	BYTE i = 0;
	BYTE j = 0;
	if (NULL == entry)
		return ERROR;

	regPageCs(0x34) ;

		data[0] = (BYTE) vid;
    	data[1] = (BYTE) (vid >> 8);
    /* RW_CTRL_ADDR_BIT: Bit[12], Read/Write Control, Read(0)/Write(1) */
    /* REQUEST_ADDR_BIT: Bit[13], Access Request, request(1) */
    	data[1] &= (~(BYTE)0x10);
    	data[1] |= (BYTE)0x20;	
		if (OK != BCM5325EEntryWrt(0x06, data, 2))
		{
			/*printf(" 1. Wt 0x%x Err!\r\n",ctrl_value);*/
			return ERROR;
		}		
		do
		{
			if (i>10)
				return ERROR;
			if( OK != BCM5325EWordRegRead(0x06, &rd_value))
				return ERROR;
			if ( (rd_value & 0x2000 )== 0 )
				break;
			for(j = 0;j<1000;j++);
			
			i++;
			
		}while(1);	

		if ( OK != BCM5325EEntryRead(0x0c, data, 4))
			return ERROR;

	entry->portlist = (WORD)(data[0]&0x3f);
	{
		BYTE ti = ((data[0]&0xc0)>>6);
		BYTE pi = (BYTE)(((WORD)(data[1] & 0xf))<< 2);
		entry->untagMap = ti+pi;
	}

	entry->vid = (((WORD)(data[1]&0xf0)) + ((WORD)(data[2]&0xf))<< 8) + vid;
	entry->valid = 		(data[2] & 0x10)>>4;

	return OK;
}



int vlan8021QPortAdd( WORD vid, BYTE port, BYTE untagMap)
{
	VLAN8021QENTRY entry;
	int iRes = 0;
	int i = 0;
	BYTE vFlag = 0;
	if (vid > 4095)
		return VLAN_RANGE_OUT_ERR;

	for(i = 0; i<VLAN_MAX; i++)
	{
		if (vid == vlanInfo[i])
		{
			vFlag = 1;
		}
	}
	if ( 0 == vFlag )
		return VLAN_NOT_EXIST_ERR;	
		memset(&entry, 0, sizeof(VLAN8021QENTRY));
	iRes = vlan801QEntryGet( vid, &entry);
	if ( OK != iRes )
		return iRes ;
	if (vid != entry.vid)
		return ERROR;
	if (VLAN_VALID != entry.valid)
		return ERROR;	
	entry.portlist |= (1<< port);
	if (untagMap == 1)
		entry.untagMap |= ( 1<<port);
	else 
		entry.untagMap &= ~( 1<<port);
	/*printf("entry.valid %D\r\n",entry.valid);
	printf("entry.vid %D\r\n",entry.vid);
	printf("entry.portlist 0x%x\r\n",entry.portlist);
	printf("entry.untagMap 0x%x\r\n",entry.untagMap);*/
	entry.valid = VLAN_VALID;
	entry.vid = vid;
	/*printf("entry.portlist 0x%x\r\n",entry.portlist);
	printf("entry.untagMap 0x%x\r\n",entry.untagMap);*/
	iRes = vlanEntryWrt( &entry );
	return iRes;
}



int vlan8021QPortDel( WORD vid, BYTE port)
{
	VLAN8021QENTRY entry;
	int iRes = 0;
	int i = 0;
	BYTE vFlag = 0;	
	if (vid > 4095)
		return VLAN_RANGE_OUT_ERR;
	for(i = 0; i<VLAN_MAX; i++)
	{
		if (vid == vlanInfo[i])
		{
			vFlag = 1;
		}
	}
	if ( 0 == vFlag )
		return VLAN_NOT_EXIST_ERR;	
	
	iRes = vlan801QEntryGet( vid, &entry);
	if ( OK != iRes )
		return iRes ;
	
	/*printf("entry.valid %D\r\n",entry.valid);
	printf("entry.vid %D\r\n",entry.vid);
	printf("entry.portlist = 0x%x\r\n",entry.portlist);
	printf("entry.untagMap = 0x%x\r\n",entry.untagMap);	*/
	if (vid != entry.vid)
		return ERROR;
	if (VLAN_VALID != entry.valid)
		return ERROR;
	entry.portlist &= ~(1<< port);
	entry.untagMap &= ~(1<< port);
	/*printf("entry.portlist = 0x%x\r\n",entry.portlist);
	printf("entry.untagMap = 0x%x\r\n",entry.untagMap);*/
	iRes = vlanEntryWrt( &entry );
	return iRes;
}

int bcm5325eVlanConfig(int Product_type)
{

	/*slot 3*/
	 vlan8021QCreate(5);
	 setPortPvid( 5 , 5 );
	 vlan8021QPortAdd(5,5,1);
	 if(Product_type == V2R1_OLT_GFA6700)
	 	vlan8021QPortAdd(5,4,1);
	vlan8021QPortAdd(5,3,1);
	vlan8021QPortAdd(5,2,1);
	vlan8021QPortAdd(5,1,1);
	vlan8021QPortAdd(5,0,1);
	
	 /*slot 8*/
	  if(Product_type == V2R1_OLT_GFA6700)
	  	{
		 vlan8021QCreate(4);
		 setPortPvid(4 , 4);
		 vlan8021QPortAdd(4,5,1);
		 vlan8021QPortAdd(4,4,1);
	  	}
	  
	 /*slot 7*/ /* pon 3/2 */
	 vlan8021QCreate(3);
	 setPortPvid( 3 , 3 );
	 vlan8021QPortAdd(3,3,1);
	 vlan8021QPortAdd(3,5,1);

	 /*slot 6*/ /* pon 3/1 */
	 vlan8021QCreate(2);
	 setPortPvid( 2 , 2 );
	 vlan8021QPortAdd(2,2,1);
	 vlan8021QPortAdd(2,5,1);

	 /*slot 5*/ /* pon 2/2 */
	 vlan8021QCreate(7);
	 setPortPvid( 1 , 7 );
	 vlan8021QPortAdd(7,1,1);
	 vlan8021QPortAdd(7,5,1);
	
	 /*slot 4*/ /* pon 2/1 */
	 vlan8021QCreate(8);
	 setPortPvid(0 , 8 );
	 vlan8021QPortAdd(8,0,1);
	 vlan8021QPortAdd(8,5,1);
	 
	 return 0;
}

#if 0	/* removed by xieshl 20071119 */

int vlan8021QShow(void)
{

	WORD rd_value = 0;
	WORD vid = 0;
	WORD tempvid = 0;
	WORD count = 0;
	BYTE data[4] = {0};

	BYTE portlist = 0;
	BYTE ungtaglist = 0;
	BYTE valid = 0;
	BYTE i = 0;
	BYTE j = 0;
	

	printf("  vid  port  tag    valid\r\n");
	printf("  ----------------------------------\r\n");	
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
			/*printf(" 1. Wt 0x%x Err!\r\n",ctrl_value);*/
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
			
			
		tempvid = (((WORD)(data[1]&0xf0)) + ((WORD)(data[2]&0xf))<< 8) + vid;
		valid = (data[2] & 0x10)>>4;
	
		if (vid != tempvid)
			return ERROR;
		if (VLAN_VALID != valid)
			return ERROR;
		

		/*printf(" vid %d ",vid);
		printf(" portlist 0x%x   ",portlist);
		printf("untag-list 0x%x   ",ungtaglist);
		printf("valid %d\r\n",valid);*/

		printf("  %2d  ",vid);
		for(j=0,count=0;count<6;count++)
		{	
			if (portlist & (1 << count))
			{
				if(5 != count)
				{
					(j==0)?printf("%3d",count):printf("      %3d",count);
					j++;
				}
				else if (5 == count)
				{
					(j==0)?printf(" mii(%d)",count):printf("    mii(%d)",count);
					j++;
				}
					
				(ungtaglist & (1 << count))?printf("    U"):printf("    T");
				if(count != 5)
				{
					(valid == 1)?printf("    valid\r\n"):printf("    invalid\r\n");
				}
				else
				{
					(valid == 1)?printf("    valid"):printf("    invalid");	
				}
			}
		}
		printf("\r\n");
	}
	printf("\r\n");
	return OK;
}

int vlanshow(WORD vid)
{
	int iRes;
	int j = 0, count = 0;
	WORD tempvid = 0;

	VLAN8021QENTRY entry;
	memset(&entry, 0, sizeof(VLAN8021QENTRY));
	if ((iRes = vlan801QEntryGet( vid, &entry))!= OK)
		return iRes;
	
	if (vid != tempvid)
		return ERROR;
	if (VLAN_VALID != entry.valid)
		return ERROR;	
	printf("  vid  port  tag    valid\r\n");
	printf("  ----------------------------------\r\n");	
	printf("  %2d  ",vid);
		for(j=0,count=0;count<6;count++)
		{	
			if (entry.portlist & (1 << count))
			{
				if(5 != count)
				{
					(j==0)?printf("%3d",count):printf("      %3d",count);
					j++;
				}
				else if (5 == count)
				{
					(j==0)?printf(" mii(%d)",count):printf("    mii(%d)",count);
					j++;
				}
					
				(entry.untagMap & (1 << count))?printf("    U"):printf("    T");
				if(count != 5)
				{
					(entry.valid == 1)?printf("    valid\r\n"):printf("    invalid\r\n");
				}
				else
				{
					(entry.valid == 1)?printf("    valid"):printf("    invalid");	
				}
			}
		}

	return 0;
}

int pvidshow(void)
{
	BYTE port = 0;
	WORD pvid = 0;
	for(port = 0; port<6; port++)
	{
		readPortPvid( port , &pvid );
		printf(" port %d  pvid %d\r\n",port,pvid);
	}
	return OK;
		
}

int vlanInit(void)
{
	 vlan8021QCreate(5);
	 setPortPvid( 5 , 5 );
	 vlan8021QPortAdd(5,5,1);
	 vlan8021QPortAdd(5,4,1);
	vlan8021QPortAdd(5,3,1);
	vlan8021QPortAdd(5,2,1);
	vlan8021QPortAdd(5,1,1);
	vlan8021QPortAdd(5,0,1);

	 /*slot 8*/
	 vlan8021QCreate(4);
	 setPortPvid(4 , 4);
	 vlan8021QPortAdd(4,5,1);
	 vlan8021QPortAdd(4,4,1);

	 /*slot 6*/
	 vlan8021QCreate(3);
	 setPortPvid( 3 , 3 );
	 vlan8021QPortAdd(3,3,1);
	 vlan8021QPortAdd(3,5,1);
	 
	 vlan8021QCreate(2);
	 setPortPvid( 2 , 2 );
	 vlan8021QPortAdd(2,2,1);
	 vlan8021QPortAdd(2,5,1);
	 
	 vlan8021QCreate(7);
	 setPortPvid( 1 , 7 );
	 vlan8021QPortAdd(7,1,1);
	 vlan8021QPortAdd(7,5,1);

	 /*slot 4*/
	 vlan8021QCreate(8);
	 setPortPvid(0 , 8 );
	 vlan8021QPortAdd(8,0,1);
	 vlan8021QPortAdd(8,5,1);	 
	 return 0;
}
#endif

