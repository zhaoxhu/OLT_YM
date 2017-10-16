#ifdef	__cplusplus
extern "C"
{
#endif

#include "syscfg.h"

#include  "OltGeneral.h"
#if ( EPON_MODULE_EPON_5325E_MDIO == EPON_MODULE_YES )

#include "vos/vospubh/vos_base.h"
#include "vos/vospubh/vos_io.h"
#include "cli/cli.h"
#include "cli/cl_cmd.h"
#include "cli/cl_mod.h"
#include "sys/console/sys_console.h"
#include "sys/main/sys_main.h"
#include "Vos_string.h"
#include "Vos_task.h"

#include  "PonGeneral.h"

#ifndef OK
#define OK 	0
#endif
#ifndef ERROR
#define ERROR (-1)
#endif
#ifndef VLAN_MAX
#define VLAN_MAX 	16
#endif
#ifndef TYPE_UNICAST
#define TYPE_UNICAST		1
#endif

#define PHYADDR 0x1e
#define ADDR_PAGE 16
#define ADDR_REG 17
#define VAL1_REG 24
#define VAL2_REG 25
#define VAL3_REG 26
#define VAL4_REG 27

/*#ifndef  PON_olt_id_t
typedef short int  PON_olt_id_t;
#endif*/

extern long PAS_read_mdio_register 
                                   ( const PON_olt_id_t    olt_id, 
						const short int       phy_address, 
						const short int       reg_address,
						unsigned short int   *value);


extern short int PAS_write_mdio_register 
					( const PON_olt_id_t		  olt_id, 
						const short int			  phy_address, 
						const short int			  reg_address, 
						const unsigned short int   value );


int miim_portEnableSet( short int PonPortIdx, uchar_t port, uchar_t enable );

#define mdio_delay			/*VOS_TaskDelay(1)*/
#define mdio_delay_never	VOS_TaskDelay(1)
#define mdio_wait_times		3
/*Begin: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
int  miim_5325_read_nByte(unsigned int PonPortIdx, unsigned short int addPage, unsigned short int addReg, unsigned char ByteNum, unsigned char *nByte)
{
	unsigned short int  value;
	int i;
	short int Ret;
	int lRet;
	
	if(( PonPortIdx < 0 ) ||( PonPortIdx > 20 )) 
	{
		sys_console_printf(" PONid %d out of range %d\r\n", PonPortIdx );
		return( ERROR );
	}

	/* set page number, bit0=1, bit15:8=addPage */ 
	value = (((addPage <<8 ) | 1) & 0xff01);
	
	Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, ADDR_PAGE,value );
	if( Ret != OK )
	{
		sys_console_printf("\r\n1:read from 5325e by pon%d failed-write page addr\r\n", PonPortIdx );
		return( ERROR );
	}
	mdio_delay;
	
	/*set register address, bit1:0=10b, bit15:8=addReg */
	value = (((addReg << 8) |2) & 0xff02);
	Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR,ADDR_REG,value);
	if( Ret != OK )
	{
		sys_console_printf("\r\n2:read from 5325e by pon%d failed-write addr reg\r\n", PonPortIdx );
		return( ERROR );
	}
	mdio_delay_never;
	
	/*wait for ADD_REG[1:0]=00*/
	for( i=0; i<mdio_wait_times; i++ )
	{
		value = 0;
		lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, ADDR_REG, &value );
		if( lRet == OK )
		{
			value = value & 0x3;
			if(value == 0)
				break;
		}
		mdio_delay_never;
	}
	
	if( i >= mdio_wait_times )
	{	
		sys_console_printf("\r\n3: Read 5325e by pon%d failed, return value=%x\r\n",PonPortIdx, value);
		return(ERROR);
	}

	switch( ByteNum )
	{
		case 1:
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL1_REG,  (ushort_t *)&value );
			*nByte = (uchar_t)(value & 0xff);
			break;
			
		case 2:
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL1_REG,  (ushort_t *)nByte );	
			break;
			
		case 4:
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL1_REG,  (ushort_t *)(nByte+2) );	
			if( lRet != OK )
				break;
			mdio_delay;
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL2_REG,  (ushort_t *)nByte );	
			break;
			
		case 6:
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL3_REG,  (ushort_t *)nByte );	
			if( lRet != OK )
				break;
			mdio_delay;
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL2_REG,  (ushort_t *)(nByte+2) );	
			if( lRet != OK )
				break;
			mdio_delay;
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL1_REG,  (ushort_t *)(nByte+4) );	
			break;

		case 8:
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL4_REG,  (ushort_t *)nByte );	
			if( lRet != OK )
				break;
			mdio_delay;
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL3_REG,  (ushort_t *)(nByte+2) );	
			if( lRet != OK )
				break;
			mdio_delay;
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL2_REG,  (ushort_t *)(nByte+4) );	
			if( lRet != OK )
				break;
			mdio_delay;
			lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, VAL1_REG,  (ushort_t *)(nByte+6) );	
			break;

		default:
			lRet = ERROR;
			break;
	}
	if( lRet != OK )
	{
		printf("\r\n read from 5325e by pon%d failed,return value=%x\r\n",PonPortIdx, lRet );
		return( ERROR );
	}
		
	mdio_delay;

	return(OK);
}

UINT miim_5325_write_nByte(short int PonPortIdx, unsigned short int addPage,unsigned short int addReg, unsigned char ByteNum, unsigned char  *value_reg)
{
	unsigned short int value;
	int i;
	short int Ret;
	int lRet = 0;
	
	if(( PonPortIdx < 0 ) ||( PonPortIdx > 20 )) 
	{
		sys_console_printf(" PONid %d out of range %d\r\n", PonPortIdx );
		return( ERROR );
	}
	
	/*set page number, bit0=1, bit15:8=addPage */ 
	value = (((addPage<<8)|1) & 0xff01);
	Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, ADDR_PAGE,value );
	if( Ret != OK )
	{
		sys_console_printf("\r\n 1:write to 5325e by pon%d failed-write page addr\r\n", PonPortIdx );
		return( ERROR );
	}
	mdio_delay;
	
	/*set value*/
	switch(ByteNum)
	{
		case 1:
			value = value_reg[0];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL1_REG, value );
			break;
		case 2:
			value =value_reg[0];
			value = (value << 8) + value_reg[1];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL1_REG, value );							
			break;
		case 4:
			value =value_reg[2];
			value = (value << 8) + value_reg[3];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL1_REG, value );				
			if( Ret != OK )
				break;
			mdio_delay;
			value =value_reg[0];
			value = (value << 8) + value_reg[1];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL2_REG, value );				
			break;
		case 6:
			value =value_reg[0];
			value =( value << 8 ) + value_reg[1];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL3_REG, value );				
			if( Ret != OK )
				break;
			mdio_delay;
			value =value_reg[2];
			value = (value << 8) + value_reg[3];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL2_REG, value );	
			if( Ret != OK )
				break;
			mdio_delay;
			value =value_reg[4];
			value = (value << 8) + value_reg[5];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL1_REG, value );				
			break;
		case 8:
			value =value_reg[0];
			value = (value << 8)+ value_reg[1];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL4_REG, value );				
			if( Ret != OK )
				break;
			mdio_delay;
			value =value_reg[2];
			value = (value << 8 )+ value_reg[3];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL3_REG, value );	
			if( Ret != OK )
				break;
			mdio_delay;
			value =value_reg[4];
			value = (value << 8) + value_reg[5];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL2_REG, value );	
			if(Ret != OK )
				break;
			mdio_delay;
			value =value_reg[6];
			value = (value << 8) + value_reg[7];
			Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, VAL1_REG, value );
			break;
		default:
			Ret = ERROR;
			break;
	}	

	if( Ret != OK )
	{
		sys_console_printf("\r\n2:write to 5325e by pon%d failed-write reg value\r\n", PonPortIdx );
		return( ERROR );
	}
	mdio_delay;
	
	/*set register address, bit1:0=01b, bit15:8=addReg */
	value = (((unsigned short int )addReg<<8)|1)&0xfffd;
	Ret = OLT_WriteMdioRegister(PonPortIdx, PHYADDR, ADDR_REG, value );
	if( Ret != OK )
	{
		sys_console_printf("\r\n3:write to 5325e by pon%d failed-write reg addr\r\n", PonPortIdx );
		return( ERROR );
	}
	mdio_delay;
	
	/*wait for ADD_REG[1:0]=00*/
	for( i=0; i<mdio_wait_times; i++ )
	{
		value = 0;
		lRet = OLT_ReadMdioRegister( PonPortIdx, PHYADDR, ADDR_REG, &value );
		if( lRet == OK )
		{
			value = value & 0x3;
			if (value == 0)
				break;
		}
		mdio_delay_never;
	}
	
	if( i >= mdio_wait_times )
	{
		sys_console_printf("\r\n4:write to5325e by pon%d failed,return value=%x\r\n",PonPortIdx, lRet );
		return( ERROR );
	}	
	
	return(OK);
}
/*End: for 10G EPON of PMC8411 PAS Adapter Extension by jinhl @2012-11-12*/
#if 0
int test_mdio( uchar_t reg, int val )
{
	short int Ret;
	short int PonPortIdx = 4;
	unsigned short int value;
	int lRet = 0, i;

	value = 0x3401;
	Ret = PAS_write_mdio_register(PonPortIdx, PHYADDR, ADDR_PAGE,value );
	mdio_delay;

	value = val;
	Ret = PAS_write_mdio_register(PonPortIdx, PHYADDR, VAL1_REG, value );				
	mdio_delay;
/*	
	value = 0;
	Ret = PAS_write_mdio_register(PonPortIdx, PHYADDR, VAL2_REG, value );	
	mdio_delay;

	value = 0;
	Ret = PAS_write_mdio_register(PonPortIdx, PHYADDR, VAL3_REG, value );	
	mdio_delay;

	value = 0;
	Ret = PAS_write_mdio_register(PonPortIdx, PHYADDR, VAL4_REG, value );
	mdio_delay;
*/
	value = reg;
	value = (value << 8) + 1;
	Ret = PAS_write_mdio_register(PonPortIdx, PHYADDR, ADDR_REG, value );

	mdio_delay_never;
	
	/*wait for ADD_REG[1:0]=00*/
	for( i=0; i<mdio_wait_times; i++ )
	{
		value = 0;
		lRet = PAS_read_mdio_register( PonPortIdx, PHYADDR, ADDR_REG, &value );
		if( lRet == OK )
		{
			if( (value & 0x3) == 0 )
				break;
		}
		mdio_delay_never;
		sys_console_printf("MDIO: check %d\r\n",i);
	}
	
	if( i >= mdio_wait_times )
	{
		sys_console_printf("\r\n4:write to5325e by pon%d failed,return value=%x\r\n",PonPortIdx, lRet );
		return( VOS_ERROR );
	}
	sys_console_printf("test ok\r\n");
	return VOS_OK;
}
#endif

#define MDIO_VLAN_ENTRY_VALID		1
#define MDIO_VLAN_ENTRY_INVALID	0
#define MDIO_VLAN_DEFAULT_VID		1
typedef struct {
	ulong_t reserved:11;
	ulong_t valid:1;
	ulong_t h8_vid:8;
	ulong_t untaglist:6;
	ulong_t portlist :6;
}__attribute__((packed)) mdioVlanEntry_t;		/* page=0x34, reg=0x08~0x0b */

#define MDIO_VLAN_ACCESS_START	1
#define MDIO_VLAN_ACCESS_DOWN	0
#define MDIO_VLAN_ACCESS_WRITE	1
#define MDIO_VLAN_ACCESS_READ		0
typedef struct {
	ushort_t reserved:2;
	ushort_t start_done:1;
	ushort_t read_write:1;
	ushort_t h8_vid:8;
	ushort_t l4_vid:4;
}__attribute__((packed)) mdioVlanAccess_t;		/* page=0x34, reg=0x06~0x07 */

ushort_t vlanEntry[VLAN_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#define BCM5325E_PORT1_BIT		0x01
#define BCM5325E_PORT2_BIT		0x02
#define BCM5325E_PORT3_BIT		0x04
#define BCM5325E_PORT4_BIT		0x08
#define BCM5325E_PORT5_BIT		0x10
#define BCM5325E_PORT_MII_BIT	0x20

extern short int GetPonPortIdxBySlot( short int slot, short  int port );
extern int GetCardIdxByPonChip( short int PonChipIdx );
extern int GetPonPortByPonChip( short int PonChipIdx );
extern short int  GetPonChipTypeByPonPort( short int PonPortIdx );
int miim_vlanCreate( ushort_t PonPortIdx, ushort_t vid, ushort_t portlist );
int miim_defaultVlanCreate( ushort_t PonPortIdx  );
int miim_vlanEntryWrt( ushort_t PonPortIdx, ushort_t vid, mdioVlanEntry_t *entry);
int miim_pvidSet( ushort_t PonPortIdx, uchar_t portId , ushort_t pvid);

int miim_vlanInit( ushort_t PonPortIdx )
{
	uchar_t reg_val;
	/*mdioVlanEntry_t entry;*/
	ushort_t vid;
	ushort_t portlist;
	ushort_t toMasterPortlist;
	uchar_t slotno4 = 4;

	if( GetPonChipTypeByPonPort(PonPortIdx) != PONCHIP_PAS5201 )
		return OK;

	/* slot4热拔插时，如果原来为空热插入PON板，或者原来为SW板热拔并插入PON板，
	    下列配置对于slot4存在问题，暂不考虑 */
	if( __SYS_MODULE_TYPE__(slotno4) == MODULE_E_GFA_EPON )
		toMasterPortlist = BCM5325E_PORT4_BIT;
	else
		toMasterPortlist = BCM5325E_PORT4_BIT | BCM5325E_PORT5_BIT;

	/* 当slot4为PON板时，关闭和slot8相连的端口 */
	if( GetCardIdxByPonChip(PonPortIdx) == slotno4 )
	{
		miim_portEnableSet( PonPortIdx, 4, 0 );
	}
	
	/* create default VLAN */
	miim_defaultVlanCreate( PonPortIdx );

	/* create VLAN 2, PON1 */
	portlist = (BCM5325E_PORT1_BIT | toMasterPortlist);
	vid = 2;
	miim_vlanCreate( PonPortIdx, vid, portlist );
	miim_pvidSet( PonPortIdx, 0, vid );

	/* create VLAN 3, PON2 */
	portlist = (BCM5325E_PORT2_BIT | toMasterPortlist);
	vid = 3;
	miim_vlanCreate( PonPortIdx, vid, portlist );
	miim_pvidSet( PonPortIdx, 1, vid );

	/* create VLAN 4, PON3 */
	portlist = (BCM5325E_PORT3_BIT | toMasterPortlist);
	vid = 4;
	miim_vlanCreate( PonPortIdx, vid, portlist );
	miim_pvidSet( PonPortIdx, 2, vid );

	/* create VLAN 5, PON4 */
	portlist = (BCM5325E_PORT_MII_BIT | toMasterPortlist);
	vid = 5;
	miim_vlanCreate( PonPortIdx, vid, portlist );
	miim_pvidSet( PonPortIdx, 5, vid );

	/* The high 8-bits of the VID of incoming frames are not checked.*/
	reg_val = 0x20;
	miim_5325_write_nByte( PonPortIdx, 0x34, 0x05, 1, &reg_val );
	
	/* enable 8021Q, and SVL mode */
	reg_val = 0x80;
	miim_5325_write_nByte( PonPortIdx, 0x34, 0x00, 1, &reg_val );

	return OK;	
}

int miim_vlanCreate( ushort_t PonPortIdx, ushort_t vid, ushort_t portlist )
{
	mdioVlanEntry_t entry;

	if( vid > VLAN_MAX )	/* 为了处理简单，我们取vid的值不大于16 */
	{
		sys_console_printf(" vid can't be greater than 16\r\n");
		return ERROR;
	}
	VOS_MemZero(&entry, sizeof(mdioVlanEntry_t));
	entry.portlist = (portlist & 0x3f);
	entry.untaglist = entry.portlist;
	entry.valid = MDIO_VLAN_ENTRY_VALID;
	entry.h8_vid = (vid >> 4);
	vlanEntry[vid] = vid;
	if (OK != miim_vlanEntryWrt( PonPortIdx, vid, &entry ))
		return ERROR;	

	return OK;	
}

int miim_defaultVlanCreate( ushort_t PonPortIdx  )
{
	mdioVlanEntry_t entry;
	ushort_t vid = MDIO_VLAN_DEFAULT_VID;
	
	VOS_MemZero(&entry, sizeof(mdioVlanEntry_t));
	entry.portlist = 0x3f;
	entry.untaglist = 0x3f;
	entry.valid = MDIO_VLAN_ENTRY_VALID;
	entry.h8_vid = (vid >> 4);
	vlanEntry[vid] = vid;
	if (OK != miim_vlanEntryWrt( PonPortIdx, vid, &entry ))
		return ERROR;	

	return OK;	
}

int miim_vlanEntryWrt( ushort_t PonPortIdx, ushort_t vid, mdioVlanEntry_t *entry)
{
	int i = 0;
	mdioVlanAccess_t access;
	/*uchar_t *data = (uchar_t *)entry;*/

	if( miim_5325_write_nByte( PonPortIdx, 0x34, 0x08, 4, (uchar_t *)entry ) != OK )
	{
		sys_console_printf("write vlan error\r\n");
	}
	/*sys_console_printf("\r\nwrite:%02x-%02x-%02x-%02x\r\n", data[0],data[1],data[2],data[3] );*/

	access.reserved = 0;
	access.start_done = MDIO_VLAN_ACCESS_START;
	access.read_write = MDIO_VLAN_ACCESS_WRITE;
	access.h8_vid = ((vid >> 4) & 0xff);
	access.l4_vid =  (vid & 0x0f);

	if( miim_5325_write_nByte(PonPortIdx, 0x34, 0x06, 2, (uchar_t *)&access) != OK )
	{
		printf(" 1. Wt 0x%x Err!\r\n",0x06);
		return ERROR;
	}		
	for( i=0; i<mdio_wait_times; i++ )
	{
		miim_5325_read_nByte( PonPortIdx, 0x34, 0x06, 2, (uchar_t *)&access );
		if ( access.start_done == MDIO_VLAN_ACCESS_DOWN )
			break;
		mdio_delay_never;
	}
	if (i > mdio_wait_times)
	{
		sys_console_printf("vlan %d write error\r\n", vid);
		return ERROR;
	}
	/*sys_console_printf("vlan %d write ok\r\n", vid);*/
	return OK;	
}

int  miim_pvidSet( ushort_t PonPortIdx, uchar_t portId , ushort_t pvid)
{
	uchar_t reg_addr ;

	if( pvid != vlanEntry[pvid] )
		return ERROR;
	
	reg_addr = 0x10 + (portId << 1) ;
	/*regPageCs(0x34);
	if (OK != BCM5325EWordRegWrite(reg_addr , pvid))*/
	if( miim_5325_write_nByte( PonPortIdx, 0x34, reg_addr, 2, (uchar_t *)&pvid ) != OK )
	{
		sys_console_printf("set pvid error\r\n");
		return ERROR;
	}
	/*sys_console_printf(" set pvid ok\r\n" );*/

	return OK;
}

int miim_pvidGet( ushort_t PonPortIdx, uchar_t portId , ushort_t *pPvid)
{
	ushort_t pvid = 0;
	uchar_t reg_addr = 0x10 + (portId << 1) ;
	/*regPageCs(0x34);
	if (OK != BCM5325EWordRegRead(reg_addr , pvidP) )*/
	if( miim_5325_read_nByte(PonPortIdx, 0x34, reg_addr, 2, (uchar_t *)&pvid) != OK )
		return ERROR;

	*pPvid = pvid;
	/*sys_console_printf( " port%d, pvid=%d\r\n", portId, pvid );*/
	return OK;
}

int miim_portEnableSet( short int PonPortIdx, uchar_t port, uchar_t enable )
{
	uchar_t status = 0;
	uchar_t reg_addr = 0x00 + port;

	if( enable == 0 )
		status = 3;
	if( miim_5325_write_nByte( PonPortIdx, 0x00, reg_addr, 1, (uchar_t *)&status ) != OK )
	{
		sys_console_printf("set port %d-%d enable error\r\n", PonPortIdx, port );
		return ERROR;
	}
	return OK;
}

int miim_portEnableGet( ushort_t PonPortIdx, uchar_t port )
{
	uchar_t status = 0;
	uchar_t reg_addr = 0x00 + port;
	
	if( miim_5325_read_nByte( PonPortIdx, 0x00, reg_addr, 1, (uchar_t *)&status ) != OK )
	{
		sys_console_printf("get port %d-%d enable error\r\n", PonPortIdx, port );
		return 0;
	}
	/*sys_console_printf("port%d, status=%d\r\n", port+1, status );*/
	if( status & 0x03 )
		return 0;
	return 1;
}

int miim_cmdVlanShow( struct vty *vty )
{
	ushort_t vid = 0;
	ushort_t count = 0;
	uchar_t i, j, k;
	ushort_t PonPortIdx = 4;
	ULONG slotno = 7;
	
	mdioVlanEntry_t entry;
	mdioVlanAccess_t access;
	/*uchar_t *pv = (uchar_t*)&entry;*/

	for( slotno=4; slotno<=8; slotno++ )
	{
		if( __SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA_EPON )
			continue;
			
		PonPortIdx = GetPonPortIdxBySlot( slotno, FIRSTPONPORTPERCARD );
		if( GetPonChipTypeByPonPort(PonPortIdx) != PONCHIP_PAS5201 )
			continue;
		
		VOS_MemZero( &entry, sizeof(mdioVlanEntry_t) );

		vty_out( vty, "\r\nslot%d: vid  portlist(untagged)\r\n", slotno );
		vty_out( vty, "      ------------------------\r\n");	
		for(i = 0; i<VLAN_MAX; i++)
		{	
			if(0 == vlanEntry[i])
			{
				continue;
			}
			vid = vlanEntry[i];

			access.reserved = 0;
			access.start_done = MDIO_VLAN_ACCESS_START;
			access.read_write = MDIO_VLAN_ACCESS_READ;
			access.h8_vid = ((vid >> 4) & 0xff);
			access.l4_vid =  (vid & 0x0f);
			if( miim_5325_write_nByte(PonPortIdx, 0x34, 0x06, 2, (uchar_t *)&access) != OK )
			{
				vty_out( vty, " 1-Wt 0x06 Err!\r\n" );
				return ERROR;
			}

			for( k=0; i<mdio_wait_times; k++ )
			{
				if( miim_5325_read_nByte(PonPortIdx, 0x34, 0x06, 2, (uchar_t *)&access) != OK )
				{
					vty_out( vty, " 3-Rd 0x06 Err!\r\n" );
					return ERROR;
				}		
				if ( access.start_done == MDIO_VLAN_ACCESS_DOWN )
					break;
				mdio_delay_never;
			}	
			if( k >= mdio_wait_times )
			{
				vty_out( vty, " 2-Rd 0x06 Err!\r\n" );
				return ERROR;
			}		

			if( miim_5325_read_nByte(PonPortIdx, 0x34, 0x0c, 4, (uchar_t *)&entry) != OK )
			{
				vty_out( vty, " 4-Rd 0x0c Err!\r\n" );
				return ERROR;
			}		
			/*vty_out( vty, "\r\nread:%02x-%02x-%02x-%02x\r\n", pv[0],pv[1],pv[2],pv[3] );*/

			if( (entry.h8_vid |vid) != vid )
			{
				vty_out( vty, " vid error\r\n" );
				return ERROR;
			}		
			if( MDIO_VLAN_ENTRY_VALID != entry.valid )
			{
				vty_out( vty, " vlan invalid\r\n" );
				return ERROR;
			}	
			
			vty_out( vty, "       %2d   ", vid );
			for( j=0,count=0; count<6; count++ )
			{	
				if (entry.portlist & (1 << count))
				{
					(j==0)?vty_out( vty, "%d", (count+1)) : vty_out( vty, " ,%d", (count+1) );
					j++;
				}
			}
			vty_out( vty, "\r\n");
		}
		vty_out( vty, "\r\n");
	}
	vty_out( vty, "\r\n");
	
	return OK;
}

int miim_cmdPvidShow( struct vty *vty )
{
	uchar_t port = 0;
	ushort_t pvid = 0;
	ulong_t slotno = 7;
	ushort_t PonPortIdx = 4;
	int enable;

	for( slotno=4; slotno<=8; slotno++ )
	{
		if( __SYS_MODULE_TYPE__(slotno) != MODULE_E_GFA_EPON )
			continue;
			
		PonPortIdx = GetPonPortIdxBySlot( slotno, FIRSTPONPORTPERCARD );
		if( GetPonChipTypeByPonPort(PonPortIdx) != PONCHIP_PAS5201 )
			continue;

		vty_out( vty, "\r\nslot%d: port  pvid  enable\r\n", slotno);
		vty_out( vty, "      --------------------\r\n");
		for(port = 0; port<6; port++)
		{
			miim_pvidGet( PonPortIdx, port , &pvid );
			enable = miim_portEnableGet(PonPortIdx, port);
			vty_out( vty, "        %d     %d      %d\r\n", port, pvid, enable );
		}
	}
	return OK;
}
#endif

#ifdef	__cplusplus
}
#endif/* __cplusplus */


