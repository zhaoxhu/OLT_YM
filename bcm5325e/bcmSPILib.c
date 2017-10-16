/**************************************************************************************
 *
 *  bcmSPILib.c 
 *
 *  Copyright (c) 2005-2009 GW Technologies Co., LTD.
 *  All rights reserved.
 *  
 *  modification history:
 *  ____________________
 *  2006-5-8 written by yubo
 *  2006-5-16 add opration to GPIO (write_gpio();read_gpio();)by Liu Chunyan
 *  2006-5-16 change SPI timming by Liu Chunyan
 *
 *  DESCRIPTION:
 *  SPI Lib. 
 *  
 *  The BCM5325E behaves only as a slave device. 
 *  SS is asynchronous. If SS is asserted during SCK high then BCM5325E samples data on the rising edge 
 *  of SCK and references the falling edge to output data. 
 *  Otherwise BCM5325E samples data on the falling edge and outputs data on the rising edge of SCK. 
 *        
 *
 **************************************************************************************/
#include "bcm5325eHeader.h"
#include "V2R1_product.h"

typedef int bit;
#define ERROR (-1)

/*BYTE SCK , SS , MOSI , MISO ;*/

/* removed by xieshl 20090123, 和BSP定义重复 */
#if 1
extern bit read_gpio(int GPIOx);
extern void write_gpio(int GPIOx,bit a);
#else	
void write_gpio(int GPIOx,bit a)
{
    unsigned long reg_data;
    reg_data = *(volatile unsigned long *)GPIO0_OR;
    if (a)
        *(volatile unsigned long *)GPIO0_OR = reg_data | (0x80000000>>GPIOx);/*GPIOx=1;*/
    else
        *(volatile unsigned long *)GPIO0_OR = reg_data & (~(0x80000000>>GPIOx));/*GPIOx=0;*/
    if (GPIOx<16)
    {
        reg_data = *(volatile unsigned long *)GPIO0_OSRH;
        *(volatile unsigned long *)GPIO0_OSRH = reg_data & (~(0xc0000000>>(GPIOx * 2)));/*OSRH[]=00;*/
    }
    else
    {
        reg_data = *(volatile unsigned long *)GPIO0_OSRL;
        *(volatile unsigned long *)GPIO0_OSRL = reg_data & (~(0xc0000000>>(GPIOx*2-32)));/*OSRL[]=00;*/
    }
    reg_data = *(volatile unsigned long *)GPIO0_ODR;
    *(volatile unsigned long *)GPIO0_ODR = reg_data & (~(0x80000000>>GPIOx));/*ODR[x]=0;*/
    reg_data = *(volatile unsigned long *)GPIO0_TCR;
    *(volatile unsigned long *)GPIO0_TCR = reg_data | (0x80000000>>GPIOx);/*TCR[x]=[1];*/
}

bit read_gpio(int GPIOx)
{
    unsigned long reg_data;
    reg_data = *(volatile unsigned long *)GPIO0_ODR;
    *(volatile unsigned long *)GPIO0_ODR = reg_data & (~(0x80000000>>GPIOx));/*ODR[x]=0;*/
    reg_data = *(volatile unsigned long *)GPIO0_TCR;
    *(volatile unsigned long *)GPIO0_TCR = reg_data & (~(0x80000000>>GPIOx));/*TCR[x]=[0];*/
    reg_data = *(volatile unsigned long *)GPIO0_IR & (0x80000000>>GPIOx);
    return(reg_data);
}
#endif
#if  !defined(BCM_DRV_568)&& !defined(BCM_DRV_635)&& !defined(BCM_DRV_646)
/*
** zhangxinhui, 2011-01-20
** 在568的驱动使用中证实该驱动有问题，
** 所以一起改成正确的接口
*/
#if 1
BYTE currentPG = 0;

/*******************************************************************************
**                                                                              
** Function Name: spiRead
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    SPI read function 
**  INPUTS:         BYTE address : the address of register
**  OUTPUTS:        	No                                                          
**  RETURN :   	    the value of register                                                          
*******************************************************************************/  
BYTE spiRead(BYTE address)
{
	BYTE	cmd[8] = {0};	/*CMD BIT*/
	BYTE	addr[8] = {0};	/*ADDR BIT*/
	BYTE	data[8] = {0};	/*DATA BIT*/
	BYTE	bits ;
	BYTE	value = 0 ;
	/* taskLock(); */
	
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		cmd[bits] = ((SPIREAD & (0x01 << bits)) >> bits) ;
		addr[bits] = ((address & (0x01 << bits)) >> bits) ;
	}
	
	/*initial status*/
	write_gpio(SCK,1);  /*SCK = 1 ;*/
	write_gpio(SS,1);    /*SS = 1 ;*/
	write_gpio(MOSI,0); /*MOSI = 0 ;*/
	write_gpio(MISO,0); /*MISO = 0 ;*/

	/*
	** The BCM5325E behaves only as a slave device. SS is asynchronous. If SS is asserted during SCK high then
	** BCM5325E samples data on the rising edge of SCK and references the falling edge to output data. Otherwise
	** BCM5325E samples data on the falling edge and outputs data on the rising edge of SCK.
	*/
	spi_delay();
	write_gpio(SS,0);	/*SS = 0 ; (1) CS valid */
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
        	write_gpio(SCK,0);              /*SCK = 1 ;*/
		write_gpio(MOSI,cmd[7-bits]); /*MOSI = cmd[7 - bits] ;*/
		write_gpio(SCK,1);              /*SCK = 0 ;*/
	}

	/*write_gpio(SCK,0);*/	                /*SCK = 0 ; (3) write ADDR BYTE */
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		write_gpio(SCK,0);              /*SCK = 1 ;*/
		write_gpio(MOSI,addr[7-bits]);/*MOSI = addr[7 - bits] ;*/
		write_gpio(SCK,1);              /*SCK = 0 ;*/
	}

	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		write_gpio(SCK,0);              /*SCK = 1 ;*/
		if (read_gpio(MISO))
		    data[7-bits] = 1 ;
		else
		    data[7-bits] = 0 ; 
		write_gpio(SCK,1);              /*SCK = 0 ;*/
	}
	write_gpio(SS,1);   /*SS = 1 ;*/
	
	for(bits = 0 ; bits < BITNUMBER ; bits++)
		value |= (data[bits] << bits) ;

	/* taskUnlock(); */
	return value ;
}

/*******************************************************************************
**                                                                              
** Function Name: spiWrite
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    SPI write function 
**  INPUTS:         BYTE address : the address of register
**		    BYTE setData : the value of setting	
**  OUTPUTS:        No                                                          
**  RETURN :   	    No                                                         
*******************************************************************************/  
void spiWrite(BYTE address , BYTE *setData , BYTE nByte)
{
	BYTE	cmd[8] = {0};	/*CMD BIT*/
	BYTE	addr[8] = {0};	/*ADDR BIT*/
	BYTE	data[8] = {0};	/*DATA BIT*/
	BYTE	bits , n ;
	/* taskLock(); */
	
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		cmd[bits] = ((SPIWRITE & (0x01 << bits)) >> bits) ;
		addr[bits] = ((address & (0x01 << bits)) >> bits) ;
	}
	/*initial status*/
	write_gpio(SCK,1);		/*SCK = 1 ;*/
	write_gpio(SS,1);		/*SS = 1 ;*/
	write_gpio(MISO,0);
	write_gpio(MOSI,0);

	/*
	** The BCM5325E behaves only as a slave device. SS is asynchronous. If SS is asserted during SCK high then
	** BCM5325E samples data on the rising edge of SCK and references the falling edge to output data. Otherwise
	** BCM5325E samples data on the falling edge and outputs data on the rising edge of SCK.
	*/
	spi_delay();
	write_gpio(SS,0);		/*SS = 0 ; (1) CS valid */
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		write_gpio(SCK,0);/*SCK = 1 ;*/
		write_gpio(MOSI,cmd[7-bits]);/*MOSI = cmd[7 - bits] ;*/
		write_gpio(SCK,1);/*SCK = 0 ;*/
	}

	/*write_gpio(SCK,0);*/	/*SCK = 0 ; (3) write ADDR BYTE */
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		write_gpio(SCK,0);/*SCK = 1 ;*/
		write_gpio(MOSI,addr[7-bits]);/*MOSI = addr[7 - bits] ;*/
		write_gpio(SCK,1);/*SCK = 0 ;*/
	}
	
	for(n = 0 ; n < nByte ; n++)
	{
		for(bits = 0 ; bits < BITNUMBER ; bits++)
		{
			data[bits] = ((setData[n] & (0x01 << bits)) >> bits) ;
		}

		/*write_gpio(SCK,0);*/	/* SCK = 0 ;(4) write data BYTE1 */
		for(bits = 0 ; bits < BITNUMBER ; bits++)
		{
			write_gpio(SCK,0);	/*SCK = 1 ;*/
			write_gpio(MOSI,data[7-bits]);/*MOSI = data[7 - bits] ;*/
			write_gpio(SCK,1);/*SCK = 0 ;*/
		}
	}
	write_gpio(SS,1);   /*SS = 1 ;*/

	/* taskUnlock(); */
	return;
} 
/*******************************************************************************
**                                                                              
** Function Name: regSpiStatusIdle
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    judge the status of SPI
**  INPUTS:         No
**  OUTPUTS:        No                                                          
**  RETURN :   	    idle:OK ; busy:ERROR                                                         
*******************************************************************************/  
int regSpiStatusIdle(void)
{
	BYTE status = 0 ;
	BYTE count = 0 ;
	WORD delay ;
	BYTE retry = 0;
	extern BYTE currentPG;

	status = spiRead(SPISTATUSADDR) ;
	while((status & 0x80) != 0)
	{
		for(delay = 0 ; delay < 2000 ; delay++) ;
		status = spiRead(SPISTATUSADDR) ;
		count++ ;

		if((count > 10) && (retry <= 10)){
			/* Write current page address, and try again */
			spiWrite(PAGERCSADDR , &currentPG , 1) ;
			count = 0;
			retry ++;
		}
		else if ((count >10) && (retry > 10))
			return ERROR;
		else
			continue;
	}
	return OK ;
}

/*******************************************************************************
**                                                                              
** Function Name: regSpiStatusRack
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    judge the status of SPI
**  INPUTS:         No
**  OUTPUTS:        No                                                          
**  RETURN :   	    complete:OK ; else:ERROR                                                         
*******************************************************************************/  
int regSpiStatusRack(void)
{
	BYTE status = 0 ;
	BYTE count = 0 ;
	WORD delay ;
	BYTE retry = 0;
	extern BYTE currentPG;

	status = spiRead(SPISTATUSADDR) ;
	while((status & 0x80) != 0)
	{
		for(delay = 0 ; delay < 2000 ; delay++) ;
		status = spiRead(SPISTATUSADDR) ;
		count++ ;

		if((count > 10) && (retry <= 10)){
			/* Write current page address, and try again */
			spiWrite(PAGERCSADDR , &currentPG , 1) ;
			count = 0;
			retry ++;
		}
		else if ((count >10) && (retry > 10))
			return ERROR;
		else
			continue;
	}
	return OK ;
}

/*******************************************************************************
**                                                                              
** Function Name: readSpiBYTE
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    read BYTE register
**  INPUTS:         No
**  OUTPUTS:        No                                                          
**  RETURN :   	    value of register                                                         
*******************************************************************************/  
BYTE readSpiBYTE(void)
{
	BYTE	value = 0 ;
	
	value = spiRead(SPIDATA0ADDR) ; 
	return value ;	
}

/*******************************************************************************
**                                                                              
** Function Name: regPageCs
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    write page register
**  INPUTS:         BYTE pageAddr : the pager address
**  OUTPUTS:        No                                                          
**  RETURN :   	    OK                                                         
*******************************************************************************/  
int regPageCs(BYTE pageAddr)
{
	BYTE temp ;
	temp = pageAddr ;
	currentPG = pageAddr;
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiWrite(PAGERCSADDR , &pageAddr , 1) ;
	return OK ;
}
#else
/*******************************************************************************
**                                                                              
** Function Name: spiRead
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    SPI read function 
**  INPUTS:         BYTE address : the address of register
**  OUTPUTS:        	No                                                          
**  RETURN :   	    the value of register                                                          
*******************************************************************************/  
BYTE spiRead(BYTE address)
{
	BYTE	cmd[8] = {0};	/*CMD BIT*/
	BYTE	addr[8] = {0};	/*ADDR BIT*/
	BYTE	data[8] = {0};	/*DATA BIT*/
	BYTE	bits ;
	BYTE	value = 0 ;
	
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		cmd[bits] = ((SPIREAD & (0x01 << bits)) >> bits) ;
		addr[bits] = ((address & (0x01 << bits)) >> bits) ;
	}
	/*initial status*/
	write_gpio(SCK,1);  /*SCK = 1 ;*/
	write_gpio(SS,1);   /*SS = 1 ;*/
	write_gpio(MOSI,0); /*MOSI = 0 ;*/
	write_gpio(MISO,0); /*MISO = 0 ;*/
	write_gpio(SS,0);	/*SS = 0 ; (1) CS valid */
	/*write_gpio(SCK,0);  SCK = 0 ; (2) write CMD BYTE */
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
        write_gpio(SCK,1);              /*SCK = 1 ;*/
		write_gpio(MOSI,cmd[7-bits]); /*MOSI = cmd[7 - bits] ;*/
		/*for(delay = 0 ; delay < 0xFF ; delay++) ;*/
		write_gpio(SCK,0);              /*SCK = 0 ;*/
	}
	/*for(delay = 0 ; delay < 0xFF ; delay++) ;*/
	write_gpio(SCK,0);	                /*SCK = 0 ; (3) write ADDR BYTE */
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		write_gpio(SCK,1);              /*SCK = 1 ;*/
		write_gpio(MOSI,addr[7-bits]);/*MOSI = addr[7 - bits] ;*/
		/*for(delay = 0 ; delay < 0xFF ; delay++) ;*/
		write_gpio(SCK,0);              /*SCK = 0 ;*/
	}
	/*for(delay = 0 ; delay < 0xFF ; delay++) ;*/
	write_gpio(SCK,0);                  /*SCK = 0 ;	 (4) read data BYTE1  (discard:in reading page register)*/
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		write_gpio(SCK,1);              /*SCK = 1 ;*/
		/*for(delay = 0 ; delay < 0xFF ; delay++) ;*/
		write_gpio(SCK,0);              /*SCK = 0 ;*/

		if (read_gpio(MISO))
		    data[7-bits] = 1 ;
		else
		    data[7-bits] = 0 ; 
	}
	write_gpio(SS,1);   /*SS = 1 ;*/
	
	for(bits = 0 ; bits < BITNUMBER ; bits++)
		value |= (data[bits] << bits) ;
	return value ;
}

/*******************************************************************************
**                                                                              
** Function Name: spiWrite
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    SPI write function 
**  INPUTS:         BYTE address : the address of register
**		    BYTE setData : the value of setting	
**  OUTPUTS:        No                                                          
**  RETURN :   	    No                                                         
*******************************************************************************/  
void spiWrite(BYTE address , BYTE *setData , BYTE nByte)
{
	BYTE	cmd[8] = {0};	/*CMD BIT*/
	BYTE	addr[8] = {0};	/*ADDR BIT*/
	BYTE	data[8] = {0};	/*DATA BIT*/
	BYTE	bits , n ;
	
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		cmd[bits] = ((SPIWRITE & (0x01 << bits)) >> bits) ;
		addr[bits] = ((address & (0x01 << bits)) >> bits) ;
	}
	/*initial status*/
	write_gpio(SCK,1);/*SCK = 1 ;*/
	write_gpio(SS,1);/*SS = 1 ;*/
	write_gpio(MISO,0);
	write_gpio(MOSI,0);
	write_gpio(SS,0);	/*SS = 0 ; (1) CS valid */
	/*write_gpio(SCK,0);	 SCK = 0 ;(2) write CMD BYTE */
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		write_gpio(SCK,1);/*SCK = 1 ;*/
		write_gpio(MOSI,cmd[7-bits]);/*MOSI = cmd[7 - bits] ;*/
		write_gpio(SCK,0);/*SCK = 0 ;*/
	}
	write_gpio(SCK,0);	/*SCK = 0 ; (3) write ADDR BYTE */
	for(bits = 0 ; bits < BITNUMBER ; bits++)
	{
		write_gpio(SCK,1);/*SCK = 1 ;*/
		write_gpio(MOSI,addr[7-bits]);/*MOSI = addr[7 - bits] ;*/
		write_gpio(SCK,0);/*SCK = 0 ;*/
	}
	for(n = 0 ; n < nByte ; n++)
	{
		for(bits = 0 ; bits < BITNUMBER ; bits++)
		{
#if 1		
			data[bits] = ((setData[n] & (0x01 << bits)) >> bits) ;
#else
			data[bits] = ((setData[nByte-n-1] & (0x01 << bits)) >> bits) ;/*高位在前*/
#endif
		}
		write_gpio(SCK,0);	/* SCK = 0 ;(4) write data BYTE1 */
		for(bits = 0 ; bits < BITNUMBER ; bits++)
		{
			write_gpio(SCK,1);/*SCK = 1 ;*/
			write_gpio(MOSI,data[7-bits]);/*MOSI = data[7 - bits] ;*/
			write_gpio(SCK,0);/*SCK = 0 ;*/
		}
	}
	write_gpio(SS,1);   /*SS = 1 ;*/

} 
/*******************************************************************************
**                                                                              
** Function Name: regSpiStatusIdle
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    judge the status of SPI
**  INPUTS:         No
**  OUTPUTS:        No                                                          
**  RETURN :   	    idle:OK ; busy:ERROR                                                         
*******************************************************************************/  
int regSpiStatusIdle(void)
{
	BYTE status = 0 ;
	BYTE count = 0 ;
	WORD delay ;
	
	status = spiRead(SPISTATUSADDR) ;
	while((status & 0x80) != 0)
	{
		for(delay = 0 ; delay < 2000 ; delay++) ;
		status = spiRead(SPISTATUSADDR) ;
		count++ ;
		if(count > 100)
			return ERROR ;
	}
	return OK ;
}
/*******************************************************************************
**                                                                              
** Function Name: regSpiStatusRack
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    judge the status of SPI
**  INPUTS:         No
**  OUTPUTS:        No                                                          
**  RETURN :   	    complete:OK ; else:ERROR                                                         
*******************************************************************************/  
int regSpiStatusRack(void)
{
	BYTE status = 0 ;
	BYTE count = 0 ;
	WORD delay ;
	
	status = spiRead(SPISTATUSADDR) ;
	while((status & 0x20) != 0x20)
	{
		for(delay = 0 ; delay < 2000 ; delay++) ;
		status = spiRead(SPISTATUSADDR) ;
		count++ ;
		if(count > 100)
			return ERROR ;
	}
	return OK ;
}
/*******************************************************************************
**                                                                              
** Function Name: readSpiBYTE
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    read BYTE register
**  INPUTS:         No
**  OUTPUTS:        No                                                          
**  RETURN :   	    value of register                                                         
*******************************************************************************/  
BYTE readSpiBYTE(void)
{
	BYTE	value = 0 ;
	
	value = spiRead(SPIDATA0ADDR) ; 
	return value ;	
}

/*******************************************************************************
**                                                                              
** Function Name: regPageCs
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    write page register
**  INPUTS:         BYTE pageAddr : the pager address
**  OUTPUTS:        No                                                          
**  RETURN :   	    OK                                                         
*******************************************************************************/  
int regPageCs(BYTE pageAddr)
{
	BYTE temp ;
	temp = pageAddr ;
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiWrite(PAGERCSADDR , &pageAddr , 1) ;
	return OK ;
}
#endif
#endif /*BCM_DRV_568*/

/*******************************************************************************
**                                                                              
** Function Name: BCM5325EByteRegRead
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    read byte register
**  INPUTS:         BYTE addr : the address of register
**  OUTPUTS:        BYTE *dataP : the value of register                                                         
**  RETURN :   	    ok                                                        
*******************************************************************************/  
int BCM5325EByteRegRead(BYTE addr , BYTE *dataP)
{
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiRead(addr) ;
	if(regSpiStatusRack() != OK)
		return ERROR ;
	*dataP = readSpiBYTE() ; 
	return OK ;
}

/*******************************************************************************
**                                                                              
** Function Name: BCM5325EWordRegRead
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    read byte register
**  INPUTS:         BYTE addr : the address of register
**  OUTPUTS:        WORD *dataP : the value of register                                                         
**  RETURN :   	    ok                                                        
*******************************************************************************/  
int BCM5325EWordRegRead(BYTE addr , WORD *dataP) 
{
	BYTE temp = 0 ;
	WORD value = 0 ;

	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiRead(addr) ;
	if(regSpiStatusRack() != OK)
		return ERROR ;
	temp = spiRead(SPIDATA0ADDR) ; 
	value |= temp ;
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiRead(addr + 1) ;
	if(regSpiStatusRack() != OK)
		return ERROR ;
	temp = spiRead(SPIDATA0ADDR) ;
	value |= (temp << 8) ;
	*dataP = value ;
	return OK ;
}

/*******************************************************************************
**                                                                              
** Function Name: BCM5325EDwordRegRead
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    read byte register
**  INPUTS:         BYTE addr : the address of register
**  OUTPUTS:        DWORD *dataP : the value of register                                                         
**  RETURN :   	    ok                                                        
*******************************************************************************/  
int BCM5325EDwordRegRead(BYTE addr , DWORD *dataP) 
{
	BYTE temp = 0 ;
	DWORD value = 0 ;

	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiRead(addr) ;
	if(regSpiStatusRack() != OK)
		return ERROR ;
	temp = spiRead(SPIDATA0ADDR) ; 
	value |= temp ;
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiRead(addr + 1) ;
	if(regSpiStatusRack() != OK)
		return ERROR ;
	temp = spiRead(SPIDATA0ADDR) ;
	value |= (temp << 8) ;
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiRead(addr + 2) ;
	if(regSpiStatusRack() != OK)
		return ERROR ;
	temp = spiRead(SPIDATA0ADDR) ;
	value |= (temp << 16) ;
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiRead(addr + 3) ;
	if(regSpiStatusRack() != OK)
		return ERROR ;
	temp = spiRead(SPIDATA0ADDR) ;
	value |= (temp << 24) ;
	*dataP = value ;
	return OK ;
}

/*******************************************************************************
**                                                                              
** Function Name: BCM5325EByteRegWrite
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    write byte register
**  INPUTS:         BYTE addr : the address of register
**		    BYTE value : the value of setting   
**  OUTPUTS:        No                                                      
**  RETURN :   	    ok                                                        
*******************************************************************************/  
int BCM5325EByteRegWrite(BYTE addr , BYTE value)
{
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiWrite(addr , &value , 1) ;
	return OK ;
}
/*******************************************************************************
**                                                                              
** Function Name: BCM5325EWordRegWrite
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    write Word register
**  INPUTS:         BYTE addr : the address of register
**		    WORD value : the value of setting   
**  OUTPUTS:        No                                                      
**  RETURN :   	    ok                                                        
*******************************************************************************/  
int BCM5325EWordRegWrite(BYTE addr , WORD value)
{
	BYTE temp[2] = {0} ;

	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	temp[0] = (BYTE)(value & 0xFF) ;
	temp[1] = (BYTE)((value >> 8)& 0xFF) ; 
	spiWrite(addr , temp , 2) ;
	return OK ;
}
/*******************************************************************************
**                                                                              
** Function Name: BCM5325EDwordRegWrite
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    write Dword register
**  INPUTS:         BYTE addr : the address of register
**		    DWORD value : the value of setting   
**  OUTPUTS:        No                                                      
**  RETURN :   	    ok                                                        
*******************************************************************************/  
int BCM5325EDwordRegWrite(BYTE addr , DWORD value)
{
	BYTE temp[4] = {0} ;
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	temp[0] = (BYTE)(value & 0xFF) ;
	temp[1] = (BYTE)((value >> 8)& 0xFF) ;
	temp[2] = (BYTE)((value >> 16)& 0xFF) ;
	temp[3] = (BYTE)((value >> 24)& 0xFF) ;
	spiWrite(addr , temp , 4) ;
	return OK ;
}


/*added by wutw*/
int regPageCsRd(BYTE *pValue)
{
	if (pValue == 0)
		return (-1);
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	*pValue = spiRead(PAGERCSADDR);
	/*spiWrite(PAGERCSADDR , &pageAddr , 1) ;*/
	return OK ;
}

/*added by wutw*/
int regPageCsStatusRd(BYTE *pValue)
{
	if (pValue == 0)
		return (-1);
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	*pValue = spiRead(PAGERCSADDR);
	/*spiWrite(PAGERCSADDR , &pageAddr , 1) ;*/
	return OK ;
}


/*added by wutu 2006/12/8*/
/*******************************************************************************
**                                                                              
** Function Name: BCM5325EEntryRead
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    read byte register
**  INPUTS:         BYTE addr : the address of register
**  OUTPUTS:        DWORD *dataP : the value of register                                                         
**  RETURN :   	    ok                                                        
*******************************************************************************/  
int BCM5325EEntryRead(BYTE addr , BYTE *dataP, BYTE dataLen) 
{
	BYTE temp = 0 ;
	BYTE tempAddr = 0;
	BYTE i = 0;


	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	
	for(tempAddr = addr,i = 0; tempAddr < addr+dataLen; tempAddr++,i++)
	{
		spiRead(tempAddr) ;	
		if(regSpiStatusRack() != OK)
		return ERROR ;
		temp = spiRead(SPIDATA0ADDR) ; 
		dataP[i]= temp;
		/*printf("tempAddr = 0x%x\r\n",tempAddr);
		printf("temp 0x%x\r\n",temp);
		printf("*(dataP+1) 0x%x\r\n",*(dataP+1));
		printf("dataP[%d] 0x%x\r\n",i,dataP[i]);*/
	}
	return OK ;
}


/*added by wutu 2006/12/8*/
/*******************************************************************************
**                                                                              
** Function Name: BCM5325EEntryRead
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    read byte register
**  INPUTS:         BYTE addr : the address of register
**  OUTPUTS:        DWORD *dataP : the value of register                                                         
**  RETURN :   	    ok                                                        
*******************************************************************************/  
int BCM5325EEntryWrt(BYTE addr , BYTE *dataP, BYTE dataLen) 
{
	if(regSpiStatusIdle() != OK)		/* SPI Idle */
		return ERROR ;
	spiWrite(addr , dataP , dataLen) ;
	return OK ;
}

/*************************************test***********************************/

int bcm5635eRegRead(BYTE addr_page, BYTE addr_reg)
{
	BYTE reg_data ;
	regPageCs(addr_page) ;
	if (OK != BCM5325EByteRegRead(addr_reg , &reg_data) )
	{
		/*printf(" 1. Rd 0x%x Error!\r\n",addr_reg);*/
		return -1;
	}
/*	printf("reg[0x%02x] value = 0x%02x\n",addr_reg,reg_data);*/
	return 0;
}

int bcm5635eRegWrt(BYTE addr_page, BYTE addr_reg, BYTE value)
{
	BYTE reg_data ;
	regPageCs(addr_page) ;
	if (OK != BCM5325EByteRegRead(addr_reg , &reg_data) )
	{
		/*printf(" 1. Rd 0x%x Error!\r\n",addr_reg);*/
		return -1;
	}
/*	printf("reg[0x%02x] value = 0x%02x\n",addr_reg,reg_data);*/

	reg_data = value;
	if (OK != BCM5325EByteRegWrite(addr_reg,reg_data))
	{
	/*	printf(" 1. Wt 0x%x Err!\r\n",addr_reg);*/
		return -1;
	}
	if (OK != BCM5325EByteRegRead(addr_reg , &reg_data) )
	{
		/*printf(" 1. Rd 0x%x Error!\r\n",addr_reg);*/
		return -1;
	}
	/*printf("reg[0x%02x] value = 0x%02x\n",addr_reg,reg_data);*/

	return 0;
}


int bcm5635edoubleTypeWrt(BYTE addr_page, BYTE addr_reg, WORD value)
{
	WORD reg_data ;
	regPageCs(addr_page) ;
	if (OK != BCM5325EWordRegRead(addr_reg , &reg_data) )
	{
	/*	printf(" 1. Rd 0x%x Error!\r\n",addr_reg);*/
		return -1;	
	}
/*	printf("reg[0x%02x] value = 0x%02x\n",addr_reg,reg_data);*/
	if (OK != BCM5325EWordRegWrite(addr_reg , value))
	{
		/*printf(" 1. Wt 0x%x Err!\r\n",addr_reg);*/
		return -1;
	}
	
	if (OK != BCM5325EWordRegRead(addr_reg , &reg_data) )
	{
	/*	printf(" 1. Rd 0x%x Error!\r\n",addr_reg);*/
		return -1;	
	}
/*	printf("reg[0x%02x] value = 0x%02x\n",addr_reg,reg_data);*/
	return 0;
}


int bcm5635edoubleTypeRd(BYTE addr_page, BYTE addr_reg)
{
	WORD reg_data ;
	regPageCs(addr_page) ;
	if (OK != BCM5325EWordRegRead(addr_reg , &reg_data) )
	{
	/*	printf(" 1. Rd 0x%x Error!\r\n",addr_reg);*/
		return -1;	
	}
/*	printf("reg[0x%02x] value = 0x%02x\n",addr_reg,reg_data);*/
	return 0;
}




