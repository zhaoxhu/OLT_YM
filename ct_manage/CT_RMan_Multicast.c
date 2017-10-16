#include  "OltGeneral.h"
#ifdef CTC_OBSOLETE		/* removed by xieshl 20120601 */
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "gwEponSys.h"/*add by shixh@20070730*/
#include  "../cli/Olt_cli.h"

#include "CT_RMan_Main.h"

#ifdef __CT_EXTOAM_SUPPORT


/*----------------------------------------------------------------------------*/

void pntPdu( char* pdu, USHORT pdulen )
{
	USHORT i=0;

    char sz[80]="";
	sys_console_printf("\r\nprint pdu dat is :\r\n");
	
	for( i = 0; i<pdulen; i++ )
		{
			VOS_Sprintf( sz+3*(i%16), "%02X ", pdu[i] );
			if( (i+1)%16 == 0 )
			{
				sys_console_printf( "%s\r\n", sz );
				VOS_MemSet( sz, 0, 80 );
			}
		}
}

int CT_RMan_MultiVlan_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_MultiVlan_t* pMultiVlan )
{

    char *pRecvPduData = NULL;
	ULONG	recvlen = 0;
    short int ponid = 0;
	int headlen = 0;
	pdu_head_t *pdu = (pdu_head_t *)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;

	headlen = sizeof( pdu_head_t );
	VOS_MemSet( pdu, 0, headlen );

	sys_console_printf("\r\ninit pdu OK!, headlen=%d\r\n", headlen );

	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 0x01;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_ATTRIBUTE;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MULTCASTVLAN;

	sys_console_printf( "\r\npdu generate OK !\r\n" );

	*(((char*)pdu)+headlen)=0x01;
	*(((char*)pdu)+headlen+1)= MULTICAST_VID_LIST;

	sys_console_printf( "\r\nact code add OK!\r\n" );

	pntPdu( (char*)pdu, headlen+2 );

    ponid = GetPonPortIdxBySlot( slotno, pon );
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+2, &pRecvPduData, &recvlen ) == VOS_OK )
	{
		char * p = pRecvPduData+headlen;
		int width = *p;
		pMultiVlan->vidnum = (width-1)/2;

		VOS_MemCpy( (char*)&pMultiVlan->vlanid[0], p+1, width );
			
		return VOS_OK;
	}

	CT_ExtOam_Free( pdu );
	
	return VOS_ERROR;
}

int CT_RMan_MultiVlan_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, CT_RMan_MultiVlan_t* pMultiVlan )
{

    pdu_ack_t *pRecvPduData = NULL;
	ULONG	recvlen = 0;
    short int ponid = 0;
	int headlen = 0;
	pdu_head_t *pdu = (pdu_head_t *)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;

	headlen = sizeof( pdu_head_t );
	VOS_MemSet( pdu, 0, headlen );

	pdu->opcode = EXT_VAR_SET_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 0x01;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_OPERATION;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MULTCASTVLAN;

	*(((char*)pdu)+headlen)= pMultiVlan->vidnum*2+1;
	VOS_MemCpy( ((char*)pdu)+headlen+1, pMultiVlan, pMultiVlan->vidnum*2+1 );

    ponid = GetPonPortIdxBySlot( slotno, pon );
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+2+2*pMultiVlan->vidnum, &pRecvPduData, &recvlen ) == VOS_OK )
	{
		if( pRecvPduData->ack == SET_REQUEST_RET_OK )

			return VOS_OK;
	}
	return VOS_ERROR;
}

int CT_RMan_MultiTagStripe_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR* pTagStripe )
{
	pdu_head_t *pdu = NULL;

    short int ponid = 0;

	char* pRecvPduData = NULL;

	ULONG	recvlen = 0;
	
	int headlen = sizeof( pdu_head_t );

	pdu = (pdu_head_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;

	VOS_MemSet( (char*)pdu, 0, headlen );

	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 0x01;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_ATTRIBUTE;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MULTCASTTAGSTRIPE;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen, &pRecvPduData, &recvlen ) == VOS_OK )
	{
		*pTagStripe = *(pRecvPduData+headlen+1);
		
	}

	CT_ExtOam_Free( pdu );
	
	return VOS_OK;
}

int CT_RMan_MultiTagStripe_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort,
			UCHAR call_flag, ULONG call_notify, UCHAR TagStripe )
{

	pdu_head_t *pdu = NULL;

    short int ponid = 0;

	pdu_ack_t* pRecvPduData = NULL;

	ULONG	recvlen = 0;
	
	int headlen = sizeof( pdu_head_t );

	pdu = (pdu_head_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;
	
	VOS_MemSet( (char*)pdu, 0, headlen );

	pdu->opcode = EXT_VAR_SET_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 0x01;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_OPERATION;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MULTCASTTAGSTRIPE;

	ponid = GetPonPortIdxBySlot( slotno, pon );

	*((char*)pdu+headlen) = 0x01;
	*((char*)pdu+headlen+1)=TagStripe;
	
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+2, &pRecvPduData, &recvlen ) == VOS_OK &&
		pRecvPduData->ack == SET_REQUEST_RET_OK )
	{
		CT_ExtOam_Free( pdu );
		return VOS_OK;
	}

	CT_ExtOam_Free( pdu );
	
	return VOS_ERROR;
}

int CT_RMan_MultiSwitch_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, UCHAR* pSwitch )
{
	pdu_no_iis_t *pdu = NULL;

    short int ponid = 0;

	char* pRecvPduData = NULL;

	ULONG	recvlen = 0;
	
	int headlen = sizeof( pdu_no_iis_t );

	pdu = (pdu_no_iis_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;

	VOS_MemSet( (char*)pdu, 0, headlen );

	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->vd.branch = CT_EXT_ATTRIBUTE;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MULTCASTSWITCH;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen, &pRecvPduData, &recvlen ) == VOS_OK )
	{
		*pSwitch = *(pRecvPduData+headlen+1);
		
	}

	CT_ExtOam_Free( pdu );
	
	return VOS_OK;	

}

int CT_RMan_MultiSwitch_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, UCHAR Switch )
{
	pdu_no_iis_t *pdu = NULL;

    short int ponid = 0;

	pdu_ack_t* pRecvPduData = NULL;

	ULONG	recvlen = 0;
	
	int headlen = sizeof( pdu_no_iis_t );

	pdu = (pdu_no_iis_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;
	
	VOS_MemSet( (char*)pdu, 0, headlen );

	pdu->opcode = EXT_VAR_SET_CODE;
	pdu->vd.branch = CT_EXT_OPERATION;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MULTCASTSWITCH;
	

	ponid = GetPonPortIdxBySlot( slotno, pon );

	*((char*)pdu+headlen) = 0x01;
	*((char*)pdu+headlen+1)=Switch;
	
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+2, &pRecvPduData, &recvlen ) == VOS_OK &&
		pRecvPduData->ack == SET_REQUEST_RET_OK )
	{
		CT_ExtOam_Free( pdu );
		return VOS_OK;
	}

	CT_ExtOam_Free( pdu );
	
	return VOS_ERROR;
}

int CT_RMan_MultiCtrl_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag, 
			ULONG call_notify, CT_RMan_MultiCtrl_t* pMultiCtrl )
{
	pdu_no_iis_t *pdu = NULL;

    short int ponid = 0;

	char* pRecvPduData = NULL;

	ULONG	recvlen = 0;
	
	int headlen = sizeof( pdu_no_iis_t );

	pdu = (pdu_no_iis_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;

	VOS_MemSet( (char*)pdu, 0, headlen );

	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->vd.branch = CT_EXT_ATTRIBUTE;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MULTCASTCONTROL;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen, &pRecvPduData, &recvlen ) == VOS_OK )
	{
		int width = *( pRecvPduData+headlen );
		VOS_MemCpy( pMultiCtrl, pRecvPduData+headlen+1, width-1 );
	}

	CT_ExtOam_Free( pdu );
	
	return VOS_OK;	
}

int CT_RMan_MultiCtrl_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR call_flag,
			ULONG call_notify, CT_RMan_MultiCtrl_t* pMultiCtrl )
{
	pdu_no_iis_t *pdu = NULL;

    short int ponid = 0;

	pdu_ack_t* pRecvPduData = NULL;

	ULONG	recvlen = 0;

	int width = 0; 
	
	int headlen = sizeof( pdu_no_iis_t );

	pdu = (pdu_no_iis_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;
	
	VOS_MemSet( (char*)pdu, 0, headlen );

	pdu->opcode = EXT_VAR_SET_CODE;
	pdu->vd.branch = CT_EXT_OPERATION;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MULTCASTCONTROL;
	

	ponid = GetPonPortIdxBySlot( slotno, pon );

	if( pMultiCtrl->action == 2 )
	{
	    width = 1;
		*( ((char*)pdu)+headlen ) = width;
		*( ((char*)pdu)+headlen+1 ) = pMultiCtrl->action;
	}
	else
	{
	    width = 3+10*pMultiCtrl->itemnum;
		*( ((char*)pdu)+headlen )= width;
		VOS_MemCpy( ((char*)pdu)+headlen+1, (char*)pMultiCtrl, width );
	}
	
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+2, &pRecvPduData, &recvlen ) == VOS_OK &&
		pRecvPduData->ack == SET_REQUEST_RET_OK )
	{
		CT_ExtOam_Free( pdu );
		return VOS_OK;
	}

	CT_ExtOam_Free( pdu );
	
	return VOS_ERROR;
}

int CT_RMan_GroupNum_get( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR* pNum )
{
	pdu_head_t *pdu = NULL;

    short int ponid = 0;

	char* pRecvPduData = NULL;

	ULONG	recvlen = 0;
	
	int headlen = sizeof( pdu_head_t );

	pdu = (pdu_head_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;

	VOS_MemSet( (char*)pdu, 0, headlen );

	pdu->opcode = EXT_VAR_REQ_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 0x01;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_ATTRIBUTE;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MAXGROUPNUM;

	ponid = GetPonPortIdxBySlot( slotno, pon );
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen, &pRecvPduData, &recvlen ) == VOS_OK )
	{
		*pNum= *(pRecvPduData+headlen+1);
	}

	CT_ExtOam_Free( pdu );
	
	return VOS_OK;
}

int CT_RMan_GroupNum_set( UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify, UCHAR num )
{
	pdu_head_t *pdu = NULL;

    short int ponid = 0;

	pdu_ack_t* pRecvPduData = NULL;

	ULONG	recvlen = 0;
	
	int headlen = sizeof( pdu_head_t );

	pdu = (pdu_head_t*)CT_ExtOam_Alloc();
	if( pdu == NULL )
		return VOS_ERROR;
	
	VOS_MemSet( (char*)pdu, 0, headlen );

	pdu->opcode = EXT_VAR_SET_CODE;
	pdu->iit.branch = INSTANCE_INDEX_VAL;
	pdu->iit.leaf = CT_EXT_OBJ_LEAF_PORT;
	pdu->iit.width = 0x01;
	pdu->iit.value = ethPort;

	pdu->vd.branch = CT_EXT_OPERATION;
	pdu->vd.leaf = CT_EXT_ATTR_LEAF_MAXGROUPNUM;

	ponid = GetPonPortIdxBySlot( slotno, pon );

	*((char*)pdu+headlen) = 0x01;
	*((char*)pdu+headlen+1)=num;
	
	if( CT_ExtOAM_Send( ponid, onu, call_flag, call_notify, pdu, headlen+2, &pRecvPduData, &recvlen ) == VOS_OK &&
		pRecvPduData->ack == SET_REQUEST_RET_OK )
	{
		CT_ExtOam_Free( pdu );
		return VOS_OK;
	}

	CT_ExtOam_Free( pdu );
	
	return VOS_ERROR;
}

void test_MultiVlan_get(UCHAR slotno, UCHAR pon, UCHAR onu, UCHAR ethPort, 
			UCHAR call_flag, ULONG call_notify )
{
	CT_RMan_MultiVlan_t tdata[128];
	VOS_MemSet( &tdata[0], 0, sizeof(tdata) );

	if( CT_RMan_MultiVlan_get( slotno, pon, onu, ethPort, call_flag, call_notify, tdata ) == VOS_OK )
		{
			sys_console_printf( "\r\ntest_MultiVlan_get OK!" );
		}
}

#endif /*__CT_EXTOAM_SUPPORT*/

/*----------------------------------------------------------------------------*/

/* CLI */

/*static void printMultiVidList( struct vty* vty, short int* pSrc, short int num, int width, int numperrow )
{
	int entryIdx = 0;
	for( entryIdx=0; entryIdx<num; entryIdx++ )
	{
		int i=0;
		int rows = (num%numperrow==0)?num/numperrow:num/numperrow+1;
		
		for( i=0; i<rows;i++)
		{
			char sz[80]="";
			int j=0;
			for( j=0; j<numperrow; j++ )
				sprintf( sz+width*j, "%4d ", pSrc[i*numperrow+j] );
			VOS_StrCat( sz, "\r\n" );
			vty_out( vty, sz );
		}
	}
}*/


/*add by shixh@2007/08/03*/
/*show  multicast-vlan*/
DEFUN(show_multivlan,
	show_multivlan_cmd,
	"show ctc multicast-vlan {<port_list>}*1",
	"dispaly running configure data\n"
	CTC_STR
	"multicast-vlan information\n"
	"input port number\n")
{
	/*unsigned char		port_number;
	ULONG   vid;*/

	 ULONG   devIdx;
	 ULONG   brdIdx;
	 ULONG   pon;
	 ULONG   onu;
	 ULONG idx[5];
	 ULONG  ethIdx;
	 ulong_t   portNum;

	 /*CTC_STACK_multicast_vlan_t    m_vlan;*/
       if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM)
	{
		vty_out(vty, "\r\nno ethernet port\r\n");
		return CMD_WARNING;
	}
	

	if( argc == 0  )
	{
		idx[0] = devIdx;
		idx[1]=1;
		idx[2]=0;
		idx[3] =0;
		while(getNextEthMulcastVlanEntry(4,idx)==VOS_OK)
		{
			if(idx[0]!=devIdx)
				break;
			vty_out(  vty, " port%d multicast vlan id:%d\r\n", idx[2],idx[3]);
		}
	}
	else
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			if(ethIdx>portNum)
			{
			vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
			VOS_Free(_pulIfArray);
			return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();*/	/* removed by xieshl 20120906, 解决内存丢失问题，同时非法端口不处理，但不报错，下同 */
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if( ethIdx <= portNum )
		{
		   	idx[0] = devIdx;
			idx[1]=1;
			idx[2]=ethIdx;
			idx[3] =0;
			while(getNextEthMulcastVlanEntry(4,idx)==VOS_OK)
			{
				if(idx[0]!=devIdx)
					break;
				vty_out(  vty, " port%d multicast vlan id:%d\r\n", idx[2],idx[3]);
			} 
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}
	return CMD_SUCCESS;
}

static int config_ctc_mvlan( struct vty *vty, int addflag, ULONG devIdx, ULONG brdIdx, ULONG ethIdx, ULONG vid )
{
	if( addflag )
	{
		if (setEthMultiCastVlanStatus(devIdx, brdIdx, ethIdx, vid, 4)==VOS_OK)
		{
			vty_out(vty, "\r\n port=%d is add to vid:%d \r\n", ethIdx,vid);
			if (setEthMultiCastAction(devIdx,1,ethIdx,3)==VOS_ERROR)	
				vty_out(vty, "\r\n set error! \r\n");
		}
		else
			vty_out(vty, "\r\n set port=%d status error!!\r\n", ethIdx);
	}
	else
	{
		if (setEthMultiCastVlanStatus(devIdx, brdIdx, ethIdx, vid, 6)==VOS_OK)
		{
			vty_out(vty, "\r\n port=%d multicast vid%d is delete!!\r\n", ethIdx,vid);
			if (setEthMultiCastAction(devIdx,1,ethIdx,3)==VOS_ERROR)	
				vty_out(vty, "\r\n set error! \r\n");
		}	     
		else
			vty_out(vty, "\r\n port=%d multicast vid delete error!!\r\n", ethIdx);
	}
	return CMD_SUCCESS;
}

/*add by shixh@2007/08/03*/
/*multicast-vlan add port*/	  
DEFUN(multicast_vlan_add_delete,
	multicast_vlan_add_delete_cmd,
	"ctc multicast-vlan [all|<port_list>] [add|delete] <1-4094>",
	CTC_STR
	"config multicast-vlan\n"
	"input port number\n"
	"all ports\n"
	"add\n"
	"delete\n"
	"input vlan id\n"
	)
{
	ULONG   devIdx, brdIdx, ethIdx;
	ULONG   vid;
	ULONG   pon, onu;
	ULONG   portNum;

	int addflag = 0;
	 
       if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		(portNum > MAX_ETH_PORT_NUM) )
	{
		vty_out(vty, "\r\n no ethernet port\r\n");
		return CMD_WARNING;
	}
	
	vid=VOS_AtoI( argv[2] );
	if( (vid > 4094) || (vid == 0) )
		return CMD_WARNING;

	if(VOS_StriCmp(argv[1], "add")==0)
		addflag = 1;

	brdIdx = 1;
	if(VOS_StriCmp(argv[0], "all")==0)
	{
		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
		{
			config_ctc_mvlan( vty, addflag, devIdx, brdIdx, ethIdx, vid );
		}
	}			   
	else 
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			if(ethIdx>portNum)
			{
				vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();*/
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[0], ethIdx )
		if( ethIdx <= portNum )
		{
			config_ctc_mvlan( vty, addflag, devIdx, brdIdx, ethIdx, vid );
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}	
	       		
	return   CMD_SUCCESS;
  }

static int delete_ctc_mvlan_port( struct vty *vty, ULONG devIdx, ULONG brdIdx, ULONG ethIdx )
{
	ULONG idx[5];

       idx[0] = devIdx;
	idx[1] = brdIdx;
	idx[2] = ethIdx;
	idx[3] =0;
      	while(getNextEthMulcastVlanEntry(4, idx)==VOS_OK)
       {
      		if( idx[0] != devIdx || idx[2]!= ethIdx )
			continue;
      		if (setEthMultiCastVlanStatus(devIdx, brdIdx, idx[2], idx[3], 6)==VOS_OK)
			vty_out(vty, "\r\n port=%d multicast vid %d is delete!!\r\n", ethIdx, idx[3] );
		else
		   	vty_out(vty, "\r\n Set multicast vlan status error!!\r\n");	

	      	/*idx[0] = devIdx;
		idx[1]=1;
		idx[2]=ethIdx;
		idx[3] =0;*/
       }
	if (setEthMultiCastAction(devIdx, brdIdx, ethIdx, 3)==VOS_OK)
             	vty_out(vty, "\r\n delete port=%d all multicast vlan!!\r\n", ethIdx);
	else
		vty_out(vty, "\r\n get multicast vlan Action error!!\r\n");

	return CMD_SUCCESS;
}

/*add by shixh@2007/08/03*/
/* multicast-vlan delete all*/
DEFUN(undo_multicast_vlan,
	undo_multicast_vlan_cmd,
	"undo ctc multicast-vlan [all|<port_list>]",
	CTC_STR
	"undo operation\n"
	"multicast-vlan\n"
	"all ports\n"
	"port number\n"
	)
{
	 ULONG  devIdx, brdIdx, ethIdx;
	 ULONG  pon, onu;
	 
	 ULONG  portNum;
	 
       if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "\r\nno ethernet port\r\n");
		return CMD_WARNING;
	}
	
	if(VOS_StriCmp(argv[0], "all")==0)
	{
		for(ethIdx=1; ethIdx<=portNum; ethIdx++)
		{
			delete_ctc_mvlan_port( vty, devIdx, brdIdx, ethIdx );
		}				   
	}
	else 
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			if(ethIdx>portNum)
			{
				vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();*/
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if( ethIdx <= portNum )
		{
			delete_ctc_mvlan_port( vty, devIdx, brdIdx, ethIdx );
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}
	return   CMD_SUCCESS;
     }



/*add by shixh@2007/08/03*/
/*show  multicast-tag-stripe */
DEFUN(multicast_tag_stripe_get,
	multicast_tag_stripe_get_cmd,
	"show ctc multicast-tag-stripe {<port_list>}*1",
	SHOW_STR
	CTC_STR
	"show multicast-tag stripe\n"
	"input port number\n"
	)
{
       /*PON_olt_id_t		olt_id;
	PON_onu_id_t		onu_id;*/
	
 	 ULONG   devIdx, brdIdx, ethIdx;
 	 ULONG   strip;
  	 
         ulong_t  pon, onu;
	 
	int ret = CMD_SUCCESS;
	 /*   ULONG ulIfIndex = 0;	*/

	 char * tagStatus[] = {"error","unstriped","striped", "switch"};
	 
  	ULONG portNum = 0;   

	/*ulIfIndex = ( ULONG ) ( vty->index ) ;	*/
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "\r\nno ethernet port\r\n");
		return CMD_WARNING;
	}
	
	/*if( parse_onu_command_parameter( vty,  &olt_id, &onu_id ) == VOS_ERROR)
	{
		vty_out(  vty, "wrong olt or onu id\r\n" );
		ret =  CMD_WARNING;
	}*/
	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	{
		for( ethIdx=1; ethIdx<=portNum; ethIdx++ )
		{
			if( getEthMultiTagStriped(devIdx, 1, ethIdx,&strip)  == CTC_STACK_EXIT_OK ) 
			{
				if( strip > 3 )
					strip = 0;
				vty_out( vty, "\r\nport %d multicast-tag-stripe  is: %s\r\n", ethIdx, tagStatus[strip]);
				
				ret = CMD_SUCCESS;
			}
			else
			{
				vty_out( vty, "\r\n multicast-tag-stripe get fail!\r\n" );
				ret =  CMD_WARNING;
				
			}
		}
	}

	else
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if(ethIdx>portNum)
		{
		vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
		VOS_Free(_pulIfArray);
		return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();*/
		
		BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if( ethIdx <= portNum )
		{
			if( getEthMultiTagStriped(devIdx, 1, ethIdx,&strip)  == CTC_STACK_EXIT_OK ) 
				{
				if( strip > 3 )
					strip = 0;
				vty_out( vty, "\r\nport %d multicast-tag-stripe  is: %s\r\n", ethIdx,tagStatus[strip]);
				ret = CMD_SUCCESS;
				}
			else
				{
				vty_out( vty, "\r\n multicast-tag-stripe get fail!\r\n" );
				ret =  CMD_WARNING;
		       	 }
		}
		END_PARSE_PORT_LIST_TO_PORT();
	}

	return ret;
}


/*add by shixh@2007/08/03*/
/*set multicast-tag-stripe */
DEFUN(
	multicast_tag_stripe_set,
	multicast_tag_stripe_set_cmd,
	"ctc multicast-tag-stripe [all|<port_list>] [striped|unstriped|switch]",
	CTC_STR
	"set ctc multicast information\n"
	"all port\n"
	"port number\n"
	"tag striped\n"
	"tag unstriped\n"
	"tag switch\n"
	)
{
 	ULONG devIdx;
 	ULONG brdIdx;
 	ULONG ethIdx;
  	 
 	ULONG pon;
 	ULONG onu;
	
 	ULONG portNum = 0;
	ULONG strip;

	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "\r\nno ethernet port\r\n");
		return CMD_WARNING;
	}

	if( VOS_StriCmp(argv[1], "striped") == 0 )    
		strip = 2;
	else if( VOS_StriCmp(argv[1], "switch") == 0 )
		strip = 3;
	else
		strip = 1;
	
	if( VOS_StriCmp(argv[0], "all") == 0 )
	{
		for( ethIdx=1; ethIdx<=portNum; ethIdx++ )
		{
			if( setEthMultiTagStriped( devIdx, 1, ethIdx, strip) == VOS_ERROR)
				vty_out(vty, "\r\nset port %d multicast-tag error!\r\n", ethIdx);
		}
	}
	else
	{
		/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
			if(ethIdx>portNum)
			{
				vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
			}
		END_PARSE_PORT_LIST_TO_PORT();*/
		
  	   	BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if( ethIdx <= portNum )
		{
			if( setEthMultiTagStriped( devIdx, 1, ethIdx, strip) == VOS_ERROR )
				vty_out(vty, "\r\nset port %d multicast-tag error!\r\n", ethIdx);	
		}
		END_PARSE_PORT_LIST_TO_PORT();
  	}
	 
	return CMD_SUCCESS;
}

/*add by shixh@2007/08/03*/
/*show multicast-switch*/
DEFUN(
	multicast_switch_get,
	multicast_switch_get_cmd,
	"show ctc multicast",
	"dispaly running configure data\n"
	CTC_STR
	"multicast switch information\n")
{
   PON_olt_id_t				  olt_id;
   PON_onu_id_t				onu_id;
    short int OnuIdx;
   CTC_STACK_multicast_protocol_t  multicast_protocol;
   
	if( parse_onu_command_parameter( vty,  &olt_id, &OnuIdx, &onu_id ) == VOS_ERROR)
	{
		vty_out(  vty, "wrong olt or onu id\r\n" );
		return  CMD_WARNING;
       }
	if(argc==0)
	{
	#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
	if (OnuMgt_GetMulticastSwitch(olt_id, OnuIdx, &multicast_protocol)==VOS_OK)
	#else
	if (CTC_STACK_get_multicast_switch(olt_id,onu_id,&multicast_protocol)==VOS_OK)
	#endif
	{
    	 if(multicast_protocol==CTC_STACK_PROTOCOL_IGMP_SNOOPING)
      		 vty_out(  vty, "multicast switch is IGMP SNOOPING!\r\n" );
	 else
		vty_out(  vty, "multicast switch is CTC!\r\n" );	
	}
	}
/*else
        vty_out(  vty, "PARA ERROR!\r\n" );*/
	return CMD_SUCCESS;
}



/*add by shixh@2007/08/03*/
/*set  multicast-switch*/
DEFUN(multicast_switch_set,
	multicast_switch_set_cmd,
	"ctc multicast [enable|disable]",
	CTC_STR
	"set ctc multicast switch information\n"
	"set snooping\n"
	"set ctc\n"
	)
{
   PON_olt_id_t   olt_id;
   PON_onu_id_t  onu_id;
    short int OnuIdx;
   /*CTC_STACK_multicast_protocol_t  multicast_protocol;*/
   
	if( parse_onu_command_parameter( vty,  &olt_id, &OnuIdx, &onu_id ) == VOS_ERROR)
	{
		vty_out(  vty, "wrong olt or onu id\r\n" );
		return  CMD_WARNING;
       }

	if(VOS_StriCmp(argv[0], "enable")==0)
	{
	    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
		if (OnuMgt_SetMulticastSwitch(olt_id,OnuIdx,1)==VOS_OK)
	    #else
		if (CTC_STACK_set_multicast_switch(olt_id,onu_id,1)==VOS_OK)
	    #endif
	          vty_out(  vty, "multicast switch is set CTC!\r\n" );
	}
	else /*if(VOS_StriCmp(argv[0], "snooping")==0)*/
	{
	    #if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
		if (OnuMgt_SetMulticastSwitch(olt_id,OnuIdx,0)==VOS_OK)
		#else
	    if (CTC_STACK_set_multicast_switch(olt_id,onu_id,0)==VOS_OK)
		#endif
             vty_out(  vty, "multicast switch is set IGMP SNOOPING!\r\n" );
	}

	return CMD_SUCCESS;
}

/*add by shixh@2007/08/03*/
/*show  multicast-control*/
DEFUN(multicast_control_get,
	multicast_control_get_cmd,
	"show ctc multicast-control",
	CTC_STR
	"dispaly running configure data\n"
	"show multicast control information\n")
{
           PON_olt_id_t			   olt_id; 
           PON_onu_id_t			   onu_id;
		   PON_llid_t	llid;
		   ULONG     devIdx;
		   ULONG      brdIdx;
		   ULONG   pon;
		   ULONG   onu;
		   	
           CTC_STACK_multicast_control_t   multicast_control;

		    int   i;
          
        if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;
		
       if( parse_onuidx_command_parameter(devIdx, &olt_id, &onu_id) == VOS_ERROR )
		return VOS_ERROR;
	
	if( GetOnuOperStatus(olt_id, onu_id) != 1 )
	{
    		return VOS_ERROR;
	}
	llid = GetLlidByOnuIdx( olt_id, onu_id );
	if( llid == -1 )
		return VOS_ERROR;

		#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
		if(OnuMgt_GetMulticastControl(olt_id,onu_id,&multicast_control)==VOS_OK)
		#else
	    if(CTC_STACK_get_multicast_control(olt_id,llid,&multicast_control)==VOS_OK)	
		#endif
	    	{
              	vty_out(vty, "\r\n onu%d multicast control entries: count:%d\r\n", llid,multicast_control.num_of_entries);	              
			for(i=1;i<=multicast_control.num_of_entries;i++)
				{
				vty_out(vty, "  %d uesr id: %d\r\n", i,multicast_control.entries[i].user_id);
				vty_out(vty, "  %d vid: %d\r\n", i,multicast_control.entries[i].vid);
				vty_out(vty, "  %d DA: %02x%02x.%02x%02x.%02x%02x\r\n", i,multicast_control.entries[i].da[0],multicast_control.entries[i].da[1],multicast_control.entries[i].da[2],multicast_control.entries[i].da[3],multicast_control.entries[i].da[4],multicast_control.entries[i].da[5]);
				}	

			vty_out(vty, "\r\n onu%d multicast control type :", llid);
			if(multicast_control.control_type==CTC_STACK_MULTICAST_CONTROL_TYPE_DA_ONLY)
				vty_out(vty, "DA only\r\n");
			else if(multicast_control.control_type==CTC_STACK_MULTICAST_CONTROL_TYPE_DA_VID)
				 vty_out(vty, "DA vid\r\n");	
			else if(multicast_control.control_type==CTC_STACK_MULTICAST_CONTROL_TYPE_DA_SAIP)
				 vty_out(vty, "DA SA\r\n");	
			else if(multicast_control.control_type==CTC_STACK_MULTICAST_CONTROL_TYPE_DIP_VID)
				 vty_out(vty, "DIP vid\r\n");	
			else
			 	 vty_out(vty, "get fail\r\n");

			vty_out(vty, "\r\n onu%d multicast control action :", llid);
		      if(multicast_control.action==CTC_MULTICAST_CONTROL_ACTION_DELETE)
			  	vty_out(vty, "delete\r\n");
		      else if(multicast_control.action==CTC_MULTICAST_CONTROL_ACTION_ADD)
			  	vty_out(vty, "add\r\n");
			else if(multicast_control.action==CTC_MULTICAST_CONTROL_ACTION_CLEAR)
			  	vty_out(vty, "clear\r\n");
			else if(multicast_control.action==CTC_MULTICAST_CONTROL_ACTION_LIST)
			  	vty_out(vty, "list\r\n");
			else
				vty_out(vty, "get fail\r\n");
	    	}
	else
		vty_out(vty, "\r\n get multicast-control error!\r\n");
	return  CMD_SUCCESS;
}


/*add by shixh@2007/08/03*/
/*show multicast-group-number*/
DEFUN(multicast_group_number_get,
	multicast_group_number_get_cmd,
	"show ctc multicast-group {[all|<port_list>]}*1",
	SHOW_STR
	CTC_STR
	"show multicast group number\n"
	"All ports number\n"
	"Port number\n"
	)
{
      
              ULONG   num;

		ULONG devIdx;
		ULONG brdIdx;
		ULONG ethIdx;
		ULONG  pon;
		ULONG  onu;
		ULONG  portNum;
	

	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "\r\nno ethernet port\r\n");
		return CMD_WARNING;
	}
	

	if( argc == 0 || (argc==1&&VOS_StriCmp(argv[0], "all")==0) )
	   for(ethIdx=1;ethIdx<=portNum;ethIdx++)
	   	{
		if( getEthMultiGroupNum(devIdx, 1,ethIdx, &num)  == VOS_OK ) 			
		 	vty_out( vty, "\r\nport %d get multicast group num is %d\r\n",ethIdx,num);
	   	}
	else
	      {
	      /*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if(ethIdx>portNum)
		{
		vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
		VOS_Free(_pulIfArray);
		return CMD_WARNING;
		}
		END_PARSE_PORT_LIST_TO_PORT();*/
		
	      BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
		if( ethIdx <= portNum )
		{
		 if( getEthMultiGroupNum(devIdx, 1,ethIdx, &num)  == VOS_OK ) 			 
			vty_out( vty, "\r\nport %d get multicast group num is %d\r\n",ethIdx,num);
		}
		END_PARSE_PORT_LIST_TO_PORT();
		#if 0
		ethIdx = VOS_AtoI( argv[0] );
 		if(ethIdx<25&&ethIdx>0)
 			{
		       if( getEthMultiGroupNum(devIdx, 1,ethIdx, &num)  == VOS_OK ) 			
			    vty_out( vty, "\r\nport %d multicast group num is: %d\r\n",ethIdx,num);
		       else
			   	vty_out( vty, "\r\nport %d get multicast group num fail!!\r\n",ethIdx);
 			}
		else
			vty_out( vty, "\r\nout of range<1~24>!\r\n" );
		#endif
	       }

	       return CMD_SUCCESS;

}

/*add by shixh@2007/08/03*/
/*set port multicast-group-number*/
DEFUN(multicast_group_number_set,
	multicast_group_number_set_cmd,
	"ctc multicast-group [all|<port_list>] <1-64>",
	CTC_STR
	"set multicast group number\n"
	"all ports\n"
	"port object\n"
	"input multicast group number\n")
{
   ULONG   devIdx;
   ULONG   brdIdx;
   ULONG   ethIdx;
   ULONG   num;

   ULONG    pon;
   ULONG    onu;
   ULONG    portNum;
   
	if(parse_onu_devidx_command_parameter(/* ulIfIndex*/vty, &devIdx, &brdIdx, &pon, &onu) != VOS_OK )
		return CMD_WARNING;

	if( (getDeviceCapEthPortNum( devIdx, &portNum) != VOS_OK) ||
		portNum > MAX_ETH_PORT_NUM )
	{
		vty_out(vty, "\r\nno ethernet port\r\n");
		return CMD_WARNING;
	}
	
	if(argc==2)
	{
		 num=VOS_AtoI( argv[1] );
		 if( (num<65) && (num>0) )
		 {
			if(VOS_StriCmp(argv[0], "all")==0)
		   	{
		 		for(ethIdx=1;ethIdx<=portNum;ethIdx++)
	       			if (setEthMultiGroupNum(devIdx,1,ethIdx,num)==VOS_ERROR)
            					vty_out( vty, "\r\nport %d set multicast group num fail!!\r\n",ethIdx);
	           	}
			else
			{
				/*BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
				if(ethIdx>portNum)
				{
				vty_out(vty, "ethernet port is out of range %d\r\n", portNum);	
				VOS_Free(_pulIfArray);
				return CMD_WARNING;
				}
				END_PARSE_PORT_LIST_TO_PORT();*/
		
				BEGIN_PARSE_PORT_LIST_TO_PORT( argv[ 0], ethIdx )
				if( ethIdx <= portNum )
				{
				 if( setEthMultiGroupNum(devIdx, 1,ethIdx, num)  == VOS_ERROR ) 			
			   		vty_out( vty, "\r\nport %d set multicast group num fail!\r\n",ethIdx);
				}
				END_PARSE_PORT_LIST_TO_PORT();
				#if 0
				ethIdx=VOS_AtoI( argv[0] );
				if(ethIdx<25&&ethIdx>0)
					{
				    if (setEthMultiGroupNum(devIdx,1,ethIdx,num)==VOS_OK)
            			     vty_out(  vty, "set port%d multicast group num is:%d\r\n" ,ethIdx,num);
					}
				   else
					vty_out(  vty, "\r\n out of the range<1~25>\r\n" );
				   #endif
			}
		 }
		else
			vty_out(  vty, "\r\n out of the range<1~64>\r\n" );	
	}
	/*else
        	vty_out(  vty, "\r\n PARA ERROR!\r\n" );*/
      return CMD_SUCCESS;
}


/*----------------------------------------------------------------------------*/

LONG CT_RMan_Multicast_Command_Install()
{
	install_element ( ONU_CTC_NODE, &show_multivlan_cmd);
	install_element ( ONU_CTC_NODE, &multicast_vlan_add_delete_cmd);
	install_element ( ONU_CTC_NODE, &undo_multicast_vlan_cmd);
	
	install_element ( ONU_CTC_NODE, &multicast_tag_stripe_get_cmd);
	install_element ( ONU_CTC_NODE, &multicast_tag_stripe_set_cmd);

	install_element ( ONU_CTC_NODE, &multicast_switch_get_cmd);
	install_element ( ONU_CTC_NODE, &multicast_switch_set_cmd);
	
       install_element ( ONU_CTC_NODE, &multicast_control_get_cmd);
	   
	install_element ( ONU_CTC_NODE, &multicast_group_number_get_cmd);
	install_element ( ONU_CTC_NODE, &multicast_group_number_set_cmd);

	CT_MVlanSwitchCli_Init();

	return VOS_OK;
}

/*----------------------------------------------------------------------------*/
#endif


