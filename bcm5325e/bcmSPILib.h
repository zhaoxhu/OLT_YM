#ifndef _BCMSPILIB_H_
#define	_BCMSPILIB_H_

/*
typedef unsigned char BYTE ;
typedef unsigned short WORD ;
typedef unsigned long DWORD ;
*/

#define	BITNUMBER	8

/*
#define	SCK
#define	SS
#define	MOSI
#define	MISO
*/

#define	PAGERCSADDR	0xFF
#define SPISTATUSADDR	0xFE
#define	SPIDATA0ADDR	0xF0

#define SPIREAD		0x60
#define	SPIWRITE	0x61

int regPageCs(BYTE pageAddr) ;
int BCM5325EByteRegRead(BYTE addr , BYTE *dataP) ;
int BCM5325EWordRegRead(BYTE addr , WORD *dataP) ;
int BCM5325EDwordRegRead(BYTE addr , DWORD *dataP) ;
int BCM5325EByteRegWrite(BYTE addr , BYTE value) ;
int BCM5325EWordRegWrite(BYTE addr , WORD value) ;
int BCM5325EDwordRegWrite(BYTE addr , DWORD value) ;
int BCM5325EEntryRead(BYTE addr , BYTE *dataP, BYTE dataLen) ;
int BCM5325EEntryWrt(BYTE addr , BYTE *dataP, BYTE dataLen);
int bcm5635eRegWrt(BYTE addr_page, BYTE addr_reg, BYTE value);
#define GPIO0_BASE          0xEF600700

#define GPIO0_OR               (GPIO0_BASE+0x0)   /* Output Register          */
#define GPIO0_TCR              (GPIO0_BASE+0x4)   /* Three-State Control      */
#define GPIO0_OSRH             (GPIO0_BASE+0x8)   /* Output Select High       */
#define GPIO0_OSRL             (GPIO0_BASE+0xC)   /* Output Select Low        */
#define GPIO0_TSRH             (GPIO0_BASE+0x10)  /* Three-State Select High  */
#define GPIO0_TSRL             (GPIO0_BASE+0x14)  /* Three-State Select Low   */
#define GPIO0_ODR              (GPIO0_BASE+0x18)  /* Open Drain Register      */
#define GPIO0_IR               (GPIO0_BASE+0x1C)  /* Input Register           */
#define GPIO0_RR1              (GPIO0_BASE+0x20)  /* Receive Register 1       */
#define GPIO0_RR2              (GPIO0_BASE+0x24)  /* Receive Register 2       */
#define GPIO0_ISR1H            (GPIO0_BASE+0x30)  /* Input Select 1 High      */
#define GPIO0_ISR1L            (GPIO0_BASE+0x34)  /* Input Select 1 Low       */
#define GPIO0_ISR2H            (GPIO0_BASE+0x34)  /* Input Select 2 High      */
#define GPIO0_ISR2L            (GPIO0_BASE+0x3C)  /* Input Select 2 Low       */

/*
#define SCK                    0
#define SS                     1
#define MISO                   2
#define MOSI                   3
*/


#endif	/*_BCMSPILIB_H_*/
