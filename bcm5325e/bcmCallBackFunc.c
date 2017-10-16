/**************************************************************************************
 *
 *  bcmCallBackFunc.c 
 *
 *  Copyright (c) 2005-2009 GW Technologies Co., LTD.
 *  All rights reserved.
 *  
 *  modification history:
 *  ____________________
 *  2006-5-8 written by yubo
 *
 *  DESCRIPTION:
 *  BCM5325e callback function . 
 *  
 **************************************************************************************/
 
 #include "bcm5325eHeader.h"
 
 typedef unsigned char UCHAR ;
typedef unsigned long ULONG ;
 
 #define ETHERNET_INIT	1
 #define ETHERNET_MULTICAST_SETUP 2
 #define ETHERNET_MACADDRESS_SETUP 3
 #define ETHERNET_MACADDRESS_GET	4
 #define ETHERNET_LOOPBACK	5
 #define ETHERNET_NOLOOPBACK	6
 #define ETHERNET_PHY_CHECK_LINK	7
 #define ETHERNET_LOOPBACKTEST	8
 #define ETHERNET_EXTERNAL_LOOPBACKTEST 9
 #define ETHERNET_FDX	10
 #define ETHERNET_HDX 11
 #define ETHERNET_SPD_100M 12
 #define ETHERNET_SPD_10M 13
 #define ETHERNET_AUTO_NEGOTIATION 14
 #define ETHERNET_AUTO_NEGODISABLE 15
 #define ETHERNET_WRITEMACTOEEPROM 16
 #define ETHERNET_READMACFROMEEPROM 17
 #define ETHERNET_OK	18

 #define ETHERNET_CMD_ID_ILLEGAL 19       
 
/*******************************************************************************
**                                                                              
** Function Name: Drv_EthernetBcm5325E_IoCtl
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    set BCM5325E IO
**  INPUTS:         ULONG ulDeviceID : device ID
**									ULONG ulCmd : cmdID
**									UCHAR * pData : receive data
**  OUTPUTS:        	No                                                          
**  RETURN :   	    operate status                                                         
*******************************************************************************/  
int Drv_EthernetBcm5325E_IoCtl( ULONG ulDeviceID, ULONG ulCmd, UCHAR * pData )
{
		ULONG ulTemp = ETHERNET_OK;
		/*UCHAR type = 0 , *arlP ;*/

    switch ( ulCmd )
    {
    		case ETHERNET_INIT:
    		{
					/*hwResetVt6526Drv( 2);  other switch init */
          /*initPortVlan() ;  	*/							/*set port vlan : default value */
          break;
				}
        case ETHERNET_MULTICAST_SETUP:
          break;
        case ETHERNET_MACADDRESS_SETUP:			/*set BCM5325E MAC*/
				{
					/*writeARLEntry(arlP , type) ;*/
	        break;
	      }
				case ETHERNET_MACADDRESS_GET:            
        case ETHERNET_LOOPBACK:                                   
        case ETHERNET_NOLOOPBACK:                                      
        case ETHERNET_PHY_CHECK_LINK:                                      
        case ETHERNET_LOOPBACKTEST:
        case ETHERNET_EXTERNAL_LOOPBACKTEST:
        case ETHERNET_FDX:
        case ETHERNET_HDX:
        case ETHERNET_SPD_100M:               
        case ETHERNET_SPD_10M:   
        case ETHERNET_AUTO_NEGOTIATION:
        case ETHERNET_AUTO_NEGODISABLE:
        case ETHERNET_WRITEMACTOEEPROM:        
        case ETHERNET_READMACFROMEEPROM:
					break;
				default:
          ulTemp = (ULONG)ETHERNET_CMD_ID_ILLEGAL;
          break ;
		}
		if ( ulTemp != ETHERNET_OK)
    {
        /* guxiaowei modified 03/1/10 13:28:12 */
        /*printf("\r\n IoCtl: IoCtrl Error = %d \r\n", ulTemp );*/
    }
    return ( ( LONG ) ulTemp );
}
 