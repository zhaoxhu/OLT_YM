#include "vxWorks.h"
#include "intLib.h"
#include "taskLib.h"
#include "string.h"
/*#include "../PAS/Platform/VxWorks/ETHERNET/ETHERNET_Vxworks_expo.h"*/

extern int Receive_polling_thread_func(void *lpPacketRecBuf, unsigned int buf_data_size);
extern int slotNum2PhyPort53212(int port);
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
extern int bcmRecPacketPreHandle(char *buf,unsigned int len,int *port, unsigned short  *vlan);
extern int bcmSendPacketPreHandler(char *pBuf,unsigned int len, int port ,unsigned short vlan, char pri);
extern int bcmRecPacketDelTag(char *buf,unsigned int len);

extern void iInit(void (*p_callback)(void * , unsigned int), void* pRecv, void* pSend);
extern STATUS iSend(char* pBuf, int len, int port ,unsigned short vlan,char pri);
extern STATUS iSend2(char* pBuf, int len);
#else
extern void iInit(void (*p_callback)(void * , unsigned int));
extern STATUS iSend(char* pBuf, int len);
#endif
/*Receive_Pas_fun  pas_reveive_func;*/

extern long sys_console_printf(const char *format,...); 

SEM_ID	sendSem;


/*==============================*/
int ethSendToMII(void *sendBuffP , unsigned int packetSize);
void initNetAppDriver(void);
int ethRcvFromMII(void *rcvBuffP , unsigned int packetSize);
/*******************************************************************************
**                                                                              
** Function Name: initNetAppDriver
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    initial MII port read/write driver
**  INPUTS:         No
**  OUTPUTS:        No                                                        
**  RETURN :   	    No                                                        
*******************************************************************************/  
void initNetAppDriver(void)
{
	sendSem = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE ) ;	
	/*iInit((void *)ethRcvFromMII);*/
}

/*******************************************************************************
**                                                                              
** Function Name: ethSendToMII
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    write data to MII port
**  INPUTS:         void *sendBuffP :point to send data buffer
**		    unsigned int packetSize : the length of send data
**  OUTPUTS:        No	                                                        
**  RETURN :   	    OK ; ERROR                                                      
*******************************************************************************/  
int testdebug = 1;
extern int Emac1ReInitCtrlChannelFlag;
int Emac1ErrorCounter=0;
int   EMAC1ERRORMAX = 20;
extern void iDeInit();
int (*Ctrlchannel_hoot_t) (void  *, unsigned int) = NULL;
int EthSendCounter=0;
int ethSendToMII_1(void *sendBuffP , unsigned int packetSize)
{
    int port;
	/*semTake(sendSem , WAIT_FOREVER) ;
	int iLockKey;*/
	STATUS ret = OK;
	EthSendCounter++;
	if(Ctrlchannel_hoot_t)
		Ctrlchannel_hoot_t(sendBuffP,packetSize);
	/*
	taskLock();
	iLockKey=intLock();
	if(testdebug == 1 )*/
#if defined(USE_BSP_PPC405) || defined(USE_BSP_1010)
#if 1
    port=*((char*)sendBuffP + 5);
	port=slotNum2PhyPort53212(port);	
	ret = iSend(sendBuffP, packetSize, port, 0, 0);
#else
	ret = iSend2(sendBuffP, packetSize);
#endif
#else
	ret = iSend(sendBuffP , packetSize);
#endif
	/*if( ret == EAGAIN)*/
	if( ret != OK)
	{
		/*semGive(sendSem) ;
		printf(" task:msg err %s\r\n", taskName(taskIdSelf()));*/
		Emac1ErrorCounter ++;
		if( Emac1ErrorCounter > EMAC1ERRORMAX )
			{
			Emac1ReInitCtrlChannelFlag = 0xaa55;
			Emac1ErrorCounter  = 0;
			}/*
		intUnlock(iLockKey);
		taskUnlock();*/
		return ERROR;	
	}/*
	intUnlock(iLockKey);
	taskUnlock();*/
	if(Emac1ErrorCounter > 0 ) 
		Emac1ErrorCounter --;
	/*semGive(sendSem) ;*/
	return OK ;
}

int ethSendToMII(void *sendBuffP , unsigned int packetSize)
{	
	int ret, iLockKey;
	semTake(sendSem,WAIT_FOREVER );
	iLockKey=intLock();
	ret = ethSendToMII_1(sendBuffP, packetSize);
	intUnlock(iLockKey);
	semGive(sendSem);

	return(ret);
}

/*******************************************************************************
**                                                                              
** Function Name: ethRcvFromMII
**  ___________________________________________________________________________ 
**                                                                              
**  DESCRIPTION:    read data from MII port
**  INPUTS:         void *sendBuffP :point to receive data buffer
**		    unsigned int packetSize : the length of receive data
**  OUTPUTS:        No	                                                        
**  RETURN :   	    OK ; ERROR                                                      
*******************************************************************************/  
int ethRcvFromMII(void *rcvBuffP , unsigned int packetSize)
{

#if 0
	unsigned char *dataBuffP;
	
	/*copy packet to new memory*/
	dataBuffP = (unsigned char *)g_malloc(packetSize) ;

	memset( dataBuffP, 0, packetSize );
	
	if(dataBuffP == NULL)
	{
		sys_console_printf("dataBuffP = NULL !\r\n");
		return (-1);
	}
	memcpy(dataBuffP , rcvBuffP , packetSize);
	/*offer packet to top layer*/
	/*if(pas_reveive_func == NULL)
		free(rcvBuffP);*/
	/*(*pas_reveive_func)(dataBuffP , packetSize);*/
	
	{
				int lPortNo = 1;
				int iLoop = 0;
				sys_console_printf("ethRcvFromMII()!	\r\n");
					     sys_console_printf("Rev bpdu \r\n");
					     sys_console_printf("length=%d \r\n",packetSize);
					     sys_console_printf("Pacekt Content:\r\n");
						      for(iLoop=0;iLoop<packetSize;iLoop++)
						      	{
			                                sys_console_printf("%02x ",dataBuffP[iLoop]);
							  }
							  sys_console_printf("----------------\r\n");
							  sys_console_printf("\r\n\r\n");  

			}	
#endif
	Receive_polling_thread_func( rcvBuffP , packetSize) ;

/*	free(dataBuffP );*/
	return OK ;
}