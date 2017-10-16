#ifndef BCM5325EHEADER_H
#define BCM5325EHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*typedef NULL	0;*/
typedef unsigned char BYTE ;
typedef unsigned short WORD ;
typedef unsigned long DWORD ;

/*typedef unsigned char UCHAR ;
typedef unsigned long ULONG ;*/
typedef long	LONG ; 
typedef char	CHAR ;
#include "bcmSPILib.h"
#include "bcm5325eARLLib.h"
#include "bcm5325eCtrlRegLib.h"
#include "bcm5325eManageLib.h"
#include "bcm5325eStatusLib.h"
#include "bcm5325eVlanLib.h"

/*#define ERROR (-1)*/
#define OK 0

#ifdef __cplusplus
}
#endif

#endif	/*_BCM5325EHEADER_H_*/	