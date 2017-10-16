/**************************************************************
*
*   PonStdOam.h -- The pon 802.3ah Interface header
*
*  
*    Copyright (c)  2014.7 , GW Technologies Co., LTD.
*    All rights reserved.
* 
*    modification history
*
*   Version	  |      Date	   |    Change				|    Author	  
*   ----------|--- --------|---------------------|------------
*	1.00	        | 25/07/2014  | Creation				| liwei056
*
***************************************************************/

#ifndef _PON_STD_OAM_H_
#define  _PON_STD_OAM_H_


/******************************************************
 *	Definitions of Standard OAM
 ******************************************************
 */

/* discovery state */
/*
 *	The standard 802.3ah 2004 has two definitions of this enum 
 *  (section 30.3.6.1.4, page 52 and Annex 30B  page 187)
 *  we declare the enum as define at Annex30B
 */
typedef enum
{
    PON_DISCOVERY_LINK_FAULT            = 1 ,
    PON_DISCOVERY_ACTIVE_SEND_LOCAL     = 2 ,
    PON_DISCOVERY_PASSIVE_WAIT          = 3 ,
    PON_DISCOVERY_SEND_LOCAL_REMOTE     = 4 ,
    PON_DISCOVERY_SEND_LOCAL_REMOTE_OK  = 5 ,
    PON_DISCOVERY_SEND_ANY              = 6 
} PON_discovery_state_t;


int PonStdOamDiscovery(short int olt_id, short int llid, unsigned char link_mac[6]);
int PonStdOamDisconect(short int olt_id, short int llid, unsigned char link_mac[6]);

int PonStdOamPktRecvHandler(short int olt_id, short int llid, char *oam_pkt, int pkt_len);
int PonStdOamPktSendHandler(unsigned long slot_id, unsigned long port_id, unsigned long sub_if, char *oam_pkt, int pkt_len);
int PonStdOamPortIsUp(unsigned long slot_id, unsigned long port_id, unsigned long sub_if, unsigned long *is_up);
int PonStdOamGetPeerStatus(short int olt_id, short int llid, int status_code, unsigned long *status_val, unsigned char *status_buf);
int PonStdOamGetHeartInfos(short int olt_id, short int llid, unsigned short *send_size, unsigned char *send_data, unsigned short *recv_size, unsigned char *recv_data);
int PonStdOamGetHeartMode(short int olt_id, short int llid, int *heart_mode);
int PonStdOamSetHeartMode(short int olt_id, short int llid, int heart_mode);

int PonExtOamEvtTx(short int olt_id, short int llid, char *oam_pkt, int pkt_len);
int PonExtOamInfoTx(short int olt_id, short int llid, char *oam_pkt, int pkt_len);
int PonExtOamPktTx(short int olt_id, short int llid, char *oam_pkt, int pkt_len);


#endif

