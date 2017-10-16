
#ifdef __cpluplus
extern "C" {
#endif

#include "OltGeneral.h"
#include "PonGeneral.h"
#include "OnuGeneral.h"
#include "V2R1_Product.h"

#include "gwEponSys.h"
#include "gwEponMibData.h"
#include "V2R1General.h"
#include "Cdp_pub.h"
#include "onuBwBasedMac.h"

ULONG onuBwDebug = 0;
#define ONU_BW_DEBUG(x) if(onuBwDebug&1) { sys_console_printf x ; }

ULONG onuBwSemId = 0;
ULONG g_onuBwListMaxLen = 4096;			/* 链表最大长度 */
ULONG g_onuUpBwListCurLen = 0;				/* 当前链表长度 */
onu_up_bw_list_t *gp_onuUpBwList = NULL;		/* 上行带宽数据列表头指针 */
onu_up_bw_list_t *gp_onuUpBwListLast = NULL;	/* 上行带宽数据列表尾指针 */


#define ONU_BW_MAC_SEM_TAKE		{\
	if( VOS_SemTake(onuBwSemId, 5000) == VOS_ERROR )\
	{	if( onuBwDebug&2 ) sys_console_printf( "bw T_E:%s %d\r\n", __FILE__, __LINE__);} }
	/*else\
	{	if( onuBwDebug==6 ) sys_console_printf( "bw T_O:%s %d\r\n", __FILE__, __LINE__); }  }*/
	
#define ONU_BW_MAC_SEM_GIVE		{\
	if( VOS_SemGive(onuBwSemId) == VOS_ERROR )\
	{	if( onuBwDebug&2 ) sys_console_printf( "bw bw G_E:%s %d\r\n", __FILE__, __LINE__);} }
	/*else\
	{	if( onuBwDebug==6 ) sys_console_printf( "bw G_O:%s %d\r\n", __FILE__, __LINE__); }  }*/

extern LONG GetMacAddr( CHAR * szStr, CHAR * pucMacAddr );
extern CHAR *macAddress_To_Strings(UCHAR *pMacAddr);
#ifdef PLATO_DBA_V3
extern int  SetOnuUplinkBW_2(short int PonPortIdx, short int OnuIdx, unsigned int UplinkClass, unsigned int Uplinkdelay, unsigned int fixed_bw, unsigned int assured_bw, unsigned int best_effort_bw );
#else
extern int  SetOnuUplinkBW_1(short int PonPortIdx, short int OnuIdx, unsigned int UplinkClass, unsigned int Uplinkdelay, unsigned int assured_bw, unsigned int best_effort_bw );
#endif
extern int  SetOnuDownlinkBW_1(short int PonPortIdx, short int OnuIdx, unsigned int DownlinkClass, unsigned int DownlinkDelay, unsigned int assured_bw, unsigned int best_effort_bw );
extern LONG (*assign_onuBwBasedMac_hookrtn)(SHORT onuMgtIdx, UCHAR * pMacAddr);

LONG assignOnuBwBasedMacRegisterCallback( SHORT onuMgtIdx, UCHAR *pMacAddr );
LONG onu_bw_cli_cmd_install();

int init_onuBwBasedMac()
{
	if( onuBwSemId == 0 )
		onuBwSemId = VOS_SemMCreate(VOS_SEM_Q_FIFO);

	assign_onuBwBasedMac_hookrtn = assignOnuBwBasedMacRegisterCallback;
	
	onu_bw_cli_cmd_install();
	
	return VOS_OK;
}

static onu_up_bw_list_t *onu_up_bw_list_search(UCHAR *pOnuMacAddr)
{
	onu_up_bw_list_t *pList;
	if( pOnuMacAddr == NULL )
		return NULL;

	pList = gp_onuUpBwList;
	while( pList )
	{
		if( MAC_ADDR_IS_EQUAL(pList->node.macAddr, pOnuMacAddr) )
		{
			if( (pList->node.rowStatus == 0) || (pList->node.rowStatus > RS_NOTREADY) )
			{
				pList->node.rowStatus = RS_NOTREADY;
			}
			break;
		}
		pList = pList->next;
	}
	
	return pList;
}

/* 创建一个空节点 */
#ifdef _DISTRIBUTE_PLATFORM_
extern int cl_vty_all_out( const char * format, ... );
extern int cl_install_module( struct cl_cmd_module * m );
#endif
static onu_up_bw_list_t * onu_up_bw_list_new( UCHAR *pOnuMacAddr )
{
	onu_up_bw_list_t *pList = NULL;
	if( pOnuMacAddr == NULL )
		return pList;
	
	pList = onu_up_bw_list_search( pOnuMacAddr );
	if( pList == NULL )
	{
		if( g_onuUpBwListCurLen >= g_onuBwListMaxLen )
		{
#ifdef _DISTRIBUTE_PLATFORM_
			cl_vty_all_out("\r\n up-bandwidth list is full\r\n");
#else
			sys_console_printf("\r\n up-bandwidth list is full\r\n");
#endif
			return pList;
		}
		pList = VOS_Malloc( sizeof(onu_up_bw_list_t), MODULE_ONU );
		if( pList )
		{
			VOS_MemZero( &(pList->node), sizeof(onu_up_bw_node_t) );
			VOS_MemCpy( pList->node.macAddr, pOnuMacAddr, 6 );
			pList->next = NULL;
			pList->node.rowStatus = RS_NOTREADY;

			if( gp_onuUpBwList == NULL )
			{
				gp_onuUpBwList = pList;
			}
			else
			{
				gp_onuUpBwListLast->next = pList;
			}
			gp_onuUpBwListLast = pList;

			g_onuUpBwListCurLen++;
		}
	}
	
	return pList;
}

/* 删除一个节点 */
static LONG onu_up_bw_list_free( UCHAR *pOnuMacAddr )
{
	LONG rc = VOS_ERROR;
	onu_up_bw_list_t *pList, *pPreList;
	if( pOnuMacAddr == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	
	pList = gp_onuUpBwList;
	pPreList = NULL;
	while( pList )
	{
		if( MAC_ADDR_IS_EQUAL(pList->node.macAddr, pOnuMacAddr) )
		{
			if( pPreList == NULL )
			{
				if( pList == gp_onuUpBwListLast )
					gp_onuUpBwListLast = gp_onuUpBwList->next;
				gp_onuUpBwList = gp_onuUpBwList->next;
				VOS_Free( pList );
			}
			else
			{
				if( pList == gp_onuUpBwListLast )
					gp_onuUpBwListLast = pPreList;
				pPreList->next = pList->next;
				VOS_Free( pList );
			}

			if( g_onuUpBwListCurLen != 0 ) g_onuUpBwListCurLen--;
			
			rc = VOS_OK;
			break;
		}
		pPreList = pList;
		pList = pList->next;
	}
	
	ONU_BW_MAC_SEM_GIVE;

	return rc;
}

/* 插入一个节点 */
static LONG onu_up_bw_list_insert( onu_up_bw_node_t *pOnuBwNode )
{
	LONG rc = VOS_OK;
	onu_up_bw_list_t *pList;
	if( pOnuBwNode == NULL )
		return VOS_ERROR;
	
	ONU_BW_MAC_SEM_TAKE;

	pList = onu_up_bw_list_search( pOnuBwNode->macAddr );
	if( pList )
	{
		VOS_MemCpy( &(pList->node), pOnuBwNode, sizeof(onu_up_bw_node_t) );
		pList->node.rowStatus = RS_ACTIVE;
	}
	else
	{
		if( g_onuUpBwListCurLen >= g_onuBwListMaxLen )
		{
			ONU_BW_MAC_SEM_GIVE;
#ifdef _DISTRIBUTE_PLATFORM_
			cl_vty_all_out("\r\n up-bandwidth list is full\r\n");
#else
			sys_console_printf("\r\n up-bandwidth list is full\r\n");
#endif
			return VOS_ERROR;
		}

		pList = VOS_Malloc( sizeof(onu_up_bw_list_t), MODULE_ONU );
		if( pList == NULL )
		{
			rc = VOS_ERROR;
		}
		else
		{
			VOS_MemCpy( &(pList->node), pOnuBwNode, sizeof(onu_up_bw_node_t) );
			pList->next = NULL;
			pList->node.rowStatus = RS_ACTIVE;

			if( (gp_onuUpBwList == NULL) || (gp_onuUpBwListLast == NULL) )
			{
				gp_onuUpBwList = pList;
			}
			else
			{
				gp_onuUpBwListLast->next = pList;
			}
			gp_onuUpBwListLast = pList;

			g_onuUpBwListCurLen++;
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

ULONG g_onuDownBwListCurLen = 0;				/* 当前链表长度 */
onu_down_bw_list_t *gp_onuDownBwList = NULL;		/* 下行带宽数据列表头指针 */
onu_down_bw_list_t *gp_onuDownBwListLast = NULL;	/* 下行带宽数据列表尾指针 */

static onu_down_bw_list_t *onu_down_bw_list_search(UCHAR *pOnuMacAddr)
{
	onu_down_bw_list_t *pList;
	if( pOnuMacAddr == NULL )
		return NULL;

	/*VOS_SemTake( onuBwSemId, WAIT_FOREVER );*/
	pList = gp_onuDownBwList;
	while( pList )
	{
		if( MAC_ADDR_IS_EQUAL(pList->node.macAddr, pOnuMacAddr) )
		{
			break;
		}
		pList = pList->next;
	}
	/*VOS_SemGive( onuBwSemId );*/
	
	return pList;
}

static onu_down_bw_list_t * onu_down_bw_list_new( UCHAR *pOnuMacAddr )
{
	onu_down_bw_list_t *pList = NULL;
	if( pOnuMacAddr == NULL )
		return pList;
	
	pList = onu_down_bw_list_search( pOnuMacAddr );
	if( pList == NULL )
	{
		if( g_onuDownBwListCurLen >= g_onuBwListMaxLen )
		{
#ifdef _DISTRIBUTE_PLATFORM_
			cl_vty_all_out("\r\n down-bandwidth list is full\r\n");
#else
			sys_console_printf("\r\n down-bandwidth list is full\r\n");
#endif
			return pList;
		}

		pList = VOS_Malloc( sizeof(onu_down_bw_list_t), MODULE_ONU );
		if( pList )
		{
			VOS_MemZero( &(pList->node), sizeof(onu_down_bw_node_t) );
			VOS_MemCpy( pList->node.macAddr, pOnuMacAddr, 6 );
			pList->next = NULL;
			pList->node.rowStatus = RS_NOTREADY;

			if( gp_onuDownBwList == NULL )
			{
				gp_onuDownBwList = pList;
			}
			else
			{
				gp_onuDownBwListLast->next = pList;
			}
			gp_onuDownBwListLast = pList;

			g_onuDownBwListCurLen++;
		}
	}
	
	return pList;
}

static LONG onu_down_bw_list_free( UCHAR *pOnuMacAddr )
{
	LONG rc = VOS_ERROR;
	onu_down_bw_list_t *pList, *pPreList;
	if( pOnuMacAddr == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	
	pList = gp_onuDownBwList;
	pPreList = NULL;
	while( pList )
	{
		if( MAC_ADDR_IS_EQUAL(pList->node.macAddr, pOnuMacAddr) )
		{
			if( pPreList == NULL )
			{
				if( pList == gp_onuDownBwListLast )
					gp_onuDownBwListLast = gp_onuDownBwList->next;
				gp_onuDownBwList = gp_onuDownBwList->next;
				VOS_Free( pList );
			}
			else
			{
				if( pList == gp_onuDownBwListLast )
					gp_onuDownBwListLast = pPreList;
				pPreList->next = pList->next;
				VOS_Free( pList );
			}
			if( g_onuDownBwListCurLen != 0 ) g_onuDownBwListCurLen--;
			
			rc = VOS_OK;
			break;
		}
		pPreList = pList;
		pList = pList->next;
	}
	
	ONU_BW_MAC_SEM_GIVE;

	return rc;
}

static LONG onu_down_bw_list_insert( onu_down_bw_node_t *pOnuBwNode )
{
	LONG rc = VOS_OK;
	onu_down_bw_list_t *pList;
	if( pOnuBwNode == NULL )
		return VOS_ERROR;
	
	ONU_BW_MAC_SEM_TAKE;

	pList = onu_down_bw_list_search( pOnuBwNode->macAddr );
	if( pList )
	{
		VOS_MemCpy( &(pList->node), pOnuBwNode, sizeof(onu_down_bw_node_t) );
		pList->node.rowStatus = RS_ACTIVE;
	}
	else
	{
		if( g_onuDownBwListCurLen >= g_onuBwListMaxLen )
		{
			ONU_BW_MAC_SEM_GIVE;
#ifdef _DISTRIBUTE_PLATFORM_
			cl_vty_all_out("\r\n down-bandwidth list is full\r\n");
#else
			sys_console_printf("\r\n down-bandwidth list is full\r\n");
#endif
			return VOS_ERROR;
		}

		pList = VOS_Malloc( sizeof(onu_down_bw_list_t), MODULE_ONU );
		if( pList == NULL )
		{
			rc = VOS_ERROR;
		}
		else
		{
			VOS_MemCpy( &(pList->node), pOnuBwNode, sizeof(onu_down_bw_node_t) );
			pList->next = NULL;
			pList->node.rowStatus = RS_ACTIVE;

			if( (gp_onuDownBwList == NULL) || (gp_onuDownBwListLast == NULL) )
			{
				gp_onuDownBwList = pList;
			}
			else
			{
				gp_onuDownBwListLast->next = pList;
			}
			gp_onuDownBwListLast = pList;

			g_onuDownBwListCurLen++;
		}
	}

	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

/* modified by xieshl 20120704, 重启6900 PON板，删除ONU后再注册，基于mac地址的带宽配置将不起作用 */
LONG onu_bw_list_dispatch( ULONG operCode, onu_up_bw_node_t *pNode )
{
	LONG rc = VOS_ERROR;
	LONG nodeLen = 0;

	if( pNode == NULL )
		return nodeLen;
	
	switch( operCode )
	{
		case ONU_BW_CODE_UP_INSERT:
			rc = onu_up_bw_list_insert( pNode );
			nodeLen = sizeof(onu_up_bw_node_t);
			break;
			
		case ONU_BW_CODE_UP_DELETE:
			rc = onu_up_bw_list_free( pNode->macAddr );
			nodeLen = sizeof(onu_up_bw_node_t);
			break;
			
		case ONU_BW_CODE_DOWN_INSERT:
			rc = onu_down_bw_list_insert( (onu_down_bw_node_t *)pNode );
			nodeLen = sizeof(onu_down_bw_node_t);
			break;
			
		case ONU_BW_CODE_DOWN_DELETE:
			rc = onu_down_bw_list_free( pNode->macAddr );
			nodeLen = sizeof(onu_down_bw_node_t);
			break;

		/*case ONU_BW_CODE_UP_SYNC:
			if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
				rc = VOS_OK;
			else
				rc = onu_up_bw_list_insert( pNode );
			nodeLen = sizeof(onu_up_bw_node_t);
			break;
		case ONU_BW_CODE_DOWN_SYNC:
			if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
				rc = VOS_OK;
			else
				rc = onu_down_bw_list_insert( pNode );
			nodeLen = sizeof(onu_down_bw_node_t);
			break;
			*/
		default:
			break;
	}
		
	if( rc == VOS_ERROR )
		nodeLen = 0;

	return nodeLen;
}

LONG onu_bw_list_sync_send( ULONG slotno, ULONG operCode, VOID *pData, ULONG dataLen )
{
	LONG rc;
	onu_sync_bw_msg_t  * pMsg = NULL;
	LONG msglen=sizeof(onu_sync_bw_msg_t);

	pMsg = ( onu_sync_bw_msg_t * ) CDP_AllocMsg( msglen, MODULE_ONU );              
	if(pMsg == NULL)
	{
		VOS_ASSERT( 0 );
		return VOS_ERROR;
	}
		
	pMsg->OnuSyncMsgHead.onuEventId = ONU_EVENT_BW_BASED_MAC;
	pMsg->OnuSyncMsgHead.ponSlotIdx = slotno;
	pMsg->OnuSyncMsgHead.portIdx = 0;
	pMsg->OnuSyncMsgHead.onuIdx = 0;
	pMsg->operCode = operCode;
	VOS_MemCpy( (VOID*)&(pMsg->node), pData, dataLen );

	rc = CDP_Send( RPU_TID_CDP_ONU, slotno,  RPU_TID_CDP_ONU,  CDP_MSG_TM_ASYNC, pMsg, msglen, MODULE_ONU );
	if( rc !=  VOS_OK )
	{
		CDP_FreeMsg( (VOID *) pMsg );
	}
	return rc;
}
int OnuEvent_GetUpLinkBandWidthFlagByMac(char *pMacAddr)
{
    int ret = VOS_ERROR;
	onu_up_bw_list_t *pUpBwList = onu_up_bw_list_search( pMacAddr );
	if( pUpBwList )
	{
		if( pUpBwList->node.rowStatus == RS_ACTIVE )
            ret = VOS_OK;
	}
    return ret;
}
int OnuEvent_GetUpLinkBandWidthByMac(char *pMacAddr, ONU_bw_t* bw)
{
    int ret = VOS_ERROR;
	onu_up_bw_list_t *pUpBwList = onu_up_bw_list_search( pMacAddr );
	if( pUpBwList )
	{
		if( pUpBwList->node.rowStatus == RS_ACTIVE )
		{
            ret = VOS_OK;
            bw->bw_gr = pUpBwList->node.assuredBw;
            bw->bw_be = pUpBwList->node.bestEffortBw;
            bw->bw_class = pUpBwList->node.bwClass;
            bw->bw_delay = pUpBwList->node.bwDelay;
            bw->bw_fixed = pUpBwList->node.fixedBw;
            bw->bw_direction = OLT_CFG_DIR_UPLINK;            
		}
	}
    return ret;
}
int OnuEvent_GetDownLinkBandWidthByMac(char *pMacAddr, ONU_bw_t* bw)
{
    int ret = VOS_ERROR;
	onu_down_bw_list_t *pDownBwList = onu_down_bw_list_search( pMacAddr );
	if( pDownBwList )
	{
		if( pDownBwList->node.rowStatus == RS_ACTIVE )
		{
            ret = VOS_OK;
            bw->bw_gr = pDownBwList->node.assuredBw;
            bw->bw_be = pDownBwList->node.bestEffortBw;
            bw->bw_class = pDownBwList->node.bwClass;
            bw->bw_delay = pDownBwList->node.bwDelay;
            bw->bw_direction = OLT_CFG_DIR_DOWNLINK;                        
		}
	}
    return ret;
}
int OnuEvent_GetDownLinkBandWidthFlagByMac(char *pMacAddr)
{
    int ret = VOS_ERROR;
	onu_down_bw_list_t *pDownBwList = onu_down_bw_list_search( pMacAddr );
	if( pDownBwList )
	{
		if( pDownBwList->node.rowStatus == RS_ACTIVE )
            ret = VOS_OK;
	}
    return ret;
}
int OnuEvent_UpdateOnuBandWidthByMac(short int PonPortIdx, short int OnuIdx, ONU_bw_t *BW )
{
    int onuMgtIdx = PonPortIdx*MAXONUPERPON+OnuIdx;
    char mac_addr[6];
    onu_up_bw_node_t pNode;    
    LONG nodeLen = 0;
    VOS_MemZero(&pNode, sizeof(onu_up_bw_node_t));
    ONU_MGMT_SEM_TAKE;
    VOS_MemCpy(pNode.macAddr, OnuMgmtTable[onuMgtIdx].DeviceInfo.MacAddr, 6);
    ONU_MGMT_SEM_GIVE;
    
    if((BW->bw_direction & OLT_CFG_DIR_BOTH) == OLT_CFG_DIR_BOTH)
    {
        if(BW->bw_direction & OLT_CFG_DIR_UNDO)
        {
            onu_up_bw_list_t *pUpBwList = onu_up_bw_list_search( pNode.macAddr );
        	onu_down_bw_list_t *pDownBwList = onu_down_bw_list_search( pNode.macAddr );
            
        	if( pUpBwList )
        	{
        		if( pUpBwList->node.rowStatus == RS_ACTIVE )
        		{
                	if( (nodeLen = onu_bw_list_dispatch(ONU_BW_CODE_UP_DELETE, &pNode)) == 0 )
                	{
                		VOS_ASSERT(0);
                		return VOS_ERROR;
                	}
        		}
        	}
        	if( pDownBwList )
        	{
        		if( pDownBwList->node.rowStatus == RS_ACTIVE )
        		{
                	if( (nodeLen = onu_bw_list_dispatch(ONU_BW_CODE_DOWN_DELETE, &pNode)) == 0 )
                	{
                		VOS_ASSERT(0);
                		return VOS_ERROR;
                	}
                 
        		}
        	}
        }
        else
        {
            /*不支持*/
        }
    }
    else
    {
        if(BW->bw_direction & OLT_CFG_DIR_UPLINK)
        {    
            onu_up_bw_list_t *pUpBwList = onu_up_bw_list_search( pNode.macAddr );
            
        	if( pUpBwList )
        	{
        		if( pUpBwList->node.rowStatus == RS_ACTIVE )
        		{
                    if(BW->bw_direction & OLT_CFG_DIR_UNDO)
                    {
                    	if( (nodeLen = onu_bw_list_dispatch(ONU_BW_CODE_UP_DELETE, &pNode)) == 0 )
                    	{
                    		VOS_ASSERT(0);
                    		return VOS_ERROR;
                    	}
                    }
                    else
                    {
                    	pNode.fixedBw = BW->bw_fixed;
                    	pNode.assuredBw = BW->bw_gr;
                    	pNode.bestEffortBw = BW->bw_be;
                    	pNode.bwClass = BW->bw_class;
                    	pNode.bwDelay = BW->bw_delay;
                    	if( (nodeLen = onu_bw_list_dispatch(ONU_BW_CODE_UP_INSERT, &pNode)) == 0 )
                    	{
                    		VOS_ASSERT(0);
                    		return VOS_ERROR;
                    	}
                    }
        		}
        	}
        }
        if(BW->bw_direction & OLT_CFG_DIR_DOWNLINK)
        {   
        	onu_down_bw_list_t *pDownBwList = onu_down_bw_list_search( pNode.macAddr );
        	if( pDownBwList )
        	{
        		if( pDownBwList->node.rowStatus == RS_ACTIVE )
        		{
                    if(BW->bw_direction & OLT_CFG_DIR_UNDO)
                    {
                    	if( (nodeLen = onu_bw_list_dispatch(ONU_BW_CODE_DOWN_DELETE, &pNode)) == 0 )
                    	{
                    		VOS_ASSERT(0);
                    		return VOS_ERROR;
                    	}
                    }
                    else
                    {
                    	pNode.assuredBw = BW->bw_gr;
                    	pNode.bestEffortBw = BW->bw_be;
                    	pNode.bwClass = BW->bw_class;
                    	pNode.bwDelay = BW->bw_delay;
                    	if( (nodeLen = onu_bw_list_dispatch(ONU_BW_CODE_DOWN_INSERT, &pNode)) == 0 )
                    	{
                    		VOS_ASSERT(0);
                    		return VOS_ERROR;
                    	}
                    }
        		}
        	}
    	}    
    }
    return VOS_OK;
}
int OnuEvent_GetDirectionByMacCode(ULONG operCode)
{
    int direction = 0;
    if(operCode == ONU_BW_CODE_UP_INSERT)
    {
        direction = OLT_CFG_DIR_UPLINK;
	}
    
    if(operCode == ONU_BW_CODE_DOWN_INSERT)
	{
        direction = OLT_CFG_DIR_DOWNLINK;
            return( RERROR );
	}

    if(operCode == ONU_BW_CODE_UP_DELETE)
    {
        direction = OLT_CFG_DIR_UPLINK | OLT_CFG_DIR_UNDO;
    }
    
    if(operCode == ONU_BW_CODE_DOWN_DELETE)
    {
        direction = OLT_CFG_DIR_DOWNLINK | OLT_CFG_DIR_UNDO;
    }
    
    return direction;
}
int OnuEvent_UpdateOnuTableBandWidth(short int PonPortIdx, short int OnuIdx, ULONG operCode, onu_up_bw_node_t *pNode )
{
    int onuMgtIdx = PonPortIdx*MAXONUPERPON+OnuIdx;
    int direction = 0;
    UpdateProvisionedBWInfo( PonPortIdx );
	if(pNode->bwClass > PRECEDENCE_OF_FLOW_7)
        return( RERROR );
	if((pNode->bwDelay != V2R1_DELAY_HIGH ) && ( pNode->bwDelay != V2R1_DELAY_LOW )) 
        return( RERROR );

    if(operCode == ONU_BW_CODE_UP_INSERT)
    {
        direction = OLT_CFG_DIR_UPLINK;
        onuMgtIdx = PonPortIdx * MAXONUPERPON + OnuIdx;
    	if((pNode->fixedBw + pNode->assuredBw) > ( PonPortTable[PonPortIdx].RemainBW + OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_fixed + OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_gr) )
    	{
    		return ( V2R1_EXCEED_RANGE );
    	}        
    	if((pNode->fixedBw + pNode->assuredBw) > pNode->bestEffortBw) 
            return( RERROR );
        
        ONU_MGMT_SEM_TAKE;
		/*if(OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus != LLID_ENTRY_ACTIVE )*/
		OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_ACTIVE;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_fixed = pNode->fixedBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_gr = pNode->assuredBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_be = pNode->bestEffortBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkClass = pNode->bwClass;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkDelay = pNode->bwDelay;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].BandWidthIsDefault &= ~OLT_CFG_DIR_UPLINK; /* added by liwei056@2011-12-19 for D14193 */
		ONU_MGMT_SEM_GIVE;
	}
    
    if(operCode == ONU_BW_CODE_DOWN_INSERT)
	{
        direction = OLT_CFG_DIR_DOWNLINK;
        
		if( pNode->assuredBw > PonPortTable[PonPortIdx].DownlinkRemainBw + OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_gr )
		{
			/* send trap to NMS */
			return ( V2R1_EXCEED_RANGE );
		}
        if(pNode->assuredBw > pNode->bestEffortBw) 
            return( RERROR );
        
		ONU_MGMT_SEM_TAKE;
		/*if(OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus != LLID_ENTRY_ACTIVE )*/
		OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_ACTIVE;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_gr = pNode->assuredBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_be = pNode->bestEffortBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkClass = pNode->bwClass;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkDelay = pNode->bwDelay;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].BandWidthIsDefault &= ~OLT_CFG_DIR_DOWNLINK; /* added by liwei056@2011-12-19 for D14193 */
		ONU_MGMT_SEM_GIVE;
	}

    if(operCode == ONU_BW_CODE_UP_DELETE)
    {
        direction = OLT_CFG_DIR_UPLINK | OLT_CFG_DIR_UNDO;
        
        ONU_MGMT_SEM_TAKE;
		/*if(OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus != LLID_ENTRY_ACTIVE )*/
		OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_fixed = 0;
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_UPLINK, 
				&OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_gr, &OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_be);
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkClass = OnuConfigDefault.UplinkClass;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkDelay = OnuConfigDefault.UplinkDelay;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].BandWidthIsDefault |= OLT_CFG_DIR_UPLINK; /* added by liwei056@2011-12-19 for D14193 */
		ONU_MGMT_SEM_GIVE;
    }

    if(operCode == ONU_BW_CODE_DOWN_DELETE)
    {
        direction = OLT_CFG_DIR_DOWNLINK | OLT_CFG_DIR_UNDO;
        
		ONU_MGMT_SEM_TAKE;
		/*if(OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus != LLID_ENTRY_ACTIVE )*/
		OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_NOT_IN_ACTIVE;
		GetOnuDefaultBWByPonRate(PonPortIdx, OnuIdx, OLT_CFG_DIR_DOWNLINK, 
				&OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_gr, &OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_be);
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkClass = OnuConfigDefault.DownlinkClass;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkDelay = OnuConfigDefault.DownlinkDelay;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].BandWidthIsDefault |= OLT_CFG_DIR_DOWNLINK; /* added by liwei056@2011-12-19 for D14193 */
		ONU_MGMT_SEM_GIVE;
    }
    return direction;
}
/* 带宽配置和同步 */
LONG OnuMgtSync_OnuBwBasedMac( ULONG operCode, onu_up_bw_node_t *pNode )
{
	LONG rc = VOS_OK;
	LONG nodeLen;
	ULONG slotno;
    short int OnuIdx, PonPortIdx;
    int onuEntryBase, OnuEntry;
	if( (nodeLen = onu_bw_list_dispatch(operCode, pNode)) == 0 )
	{
	/*deleted by liyang @2015-05-18 for first to set "undo onu BW base MAC " error*/
	/*	VOS_ASSERT(0);*/
		return VOS_ERROR;
	}
	
	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		for( slotno=1; slotno<=SYS_CHASSIS_SWITCH_SLOTNUM; slotno++ )
		{
			if( !SYS_MODULE_IS_CPU_PON(slotno) )
				continue;

			onu_bw_list_sync_send( slotno, operCode, pNode, nodeLen );
		}
	}
#if 0    
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
	    for( PonPortIdx=0; PonPortIdx < MAXPON; PonPortIdx++ )
	    {
	        onuEntryBase = PonPortIdx * MAXONUPERPON;
	        for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
	        {
	            OnuEntry = onuEntryBase+OnuIdx;
	            if( MAC_ADDR_IS_EQUAL(OnuMgmtTable[OnuEntry].DeviceInfo.MacAddr, pNode->macAddr) )
	            {
	                int direction = OnuEvent_GetDirectionByMacCode(operCode);
	                if(direction)
	                {
	                    if(GetOnuOperStatus(PonPortIdx, OnuIdx) == ONU_OPER_STATUS_UP)
	                        OnuEvent_SetBandWidthMsg_Send(PonPortIdx, OnuIdx, direction);    			
	                }
	            }
	        }
	    }
    }
#endif    
    
	return rc;
}

LONG OnuMgtSync_OnuBwBasedMacProc( VOID *pmsg )
{
	onu_sync_bw_msg_t *pMsg = (onu_sync_bw_msg_t *)pmsg;
	if( pMsg == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	return OnuMgtSync_OnuBwBasedMac( pMsg->operCode, &(pMsg->node) );
}

/*LONG restoreOnuBwBasedMacCallback( short int PonPortIdx, short int OnuIdx, UCHAR *pMacAddr )
{
	return VOS_OK;
}*/

/* ONU注册时带宽分配回调函数，如果该ONU 的MAC地址配置了带宽，则以该数据设置PON芯片，
    并重新修改基于ONU ID 的配置数据；否则，则以基于ONU ID 的带宽(LLID管理表) 数据设置PON芯片 */
LONG assignOnuBwBasedMacRegisterCallback(SHORT onuMgtIdx, UCHAR *pMacAddr)
{
	onu_up_bw_list_t *pUpBwList = NULL;
	onu_down_bw_list_t *pDownBwList = NULL;
	ULONG upClass = 0, upDelay = 0, upFixedBw = 0, upAssBw = 0, upBeBw = 0;
	ULONG downClass = 0, downDelay = 0, downAssBw = 0, downBeBw = 0;
	ULONG upBwStatus = 0, downBwStatus = 0;

	if( onuMgtIdx < 0 || onuMgtIdx > MAXONU )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( pMacAddr == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( MAC_ADDR_IS_INVALID(pMacAddr) )
		return VOS_ERROR;

	ONU_BW_MAC_SEM_TAKE;

	pUpBwList = onu_up_bw_list_search( pMacAddr );
	if( pUpBwList )
	{
		if( pUpBwList->node.rowStatus == RS_ACTIVE )
		{
			upBwStatus = 1;
			upClass = pUpBwList->node.bwClass;
			upDelay = pUpBwList->node.bwDelay;
			upFixedBw = pUpBwList->node.fixedBw;
			upAssBw = pUpBwList->node.assuredBw;
			upBeBw = pUpBwList->node.bestEffortBw;
		}
	}
	pDownBwList = onu_down_bw_list_search( pMacAddr );
	if( pDownBwList )
	{
		if( pDownBwList->node.rowStatus == RS_ACTIVE )
		{
			downBwStatus = 1;
			downClass = pDownBwList->node.bwClass;
			downDelay = pDownBwList->node.bwDelay;
			downAssBw = pDownBwList->node.assuredBw;
			downBeBw = pDownBwList->node.bestEffortBw;
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	if( upBwStatus )
	{
		ONU_MGMT_SEM_TAKE;
		/*if(OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus != LLID_ENTRY_ACTIVE )*/
		OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_ACTIVE;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_fixed = upFixedBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_gr = upAssBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkBandwidth_be = upBeBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkClass = upClass;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].UplinkDelay = upDelay;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].BandWidthIsDefault &= ~OLT_CFG_DIR_UPLINK; /* added by liwei056@2011-12-19 for D14193 */
		ONU_MGMT_SEM_GIVE;
		ONU_BW_DEBUG( ("\r\n restore onu %d/%d/%d up-bandwidth based mac addr.\r\n", GetCardIdxByPonChip(onuMgtIdx/MAXONUPERPON), GetPonPortByPonChip(onuMgtIdx/MAXONUPERPON), onuMgtIdx%MAXONUPERPON+1) );
	}

	if( downBwStatus )
	{
		ONU_MGMT_SEM_TAKE;
		/*if(OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus != LLID_ENTRY_ACTIVE )*/
		OnuMgmtTable[onuMgtIdx].LlidTable[0].EntryStatus = LLID_ENTRY_ACTIVE;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_gr = downAssBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkBandwidth_be = downBeBw;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkClass = downClass;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].DownlinkDelay = downDelay;
		OnuMgmtTable[onuMgtIdx].LlidTable[0].BandWidthIsDefault &= ~OLT_CFG_DIR_DOWNLINK; /* added by liwei056@2011-12-19 for D14193 */
		ONU_MGMT_SEM_GIVE;
		ONU_BW_DEBUG( ("\r\n restore onu %d/%d/%d down-bandwidth based mac addr.\r\n", GetCardIdxByPonChip(onuMgtIdx/MAXONUPERPON), GetPonPortByPonChip(onuMgtIdx/MAXONUPERPON), onuMgtIdx%MAXONUPERPON+1) );
	}

	return VOS_OK;
}

/* added by xieshl 20120704, 基于MAC地址的带宽分配支持PON板热拔插，解决删除ONU或者PON板重启时，
    会导致PON板上配置丢失问题，问题单14205 */
LONG restoreOnuBwBasedMacHotCallBack( ULONG slotno )
{
	onu_up_bw_node_t upBwNode;
	onu_up_bw_node_t *pUpBwNode = &upBwNode;
	onu_down_bw_node_t *pDownBwNode = (onu_down_bw_node_t *)&upBwNode;
	UCHAR onuMacAddr[6];

	if( !SYS_MODULE_IS_CPU_PON(slotno) )
		return VOS_OK;

	if ( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		VOS_MemZero( onuMacAddr, sizeof(onuMacAddr) );
		while( getNextOnuUpBwBasedMacNode(onuMacAddr, pUpBwNode) == VOS_OK )
		{
			if( MAC_ADDR_IS_ZERO(pUpBwNode->macAddr) )
			{
				VOS_ASSERT(0);
				break;
			}
			if( pUpBwNode->rowStatus == RS_ACTIVE )
				onu_bw_list_sync_send( slotno, ONU_BW_CODE_UP_INSERT, pUpBwNode, sizeof(onu_up_bw_node_t) );
			VOS_MemCpy( onuMacAddr, pUpBwNode->macAddr, sizeof(onuMacAddr) );
		}

		VOS_MemZero( onuMacAddr, sizeof(onuMacAddr) );
		while( getNextOnuDownBwBasedMacNode(onuMacAddr, pDownBwNode) == VOS_OK )
		{
			if( MAC_ADDR_IS_ZERO(pDownBwNode->macAddr) )
			{
				VOS_ASSERT(0);
				break;
			}
			if( pDownBwNode->rowStatus == RS_ACTIVE )
				onu_bw_list_sync_send( slotno, ONU_BW_CODE_DOWN_INSERT, pDownBwNode, sizeof(onu_down_bw_node_t) );
			VOS_MemCpy( onuMacAddr, pDownBwNode->macAddr, sizeof(onuMacAddr) );
		}
	}
	return VOS_OK;
}

/* SNMP管理API */
#if(RPU_MODULE_SNMP == RPU_YES)

#define mib_onuUBwBasedMacAddr		1
#define mib_onuUBwBasedMacFixed		2
#define mib_onuUBwBasedMacAssured		3
#define mib_onuUBwBasedMacBestEffort	4
#define mib_onuUBwBasedMacClass		5
#define mib_onuUBwBasedMacDelay		6
#define mib_onuUBwBasedMacStatus		7

/* GET上行带宽管理对象值 */
static LONG getOnuUBwMibObjectsValue( ULONG mib, UCHAR *pOnuMacAddr, ULONG *pValue )
{
	LONG rc = VOS_ERROR;
	onu_up_bw_list_t *pList = NULL;
	
	if( (pOnuMacAddr == NULL) || (pValue == NULL) )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	
	pList = onu_up_bw_list_search( pOnuMacAddr );
	if( pList )
	{
		rc = VOS_OK;
		switch( mib )
		{
			case mib_onuUBwBasedMacFixed:
				*pValue = pList->node.fixedBw;
				break;
			case mib_onuUBwBasedMacAssured:
				*pValue = pList->node.assuredBw;
				break;
			case mib_onuUBwBasedMacBestEffort:
				*pValue = pList->node.bestEffortBw;
				break;
			case mib_onuUBwBasedMacClass:
				*pValue = pList->node.bwClass;
				break;
			case mib_onuUBwBasedMacDelay:
				*pValue = pList->node.bwDelay;
				break;
			case mib_onuUBwBasedMacStatus:
				*pValue = pList->node.rowStatus;
				break;
			case mib_onuUBwBasedMacAddr:
			default:
				rc = VOS_ERROR;
				break;
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

/* SET上行带宽管理对象值 */
static LONG setOnuUBwMibObjectsValue( ULONG mib, UCHAR *pOnuMacAddr, ULONG value )
{
	LONG rc = VOS_ERROR;
	onu_up_bw_list_t *pList = NULL;
	
	if( pOnuMacAddr == NULL )
		return rc;

	if( mib == mib_onuUBwBasedMacStatus )
	{
		if( value == RS_CREATEANDWAIT )
		{
			ONU_BW_MAC_SEM_TAKE;
			pList = onu_up_bw_list_new( pOnuMacAddr );
			if( pList )
			{
				if( pList->node.rowStatus == RS_NOTREADY )
				{
					rc = VOS_OK;
				}
			}
			ONU_BW_MAC_SEM_GIVE;
			return rc;
		}
	}
	
	ONU_BW_MAC_SEM_TAKE;
	
	pList = onu_up_bw_list_search( pOnuMacAddr );
	if( pList )
	{
		if( (pList->node.rowStatus == RS_NOTINSERVICE) || (pList->node.rowStatus == RS_NOTREADY) )
		{
			rc = VOS_OK;
		}
		rc = VOS_OK;
		switch( mib )
		{
			case mib_onuUBwBasedMacFixed:
				pList->node.fixedBw = value;
				break;
			case mib_onuUBwBasedMacAssured:
				pList->node.assuredBw = value;
				break;
			case mib_onuUBwBasedMacBestEffort:
				pList->node.bestEffortBw = value;
				break;
			case mib_onuUBwBasedMacClass:
				pList->node.bwClass = value;
				break;
			case mib_onuUBwBasedMacDelay:
				pList->node.bwDelay = value;
				break;

			case mib_onuUBwBasedMacStatus:
				if( value == RS_ACTIVE )
				{
					setOnuUpBwBasedMac( pOnuMacAddr, pList->node.bwClass, pList->node.bwDelay, pList->node.fixedBw, pList->node.assuredBw, pList->node.bestEffortBw );
				}
				else if( value == RS_DESTROY )
				{
					delOnuUpBwBasedMac( pOnuMacAddr );
				}
				else
					rc = VOS_ERROR;
				
				/*pList->node.rowStatus = value;*/
				break;
			case mib_onuUBwBasedMacAddr:
			default:
				rc = VOS_ERROR;
				break;
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}


LONG getOnuUBwBasedMacFixed( UCHAR *pOnuMacAddr, ULONG *pFixedBw )
{
	return getOnuUBwMibObjectsValue(mib_onuUBwBasedMacFixed, pOnuMacAddr, pFixedBw );
}

LONG setOnuUBwBasedMacFixed( UCHAR *pOnuMacAddr, ULONG fixedBw )
{
	return setOnuUBwMibObjectsValue(mib_onuUBwBasedMacFixed, pOnuMacAddr, fixedBw );
}

LONG getOnuUBwBasedMacAssured( UCHAR *pOnuMacAddr, ULONG *pAssuredBw )
{
	return getOnuUBwMibObjectsValue(mib_onuUBwBasedMacAssured, pOnuMacAddr, pAssuredBw );
}

LONG setOnuUBwBasedMacAssured( UCHAR *pOnuMacAddr, ULONG assuredBw )
{
	return setOnuUBwMibObjectsValue(mib_onuUBwBasedMacAssured, pOnuMacAddr, assuredBw );
}

LONG getOnuUBwBasedMacBestEffort( UCHAR *pOnuMacAddr, ULONG *pBeBw )
{
	return getOnuUBwMibObjectsValue(mib_onuUBwBasedMacBestEffort, pOnuMacAddr, pBeBw );
}

LONG setOnuUBwBasedMacBestEffort( UCHAR *pOnuMacAddr, ULONG beBw )
{
	return setOnuUBwMibObjectsValue(mib_onuUBwBasedMacBestEffort, pOnuMacAddr, beBw );
}

LONG getOnuUBwBasedMacClass( UCHAR *pOnuMacAddr, ULONG *pBwClass )
{
	return getOnuUBwMibObjectsValue(mib_onuUBwBasedMacClass, pOnuMacAddr, pBwClass );
}

LONG setOnuUBwBasedMacClass( UCHAR *pOnuMacAddr, ULONG bwClass )
{
	return setOnuUBwMibObjectsValue(mib_onuUBwBasedMacClass, pOnuMacAddr, bwClass );
}

LONG getOnuUBwBasedMacDelay( UCHAR *pOnuMacAddr, ULONG *pBwDelay )
{
	return getOnuUBwMibObjectsValue(mib_onuUBwBasedMacDelay, pOnuMacAddr, pBwDelay );
}

LONG setOnuUBwBasedMacDelay( UCHAR *pOnuMacAddr, ULONG bwDelay )
{
	return setOnuUBwMibObjectsValue(mib_onuUBwBasedMacDelay, pOnuMacAddr, bwDelay );
}

LONG getOnuUBwBasedMacStatus( UCHAR *pOnuMacAddr, ULONG *pRowStatus )
{
	return getOnuUBwMibObjectsValue(mib_onuUBwBasedMacStatus, pOnuMacAddr, pRowStatus );
}

LONG setOnuUBwBasedMacStatus( UCHAR *pOnuMacAddr, ULONG rowStatus )
{
	return setOnuUBwMibObjectsValue(mib_onuUBwBasedMacStatus, pOnuMacAddr, rowStatus );
}

LONG checkOnuUBwBasedMacIdx(UCHAR *pOnuMacAddr )
{
	LONG rc = VOS_ERROR;
	onu_up_bw_list_t *pList = NULL;
	
	if( pOnuMacAddr == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	pList = onu_up_bw_list_search( pOnuMacAddr );
	if( pList )
	{
		rc = VOS_OK;
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

/* 取第一个索引对应的MAC地址 */
LONG getFirstOnuUBwBasedMacIdx( UCHAR *pOnuMacAddr )
{
	LONG rc = VOS_ERROR;
	
	if( pOnuMacAddr == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	if( gp_onuUpBwList )
	{
		VOS_MemCpy( pOnuMacAddr, &(gp_onuUpBwList->node.macAddr), 6 );
		rc = VOS_OK;
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

/* 取下一个MAC地址 */
LONG getNextOnuUBwBasedMacIdx(UCHAR *pOnuMacAddr, UCHAR *pNextOnuMacAddr )
{
	LONG rc = VOS_ERROR;
	onu_up_bw_list_t *pList = NULL;
	
	if( pNextOnuMacAddr == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	if( pOnuMacAddr == NULL )
	{
		if( gp_onuUpBwList )
		{
			VOS_MemCpy( pNextOnuMacAddr, &(gp_onuUpBwList->node.macAddr), 6 );
			rc = VOS_OK;
		}
	}
	else
	{
		pList = onu_up_bw_list_search( pOnuMacAddr );
		if( pList )
		{
			pList = pList->next;
			if( pList )
			{
				VOS_MemCpy( pNextOnuMacAddr, &(pList->node.macAddr), 6 );
				rc = VOS_OK;
			}
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}


/*LONG getOnuDownBwBasedMac(UCHAR *pOnuMacAddr, onu_down_bw_node_t *pNode )
{
	LONG rc = VOS_ERROR;
	onu_down_bw_list_t *pList = NULL;
	
	if( (pOnuMacAddr == NULL) || (pNode == NULL) )
		return rc;

	VOS_SemTake( onuBwSemId, WAIT_FOREVER );
	
	pList = onu_down_bw_list_search( pOnuMacAddr );
	if( pList )
	{
		VOS_MemCpy( pNode, &(pList->node), sizeof(onu_down_bw_node_t) );
		rc = VOS_OK;
	}
	VOS_SemGive( onuBwSemId );
	
	return rc;
}*/

#define mib_onuDBwBasedMacAddr		1
#define mib_onuDBwBasedMacFixed		2
#define mib_onuDBwBasedMacAssured		3
#define mib_onuDBwBasedMacBestEffort	4
#define mib_onuDBwBasedMacClass		5
#define mib_onuDBwBasedMacDelay		6
#define mib_onuDBwBasedMacStatus		7

/* GET下行带宽管理对象值 */
static LONG getOnuDBwMibObjectsValue( ULONG mib, UCHAR *pOnuMacAddr, ULONG *pValue )
{
	LONG rc = VOS_ERROR;
	onu_down_bw_list_t *pList = NULL;
	
	if( (pOnuMacAddr == NULL) || (pValue == NULL) )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	
	pList = onu_down_bw_list_search( pOnuMacAddr );
	if( pList )
	{
		rc = VOS_OK;
		switch( mib )
		{
			case mib_onuDBwBasedMacAssured:
				*pValue = pList->node.assuredBw;
				break;
			case mib_onuDBwBasedMacBestEffort:
				*pValue = pList->node.bestEffortBw;
				break;
			case mib_onuDBwBasedMacClass:
				*pValue = pList->node.bwClass;
				break;
			case mib_onuDBwBasedMacDelay:
				*pValue = pList->node.bwDelay;
				break;
			case mib_onuDBwBasedMacStatus:
				*pValue = pList->node.rowStatus;
				break;
			case mib_onuDBwBasedMacAddr:
			case mib_onuDBwBasedMacFixed:
			default:
				rc = VOS_ERROR;
				break;
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

/* SET下行带宽管理对象值 */
static LONG setOnuDBwMibObjectsValue( ULONG mib, UCHAR *pOnuMacAddr, ULONG value )
{
	LONG rc = VOS_ERROR;
	onu_down_bw_list_t *pList = NULL;
	
	if( pOnuMacAddr == NULL )
		return rc;

	if( mib == mib_onuDBwBasedMacStatus )
	{
		if( value == RS_CREATEANDWAIT )
		{
			ONU_BW_MAC_SEM_TAKE;
			pList = onu_down_bw_list_new( pOnuMacAddr );
			if( pList )
			{
				if( pList->node.rowStatus == RS_NOTREADY )
				{
					rc = VOS_OK;
				}
			}
			ONU_BW_MAC_SEM_GIVE;
			return rc;
		}
	}
	
	ONU_BW_MAC_SEM_TAKE;
	
	pList = onu_down_bw_list_search( pOnuMacAddr );
	if( pList )
	{
		if( (pList->node.rowStatus == RS_NOTINSERVICE) || (pList->node.rowStatus == RS_NOTREADY) )
		{
			rc = VOS_OK;
		}
		rc = VOS_OK;
		switch( mib )
		{
			case mib_onuDBwBasedMacAssured:
				pList->node.assuredBw = value;
				break;
			case mib_onuDBwBasedMacBestEffort:
				pList->node.bestEffortBw = value;
				break;
			case mib_onuDBwBasedMacClass:
				pList->node.bwClass = value;
				break;
			case mib_onuDBwBasedMacDelay:
				pList->node.bwDelay = value;
				break;

			case mib_onuDBwBasedMacStatus:
				if( value == RS_ACTIVE )
				{
					setOnuDownBwBasedMac( pOnuMacAddr, pList->node.bwClass, pList->node.bwDelay, pList->node.assuredBw, pList->node.bestEffortBw );
				}
				else if( value == RS_DESTROY )
				{
					delOnuDownBwBasedMac( pOnuMacAddr );
				}
				else
					rc = VOS_ERROR;
				
				/*pList->node.rowStatus = value;*/
				break;
			case mib_onuDBwBasedMacFixed:
			case mib_onuDBwBasedMacAddr:
			default:
				rc = VOS_ERROR;
				break;
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}


LONG getOnuDBwBasedMacFixed( UCHAR *pOnuMacAddr, ULONG *pFixedBw )
{
	return getOnuDBwMibObjectsValue(mib_onuDBwBasedMacFixed, pOnuMacAddr, pFixedBw );
}

LONG setOnuDBwBasedMacFixed( UCHAR *pOnuMacAddr, ULONG fixedBw )
{
	return setOnuDBwMibObjectsValue(mib_onuDBwBasedMacFixed, pOnuMacAddr, fixedBw );
}

LONG getOnuDBwBasedMacAssured( UCHAR *pOnuMacAddr, ULONG *pAssuredBw )
{
	return getOnuDBwMibObjectsValue(mib_onuDBwBasedMacAssured, pOnuMacAddr, pAssuredBw );
}

LONG setOnuDBwBasedMacAssured( UCHAR *pOnuMacAddr, ULONG assuredBw )
{
	return setOnuDBwMibObjectsValue(mib_onuDBwBasedMacAssured, pOnuMacAddr, assuredBw );
}

LONG getOnuDBwBasedMacBestEffort( UCHAR *pOnuMacAddr, ULONG *pBeBw )
{
	return getOnuDBwMibObjectsValue(mib_onuDBwBasedMacBestEffort, pOnuMacAddr, pBeBw );
}

LONG setOnuDBwBasedMacBestEffort( UCHAR *pOnuMacAddr, ULONG beBw )
{
	return setOnuDBwMibObjectsValue(mib_onuDBwBasedMacBestEffort, pOnuMacAddr, beBw );
}

LONG getOnuDBwBasedMacClass( UCHAR *pOnuMacAddr, ULONG *pBwClass )
{
	return getOnuDBwMibObjectsValue(mib_onuDBwBasedMacClass, pOnuMacAddr, pBwClass );
}

LONG setOnuDBwBasedMacClass( UCHAR *pOnuMacAddr, ULONG bwClass )
{
	return setOnuDBwMibObjectsValue(mib_onuDBwBasedMacClass, pOnuMacAddr, bwClass );
}

LONG getOnuDBwBasedMacDelay( UCHAR *pOnuMacAddr, ULONG *pBwDelay )
{
	return getOnuDBwMibObjectsValue(mib_onuDBwBasedMacDelay, pOnuMacAddr, pBwDelay );
}

LONG setOnuDBwBasedMacDelay( UCHAR *pOnuMacAddr, ULONG bwDelay )
{
	return setOnuDBwMibObjectsValue(mib_onuDBwBasedMacDelay, pOnuMacAddr, bwDelay );
}

LONG getOnuDBwBasedMacStatus( UCHAR *pOnuMacAddr, ULONG *pRowStatus )
{
	return getOnuDBwMibObjectsValue(mib_onuDBwBasedMacStatus, pOnuMacAddr, pRowStatus );
}

LONG setOnuDBwBasedMacStatus( UCHAR *pOnuMacAddr, ULONG rowStatus )
{
	return setOnuDBwMibObjectsValue(mib_onuDBwBasedMacStatus, pOnuMacAddr, rowStatus );
}

LONG checkOnuDBwBasedMacIdx(UCHAR *pOnuMacAddr )
{
	LONG rc = VOS_ERROR;
	onu_down_bw_list_t *pList = NULL;
	
	if( pOnuMacAddr == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	pList = onu_down_bw_list_search( pOnuMacAddr );
	if( pList )
	{
		rc = VOS_OK;
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

LONG getFirstOnuDBwBasedMacIdx( UCHAR *pOnuMacAddr )
{
	LONG rc = VOS_ERROR;
	
	if( pOnuMacAddr == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	if( gp_onuDownBwList )
	{
		VOS_MemCpy( pOnuMacAddr, &(gp_onuDownBwList->node.macAddr), 6 );
		rc = VOS_OK;
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

LONG getNextOnuDBwBasedMacIdx(UCHAR *pOnuMacAddr, UCHAR *pNextOnuMacAddr )
{
	LONG rc = VOS_ERROR;
	onu_down_bw_list_t *pList = NULL;
	
	if( pNextOnuMacAddr == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	if( pOnuMacAddr == NULL )
	{
		if( gp_onuDownBwList )
		{
			VOS_MemCpy( pNextOnuMacAddr, &(gp_onuDownBwList->node.macAddr), 6 );
			rc = VOS_OK;
		}
	}
	else
	{
		pList = onu_down_bw_list_search( pOnuMacAddr );
		if( pList )
		{
			pList = pList->next;
			if( pList )
			{
				VOS_MemCpy( pNextOnuMacAddr, &(pList->node.macAddr), 6 );
				rc = VOS_OK;
			}
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

#endif

/* 从链表中读取第一个节点 */
LONG getFirstOnuUpBwBasedMacNode( onu_up_bw_node_t *pNode )
{
	LONG rc = VOS_ERROR;
	
	if( pNode == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	if( gp_onuUpBwList )
	{
		VOS_MemCpy( pNode, &(gp_onuUpBwList->node), sizeof(onu_up_bw_node_t) );
		rc = VOS_OK;
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

/* 从链表中读取当前mac 地址对应节点的下一个节点 */
LONG getNextOnuUpBwBasedMacNode(UCHAR *pOnuMacAddr, onu_up_bw_node_t *pNextNode )
{
	LONG rc = VOS_ERROR;
	onu_up_bw_list_t *pList = NULL;
	
	if( pNextNode == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	if( (pOnuMacAddr == NULL) || MAC_ADDR_IS_ZERO(pOnuMacAddr) )
	{
		if( gp_onuUpBwList )
		{
			VOS_MemCpy( pNextNode, &(gp_onuUpBwList->node), sizeof(onu_up_bw_node_t) );
			rc = VOS_OK;
		}
	}
	else
	{
		pList = onu_up_bw_list_search( pOnuMacAddr );
		if( pList )
		{
			pList = pList->next;
			if( pList )
			{
				VOS_MemCpy( pNextNode, &(pList->node), sizeof(onu_up_bw_node_t) );
				rc = VOS_OK;
			}
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}


/* 基于ONU MAC 地址的上行带宽配置API */
LONG setOnuUpBwBasedMac( UCHAR *pMacAddr, ULONG bwClass, ULONG bwDelay, ULONG fixedBw, ULONG assuredBw, ULONG bestEffortBw )
{
	short int PonPortIdx, OnuIdx, slotno, port;
	int onuEntryBase, onuEntry;
	/*int onuStatus;*/
	onu_up_bw_node_t upBwNode;

	if( pMacAddr == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( MAC_ADDR_IS_INVALID(pMacAddr) )
		return VOS_ERROR;
    
	ONU_MGMT_SEM_TAKE;
	for( PonPortIdx=0; PonPortIdx < MAXPON; PonPortIdx++ )
	{
		slotno = GetCardIdxByPonChip(PonPortIdx);
		port = GetPonPortByPonChip(PonPortIdx);
		onuEntryBase = PonPortIdx * MAXONUPERPON;
		for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
		{
			onuEntry = onuEntryBase+OnuIdx;
			if( MAC_ADDR_IS_EQUAL(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, pMacAddr) )
			{
				/*onuStatus = OnuMgmtTable[onuEntry].OperStatus;
				if( onuStatus == ONU_OPER_STATUS_UP )*/
				{
					if(SYS_MODULE_IS_RUNNING(slotno) && SYS_MODULE_IS_16EPON(slotno) && fixedBw > 8192)
					{
						sys_console_printf("\r\n ONU(%d/%d/%d) failed  to set DBA, BCM55524 card's fixed-bandwidth should be no greater than 8192kbit/s.\r\n", slotno, port, (OnuIdx + 1));
						continue;
					}
#ifdef PLATO_DBA_V3
					SetOnuUplinkBW_ByMac( PonPortIdx, OnuIdx, bwClass, bwDelay, fixedBw, assuredBw, bestEffortBw );
#else
					SetOnuUplinkBW_1( PonPortIdx, OnuIdx, bwClass, bwDelay, assuredBw, bestEffortBw );
#endif
					ONU_BW_DEBUG( ("\r\n set onu %d/%d/%d up-bandwidth based mac addr.\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1) );
				}
			}
		}
 	}
	ONU_MGMT_SEM_GIVE;
    
	VOS_MemCpy( upBwNode.macAddr, pMacAddr, 6 );
	upBwNode.fixedBw = fixedBw;
	upBwNode.assuredBw = assuredBw;
	upBwNode.bestEffortBw = bestEffortBw;
	upBwNode.bwClass = bwClass;
	upBwNode.bwDelay = bwDelay;
	OnuMgtSync_OnuBwBasedMac( ONU_BW_CODE_UP_INSERT, &upBwNode );

	return VOS_OK;
}

/* 基于ONU MAC 地址的上行带宽删除API */
LONG delOnuUpBwBasedMac( UCHAR *pMacAddr )
{
	short int PonPortIdx, OnuIdx;
	int onuEntryBase, onuEntry;
	bool isExist;
	onu_up_bw_node_t upBwNode;

	unsigned int fixedBw = 0;
	unsigned int assuredBw = 0;
	unsigned int bestEffortBw = 0;
	unsigned int bwClass = 2;
	unsigned int bwDelay = CLIE_BANDWITHDELAY_LOW;


	if( pMacAddr == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( MAC_ADDR_IS_INVALID(pMacAddr) )
		return VOS_ERROR;
#if 1
	for( PonPortIdx=0; PonPortIdx < MAXPON; PonPortIdx++ )
	{
		onuEntryBase = PonPortIdx * MAXONUPERPON;
		for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
		{
			onuEntry = onuEntryBase+OnuIdx;

			ONU_MGMT_SEM_TAKE;
			isExist = MAC_ADDR_IS_EQUAL(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, pMacAddr);
			ONU_MGMT_SEM_GIVE;
			
			if( isExist )
			{
				{
#ifdef PLATO_DBA_V3
					SetOnuUplinkBW_ByMac( PonPortIdx, OnuIdx, bwClass, bwDelay, fixedBw, assuredBw, bestEffortBw );
#else
					SetOnuUplinkBW_1( PonPortIdx, OnuIdx, bwClass, bwDelay, assuredBw, bestEffortBw );
#endif
					ONU_BW_DEBUG( ("\r\n set onu %d/%d/%d up-bandwidth based mac addr.\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1) );
				}
			}
		}
 	}
#endif	
	VOS_MemCpy( upBwNode.macAddr, pMacAddr, 6 );
	upBwNode.fixedBw = fixedBw;
	upBwNode.assuredBw = assuredBw;
	upBwNode.bestEffortBw = bestEffortBw;
	upBwNode.bwClass = bwClass;
	upBwNode.bwDelay = bwDelay;
	OnuMgtSync_OnuBwBasedMac( ONU_BW_CODE_UP_DELETE, &upBwNode );

	return VOS_OK;
}

/* 从链表中读取第一个节点 */
LONG getFirstOnuDownBwBasedMacNode( onu_down_bw_node_t *pNode )
{
	LONG rc = VOS_ERROR;
	
	if( pNode == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	if( gp_onuDownBwList )
	{
		VOS_MemCpy( pNode, &(gp_onuDownBwList->node), sizeof(onu_down_bw_node_t) );
		rc = VOS_OK;
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

/* 从链表中读取当前mac 地址对应节点的下一个节点 */
LONG getNextOnuDownBwBasedMacNode(UCHAR *pOnuMacAddr, onu_down_bw_node_t *pNextNode )
{
	LONG rc = VOS_ERROR;
	onu_down_bw_list_t *pList = NULL;
	
	if( pNextNode == NULL )
		return rc;

	ONU_BW_MAC_SEM_TAKE;
	if( (pOnuMacAddr == NULL) || MAC_ADDR_IS_ZERO(pOnuMacAddr) )
	{
		if( gp_onuDownBwList )
		{
			VOS_MemCpy( pNextNode, &(gp_onuDownBwList->node), sizeof(onu_down_bw_node_t) );
			rc = VOS_OK;
		}
	}
	else
	{
		pList = onu_down_bw_list_search( pOnuMacAddr );
		if( pList )
		{
			pList = pList->next;
			if( pList )
			{
				VOS_MemCpy( pNextNode, &(pList->node), sizeof(onu_down_bw_node_t) );
				rc = VOS_OK;
			}
		}
	}
	ONU_BW_MAC_SEM_GIVE;
	
	return rc;
}

/* 基于ONU MAC 地址的下行带宽配置API */
LONG setOnuDownBwBasedMac( UCHAR *pMacAddr, ULONG bwClass, ULONG bwDelay, ULONG assuredBw, ULONG bestEffortBw )
{
	short int PonPortIdx, OnuIdx;
	int onuEntryBase, onuEntry;
	/*int onuStatus;*/
	onu_down_bw_node_t downBwNode;

	if( pMacAddr == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( MAC_ADDR_IS_INVALID(pMacAddr) )
		return VOS_ERROR;

	ONU_MGMT_SEM_TAKE;
	for( PonPortIdx=0; PonPortIdx < MAXPON; PonPortIdx++ )
	{
		onuEntryBase = PonPortIdx * MAXONUPERPON;
		for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
		{
			onuEntry = onuEntryBase+OnuIdx;
			if( MAC_ADDR_IS_EQUAL(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, pMacAddr) )
			{
				/*onuStatus = OnuMgmtTable[onuEntry].OperStatus;
				if( onuStatus == ONU_OPER_STATUS_UP )*/
				{
					SetOnuDownlinkBW_ByMac( PonPortIdx, OnuIdx, bwClass, bwDelay, assuredBw, bestEffortBw );
					ONU_BW_DEBUG( ("\r\n set onu %d/%d/%d down-bandwidth based mac addr.\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1) );
				}
			}
		}
 	}
	ONU_MGMT_SEM_GIVE;

	VOS_MemCpy( downBwNode.macAddr, pMacAddr, 6 );
	downBwNode.assuredBw = assuredBw;
	downBwNode.bestEffortBw = bestEffortBw;
	downBwNode.bwClass = bwClass;
	downBwNode.bwDelay = bwDelay;
	OnuMgtSync_OnuBwBasedMac( ONU_BW_CODE_DOWN_INSERT, (VOID *)&downBwNode );

	return VOS_OK;
}

/* 基于ONU MAC 地址的下行带宽删除API */
LONG delOnuDownBwBasedMac( UCHAR *pMacAddr )
{
	short int PonPortIdx, OnuIdx;
	int onuEntryBase, onuEntry;
	bool isExist;
	onu_down_bw_node_t downBwNode;

	unsigned int assuredBw = 0;
	unsigned int bestEffortBw = 0;
	unsigned int bwClass = 2;
	unsigned int bwDelay = CLIE_BANDWITHDELAY_LOW;


	if( pMacAddr == NULL )
	{
		VOS_ASSERT(0);
		return VOS_ERROR;
	}
	if( MAC_ADDR_IS_INVALID(pMacAddr) )
		return VOS_ERROR;
#if 1
	for( PonPortIdx=0; PonPortIdx < MAXPON; PonPortIdx++ )
	{
		onuEntryBase = PonPortIdx * MAXONUPERPON;
		for(OnuIdx=0; OnuIdx< MAXONUPERPON; OnuIdx++)
		{
			onuEntry = onuEntryBase+OnuIdx;

			ONU_MGMT_SEM_TAKE;
			isExist = MAC_ADDR_IS_EQUAL(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, pMacAddr);
			ONU_MGMT_SEM_GIVE;
			
			if( isExist )
			{
				/*onuStatus = OnuMgmtTable[onuEntry].OperStatus;
				if( onuStatus == ONU_OPER_STATUS_UP )*/
				{
					SetOnuDownlinkBW_ByMac( PonPortIdx, OnuIdx, bwClass, bwDelay, assuredBw, bestEffortBw );
					ONU_BW_DEBUG( ("\r\n set onu %d/%d/%d down-bandwidth based mac addr.\r\n", GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1) );
				}
			}
		}
 	}
#endif	
	VOS_MemCpy( downBwNode.macAddr, pMacAddr, 6 );
	downBwNode.assuredBw = assuredBw;
	downBwNode.bestEffortBw = bestEffortBw;
	downBwNode.bwClass = bwClass;
	downBwNode.bwDelay = bwDelay;
	OnuMgtSync_OnuBwBasedMac( ONU_BW_CODE_DOWN_DELETE, (VOID *)&downBwNode );

	return VOS_OK;
}


DEFUN( config_onu_up_bw_based_mac,
       config_onu_up_bw_based_mac_cmd,
	"bandwidth-mac up <H.H.H> <0-7> [high|low] <0-1000000> <64-1000000> <64-1000000>",
	"config bandwidth based onu mac addr.\n"
	"up bandwidth based onu mac addr.\n"
	"input onu mac address\n"
	"input bandwidth class\n"
	"bandwidth delay high\n"
	"bandwidth delay low\n"
	"input the fixed bandwidth(unit:Kbit/s),when <1000,should be 64*n\n"
	"input the assured bandwidth(unit:Kbit/s),when <1000,should be 64*n\n"
	"input the best effort bandwidth(unit:Kbit/s), when <1000,should be 64*n;and should be no smaller than assured-bw\n"
      )
{
	UCHAR macAddr[6];
	ULONG fixedBw = 0;
	ULONG assBw = 0;
	ULONG bestEfBw = 0;
	ULONG bwClass = 0;
	ULONG bwDelay = CLIE_BANDWITHDELAY_LOW;

	if( VOS_ERROR == GetMacAddr(argv[0], macAddr) )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}
	if( MAC_ADDR_IS_INVALID(macAddr) )
	{
		vty_out(vty,"This is a Invalid mac address\r\n");
		return (CMD_WARNING);
	}

	bwClass = ( unsigned int ) VOS_AtoL( argv[1] );
	
	if ( !VOS_StrCmp( argv[2], "high" ) )
	{
		bwDelay = CLIE_BANDWITHDELAY_HIGH;
	}

	fixedBw = ( UINT32 ) VOS_AtoL( argv[ 3 ] );
	if( fixedBw < 1000 )
	{
		if (0 != (fixedBw % CLI_EPON_BANDWIDTH_MIN))
		{
			vty_out( vty, "  %% when fixed-bw < 1000, should be 64*n\r\n" );
			return CMD_WARNING;
		}
	}
	
	assBw = ( UINT32 ) VOS_AtoL( argv[4] );
	if( assBw < CLI_EPON_BANDWIDTH_MIN )
	{
		vty_out( vty, "  %% Parameter is error.assured-bw should be no smaller than 64kbit/s.\r\n" );
		return CMD_WARNING;
	}
	if( assBw < 1000 )
	{
		if (0 != (assBw % CLI_EPON_BANDWIDTH_MIN))
		{
			vty_out( vty, "  %% when Assured-bw<1000,should be 64*n\r\n" );
			return CMD_WARNING;
		}
	}

	bestEfBw = ( UINT32 ) VOS_AtoL( argv[5] );
#ifdef PLATO_DBA_V3
	if(bestEfBw < (assBw+fixedBw))
#else
	if(bestEfBw < assBw )
#endif
	{
		vty_out( vty, "  %% Parameter error. Best-effort-bw must be greater than or equal to ");
#ifdef PLATO_DBA_V3
		vty_out(vty,"fixed-bw+");
#endif
		vty_out(vty,"assured-bw\r\n");
		return CMD_WARNING;
	}
	if (bestEfBw < CLI_EPON_BANDWIDTH_MIN)
	{
		vty_out( vty, "  %% Parameter is error.Best-effort-bw should be no smaller than 64kbit/s.\r\n" );
		return CMD_WARNING;
	}
	if(bestEfBw<1000)
	{
		if (0 != (bestEfBw % CLI_EPON_BANDWIDTH_MIN) )
		{
			vty_out( vty, "  %% when Best-effort-bw<1000,should be 64*n\r\n" );
			return CMD_WARNING;
		}
	}
	
	setOnuUpBwBasedMac( macAddr, bwClass, bwDelay, fixedBw,assBw, bestEfBw );

	return CMD_SUCCESS;
}

DEFUN( config_onu_down_bw_based_mac,
       config_onu_down_bw_based_mac_cmd,
	"bandwidth-mac down <H.H.H> <0-7> [high|low] <64-1000000> <64-1000000>",
	"config bandwidth based onu mac addr.\n"
	"down bandwidth based onu mac addr.\n"
	"input onu mac address\n"
	"input bandwidth class\n"
	"bandwidth delay high\n"
	"bandwidth delay low\n"
	"input the assured bandwidth(unit:Kbit/s),when <1000,should be 64*n\n"
	"input the best effort bandwidth(unit:Kbit/s), when <1000,should be 64*n;and should be no smaller than assured-bw\n"
      )
{
	UCHAR macAddr[6];
	ULONG assBw = 0;
	ULONG bestEfBw = 0;
	ULONG bwClass = 0;
	ULONG bwDelay = CLIE_BANDWITHDELAY_LOW;

	if( VOS_ERROR == GetMacAddr(argv[0], macAddr) )
	{
		vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;
	}
	if( MAC_ADDR_IS_INVALID(macAddr) )
	{
		vty_out(vty,"This is a Invalid mac address\r\n");
		return (CMD_WARNING);
	}

	bwClass = ( unsigned int ) VOS_AtoL( argv[1] );
	
	if ( !VOS_StrCmp( argv[2], "high" ) )
	{
		bwDelay = CLIE_BANDWITHDELAY_HIGH;
	}

	assBw = ( UINT32 ) VOS_AtoL( argv[3] );
	if( assBw < CLI_EPON_BANDWIDTH_MIN )
	{
		vty_out( vty, "  %% Parameter is error.assured-bw should be no smaller than 64kbit/s.\r\n" );
		return CMD_WARNING;
	}
	if( assBw < 1000 )
	{
		if (0 != (assBw % CLI_EPON_BANDWIDTH_MIN))
		{
			vty_out( vty, "  %% when Assured-bw<1000,should be 64*n\r\n" );
			return CMD_WARNING;
		}
	}

	bestEfBw = ( UINT32 ) VOS_AtoL( argv[4] );
	if(bestEfBw < assBw )
	{
		vty_out( vty, "  %% Parameter error. Best-effort-bw must be greater than or equal to assured-bw\r\n");
		return CMD_WARNING;
	}
	if (bestEfBw < CLI_EPON_BANDWIDTH_MIN)
	{
		vty_out( vty, "  %% Parameter is error.Best-effort-bw should be no smaller than 64kbit/s.\r\n" );
		return CMD_WARNING;
	}
	if(bestEfBw<1000)
	{
		if (0 != (bestEfBw % CLI_EPON_BANDWIDTH_MIN) )
		{
			vty_out( vty, "  %% when Best-effort-bw<1000,should be 64*n\r\n" );
			return CMD_WARNING;
		}
	}
	
	setOnuDownBwBasedMac( macAddr, bwClass, bwDelay, assBw, bestEfBw );

	return CMD_SUCCESS;
}

DEFUN( undo_onu_up_bw_based_mac,
       undo_onu_up_bw_based_mac_cmd,
	"undo bandwidth-mac [up|down] <H.H.H>",
	"undo bandwidth based onu mac addr.\n"
	"undo bandwidth based onu mac addr.\n"
	"up bandwidth based onu mac addr.\n"
	"down bandwidth based onu mac addr.\n"
	"input onu mac address\n"
      )
{
	UCHAR macAddr[6];

	if( VOS_ERROR == GetMacAddr(argv[1], macAddr) )
	{
		/*vty_out( vty, "  %% Invalid MAC address.\r\n" );
		return CMD_WARNING;*/
	}
	/*if( MAC_ADDR_IS_INVALID(macAddr) )
	{
		vty_out(vty,"This is a Invalid mac address\r\n");
		return (CMD_WARNING);
	}*/

	if( argv[0][0] == 'u' )
		delOnuUpBwBasedMac( macAddr );
	else if( argv[0][0] == 'd' )
		delOnuDownBwBasedMac( macAddr );
	else
		return CMD_WARNING;

	return CMD_SUCCESS;
}

DEFUN( show_onu_bw_based_mac,
       show_onu_bw_based_mac_cmd,
	"show bandwidth-mac {[up|down]}*1",
	DescStringCommonShow
	"show bandwidth based onu mac addr.\n"
	"up dir\n"
	"down dir\n"
	)
{
	int nullFlag = 1;
	int dir = 3;
	ULONG counter = 0;
	onu_up_bw_node_t upBwNode;
	onu_down_bw_node_t downBwNode;

	if( argc != 0 )
	{
		if( argv[0][0] == 'u' )
			dir = 1;
		else
			dir = 2;
	}

	if( dir & 1 )
	{
		if( getFirstOnuUpBwBasedMacNode(&upBwNode) == VOS_OK )
		{
			vty_out( vty, " Up bandwidth:\r\n" );
			vty_out( vty, " %-16s%-8s%-8s%-12s%-12s%-13s%s\r\n", "onu-mac", "class", "delay", "fixed-bw", "assured-bw", "best-effort", "status" );

			do {
				counter++;
				vty_out( vty, "%-18s%-8d%-8s%-12d%-12d%-12d%s\r\n", macAddress_To_Strings(upBwNode.macAddr), 
					upBwNode.bwClass, (upBwNode.bwDelay==V2R1_DELAY_LOW ? "low" : "high"),
					upBwNode.fixedBw, upBwNode.assuredBw, upBwNode.bestEffortBw,
					(upBwNode.rowStatus == RS_ACTIVE ? "active" : "notReady") );
			} while( getNextOnuUpBwBasedMacNode(upBwNode.macAddr, &upBwNode) == VOS_OK );
			
			vty_out( vty, " count:%d\r\n", counter );

			nullFlag = 0;
		}
		vty_out( vty, "\r\n" );
	}

	if( dir & 2 )
	{
		counter = 0;
		if( getFirstOnuDownBwBasedMacNode(&downBwNode) == VOS_OK )
		{
			vty_out( vty, " Down bandwidth:\r\n" );
			vty_out( vty, " %-16s%-8s%-8s%-12s%-13s%s\r\n", "onu-mac", "class", "delay", "assured-bw", "best-effort", "status" );

			do {
				counter++;
				vty_out( vty, "%-18s%-8d%-8s%-12d%-12d%s\r\n", macAddress_To_Strings(downBwNode.macAddr), 
					downBwNode.bwClass, (downBwNode.bwDelay==V2R1_DELAY_LOW ? "low" : "high"),
					downBwNode.assuredBw, downBwNode.bestEffortBw,
					(downBwNode.rowStatus == RS_ACTIVE ? "active" : "notReady") );
			} while( getNextOnuDownBwBasedMacNode(downBwNode.macAddr, &downBwNode) == VOS_OK );
			
			vty_out( vty, " count:%d\r\n", counter );

			nullFlag = 0;
		}
	}
	
	if( nullFlag )
		vty_out( vty, " Not find bandwidth based onu mac addr.\r\n" );

	return CMD_SUCCESS;
}

LONG onu_bw_cli_cmd_install()
{
	install_element ( CONFIG_NODE, &config_onu_up_bw_based_mac_cmd);
	install_element ( CONFIG_NODE, &config_onu_down_bw_based_mac_cmd);
	install_element ( CONFIG_NODE, &undo_onu_up_bw_based_mac_cmd);
	install_element ( CONFIG_NODE, &show_onu_bw_based_mac_cmd);
	return VOS_OK;
}

int onu_bw_showrun( struct vty * vty )
{
	onu_up_bw_node_t upNode;
	onu_down_bw_node_t downNode;

	vty_out( vty, "!Onu bandwidth config based mac addr.\r\n" );
	
	if( getFirstOnuUpBwBasedMacNode(&upNode) == VOS_OK )
	{
		do
		{
			vty_out( vty,  " bandwidth-mac up %s %d %s %d %d %d\r\n", macAddress_To_Strings(upNode.macAddr),
				upNode.bwClass, (upNode.bwDelay==V2R1_DELAY_LOW ? "low" : "high"), 
				upNode.fixedBw, upNode.assuredBw, upNode.bestEffortBw );
			
		} while( getNextOnuUpBwBasedMacNode(upNode.macAddr, &upNode) == VOS_OK );
	}
	
	if( getFirstOnuDownBwBasedMacNode(&downNode) == VOS_OK )
	{
		do
		{
			vty_out( vty,  " bandwidth-mac down %s %d %s %d %d\r\n", macAddress_To_Strings(downNode.macAddr),
				downNode.bwClass, (downNode.bwDelay==V2R1_DELAY_LOW ? "low" : "high"), 
				downNode.assuredBw, downNode.bestEffortBw );
			
		} while( getNextOnuDownBwBasedMacNode(downNode.macAddr, &downNode) == VOS_OK );
	}

	vty_out( vty, "!\r\n\r\n" );

	return VOS_OK;
}

LONG onu_bw_based_mac_module_init()
{
    struct cl_cmd_module * onu_bw_module = NULL;

    onu_bw_module = ( struct cl_cmd_module * ) VOS_Malloc( sizeof( struct cl_cmd_module ), MODULE_ONU);
    if ( !onu_bw_module )
    {
        ASSERT( 0 );
        return VOS_ERROR;
    }

    VOS_MemZero( ( char * ) onu_bw_module, sizeof( struct cl_cmd_module ) );

    onu_bw_module->module_name = ( char * ) VOS_Malloc( 20, MODULE_EVENT );
    if ( !onu_bw_module->module_name )
    {
        ASSERT( 0 );
        VOS_Free( onu_bw_module );
        return VOS_ERROR;
    }
    VOS_StrCpy( onu_bw_module->module_name, "bandwidth-mac" );

    onu_bw_module->init_func = init_onuBwBasedMac;
    onu_bw_module->showrun_func = onu_bw_showrun;
    onu_bw_module->next = NULL;
    onu_bw_module->prev = NULL;
    cl_install_module( onu_bw_module );

    return VOS_OK;
}


#ifdef __cplusplus
}
#endif
