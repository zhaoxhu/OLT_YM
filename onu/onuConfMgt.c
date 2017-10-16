/*
 * onuConfMgt.c
 *
 *  Created on: 2011-5-6
 *      Author: wangxy
 */

#ifdef  __cplusplus
extern "C"
{
#endif

#include  "OltGeneral.h"
#include  "gwEponSys.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"
#include  "includeFromPas.h"
#include    "lib_gwEponMib.h"
#include "cdp_pub.h"
#include "onuConfMgt.h"

#include "cli/cli.h"
#include "cli/cl_buf.h"
#include "cli/cl_vect.h"

#include "ifm/pon/pon_type.h"

#include "vos/vospubh/vos_math.h"
#include "llib/ziplib/LzmaLib.h"

#include "device_flash.h"
#include "cpi/cdsms/cdsms_file.h"
#include "../ct_manage/CT_Onu_event.h"

#include "Addr_aux.h"

extern unsigned char *cl_config_mem_file_init( void );
extern struct vty *vty_new ( void );
ULONG get_ctcOnuOtherVendorAccept();
#define CDP_DEBUG

#define FILE_WRITE_NOAPPEND 0
#define FILE_WRITE_APPEND 1
#define ONU_CONF_FILE_END_FLAG 0x55aa55aa
#define GW_ONU_RESUME_BUF_MAX_SIZE RPC_CMD_BUF_LEN*16
#define SYS_LOCAL_MODULE_TYPE_IS_6900_SW  (__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW)
#define SYS_LOCAL_MODULE_TYPE_IS_8000_SW  (__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA8000_SW)
/*#undef  CDP_DEBUG*/
ULONG onu_conf_flash_empty = 0;
ULONG onu_conf_sem_deb = 0;
void *lzma_malloc(int size);
void lzma_free(void *obj);

/*调试信息输出标志变量*/
ULONG onu_conf_print_debug = 0;
ULONG g_onu_temp_profile_num = 0;
#define HASH_BUCKET_DEPTH   4094
extern int UpdatePendingQueueTime;


enum{
    ONU_CONF_EXEC_RESULT_NULL,
	ONU_CONF_EXEC_RESULT_OK,
	ONU_CONF_EXEC_RESULT_ERR
};

enum{
    MIB_ONU_PROFILE_APPLY = 1,
    MIB_ONU_PROFILE_CANCEL
};

typedef struct element_onuconfhashbucket
{
    void *confdata;
    struct element_onuconfhashbucket *next;
} element_onuconfhashbucket_t;

typedef struct onuconfhashitem{
    char used;
    element_onuconfhashbucket_t data;
}onuconfhashitem_t;

typedef struct ONUCONFSYNDMSG{
    USHORT slotid;
    USHORT ponid;
    USHORT onuid;
    USHORT msgtype;
    int  data[1];
}ONUCONFSYNDMSG_T, *ONUCONFSYNDMSGPTR_T;

struct rename_list{
    char filename[ONU_CONFIG_NAME_LEN];
    char rename[ONU_CONFIG_NAME_LEN];
    char valid;
};

typedef  void (*onuconf_event_callback)(ULONG aulMsg[4]);

#define MAX_RENAME_NUM 8
struct rename_list g_rename_list[MAX_RENAME_NUM];

#ifndef ONUID_MAP
typedef struct onu_conf_save_stack_list{
    void *data;
    struct onu_conf_save_stack_list *next;
}onu_conf_save_stack_list_t, *onu_conf_save_stack_list_ptr_t;
#endif

enum{
    MSG_ONU_CONFSYND_CARD_REQ,              /* 0 向指定PON板改送全部配置文件和MAC地址关联*/
    MSG_ONU_CONFSYND_REQ_CDP,               /* 1 更新某个配置文件或指定ONU指定的配置文件*/
    MSG_ONU_CONFSYND_MAC_REQ_CDP,           /* 2 为指定的ONU更新MAC关联*/
    MSG_ONU_CONFSYND_DEL_CDP,               /* 3 删除某个配置文件*/
    MSG_ONU_CONFSYND_EXECUTE_CDP,           /* 4 要求PON板执行某个ONU的数据配置恢复*/
    MSG_ONU_CONFSYND_UNDO_ASSOCIATION_BY_NAME,  /* 5 根据文件名去关联*/
    MSG_ONU_CONFSYND_UNDO_ASSOCIATION_BY_ONUID, /* 6 根据地ONUID去关联*/
    MSG_ONU_CONFSYND_ONUID_MAP_REQ_CDP,     /* 7 同步ONU ID映射表*/
    MSG_ONU_CONFSYND_RESULT_REPORT_CDP,     /* 8 向主控板发送数据恢复结果*/
    MSG_ONU_CONFSYND_DEBUG_REPORT_CDP,       /* 9 向主控板改送调试信息*/
    MSG_ONU_CONFSYND_APPLY_REQ,              /* 10 向某ONU发送应用配置指令*/
    MSG_ONU_CONFSYND_RENAME_REQ_CDP,         /* 11 要求重命名某配置文件*/
    MSG_ONU_CONFSYND_RENAME_CHECK_CDP,       /* 12 rename操作检查消息*/
    MSG_ONU_CONFSYND_REQ_ACK_CDP,            /* 13 需要应答的命令的响应消息*/
    MSG_ONU_CONFSYND_TRANSMISSION_FLAG_CDP,  /* 14 Transmisstion-flag CDP 响应消息*/
    MSG_ONU_CONFSYND_ALARM_DATA_CDP,          /* 15 CTC-ONU ALARM Pon板掉电恢复 CDP 响应消息*/
    MSG_ONU_OTHER_VENDOR_REGISTRATION_CDP,    /* 16 其它厂商的CTC-ONU允许注册规则设定消息*/
    MSG_ONU_CONFSYND_CARD_REQ_FINISHED_CDP,	/* 17 板同步完毕通知 master active --> slave*/
    MSG_ONU_CONFSYND_CARD_REQ_FINISHED_ACK_CDP, /* 18 板同步完毕响应 slave-->master active*/
    MSG_ONU_CONFSYND_PRIVATE_PTY_ENABLE_CDP, /* 19 new add by luh 2012-12-21*/
    MSG_ONU_CONFSYND_PENDING_UPDATETIME_CDP, /* 20 new add by luh 2012-12-21*/    
    MSG_ONU_CONFSYND_ONU_MODEL_BLACK_LIST_CDP,/* 21 */
    MSG_ONU_CONFSYND_CDP_MAX /* 22 */
};

/*added by wangjiah@2017-03-08 for pon protection onuconf debug:begin*/
int DEBUGONUCONF = V2R1_DISABLE;
int MAX_DEBUG_ONU = 0;
#define ONUCONF_DEBUG if(V2R1_ENABLE == DEBUGONUCONF) sys_console_printf
#define ONUCONF_ONU_LIMIT_DEBUG if(V2R1_ENABLE == DEBUGONUCONF && MAX_DEBUG_ONU >= onuid) sys_console_printf
/*added by wangjiah@2017-03-08 for pon protection onuconf debug:end*/

#define MAX_ONUMODEL_BLACKLIST 10
ULONG onuModelBlacklList[MAX_ONUMODEL_BLACKLIST] = {0,0,0,0,0,0,0,0,0,0};

/*配置文件修改纪录表*/
static cl_vector g_onuConfFileOpRecord;

/*主控板向PON板广播配置数据的控制字，每bit代表一个PON板同步完成状态*/
static ULONG s_onuConfBroadcastCardMask = 0;

/*系统中ONU配置数据申请内存的首地址*/
static onuConfDataItem_t *g_onuConfDataMem;
static int g_onuConfDataItemNum = 0;
static onuconfhashitem_t *g_onuConfHashMem;
static int g_onuConfHashItemNum = 0;

/*内存申请互斥信号*/
ULONG g_onuConfMemSemId = 0;
/**/
static onuconf_event_callback g_onuconf_event_callback[MSG_ONU_CONFSYND_CDP_MAX];

#ifndef ONUID_MAP
static onu_conf_save_stack_list_ptr_t g_onuconf_save_stack_head = NULL, g_onuconf_save_stack_tail=NULL;
#endif
extern int g_SystemLoadConfComplete;
int g_maxPrimeNumber = 0;
int *g_onuConfBucket = NULL, *g_onuConfMacBucket = NULL;
/*主备同步数据的时间间隔*/
extern LONG g_lBACBatchTimer;
int g_gwonu_pty_flag = 1;
/*
 * onu id与配置文件关联全局变量，一维下标为ponid，二维下标为onuid
 */
static onuConfOnuIdMapEntry_t g_onuConfOnuIdMapArry[SYS_MAX_PON_PORTNUM][MAXONUPERPONNOLIMIT];

/*
 * author wangxy
 * 其它厂家的CTC ONU注册规则，1：允许注册，0：拒绝注册
 */
static ULONG ctc_onu_other_vendor_accept = 1;
/*
 * 配置文件访问的控制信号量
 */
ULONG g_onuConfSemId = 0;

char *getOnuConfNamePtrByPonId(short int ponid, short int onuid);
short int  onuConfGetPonIdFromPonProtectGroup(short int ponid);

static void delElementFromRestoreQueue(short int queid, onu_conf_res_queue_list_t *pd);
static int checkCompressConfigFormat(const char *buf, const int length);
static int sfun_OnuConfRename(const char *filename, const char *rename);
static int sfun_clrOnuConfNameMapByPonId(const char *name, short int ponid, short int onuid);

int addOnuProfileSyncByslot(short int slot, const char *name);
int deleteOnuProfileSyncBySlot(short int slot, const char *name);
void *getOnuConfHashBucketElement(const char *name, void **head);
int sendOnuConfExecuteMsg(USHORT slot, USHORT pon, USHORT onu, int act);
int OnuProfile_Action_ByCode(g_OnuProfile_action_code_t code, short int card, short int ponid, short int onuid, const char*name, const char *rename, void* ptr);
static int getQueIdBySlotno(int slotno);
static int getSemIdBySlotno(int slotno);
int addOnuToRestoreQueueByName(SHORT pon_id, SHORT onu_id, const char *name, onu_conf_res_act_t act);
int addOnuToRestoreQueue(SHORT pon_id, SHORT onu_id, onu_conf_res_act_t act, UCHAR restore_flags);
int sendOnuAlarmConfSyndDataBySlot(USHORT slot);
int ResumeOnuTransmissionflagBySlot(USHORT slot, int enable);
int ResumeOtherCtcOnuRegistrationBySlot(USHORT slot, ULONG v);
int ResumeUpdatePendingPeriodBySlot(USHORT slot, int period);
int SearchOnuVlanPort(short int PonPortIdx, short int OnuIdx, USHORT vid, ULONG *brdIdx, ULONG *portIdx);
int SearchOnuVlanPortByPon(short int PonPortIdx, USHORT vid, short int *OnuIdx, ULONG *brdIdx, ULONG *portIdx);
int ResumeOnuModelBlackListBySlot(USHORT slot);

#ifndef ONUID_MAP
#define ONU_CONF_NAME_PTR_GET(ponid, onuid) \
        OnuMgmtTable[ponid*MAXONUPERPON+onuid].configFileName
#else
#define ONU_CONF_NAME_PTR_GET(ponid, onuid) \
		getOnuConfNamePtrByPonId(ponid, onuid)
#endif


#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_NO)
#define ONU_PORT_VALID(port) (port >= 1 && port <= (ONU_MAX_PORT))
#else
#define ONU_PORT_VALID(port) TRUE
#endif

#define vty_out_onuconf_port_var(ptr_port, var, rmask, rv, rnum) \
{ \
    int mask = 0xffffffff; \
    int i=0; \
    while(mask) \
    { \
        int gmask = 0; \
        int gv = -1; \
        for(i=0; i<32; i++) \
        { \
            if(mask&(1<<i)) \
            { \
                if(gv == -1 || gv == (ptr_port[i].var)) \
                { \
                    gv = ptr_port[i].var; \
                    gmask |= 1<<i; \
                    mask &= ~(1<<i); \
                } \
                else \
                    continue; \
            } \
        } \
        if(gmask) \
        { \
            rmask[rnum] = gmask; \
            rv[rnum] = gv; \
            rnum++; \
        } \
    } \
}

#define vty_out_port_var_undo_list(portptr, defptr, var, rmask) \
{ \
	int i=0; \
	rmask = 0; \
	for(i=0; i<ONU_MAX_PORT; i++)\
	{\
		if((portptr[i].var) !=(defptr[i].var))\
			rmask |= 1<<i;\
	}\
}

typedef union tagIFM_Onu_Profile_IfIndex
{
    struct tagPhy
    {
        unsigned type:6;
        unsigned reserv:26;

    }pindex;
    ULONG ulIfindex;
} IFM_ONU_PROFILE_IF_INDEX_U;
int assert_counter = 100;
int test_assert()
{
	while(assert_counter)
		{
		VOS_ASSERT_X(0, "OnuConfMgt.c", 1818, 0);
		VOS_ASSERT_X(0, "PonHander.c", 2000, 0);
		VOS_ASSERT_X(0, "Olt_cli.c", 3333, 0);
		VOS_ASSERT_X(0, "OnuGeneral.c", 4444, 0);
		VOS_ASSERT_X(0, "Ospf_rount.c", 1234, 0);
		VOS_ASSERT_X(0, "sn_api.c", 4422, 0);
		VOS_ASSERT_X(0, "Qos_class_cli.c", 5566, 0);
		VOS_ASSERT_X(0, "Qos_main.c", 12345, 0);
		VOS_ASSERT_X(0, "Radius_cli.c", 3214, 0);
		assert_counter--;
		}
}
#if 1
static int
isPrimeNumber(int a)
{

/*    int k = sqrt((double)a);*/
    int k = a/2+1;
    int i = 0;

    for (i = 2; i <= k; i++)
    {
        if (!(a % i))
            break;
    }

    if (i >= k + 1 && a != 1)
        return 1;
    else
        return 0;
}

static int
findMaxPrimeNum(int max)
{
    int i = 0;
    int prime = 2;
    for (i = max; i >= 2; i--)
        if (isPrimeNumber(i))
        {
            prime = i;
            break;
        }

    return prime;
}

int
getOnuConfHashIndex(const char *szName)
{

    unsigned int seed = 131;
    unsigned int hash = 0;

    while (*szName)
    {
        hash = hash * seed + (*szName++);
    }

    return (hash % g_maxPrimeNumber);
}

int
initOnuConfHashBucket(const int depth)
{
    if(g_onuConfBucket && g_onuConfMacBucket)
        return 1;

    g_maxPrimeNumber = findMaxPrimeNum(depth);

    g_onuConfBucket = VOS_Malloc(depth * sizeof(int), MODULE_RPU_CTC);

    if (g_onuConfBucket)
    {
        memset(g_onuConfBucket, 0, sizeof(int)*depth);
    }

    g_onuConfMacBucket = VOS_Malloc(depth * sizeof(int), MODULE_RPU_CTC);

    if(g_onuConfMacBucket)
    {
        VOS_MemSet(g_onuConfMacBucket, 0, sizeof(int)*depth);
    }

    if(g_onuConfBucket && g_onuConfMacBucket)
        return 1;
    else
    {
        if(g_onuConfBucket)
            VOS_Free(g_onuConfBucket);
        if(g_onuConfMacBucket)
            VOS_Free(g_onuConfMacBucket);
        return 0;
    }

}
#endif

#if 1/*onu-profile:add and sync*/
int setOnuConfToHashBucket(const char *name, void *ptr)
{
    int iRet = 0;
    int *ph = 0;
    
    ONU_CONF_SEM_TAKE
    {
        element_onuconfhashbucket_t *p =
                (element_onuconfhashbucket_t*) getOnuConfHashBucketElement(name,
                        (void**)&ph);
        if (!p)
        {
            element_onuconfhashbucket_t *p = onuconf_malloc(ONU_CONF_MEM_HASH_ID);

            if (p)
            {
                p->confdata = ptr;
                p->next = NULL;

                if (*ph)
                {
					element_onuconfhashbucket_t * pp = (element_onuconfhashbucket_t*)(*ph);
					while(pp)
					{
						if(!pp->next)
						{
							pp->next = p;
							break;
						}
						pp = pp->next;
					}
                }
                else
                {
                    *ph = (int)p;
                }
                
                /*added by luh 2012-10-26; 增加临时文件计数*/
                if(!VOS_StrnCmp(name, "*auto", 5))
                    g_onu_temp_profile_num++;
                
            }
            else
                iRet = -1;
        }
        else
        {
            /*VOS_Free(p->confdata);*/
            onuconf_free(p->confdata, ONU_CONF_MEM_DATA_ID);
            p->confdata = ptr;
        }

    }
    ONU_CONF_SEM_GIVE

    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        ULONG uv[4] = {0,0,0,0};
        uv[2] = (ULONG)name;
        uv[3] = EM_ONU_PROFILE_CREATED;
        processEventCallbackList(EVENT_ONUPROFILE_CREATE, uv);
    }
    return iRet;
}

int addOnuProfileSyncBroadcastByName(const char *name)
{        
    int i,ret=0;
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for (i = 1; i <= SYS_CHASSIS_SLOTNUM; i++)
        {
            ret |= addOnuProfileSyncByslot(i, name);
        }
    }
    return ret ? VOS_ERROR : VOS_OK;
}
int
sendOnuConfSyndMsg(USHORT dstSlot, ONUCONFSYNDMSGPTR_T pData, ULONG ulLen)
{
    int ret = VOS_ERROR;

	ONUCONFSYNDMSGPTR_T p = NULL;

	if(!pData)
	{
		VOS_ASSERT(0);
		return ret;
	}

	if(!SYS_MODULE_IS_READY(dstSlot) || !SYS_MODULE_SLOT_ISHAVECPU(dstSlot) || dstSlot == SYS_LOCAL_MODULE_SLOTNO)
	{
	    VOS_Free(pData);
		return ret;
	}

    p =  CDP_AllocMsg(ulLen, MODULE_RPU_CTC);
    if (p)
    {

        VOS_MemCpy(p, pData, ulLen);

        if (CDP_Send(RPU_TID_CDP_ONU_CONFSYND, dstSlot,
                RPU_TID_CDP_ONU_CONFSYND, CDP_MSG_TM_SYNC, p, ulLen,
                MODULE_RPU_CTC) == VOS_OK)
            ret = VOS_OK;

        CDP_FreeMsg(p);

    }

    VOS_Free(pData);

    return ret;
}
/*
int sendOnuConfSyndReqMsg(USHORT slot, USHORT pon, USHORT onu)
{
    int ret = VOS_OK;

#ifndef ONUID_MAP
    int ppidx = GetGlobalPonPortIdxBySlot(slot, pon);
    int onuEntry = ppidx*MAXONUPERPON+onu-1;



    ONUConfigData_t * pd = getOnuConfFromHashBucket(OnuMgmtTable[onuEntry].configFileName);
#else
    int ponid = GetPonPortIdxBySlot(slot, pon);
    int onuid = onu-1;

    if(ponid == -1 || onuid == -1)
    {
        ret = VOS_ERROR;
        return ret;
    }

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
#endif

    if(pd)
    {
#if 0
        ULONG len = sizeof(ONUCONFSYNDMSG_T)+sizeof(ONUConfigData_t);
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
        if(p)
        {
            p->slotid = slot;
            p->ponid = pon;
            p->onuid = onu-1;
            p->msgtype = MSG_ONU_CONFSYND_REQ_CDP;
            VOS_MemCpy(p->data, pd, sizeof(ONUConfigData_t));

            ret = sendOnuConfSyndMsg(slot, p, len);

        }
        else
            ret = VOS_ERROR;
#else
        char *data = NULL;
        ULONG datalen = onuconf_build_lzma_data((char*)pd, sizeof(ONUConfigData_t), &data);

        if(datalen)
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T)+datalen;
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = slot;
                p->ponid = pon;
                p->onuid = onu-1;
                p->msgtype = MSG_ONU_CONFSYND_REQ_CDP;
                VOS_MemCpy(p->data, data, datalen);
                free(data);

                ret = sendOnuConfSyndMsg(slot, p, len);

            }
            else
                ret = VOS_ERROR;
        }
        else
        	ret = VOS_ERROR;
#endif
    }
    else
        ret = VOS_ERROR;

#ifdef ONUID_MAP
    }
    ONU_CONF_SEM_GIVE
#endif

    return ret;
}
*/
int addOnuProfileSyncByslot(short int slot, const char *name)
{
    int ret = VOS_OK;

    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return VOS_OK;
    
    ONU_CONF_SEM_TAKE
    if (slot != SYS_LOCAL_MODULE_SLOTNO && 
        (SYS_MODULE_SLOT_ISHAVECPU(slot)||SYS_MODULE_ISMASTERSTANDBY(slot)))        
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(name);
        if(pd)
        {
            char *data = NULL;
            ULONG datalen = onuconf_build_lzma_data((char*)pd, sizeof(ONUConfigData_t), &data);

            if(datalen)
            {
                ULONG len = sizeof(ONUCONFSYNDMSG_T)+datalen;
                ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
                if(p)
                {
                    p->slotid = slot;
                    p->ponid = 0;
                    p->onuid = 0;
                    p->msgtype = MSG_ONU_CONFSYND_REQ_CDP;
                    VOS_MemCpy(p->data, data, datalen);
                    /*free(data);*/
                    ret = sendOnuConfSyndMsg(slot, p, len);
                }
                else
                    ret = VOS_ERROR; 
                if (data) g_free(data); /*2013-2-21 added by luh, 不论是否申请用户内存成功，此处都必须释放系统内存*/
            }
            else
            	ret = VOS_ERROR;
        }
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE;

    return ret;
}

/*int sendOnuConfSyndReqMsgByName(USHORT slot, const char *name)
{

	int ret = VOS_OK;

    ONUConfigData_t * pd = getOnuConfFromHashBucket(name);

    if(pd)
    {
#if 0
        ULONG len = sizeof(ONUCONFSYNDMSG_T)+sizeof(ONUConfigData_t);
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
        if(p)
        {

            p->slotid = slot;
            p->ponid = 0;
            p->onuid = 0;
            p->msgtype = MSG_ONU_CONFSYND_REQ_CDP;
            VOS_MemCpy(p->data, pd, sizeof(ONUConfigData_t));
            ret = sendOnuConfSyndMsg(slot, p, len);

        }
        else
           ret = VOS_ERROR;
#else
        char *data = NULL;
        ULONG datalen = onuconf_build_lzma_data((char*)pd, sizeof(ONUConfigData_t), &data);

        if(datalen)
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T)+datalen;
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = slot;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_REQ_CDP;
                VOS_MemCpy(p->data, data, datalen);
                free(data);

                ret = sendOnuConfSyndMsg(slot, p, len);

            }
            else
                ret = VOS_ERROR;
        }
        else
        	ret = VOS_ERROR;
#endif
    }
    else
        ret = VOS_ERROR;

    return ret;
}*/

/*备用主控，板插入回调*/
LONG card_insert_callback_sync_onuconf(int slotno,LONG module_type)
{
	if(!(SYS_LOCAL_MODULE_ISMASTERACTIVE && SYS_MODULE_ISHAVECPU(SYS_MODULE_TYPE(slotno))))
		return VOS_OK;

    if(SYS_MODULE_ISMASTERSTANDBY(slotno))
    {
	    if( SYS_LOCAL_MODULE_ISMASTERACTIVE && (device_standby_master_slotno_get() != 0) )
	        config_sync_notify_event();
        startOnuConfSyndReqByCard(slotno);
    }

    return VOS_OK;
}
/*Pon板激活时发送配置文件要到各个板同步任务发送，执行效率快*/
int startOnuConfSyndReqByCard(int slotid)
{

    ULONG queid;
    ULONG ulArgs[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_CARD_REQ, 0, 0};
	/*ULONG semid = getSemIdBySlotno(slotid);*/
    ulArgs[3] = slotid;

	if(!(SYS_LOCAL_MODULE_ISMASTERACTIVE && SYS_MODULE_ISHAVECPU(SYS_MODULE_TYPE(slotid))))
		return VOS_ERROR;

    if( ( slotid == SYS_LOCAL_MODULE_SLOTNO ) || (!(SYS_MODULE_SLOT_ISHAVECPU(slotid) || SYS_MODULE_ISMASTERSTANDBY(slotid))) )
        return VOS_ERROR;

    queid = getQueIdBySlotno(slotid);

    /*if(VOS_SemTake(semid, ONU_CONF_CARD_SYNC_WAIT_TIME*VOS_TICK_SECOND) != VOS_OK)
    {
    	VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "card_insert_callback_sync_onuconf (slot %d) sem take fail\r\n", slotid);
    	return VOS_ERROR;
    }
    else
    */
    if(onuconf_wait_cardsyncfinished_signal(slotid, 1) != VOS_OK)
    {    
        /*added by luh@2015-2-6. 如果take 不到信号量，强制释放，同pon板的处理*/
    	onuconf_signal_cardsyncfinished(slotid); 
#if 1
    	/*2015-04-21. 如果上一次出错，应该释放信号量后，再进行尝试。而不需等1个小时。Q.25339*/
    	ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP, ("card sync begin: slot %d, ticks: %u, semid:%u\r\n", slotid, VOS_GetTick(), getSemIdBySlotno(slotid)));
        return VOS_QueSend(queid, ulArgs,WAIT_FOREVER, MSG_PRI_NORMAL);
#else    	
    	VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "startOnuConfSyndReqByCard (slot %d) sem take fail\r\n", slotid);
    	return VOS_ERROR;
#endif    	
    }
    else
    {
    	ULONG semid = getSemIdBySlotno(slotid);
    	ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP, ("card sync begin: slot %d, ticks: %u, semid:%u\r\n", slotid, VOS_GetTick(), semid));
        return VOS_QueSend(queid, ulArgs,WAIT_FOREVER, MSG_PRI_NORMAL);
    }

}
/*int sendOnuConfMacToAllPonBoard(const char *mac, const char* name)
{
    int i, ret = 0;

    for(i=1;i<=SYS_CHASSIS_SLAVE_SLOTNUM;i++)
        if(SYS_MODULE_IS_PON(i))
            ret |= sendOnuConfMacSyndReqMsg(i, mac, name);

    return ret?VOS_ERROR:VOS_OK;
}*/

/*int sendOnuConfSyndBroadcastReqMsg(const char *name)
{

        int i,ret=0;

        for (i = 1; i <= SYS_CHASSIS_SLOTNUM; i++)
        {
            if (SYS_MODULE_IS_PON(i))
            {
                ret |= sendOnuConfSyndReqMsgByName(i, name);
            }
        }
        return ret ? VOS_ERROR : VOS_OK;

}*/

/*int sendOnuConfMacSyndReqMsg(USHORT slot, const char *mac, const char *name)
{
    int ret = VOS_OK;
    ULONG len = sizeof(ONUCONFSYNDMSG_T) + sizeof(onuConfMacHashEntry_t);
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);

    if(VOS_StrLen(name) >= ONU_CONFIG_NAME_LEN)
        ret = VOS_ERROR;
    else if (p)
    {
        onuConfMacHashEntry_t *pd = (onuConfMacHashEntry_t*)p->data;
        VOS_MemSet(pd, 0, sizeof(onuConfMacHashEntry_t));
        VOS_StrCpy(pd->name, name);
        VOS_MemCpy(pd->mac, mac, 6);
        p->slotid = slot;
        p->ponid = 0;
        p->onuid = 0;
        p->msgtype = MSG_ONU_CONFSYND_MAC_REQ_CDP;
        VOS_MemCpy(p->data, pd, sizeof(onuConfMacHashEntry_t));

#if 1
        sendOnuConfSyndMsg(slot, p, len);
#else
        if(VOS_SemTake(getSemIdBySlotno(slot), VOS_TICK_SECOND*s_semTimeOutSeconds) == VOS_OK)
        {
            sendOnuConfSyndMsg(slot, p, len);
            if(VOS_SemTake(getSemIdBySlotno(slot), VOS_TICK_SECOND*s_semTimeOutSeconds) == VOS_OK)
            {
                VOS_SemGive(getSemIdBySlotno(slot));
                ret = VOS_OK;
            }
            else
            {
                cl_vty_console_out("\r\n------ get ack timeout ------\r\n");
                ret = VOS_ERROR;
            }
        }
        else
        {
            cl_vty_console_out("\r\n------ take semphere timeout ------\r\n");
            ret = VOS_ERROR;
        }
#endif

    }
    else
        ret = VOS_ERROR;

    return ret;
}*/

int sendOnuConfSyndReqAckMsg(int slot, int result)
{
    ULONG len = sizeof(ONUCONFSYNDMSG_T);
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
    if (p)
    {
        p->slotid = slot;
        p->ponid = 0;
        p->onuid = 0;
        p->msgtype = MSG_ONU_CONFSYND_REQ_ACK_CDP;
        p->data[0] = result;
        return sendOnuConfSyndMsg(SYS_MASTER_ACTIVE_SLOTNO, p, len);
    }
    else
        return VOS_ERROR;
}

int startOnuConfSyndRequestWitchDstSlot(int slotno)
{
    int i = 0;

    /*added by luh 2012-8-20增加透传命令行在热插拔时的恢复*/
    if(ONU_TRANSMISSION_ENABLE != OnuTransmission_flag)
        ResumeOnuTransmissionflagBySlot(slotno, OnuTransmission_flag);
    if(UpdatePendingQueueTime != 120)
        ResumeUpdatePendingPeriodBySlot(slotno, UpdatePendingQueueTime);
    if(g_gwonu_pty_flag)
        SetPrivatePtyEnable(g_gwonu_pty_flag);    
    if(!get_ctcOnuOtherVendorAccept())
	ResumeOtherCtcOnuRegistrationBySlot(slotno, 0);
	ResumeOnuModelBlackListBySlot(slotno);
    /*CTC Onu配置告警恢复*/
    sendOnuAlarmConfSyndDataBySlot(slotno);
    
#ifdef CDP_DEBUG
    BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
    cl_vty_console_out("\r\n------broadcasting onu conf files to slot %d------\r\n", slotno);
    END_ONUCONF_DEBUG_PRINT()
#endif
    /*for (j = 0; j < 1000; j++)*/
    {
        for (i = 0; i < g_maxPrimeNumber; i++)
        {
            ONU_CONF_SEM_TAKE
            {
                element_onuconfhashbucket_t *pe = (element_onuconfhashbucket_t*) g_onuConfBucket[i];

                while (pe)
                {
                    ONUConfigData_t * pd = (ONUConfigData_t *) pe->confdata;
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, slotno, 0, 0, pd->confname, NULL, NULL);                            
                    /*sendOnuConfSyndReqMsgByName(slotno, pd->confname);*/
                    pe = pe->next;
                }
            }
            ONU_CONF_SEM_GIVE

        }
        /*cl_vty_console_out("\r\n------%d bucket send complete!------\r\n", j+1);*/
    }

#ifdef CDP_DEBUG
    BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
    cl_vty_console_out("\r\n------broadcast onu conf files to slot %d done!------\r\n", slotno);
    END_ONUCONF_DEBUG_PRINT()
#endif
#if 1
    /*备用主控直接同步关联关系，各pon板通过文件同步*/
    if(SYS_MODULE_ISMASTERSTANDBY(slotno))
    {
        for(i=0; i<MAXPON; i++)
        {
            OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, slotno, i, 0, NULL, NULL, NULL);        
        }
    }
#if 0    
    else if(SYS_MODULE_IS_PON(slotno))
    {
        /*added by luh 2012-11-9;各Pon板只同步跟自己相关的关联关系*/
        for(i=1;i<MAXPONPORT_PER_BOARD;i++)
        {
            short int PonPortIdx = GetPonPortIdxBySlot(slotno, i);
            if(PonPortIdx==RERROR)
                continue;
            OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, slotno, PonPortIdx, 0, NULL, NULL, NULL);        
        }
    }
#endif    
#endif    
    return VOS_OK;
}
#endif

#if 1/*onu-profile:delete and sync*/
void deleteOnuConfFromHashBucket(const char *name)
{

    element_onuconfhashbucket_t *pp, *pc;

    if(!openOnuConfigFile(name, ONU_CONF_OPEN_FLAG_WRITE))
        return;

    /*无论何种条件都不允许删除onuconfdef文件*/
    if(!VOS_StrCmp(name, DEFAULT_ONU_CONF))
        return;
    
    if(name)
    {
        int idx = getOnuConfHashIndex(name);

        pp = (element_onuconfhashbucket_t *)g_onuConfBucket[idx];
        pc = pp;

        while (pc)
        {
            ONUConfigData_t *pd = pc->confdata;
            if (!VOS_StriCmp(pd->confname, name))
            {
                if ( pc == (element_onuconfhashbucket_t *)g_onuConfBucket[idx] )
                {
                    g_onuConfBucket[idx] = (int)pc->next;
                }
                else
                    pp->next = pc->next;
                
                onuconf_free(pc->confdata, ONU_CONF_MEM_DATA_ID);
                onuconf_free(pc, ONU_CONF_MEM_HASH_ID);
                /*added by luh 2012-10-26; 增加临时文件计数*/
                if(!VOS_StrnCmp(name, "*auto", 5)&&g_onu_temp_profile_num)
                    g_onu_temp_profile_num--;

                break;
            }
            pp = pc;
            pc = pc->next;
        }
    }
    else
        VOS_ASSERT(0);
    /*del by luh 2012-11-22, 问题单16378，删除配置文件无法save成功*/
    if(/*pc && */SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        /*ULONG uv[4] = {0, 0, name, EM_ONU_PROFILE_DELETED};*/
		ULONG uv[4] = {0,0,0,0};
        uv[2] = (ULONG)name;
        uv[3] = EM_ONU_PROFILE_DELETED;

        processEventCallbackList(EVENT_ONUPROFILE_DELETE, uv);

    }
}

int deleteOnuProfileSyncBroadcastByName(const char *name)
{
    int i,ret=0;
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for (i = 1; i <= SYS_CHASSIS_SLOTNUM; i++)
        {
            if (i != SYS_LOCAL_MODULE_SLOTNO 
                && (SYS_MODULE_SLOT_ISHAVECPU(i)||SYS_MODULE_ISMASTERSTANDBY(i)))
            {
                ret |= deleteOnuProfileSyncBySlot(i, name);
            }
        }
    }
    return ret ? VOS_ERROR : VOS_OK;
}

void startOnuConfSyndBroadcastDelMsg(const char *name)
{
    int i;
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return;
    
    for(i=1;i<=SYS_CHASSIS_SLOTNUM;i++)
    {
        sendOnuConfSyndDelReqMsg(i,  name);
    }
}
int sendOnuConfLocalDelReqMsg(const char *name)
{
    ULONG ulArgs[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_DEL_CDP, 0, 0};
    ULONG len = sizeof(ONUCONFSYNDMSG_T) + VOS_StrLen(name), queid = 0;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
        if (p)
        {
            p->slotid = 0;
            p->ponid = 0;
            p->onuid = 0;
            p->msgtype = MSG_ONU_CONFSYND_DEL_CDP;
            VOS_StrCpy((char*)p->data, name);
            ulArgs[2] = len;
            ulArgs[3] = (ULONG)p;
            queid = getQueIdBySlotno(SYS_LOCAL_MODULE_SLOTNO);
            return VOS_QueSend(queid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
        }
        else
        {
            return VOS_ERROR;
        }
    }
    else
        return VOS_OK;        
}
int sendOnuConfSyndDelReqMsg(int slot, const char *name)
{
    ULONG ulArgs[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_DEL_CDP, 0, 0};
    ULONG len = sizeof(ONUCONFSYNDMSG_T) + VOS_StrLen(name), queid = 0;

    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return VOS_OK;

    if (slot != SYS_LOCAL_MODULE_SLOTNO && 
    (SYS_MODULE_SLOT_ISHAVECPU(slot)||SYS_MODULE_ISMASTERSTANDBY(slot)))
    {
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
        if (p)
        {
            p->slotid = slot;
            p->ponid = 0;
            p->onuid = 0;
            p->msgtype = MSG_ONU_CONFSYND_DEL_CDP;
            VOS_StrCpy((char*)p->data, name);
#if 1
            /*return sendOnuConfSyndMsg(slot, p, len);*/
            ulArgs[2] = len;
            ulArgs[3] = (ULONG)p;
            queid = getQueIdBySlotno(slot);
            return VOS_QueSend(queid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
#else
            if(VOS_SemTake(getSemIdBySlotno(slot), VOS_TICK_SECOND*s_semTimeOutSeconds) == VOS_OK)
            {
                sendOnuConfSyndMsg(slot, p, len);
                if(VOS_SemTake(getSemIdBySlotno(slot), VOS_TICK_SECOND*s_semTimeOutSeconds) == VOS_OK)
                {
                    VOS_SemGive(getSemIdBySlotno(slot));
                    return VOS_OK;
                }
                else
                    return VOS_ERROR;
            }
            else
            	return VOS_ERROR;
#endif
        }
        else
            return VOS_ERROR;
    }
    else
        return VOS_OK;
}

int deleteOnuProfileSyncBySlot(short int slot, const char *name)
{
    int ret = VOS_OK;
    ONU_CONF_SEM_TAKE
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        ULONG len = sizeof(ONUCONFSYNDMSG_T)+VOS_StrLen(name);
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
        if(p)
        {
            p->slotid = slot;
            p->ponid = 0;
            p->onuid = 0;
            p->msgtype = MSG_ONU_CONFSYND_DEL_CDP;
            VOS_StrCpy((char*)p->data, name);
            ret = sendOnuConfSyndMsg(slot, p, len);
        }
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE;    

    return ret;
}
void deleteAllDisusedConfigFile()
{
    int i = 0;
    ONU_CONF_SEM_TAKE
    {
        for(i=0; i<g_maxPrimeNumber; i++)
        {
            element_onuconfhashbucket_t *pe = (element_onuconfhashbucket_t*) g_onuConfBucket[i];
            while (pe)
            {
                ONUConfigData_t * pd = (ONUConfigData_t *) pe->confdata;
                if(!onuconfHaveAssociatedOnu(pd->confname))
                {
                    OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, pd->confname, NULL, NULL);
                    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
                        OnuProfile_Action_ByCode(OnuProfile_Delete_SyncBroadcast, 0, 0, 0, pd->confname, NULL, NULL);
                }
                pe = pe->next;
            }
        }
    }
    ONU_CONF_SEM_GIVE
}
#endif

#if 1/*onu-profile:modify and sync*/
static int sfun_OnuConfRename(const char *filename, const char *rename)
{
    int ret = VOS_ERROR;
    /*本地重命名操作*/
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t * pd = getOnuConfFromHashBucket(filename);
        /*ONUConfigData_t * pnew = onuconf_malloc(ONU_CONF_MEM_DATA_ID);*/
        ONUConfigData_t * pnew = getOnuConfFromHashBucket(rename);

        if(pd && pnew)
        {
            int pon_id, offset, onuid, bitnum;

            ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("copy %s to %s\r\n", filename, rename));
            onuconfCopy(pnew, pd);

            ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("delete %s from hash table\r\n", pd->confname));
            OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, pd->confname, NULL, NULL);                            
            /*del by luh 2012-11-9; rename的配置文件是由临时文件覆盖源文件，无需修改关联关系，且其中间过程关联关系不发生变化*/
#if 0
            for(pon_id = 0; pon_id <SYS_MAX_PON_PORTNUM; pon_id++)
            {
                for(onuid=0; onuid<MAXONUPERPON; onuid++)
                {
                    offset = onuid/8;
                    bitnum = onuid&7;
                    if(pnew->onulist[pon_id][offset]&(1<<bitnum))
                    {
                        BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_GENERAL)
                                int slot = GetGlobalCardIdxByPonChip(pon_id);
                                int port = GetGlobalPonPortByPonChip(pon_id);
                        ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("apply profile %s to onu %d/%d/%d\r\n", pnew->confname, slot, port, onuid+1));
                        END_ONUCONF_DEBUG_PRINT()
                        /*onuconfAssociateOnuId(pon_id, onuid, pnew->confname);*/
                        ONU_CONF_SEM_TAKE
                        {
                            OnuProfile_Action_ByCode(OnuMap_Update, 0, pon_id, onuid, pnew->confname, NULL, NULL);
                        }
                        ONU_CONF_SEM_GIVE
                    }
                }
            }
#endif
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int onuconfCompare(ONUConfigData_t *pd, ONUConfigData_t *po)
{
    if(!pd || !po)
        return VOS_ERROR;
    else
    {
        char *ph = (char*)pd;
        char * pend = (char*)&pd->reserved;
        int headlen = pend-ph+1;
        int len = sizeof(ONUConfigData_t)-headlen;

        return VOS_MemCmp(&pd->reserved+1, &po->reserved+1, len);
    }
}

int onuconfCopy(ONUConfigData_t *pd, ONUConfigData_t *po)
{
    if(!pd || !po)
        return VOS_ERROR;
    else if(VOS_StrCmp(pd->confname, DEFAULT_ONU_CONF))
    {
        char *ph = (char*)pd;
        char * pend = (char*)&pd->reserved;
        int headlen = pend-ph+1;
        int len = sizeof(ONUConfigData_t)-headlen;
		/*modified by liyang 2014-08-14*/
        VOS_MemCpy(&pd->reserved+1, &po->reserved+1, len);

        if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
        {
            /*ULONG uv[4] = {0, 0, pd->confname, EM_ONU_PROFILE_MODIFIED};*/
			ULONG uv[4] = {0,0,0,0};
	        uv[2] = (ULONG)pd->confname;
	        uv[3] = EM_ONU_PROFILE_MODIFIED;
            processEventCallbackList(EVENT_ONUPROFILE_MODIFIED, uv);
        }        
    }

	return VOS_OK;
}

int parseOnuPrivateConfigByOnuId(ULONG slot, ULONG port, ULONG onuid, ONUConfigData_t *pOld, ONUConfigData_t *pNew)
{
    int ret = VOS_ERROR;
    if(pOld && pNew)
    {
        char *ph = (char*)pNew;
        char * pend = (char*)&pNew->reserved;
        int headlen = pend-ph+1;
        int len = sizeof(ONUConfigData_t)-headlen;

        /*step 1、拷贝原来的数据结构*/
        VOS_MemCpy(&pNew->reserved+1, &pOld->reserved+1, len);

        /*step 2、如果原profile中存在批量规则，则需要根据onuid解析为具体适用的vlan数据*/
        if(pOld->rules.number_rules)
        {
            ULONG begin_slot,begin_port,begin_onuid;
            ULONG end_slot,end_port,end_onuid;  

            if(slot&&port&&onuid)
            {
                ONUVlanConf_t temp_vlan;
                int i = 0, j = 0, k =0, m = 0;
                ULONG fenum, devidx = 0,v_portlist = 0;
                int onu_num;
                short int v_vid;
                short int olt_id = GetPonPortIdxBySlot(slot, port);

                VOS_MemZero(&temp_vlan,sizeof(ONUVlanConf_t));
                devidx = MAKEDEVID(slot, port, onuid);
                if(getDeviceCapEthPortNum(devidx, &fenum) == VOS_OK)
                {
                    /*不在线的onu，按最大端口进行配置*/
                    if(fenum == 0) 
                        fenum = ONU_MAX_PORT;
                }
                if(pOld->rules.number_rules)
                {    
                    for(k=0;k<fenum;k++)
                        v_portlist |= (1<<k);                    
                    for(i=0;i<pOld->rules.number_rules;i++)
                    {
                        if(pOld->rules.rule[i].rule_mode)
                        {
                            begin_slot = GET_PONSLOT(pOld->rules.rule[i].begin_onuid);
                            begin_port = GET_PONPORT(pOld->rules.rule[i].begin_onuid);
                            begin_onuid = GET_ONUID(pOld->rules.rule[i].begin_onuid);
                            end_slot = GET_PONSLOT(pOld->rules.rule[i].end_onuid);
                            end_port = GET_PONPORT(pOld->rules.rule[i].end_onuid);
                            end_onuid = GET_ONUID(pOld->rules.rule[i].end_onuid);
                            
                            /*检查onu是否在范围之内*/
                            onu_num = Get_OnuNumFromOnuRange(slot,port,onuid,end_slot,end_port,end_onuid);
                            if(onu_num == VOS_ERROR)
                                continue;
                            onu_num = Get_OnuNumFromOnuRange(begin_slot,begin_port,begin_onuid,slot,port,onuid);
                            if(onu_num == VOS_ERROR)
                                continue;
                                                        
                            v_vid = pOld->rules.rule[i].begin_vid + (onu_num - 1)*pOld->rules.rule[i].step;
                            if(pOld->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID)
                            {
                                for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                {
                                    if(temp_vlan.entryarry[j].vlanid == pOld->rules.rule[i].begin_vid)
                                		break;
                                }
                                if(j < temp_vlan.onuVlanEntryNum)
                                {                                    
                                    if(pOld->rules.rule[i].mode == 2)
                                    {
                                        temp_vlan.entryarry[j].untagPortMask |= v_portlist;  
                                    }
                                    temp_vlan.entryarry[j].allPortMask |= v_portlist;
                                }
                                else
                                {
                                    k= temp_vlan.onuVlanEntryNum;
                                    temp_vlan.entryarry[k].vlanid = pOld->rules.rule[i].begin_vid;
                                    if(pOld->rules.rule[i].mode == 2)
                                    {
                                        temp_vlan.entryarry[j].untagPortMask |= v_portlist;  
                                    }
                                    temp_vlan.entryarry[j].allPortMask |= v_portlist;
                                    temp_vlan.onuVlanEntryNum++;
                                }
                            }
                            else if(pOld->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID_PORT)
                            {
                                for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                {
                                    if(temp_vlan.entryarry[j].vlanid == pOld->rules.rule[i].begin_vid)
                                		break;
                                }
                                if(j < temp_vlan.onuVlanEntryNum)
                                {
                                    if(pOld->rules.rule[i].mode == 2)
                                    {                                        
                                        temp_vlan.entryarry[j].untagPortMask |= pOld->rules.rule[i].portlist & v_portlist;                                                                            
                                    }
                                    temp_vlan.entryarry[j].allPortMask |= pOld->rules.rule[i].portlist & v_portlist;
                                }
                                else
                                {
                                    k= temp_vlan.onuVlanEntryNum;
                                    temp_vlan.entryarry[k].vlanid = pOld->rules.rule[i].begin_vid;
                                    if(pOld->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[k].untagPortMask |= pOld->rules.rule[i].portlist & v_portlist;     
                                    temp_vlan.entryarry[k].allPortMask |= pOld->rules.rule[i].portlist & v_portlist;
                                    temp_vlan.onuVlanEntryNum++;
                                }
                            }
                            else if(pOld->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE)
                            {
                                for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                {
                                    if(temp_vlan.entryarry[j].vlanid == v_vid)
                                		break;
                                }
                                if(j < temp_vlan.onuVlanEntryNum)
                                {
                                    if(pOld->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[j].untagPortMask |= v_portlist;                                                                            
                                    temp_vlan.entryarry[j].allPortMask |= v_portlist;
                                }
                                else
                                {
                                    k= temp_vlan.onuVlanEntryNum;
                                    temp_vlan.entryarry[k].vlanid = v_vid;
                                    if(pOld->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[k].untagPortMask |= v_portlist;     
                                    temp_vlan.entryarry[k].allPortMask |= v_portlist;
                                    temp_vlan.onuVlanEntryNum++;
                                }
                            }
                            else if(pOld->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_PORT)
                            {
                                int t_vid = v_vid;
                                for(m=0;m<fenum;m++)
                                {
                                    if(t_vid >= (v_vid + pOld->rules.rule[i].step))
                                        t_vid = v_vid;
                                    
                                    for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                    {
                                        if(temp_vlan.entryarry[j].vlanid == t_vid)
                                    		break;
                                    }
                                    if(j < temp_vlan.onuVlanEntryNum)
                                    {
                                        if(pOld->rules.rule[i].mode == 2)
                                            temp_vlan.entryarry[j].untagPortMask |= 1<<m;                                                                            
                                        temp_vlan.entryarry[j].allPortMask |= 1<<m;
                                    }
                                    else
                                    {
                                        k= temp_vlan.onuVlanEntryNum;
                                        temp_vlan.entryarry[k].vlanid = t_vid;
                                        if(pOld->rules.rule[i].mode == 2)
                                            temp_vlan.entryarry[k].untagPortMask |= 1<<m;     
                                        temp_vlan.entryarry[k].allPortMask |= 1<<m;
                                        temp_vlan.onuVlanEntryNum++;
                                    }  
                                    t_vid += pOld->rules.rule[i].port_step;                 
                                }
                            }
                            else if(pOld->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_ONEPORT)
                            {
                                for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                {
                                    if(temp_vlan.entryarry[j].vlanid == v_vid)
                                		break;
                                }
                                if(j < temp_vlan.onuVlanEntryNum)
                                {
                                    if(pOld->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[j].untagPortMask |= pOld->rules.rule[i].portlist;                                                                            
                                    temp_vlan.entryarry[j].allPortMask |= pOld->rules.rule[i].portlist;
                                }
                                else
                                {
                                    k= temp_vlan.onuVlanEntryNum;
                                    temp_vlan.entryarry[k].vlanid = v_vid;
                                    if(pOld->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[k].untagPortMask |= pOld->rules.rule[i].portlist;     
                                    temp_vlan.entryarry[k].allPortMask |= pOld->rules.rule[i].portlist;
                                    temp_vlan.onuVlanEntryNum++;
                                }  
                            }
                        }
                    }
                }
                
                if(temp_vlan.onuVlanEntryNum)
                {
                    for(i=0;i<temp_vlan.onuVlanEntryNum;i++)
                    {
                        int find = 0;
                        int MaxVlanNum = pNew->vlanconf.onuVlanEntryNum;
                        for(j=0;j<MaxVlanNum;j++)
                        {
                            if(temp_vlan.entryarry[i].vlanid == pNew->vlanconf.entryarry[j].vlanid)
                            {
                                find = 1;
                                pNew->vlanconf.entryarry[j].allPortMask |= temp_vlan.entryarry[i].allPortMask;
                                pNew->vlanconf.entryarry[j].untagPortMask |= temp_vlan.entryarry[i].untagPortMask;

                                /*added by luh@2015-07-29, 需要清除default vlan中的untag端口*/
                                pNew->vlanconf.entryarry[0].untagPortMask &= ~(temp_vlan.entryarry[i].untagPortMask);
                                pNew->vlanconf.entryarry[0].allPortMask &= ~(temp_vlan.entryarry[i].untagPortMask);
                                
                                /*设置端口的pvid*/                                
                                if(temp_vlan.entryarry[i].untagPortMask)
                                {
#if 0                                
                                    for(k=0;k<fenum;k++)
                                    {
                                        if(temp_vlan.entryarry[i].untagPortMask&(1<<k))
                                        {
                                            if(pNew->portconf[k].defaultVid&0xfff == 1)
                                            {
                                                pNew->portconf[k].defaultVid &= 0xfffff000;
                                                pNew->portconf[k].defaultVid |= temp_vlan.entryarry[i].vlanid;
                                            }
                                        }
                                    }
#else                                    
                                    ULONG portmask = temp_vlan.entryarry[i].untagPortMask;
                                    k = 0;
                                    while(portmask)
                                    {
                                        if(portmask&1)
                                        {
                                            pNew->portconf[k].defaultVid &= 0xfffff000;
                                            pNew->portconf[k].defaultVid |= temp_vlan.entryarry[i].vlanid;
                                        }
                                        portmask >>= 1;
                                        k++;
                                    }
#endif
                                }
                            }                            
                        }
                        if(!find)
                        {
                            pNew->vlanconf.entryarry[j].vlanid |= temp_vlan.entryarry[i].vlanid;
                            pNew->vlanconf.entryarry[j].allPortMask |= temp_vlan.entryarry[i].allPortMask;
                            pNew->vlanconf.entryarry[j].untagPortMask |= temp_vlan.entryarry[i].untagPortMask;

                            /*added by luh@2015-07-29, 需要清除default vlan中的untag端口*/
                            pNew->vlanconf.entryarry[0].untagPortMask &= ~(temp_vlan.entryarry[i].untagPortMask);
                            pNew->vlanconf.entryarry[0].allPortMask &= ~(temp_vlan.entryarry[i].untagPortMask);
                            
                            /*设置端口的pvid*/
                            if(temp_vlan.entryarry[i].untagPortMask)
                            {
#if 0                            
                                for(k=0;k<fenum;k++)
                                {
                                    if(temp_vlan.entryarry[i].untagPortMask&(1<<k))
                                    {
                                        if(pNew->portconf[k].defaultVid&0xfff == 1)
                                        {
                                            pNew->portconf[k].defaultVid &= 0xfffff000;
                                            pNew->portconf[k].defaultVid |= temp_vlan.entryarry[i].vlanid;
                                        }
                                    }
                                }
#else
                                ULONG portmask = temp_vlan.entryarry[i].untagPortMask;
                                k = 0;
                                while(portmask)
                                {
                                    if(portmask&1)
                                    {
                                        pNew->portconf[k].defaultVid &= 0xfffff000;
                                        pNew->portconf[k].defaultVid |= temp_vlan.entryarry[i].vlanid;
                                    }
                                    portmask >>= 1;
                                    k++;
                                }
#endif
                            }                        
                            pNew->vlanconf.onuVlanEntryNum++;
                        }
                    }
                }
                    
            }
            /*step 3、解析完规则后，需要清除掉原来的规则*/
            VOS_MemZero(&pNew->rules, sizeof(onu_vlan_bat_rules_t));
            
        }
        ret = VOS_OK;
    }
    return ret;
}

int onu_profile_rename(const char *filename, const char *rename)
{

    int ret = VOS_ERROR;
    /*modified by luh 2012-11-22，解决问题单16381*/
#if 0    
    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER && !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
#else
    /*6900主控板和各种备用主控可以直接进行配置文件的更新*/
    /*而各种pon板及61、67当前主控、69m各槽位、69s需要等所有*/
    /*Onu执行完undo和do操作之后才能进行配置文件的更新和临时文件的删除2013-1-23*/
    if(SYS_LOCAL_MODULE_ISMASTERSTANDBY || SYS_LOCAL_MODULE_TYPE_IS_6900_SW || SYS_LOCAL_MODULE_TYPE_IS_8000_SW)
#endif
    {
    	ret = sfun_OnuConfRename(filename, rename);
    }

    /*else*/ /*非主控板*/
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && !SYS_LOCAL_MODULE_ISMASTERSTANDBY)
    {

        short int pon_id, onu_id;
		int slot_swap = 0, port_swap = 0;
    
        /*缓存重命名关系 filename -- rename*/
        addToRenameList(filename, rename);

		ONU_CONF_SEM_TAKE
        {
			if(!onuconfHaveAssociatedOnu(rename))
			{
				sfun_OnuConfRename( filename, rename);
				delFromRenameList(filename);
				ret = VOS_OK;
			}
			else
			{
				ONUConfigData_t *pd = getOnuConfFromHashBucket(filename);
                int lvalid_onuNum = 0;/*恢复的onu数目*/
	        	/*关联被重命名的配置文件*/
		        if(pd)
		        {
		            for(pon_id=0; pon_id<MAXPON; pon_id++)
		            {
		                int slot = GetCardIdxByPonChip(pon_id);
		                int port = GetGlobalPonPortByPonChip(pon_id);
                        /*modified by luh 2013-08-20, for 6100在线修改配置文件在3槽位不能正确回复配置*/
#if 0                        
		                if((SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER && slot != SYS_LOCAL_MODULE_SLOTNO) || (SlotCardIsPonBoard(slot) != VOS_OK))
#else
		                if((SYS_LOCAL_MODULE_TYPE_IS_CPU_PON && slot != SYS_LOCAL_MODULE_SLOTNO) || (SlotCardIsPonBoard(slot) != VOS_OK))
#endif
		                {
		                    ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("slot %d is illegal!\r\n", slot));
							continue;
		                }
						else
		                {
							/* added by wangjiah@2017-03-23:begin
							 * Only undo/do association ACTIVE pon port in assinner-poncard Protection
							 * in case of deleting *auto config file when undo/do association PASSIVE pon port
							 * */
                            short int ponportidx = GetPonPortIdxBySlot(slot, port);
							if( SYS_LOCAL_MODULE_SLOTNO == slot && VOS_OK == PonPortSwapSlotQuery(ponportidx, &slot_swap, &port_swap) )
							{
								if( slot == slot_swap && V2R1_PON_PORT_SWAP_PASSIVE == PonPortHotStatusQuery(ponportidx) )
								{
									continue;
								}
							}
							/* added by wangjiah@2017-03-23:end */
							
							ponportidx = onuConfGetPonIdFromPonProtectGroup(ponportidx);
                            char *buf = pd->onulist[ponportidx];
                            int num = MAXONUPERPON/8+1;
                            int i = 0;
                            for(i=0; i<num; i++)
                            {
                                if(*buf)
                                {
                                    int j=0;
                                    for(j=0; j<8; j++)
                                    {
                                        if((*buf)&(1<<j))
                                        {
                                            onu_id = i*8+j;
                                            lvalid_onuNum ++;
                                            ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("%d:%d add to wait queue begin!\r\n", pon_id, onu_id));
                                            OnuProfile_Action_ByCode(OnuAction_Undo_AssociateAndSync, 0, pon_id, onu_id, rename, NULL, NULL);
                                            OnuProfile_Action_ByCode(OnuAction_Do_AssociateAndSync, 0, pon_id, onu_id, filename, NULL, NULL);
                                            /*恢复新的配置文件*/
                                            ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("%d:%d add to wait queue end!\r\n", pon_id, onu_id));
                                        }
                                    }
                                }
                                buf++;
                            }
		                }
		            }
		            ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("onus add to wait queue complete!\r\n"));
					ret = VOS_OK;
	        	}
				else
				{
					delFromRenameList(filename);
				}
                /*6900M的1槽位既是主控又是pon的管理者，可能没有关联Onu但是也执行到*/
                /*这里，所以需要在这里再进行过滤，保证配置文件一致.added by luh 2013-1-23*/
                if(!lvalid_onuNum)
    			{
    				sfun_OnuConfRename( filename, rename);
    				delFromRenameList(filename);
    				ret = VOS_OK;
    			}
			}
		}
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int sendOnuConfSyncBroadcastRenameMsg(const char *filename, const char *rename)
{
    int ret = VOS_ERROR;
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE/* && !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER*/)
    {
        /*向从板发送重命名指令*/
        int slot;
        for(slot=1; slot <= SYS_CHASSIS_SLOTNUM; slot++)
        {
            /*if( slot != SYS_LOCAL_MODULE_SLOTNO && SYS_MODULE_SLOT_ISHAVECPU(slot))*/
                ret = sendOnuConfSyncRenameMsg(slot, filename, rename);
        }
    }    

    return ret;
}
int sendOnuConfSyncUnicastRenameMsg(short int slot, const char *filename, const char *rename)
{
    int ret = VOS_ERROR;
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE/* && !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER*/)
    {
        /*向从板发送重命名指令*/
        if( slot != SYS_LOCAL_MODULE_SLOTNO && SYS_MODULE_SLOT_ISHAVECPU(slot))
            ret = sendOnuConfSyncRenameMsg(slot, filename, rename);
    }    

    return ret;
}
int sendOnuConfSyncRenameMsg(ULONG slot, const char * filename, const char *rename)
{
    int len = 2*ONU_CONFIG_NAME_LEN;
    ULONG slen = sizeof(ONUCONFSYNDMSG_T)+len;
    
    if (slot != SYS_LOCAL_MODULE_SLOTNO && 
    (SYS_MODULE_SLOT_ISHAVECPU(slot)||SYS_MODULE_ISMASTERSTANDBY(slot)))
    {
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(slen, MODULE_RPU_CTC);
        if (p)
        {
            ULONG queid = 0;
            ULONG ulArgs[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_RENAME_REQ_CDP, 0, 0};

            p->slotid = slot;
            p->msgtype = MSG_ONU_CONFSYND_RENAME_REQ_CDP;
            VOS_MemCpy(p->data, filename, ONU_CONFIG_NAME_LEN);
            VOS_MemCpy(((char*)p->data)+ONU_CONFIG_NAME_LEN, rename, ONU_CONFIG_NAME_LEN);

            queid = getQueIdBySlotno(slot);
            ulArgs[2] = slen;
            ulArgs[3] = (ULONG)p;

            return VOS_QueSend(queid, ulArgs,WAIT_FOREVER, MSG_PRI_NORMAL);

        }
        else
            return VOS_ERROR;
    }
}

#endif

#if 1/*onu-profile:search by profile name*/
ONUConfigData_t *
getOnuConfFromHashBucket(const char *name)
{
    int idx ;
    element_onuconfhashbucket_t *p = NULL;

    if(!g_onuConfBucket)
        initOnuConfHashBucket(HASH_BUCKET_DEPTH);

    if(!name)
        return NULL;

    idx = getOnuConfHashIndex(name);

     p = (element_onuconfhashbucket_t*) (*(g_onuConfBucket + idx));

    while (p)
    {
        ONUConfigData_t * pd = (ONUConfigData_t *) p->confdata;
        if (pd && (!VOS_StriCmp(pd->confname, name)))
            return pd;
        p = p->next;
    }

    return NULL;
}

void *
getOnuConfHashBucketElement(const char *name, void **head)
{
    int idx = getOnuConfHashIndex(name);

    element_onuconfhashbucket_t *p =
            (element_onuconfhashbucket_t*) (*(g_onuConfBucket + idx));
    *head = g_onuConfBucket + idx;

    while (p)
    {
        ONUConfigData_t * pd = (ONUConfigData_t *) p->confdata;
        if (pd && (!VOS_StriCmp(pd->confname, name)))
            return p;
        p = p->next;
    }

    return NULL;
}


void *
getOnuConfMacHashBucketElement(const char *mac, void **head)
{
    int idx = getOnuConfHashIndex(mac);

    element_onuconfhashbucket_t *p =
            (element_onuconfhashbucket_t*) (*(g_onuConfMacBucket + idx));
    *head = g_onuConfMacBucket + idx;

    while (p)
    {
        onuConfMacHashEntry_t *pd = p->confdata;
        if ((!VOS_MemCmp(pd->mac, mac, 6)))
            return p;
        p = p->next;
    }

    return NULL;
}
char *
getOnuConfMacFromHashBucket(const char *mac)
{
    int * head ;
    element_onuconfhashbucket_t *p = NULL;

    if(!g_onuConfMacBucket)
        initOnuConfHashBucket(HASH_BUCKET_DEPTH);

    p = getOnuConfMacHashBucketElement(mac, (void**)&head);
    if(p)
        return ((onuConfMacHashEntry_t*)p->confdata)->name;

    return NULL;
}
#endif

#if 1/*onu-profile:关联关系变更*/
static int sfun_setOnuConfOnuIdMapByPonId(short int ponid, short int onuid, const char *name)
{

    int ret = VOS_ERROR;

    OLT_ID_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if (ponid != VOS_ERROR && onuid != VOS_ERROR)
    {

        VOS_StrCpy(g_onuConfOnuIdMapArry[ponid][onuid].name, name);
		ONUCONF_ONU_LIMIT_DEBUG("\r\n set onu(%d/%d) conf onuid map: %s\r\n", ponid, onuid,g_onuConfOnuIdMapArry[ponid][onuid].name);
        ret = VOS_OK;

    }

    return ret;

}
static int sfun_setOnuConfOnuIdMapByDevIdx(short int slot, short int pon, short int onu, const char *name)
{

    short int ponid = GetGlobalPonPortIdxBySlot(slot, pon);
    short int onuid = onu-1;

    return sfun_setOnuConfOnuIdMapByPonId(ponid, onuid, name);

}

static int sfun_setOnuConfNameMapByPonId(const char *name, short int ponid, short int onuid)
{
    int ret = VOS_ERROR;

    OLT_ID_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if (ponid != VOS_ERROR && onuid != VOS_ERROR)
    {

        ONUConfigData_t *pd = getOnuConfFromHashBucket(name);

        int offset = onuid / 8;
        int bitnum = onuid % 8;

        if (pd)
        {
            pd->onulist[ponid][offset] |= 1 << bitnum;
            ret = VOS_OK;
        }
    }
#if 0
    if(ret == VOS_OK && SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        ULONG uv[4] = {0, 0, 0, EM_ONU_PROFILE_MODIFIED};
        uv[2] = name;
        processEventCallbackList(EVENT_ONUPROFILE_MODIFIED, uv);
    }
#endif    
    return ret;
}

static int sfun_setOnuConfNameMapByDevIdx(const char *name, int slot, int pon, int onu)
{
    short int ponid = GetGlobalPonPortIdxBySlot(slot, pon);
    short int onuid = onu-1;

    return sfun_setOnuConfNameMapByPonId(name, ponid, onuid);
}

static int sfun_clrOnuConfNameMapByPonId(const char *name, short int ponid, short int onuid)
{
    int ret = VOS_ERROR;

    OLT_ID_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if (ponid != VOS_ERROR && onuid != VOS_ERROR)
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(name);

        int offset = onuid / 8;
        int bitnum = onuid % 8;

        if (pd)
        {
            pd->onulist[ponid][offset] &= ~(1 << bitnum);
            ret = VOS_OK;
        }
    }
#if 0
    if(ret == VOS_OK)
    {
        ULONG uv[4] = {0, 0, 0, EM_ONU_PROFILE_MODIFIED};
        uv[2] = name;
        processEventCallbackList(EVENT_ONUPROFILE_MODIFIED, uv);
    }
#endif
    return ret;
}

static int sfun_clrOnuConfNameMapByDevIdx(const char *name, int slot, int pon, int onu)
{
    short int ponid = GetGlobalPonPortIdxBySlot(slot, pon);
    short int onuid = onu-1;

    return sfun_clrOnuConfNameMapByPonId(name, ponid, onuid);
}

int OnuProfile_Set_NameMapBySlot(const char *name, int slot, int port, int onu)
{
    short int ponid = GetGlobalPonPortIdxBySlot(slot, port);
    short int onuid = onu-1;
    
    if(VOS_MemCmp(g_onuConfOnuIdMapArry[ponid][onuid].name, name, ONU_CONFIG_NAME_LEN) != 0)
    {
        sfun_clrOnuConfNameMapByPonId(g_onuConfOnuIdMapArry[ponid][onuid].name, ponid, onuid);
        sfun_setOnuConfOnuIdMapByPonId(ponid, onuid, name);
    }
    sfun_setOnuConfNameMapByPonId(name, ponid, onuid);
}

int OnuProfile_Set_NameMapByPonid(const char *name, short int ponid, short int onuid)
{
    short int slot = GetGlobalCardIdxByPonChip(ponid);
    short int port = GetGlobalPonPortByPonChip(ponid);
    short int local_ponid = GetPonPortIdxBySlot(slot, port);

    int undo_flag = 0;
    int ret = 0;
    char undo_name[ONU_CONFIG_NAME_LEN]={0};
    /*modified by luh 2012-11-22, 增加对保护pon口的判断，选小口进行映射关系的修改*/
    short int lglobal_ponid = onuConfGetPonIdFromPonProtectGroup(local_ponid);
    if(lglobal_ponid == RERROR)
        return VOS_ERROR;

	/*modi by wangjiah@2017-03-08: begin
	** call sfun_setOnuConfNameMapByPonId after sfun_setOnuConfOnuIdMapByPonId
	** to keep onu config-file bitmap consistent with g_onuConfOnuIdMapArray*/
	ONUCONF_ONU_LIMIT_DEBUG("\r\n local_ponid : %d / %s; lglobal_ponid: %d / %s ; name :%s\r\n", local_ponid, g_onuConfOnuIdMapArry[local_ponid][onuid].name, lglobal_ponid, g_onuConfOnuIdMapArry[lglobal_ponid][onuid].name, name);
    
    if(VOS_StrnCmp(g_onuConfOnuIdMapArry[lglobal_ponid][onuid].name, name, ONU_CONFIG_NAME_LEN) != 0)
    {
        undo_flag = 1;
        VOS_MemCpy(undo_name, g_onuConfOnuIdMapArry[lglobal_ponid][onuid].name, ONU_CONFIG_NAME_LEN);
        sfun_clrOnuConfNameMapByPonId(g_onuConfOnuIdMapArry[lglobal_ponid][onuid].name, lglobal_ponid, onuid);
        sfun_setOnuConfOnuIdMapByPonId(lglobal_ponid, onuid, name);
		ONUCONF_ONU_LIMIT_DEBUG("\r\nglobal:clrOnuConfNameMapByPonId(%s,%d,%d)\r\n", undo_name, lglobal_ponid, onuid);
		ret = sfun_setOnuConfNameMapByPonId(name, lglobal_ponid, onuid);
		ONUCONF_ONU_LIMIT_DEBUG("\r\nsetOnuConfNameMapByPonId(%s,%d,%d)\r\n", name, lglobal_ponid, onuid);
    }
    if(lglobal_ponid != local_ponid)
    {
        if(VOS_StrnCmp(g_onuConfOnuIdMapArry[local_ponid][onuid].name, DEFAULT_ONU_CONF, ONU_CONFIG_NAME_LEN) != 0) 
        {
            VOS_MemCpy(undo_name, g_onuConfOnuIdMapArry[local_ponid][onuid].name, ONU_CONFIG_NAME_LEN);
            sfun_clrOnuConfNameMapByPonId(g_onuConfOnuIdMapArry[local_ponid][onuid].name, local_ponid, onuid);
            sfun_setOnuConfOnuIdMapByPonId(local_ponid, onuid, DEFAULT_ONU_CONF);        
			ONUCONF_ONU_LIMIT_DEBUG("\r\nlocal:clrOnuConfNameMapByPonId(%s,%d,%d)\r\n", undo_name, local_ponid, onuid);
			ret = sfun_setOnuConfNameMapByPonId(DEFAULT_ONU_CONF, local_ponid, onuid);
			ONUCONF_ONU_LIMIT_DEBUG(onuid, ("\r\nlocal:setOnuConfNameMapByPonId(%s,%d,%d)\r\n", name, local_ponid, onuid));
        }
    }
	/*modi by wangjiah@2017-03-08: end*/
    


    if(ret == VOS_OK)
    {
        ULONG uv[4] = {0, 0, 0, EM_ONU_PROFILE_MODIFIED};
        uv[2] = (ULONG)name;
        processEventCallbackList(EVENT_ONUPROFILE_MODIFIED, uv);
    }
    
}

int OnuProfile_Clear_NameMapByName(const char *name)
{
    int ret = VOS_OK;
    if(name)
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(name);
            if(pd)
            {
                int i,j;

                for(i=0; i<SYS_MAX_PON_PORTNUM; i++)
                {
                    for(j=0; j<MAXONUPERPON; j++)
                    {
                        int offset = j/8;
                        int bitnum = j%8;
                        if(pd->onulist[i][offset]&(1<<bitnum))
                        {
                            OnuProfile_Set_NameMapByPonid(i, j, DEFAULT_ONU_CONF);
                        }
                    }
                }
            }
            else
                ret = VOS_ERROR;
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

char *OnuProfile_Get_ConfNameByOnuId(short int ponid, short int onuid)
{
    if(ponid == -1 || onuid == -1)
    {
        VOS_ASSERT(0);
        return NULL;
    }
    else
    {
        int pon_id = onuConfGetPonIdFromPonProtectGroup(ponid);
        if(pon_id < SYS_MAX_PON_PORTNUM && pon_id >= 0)
            return g_onuConfOnuIdMapArry[pon_id][onuid].name;
        else
        {
			VOS_ASSERT(0);
            return NULL;
        }
    }
}
int updateAllOnuMapSyncBroadcastByPonId(short int PonPortidx)
{
    int i, ret=0;
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for (i = 1; i <= SYS_CHASSIS_SLOTNUM; i++)
        {
            ret |= sendOnuConfSyndOnuIdMapReqMsg(i, PonPortidx);
        }
    }
    return ret ? VOS_ERROR : VOS_OK;
}
int sendOnuConfSyndOnuIdMapReqMsg(ULONG dstSlot, ULONG ponid)
{
    int ret = VOS_OK;
	ULONG ponPortIdx = 0;

    if(ponid == -1 )
    {
        ret = VOS_ERROR;
        return ret;
    }
	ponPortIdx = onuConfGetPonIdFromPonProtectGroup(ponid); 
	if(ponPortIdx == ERROR)
	{
		return VOS_ERROR;
	}
    
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return VOS_OK;

    ONU_CONF_SEM_TAKE
    if (dstSlot != SYS_LOCAL_MODULE_SLOTNO 
        && SYS_MODULE_SLOT_ISHAVECPU(dstSlot))
    {

        int slot = GetCardIdxByPonChip(ponid);
        int pon = GetPonPortByPonChip(ponid);

        int length = MAXONUPERPON * sizeof(onuConfOnuIdMapEntry_t);

        ULONG len = sizeof(ONUCONFSYNDMSG_T) + length;
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);

        if (p)
        {
            p->slotid = slot;
            p->ponid = pon;
            p->onuid = 0;
            p->msgtype = MSG_ONU_CONFSYND_ONUID_MAP_REQ_CDP;
            VOS_MemCpy(p->data, &g_onuConfOnuIdMapArry[ponPortIdx][0], length);

            ret = sendOnuConfSyndMsg(dstSlot, p, len);

        }
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE;

    return ret;
}

#endif

#if 1/*onu-profile:association-do or undo(based on onuIdx or profile name)*/
int onu_profile_associate_by_index(struct vty* vty, ULONG slot, ULONG port, ULONG onuid,const char *filename)
{
    LONG    lRet = 0;
    ULONG  onuEntry = 0, swap_slot = 0, swap_port = 0;
    short int ponPortIdx = 0;
	short int activePonPortIdx = 0;
	short int tmp = 0;
    short int OnuIdx = 0;
	short int swap_ponid = 0;
    short int to_slot= device_standby_master_slotno_get();
    char *name = NULL;
	char isPonPortSwap = FALSE;

    ponPortIdx = GetPonPortIdxBySlot((short int)slot,(short int) port);
    OnuIdx = onuid-1;
    onuEntry = ponPortIdx*MAXONUPERPON+OnuIdx;

    if(!isOnuConfExist(filename))
    {
        vty_out(vty, "\r\nfile %s doesn't exist! please check the file name you entered!\r\n", filename);
        return CMD_WARNING;
    }

	name = ONU_CONF_NAME_PTR_GET(ponPortIdx, OnuIdx);
	if(name && (!VOS_StrCmp(name, filename)))
		return CMD_SUCCESS;
    /*2013-3-28,私有配置文件只能关联一个onu，且此onu id必须与配置文件匹配*/
    if(!onuConfIsSharedByName(filename))
    {
        char sz[80]="";
        VOS_Sprintf(sz, "onu%d/%d/%d", slot, port, onuid);
        
        if(onuconfHaveAssociatedOnu(filename))
        {
            vty_out(vty, "\r\nassociation with onu %d/%d/%d fail!\r\nthe private config file has been associated with one ONU!\r\n", slot, port, onuid);
            return CMD_WARNING;
        }
        if(VOS_StrCmp(filename, sz) != 0)
        {
            vty_out(vty, "\r\nassociation with onu %d/%d/%d fail!\r\nthe private config file must be associated with the ONU it related to!\r\n", slot, port, onuid);
            return CMD_WARNING;
        }
    }
	/* added by wangjiahe@2017-03-17
	** associate onuconf with active pon first,
	** then sync to passive pon */
	if(PonPortAutoProtectPortQuery(slot, port, &swap_slot, &swap_port) == VOS_OK)
	{
		swap_ponid = GetPonPortIdxBySlot(swap_slot, swap_port);
		if(swap_ponid != RERROR)
		{
			isPonPortSwap = TRUE;
			activePonPortIdx = (PonPortHotStatusQuery(ponPortIdx) == V2R1_PON_PORT_SWAP_ACTIVE) ? ponPortIdx : swap_ponid;
			if(activePonPortIdx != ponPortIdx)
			{
				activePonPortIdx = swap_ponid;
				swap_ponid = ponPortIdx;
				ponPortIdx = activePonPortIdx;

				tmp = swap_slot;
				swap_slot = slot;
				slot = tmp;

				tmp = swap_port;
				swap_port = port;
				port = tmp;
			}
		}
	}
	/*if(SYS_LOCAL_MODULE_ISMASTERACTIVE && (!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER))
	  addOnuProfileSyncBroadcastByName(filename);*/
	/*先执行原关联文件的去配置操作*/
	OnuProfile_Action_ByCode(OnuAction_Undo_AssociateAndSync, 0, ponPortIdx, OnuIdx, NULL, NULL, NULL);
	/*关联新的配置文件或重新关联文件*/
	OnuProfile_Action_ByCode(OnuMap_Update, 0, ponPortIdx, OnuIdx, filename, NULL, NULL);
	OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, slot, ponPortIdx, 0, NULL, NULL, NULL);        
	if(to_slot)
		OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, ponPortIdx, 0, NULL, NULL, NULL);        

	/*恢复新的配置文件*/
	OnuProfile_Action_ByCode(OnuAction_Do_AssociateAndSync, 0, ponPortIdx, OnuIdx, NULL, NULL, NULL);

	/*
	 * added by wangxiaoyu 2011-08-29
	 * PON端口保护时，两个PON端口上相同ONU ID的配置关联同时被更新
	 * */
	if(isPonPortSwap)
	{
		if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{
			OnuProfile_Action_ByCode(OnuMap_Update, 0, swap_ponid, OnuIdx, filename, NULL, NULL);
			OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, swap_slot, swap_ponid, 0, NULL, NULL, NULL);        
			if(to_slot)
				OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, swap_ponid, 0, NULL, NULL, NULL);        
		}
	}

    /*
    sendOnuConfExecuteMsg(ulSlot, ulPort, ulOnuid);
     */
    BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_GENERAL)
    vty_out(vty, "association with onu %d/%d/%d OK!\r\n", slot, port, onuid);
    END_ONUCONF_DEBUG_PRINT()

    return CMD_SUCCESS;
}
int OnuProfile_Action_ProfileByOnuId(short int ponid, short int onuid, const char* name, onu_conf_res_act_t code) 
{
    int ret = VOS_OK;
    short int slot = GetCardIdxByPonChip(ponid);
    short int port = GetPonPortByPonChip(ponid);
    /*modi by luh @2014-10-11 统一修改判断条件*/
    if((SYS_LOCAL_MODULE_ISMASTERACTIVE && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER && !SYS_LOCAL_MODULE_TYPE_IS_CPU_PON)
        || slot == SYS_LOCAL_MODULE_SLOTNO)
	{
        if(name)
            addOnuToRestoreQueueByName(ponid, onuid, name, code);
        else
            addOnuToRestoreQueue(ponid, onuid, code, -1);
	}
	else if(OLT_ISLOCAL(ponid)/*added by wangjiah avoid sending onuConf to remote OLT*/)
	{
        sendOnuConfExecuteMsg(slot, port, onuid+1, code);
	}
    return ret;
}
int sendOnuConfExecuteMsg(USHORT slot, USHORT pon, USHORT onu, int act)
{

    ULONG len = sizeof(ONUCONFSYNDMSG_T);
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
    if (p)
    {
        p->slotid = slot;
        p->ponid = pon;
        p->onuid = onu - 1;
        p->msgtype = MSG_ONU_CONFSYND_EXECUTE_CDP;
        p->data[0] = act;

        return sendOnuConfSyndMsg(slot, p, len);

    }
    else
        return VOS_ERROR;
}

int sendOnuConfSyndUndoAssociationByOnuIdMsg(USHORT slot, USHORT pon, USHORT onu)
{

    int ret = VOS_OK;

    ULONG len = sizeof(ONUCONFSYNDMSG_T);

    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
    if (p)
    {
        p->slotid = slot;
        p->ponid = pon;
        p->onuid = onu;
        p->msgtype = MSG_ONU_CONFSYND_UNDO_ASSOCIATION_BY_ONUID;

        sendOnuConfSyndMsg(slot, p, len);

    }
    else
        ret = VOS_ERROR;

    return ret;
}

int sendOnuConfSyndUndoAssociationByNameMsg(USHORT slot, char *name, int nlen)
{

    int ret = VOS_OK;

    ULONG len = sizeof(ONUCONFSYNDMSG_T)+nlen;

    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
    if (p)
    {
        VOS_MemZero(p, len);
        p->slotid = slot;
        p->ponid = 0;
        p->onuid = 0;
        p->msgtype = MSG_ONU_CONFSYND_UNDO_ASSOCIATION_BY_NAME;
        VOS_MemCpy(p->data, name, nlen);

        sendOnuConfSyndMsg(slot, p, len);

    }
    else
        ret = VOS_ERROR;

    return ret;
}
int onuconfHaveAssociatedOnu(const char *name)
{
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(name);

        if(pd)
        {
            int i = 0, j=0 , n = MAXONUPERPON/8+1;
            for(i=0; i<SYS_MAX_PON_PORTNUM;i++)
                for(j=0; j<n; j++)
                    if(pd->onulist[i][j])
                    {
                        ONU_CONF_SEM_GIVE;
                        return TRUE;
                    }

        }
    }
    ONU_CONF_SEM_GIVE

    return FALSE;
}

int onuconfUndoAssociationByName(const char *name)
{
    int ret = VOS_OK;

    if(name)
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(name);
            if(pd)
            {
                int i,j;

                for(i=0; i<SYS_MAX_PON_PORTNUM; i++)
                {
                    for(j=0; j<MAXONUPERPON; j++)
                    {
                        int offset = j/8;
                        int bitnum = j%8;
                        if(pd->onulist[i][offset]&(1<<bitnum))
                        {
                            OnuProfile_Action_ByCode(OnuMap_Update, 0, i, j, DEFAULT_ONU_CONF, NULL, NULL);
                        }
                    }
                }
            }
            else
                ret = VOS_ERROR;
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

#endif
#if 1
/*2012-11-14, added by luh; Onu 节点的超时处理，不再修改onu的本地配置，只修改跟profile相关的内容*/
int SearchOnuVlanPortBySlot(short int slot, USHORT vid, short int *port, short int *OnuIdx, ULONG *brdIdx, ULONG *portIdx)
{
    int ret = VOS_ERROR;
    short int ulPort = 0;
    short int ulOnuIdx = 0;
    ULONG ulBrdIdx = 0;
    ULONG ulPortIdx = 0;
    for(ulPort = 1; ulPort <= PONPORTPERCARD; ulPort++)
    {
        short int PonPortIdx = GetPonPortIdxBySlot(slot, ulPort);
        if(PonPortIdx == RERROR)
            continue;
        
        if(SearchOnuVlanPortByPon(PonPortIdx, vid, &ulOnuIdx, &ulBrdIdx, &ulPortIdx) == VOS_OK)
        {
            *port = ulPort;
            *OnuIdx = ulOnuIdx;
            *brdIdx = ulBrdIdx;
            *portIdx = ulPortIdx;
            ret = VOS_OK;
            break;
        }
    }
    return ret;
}
int SearchOnuVlanPortByPon(short int PonPortIdx, USHORT vid, short int *OnuIdx, ULONG *brdIdx, ULONG *portIdx)
{
    int ret = VOS_ERROR;
    short int ulOnuIdx = 0;
    ULONG ulBrdIdx = 0;
    ULONG ulPortIdx = 0;
    for(ulOnuIdx=0;ulOnuIdx<MAXONUPERPON;ulOnuIdx++)
    {
        if(ThisIsValidOnu(PonPortIdx, ulOnuIdx) != ROK)
            continue;
        
        if(SearchOnuVlanPort(PonPortIdx, ulOnuIdx, vid, &ulBrdIdx, &ulPortIdx) == VOS_OK)
        {
            *OnuIdx = ulOnuIdx;
            *brdIdx = ulBrdIdx;
            *portIdx = ulPortIdx;
            ret = VOS_OK;
            break;
        }
    }
    return ret;
}
int SearchOnuVlanPort(short int PonPortIdx, short int OnuIdx, USHORT vid, ULONG *brdIdx, ULONG *portIdx)
{
    int ret = VOS_ERROR;
    int onuPortNum = 0;
    ULONG all = 0, untag = 0; 
    short int slot = GetCardIdxByPonChip(PonPortIdx);
    short int port = GetPonPortByPonChip(PonPortIdx);
    ULONG ulPortNum = 0;
    
    if(SearchOnuPortByVlanRules(vid, slot, port, OnuIdx+1, &ulPortNum) == VOS_OK)
    {
        *brdIdx = 1;
        *portIdx = ulPortNum;
        ret = VOS_OK;
    }
    else
    {
        if(get_onuconf_vlanPortlist(PonPortIdx, OnuIdx, vid, &all, &untag) == ROK)
        {
            int i = 1;
            while(all)
            {
                if(all&1)
                {
                    *brdIdx = 1;
                    *portIdx = i;
                    ret = VOS_OK;
                    break;
                }
                i++;
                all>>=1;
            }
        }
        else
        {
            
        }
    }

    if(VOS_OK == ret && ONU_OPER_STATUS_UP == GetOnuOperStatus(PonPortIdx, OnuIdx))
    {        
        ULONG deviceId = MAKEDEVID(slot, port, OnuIdx+1);
#if 0        
        CTC_getDeviceCapEthPortNum(PonPortIdx, OnuIdx, &onuPortNum);
#else
        getDeviceCapEthPortNum(deviceId, &onuPortNum);
#endif
        if(*portIdx > onuPortNum)
            ret = VOS_ERROR;
    }
    return ret;
}
void OnuNodeIdletimeOutHandler(struct vty *vty)
{
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
	short int ponid = 0, onuid = 0;
    char sz[80]="";
	ulIfIndex =(ULONG) vty->index;
	PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	ponid = GetPonPortIdxBySlot(ulSlot, ulPort);
	onuid = ulOnuid-1;
    
	/*avoid standby master write config file error, sync current config file*/
	if(ponid != RERROR && onuid != RERROR && SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
        if(onuConfCheckByPtrNoSuffix(vty->onuconfptr) == VOS_OK)
        {
            char name[ONU_CONFIG_NAME_LEN];            
            ONUConfigData_t *data = vty->onuconfptr;
            if(data)
                closeOnuConfigFile(data->confname);
            
            VOS_MemCpy(name, ONU_CONF_NAME_PTR_GET(ponid, onuid), ONU_CONFIG_NAME_LEN);    
            OnuProfile_Action_ByCode(OnuMap_Update, 0, ponid, onuid, data->confname, NULL, NULL);
            OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, ulSlot, ponid, onuid, data->confname, NULL, NULL);
            /*如果原来关联的是共享文件，则删除*/
            OnuProfile_Action_ByCode(OnuProfile_Delete_SyncBroadcast, 0, 0, 0, name, NULL, NULL);
            OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, name, NULL, NULL);

        }
        else
        {
            char* name = ONU_CONF_NAME_PTR_GET(ponid, onuid);
            if(name)
            {
                ONUConfigData_t *p = NULL, *pd = NULL;
                VOS_Sprintf(sz, "*auto%s", name);
                p = getOnuConfFromHashBucket(name);
                pd = getOnuConfFromHashBucket(sz);

                if(pd && p)
                {
                    closeOnuConfigFile(p->confname);
                    closeOnuConfigFile(pd->confname);
                    onuconfCopy(p, pd); /*pd中缓存了最开始的配置，这时放弃最新配置*/                   
                    OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, sz, NULL, NULL);
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, ulSlot, 0, 0, name, NULL, NULL);                    
                }
            }
        }  
	}

}
int OnuProfile_Action_ByCode(g_OnuProfile_action_code_t code, short int card, short int ponid, short int onuid, const char*name, const char *rename, void* ptr)
{
    int ret = VOS_OK;
    short int slot = 0, port = 0;
    short int to_slot = 0;
    if(code>= OnuProfile_code_max|| code<=OnuProfile_code_min)
        return VOS_ERROR;
    
    switch(code)
    {
        case OnuProfile_Add:
            ret = setOnuConfToHashBucket(name, ptr);            
            break;
        case OnuProfile_Add_SyncUnicast:
            ret = addOnuProfileSyncByslot(card, name);
            break;
        case OnuProfile_Add_SyncBroadcast:
            ret = addOnuProfileSyncBroadcastByName(name);            
            break;
        case OnuProfile_Delete:
            deleteOnuConfFromHashBucket(name);            
            break;
        case OnuProfile_Delete_SyncUnicast:
            sendOnuConfSyndDelReqMsg(card, name);
            break;
        case OnuProfile_Delete_SyncBroadcast:
            startOnuConfSyndBroadcastDelMsg(name);            
            break;
        case OnuProfile_Modify:
            ret = onu_profile_rename(name, rename);
            break;
        case OnuProfile_Modify_SyncUnicast:
            ret = sendOnuConfSyncUnicastRenameMsg(card, name, rename);
            break;            
        case OnuProfile_Modify_SyncBroadcast:
            ret = sendOnuConfSyncBroadcastRenameMsg(name, rename);
            break;
        case OnuMap_Update:
            ret = OnuProfile_Set_NameMapByPonid(name, ponid, onuid);            
            break;
        case OnuMap_Update_SyncUnicast:
            ret = sendOnuConfSyndOnuIdMapReqMsg(card, ponid); 
            break;
        case OnuMap_Update_SyncBroadcast:
            ret = updateAllOnuMapSyncBroadcastByPonId(ponid);
            break;
        case OnuAction_Do_AssociateAndSync:
            ret = OnuProfile_Action_ProfileByOnuId(ponid, onuid, name, ONU_CONF_RES_ACTION_DO);            
            break;
        case OnuAction_Undo_AssociateAndSync:
            ret = OnuProfile_Action_ProfileByOnuId(ponid, onuid, name, ONU_CONF_RES_ACTION_UNDO);           
            break;
        default:
            ret = ERROR;
            break;
            
    }
}
int OnuProfile_VtytimeoutCallback(struct vty *vty)
{
    ONUConfigData_t *data = NULL;
    if(vty == NULL)
        return VOS_ERROR;
    
    switch(vty->node)
    {
    	case ONU_NODE:
    	case ONU_GT821_GT831_NODE:
    	case ONU_GT813_NODE:
    	case ONU_GT865_NODE:
    	case ONU_CTC_NODE:
    	case ONU_GPON_NODE:
    	case ONU_RESERVED1_NODE:
    	case ONU_RESERVED2_NODE:
    	case ONU_RESERVED3_NODE:
#if 0            
            if(onuConfCheckByPtrNoSuffix(vty->onuconfptr) == VOS_OK)
            {
                cl_onuNodeVtyExitConfirmYes(vty);
                data = vty->orig_profileptr;
                if(data)
                    closeOnuConfigFile(data->confname);
                
                data = vty->onuconfptr;
                if(data)
                    closeOnuConfigFile(data->confname);
            }
            else
            {
                char sz[80] = "";
                data = vty->orig_profileptr;
                if(data->confname)
                {
                    ONUConfigData_t *p = NULL, *pd = NULL;
                    VOS_Sprintf(sz, "*auto%s", data->confname);
                    p = getOnuConfFromHashBucket(data->confname);
                    pd = getOnuConfFromHashBucket(sz);

                    if(pd && p)
                    {
                        closeOnuConfigFile(p->confname);
                        closeOnuConfigFile(pd->confname);
                        if(onuconfCompare(pd, p) != VOS_OK)
                        {
                            ULONG uv[4] = {0, 0, data->confname, EM_ONU_PROFILE_MODIFIED};
                            processEventCallbackList(EVENT_ONUPROFILE_MODIFIED, uv);
                        }
                        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, sz, NULL, NULL);
                    }
                }
            }
#else
            OnuNodeIdletimeOutHandler(vty);
#endif
    		break;
    	case ONU_PROFILE_NODE:
    	    cl_profileNodeVtyExitConfirmNo(vty);
            data = vty->orig_profileptr;
            if(data)
                closeOnuConfigFile(data->confname);
            
            data = vty->onuconfptr;
            if(data)
                closeOnuConfigFile(data->confname);
            
            break;
        default:
            break;
    }
    return VOS_OK;
}
int ReleasePermissionByOnuid(short int ponid, short int onuid)
{
    char *name = getOnuConfNamePtrByPonId(ponid, onuid);
    ONUConfigData_t *pd = NULL;
    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        ONU_CONF_SEM_TAKE
        {
            pd = getOnuConfFromHashBucket(name);
            if(pd && pd->owner)
            {
                pd->owner = 0;
            }
        }
        ONU_CONF_SEM_GIVE
    }
}
void ReleasePermissionByName(const char *name)
{
    ONUConfigData_t *pd = NULL;
    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
    {
        ONU_CONF_SEM_TAKE
        {
            pd = getOnuConfFromHashBucket(name);
            if(pd && pd->owner)
            {
                pd->owner = 0;
            }
        }
        ONU_CONF_SEM_GIVE
    }
}
#endif
#if 1
ULONG calcOnuConfigSize(void)
{
    return sizeof(ONUConfigData_t);
}

ULONG get_ctcOnuOtherVendorAccept()
{
    ULONG ret = 0;
    VOS_TaskLock();
    ret = ctc_onu_other_vendor_accept;
    VOS_TaskUnlock();
    return ret;
}

ULONG set_ctcOnuOtherVendorAccept(const ULONG v)
{
    VOS_TaskLock();
    ctc_onu_other_vendor_accept = v;
    VOS_TaskUnlock();
    return v;
}

void onuconf_DebugLevelSet(int level)
{
    onu_conf_print_debug |= (1<<level);
}

void onuconf_DebugLevelClr(int level)
{
    onu_conf_print_debug &= ~(1<<level);
}

int onuconf_isDebugLevelSet(int level)
{
    return (onu_conf_print_debug &(1<<level));
}

int addToRenameList(char *name, char *rename)
{
    int i = 0;

	VOS_TaskLock();
	
    for(i=0; i<MAX_RENAME_NUM; i++)
    {
        if(!g_rename_list[i].valid)
        {
            VOS_StrCpy(g_rename_list[i].filename, name);
            VOS_StrCpy(g_rename_list[i].rename, rename);
            g_rename_list[i].valid = 1;
            break;
        }
    }
	VOS_TaskUnlock();

    if(i == MAX_RENAME_NUM)
        return VOS_ERROR;
    else
        return VOS_OK;
}

int delFromRenameList(char *name)
{
    int i = 0;

	VOS_TaskLock();
    for(i=0; i<MAX_RENAME_NUM; i++)
    {
        if(g_rename_list[i].valid && !VOS_StrCmp(g_rename_list[i].filename, name))
        {
            g_rename_list[i].filename[0] = 0;
            g_rename_list[i].valid = 0;
            break;
        }
    }
	VOS_TaskUnlock();

    if(i==MAX_RENAME_NUM)
        return VOS_ERROR;
    else
        return VOS_OK;
}

int getFromRenameList(char *name, char *rename)
{
    int i = 0;
    for(i=0; i<MAX_RENAME_NUM; i++)
    {
        if(g_rename_list[i].valid && !VOS_StrCmp(g_rename_list[i].filename, name))
            break;
    }

    if(i == MAX_RENAME_NUM)
    {
        return VOS_ERROR;
    }
    else
    {
        VOS_StrCpy(rename, g_rename_list[i].rename);
        return VOS_OK;
    }
}

ULONG IFM_ONU_PROFILE_CREATE_INDEX(const char *name)
{
    IFM_ONU_PROFILE_IF_INDEX_U unPIfIndex;

    /* this is a union, so the first line is used to clear the structure */
    unPIfIndex.ulIfindex = 0;
    unPIfIndex.pindex.type = IFM_ONU_PROFILE_TYPE;
    unPIfIndex.pindex.reserv = getOnuConfHashIndex(name);


    return unPIfIndex.ulIfindex;
}

LONG IsProfileNodeVty( ULONG ulIfIndex, ULONG *suffix)
{
    IFM_ONU_PROFILE_IF_INDEX_U unPIfIndex;

    unPIfIndex.ulIfindex = ulIfIndex;
    if ( unPIfIndex.pindex.type != IFM_ONU_PROFILE_TYPE )
        return FALSE;
    else
    {
        *suffix = unPIfIndex.pindex.reserv;
        return TRUE;
    }
}

#ifndef ONUID_MAP
static void addOnuConfToSaveStack(void *data)
{

    onu_conf_save_stack_list_ptr_t pd = VOS_Malloc(sizeof(onu_conf_save_stack_list_t), MODULE_RPU_CTC);

    if(pd)
    {
        pd->data = data;
        pd->next = NULL;

        if(g_onuconf_save_stack_head == g_onuconf_save_stack_tail &&
                g_onuconf_save_stack_head == NULL)
        {
            g_onuconf_save_stack_head = pd;
            g_onuconf_save_stack_tail = pd;
        }
        else
        {
            g_onuconf_save_stack_tail->next = pd;
            g_onuconf_save_stack_tail = pd;
        }
    }
}

static void emptyOnuConfSaveStack(void)
{
    onu_conf_save_stack_list_ptr_t p = g_onuconf_save_stack_head;
    while(p)
    {
        g_onuconf_save_stack_head = p->next;
        VOS_Free(p);
        p = g_onuconf_save_stack_head;
    }

    g_onuconf_save_stack_tail = NULL;
}

static int findOnuConfInSaveStack(void *pd)
{
    onu_conf_save_stack_list_ptr_t p = g_onuconf_save_stack_head;
    while(p)
    {
        if(p->data == pd)
            return TRUE;
        p = p->next;
    }
    return FALSE;
}
#endif
char *getOnuConfNamePtrByPonId(short int ponid, short int onuid)
{
    if(ponid == -1 || onuid == -1)
    {
        VOS_ASSERT(0);
        return NULL;
    }
    else
    {
        int pon_id = onuConfGetPonIdFromPonProtectGroup(ponid);
		if(pon_id < SYS_MAX_PON_PORTNUM && pon_id >= 0)
		{
            ONUCONF_ONU_LIMIT_DEBUG("\r\n getOnuConfNamePtrByPonId(%d,%d): %s\r\n", pon_id, onuid, g_onuConfOnuIdMapArry[pon_id][onuid].name);
            return g_onuConfOnuIdMapArry[pon_id][onuid].name;
		}
        else
        {
			VOS_ASSERT(0);
            return NULL;
        }
    }
}
UCHAR getOnuConfRestoreFlag(short int ponid, short int onuid)
{

    if(ponid == -1 || onuid == -1)
        return 0;
    else
    {
        ponid = onuConfGetPonIdFromPonProtectGroup(ponid);

        if(ponid >= 0 && ponid < SYS_MAX_PON_PORTNUM)
            return g_onuConfOnuIdMapArry[ponid][onuid].configRestoreFlag;
        else
            return 0;
    }
}

int setOnuConfRestoreFlag(short int ponid, short int onuid, int v)
{

    int ret = VOS_ERROR;

    if(ponid != -1 && onuid != -1)
    {
        ponid = onuConfGetPonIdFromPonProtectGroup(ponid);
        if(ponid >= 0 && ponid < SYS_MAX_PON_PORTNUM)
        {
            g_onuConfOnuIdMapArry[ponid][onuid].configRestoreFlag = v;
            ret = VOS_OK;
        }
        else
            ret = VOS_ERROR;
    }

    return ret;
}

int setOnuConfUndoFlag(short int ponid, short int onuid, int v)
{

    int ret = VOS_ERROR;

    if(ponid != -1 && onuid != -1)
    {
        ponid = onuConfGetPonIdFromPonProtectGroup(ponid);
        if(ponid >= 0 && ponid < SYS_MAX_PON_PORTNUM)
        {
            g_onuConfOnuIdMapArry[ponid][onuid].undoconfigFlag = v;
            ret = VOS_OK;
        }
        else
            ret = VOS_ERROR;
    }
    return ret;
}
UCHAR getOnuConfUndoFlag(short int ponid, short int onuid)
{

    if(ponid == -1 || onuid == -1)
        return 0;
    else
    {
        ponid = onuConfGetPonIdFromPonProtectGroup(ponid);
        if(ponid >= 0 && ponid < SYS_MAX_PON_PORTNUM)
            return g_onuConfOnuIdMapArry[ponid][onuid].undoconfigFlag;
        else
            return 0;
    }
}

int setOnuConfResult(short int ponid, short int onuid, int v)
{

    int ret = VOS_ERROR;

    if(ponid != -1 && onuid != -1)
    {
        ponid = onuConfGetPonIdFromPonProtectGroup(ponid);
        if(ponid >= 0 && ponid < SYS_MAX_PON_PORTNUM)
        {
            g_onuConfOnuIdMapArry[ponid][onuid].configResult = v;
            ret = VOS_OK;
        }
        else
            ret = VOS_ERROR;
    }

    return ret;
}

UCHAR getOnuConfResult(short int ponid, short int onuid)
{

    if(ponid == -1 || onuid == -1)
        return 0;
    else
    {
        ponid = onuConfGetPonIdFromPonProtectGroup(ponid);
        if(ponid >= 0 && ponid < SYS_MAX_PON_PORTNUM)
            return g_onuConfOnuIdMapArry[ponid][onuid].configResult;
        else
            return 0;
    }
}
#endif
/*ONUportConf*/

int isOnuConfDefault(short int ponid, short int onuid) 
{
	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

	return VOS_StrnCmp(g_onuConfOnuIdMapArry[ponid][onuid].name, DEFAULT_ONU_CONF, ONU_CONFIG_NAME_LEN) == 0 ? TRUE: FALSE;
}

#if 1
int 
getOnuConfPortVlanMode(SHORT ponid, SHORT onuid,int port)
{
	int ret = 0;

	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

	if(ONU_PORT_VALID(port))
	{
	    ONU_CONF_SEM_TAKE
	    {
            ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if(p)
            {
            	ONUVlanConf_t *pvc = &p->vlanconf;

            	if(pvc->vmtranslationportmask&(1<<(port-1)))
            		ret = ONU_CONF_VLAN_MODE_TRANSLATION;
            	else if(pvc->vmtagportmask&(1<<(port-1)))
            		ret = ONU_CONF_VLAN_MODE_TAG;
            	else if(pvc->vmtrunkportmask&(1<<(port-1)))
            		ret = ONU_CONF_VLAN_MODE_TRUNK;
            	else if(pvc->vmaggportmask&(1<<(port-1)))
            		ret = ONU_CONF_VLAN_MODE_AGG;
            	else
                    ret = ONU_CONF_VLAN_MODE_TRANSPARENT;

            }
	    }
        ONU_CONF_SEM_GIVE
	}

	return ret;
}

int 
getOnuConfPortVlanModeByPtr(int suffix, void *pdata, int port)
{
    int ret = VOS_OK;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t * p = (ONUConfigData_t *)pdata;
                if(p)
                {
                	ONUVlanConf_t *pvc = &p->vlanconf;

                	if(pvc->vmtranslationportmask&(1<<(port-1)))
                		ret = ONU_CONF_VLAN_MODE_TRANSLATION;
                	else if(pvc->vmtagportmask&(1<<(port-1)))
                		ret = ONU_CONF_VLAN_MODE_TAG;
                	else if(pvc->vmtrunkportmask&(1<<(port-1)))
                		ret = ONU_CONF_VLAN_MODE_TRUNK;
                	else if(pvc->vmaggportmask&(1<<(port-1)))
                		ret = ONU_CONF_VLAN_MODE_AGG;
                	else
                        ret = ONU_CONF_VLAN_MODE_TRANSPARENT;

                }
            }
        }
        ONU_CONF_SEM_GIVE;
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int
SetOnuConfPortVlanMode(SHORT ponid, SHORT onuid, int port, int num)
{
	int ret = VOS_ERROR;

	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

	if(ONU_PORT_VALID(port))
	{

	    ONU_CONF_SEM_TAKE
	    {
	        ONUConfigData_t *p = getOnuConfFromHashBucket(
                    ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (p )
            {
            	ONUVlanConf_t *pvc = &p->vlanconf;
            	ULONG portlist = 1<<(port-1);
            	switch(num)
            	{
            	case ONU_CONF_VLAN_MODE_TRANSPARENT:
            		pvc->vmtransparentportmask |= portlist;
            		pvc->vmaggportmask &= ~portlist;
            		pvc->vmtranslationportmask &= ~portlist;
            		pvc->vmtrunkportmask &= ~portlist;
            		pvc->vmtagportmask &= ~portlist;

            		break;
            	case ONU_CONF_VLAN_MODE_TRANSLATION:
            		pvc->vmtransparentportmask &= ~portlist;
            		pvc->vmaggportmask &= ~portlist;
            		pvc->vmtranslationportmask |= portlist;
            		pvc->vmtrunkportmask &= ~portlist;
            		pvc->vmtagportmask &= ~portlist;

            		break;
            	case ONU_CONF_VLAN_MODE_TAG:
            		pvc->vmtransparentportmask &= ~portlist;
            		pvc->vmaggportmask &= ~portlist;
            		pvc->vmtranslationportmask &= ~portlist;
            		pvc->vmtrunkportmask &= ~portlist;
            		pvc->vmtagportmask |= portlist;
            		break;
            	case ONU_CONF_VLAN_MODE_TRUNK:
            		pvc->vmtransparentportmask &= ~portlist;
            		pvc->vmaggportmask &= ~portlist;
            		pvc->vmtranslationportmask &= ~portlist;
            		pvc->vmtrunkportmask |= portlist;
            		pvc->vmtagportmask &= ~portlist;
            	    break;
            	case ONU_CONF_VLAN_MODE_AGG:
            		pvc->vmtransparentportmask &= ~portlist;
            		pvc->vmaggportmask &= ~portlist;
            		pvc->vmtranslationportmask &= ~portlist;
            		pvc->vmtrunkportmask &= ~portlist;
            		pvc->vmtagportmask |= portlist;
            		break;
            	default:
            		break;
            	}
                ret = VOS_OK;
            }
	    }
	    ONU_CONF_SEM_GIVE
	}

	return ret;
}

int
SetOnuConfPortVlanModeByPtr(int suffix, void *pdata, USHORT port, ULONG uv)
{
    int ret = VOS_ERROR;

    if(ONU_PORT_VALID(port))
    {

        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t * p = (ONUConfigData_t *)pdata;
                if (p)
                {
                	ONUVlanConf_t *pvc = &p->vlanconf;
                	ULONG portlist = 1<<(port-1);
                	switch(uv)
                	{
                    	case ONU_CONF_VLAN_MODE_TRANSPARENT:
                    		pvc->vmtransparentportmask |= portlist;
                    		pvc->vmaggportmask &= ~portlist;
                    		pvc->vmtranslationportmask &= ~portlist;
                    		pvc->vmtrunkportmask &= ~portlist;
                    		pvc->vmtagportmask &= ~portlist;

                    		break;
                    	case ONU_CONF_VLAN_MODE_TRANSLATION:
                    		pvc->vmtransparentportmask &= ~portlist;
                    		pvc->vmaggportmask &= ~portlist;
                    		pvc->vmtranslationportmask |= portlist;
                    		pvc->vmtrunkportmask &= ~portlist;
                    		pvc->vmtagportmask &= ~portlist;

                    		break;
                    	case ONU_CONF_VLAN_MODE_TAG:
                    		pvc->vmtransparentportmask &= ~portlist;
                    		pvc->vmaggportmask &= ~portlist;
                    		pvc->vmtranslationportmask &= ~portlist;
                    		pvc->vmtrunkportmask &= ~portlist;
                    		pvc->vmtagportmask |= portlist;
                    		break;
                    	case ONU_CONF_VLAN_MODE_TRUNK:
                    		pvc->vmtransparentportmask &= ~portlist;
                    		pvc->vmaggportmask &= ~portlist;
                    		pvc->vmtranslationportmask &= ~portlist;
                    		pvc->vmtrunkportmask |= portlist;
                    		pvc->vmtagportmask &= ~portlist;
                    	    break;
                    	case ONU_CONF_VLAN_MODE_AGG:
                    		pvc->vmtransparentportmask &= ~portlist;
                    		pvc->vmaggportmask &= ~portlist;
                    		pvc->vmtranslationportmask &= ~portlist;
                    		pvc->vmtrunkportmask &= ~portlist;
                    		pvc->vmtagportmask |= portlist;
                    		break;
                    	default:
                    		break;
                	}
                    ret = VOS_OK;
                }
            }
        }
        ONU_CONF_SEM_GIVE

    }
    else
        ret = VOS_ERROR;

    return ret;
}

static int sfun_getOnuConfVlanMode(ONUConfigData_t *p)
{
	int ret;

    if (p)
    {
        ONUVlanConf_t *pvc = &p->vlanconf;

        if (pvc->vmtranslationportmask &&
                !(pvc->vmaggportmask|pvc->vmtagportmask|pvc->vmtransparentportmask|pvc->vmtrunkportmask))
            ret = ONU_CONF_VLAN_MODE_TRANSLATION;
        else if (pvc->vmtagportmask &&
                !(pvc->vmaggportmask|pvc->vmtranslationportmask|pvc->vmtransparentportmask|pvc->vmtrunkportmask))
            ret = ONU_CONF_VLAN_MODE_TAG;
        else if (pvc->vmtrunkportmask &&
                !(pvc->vmaggportmask|pvc->vmtagportmask|pvc->vmtranslationportmask|pvc->vmtransparentportmask))
            ret = ONU_CONF_VLAN_MODE_TRUNK;
        else if (pvc->vmaggportmask &&
                !(pvc->vmtagportmask|pvc->vmtranslationportmask|pvc->vmtransparentportmask|pvc->vmtrunkportmask))
            ret = ONU_CONF_VLAN_MODE_AGG;
        else if(pvc->vmtransparentportmask &&
                !(pvc->vmaggportmask|pvc->vmtagportmask|pvc->vmtranslationportmask|pvc->vmtrunkportmask))
            ret = ONU_CONF_VLAN_MODE_TRANSPARENT;
        else
            ret = ONU_CONF_VLAN_MODE_MIXED;

    }
    else
        ret = ONU_CONF_VLAN_MODE_UNKNOWN;

    return ret;
}

int getOnuConfVlanMode(SHORT ponid, SHORT onuid)
{
    int ret = 0;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        if(p)
            ret  = sfun_getOnuConfVlanMode(p);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int getOnuConfVlanModeByPtr(int suffix, void *pdata)
{
    int ret = 0;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = VOS_ERROR;
        else
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
                ret = sfun_getOnuConfVlanMode(pdata);
        }
    }
    ONU_CONF_SEM_GIVE;
    return ret;

}

static int sfun_setOnuConfVlanMode(ONUConfigData_t * p, int mode)
{
    int ret = VOS_OK;
    int i = 0;

    if (p)
    {
        ONUVlanConf_t *pvc = &p->vlanconf;

        switch(mode)
        {
        case ONU_CONF_VLAN_MODE_TRANSLATION:
            pvc->vmtranslationportmask = 0xffffffff;
            pvc->vmtagportmask = 0;
            pvc->vmaggportmask = 0;
            pvc->vmtrunkportmask = 0;
            pvc->vmtransparentportmask = 0;
            break;
        case ONU_CONF_VLAN_MODE_TAG:
            pvc->vmtranslationportmask = 0;
            pvc->vmtagportmask = 0xffffffff;
            pvc->vmaggportmask = 0;
            pvc->vmtrunkportmask = 0;
            pvc->vmtransparentportmask = 0;
            break;
        case ONU_CONF_VLAN_MODE_TRUNK:
            pvc->vmtranslationportmask = 0;
            pvc->vmtagportmask = 0;
            pvc->vmaggportmask = 0;
            pvc->vmtrunkportmask = 0xffffffff;
            pvc->vmtransparentportmask = 0;
            break;
        case ONU_CONF_VLAN_MODE_AGG:
            pvc->vmtranslationportmask = 0;
            pvc->vmtagportmask = 0;
            pvc->vmaggportmask = 0xffffffff;
            pvc->vmtrunkportmask = 0;
            pvc->vmtransparentportmask = 0;
            break;
        case ONU_CONF_VLAN_MODE_TRANSPARENT:
            pvc->vmtranslationportmask = 0;
            pvc->vmtagportmask = 0;
            pvc->vmaggportmask = 0;
            pvc->vmtrunkportmask = 0;
            pvc->vmtransparentportmask = 0xffffffff;
            break;
        default:
            ret = VOS_ERROR;
            break;
        }

        if(ret == VOS_OK)
        {

            if (p->vlanconf.onuVlanEntryNum)
            {
                p->vlanconf.onuVlanEntryNum = 0;
                VOS_MemZero(p->vlanconf.entryarry, sizeof(ONUVlanEntryArry_t));
            }

            for(i=0; i<ONU_MAX_PORT; i++)
            {
                ONUPortConf_t * opc = p->portconf;
                VOS_MemZero(&opc[i].extVlanConf, sizeof(ONUVlanExtConf_t));
            }
        }
        
        if(mode == ONU_CONF_VLAN_MODE_TRANSPARENT)
        {
            for(i=0;i<ONU_MAX_PORT;i++)
            {
                p->portconf[i].defaultVid = 1;
            }
        }
        
        if(mode == ONU_CONF_VLAN_MODE_TRUNK || mode == ONU_CONF_VLAN_MODE_TAG)
        {
            p->vlanconf.onuVlanEntryNum = 1;
            p->vlanconf.entryarry[0].allPortMask= 0xFFFFFFFF;
	        p->vlanconf.entryarry[0].untagPortMask = 0xFFFFFFFF;
			p->vlanconf.entryarry[0].vlanid = 1;
            /*2013-2-21 added by luh    基于端口组织的vlan数据也必须回复默认,解决
                        大量私有文件在配置数据没有修改时产生。*/
            for(i=0;i<ONU_MAX_PORT;i++)
            {
                p->portconf[i].defaultVid = 1;
            }            
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int setOnuConfVlanMode(SHORT ponid, SHORT onuid, int mode)
{
    int ret = 0;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_setOnuConfVlanMode(p, mode);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int setOnuConfVlanModeByPtr(int suffix, void *pdata, ULONG uv)
{
    int ret = 0;

    ONU_CONF_SEM_TAKE
    {
        if (onuConfCheckByPtr(suffix, pdata))
            ret = VOS_ERROR;
        else
        {
            if (onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
                ret = sfun_setOnuConfVlanMode(pdata, uv);
        }

    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int 
getOnuConfvlan_numberOfEntry(SHORT ponid, SHORT onuid,int port)
{
	int ret = 0;

	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

	if(ONU_PORT_VALID(port))
	{
	    ONU_CONF_SEM_TAKE
	    {
	        ONUConfigData_t *p = getOnuConfFromHashBucket(
	                ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (p)
            {
                if(p->onuType == ONU_CONF_CTC_TYPE)
                    ret = p->portconf[port - 1].extVlanConf.numberOfEntry;
                else
                    ret = p->vlanconf.onuVlanEntryNum;
            }
	    }
	    ONU_CONF_SEM_GIVE
	}

	return ret;
}
#endif

/*ONUVlanConf*/
#if 1
int get_OnuConf_Ctc_portVlanConfig(SHORT ponid, SHORT onuid, USHORT port, CTC_STACK_port_vlan_configuration_t * pconf)
{
	int ret = RERROR,i=0, j=0;
	
	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

	if(pconf)
	    VOS_MemZero(pconf, sizeof(CTC_STACK_port_vlan_configuration_t));
	else
	    return ret;

	if(ONU_PORT_VALID(port))
	{
	    int vmode = getOnuConfPortVlanMode(ponid, onuid, port);

	    ONU_CONF_SEM_TAKE
	    {
	        ONUConfigData_t *p = getOnuConfFromHashBucket(
	                ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (p)
            {

                ONUPortConf_t *opc = &p->portconf[port-1];
                ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                pconf->mode = vmode;
                pconf->default_vlan = opc->defaultVid;

                if (pconf->mode == ONU_CONF_VLAN_MODE_TRUNK)
                {
                    int i,k=0,num = p->vlanconf.onuVlanEntryNum;
                    for(i=0; i<num; i++)
                    {
                        ULONG portlist = p->vlanconf.entryarry[i].allPortMask;
						ULONG untag_list = p->vlanconf.entryarry[i].untagPortMask;
                        if((portlist&(1<<(port-1))) &&
							(!(untag_list&(1<<(port-1)))))
                            pconf->vlan_list[k++] = p->vlanconf.entryarry[i].vlanid;
                    }
                    pconf->number_of_entries = k;

                }
                else if (pconf->mode == ONU_CONF_VLAN_MODE_TRANSLATION)
                {
                    pconf->number_of_entries = pvc->numberOfEntry;
                    for(i=0; i<pvc->numberOfEntry; i++)
                    {
                        pconf->vlan_list[2*i] = pvc->extEntry.transArry[i].ovid;
                        pconf->vlan_list[2*i+1] = pvc->extEntry.transArry[i].svid;
                    }
                }
                else if(pconf->mode == ONU_CONF_VLAN_MODE_AGG)
                {
                    pconf->number_of_aggregation_tables = pvc->numberOfAggTables;
                    for(i=0; i<pvc->numberOfAggTables; i++)
                    {
                        pconf->vlan_aggregation_tables[i].target_vlan = pvc->targetVlanId[i];
                        for(j=0; j<pvc->onuVlanAggEntryNum[i]; j++)
                            pconf->vlan_aggregation_tables[i].vlan_aggregation_list[j] = pvc->extEntry.aggArry[i][j];
                    }
                }
                ret = ROK;
            }
	    }
	    ONU_CONF_SEM_GIVE
	}

	return ret;
}

int get_OnuConf_Ctc_portVlanConfigByPtr(int suffix, void *pdata, int port, CTC_STACK_port_vlan_configuration_t * pconf)
{
    int ret = RERROR,i=0, j=0;
    int vmode;

    if(pconf)
        VOS_MemZero(pconf, sizeof(CTC_STACK_port_vlan_configuration_t));
    else
        return ret;

    if(ONU_PORT_VALID(port))
    {
        vmode = getOnuConfPortVlanModeByPtr(suffix, pdata,port);
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t * p = (ONUConfigData_t *)pdata;
            if (p)
            {

                ONUPortConf_t *opc = &p->portconf[port-1];
                ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                pconf->mode = vmode;
                pconf->default_vlan = opc->defaultVid;

                if (pconf->mode == ONU_CONF_VLAN_MODE_TRUNK)
                {
                    int i,k=0;
                    for(i=0; i<p->vlanconf.onuVlanEntryNum; i++)
                    {
                        ULONG portlist = p->vlanconf.entryarry[i].allPortMask;
                        if(portlist&(1<<(port-1)))
                            pconf->vlan_list[k++] = p->vlanconf.entryarry[i].vlanid;
                    }
                    pconf->number_of_entries = k;
                }
                else if (pconf->mode == ONU_CONF_VLAN_MODE_TRANSLATION)
                {
                    pconf->number_of_entries = pvc->numberOfEntry;
                    for(i=0; i<pvc->numberOfEntry; i++)
                    {
                        pconf->vlan_list[2*i] = pvc->extEntry.transArry[i].ovid;
                        pconf->vlan_list[2*i+1] = pvc->extEntry.transArry[i].svid;
                    }
                }
                else if(pconf->mode == ONU_CONF_VLAN_MODE_AGG)
                {
                    pconf->number_of_aggregation_tables = pvc->numberOfAggTables;
                    for(i=0; i<pvc->numberOfAggTables; i++)
                    {
                        pconf->vlan_aggregation_tables[i].target_vlan = pvc->targetVlanId[i];
                        for(j=0; j<pvc->onuVlanAggEntryNum[i]; j++)
                            pconf->vlan_aggregation_tables[i].vlan_aggregation_list[j] = pvc->extEntry.aggArry[i][j];
                    }
                }

                ret = ROK;
            }
        }
        ONU_CONF_SEM_GIVE;
    }
    return ret;

}

int getOnuConfUntagPortVlan(short int ponid, short int onuid, short int port)
{
    int i = 0, ret = 0;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (p)
            {
                int VlanentryNum = p->vlanconf.onuVlanEntryNum;
                for (i = 0; i < VlanentryNum; i++)
                {
                    if(p->vlanconf.entryarry[i].untagPortMask & (1<<(port-1)))
                    {
                        ret = p->vlanconf.entryarry[i].vlanid;
                        break;
                    }
                }

            }
        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int getOnuConfUntagPortVlanByPtr(int suffix, void *pdata, short int port)
{
    int i = 0, ret = 0;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata) == VOS_OK)
            {
                ONUConfigData_t *p = (ONUConfigData_t *)pdata;
                int VlanentryNum = p->vlanconf.onuVlanEntryNum;
                for (i = 0; i < VlanentryNum; i++)
                {
                    if(p->vlanconf.entryarry[i].untagPortMask & (1<<(port-1)))
                    {
                        ret = p->vlanconf.entryarry[i].vlanid;
                        break;
                    }
                }

            }
        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int get_onuconf_vlanPortlist(SHORT ponid, SHORT onuid, USHORT vid, ULONG *allportmask, ULONG *untagmask)
{
	int i = 0,ret = RERROR;


	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

	ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(
                ONU_CONF_NAME_PTR_GET(ponid, onuid));
        /*int Mode = getOnuConfvlanMode(ponid,onuid,port);*/

        if (p)
        {
            int VlanentryNum = p->vlanconf.onuVlanEntryNum;
            for (i = 0; i < VlanentryNum; i++)
            {
                if (vid == p->vlanconf.entryarry[i].vlanid)
                {
                    *allportmask = p->vlanconf.entryarry[i].allPortMask;
                    *untagmask = p->vlanconf.entryarry[i].untagPortMask;
                    ret = ROK;
                    break;
                }
            }
        }
    }
    ONU_CONF_SEM_GIVE

	return ret;
}

int get_onuconf_vlanPortlistByPtr(int suffix, void *pdata, USHORT vid, ULONG *allportmask, ULONG *untagmask)
{
    int i = 0,ret = RERROR;

 
    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = VOS_ERROR;
        else
        {
            ONUConfigData_t * p = (ONUConfigData_t *)pdata;

            if (p)
            {
                int VlanentryNum = p->vlanconf.onuVlanEntryNum;
                for (i = 0; i < VlanentryNum; i++)
                {
                    if (vid == p->vlanconf.entryarry[i].vlanid)
                    {
                        *allportmask = p->vlanconf.entryarry[i].allPortMask;
                        *untagmask = p->vlanconf.entryarry[i].untagPortMask;
                        ret = ROK;
                        break;
                    }
                }

            }
        }
    }
    ONU_CONF_SEM_GIVE;

    return ret;

}
	
int set_onuconf_vlanPortlist(SHORT ponid, SHORT onuid, USHORT vid, ULONG allportmask, ULONG untagmask)
{
	int i = 0, ret = RERROR;

	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

	ONU_CONF_SEM_TAKE
	{

	    ONUConfigData_t *p = getOnuConfFromHashBucket(
	            ONU_CONF_NAME_PTR_GET(ponid, onuid));
        
        /*int Mode = getOnuConfvlanMode(ponid,onuid,port);*/

        if (p)
        {
            ret = ROK;

            {

                int VlanentryNum = p->vlanconf.onuVlanEntryNum;

                for (i = 0; i < VlanentryNum; i++)
                {
                    if (vid == p->vlanconf.entryarry[i].vlanid)
                    {
                        p->vlanconf.entryarry[i].allPortMask = allportmask;
                        p->vlanconf.entryarry[i].untagPortMask = untagmask;
                        break;
                    }
                }
                if (i == VlanentryNum )
                {
                    if(VlanentryNum <ONU_MAX_VLAN-1)/*modified by luh 2013-7-8*/
                    {
                        p->vlanconf.entryarry[i].vlanid = vid;
                        p->vlanconf.entryarry[i].allPortMask = allportmask;
                        p->vlanconf.entryarry[i].untagPortMask = untagmask;
                        p->vlanconf.onuVlanEntryNum++;
                    }
                    else
                        ret = VOS_ERROR;
                }
            }
        }
	}

	ONU_CONF_SEM_GIVE

	return ret;
}

int set_onuconf_vlanPortlistByPtr(int suffix, void *pdata, USHORT vid, ULONG allportmask, ULONG untagmask)
{
    int i = 0, ret = RERROR;


    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = VOS_ERROR;
        else
        {
            ONUConfigData_t * p = (ONUConfigData_t *)pdata;
            if (p)
            {
                ret = ROK;

                {

                    int VlanentryNum = p->vlanconf.onuVlanEntryNum;

                    for (i = 0; i < VlanentryNum; i++)
                    {
                        if (vid == p->vlanconf.entryarry[i].vlanid)
                        {
                            p->vlanconf.entryarry[i].allPortMask = allportmask;
                            p->vlanconf.entryarry[i].untagPortMask = untagmask;
                            break;
                        }
                    }
                    if (i == VlanentryNum )
                    {
                        if(VlanentryNum <ONU_MAX_VLAN-1)/*modified by luh 2013-7-8*/
                        {
                            p->vlanconf.entryarry[i].vlanid = vid;
                            p->vlanconf.entryarry[i].allPortMask = allportmask;
                            p->vlanconf.entryarry[i].untagPortMask = untagmask;
                            p->vlanconf.onuVlanEntryNum++;
                        }
                        else
                            ret = VOS_ERROR;
                    }
                }
            }
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;

}
int gpon_get_onuconf_vlanCfg(short int PonPortIdx, short int OnuIdx, short int *vlan_mode, ONUVlanConf_t *vlan)
{
    int i = 0,ret = RERROR, k =0;
    char *name = ONU_CONF_NAME_PTR_GET(PonPortIdx, OnuIdx);
    short int ulSlot = GetCardIdxByPonChip(PonPortIdx);
    short int ulPort = GetPonPortByPonChip(PonPortIdx);
    short int OnuId = OnuIdx + 1;
    ULONG deviceIndex = MAKEDEVID(ulSlot, ulPort, OnuId);    
    ULONG fenum = 0, v_portlist = 0;
    ONUConfigData_t *p = getOnuConfFromHashBucket(name);


    VOS_MemZero(vlan, sizeof(ONUVlanConf_t));
    if(getDeviceCapEthPortNum(deviceIndex, &fenum) == VOS_OK)
    {    
        for(k=0;k<fenum;k++)
        v_portlist |= (1<<k);
    }
    ONU_CONF_SEM_TAKE
    {
        if (p)
        {
            if(p->vlanconf.vmtrunkportmask &&
            		!p->vlanconf.vmtranslationportmask &&
            		!p->vlanconf.vmtransparentportmask &&
            		!p->vlanconf.vmtagportmask &&
            		!p->vlanconf.vmaggportmask)
    		{    	
                *vlan_mode = 1;
            }
            else
            {
                *vlan_mode = 0;
            }
            vlan->portIsolateEnable = p->vlanconf.portIsolateEnable;
			
            vlan->vmtrunkportmask = p->vlanconf.vmtrunkportmask;
            vlan->vmtranslationportmask = p->vlanconf.vmtranslationportmask;
            vlan->vmtransparentportmask = p->vlanconf.vmtransparentportmask;
            vlan->vmtagportmask = p->vlanconf.vmtagportmask;
            vlan->vmaggportmask = p->vlanconf.vmaggportmask;
            
            vlan->onuVlanEntryNum = p->vlanconf.onuVlanEntryNum;
            for (i = 0; i < p->vlanconf.onuVlanEntryNum; i++)
            {
                vlan->entryarry[i].vlanid = p->vlanconf.entryarry[i].vlanid;
                vlan->entryarry[i].allPortMask = (p->vlanconf.entryarry[i].allPortMask)&v_portlist;
                vlan->entryarry[i].untagPortMask = (p->vlanconf.entryarry[i].untagPortMask)&v_portlist;
            }

            if(ulSlot&&ulPort&&OnuId)
            {
                int j = 0, m = 0, valid_flag = 0;
                int onu_num = 0;
                short int v_vid = 0;
                short int begin_slot = 0;
                short int begin_port = 0;
                short int begin_onuid = 0;
                short int end_slot = 0;
                short int end_port = 0;
                short int end_onuid = 0;

                for(i=0;i<p->rules.number_rules;i++)
                {
                    if(p->rules.rule[i].rule_mode)
                    {
                        begin_slot = GET_PONSLOT(p->rules.rule[i].begin_onuid);
                        begin_port = GET_PONPORT(p->rules.rule[i].begin_onuid);
                        begin_onuid = GET_ONUID(p->rules.rule[i].begin_onuid);
                        end_slot = GET_PONSLOT(p->rules.rule[i].end_onuid);
                        end_port = GET_PONPORT(p->rules.rule[i].end_onuid);
                        end_onuid = GET_ONUID(p->rules.rule[i].end_onuid);
                        
                        /*检查onu是否在范围之内*/
                        onu_num = Get_OnuNumFromOnuRange(ulSlot,ulPort,OnuId,end_slot,end_port,end_onuid);
                        if(onu_num == VOS_ERROR)
                            continue;
                        onu_num = Get_OnuNumFromOnuRange(begin_slot,begin_port,begin_onuid,ulSlot,ulPort,OnuId);
                        if(onu_num == VOS_ERROR)
                            continue;
                        
                        if(!valid_flag)
                            valid_flag = 1;
                        
                        v_vid   = p->rules.rule[i].begin_vid + (onu_num - 1)*p->rules.rule[i].step;
                        if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID)
                        {
                            for(j=0;j<vlan->onuVlanEntryNum;j++)
                            {
                                if(vlan->entryarry[j].vlanid == p->rules.rule[i].begin_vid)
                                    break;
                            }
                            if(j < vlan->onuVlanEntryNum)
                            {                                    
                                if(p->rules.rule[i].mode == 2)
                                {
                                    vlan->entryarry[j].untagPortMask |= v_portlist;  
                                }
                                vlan->entryarry[j].allPortMask |= v_portlist;
                            }
                            else
                            {
                                k= vlan->onuVlanEntryNum;
                                vlan->entryarry[k].vlanid = p->rules.rule[i].begin_vid;
                                if(p->rules.rule[i].mode == 2)
                                {
                                    vlan->entryarry[j].untagPortMask |= v_portlist;  
                                }
                                vlan->entryarry[j].allPortMask |= v_portlist;
                                vlan->onuVlanEntryNum++;
                            }
                        }
                        else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID_PORT)
                        {
                            for(j=0;j<vlan->onuVlanEntryNum;j++)
                            {
                                if(vlan->entryarry[j].vlanid == p->rules.rule[i].begin_vid)
                                    break;
                            }
                            if(j < vlan->onuVlanEntryNum)
                            {
                                if(p->rules.rule[i].mode == 2)
                                {                                        
                                    vlan->entryarry[j].untagPortMask |= p->rules.rule[i].portlist & v_portlist;                                                                            
                                }
                                vlan->entryarry[j].allPortMask |= p->rules.rule[i].portlist & v_portlist;
                            }
                            else
                            {
                                k= vlan->onuVlanEntryNum;
                                vlan->entryarry[k].vlanid = p->rules.rule[i].begin_vid;
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[k].untagPortMask |= p->rules.rule[i].portlist & v_portlist;     
                                vlan->entryarry[k].allPortMask |= p->rules.rule[i].portlist & v_portlist;
                                vlan->onuVlanEntryNum++;
                            }
                        }
                        else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE)
                        {
                            for(j=0;j<vlan->onuVlanEntryNum;j++)
                            {
                                if(vlan->entryarry[j].vlanid == v_vid)
                                    break;
                            }
                            if(j < vlan->onuVlanEntryNum)
                            {
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[j].untagPortMask |= v_portlist;                                                                            
                                vlan->entryarry[j].allPortMask |= v_portlist;
                            }
                            else
                            {
                                k= vlan->onuVlanEntryNum;
                                vlan->entryarry[k].vlanid = v_vid;
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[k].untagPortMask |= v_portlist;     
                                vlan->entryarry[k].allPortMask |= v_portlist;
                                vlan->onuVlanEntryNum++;
                            }
                        }
                        else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_PORT)
                        {
                            int t_vid = v_vid;
                            for(m=0;m<fenum;m++)
                            {
                                if(t_vid >= (v_vid + p->rules.rule[i].step))
                                    t_vid = v_vid;
                                
                                for(j=0;j<vlan->onuVlanEntryNum;j++)
                                {
                                    if(vlan->entryarry[j].vlanid == t_vid)
                                        break;
                                }
                                if(j < vlan->onuVlanEntryNum)
                                {
                                    if(p->rules.rule[i].mode == 2)
                                        vlan->entryarry[j].untagPortMask |= 1<<m;                                                                            
                                    vlan->entryarry[j].allPortMask |= 1<<m;
                                }
                                else
                                {
                                    k= vlan->onuVlanEntryNum;
                                    vlan->entryarry[k].vlanid = t_vid;
                                    if(p->rules.rule[i].mode == 2)
                                        vlan->entryarry[k].untagPortMask |= 1<<m;     
                                    vlan->entryarry[k].allPortMask |= 1<<m;
                                    vlan->onuVlanEntryNum++;
                                }  
                                t_vid += p->rules.rule[i].port_step;                 
                            }
                        }
                        else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_ONEPORT)
                        {
                            for(j=0;j<vlan->onuVlanEntryNum;j++)
                            {
                                if(vlan->entryarry[j].vlanid == v_vid)
                                    break;
                            }
                            if(j < vlan->onuVlanEntryNum)
                            {
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[j].untagPortMask |= p->rules.rule[i].portlist;                                                                            
                                vlan->entryarry[j].allPortMask |= p->rules.rule[i].portlist;
                            }
                            else
                            {
                                k= vlan->onuVlanEntryNum;
                                vlan->entryarry[k].vlanid = v_vid;
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[k].untagPortMask |= p->rules.rule[i].portlist;     
                                vlan->entryarry[k].allPortMask |= p->rules.rule[i].portlist;
                                vlan->onuVlanEntryNum++;
                            }  
                        }                            
                    }
                }                                
            }
			
		for(i=1;i<vlan->onuVlanEntryNum;i++)
		{
			if(vlan->entryarry[i].untagPortMask)
			{
				vlan->entryarry[0].untagPortMask &= ~(vlan->entryarry[i].untagPortMask);
				vlan->entryarry[0].allPortMask &= ~(vlan->entryarry[i].untagPortMask);
			}
		}
			
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE;

    return ret;

}
int gpon_get_onuconf_vlanCfgByname(ONUConfigData_t *p, short int PonPortIdx, short int OnuIdx, short int *vlan_mode, ONUVlanConf_t *vlan)
{
    int i = 0,ret = RERROR, k =0;
    short int ulSlot = GetCardIdxByPonChip(PonPortIdx);
    short int ulPort = GetPonPortByPonChip(PonPortIdx);
    short int OnuId = OnuIdx + 1;
    ULONG deviceIndex = MAKEDEVID(ulSlot, ulPort, OnuId);    
    ULONG fenum = 0, v_portlist = 0;

    VOS_MemZero(vlan, sizeof(ONUVlanConf_t));
    if(getDeviceCapEthPortNum(deviceIndex, &fenum) == VOS_OK)
    {    
        for(k=0;k<fenum;k++)
        v_portlist |= (1<<k);
    }
    ONU_CONF_SEM_TAKE
    {
        if (p)
        {
            if(p->vlanconf.vmtrunkportmask &&
            		!p->vlanconf.vmtranslationportmask &&
            		!p->vlanconf.vmtransparentportmask &&
            		!p->vlanconf.vmtagportmask &&
            		!p->vlanconf.vmaggportmask)
    		{    	
                *vlan_mode = 1;
            }
            else
            {
                *vlan_mode = 0;
            }

            vlan->vmtrunkportmask = p->vlanconf.vmtrunkportmask;
            vlan->vmtranslationportmask = p->vlanconf.vmtranslationportmask;
            vlan->vmtransparentportmask = p->vlanconf.vmtransparentportmask;
            vlan->vmtagportmask = p->vlanconf.vmtagportmask;
            vlan->vmaggportmask = p->vlanconf.vmaggportmask;
            
            vlan->onuVlanEntryNum = p->vlanconf.onuVlanEntryNum;
            for (i = 0; i < p->vlanconf.onuVlanEntryNum; i++)
            {
                vlan->entryarry[i].vlanid = p->vlanconf.entryarry[i].vlanid;
                vlan->entryarry[i].allPortMask = (p->vlanconf.entryarry[i].allPortMask)&v_portlist;
                vlan->entryarry[i].untagPortMask = (p->vlanconf.entryarry[i].untagPortMask)&v_portlist;
            }

            if(ulSlot&&ulPort&&OnuId)
            {
                int j = 0, m = 0, valid_flag = 0;
                int onu_num = 0;
                short int v_vid = 0;
                short int begin_slot = 0;
                short int begin_port = 0;
                short int begin_onuid = 0;
                short int end_slot = 0;
                short int end_port = 0;
                short int end_onuid = 0;

                for(i=0;i<p->rules.number_rules;i++)
                {
                    if(p->rules.rule[i].rule_mode)
                    {
                        begin_slot = GET_PONSLOT(p->rules.rule[i].begin_onuid);
                        begin_port = GET_PONPORT(p->rules.rule[i].begin_onuid);
                        begin_onuid = GET_ONUID(p->rules.rule[i].begin_onuid);
                        end_slot = GET_PONSLOT(p->rules.rule[i].end_onuid);
                        end_port = GET_PONPORT(p->rules.rule[i].end_onuid);
                        end_onuid = GET_ONUID(p->rules.rule[i].end_onuid);
                        
                        /*检查onu是否在范围之内*/
                        onu_num = Get_OnuNumFromOnuRange(ulSlot,ulPort,OnuId,end_slot,end_port,end_onuid);
                        if(onu_num == VOS_ERROR)
                            continue;
                        onu_num = Get_OnuNumFromOnuRange(begin_slot,begin_port,begin_onuid,ulSlot,ulPort,OnuId);
                        if(onu_num == VOS_ERROR)
                            continue;
                        
                        if(!valid_flag)
                            valid_flag = 1;
                        
                        v_vid   = p->rules.rule[i].begin_vid + (onu_num - 1)*p->rules.rule[i].step;
                        if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID)
                        {
                            for(j=0;j<vlan->onuVlanEntryNum;j++)
                            {
                                if(vlan->entryarry[j].vlanid == p->rules.rule[i].begin_vid)
                                    break;
                            }
                            if(j < vlan->onuVlanEntryNum)
                            {                                    
                                if(p->rules.rule[i].mode == 2)
                                {
                                    vlan->entryarry[j].untagPortMask |= v_portlist;  
                                }
                                vlan->entryarry[j].allPortMask |= v_portlist;
                            }
                            else
                            {
                                k= vlan->onuVlanEntryNum;
                                vlan->entryarry[k].vlanid = p->rules.rule[i].begin_vid;
                                if(p->rules.rule[i].mode == 2)
                                {
                                    vlan->entryarry[j].untagPortMask |= v_portlist;  
                                }
                                vlan->entryarry[j].allPortMask |= v_portlist;
                                vlan->onuVlanEntryNum++;
                            }
                        }
                        else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID_PORT)
                        {
                            for(j=0;j<vlan->onuVlanEntryNum;j++)
                            {
                                if(vlan->entryarry[j].vlanid == p->rules.rule[i].begin_vid)
                                    break;
                            }
                            if(j < vlan->onuVlanEntryNum)
                            {
                                if(p->rules.rule[i].mode == 2)
                                {                                        
                                    vlan->entryarry[j].untagPortMask |= p->rules.rule[i].portlist & v_portlist;                                                                            
                                }
                                vlan->entryarry[j].allPortMask |= p->rules.rule[i].portlist & v_portlist;
                            }
                            else
                            {
                                k= vlan->onuVlanEntryNum;
                                vlan->entryarry[k].vlanid = p->rules.rule[i].begin_vid;
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[k].untagPortMask |= p->rules.rule[i].portlist & v_portlist;     
                                vlan->entryarry[k].allPortMask |= p->rules.rule[i].portlist & v_portlist;
                                vlan->onuVlanEntryNum++;
                            }
                        }
                        else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE)
                        {
                            for(j=0;j<vlan->onuVlanEntryNum;j++)
                            {
                                if(vlan->entryarry[j].vlanid == v_vid)
                                    break;
                            }
                            if(j < vlan->onuVlanEntryNum)
                            {
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[j].untagPortMask |= v_portlist;                                                                            
                                vlan->entryarry[j].allPortMask |= v_portlist;
                            }
                            else
                            {
                                k= vlan->onuVlanEntryNum;
                                vlan->entryarry[k].vlanid = v_vid;
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[k].untagPortMask |= v_portlist;     
                                vlan->entryarry[k].allPortMask |= v_portlist;
                                vlan->onuVlanEntryNum++;
                            }
                        }
                        else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_PORT)
                        {
                            int t_vid = v_vid;
                            for(m=0;m<fenum;m++)
                            {
                                if(t_vid >= (v_vid + p->rules.rule[i].step))
                                    t_vid = v_vid;
                                
                                for(j=0;j<vlan->onuVlanEntryNum;j++)
                                {
                                    if(vlan->entryarry[j].vlanid == t_vid)
                                        break;
                                }
                                if(j < vlan->onuVlanEntryNum)
                                {
                                    if(p->rules.rule[i].mode == 2)
                                        vlan->entryarry[j].untagPortMask |= 1<<m;                                                                            
                                    vlan->entryarry[j].allPortMask |= 1<<m;
                                }
                                else
                                {
                                    k= vlan->onuVlanEntryNum;
                                    vlan->entryarry[k].vlanid = t_vid;
                                    if(p->rules.rule[i].mode == 2)
                                        vlan->entryarry[k].untagPortMask |= 1<<m;     
                                    vlan->entryarry[k].allPortMask |= 1<<m;
                                    vlan->onuVlanEntryNum++;
                                }  
                                t_vid += p->rules.rule[i].port_step;                 
                            }
                        }
                        else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_ONEPORT)
                        {
                            for(j=0;j<vlan->onuVlanEntryNum;j++)
                            {
                                if(vlan->entryarry[j].vlanid == v_vid)
                                    break;
                            }
                            if(j < vlan->onuVlanEntryNum)
                            {
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[j].untagPortMask |= p->rules.rule[i].portlist;                                                                            
                                vlan->entryarry[j].allPortMask |= p->rules.rule[i].portlist;
                            }
                            else
                            {
                                k= vlan->onuVlanEntryNum;
                                vlan->entryarry[k].vlanid = v_vid;
                                if(p->rules.rule[i].mode == 2)
                                    vlan->entryarry[k].untagPortMask |= p->rules.rule[i].portlist;     
                                vlan->entryarry[k].allPortMask |= p->rules.rule[i].portlist;
                                vlan->onuVlanEntryNum++;
                            }  
                        }                            
                    }
                }                                
            }

		for(i=1;i<vlan->onuVlanEntryNum;i++)
		{
			if(vlan->entryarry[i].untagPortMask)
			{
				vlan->entryarry[0].untagPortMask &= ~(vlan->entryarry[i].untagPortMask);
				vlan->entryarry[0].allPortMask &= ~(vlan->entryarry[i].untagPortMask);
			}
		}
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE;

    return ret;

}

#endif
#if 1
int del_onuconf_vlan(SHORT ponid, SHORT onuid, USHORT vid)
{
	int i = 0,j = 0, k=0, ret = RERROR;

	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

	ONU_CONF_SEM_TAKE
	{
	    ONUConfigData_t *p = getOnuConfFromHashBucket(
	            ONU_CONF_NAME_PTR_GET(ponid, onuid));

        if (p)
        {
                int VlanentryNum = p->vlanconf.onuVlanEntryNum;

                for (i = 0; i < VlanentryNum; i++)
                {
                    if (vid == p->vlanconf.entryarry[i].vlanid)
                    {

                        for(k=0; k<ONU_MAX_PORT; k++)
                        {
                            if(p->vlanconf.entryarry[i].untagPortMask & (1<<k))
                                setOnuConfPortSimpleVar(ponid, onuid, k+1, sv_enum_port_default_vid, 1);
                        }

                        for (j = i; j < VlanentryNum - 1; j++)
                        {
                            p->vlanconf.entryarry[j].allPortMask
                                    = p->vlanconf.entryarry[j + 1].allPortMask;
                            p->vlanconf.entryarry[j].untagPortMask
                                    = p->vlanconf.entryarry[j + 1].untagPortMask;
                            p->vlanconf.entryarry[j].vlanid
                                    = p->vlanconf.entryarry[j + 1].vlanid;
                        }
                        /*added by luh 2013-2-19, 由于最后一个没有清空造成异常(在show profile相同时产生私有文件)*/
                        VOS_MemZero(&(p->vlanconf.entryarry[j]), sizeof(ONUVlanEntry_t));                        
                        p->vlanconf.onuVlanEntryNum--;
                        ret = ROK;
                        break;
                    }
                }
        }
    }
	ONU_CONF_SEM_GIVE

	return ret;
}

int del_onuconf_vlanByPtr(int suffix, void *pdata, USHORT vid)
{
    int i = 0,j = 0, k=0,ret = RERROR;
    
    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
              ret = VOS_ERROR;
        else
       {
                ONUConfigData_t * p = (ONUConfigData_t *)pdata;
               if (p)
                {
                    int VlanentryNum = p->vlanconf.onuVlanEntryNum;

                    for (i = 0; i < VlanentryNum; i++)
                    {
                        if (vid == p->vlanconf.entryarry[i].vlanid)
                        {

                            for(k=0; k<ONU_MAX_PORT; k++)
                            {
                                if(p->vlanconf.entryarry[i].untagPortMask & (1<<k))
                                    setOnuConfPortSimpleVarByPtr(suffix, pdata, k+1, sv_enum_port_default_vid, 1);
                            }

                            for (j = i; j < VlanentryNum - 1; j++)
                            {
                                p->vlanconf.entryarry[j].allPortMask
                                        = p->vlanconf.entryarry[j + 1].allPortMask;
                                p->vlanconf.entryarry[j].untagPortMask
                                        = p->vlanconf.entryarry[j + 1].untagPortMask;
                                p->vlanconf.entryarry[j].vlanid
                                        = p->vlanconf.entryarry[j + 1].vlanid;
                            }
                            /*added by luh 2013-12-04, 添加、删除vlan后，vlan信息没有清空*/
                            VOS_MemZero(&p->vlanconf.entryarry[VlanentryNum - 1], sizeof(ONUVlanEntry_t));
                            p->vlanconf.onuVlanEntryNum--;
                            ret = ROK;
                            break;
                        }
                    }
               }
        }
    }
    ONU_CONF_SEM_GIVE;
    return ret;

}

int 
getOnuVlanportIsolateEnable(USHORT ponid, USHORT onuid)
{
	int ret = 0;

	ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
	int ulRet = VOS_SemTake(g_onuConfSemId,WAIT_FOREVER);

	if(p&&(ulRet == VOS_OK))
		ret = p->vlanconf.portIsolateEnable;
	VOS_SemGive(g_onuConfSemId);
	return ret;
}
int 
SetOnuVlanportIsolateEnable(USHORT ponid, USHORT onuid,int num)
{
	int ret = 0;
	ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
	int ulRet = VOS_SemTake(g_onuConfSemId,WAIT_FOREVER);

	if(p&&(ulRet == VOS_OK))
		{
			p->vlanconf.portIsolateEnable=num;
			ret = 1;
		}
	VOS_SemGive(g_onuConfSemId);
	return ret;
}
 
ULONG onuconf_get_VlanEntryNum(USHORT ponid, USHORT onuid)
{
	ULONG ret = 0;
	
	ONU_CONF_SEM_TAKE
	{
		ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

		if(p)
			ret = p->vlanconf.onuVlanEntryNum;
	}
	ONU_CONF_SEM_GIVE

	return ret;
}

ULONG onuconf_get_VlanEntryNumByPtr(int suffix, void *pdata)
{
    ULONG ret = 0;

    ONU_CONF_SEM_TAKE
    {
        if (onuConfCheckByPtr(suffix, pdata) == VOS_OK)
        {
            ONUConfigData_t * p = (ONUConfigData_t *)pdata;
    		if(p)
    			ret = p->vlanconf.onuVlanEntryNum;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int onuconf_get_vlanEntryByIdx(USHORT ponid, USHORT onuid, ULONG idx,
		int *vid, ULONG *allmask, ULONG *untagmask)
{
	int ret = VOS_ERROR;

	ONU_CONF_SEM_TAKE
	{
		ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
		if(p)
		{
			int num = p->vlanconf.onuVlanEntryNum;
			if(idx < num)
			{
				*vid = p->vlanconf.entryarry[idx].vlanid;
				*allmask = p->vlanconf.entryarry[idx].allPortMask;
				*untagmask = p->vlanconf.entryarry[idx].untagPortMask;
				ret = VOS_OK;
			}
		}
	}
	ONU_CONF_SEM_GIVE

	return ret;
}
/*add by shixh */
/*vlan transation*/
int onuconf_get_vlanEntryByIdxByPtr(int suffix, void *pdata,ULONG idx,
		int *vid, ULONG *allmask, ULONG *untagmask)
{
    int ret = VOS_ERROR;

        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t * p = (ONUConfigData_t *)pdata;
                if(p)
		        {
			        int num = p->vlanconf.onuVlanEntryNum;
			        if(idx < num)
			        {
				        *vid = p->vlanconf.entryarry[idx].vlanid;
				        *allmask = p->vlanconf.entryarry[idx].allPortMask;
				        *untagmask = p->vlanconf.entryarry[idx].untagPortMask;
				        ret = VOS_OK;
			        }
		        }
            }
        }
        ONU_CONF_SEM_GIVE;
   
    return ret;
}
int
set_OnuConf_Ctc_EthPortVlanTranNewVid(SHORT ponid, SHORT onuid, USHORT port, ULONG inVid, ULONG newVid)
{
    int ret = RERROR, i = 0;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if (ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (p)
            {
                ONUPortConf_t *opc = &p->portconf[port - 1];
                ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                /* for(j=0;j<VlanentryNum;j++)*/
                {
                    /* if(inVid == p->vlanconf.entryarry[j].vlanid)*/
                    {
                        int fb = -1, fexist = 0;
                        for (i = 0; i < ONU_MAX_VLAN_TRANS; i++)
                        {
                            if (pvc->extEntry.transArry[i].ovid == 0)
                            {
                                if(fb == -1)
                                    fb = i;

                            }
                            else if(pvc->extEntry.transArry[i].ovid == inVid)
                                fexist = 1;
                            else
                                continue;
                        }

                        if(!fexist && fb != -1)
                        {
                            pvc->extEntry.transArry[fb].ovid = inVid;
                            pvc->extEntry.transArry[fb].svid = newVid;
                            pvc->numberOfEntry++;
                            ret = VOS_OK;
                        }

                    }
                }

            }
        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int
set_OnuConf_Ctc_EthPortVlanTranNewVidByPtr(int suffix, void *pdata, USHORT port, ULONG inVid, ULONG newVid)
{
    int ret = RERROR, i = 0;

    if(ONU_PORT_VALID(port))
    {

        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t * p = (ONUConfigData_t *)pdata;
                if (p)
                {
                    ONUPortConf_t *opc = &p->portconf[port - 1];
                    ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                    /* for(j=0;j<VlanentryNum;j++)*/
                    {
                        /* if(inVid == p->vlanconf.entryarry[j].vlanid)*/
                        {
                            int fb = -1, fexist = 0;
                            for (i = 0; i < ONU_MAX_VLAN_TRANS; i++)
                            {
                                if (pvc->extEntry.transArry[i].ovid == 0)
                                {
                                    if(fb == -1)
                                        fb = i;

                                }
                                else if(pvc->extEntry.transArry[i].ovid == inVid)
                                    fexist = 1;
                                else
                                    continue;
                            }

                            if(!fexist && fb != -1)
                            {
                                pvc->extEntry.transArry[fb].ovid = inVid;
                                pvc->extEntry.transArry[fb].svid = newVid;
                                pvc->numberOfEntry++;
                                ret = VOS_OK;
                            }

                        }
                    }

                }
            }
        }
        ONU_CONF_SEM_GIVE

    }
    else
        ret = VOS_ERROR;

    return ret;
}

int
del_OnuConf_Ctc_EthPortVlanTranNewVid(SHORT ponid, SHORT onuid, USHORT port, ULONG inVid)
{
    int ret = RERROR, i = 0;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if (ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (p)
            {
                ONUPortConf_t *opc = &p->portconf[port - 1];
                ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                for (i = 0; i < ONU_MAX_VLAN_TRANS; i++)
                {
                    if (pvc->extEntry.transArry[i].ovid == inVid)
                    {
                        int j;
                        ONUVlanTransEntry_t *pe = pvc->extEntry.transArry;
                        for(j=i;j<ONU_MAX_VLAN_TRANS-1; j++)
                            pe[j] = pe[j+1];
                        if(pvc->numberOfEntry)
                            pvc->numberOfEntry--;
                        ret = VOS_OK;
                        break;
                    }
                }

            }
        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int
del_OnuConf_Ctc_EthPortVlanTranNewVidByPtr(int suffix, void *pdata, USHORT port, ULONG oldvid)
{
    int ret = RERROR, i = 0;
    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t * p = (ONUConfigData_t *)pdata;
                if (p)
                {
                    ONUPortConf_t *opc = &p->portconf[port - 1];
                    ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                    for (i = 0; i < ONU_MAX_VLAN_TRANS; i++)
                    {
                        if (pvc->extEntry.transArry[i].ovid == oldvid)
                        {
                            pvc->extEntry.transArry[i].ovid = 0;
                            pvc->extEntry.transArry[i].svid = 0;

                            if(pvc->numberOfEntry)
                                pvc->numberOfEntry--;
                            ret = VOS_OK;
                            break;
                        }
                    }

                }
            }
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

/*vlan aggregarion*/
int
set_OnuConf_Ctc_EthPortVlanAgg(SHORT ponid, SHORT onuid, USHORT port, USHORT inVid[8], USHORT targetVid)
{
    int ret = RERROR, i = 0, j;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if (ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (p)
            {
                ONUPortConf_t *opc = &p->portconf[port - 1];
                ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                for (i = 0; i < ONU_MAX_VLAN_AGG_GROUP; i++)
                {

                    if (pvc->targetVlanId[i] == 0)
                    {
                        for (j = 0; j < ONU_MAX_VLAN_AGG_ENTRY; j++)
                        {
                            if (pvc->extEntry.aggArry[i][j] == 0)
                            {
                                pvc->extEntry.aggArry[i][j] = inVid[j];

                                if (pvc->extEntry.aggArry[i][j] != 0)
                                    pvc->onuVlanAggEntryNum[i]++;
                            }
                        }
                        pvc->targetVlanId[i] = targetVid;
                        pvc->numberOfAggTables++;/*新增一组集合，集合组数加1*/
                        ret = VOS_OK;
                        break;
                    }
                    else if (pvc->targetVlanId[i] == targetVid)
                    {
                        for (j = 0; j < ONU_MAX_VLAN_AGG_ENTRY; j++)
                        {

                            pvc->extEntry.aggArry[i][j] = inVid[j];

                            if (pvc->extEntry.aggArry[i][j] != 0)
                                pvc->onuVlanAggEntryNum[i]++;
                        }
                        ret = VOS_OK;
                        /* pvc->numberOfAggTables++;*//*替换原有的一组集合，所以集合组数不变*/
                        break;
                    }
                    else
                    {
                        /*未找到有效的空间存放新加的集合组，所以返回失败*/

                    }

                }

            }
        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int
set_OnuConf_Ctc_EthPortVlanAggByPtr(int suffix, void *pdata, USHORT port, USHORT inVid[8], ULONG targetVid)
{
    int ret = RERROR, i = 0, j;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t * p = (ONUConfigData_t *)pdata;
                if (p)
                {
                    ONUPortConf_t *opc = &p->portconf[port - 1];
                    ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                    for (i = 0; i < ONU_MAX_VLAN_AGG_GROUP; i++)
                    {

                        if (pvc->targetVlanId[i] == 0)
                        {
                            for (j = 0; j < ONU_MAX_VLAN_AGG_ENTRY; j++)
                            {
                                if (pvc->extEntry.aggArry[i][j] == 0)
                                {
                                    pvc->extEntry.aggArry[i][j] = inVid[j];

                                    if (pvc->extEntry.aggArry[i][j] != 0)
                                        pvc->onuVlanAggEntryNum[i]++;
                                }
                            }
                            pvc->targetVlanId[i] = targetVid;
                            pvc->numberOfAggTables++;/*新增一组集合，集合组数加1*/
                            ret = VOS_OK;
                            break;
                        }
                        else if (pvc->targetVlanId[i] == targetVid)
                        {
                            for (j = 0; j < ONU_MAX_VLAN_AGG_ENTRY; j++)
                            {

                                pvc->extEntry.aggArry[i][j] = inVid[j];

                                if (pvc->extEntry.aggArry[i][j] != 0)
                                    pvc->onuVlanAggEntryNum[i]++;
                            }
                            ret = VOS_OK;
                            /* pvc->numberOfAggTables++;*//*替换原有的一组集合，所以集合组数不变*/
                            break;
                        }
                        else
                        {
                            /*未找到有效的空间存放新加的集合组，所以返回失败*/

                        }

                    }

                }
            }
        }
        ONU_CONF_SEM_GIVE;
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int
del_OnuConf_Ctc_EthPortVlanAgg(SHORT ponid, SHORT onuid, USHORT port, ULONG targetVid)
{
    int ret = RERROR, i, j;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if (ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (p)
            {
                ONUPortConf_t *opc = &p->portconf[port - 1];
                ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                for (i = 0; i < ONU_MAX_VLAN_AGG_GROUP; i++)
                {
                    if (pvc->targetVlanId[i] == targetVid)
                    {
                        for (j = 0; j < ONU_MAX_VLAN_AGG_ENTRY; j++)
                        {
                            pvc->extEntry.aggArry[i][j] = 0;
                            /*pvc->onuVlanAggEntryNum[i]--;*/
                        }
                        pvc->onuVlanAggEntryNum[i] = 0;

                        pvc->targetVlanId[i] = 0;
                        pvc->numberOfAggTables--;
                        ret = VOS_OK;
                        break;
                    }

                }

            }
        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int
del_OnuConf_Ctc_EthPortVlanAggByPtr(int suffix, void *pdata, USHORT port, ULONG targetVid)
{
    int ret = RERROR, i, j;
    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t * p = (ONUConfigData_t *)pdata;
                if (p)
                {
                    ONUPortConf_t *opc = &p->portconf[port - 1];
                    ONUVlanExtConf_t *pvc = &opc->extVlanConf;

                    for (i = 0; i < ONU_MAX_VLAN_AGG_GROUP; i++)
                    {
                        if (pvc->targetVlanId[i] == targetVid)
                        {
                            for (j = 0; j < ONU_MAX_VLAN_AGG_ENTRY; j++)
                            {
                                pvc->extEntry.aggArry[i][j] = 0;
                                /*pvc->onuVlanAggEntryNum[i]--;*/
                            }
                            pvc->onuVlanAggEntryNum[i] = 0;

                            pvc->targetVlanId[i] = 0;
                            pvc->numberOfAggTables--;
                            ret = VOS_OK;
                            continue;
                        }

                    }

                }
            }
        }
        ONU_CONF_SEM_GIVE;
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int set_OnuVlanTagConfigEnable(SHORT ponid, SHORT onuid, SHORT port, int enable)
{
    int ret = VOS_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

        if(pd)
            {
                ONUPortConf_t *opc = (ONUPortConf_t*)&pd->portconf[port-1];
                ONUQinqEntry_t *pvc = &opc->qinqEntry;

                if(pvc->qinqMode != enable)
                {
                    VOS_MemZero(pvc->qinqEntries, sizeof(pvc->qinqEntries));
                    pvc->numberOfEntry = 0;
                }

                pvc->qinqMode = enable;
            }
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int set_OnuVlanTagConfigEnableByPtr(int suffix, void *pdata, short int port, int enable)
{
    int ret = VOS_OK;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t *pd = (ONUConfigData_t*)pdata;
                ONUPortConf_t *opc = (ONUPortConf_t*)&pd->portconf[port-1];
                ONUQinqEntry_t *pvc = &opc->qinqEntry;

                if(pvc->qinqMode != enable)
                {
                    VOS_MemZero(pvc->qinqEntries, sizeof(pvc->qinqEntries));
                    pvc->numberOfEntry = 0;
                }

                pvc->qinqMode = enable;
            }
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int get_OnuVlanTagConfigEnableByPtr(int suffix, void *pdata, SHORT port, int *enable)
{
    int ret = VOS_OK;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
			if(onuConfCheckByPtr(suffix, pdata) == VOS_OK)
			{
            	ONUConfigData_t *pd = (ONUConfigData_t*)pdata;    
                ONUPortConf_t *opc = (ONUPortConf_t*)&pd->portconf[port-1];
                ONUQinqEntry_t *pvc = &opc->qinqEntry;
                
                *enable=pvc->qinqMode;
            }
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int get_OnuVlanTagConfigEnable(SHORT ponid, SHORT onuid, SHORT port, int *enable)
{
    int ret = VOS_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

        if(pd)
            {
                ONUPortConf_t *opc = (ONUPortConf_t*)&pd->portconf[port-1];
                ONUQinqEntry_t *pvc = &opc->qinqEntry;
                
                *enable=pvc->qinqMode;
            }
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int set_OnuVlanAddTag(SHORT ponid, SHORT onuid, SHORT port, ULONG cvlan,ULONG svlan)
{
    int ret = VOS_ERROR;
    int i;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if (ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (pd)
            {
                ONUPortConf_t *opc = (ONUPortConf_t*) &pd->portconf[port - 1];
                ONUQinqEntry_t *pvc = &opc->qinqEntry;

                for (i = 0; i < ONU_MAX_QINQ_GROUP / 2; i++)
                {
                    if (pvc->qinqEntries[2 * i] == 0)
                    {
                        pvc->qinqEntries[2 * i] = cvlan;
                        pvc->qinqEntries[2 * i + 1] = svlan;
                        pvc->numberOfEntry++;
                        ret = VOS_OK;
                        break;
                    }
                }

            }

        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int set_OnuVlanAddTagByPtr(int suffix, void *pdata, SHORT port, ULONG cvlan,ULONG svlan)
{
    int ret = VOS_ERROR;
    int i;

    if (ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if (!onuConfCheckByPtr(suffix, pdata))
            {
                ONUConfigData_t *pd = (ONUConfigData_t*) pdata;

                ONUPortConf_t *opc = (ONUPortConf_t*) &pd->portconf[port - 1];
                ONUQinqEntry_t *pvc = &opc->qinqEntry;

                for (i = 0; i < ONU_MAX_QINQ_GROUP / 2; i++)
                {
                    if (pvc->qinqEntries[2 * i] == 0)
                    {
                        pvc->qinqEntries[2 * i] = cvlan;
                        pvc->qinqEntries[2 * i + 1] = svlan;
                        pvc->numberOfEntry++;
                        ret = VOS_OK;
                        break;
                    }
                }

            }

        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int set_OnuVlanDelTag(SHORT ponid, SHORT onuid, SHORT port, ULONG svlan)
{
    int ret = VOS_ERROR;
    int i;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if (ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (pd)
            {
                ONUPortConf_t *opc = (ONUPortConf_t*) &pd->portconf[port - 1];
                ONUQinqEntry_t *pvc = &opc->qinqEntry;

                for (i = 0; i < ONU_MAX_QINQ_GROUP / 2; i++)
                {
                    if (pvc->qinqEntries[2 * i+1] == svlan)
                    {
                        linux_memmove(&pvc->qinqEntries[2 * i], &pvc->qinqEntries[2 * (i + 1)],
                                sizeof(USHORT) * (ONU_MAX_QINQ_GROUP - 2 * (i + 1)));
                        pvc->numberOfEntry--;

                        ret = VOS_OK;
                        break;
                    }
                }

            }

        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int set_OnuVlanDelTagByPtr(int suffix, void *pdata, SHORT port, ULONG svlan)
{
    int ret = VOS_ERROR;
    int i;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t *pd = (ONUConfigData_t*)pdata;

                ONUPortConf_t *opc = (ONUPortConf_t*)&pd->portconf[port-1];
                ONUQinqEntry_t *pvc = &opc->qinqEntry;

               for(i=0;i<ONU_MAX_QINQ_GROUP/2;i++)
                {
                    if(pvc->qinqEntries[2*i]==svlan)
                        {
                            linux_memmove(&pvc->qinqEntries[2*i], &pvc->qinqEntries[2*(i+1)], sizeof(USHORT)*(ONU_MAX_QINQ_GROUP-2*(i+1)));
                            pvc->numberOfEntry--;

                            ret = VOS_OK;
                            break;
                        }
                }

            }
        }
        ONU_CONF_SEM_GIVE
    }

    return ret;
}

int get_OnuVlanTagConfig(SHORT ponid, SHORT onuid, USHORT port, CTC_STACK_port_qinq_configuration_t * pconf)
{
	int ret = RERROR,i=0,mode=0;
	
	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

	if(ONU_PORT_VALID(port))
	{
	   get_OnuVlanTagConfigEnable(ponid, onuid, port,&mode);

	    ONU_CONF_SEM_TAKE
	    {
	        ONUConfigData_t *p = getOnuConfFromHashBucket(
                    ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if (p)
                {
                    ONUPortConf_t *opc = &p->portconf[port-1];
                    ONUQinqEntry_t *pvc = &opc->qinqEntry;

                    if(pconf)
                        VOS_MemZero(pconf, sizeof(CTC_STACK_port_qinq_configuration_t));

                        pconf->mode = mode;
                  

                    if (pconf->mode == CTC_QINQ_MODE_PER_PORT_C)
                    {
                    pconf->default_vlan=opc->defaultVid&0x0fff;
                        
                    }
                    else if (pconf->mode == CTC_QINQ_MODE_PER_VLAN_C)
                    {
                        pconf->number_of_entries = pvc->numberOfEntry;
                        
                        for(i=0; i<pvc->numberOfEntry; i++)
                        {
                            pconf->vlan_list[2*i] = pvc->qinqEntries[2*i];
                            pconf->vlan_list[2*i+1] = pvc->qinqEntries[2*i+1];
                        }
                    }
                    else if(pconf->mode == CTC_QINQ_MODE_NONE_C)
                    {
                       
                    }
                }

            ret = ROK;
	    }
	    ONU_CONF_SEM_GIVE
	}

	return ret;
}


int get_OnuVlanTagConfigByPtr(int suffix, void *pdata, USHORT port, CTC_STACK_port_qinq_configuration_t * pconf)
{
	int ret = RERROR,i=0,mode=0;

	if(!pconf)
		return ret;
	
	if(ONU_PORT_VALID(port))
	{

	    ONU_CONF_SEM_TAKE
	    {
			if(onuConfCheckByPtr(suffix, pdata) == VOS_OK && 
				get_OnuVlanTagConfigEnableByPtr(suffix, pdata, port,&mode) == VOS_OK)
			{
				
 	        	ONUConfigData_t *p = (ONUConfigData_t*)pdata;

            	if (p)
                {
                    ONUPortConf_t *opc = &p->portconf[port-1];
                    ONUQinqEntry_t *pvc = &opc->qinqEntry;

                    
                    VOS_MemZero(pconf, sizeof(CTC_STACK_port_qinq_configuration_t));

                    pconf->mode = mode;
                  

                    if (pconf->mode == CTC_QINQ_MODE_PER_PORT_C)
                    {
                    	pconf->default_vlan=opc->defaultVid&0x0fff;
                        
                    }
                    else if (pconf->mode == CTC_QINQ_MODE_PER_VLAN_C)
                    {
                        pconf->number_of_entries = pvc->numberOfEntry;
                        
                        for(i=0; i<pvc->numberOfEntry; i++)
                        {
                            pconf->vlan_list[2*i] = pvc->qinqEntries[2*i];
                            pconf->vlan_list[2*i+1] = pvc->qinqEntries[2*i+1];
                        }
                    }
                    else if(pconf->mode == CTC_QINQ_MODE_NONE_C)
                    {
                       
                    }
                }

            	ret = ROK;
	    	}
	    }
	    ONU_CONF_SEM_GIVE
	}

	return ret;
}


int getOnuConfAtuAging(USHORT ponid, USHORT onuid, int *aging)
{
    int ret = VOS_ERROR;

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        if(p)
        {
            *aging = p->atuAging;
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int setOnuConfAtuAging(USHORT ponid, USHORT onuid, int aging)
{
    int ret = VOS_ERROR;

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        if(p)
        {
            p->atuAging = aging;
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int getOnuConfAtuLimit(USHORT ponid, USHORT onuid, int *limit)
{
    int ret = VOS_ERROR;

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        if(p)
        {
            *limit = p->atuLimit;
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int setOnuConfAtuLimit(USHORT ponid, USHORT onuid, int limit)
{
    int ret = VOS_ERROR;

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        if(p)
        {
            p->atuLimit = limit;
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

#if 0
/*begin: 支持管理IP的配置保存，mod by liuyh, 2017-5-4*/
int set_onuconf_mngIpConfig(SHORT ponid, SHORT onuid, ULONG ip, ULONG ipMask, 
    ULONG gw, USHORT cVlan, USHORT sVlan, UCHAR pri)
{
	int ret = VOS_ERROR;
    char ipMaskStr[20] = "";
    UCHAR ipM = 0;

	OLT_ASSERT(ponid);
	ONU_ASSERT(onuid);

    get_ipdotstring_from_long(ipMaskStr, ipMask);
    ipM = get_masklen_from_ipdotstring(ipMaskStr);
    if (ipM == -1)
    {
        ipM = 0;
    }
                
	ONU_CONF_SEM_TAKE
	{
	    ONUConfigData_t *p = getOnuConfFromHashBucket(
	            ONU_CONF_NAME_PTR_GET(ponid, onuid));        
        if (p)
        {
            p->mngIp.ip = ip;
            p->mngIp.ipM = ipM;
            p->mngIp.gw = gw;
            p->mngIp.cVlan = cVlan;
            p->mngIp.sVlan = sVlan;
            p->mngIp.pri = pri;    

            ret = VOS_OK;
        }
	}
	ONU_CONF_SEM_GIVE

	return ret;
}

int set_onuconf_mngIpConfigByPtr(int suffix, void *pdata, ULONG ip, ULONG ipMask, 
    ULONG gw, USHORT cVlan, USHORT sVlan, UCHAR pri)
{
    int i = 0, ret = VOS_ERROR;
    char ipMaskStr[20] = "";
    UCHAR ipM = 0;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = VOS_ERROR;
        else
        {
            ONUConfigData_t * p = (ONUConfigData_t *)pdata;
            if (p)
            {
                get_ipdotstring_from_long(ipMaskStr, ipMask);
                ipM = get_masklen_from_ipdotstring(ipMaskStr);
                if (ipM == -1)
                {
                    ipM = 0;
                }
    
                p->mngIp.ip = ip;
                p->mngIp.ipM = ipM;
                p->mngIp.gw = gw;
                p->mngIp.cVlan = cVlan;
                p->mngIp.sVlan = sVlan;
                p->mngIp.pri = pri;    

                ret = VOS_OK;
            }
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;

}
/*end: mod by liuyh, 2017-5-4*/
#endif 

int showOnuConfQosDscpToQueue(USHORT ponid, USHORT onuid,int num,int num1,int *dscpnum,int *queue)
{
    int ret = VOS_ERROR;
    int i = 0;
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        if(p)
        {
            if(p->qosMap.qosDscpQueue[num][num1])
            {
                for(i=0;i<8;i++)
                {
                    if(p->qosMap.qosDscpQueue[num][num1]&(1<<i))
                    {
                        *dscpnum = num1*8+i;
                        *queue = num;
                        ret = VOS_OK;
                    }
                }
            }           
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int showOnuConfQosPrioToQueue(USHORT ponid, USHORT onuid,int num,int *prio,int *queue)
{
    int ret = VOS_ERROR;
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        if(p)
        {
            if(p->qosMap.queue[num]!=0)
            {
                *prio = num;
                *queue = p->qosMap.queue[num]-1;
                ret = VOS_OK;
            }
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
      
}
int showOnuConfQosReplace(USHORT ponid, USHORT onuid,USHORT port,int num, int *oldprio,int *newprio)
{
    int ret = VOS_ERROR;
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        if(p)
        {
            if(p->portconf[port-1].qosPrioReplace[num]!=0)
            {
                *oldprio = num;
                *newprio = p->portconf[port-1].qosPrioReplace[num]-1;
                ret = VOS_OK;
            }
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
      
}
int onuConfCheckByPtr(int suffix, void *pdata)
{
    int ret = VOS_OK;

    if(suffix >=0 && suffix < g_maxPrimeNumber)
    {
        element_onuconfhashbucket_t *p = (element_onuconfhashbucket_t*) (*(g_onuConfBucket + suffix));
        while(p)
        {
            if(p->confdata == pdata)
                break;
            p = p->next;
        }

        if(!p)
            ret = VOS_ERROR;
    }
    else
        ret = VOS_ERROR;

    return ret;
}
static int sfun_setOnuConfQosDscpToQueue(ONUConfigData_t *pdata, int dscpnum, int queue)
{
    int ret = VOS_OK;
    int i = 0;
    ONUConfigData_t *p = pdata;
    int temp = dscpnum/8;
    int temp1 = dscpnum%8;
    int flag = 0;
    if(dscpnum<0 || dscpnum>63 || queue<0 || queue >7)
        return VOS_ERROR;
    if(p)
    {
        for(i=0;i<8;i++)
        {
            if(p->qosMap.qosDscpQueue[i][temp]&(1<<temp1))
            {
                flag = 1;
                if(i!=queue)
                {
                    p->qosMap.qosDscpQueue[i][temp] &= (~(1<<temp1));
                    p->qosMap.qosDscpQueue[queue][temp]|=(1<<temp1);
                    break;
                }
            }
        }
        if(!flag)
        {
            p->qosMap.qosDscpQueue[queue][temp]|=1<<temp1;
        }
       
    }
    else
        ret = VOS_ERROR;
        
    return ret;
}

int setOnuConfQosDscpToQueue(SHORT ponid, SHORT onuid, int dscpnum, int queue)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_setOnuConfQosDscpToQueue(p, dscpnum, queue);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int setOnuConfQosDscpToQueueByPtr(int suffix, void *pdata, int dscpnum, int queue)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = VOS_ERROR;
        else
            ret = sfun_setOnuConfQosDscpToQueue((ONUConfigData_t*)pdata, dscpnum, queue);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
static int sfun_getOnuConfQosDscpToQueue(ONUConfigData_t *pdata, int dscpnum, int *queue)
{
    int ret = VOS_OK;
    int i = 0;
    int flag = 0;
    ONUConfigData_t *p = pdata;
    int temp = dscpnum/8;
    int temp1 = dscpnum%8;
    if(dscpnum>63 || dscpnum<0)
        return VOS_ERROR;
    if(p)
    {
        for(i=0;i<8;i++)
        {
            if(p->qosMap.qosDscpQueue[i][temp]&(1<<temp1))
            {
                flag = 1;
                *queue = i;
                break;
            }
        }
        if(!flag)
            ret = VOS_ERROR;
    }
    else
        ret = VOS_ERROR;
    return ret;
}

int getOnuConfQosDscpToQueue(SHORT ponid, SHORT onuid, int dscpnum, int *queue)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_getOnuConfQosDscpToQueue(p, dscpnum, queue);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int getOnuConfQosDscpToQueueByPtr(int suffix, void *pdata, int dscpnum, int *queue)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = VOS_ERROR;
        else
            ret = sfun_getOnuConfQosDscpToQueue((ONUConfigData_t*)pdata, dscpnum, queue);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int onuConfCheckByPtrNoSuffix(void *pdata)
{
    int ret = VOS_ERROR;

    int suffix;

    for(suffix = 0; suffix < g_maxPrimeNumber; suffix++)
    {
        element_onuconfhashbucket_t *p = (element_onuconfhashbucket_t*) (*(g_onuConfBucket + suffix));
        while(p)
        {
            if(p->confdata == pdata)
            {
                ret = VOS_OK;
                return ret;
            }
            p = p->next;
        }
    }

    return ret;
}

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
static int sfun_getCmcConfSimpleVar(ONUCmcConf_t *p, int vcode, ULONG *uv)
{
    switch(vcode)
    {
    case sv_enum_cmc_max_cm:
        *uv = p->maxCm;
        break;
    default:
        return VOS_ERROR;
    }

    return VOS_OK;
}
#endif

static int sfun_getOnuConfSimpleVar(ONUConfigData_t *pdata, int vcode, ULONG *uv)
{
    int ret = VOS_OK;

    ONUConfigData_t *p = pdata;

    if(p)
    {
        if ( vcode <= sv_enum_onu_fec_enable )
        {
            switch(vcode)
            {
            case sv_enum_atu_aging:
                *uv = p->atuAging;
            break;
            case sv_enum_atu_limit:
                *uv = p->atuLimit;
                break;
            case sv_enum_igmp_enable:
                *uv = p->igmpEnable;
                break;
            case sv_enum_igmp_auth_enable:
                *uv = p->igmpAuthEnable;
                break;
            case sv_enum_igmp_groupage:
                *uv = p->igmpGroupAgeTime;
                break;
            case sv_enum_igmp_hostage:
                *uv = p->igmpHostAgeTime;
                break;
            case sv_enum_igmp_max_response_time:
                *uv = p->igmpMaxResponseTime;
                break;
            case sv_enum_igmp_max_group:
                *uv = p->igmpMaxGroup;
                break;
            case sv_enum_igmp_fastleave_enable:
                *uv = p->igmpFastLeaveEnable;
                break;
            case sv_enum_port_linkmon_enable:
                *uv = p->linkMonEnable;
                break;
            case sv_enum_port_modemon_enable:
                *uv = p->modeMonEnable;
                break;
            case sv_enum_port_isolate:
                *uv = p->vlanconf.portIsolateEnable;
                break;
            case sv_enum_VlanEntryNum:
                *uv = p->vlanconf.onuVlanEntryNum;
                break;
            case sv_enum_onu_holdover_time:
                *uv = p->holdover;
                break;
            case sv_enum_onu_fec_enable:
                *uv = p->fecenable;
                break;
            case sv_enum_qosAlgorithm:
                *uv = p->qosAlgorithm;
                break;
            case sv_enum_ingressRateLimitBase:
                *uv = p->ingressRateLimitBase;
                break;
            case sv_enum_port_mirror_egress_fromlist:
                *uv = p->egressMirrorFromList;
                break;
            case sv_enum_port_mirror_egress_tolist:
                *uv = p->egressMirrorToList;
                break;
            case sv_enum_port_mirror_ingress_fromlist:
                *uv = p->ingressMirrorFromList;
                break;
            case sv_enum_port_mirror_ingress_tolist:
                *uv = p->ingressMirrorToList;
                break;
            default:
                ret = VOS_ERROR;
                break;
            }
        }
        else
        {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
            ret = sfun_getCmcConfSimpleVar(&p->cmcCfg, vcode, uv);
#else
            ret = VOS_ERROR;
#endif
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int getOnuConfSimpleVar(SHORT ponid, SHORT onuid, int vcode, ULONG *uv)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_getOnuConfSimpleVar(p, vcode, uv);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int getOnuConfSimpleVarByPtr(int suffix, void *pdata, int vcode, ULONG *uv)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if (onuConfCheckByPtr(suffix, pdata))
            ret = VOS_ERROR;
        else
            ret = sfun_getOnuConfSimpleVar((ONUConfigData_t*) pdata, vcode, uv);
    }
    ONU_CONF_SEM_GIVE;

    return ret;
}

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
static int sfun_setCmcConfSimpleVar(ONUCmcConf_t *p, int vcode, ULONG uv)
{
    switch(vcode)
    {
    case sv_enum_cmc_max_cm:
        p->maxCm = (UCHAR)uv;
        break;
    default:
        return VOS_ERROR;
    }

    if ( !p->cmcIsEnabled )
    {
        p->cmcIsEnabled = TRUE;
    }

    return VOS_OK;
}
#endif

static int sfun_setOnuConfSimpleVar(ONUConfigData_t *p, int vcode, ULONG uv)
{
    int ret = VOS_OK;

    if(p)
    {
        if ( vcode <= sv_enum_onu_fec_enable )
        {
            switch(vcode)
            {
            case sv_enum_atu_aging:
                p->atuAging = uv;
            break;
            case sv_enum_atu_limit:
                p->atuLimit = uv;
                break;
            case sv_enum_igmp_enable:
                p->igmpEnable = uv;
                break;
            case sv_enum_igmp_auth_enable:
                p->igmpAuthEnable = uv;
                break;
            case sv_enum_igmp_groupage:
                p->igmpGroupAgeTime = uv;
                break;
            case sv_enum_igmp_hostage:
                p->igmpHostAgeTime = uv;
                break;
            case sv_enum_igmp_max_response_time:
                p->igmpMaxResponseTime = uv;
                break;
            case sv_enum_igmp_max_group:
                p->igmpMaxGroup = uv;
                break;
            case sv_enum_igmp_fastleave_enable:
                p->igmpFastLeaveEnable = uv;
                break;
            case sv_enum_port_linkmon_enable:
                p->linkMonEnable = uv;
                break;
            case sv_enum_port_modemon_enable:
                p->modeMonEnable = uv;
                break;
            case sv_enum_port_isolate:
                p->vlanconf.portIsolateEnable = uv;
                break;
            case sv_enum_VlanEntryNum:
                p->vlanconf.onuVlanEntryNum = uv;
                break;
            case sv_enum_onu_holdover_time:
                p->holdover = uv;
                break;
            case sv_enum_onu_fec_enable:
                p->fecenable = uv;
                break;
            case sv_enum_qosAlgorithm:
                p->qosAlgorithm = uv;
                break;
            case sv_enum_ingressRateLimitBase:
                p->ingressRateLimitBase = uv;
                break;
            case sv_enum_port_mirror_ingress_fromlist:
                p->ingressMirrorFromList = uv;
                break;
            case sv_enum_port_mirror_ingress_tolist:
                p->ingressMirrorToList = uv;;
                break;
            case sv_enum_port_mirror_egress_fromlist:
                p->egressMirrorFromList = uv;
                break;
            case sv_enum_port_mirror_egress_tolist:
                p->egressMirrorToList = uv;
                break;
            default:
                ret = VOS_ERROR;
                break;
            }
        }
        else
        {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
            ret = sfun_setCmcConfSimpleVar(&p->cmcCfg, vcode, uv);
#else
            ret = VOS_ERROR;
#endif
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int setOnuConfSimpleVar(SHORT ponid, SHORT onuid, int vcode, ULONG uv)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_setOnuConfSimpleVar(p, vcode, uv);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int setOnuConfSimpleVarByPtr(int suffix, void *pdata, int vcode, ULONG uv)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = VOS_ERROR;
        else
            ret = sfun_setOnuConfSimpleVar((ONUConfigData_t*)pdata, vcode, uv);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
static int sfun_getCmcConfPortSimpleVar(ONUCmcConf_t *p, USHORT port, int vcode, ULONG *uv)
{
    ULONG ulChannelDir;
    ULONG ulChannelID;
    ULONG ulChannelByteIdx, ulChannelBitIdx;
    ULONG *pulChannelU32;
    UCHAR *pucChannelU8;
    SHORT *psChannelS16;

    if ( CMC_CHANNELID_NONE != (ulChannelID = CMC_PORT2DOWNCHANNEL(port)) )
    {
        /* 下行通道设置 */
        ulChannelDir = 0;        

        if ( 0 < ulChannelID )
        {
            if ( ulChannelID > CMC_DSCHANNEL_NUM )
            {
                ulChannelID = CMC_DSCHANNEL_NUM;
            }
        }
        else
        {
            return VOS_ERROR;
        }
    }
    else
    {
        /* 上行通道设置 */
        ulChannelDir = 1;        
        
        if ( 0 < (ulChannelID = CMC_PORT2UPCHANNEL(port)) )
        {
            if ( ulChannelID > CMC_USCHANNEL_NUM )
            {
                ulChannelID = CMC_USCHANNEL_NUM;
            }
        }
        else
        {
            return VOS_ERROR;
        }
    }

    ulChannelID--;
    
    switch(vcode)
    {
    case sv_enum_cmc_channel_enable:
        if ( ulChannelDir )
        {
            pucChannelU8 = p->aucUsChannelEnable;
        }
        else
        {
            pucChannelU8 = p->aucDsChannelEnable;
        }
        
        ulChannelByteIdx = ulChannelID >> 3;
        ulChannelBitIdx  = 1 << (ulChannelID & 7);
        if ( pucChannelU8[ulChannelByteIdx] & ulChannelBitIdx )
        {
            *uv = TRUE;
        }
        else
        {
            *uv = FALSE;
        }
        break;
    case sv_enum_cmc_channel_annex:
        if ( 0 == ulChannelDir )
        {
            pucChannelU8 = p->aucDsChannelAnnex;
            ulChannelByteIdx = ulChannelID >> 3;
            ulChannelBitIdx  = 1 << (ulChannelID & 7);
            if ( pucChannelU8[ulChannelByteIdx] & ulChannelBitIdx )
            {
                *uv = CMC_CFG_DOWN_CHANNEL_ANNEX_B;
            }
            else
            {
                *uv = CMC_CFG_DOWN_CHANNEL_ANNEX_A;
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_d30:
        if ( ulChannelDir )
        {
            pucChannelU8 = p->aucUsChannelD30;
            ulChannelByteIdx = ulChannelID >> 3;
            ulChannelBitIdx  = 1 << (ulChannelID & 7);
            if ( pucChannelU8[ulChannelByteIdx] & ulChannelBitIdx )
            {
                *uv = TRUE;
            }
            else
            {
                *uv = FALSE;
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_type:
        if ( ulChannelDir )
        {
            pucChannelU8 = p->aucUsChannelType;
            ulChannelByteIdx = ulChannelID >> 3;
            ulChannelBitIdx  = 1 << (ulChannelID & 7);
            if ( pucChannelU8[ulChannelByteIdx] & ulChannelBitIdx )
            {
                *uv = CMC_CFG_UP_CHANNEL_TYPE_ATDMA;
            }
            else
            {
                *uv = CMC_CFG_UP_CHANNEL_TYPE_SCDMA;
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_profile:
        if ( ulChannelDir )
        {
            pucChannelU8 = p->aucUsChannelProfile;
            ulChannelByteIdx = ulChannelID >> 1;
            ulChannelBitIdx = (ulChannelID & 1) << 2;
            *uv = (pucChannelU8[ulChannelByteIdx] >> ulChannelBitIdx) & 0xF;
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_modulation:
        if ( 0 == ulChannelDir )
        {
            pucChannelU8 = p->aucDsChannelModulation;
            ulChannelByteIdx = ulChannelID >> 2;
            ulChannelBitIdx = (ulChannelID & 3) << 1;
            *uv = (pucChannelU8[ulChannelByteIdx] >> ulChannelBitIdx) & 3;
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_interleave:
        if ( 0 == ulChannelDir )
        {
            pucChannelU8 = p->aucDsChannelInterleaver;
            ulChannelByteIdx = ulChannelID >> 1;
            ulChannelBitIdx = (ulChannelID & 1) << 2;
            *uv = (pucChannelU8[ulChannelByteIdx] >> ulChannelBitIdx) & 0xF;
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_power:
        if ( ulChannelDir )
        {
            psChannelS16 = p->asUsChannelPower;
        }
        else
        {
            psChannelS16 = p->asDsChannelPower;
        }
        
        *uv = psChannelS16[ulChannelID];
        break;
    case sv_enum_cmc_channel_width:
        if ( ulChannelDir )
        {
            *uv = p->aulUsChannelFreqWidth[ulChannelID];
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_freq:
        if ( ulChannelDir )
        {
            pulChannelU32 = p->aulUsChannelFreq;
        }
        else
        {
            pulChannelU32 = p->aulDsChannelFreq;
        }
        
        *uv = pulChannelU32[ulChannelID];
        break;
    default:
        return VOS_ERROR;
    }

    return VOS_OK;
}
#endif

static int sfun_getOnuConfPortSimpleVar(ONUConfigData_t *p, USHORT port, int vcode, ULONG *uv)
{
    int ret = VOS_OK;

    if(p)
    {
        ONUPortConf_t *pe = &p->portconf[port-1];

        if ( vcode <= sv_enum_onu_fec_enable )
        {
            switch(vcode)
            {
            case sv_enum_port_enable:
                *uv = pe->enable;
                break;
            case sv_enum_port_fec_enable:
                *uv = pe->fecEnable;
                break;
            case sv_enum_port_mode:
                *uv = pe->mode;
                break;
            case sv_enum_port_atu_flood_enable:
                *uv = pe->atuFloodEnable;
                break;
            case sv_enum_port_atu_learn_enable:
                *uv = pe->atuLearnEnable;
                break;
            case sv_enum_port_default_vid:
                *uv = pe->defaultVid&0x0fff;
                break;
            case sv_enum_port_igmp_fastleave_enable:
                *uv = pe->igmpFastLeaveEnable;
                break;
            case sv_enum_port_igmp_max_group:
                *uv = pe->igmpMaxGroup;
                break;
            case sv_enum_port_igmp_tag_strip:
                *uv = pe->igmpTagStrip;
                break;

            case sv_enum_port_ingress_rate_action:
                *uv = pe->ingressRateAction;
                break;
            case sv_enum_port_ingress_rate_burst:
                *uv = pe->ingressBurstMode;
                break;
            case sv_enum_port_ingress_rate_limit:
                *uv = pe->ingressRateLimit;
                break;
            case sv_enum_port_default_priority:
                *uv = pe->defaultVid>>13;
                break;
            case sv_enum_port_ingress_rate_type:
                *uv = pe->ingressRateType;
                break;
            case sv_enum_port_egress_limit:
                *uv = pe->egressRateLimit;
                break;
            case sv_enum_port_loop_detect_enable:
                *uv = pe->loopDetectEnable;
                break;
            case sv_enum_port_vlan_accept_type:
                *uv = pe->vlanAcceptFrameType;
                break;
            case sv_enum_port_ingress_vlan_filter:
                *uv = pe->vlanIngressFilter;
                break;
            case sv_enum_port_pause_enable:
                *uv = pe->pauseEnable;
                break;
            case sv_enum_port_qoset_idx:
                *uv = pe->qosSetSel;
                break;
            case sv_enum_port_qoset_ip_enable:
                *uv = pe->qosIpEnable;
                break;
            case sv_enum_port_qoset_user_enable:
                *uv = pe->qosUserEnable;
                break;
            case sv_enum_port_qoset_rule:
                *uv = pe->qosRule;
                break;
        case sv_enum_onu_transparent_enable:
            *uv = pe->vlanTransparentEnable;
            break;
            default:
                *uv = 0;
                break;
            }
        }
        else
        {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
            ret = sfun_getCmcConfPortSimpleVar(&p->cmcCfg, port, vcode, uv);
#else
            ret = VOS_ERROR;
#endif
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int getOnuConfPortSimpleVar(SHORT ponid, SHORT onuid, USHORT port, int vcode, ULONG *uv)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
            ret = sfun_getOnuConfPortSimpleVar(p, port, vcode, uv);
        }
        ONU_CONF_SEM_GIVE

    }
    else
        ret = VOS_ERROR;

    return ret;
}

int getOnuConfPortSimpleVarByPtr(int suffix, void *pdata, USHORT port, int vcode, ULONG *uv)
{
    int ret = VOS_OK;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
                ret = sfun_getOnuConfPortSimpleVar((ONUConfigData_t*)pdata, port, vcode, uv);
        }
        ONU_CONF_SEM_GIVE;
    }
    else
        ret = VOS_ERROR;

    return ret;
}

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
static int sfun_setCmcConfPortSimpleVar(ONUCmcConf_t *p, USHORT port, int vcode, ULONG uv)
{
    ULONG ulChannelDir;
    ULONG ulChannelID;
    ULONG ulChannelByteMsk;
    ULONG ulChannelByteIdx;
    ULONG ulChannelBitIdx;
    ULONG *pulChannelU32;
    UCHAR *pucChannelU8;
    SHORT *psChannelS16;

    if ( CMC_CHANNELID_NONE != (ulChannelID = CMC_PORT2DOWNCHANNEL(port)) )
    {
        /* 下行通道设置 */
        ulChannelDir = 0;        

        if ( 0 < ulChannelID )
        {
            if ( ulChannelID > CMC_DSCHANNEL_NUM )
            {
                ulChannelID = CMC_DSCHANNEL_NUM;
            }
        }
    }
    else
    {
        /* 上行通道设置 */
        ulChannelDir = 1;        
        
        if ( 0 < (ulChannelID = CMC_PORT2UPCHANNEL(port)) )
        {
            if ( ulChannelID > CMC_USCHANNEL_NUM )
            {
                ulChannelID = CMC_USCHANNEL_NUM;
            }
        }
    }

    switch(vcode)
    {
    case sv_enum_cmc_channel_enable:
        if ( ulChannelDir )
        {
            pucChannelU8 = p->aucUsChannelEnable;
            ulChannelByteMsk = sizeof(p->aucUsChannelEnable);
        }
        else
        {
            pucChannelU8 = p->aucDsChannelEnable;
            ulChannelByteMsk = sizeof(p->aucDsChannelEnable);
        }
        
        if ( 0 < ulChannelID-- )
        {
            ulChannelByteIdx = ulChannelID >> 3;
            ulChannelBitIdx  = 1 << (ulChannelID & 7);
        
            if ( TRUE == uv )
            {
                pucChannelU8[ulChannelByteIdx] |= ulChannelBitIdx;
            }
            else
            {
                pucChannelU8[ulChannelByteIdx] &= ~ulChannelBitIdx;
            }
        }
        else
        {
            if ( TRUE == uv )
            {
                if ( ulChannelDir )
                {
                    *pucChannelU8 = 0xF;
                }
                else
                {
                    VOS_MemSet(pucChannelU8, 0xFF, ulChannelByteMsk);
                }
            }
            else
            {
                VOS_MemSet(pucChannelU8, 0, ulChannelByteMsk);
            }
        }
        break;
    case sv_enum_cmc_channel_annex:
        if ( ulChannelDir )
        {
            if ( 0 < ulChannelID-- )
            {
                pucChannelU8 = p->aucDsChannelAnnex;
                ulChannelByteIdx = ulChannelID >> 3;
                ulChannelBitIdx  = 1 << (ulChannelID & 7);
            
                if ( CMC_CFG_DOWN_CHANNEL_ANNEX_DEFAULT == uv )
                {
                    pucChannelU8[ulChannelByteIdx] &= ~ulChannelBitIdx;
                }
                else
                {
                    pucChannelU8[ulChannelByteIdx] |= ulChannelBitIdx;
                }
            }
            else
            {
                if ( CMC_CFG_DOWN_CHANNEL_ANNEX_DEFAULT == uv )
                {
                    VOS_MemSet(p->aucDsChannelAnnex, 0, sizeof(p->aucDsChannelAnnex));
                }
                else
                {
                    VOS_MemSet(p->aucDsChannelAnnex, 0xFF, sizeof(p->aucDsChannelAnnex));
                }
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_d30:
        if ( ulChannelDir )
        {
            if ( 0 < ulChannelID-- )
            {
                pucChannelU8 = p->aucUsChannelD30;
                ulChannelByteIdx = ulChannelID >> 3;
                ulChannelBitIdx  = 1 << (ulChannelID & 7);
            
                if ( FALSE == uv )
                {
                    pucChannelU8[ulChannelByteIdx] &= ~ulChannelBitIdx;
                }
                else
                {
                    pucChannelU8[ulChannelByteIdx] |= ulChannelBitIdx;
                }
            }
            else
            {
                if ( FALSE == uv )
                {
                    VOS_MemSet(p->aucUsChannelD30, 0, sizeof(p->aucUsChannelD30));
                }
                else
                {
                    p->aucUsChannelD30[0] = 0xF;
                }
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_type:
        if ( ulChannelDir )
        {
            if ( 0 < ulChannelID-- )
            {
                pucChannelU8 = p->aucUsChannelType;
                ulChannelByteIdx = ulChannelID >> 3;
                ulChannelBitIdx  = 1 << (ulChannelID & 7);
            
                if ( CMC_CFG_UP_CHANNEL_TYPE_DEFAULT == uv )
                {
                    pucChannelU8[ulChannelByteIdx] &= ~ulChannelBitIdx;
                }
                else
                {
                    pucChannelU8[ulChannelByteIdx] |= ulChannelBitIdx;
                }
            }
            else
            {
                if ( CMC_CFG_UP_CHANNEL_TYPE_DEFAULT == uv )
                {
                    VOS_MemSet(p->aucUsChannelType, 0, sizeof(p->aucUsChannelType));
                }
                else
                {
                    p->aucUsChannelType[0] = 0xF;
                }
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_profile:
        if ( ulChannelDir )
        {
            if ( 0 < ulChannelID-- )
            {
                pucChannelU8 = p->aucUsChannelProfile;
                ulChannelByteIdx = ulChannelID >> 1;
                ulChannelBitIdx = (ulChannelID & 1) << 2;
                ulChannelByteMsk = ~(0xF << ulChannelBitIdx);
                pucChannelU8[ulChannelByteIdx] = (pucChannelU8[ulChannelByteIdx] & (UCHAR)ulChannelByteMsk) | (uv << ulChannelBitIdx);
            }
            else
            {
                ulChannelBitIdx = (uv << 4) | (uv & 0xF);
                VOS_MemSet(p->aucUsChannelProfile, (CHAR)ulChannelBitIdx, sizeof(p->aucUsChannelProfile));
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_modulation:
        if ( 0 == ulChannelDir )
        {
            if ( 0 < ulChannelID-- )
            {
                pucChannelU8 = p->aucDsChannelModulation;
                ulChannelByteIdx = ulChannelID >> 2;
                ulChannelBitIdx = (ulChannelID & 3) << 1;
                ulChannelByteMsk = ~(3 << ulChannelBitIdx);
                pucChannelU8[ulChannelByteIdx] = (pucChannelU8[ulChannelByteIdx] & (UCHAR)ulChannelByteMsk) | (uv << ulChannelBitIdx);
            }
            else
            {
                ulChannelBitIdx = (uv << 6) | (uv << 4) | (uv << 2) | (uv & 3);
                VOS_MemSet(p->aucDsChannelModulation, (CHAR)ulChannelBitIdx, sizeof(p->aucDsChannelModulation));
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_interleave:
        if ( 0 == ulChannelDir )
        {
            if ( 0 < ulChannelID-- )
            {
                pucChannelU8 = p->aucDsChannelInterleaver;
                ulChannelByteIdx = ulChannelID >> 1;
                ulChannelBitIdx = (ulChannelID & 1) << 2;
                ulChannelByteMsk = ~(0xF << ulChannelBitIdx);
                pucChannelU8[ulChannelByteIdx] = (pucChannelU8[ulChannelByteIdx] & (UCHAR)ulChannelByteMsk) | (uv << ulChannelBitIdx);
            }
            else
            {
                ulChannelBitIdx = (uv << 4) | (uv & 0xF);
                VOS_MemSet(p->aucDsChannelInterleaver, (CHAR)ulChannelBitIdx, sizeof(p->aucDsChannelInterleaver));
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_power:
        if ( ulChannelDir )
        {
            psChannelS16 = p->asUsChannelPower;
            ulChannelByteMsk = CMC_USCHANNEL_NUM;
        }
        else
        {
            psChannelS16 = p->asDsChannelPower;
            ulChannelByteMsk = CMC_DSCHANNEL_NUM;
        }
        
        if ( 0 < ulChannelID )
        {
            ulChannelByteMsk = ulChannelID;
            ulChannelByteIdx = ulChannelID - 1;
        }
        else
        {
            ulChannelByteIdx = 0;
        }

        for (; ulChannelByteIdx < ulChannelByteMsk; ulChannelByteIdx++)
        {
            psChannelS16[ulChannelByteIdx] = (SHORT)uv;
        }
        
        break;
    case sv_enum_cmc_channel_width:
        if ( ulChannelDir )
        {
            pulChannelU32 = p->aulUsChannelFreqWidth;
            ulChannelByteMsk = CMC_USCHANNEL_NUM;
            
            if ( 0 < ulChannelID )
            {
                ulChannelByteMsk = ulChannelID;
                ulChannelByteIdx = ulChannelID - 1;
            }
            else
            {
                ulChannelByteIdx = 0;
            }

            for (; ulChannelByteIdx < ulChannelByteMsk; ulChannelByteIdx++)
            {
                pulChannelU32[ulChannelByteIdx] = uv;
            }
        }
        else
        {
            VOS_ASSERT(0);
            return VOS_ERROR;
        }
        
        break;
    case sv_enum_cmc_channel_freq:
        if ( ulChannelDir )
        {
            pulChannelU32 = p->aulUsChannelFreq;
            ulChannelByteMsk = CMC_USCHANNEL_NUM;
        }
        else
        {
            pulChannelU32 = p->aulDsChannelFreq;
            ulChannelByteMsk = CMC_DSCHANNEL_NUM;
        }

        if ( 0 < ulChannelID-- )
        {
            pulChannelU32[ulChannelID] = uv;
        }
        else
        {
            ULONG *pulValues;
            LONG  lFreqStep;

            if ( NULL != (pulValues = (ULONG*)uv) )
            {
                if ( CMC_CFG_UP_CHANNEL_FREQ_DIR_DECREASE != pulValues[2] )
                {
                    lFreqStep = pulValues[1];
                }
                else
                {
                    lFreqStep = -pulValues[1];
                }

                pulChannelU32[0] = pulValues[0];
                for (ulChannelByteIdx = 1; ulChannelByteIdx < ulChannelByteMsk; ulChannelByteIdx++)
                {
                    pulChannelU32[ulChannelByteIdx] = pulChannelU32[ulChannelByteIdx - 1] + lFreqStep;
                }
            }
            else
            {
                VOS_ASSERT(0);
                return VOS_ERROR;
            }
        }
        
        break;
    default:
        return VOS_ERROR;
    }

    if ( !p->cmcIsEnabled )
    {
        p->cmcIsEnabled = TRUE;
    }

    return VOS_OK;
}
#endif

static int sfun_setOnuConfPortSimpleVar(ONUConfigData_t *p, USHORT port, int vcode, ULONG uv)
{
    int ret = VOS_OK;

    if(p)
    {
        ONUPortConf_t *pe = &p->portconf[port-1];
        if ( vcode <= sv_enum_onu_fec_enable )
        {
            switch(vcode)
            {
            case sv_enum_port_enable:
                pe->enable = uv;
                break;
            case sv_enum_port_fec_enable:
                pe->fecEnable = uv;
                break;
            case sv_enum_port_mode:
                pe->mode = uv;
                break;
            case sv_enum_port_atu_flood_enable:
                pe->atuFloodEnable = uv;
                break;
            case sv_enum_port_atu_learn_enable:
                pe->atuLearnEnable = uv;
                break;
            case sv_enum_port_default_vid:
                pe->defaultVid &= 0xfffff000;
                pe->defaultVid |= uv;
                break;
            case sv_enum_port_default_priority:
                pe->defaultVid &= 0x1fff;
                pe->defaultVid |= uv<<13;
                break;
            case sv_enum_port_igmp_fastleave_enable:
                pe->igmpFastLeaveEnable = uv;
                break;
            case sv_enum_port_igmp_max_group:
                pe->igmpMaxGroup = uv;
                break;
            case sv_enum_port_igmp_tag_strip:
                pe->igmpTagStrip = uv;
                break;
            case sv_enum_port_ingress_rate_action:
                pe->ingressRateAction = uv;
                break;
            case sv_enum_port_ingress_rate_burst:
                pe->ingressBurstMode = uv;
                break;
            case sv_enum_port_ingress_rate_limit:
                pe->ingressRateLimit = uv;
                break;
            case sv_enum_port_ingress_rate_type:
                pe->ingressRateType = uv;
                break;
            case sv_enum_port_egress_limit:
                pe->egressRateLimit = uv;
                break;
            case sv_enum_port_loop_detect_enable:
                pe->loopDetectEnable = uv;
                break;
            case sv_enum_port_vlan_accept_type:
                pe->vlanAcceptFrameType = uv;
                break;
            case sv_enum_port_ingress_vlan_filter:
                pe->vlanIngressFilter = uv;
                break;
            case sv_enum_port_pause_enable:
                pe->pauseEnable = uv;
                break;
            case sv_enum_port_qoset_idx:
                pe->qosSetSel = uv;
                break;
            case sv_enum_port_qoset_ip_enable:
                pe->qosIpEnable = uv;
                break;
            case sv_enum_port_qoset_user_enable:
                pe->qosUserEnable = uv;
                break;
            case sv_enum_port_qoset_rule:
                pe->qosRule = uv;
                break;
        case sv_enum_onu_transparent_enable:
            pe->vlanTransparentEnable = uv;
            break;
            default:
                ret = VOS_ERROR;
                break;
            }
        }
        else
        {
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
            ret = sfun_setCmcConfPortSimpleVar(&p->cmcCfg, port, vcode, uv);
#else
            ret = VOS_ERROR;
#endif
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int setOnuConfPortSimpleVar(SHORT ponid, SHORT onuid, USHORT port, int vcode, ULONG uv)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
            ret = sfun_setOnuConfPortSimpleVar(p, port, vcode, uv);
        }
        ONU_CONF_SEM_GIVE

    }
    else
        ret = VOS_ERROR;

    return ret;
}

int setOnuConfPortSimpleVarByPtr(int suffix, void *pdata, USHORT port, int vcode, ULONG uv)
{
    int ret = VOS_OK;

    if(ONU_PORT_VALID(port))
    {

        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
                ret = sfun_setOnuConfPortSimpleVar((ONUConfigData_t*)pdata, port, vcode, uv);
        }
        ONU_CONF_SEM_GIVE

    }
    else
        ret = VOS_ERROR;

    return ret;
}
/*added by luh 2014-02-27*/
static sfun_OnuConfDelVlanPort(ONUConfigData_t *p, USHORT port)
{
    if(p && port<=ONU_MAX_PORT && port)
    {
        if (p->vlanconf.onuVlanEntryNum>1)
        {
            int gCount = 0;
            for(gCount=1; gCount<p->vlanconf.onuVlanEntryNum;gCount++)
            {
                if(p->vlanconf.entryarry[gCount].vlanid != 1)
                {
                    /*查询该vlan是否含有指定的端口*/
                    if(p->vlanconf.entryarry[gCount].allPortMask & (1<<port-1))
                    {
                        p->vlanconf.entryarry[gCount].allPortMask &= ~(1<<port-1);
                        if(p->vlanconf.entryarry[gCount].untagPortMask & (1<<port-1))
                        {
                            /*清除该vlan中的untag端口*/
                            p->vlanconf.entryarry[gCount].untagPortMask &= ~(1<<port-1); 
                            
                            /*把该端口重新加入默认vlan中*/
                            p->vlanconf.entryarry[0].untagPortMask |= 1<<port-1;
                            p->vlanconf.entryarry[0].allPortMask |= 1<<port-1;
                        }
                    }
                }
                else
                {
                    /*Do nothing*/
                }
            }
        }

        VOS_MemZero(&p->portconf[port-1].extVlanConf, sizeof(ONUVlanExtConf_t));
        p->portconf[port-1].defaultVid = 1;            
    }
    return VOS_OK;
}
int OnuConfDelVlanPort(SHORT ponid, SHORT onuid, USHORT port)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
            ret = sfun_OnuConfDelVlanPort(p, port);
        }
        ONU_CONF_SEM_GIVE

    }
    else
        ret = VOS_ERROR;

    return ret;
}

int OnuConfDelVlanPortByPtr(int suffix, void *pdata, USHORT port)
{
    int ret = VOS_OK;

    if(ONU_PORT_VALID(port))
    {

        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
                ret = sfun_OnuConfDelVlanPort((ONUConfigData_t*)pdata, port);
        }
        ONU_CONF_SEM_GIVE

    }
    else
        ret = VOS_ERROR;

    return ret;
}

int onuconf_qosRuleFieldValue_StrToVal_parase( unsigned char select, unsigned char *pMatchStr, qos_value_t *pMatchVal )
{
    int rc_err = VOS_ERROR;
    char    *pToken;
    char tmpStr[20];

    if( (pMatchStr == NULL) || (pMatchVal == NULL) )
        return rc_err;

    VOS_MemZero( pMatchVal, sizeof(qos_value_t) );
    VOS_StrnCpy( tmpStr, pMatchStr, 17 );
    tmpStr[17] = 0;

    switch( select )
    {
        case CTC_FIELD_SEL_DA_MAC:
        case CTC_FIELD_SEL_SA_MAC:
            /*
            token = strtok( tmpStr,  " " );
            if( token == NULL )
                return rc_err;
            for( i=0; i<sizeof(mac_address_t); i++ )
            {
                if( token == NULL )
                    break;

                pMatchVal->mac_address[i] = VOS_StrToUL( token, &pToken, 16 );

                token = strtok( NULL, " " );
            }
            */
            Qos_Get_Mac_Address_By_Str( pMatchVal->mac_address, tmpStr );
            break;

        case CTC_FIELD_SEL_ETHERNET_PRIORITY:
        case CTC_FIELD_SEL_VLAN_ID:
            pMatchVal->match_value = VOS_AtoL( tmpStr );
            break;

        case CTC_FIELD_SEL_ETHER_TYPE:
            pMatchVal->match_value = VOS_StrToUL( tmpStr, &pToken, 16 );
            break;

        case CTC_FIELD_SEL_DEST_IP:
        case CTC_FIELD_SEL_SRC_IP:
            pMatchVal->match_value = get_long_from_ipdotstring( tmpStr );
            /*pMatchVal->match_value = 0;*/
            /*
            token = strtok( tmpStr,  "." );
            if( token == NULL )
                return rc_err;
            for( i=0; i<4; i++ )
            {
                if( token == NULL )
                    break;

                val = VOS_AtoL( token );
                pMatchVal->match_value |= ((val & 0xff) << ((3-i) * 8));

                token = strtok( NULL, "." );
            }
            */
            break;

        case CTC_FIELD_SEL_IP_PROTOCOL_TYPE:
            pMatchVal->match_value = VOS_StrToUL( tmpStr, &pToken, 16 );
            break;

        case CTC_FIELD_SEL_IPV4_TOS_DSCP:
        case CTC_FIELD_SEL_IPV6_TRAFFIC_CLASS:
        case CTC_FIELD_SEL_L4_SRC_PORT:
        case CTC_FIELD_SEL_L4_DEST_PORT:
            pMatchVal->match_value = VOS_AtoL( tmpStr );
            break;

        default:
            return rc_err;
    }
    return VOS_OK;
}
int onuconf_qosRuleFieldValue_ValToStr_parase( unsigned char select, qos_value_t *pMatchVal, unsigned char *pMatchStr )
{
    int rc_err = VOS_ERROR;

    if( (pMatchStr == NULL) || (pMatchVal == NULL) )
        return rc_err;

    switch( select )
    {
        case CTC_FIELD_SEL_DA_MAC:
        case CTC_FIELD_SEL_SA_MAC:
            VOS_Sprintf( pMatchStr, "%02x %02x %02x %02x %02x %02x",
                    pMatchVal->mac_address[0], pMatchVal->mac_address[1],
                    pMatchVal->mac_address[2], pMatchVal->mac_address[3],
                    pMatchVal->mac_address[4], pMatchVal->mac_address[5] );
            break;

        case CTC_FIELD_SEL_ETHERNET_PRIORITY:
        case CTC_FIELD_SEL_VLAN_ID:
            VOS_Sprintf( pMatchStr, "%d", pMatchVal->match_value );
            break;

        case CTC_FIELD_SEL_ETHER_TYPE:
            VOS_Sprintf( pMatchStr, "%04x", pMatchVal->match_value );
            break;

        case CTC_FIELD_SEL_DEST_IP:
        case CTC_FIELD_SEL_SRC_IP:
            VOS_Sprintf( pMatchStr, "%d.%d.%d.%d",
                    ((pMatchVal->match_value >> 24) & 0xff),
                    ((pMatchVal->match_value >> 16) & 0xff),
                    ((pMatchVal->match_value >> 8) & 0xff),
                    (pMatchVal->match_value & 0xff) );
            break;

        case CTC_FIELD_SEL_IP_PROTOCOL_TYPE:
            VOS_Sprintf( pMatchStr, "%02x", pMatchVal->match_value );
            break;

        case CTC_FIELD_SEL_IPV4_TOS_DSCP:
        case CTC_FIELD_SEL_IPV6_TRAFFIC_CLASS:
        case CTC_FIELD_SEL_L4_SRC_PORT:
        case CTC_FIELD_SEL_L4_DEST_PORT:
            VOS_Sprintf( pMatchStr, "%d", pMatchVal->match_value );
            break;

        default:
            return rc_err;
    }
    return VOS_OK;
}
static int sfun_setOnuConfQosPrioReplace(ONUConfigData_t *pd, USHORT port,int oldprio,int newprio)
{
    int ret = VOS_OK;
    if(oldprio<0 || oldprio>7 || newprio<0 || newprio>7)
        return VOS_ERROR;
    if (pd)
    {
        if(pd->portconf[port-1].qosPrioReplace[oldprio]!=newprio+1)
        {
            pd->portconf[port-1].qosPrioReplace[oldprio]=newprio+1;
        }
    }
    else
        ret = VOS_ERROR;
    return ret;
}
int setOnuConfQosPrioReplace(SHORT ponid, SHORT onuid, USHORT port,int oldprio, int newprio)
{
    int ret = QOS_ERR_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_setOnuConfQosPrioReplace(pd, port,oldprio, newprio);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int setOnuConfQosPrioReplaceByPtr(int suffix, void *pdata, USHORT port, int oldprio, int newprio)
{
    int ret = QOS_ERR_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = QOS_ERR_NO_DATA;
        else
            ret = sfun_setOnuConfQosPrioReplace((ONUConfigData_t*)pdata, port, oldprio,newprio);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

static int sfun_getOnuConfQosPrioReplace(ONUConfigData_t *pd,USHORT port,int oldprio,int *newprio)
{
    int ret = VOS_OK;
    if(oldprio>7 || oldprio<0)
        return VOS_ERROR;
    if (pd)
    {
        if(pd->portconf[port-1].qosPrioReplace[oldprio]!=0)
        {
            *newprio = pd->portconf[port-1].qosPrioReplace[oldprio]-1;
        }
        else 
            ret = VOS_ERROR;
    }
    else
        ret = VOS_ERROR;
    return ret;
}
int getOnuConfQosPrioReplace(SHORT ponid, SHORT onuid,USHORT port,int oldprio, int *newprio)
{
    int ret = QOS_ERR_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_getOnuConfQosPrioReplace(pd, port,oldprio, newprio);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int getOnuConfQosPrioReplaceByPtr(int suffix, void *pdata, USHORT port, int oldprio, int *newprio)
{
    int ret = QOS_ERR_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = QOS_ERR_NO_DATA;
        else
            ret = sfun_getOnuConfQosPrioReplace((ONUConfigData_t*)pdata, port, oldprio,newprio);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
static int sfun_setOnuConfQosPrioToQueue(ONUConfigData_t *pd,int prio,int queueNum)
{
    int ret = VOS_OK;
    if(prio>7 || prio<0 || queueNum>8 || queueNum<0)
        return VOS_ERROR;
    if (pd)
    {
        if(pd->qosMap.queue[prio]!=queueNum+1)
        {
            pd->qosMap.queue[prio]= queueNum+1;  
        }
    }
    else 
        ret = VOS_ERROR;
    return ret;
}
int setOnuConfQosPrioToQueue(SHORT ponid, SHORT onuid,int prio,int queueNum)
{
    int ret = QOS_ERR_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_setOnuConfQosPrioToQueue(pd, prio, queueNum);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int setOnuConfQosPrioToQueueByPtr(int suffix, void *pdata, int prio,int queueNum)
{
    int ret = QOS_ERR_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = QOS_ERR_NO_DATA;
        else
            ret = sfun_setOnuConfQosPrioToQueue((ONUConfigData_t*)pdata, prio, queueNum);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

static int sfun_getOnuConfQosPrioToQueue(ONUConfigData_t *pd,int prio,int *queueNum)
{
    int ret = VOS_OK;
    if(prio<0 || prio>7)
        return VOS_ERROR;
    if (pd)
    {
        if(pd->qosMap.queue[prio]!=0)
        {
            *queueNum = pd->qosMap.queue[prio]-1;  
        }
        else
            ret = VOS_ERROR;
    }
    else
        ret = VOS_ERROR;
    return ret;
}
int getOnuConfQosPrioToQueue(SHORT ponid, SHORT onuid,int prio,int *queueNum)
{
    int ret = QOS_ERR_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_getOnuConfQosPrioToQueue(pd, prio, queueNum);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int getOnuConfQosPrioToQueueByPtr(int suffix, void *pdata, int prio,int *queueNum)
{
    int ret = QOS_ERR_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = QOS_ERR_NO_DATA;
        else
            ret = sfun_getOnuConfQosPrioToQueue((ONUConfigData_t*)pdata, prio, queueNum);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

static int sfun_getOnuConfQosSet(ONUConfigData_t *pd, USHORT qossetid, qos_classification_rules_t * prules)
{
    int ret = QOS_ERR_OK;

    if (pd)
    {
        if (!qossetid || qossetid > QOS_MAX_SET_COUNT)
            ret = QOS_ERR_PARAM;
        else
        {
            qos_classification_rules_t * p = &pd->qosset[qossetid - 1];

            VOS_MemCpy(prules, p, sizeof(qos_classification_rules_t));
        }
    }
    else
        ret = QOS_ERR_NO_DATA;

    return ret;
}

int getOnuConfQosSet(SHORT ponid, SHORT onuid, USHORT qossetid, qos_classification_rules_t * prules)
{
    int ret = QOS_ERR_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_getOnuConfQosSet(pd, qossetid, prules);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int getOnuConfQosSetByPtr(int suffix, void *pdata, USHORT qossetid, qos_classification_rules_t * prules)
{
    int ret = QOS_ERR_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = QOS_ERR_NO_DATA;
        else
            ret = sfun_getOnuConfQosSet((ONUConfigData_t*)pdata, qossetid, prules);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

static int sfun_setOnuConfQosClass(ONUConfigData_t *pd, USHORT qossetid, USHORT ruleid, USHORT classid, int field, int oper, char *val, int len)
{
    int ret = VOS_OK;

    if(pd)
    {
        if(qossetid == 0 || classid == 0 || ruleid == 0)
            ret = QOS_ERR_PARAM; 
		/*modified by liyang @2015-04-22 for out of range (4,8,4)*/
        else if(qossetid <=  QOS_MAX_SET_COUNT && ruleid <= QOS_MAX_RULE_COUNT_PER_SET && classid <= QOS_MAX_CLASS_RULE_PAIRS ) 
        {
            qos_class_rule_entry *pentry;
            qos_classification_rule_t *prule = (qos_classification_rule_t *)&pd->qosset[qossetid-1];
            prule += ruleid-1;
            pentry = &prule->entries[classid-1];

            pentry->field_select = field;
            pentry->validation_operator = oper;
            VOS_MemCpy(&pentry->value, val, len);

            if(!(prule->entrymask&(1<<(classid-1))))
                prule->num_of_entries++;
            prule->entrymask |= 1<<(classid-1);

            prule->valid = TRUE;
        }
        else
            ret = QOS_ERR_PARAM;
    }
    else
        ret = QOS_ERR_NO_DATA;

    return ret;
}

int setOnuConfQosClass(SHORT ponid, SHORT onuid, USHORT qossetid, USHORT ruleid, USHORT classid, int field, int oper, char *val, int len)
{
    int ret = QOS_ERR_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_setOnuConfQosClass(pd, qossetid, ruleid, classid, field, oper, val, len);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int setOnuConfQosClassByPtr(int suffix, void *pd, USHORT qossetid, USHORT ruleid, USHORT classid, int field, int oper, char *val, int len)
{
    int ret = QOS_ERR_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd))
            ret = QOS_ERR_NO_DATA;
        else
            ret = sfun_setOnuConfQosClass((ONUConfigData_t*)pd, qossetid, ruleid, classid, field, oper, val, len);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

static int sfun_clrOnuConfQosClass( ONUConfigData_t *pd, USHORT qossetid, USHORT ruleid, USHORT classid)
{
    int ret = QOS_ERR_OK;

    if(pd)
    {
        if(qossetid == 0 || classid == 0 || ruleid == 0)
            ret = QOS_ERR_PARAM;
        else if(qossetid > QOS_MAX_SET_COUNT || ruleid > QOS_MAX_RULE_COUNT_PER_SET || classid > QOS_MAX_CLASS_RULE_PAIRS)
            ret = QOS_ERR_PARAM;
        else
        {
            qos_class_rule_entry *pentry;
            qos_classification_rule_t *prule = (qos_classification_rule_t *)(pd->qosset+(qossetid-1));
            prule += ruleid-1;
            pentry = &prule->entries[classid-1];

            if(prule->entrymask&(1<<(classid-1)))
                prule->num_of_entries--;
            prule->entrymask &= ~(1<<(classid-1));

            if(!prule->entrymask)
                prule->valid = FALSE;

            VOS_MemCpy(pentry, 0, sizeof(qos_class_rule_entry));
        }
    }
    else
        ret = QOS_ERR_NO_DATA;

    return ret;
}

int clrOnuConfQosClass(short int ponid, short int onuid, USHORT qossetid, USHORT ruleid, USHORT classid)
{
    int ret = QOS_ERR_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_clrOnuConfQosClass(pd, qossetid, ruleid, classid);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int clrOnuConfQosClassByPtr(int suffix, void *pd, USHORT qossetid, USHORT ruleid, USHORT classid)
{
    int ret = QOS_ERR_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd))
            ret = QOS_ERR_NO_DATA;
        else
            ret = sfun_clrOnuConfQosClass(pd, qossetid, ruleid, classid);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

static int sfun_setOnuConfQosRule(ONUConfigData_t *pd, USHORT qossetid, USHORT ruleid, UCHAR queue, UCHAR prio)
{
    int ret = QOS_ERR_OK;

    if(pd)
    {
        if(!(ruleid*qossetid))
            ret = QOS_ERR_PARAM;
        else if(ruleid > QOS_MAX_RULE_COUNT_PER_SET || qossetid > QOS_MAX_SET_COUNT)
            ret = QOS_ERR_PARAM;
        else
        {
            qos_classification_rule_t * prule = (qos_classification_rule_t *)(pd->qosset+(qossetid-1));
            prule += ruleid-1;

            prule->priority_mark = prio;
            prule->queue_mapped = queue;

            prule->valid = TRUE;

        }
    }
    else
        ret = QOS_ERR_NO_DATA;

    return ret;
}

int setOnuConfQosRule(SHORT ponid, SHORT onuid, USHORT qossetid, USHORT ruleid, UCHAR queue, UCHAR prio)
{
    int ret = QOS_ERR_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

        ret = sfun_setOnuConfQosRule(pd, qossetid, ruleid, queue, prio);

    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int setOnuConfQosRuleByPtr(int suffix, void *pdata, USHORT qossetid, USHORT ruleid, UCHAR queue, UCHAR prio)
{
    int ret = QOS_ERR_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = QOS_ERR_NO_DATA;
        else
            ret = sfun_setOnuConfQosRule(pdata, qossetid, ruleid, queue, prio);

    }
    ONU_CONF_SEM_GIVE

    return ret;
}

static int sfun_clrOnuConfQosRule(ONUConfigData_t * pd, USHORT qossetid, USHORT ruleid)
{
    int ret = QOS_ERR_OK;

    if(pd)
    {
        if(!(ruleid*qossetid) || ruleid > QOS_MAX_RULE_COUNT_PER_SET || qossetid > QOS_MAX_SET_COUNT)
            ret = QOS_ERR_PARAM;
        else
        {
            qos_classification_rule_t *prule = (qos_classification_rule_t *)(pd->qosset+(qossetid-1));
            prule += ruleid-1;
            VOS_MemSet(prule, 0, sizeof(qos_classification_rule_t));
        }
    }
    else
        ret = QOS_ERR_NO_DATA;

    return ret;
}

int clrOnuConfQosRule(SHORT ponid, SHORT onuid, USHORT qossetid, USHORT ruleid)
{
    int ret = QOS_ERR_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_clrOnuConfQosRule(pd, qossetid, ruleid);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int clrOnuConfQosRuleByPtr(int suffix, void *pdata, USHORT qossetid, USHORT ruleid)
{
    int ret = QOS_ERR_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pdata))
            ret = QOS_ERR_NO_DATA;
        else
            ret = sfun_clrOnuConfQosRule(pdata, qossetid, ruleid);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

static int sfun_setOnuConfPortMulticastVlan(ONUConfigData_t *pd, SHORT port, int vid)
{
    int ret = VOS_OK;

    if(pd)
    {
        ONUPortConf_t *opc = (ONUPortConf_t*)&pd->portconf[port-1];
        int i;

        if(vid == 0)
        {
            VOS_MemZero(opc->igmpVlan, sizeof(opc->igmpVlan));
            opc->igmpVlanNum = 0;
        }
        else
        {
            if(opc->igmpVlanNum >= ONU_MAX_IGMP_VLAN)
                ret = VOS_ERROR;
            else
            {
                int fb = -1, fexist = 0;
                for(i=0; i<ONU_MAX_IGMP_VLAN; i++)
                {
                    if(opc->igmpVlan[i] == vid)
                    {
                        fexist = 1;
                        break;
                    }

                    if(opc->igmpVlan[i] == 0 && fb==-1)
                        fb = i;
                }

                if(!fexist && fb != -1)
                {
                    opc->igmpVlan[fb] = vid;
                    opc->igmpVlanNum++;
                }
            }
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int setOnuConfPortMulticastVlan(SHORT ponid, SHORT onuid, SHORT port, int vid)
{
    int ret = VOS_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
            ret = sfun_setOnuConfPortMulticastVlan(pd, port, vid);
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int setOnuConfPortMulticastVlanByPtr(int suffix, void *pd, SHORT port, int vid)
{
    int ret = VOS_OK;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pd))
                ret = VOS_ERROR;
            else
                ret = sfun_setOnuConfPortMulticastVlan(pd, port, vid);
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

static int sfun_getOnuConfPortMulticastVlan(ONUConfigData_t *pd, SHORT port , int *num, int *vids)
{
    int ret = VOS_OK;

    if(pd)
    {
        int i, c = 0;
        ONUPortConf_t *opc = (ONUPortConf_t *)&pd->portconf[port-1];
        *num = opc->igmpVlanNum;
        for(i=0; i<ONU_MAX_IGMP_VLAN; i++)
        {
            if(opc->igmpVlan[i])
                vids[c++] = opc->igmpVlan[i];
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int getOnuConfPortMulticastVlan(SHORT ponid, SHORT onuid, SHORT port , int *num, int *vids)
{
    int ret = VOS_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
            ret = sfun_getOnuConfPortMulticastVlan(pd, port, num, vids);
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int getOnuConfPortMulticastVlanByPtr(int suffix, void* pd, SHORT port , int *num, int *vids)
{
    int ret = VOS_OK;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pd))
                ret = VOS_ERROR;
            else
                ret = sfun_getOnuConfPortMulticastVlan(pd, port, num, vids);
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

static int sfun_clrOnuConfPortMulticastVlan(ONUConfigData_t *pd, SHORT port, int vid)
{
    int ret = VOS_OK;

    if(pd)
    {
        ONUPortConf_t *opc = (ONUPortConf_t*)&pd->portconf[port-1];
        int i;

        if(opc->igmpVlanNum >= ONU_MAX_IGMP_VLAN)
            ret = VOS_ERROR;
        else
        {
            for(i=0; i<ONU_MAX_IGMP_VLAN; i++)
            {
                if(opc->igmpVlan[i] == vid)
                {
                    opc->igmpVlan[i] = 0;
                    opc->igmpVlanNum--;
                    break;
                }

            }
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int clrOnuConfPortMulticastVlan(SHORT ponid, SHORT onuid, SHORT port, int vid)
{
    int ret = VOS_OK;

    OLT_LOCAL_ASSERT(ponid);
    ONU_ASSERT(onuid);

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
            ret = sfun_clrOnuConfPortMulticastVlan(pd, port, vid);
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int clrOnuConfPortMulticastVlanByPtr(int suffix, void *pd, SHORT port, int vid)
{
    int ret = VOS_OK;

    if(ONU_PORT_VALID(port))
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
                ret = sfun_clrOnuConfPortMulticastVlan(pd, port, vid);
            else
                ret = VOS_ERROR;
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}
#endif

#if 1
static int sfun_getOnuConf_igmp_groupdata(ONUConfigData_t *pdata, int num,int vcode, ULONG *uv)
{
    int ret = VOS_OK;
    if(pdata)
    {
        switch(vcode)
        {
            case sv_enum_igmp_group_gda:
                *uv = pdata->igmpGroupArry[num].gda;
            break;
            case sv_enum_igmp_group_vlanId:
                *uv = pdata->igmpGroupArry[num].multicastVlanId;
                break;
            case sv_enum_igmp_group_portmask:
                *uv = pdata->igmpGroupArry[num].mapPortMask;
                break;
            default:
                ret = VOS_ERROR;
                break;
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}


static int sfun_SetOnuConf_igmp_groupdata(ONUConfigData_t *pdata, int num,int vcode, ULONG uv)
{
    int ret = VOS_OK;

    if(pdata)
    {
        switch(vcode)
        {
        case sv_enum_igmp_group_gda:
             pdata->igmpGroupArry[num].gda = uv;
        break;
        case sv_enum_igmp_group_vlanId:
             pdata->igmpGroupArry[num].multicastVlanId = uv;
            break;
        case sv_enum_igmp_group_portmask:
             pdata->igmpGroupArry[num].mapPortMask = uv;
            break;
        default:
            ret = VOS_ERROR;
            break;
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}
int getOnuConf_igmp_groupdata(SHORT ponid, SHORT onuid, int num,int vcode, ULONG*uv)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_getOnuConf_igmp_groupdata(p, num,vcode, uv);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int getOnuConf_igmp_groupdataByPtr(int suffix, void* pd, int num,int vcode, ULONG*uv)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_getOnuConf_igmp_groupdata(pd, num,vcode, uv);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int SetOnuConf_igmp_groupdata(SHORT ponid, SHORT onuid, int num,int vcode, ULONG uv)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_SetOnuConf_igmp_groupdata(p, num,vcode, uv);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int SetOnuConf_igmp_groupdataByPtr(int suffix, void* pd, int num,int vcode, ULONG uv)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_SetOnuConf_igmp_groupdata(pd, num,vcode, uv);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

#endif

#if 1
static int sfun_getOnuConf_Qos_Mode(ONUConfigData_t *pdata, int direction, unsigned char *prio_mode)
{
    int ret = VOS_OK;
    if(direction != QOS_RULE_UP_DIRECTION && direction != QOS_RULE_DOWN_DIRECTION)
        return VOS_ERROR;
    
    if(pdata)
    {
        *prio_mode = pdata->qos_rules[direction].priority_mode;        
    }
    else
    {
        ret = VOS_ERROR;
    }
    return ret;
}

static int sfun_SetOnuConf_Qos_Mode(ONUConfigData_t *pdata, int direction, unsigned char prio_mode)
{
    int ret = VOS_OK;
    if(direction != QOS_RULE_UP_DIRECTION && direction != QOS_RULE_DOWN_DIRECTION)
        return VOS_ERROR;
    
    if(pdata)
    {
        pdata->qos_rules[direction].priority_mode = prio_mode;
    }
    else
        ret = VOS_ERROR;

    return ret;
}
int getOnuConf_Qos_Mode(SHORT ponid, SHORT onuid, int direction,unsigned char *prio_mode)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_getOnuConf_Qos_Mode(p, direction, prio_mode);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int getOnuConf_Qos_ModeByPtr(int suffix, void* pd, int direction, unsigned char *prio_mode)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_getOnuConf_Qos_Mode(pd, direction, prio_mode);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int SetOnuConf_Qos_Mode(SHORT ponid, SHORT onuid, int direction, unsigned char prio_mode)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_SetOnuConf_Qos_Mode(p, direction, prio_mode);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int SetOnuConf_Qos_ModeByPtr(int suffix, void* pd, int direction, unsigned char prio_mode)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_SetOnuConf_Qos_Mode(pd, direction, prio_mode);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
static int sfun_getOnuConf_Qos_Rule(ONUConfigData_t *pdata, int direction, int index, gw_qos_classification_rule_t *qos_rule)
{
    int ret = VOS_OK;
    if(direction != QOS_RULE_UP_DIRECTION && direction != QOS_RULE_DOWN_DIRECTION)
        return VOS_ERROR;
    if(index < 0 || index >= MAX_CLASS_RULES_FOR_SINGLE_PATH)
        return VOS_ERROR;
    
    if(pdata && qos_rule)
    {
        if(pdata->qos_rules[direction].class_entry[index].valid == 1)
        {
            qos_rule->mode = pdata->qos_rules[direction].class_entry[index].mode;
            qos_rule->value = pdata->qos_rules[direction].class_entry[index].value;
            qos_rule->priority_mark = pdata->qos_rules[direction].class_entry[index].priority_mark;
            qos_rule->queue_mapped= pdata->qos_rules[direction].class_entry[index].queue_mapped;
        }
        else
        {
            ret = VOS_ERROR;
        }
    }
    else
    {
        ret = VOS_ERROR;
    }
    return ret;
}


static int sfun_SetOnuConf_Qos_Rule(ONUConfigData_t *pdata, int direction, gw_qos_classification_rule_t qos_rule)
{
    int ret = VOS_OK;
    unsigned char num_of_class = 0;
    int i = 0;
    if(direction != QOS_RULE_UP_DIRECTION && direction != QOS_RULE_DOWN_DIRECTION)
        return VOS_ERROR;
    
    if(pdata)
    {
        num_of_class = pdata->qos_rules[direction].num_of_class;
        for(i=0;i<num_of_class; i++)
        {
            if(pdata->qos_rules[direction].class_entry[i].valid
                &&pdata->qos_rules[direction].class_entry[i].mode == qos_rule.mode
                &&pdata->qos_rules[direction].class_entry[i].value == qos_rule.value)
                break;
        }
        if(i<num_of_class)
        {
            VOS_MemCpy(&(pdata->qos_rules[direction].class_entry[i]), &qos_rule, sizeof(gw_qos_classification_rule_t));
            pdata->qos_rules[direction].class_entry[i].valid = 1;                
        }
        else
        {
            if(num_of_class<MAX_CLASS_RULES_FOR_SINGLE_PATH)
            {
                VOS_MemCpy(&(pdata->qos_rules[direction].class_entry[num_of_class]), &qos_rule, sizeof(gw_qos_classification_rule_t));
                pdata->qos_rules[direction].class_entry[num_of_class].valid = 1;
                pdata->qos_rules[direction].num_of_class++;
            }
            else
            {
                ret = VOS_ERROR;
            }            
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}
static int sfun_clrOnuConf_Qos_Rule(ONUConfigData_t *pdata, int direction, gw_qos_classification_rule_t qos_rule)
{
    int ret = VOS_ERROR;
    unsigned char num_of_class = 0;
    int i = 0;
    if(direction != QOS_RULE_UP_DIRECTION && direction != QOS_RULE_DOWN_DIRECTION)
        return ret;
    
    if(pdata)
    {
        num_of_class = pdata->qos_rules[direction].num_of_class;
        for(i=0; i<num_of_class; i++)
        {
            if(pdata->qos_rules[direction].class_entry[i].valid)
            {
                if(pdata->qos_rules[direction].class_entry[i].mode == qos_rule.mode 
                    && pdata->qos_rules[direction].class_entry[i].value == qos_rule.value
                    && pdata->qos_rules[direction].class_entry[i].priority_mark == qos_rule.priority_mark
                    && pdata->qos_rules[direction].class_entry[i].queue_mapped == qos_rule.queue_mapped)
                {
                    int j =0;
                    if(i<num_of_class-1)
                    {
                        for(j=i;j<num_of_class-1;j++)
                            VOS_MemCpy(&(pdata->qos_rules[direction].class_entry[j]),  &(pdata->qos_rules[direction].class_entry[j+1]), sizeof(gw_qos_classification_rule_t));
                    }   
                    VOS_MemZero(&(pdata->qos_rules[direction].class_entry[num_of_class-1]), sizeof(gw_qos_classification_rule_t));                    
                    pdata->qos_rules[direction].num_of_class--;
                    ret = VOS_OK;
                    break;
                }
                    
            }
        }
    }

    return ret;
}

int getOnuConf_Qos_Rule(SHORT ponid, SHORT onuid, int direction, int index, gw_qos_classification_rule_t *qos_rule)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_getOnuConf_Qos_Rule(p, direction, index, qos_rule);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int getOnuConf_Qos_RuleByPtr(int suffix, void* pd, int direction, int index, gw_qos_classification_rule_t *qos_rule)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_getOnuConf_Qos_Rule(pd, direction, index, qos_rule);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int SetOnuConf_Qos_Rule(SHORT ponid, SHORT onuid, int direction, gw_qos_classification_rule_t qos_rule)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_SetOnuConf_Qos_Rule(p, direction, qos_rule);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int SetOnuConf_Qos_RuleByPtr(int suffix, void* pd, int direction, gw_qos_classification_rule_t qos_rule)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_SetOnuConf_Qos_Rule(pd, direction, qos_rule);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int ClrOnuConf_Qos_Rule(SHORT ponid, SHORT onuid, int direction, gw_qos_classification_rule_t qos_rule)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_clrOnuConf_Qos_Rule(p, direction, qos_rule);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int ClrOnuConf_Qos_RuleByPtr(int suffix, void* pd, int direction, gw_qos_classification_rule_t qos_rule)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_clrOnuConf_Qos_Rule(pd, direction, qos_rule);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

static int sfun_getOnuConf_Pas_Rule(ONUConfigData_t *pdata, int direction, int index, gw_pas_rule_t *pas_rule)
{
    int ret = VOS_OK;
    if(direction != QOS_RULE_UP_DIRECTION && direction != QOS_RULE_DOWN_DIRECTION)
        return VOS_ERROR;
    if(index < 0 || index >= MAX_PAS_RULES_FOR_SINGLE_PATH)
        return VOS_ERROR;
    
    if(pdata)
    {
        if(pdata->pas_rules[direction].rule_entry[index].valid == 1)
        {
            pas_rule->mode = pdata->pas_rules[direction].rule_entry[index].mode;
            pas_rule->action = pdata->pas_rules[direction].rule_entry[index].action;
            pas_rule->prio_source = pdata->pas_rules[direction].rule_entry[index].prio_source;
            pas_rule->value = pdata->pas_rules[direction].rule_entry[index].value;
            pas_rule->vlan_type = pdata->pas_rules[direction].rule_entry[index].vlan_type;
            pas_rule->new_vid = pdata->pas_rules[direction].rule_entry[index].new_vid;
        }
        else
        {
            ret = VOS_ERROR;
        }
    }
    else
    {
        ret = VOS_ERROR;
    }
    return ret;
}


static int sfun_SetOnuConf_Pas_Rule(ONUConfigData_t *pdata, int direction, gw_pas_rule_t pas_rule)
{
    int ret = VOS_OK;
    int num_of_rule = 0;
    int i = 0;
    if(direction != QOS_RULE_UP_DIRECTION && direction != QOS_RULE_DOWN_DIRECTION)
        return VOS_ERROR;
    
    if(pdata)
    {
        num_of_rule = pdata->pas_rules[direction].num_of_rule;
        for(i=0;i<num_of_rule;i++)
        {
            if(pdata->pas_rules[direction].rule_entry[i].valid
                &&pdata->pas_rules[direction].rule_entry[i].mode == pas_rule.mode
                &&pdata->pas_rules[direction].rule_entry[i].value == pas_rule.value)
                break;
        }
        if(i<num_of_rule)
        {
            VOS_MemCpy(&(pdata->pas_rules[direction].rule_entry[i]), &pas_rule, sizeof(gw_pas_rule_t));
            pdata->pas_rules[direction].rule_entry[i].valid = 1;
        }
        else
        {
            if(num_of_rule<MAX_PAS_RULES_FOR_SINGLE_PATH)
            {
                VOS_MemCpy(&(pdata->pas_rules[direction].rule_entry[num_of_rule]), &pas_rule, sizeof(gw_pas_rule_t));                
                pdata->pas_rules[direction].rule_entry[num_of_rule].valid = 1;                
                pdata->pas_rules[direction].num_of_rule++;
            }
            else
            {
                ret = VOS_ERROR;
            }
        }
    }
    else
        ret = VOS_ERROR;

    return ret;
}
static int sfun_clrOnuConf_Pas_Rule(ONUConfigData_t *pdata, int direction, gw_pas_rule_t pas_rule)
{
    int ret = VOS_ERROR;
    int num_of_rule = 0;
    int i = 0;
    if(direction != QOS_RULE_UP_DIRECTION && direction != QOS_RULE_DOWN_DIRECTION)
        return ret;
    
    if(pdata)
    {
        num_of_rule = pdata->pas_rules[direction].num_of_rule;
        for(i=0;i<num_of_rule;i++)
        {
            if(pdata->pas_rules[direction].rule_entry[i].valid)
            {
                if(pdata->pas_rules[direction].rule_entry[i].mode == pas_rule.mode
                    &&pdata->pas_rules[direction].rule_entry[i].action == pas_rule.action
                    &&pdata->pas_rules[direction].rule_entry[i].prio_source == pas_rule.prio_source
                    &&pdata->pas_rules[direction].rule_entry[i].value == pas_rule.value
                    &&pdata->pas_rules[direction].rule_entry[i].vlan_type == pas_rule.vlan_type
                    &&pdata->pas_rules[direction].rule_entry[i].new_vid == pas_rule.new_vid)
                {
                    int j=0;
                    if(i<num_of_rule-1)
                    {
                        for(j=i;j<num_of_rule-1;j++)
                            VOS_MemCmp(&(pdata->pas_rules[direction].rule_entry[j]), &(pdata->pas_rules[direction].rule_entry[j+1]), sizeof(gw_pas_rule_t));
                    }
                    VOS_MemZero(&(pdata->pas_rules[direction].rule_entry[num_of_rule-1]), sizeof(gw_pas_rule_t));
                    pdata->pas_rules[direction].num_of_rule--;                    
                    ret = VOS_OK;
                    break;
                }
            }
        }
    }

    return ret;
}

int getOnuConf_Pas_Rule(SHORT ponid, SHORT onuid, int direction, int index, gw_pas_rule_t *pas_rule)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_getOnuConf_Pas_Rule(p, direction, index, pas_rule);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int getOnuConf_Pas_RuleByPtr(int suffix, void* pd, int direction, int index, gw_pas_rule_t *pas_rule)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_getOnuConf_Pas_Rule(pd, direction, index, pas_rule);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int SetOnuConf_Pas_Rule(SHORT ponid, SHORT onuid, int direction, gw_pas_rule_t pas_rule)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_SetOnuConf_Pas_Rule(p, direction, pas_rule);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int SetOnuConf_Pas_RuleByPtr(int suffix, void* pd, int direction, gw_pas_rule_t pas_rule)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_SetOnuConf_Pas_Rule(pd, direction, pas_rule);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int ClrOnuConf_Pas_Rule(SHORT ponid, SHORT onuid, int direction, gw_pas_rule_t pas_rule)
{
    int ret = VOS_OK;

    OLT_ASSERT(ponid);
    ONU_ASSERT(onuid);

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
        ret = sfun_clrOnuConf_Pas_Rule(p, direction, pas_rule);
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int ClrOnuConf_Pas_RuleByPtr(int suffix, void* pd, int direction, gw_pas_rule_t pas_rule)
{
    int ret = VOS_OK;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd) == VOS_OK)
            ret = sfun_clrOnuConf_Pas_Rule(pd, direction, pas_rule);
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

#endif

#if 1
int onuConfIsShared(USHORT ponid, USHORT onuid)
{
    ONUConfigData_t *pd = getOnuConfFromHashBucket(
            ONU_CONF_NAME_PTR_GET(ponid, onuid));
    if (pd)
        return (pd->share ? TRUE : FALSE);
    else
        return FALSE;
}

int onuConfIsSharedByName(char *name)
{
    ONUConfigData_t *pd = getOnuConfFromHashBucket(name);
    if (pd)
        return (pd->share ? TRUE : FALSE);
    else
        return FALSE;
}

#define onu_resconf_section_begin

typedef struct onu_resconf_ctrl{
    USHORT pon_id;
    USHORT onu_id;
    UCHAR  trynum;
    UCHAR  mac[6];
}onu_resconf_ctrl_t;

/*
static ULONG s_taskOnuResConfId[CARD_MAX_PON_PORTNUM], s_semOnuResConf[CARD_MAX_PON_PORTNUM];
*/
static ULONG *s_taskOnuResConfId = NULL, *s_semOnuResConf = NULL;

static onu_conf_res_queue_list_t ** s_waitRestoreOnuQueue, **s_waitRestoreOnuQueueTail;

static ULONG *s_onuConfSyndTaskId, *s_onuConfSyndQueId, *s_onuConfSyncSemId;

static ULONG s_timer_sync_standby_master;

void changeSyncTimer(ULONG seconds)
{
    VOS_TimerChange(MODULE_RPU_CTC, s_timer_sync_standby_master, seconds*1000);
}

static int getQueIdBySlotno(int slotno)
{
    int i=0;
#if 0
    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER && !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
#else
    if(SYS_LOCAL_MODULE_TYPE_IS_6900_SW || SYS_LOCAL_MODULE_TYPE_IS_8000_SW)
#endif        
    {
        if(SYS_MODULE_WORKMODE_ISMASTER(slotno))
        {
        	if(!SYS_MODULE_ISMASTERACTIVE(slotno))
                return s_onuConfSyndQueId[MAX_PON_BOARD_NUM];
        	else
        		return s_onuConfSyndQueId[MAX_PON_BOARD_NUM+1];

        }
        else
        {
            for(i=0; i<MAX_PON_BOARD_NUM;i++)
                if(slotno == PON[i])
                    break;
            return s_onuConfSyndQueId[i];
        }

    }
    else
        return *s_onuConfSyndQueId;

}

static int getSemIdBySlotno(int slotno)
{
    int i=0;

    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER && !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
        if(SYS_MODULE_WORKMODE_ISMASTER(slotno))
        {
        	if(!SYS_MODULE_ISMASTERACTIVE(slotno))
            return s_onuConfSyncSemId[MAX_PON_BOARD_NUM];
        	else
        		return s_onuConfSyncSemId[MAX_PON_BOARD_NUM+1];
        }
        else
        {
            for(i=0; i<MAX_PON_BOARD_NUM;i++)
                if(slotno == PON[i])
                    break;
            return s_onuConfSyncSemId[i];
        }

    }
    else
        return *s_onuConfSyncSemId;

}

 
int portListLongToString(ULONG list, char *str)
{
    int bp=0, ep=0, i, len=0;

    for(i=0;i<sizeof(list)*8;i++)
    {
        if(list &(1<<i))
        {
            if(!(bp*ep))
            {
                bp = i+1;
                ep = i+1;
            }
            else
            {
                ep = i+1;
            }
        }
        else if(bp*ep)
        {
            if(bp == ep)
                len += VOS_Sprintf(str+len, "%d,", bp);
            else
            {
                len += VOS_Sprintf(str+len, "%d-%d,", bp , ep);
            }

            bp = 0;
            ep = 0;
        }
    }

    if(bp*ep)
    {
        if(bp == ep)
            len += VOS_Sprintf(str+len, "%d,", bp);
        else
        {
            len += VOS_Sprintf(str+len, "%d-%d,", bp , ep);
        }
    }

    if (0 < len)
        str[len-1] = 0;
    else
        str[0] = 0;

    return len;
}
int VlanListArrayToString(ULONG *list, int length, char *str)
{
    int bp=0, ep=0, i, len=0;
    int flag;
    for(i=0;i<length;i++)
    {
        flag = 0;
        if(!(bp*ep))
        {
            bp = list[i];
            ep = list[i];
        }
        if(list[i]+1 == list[i+1] && i<length-1)
        {
            ep = list[i+1];
        }
        else if(bp*ep && i<length-1)
        {
            if(bp == ep)
            {
                if(bp/1000)
                {
                    flag = 1;
                    len += VOS_Sprintf(str+len, "%d", bp/1000);
                    bp %= 1000;
                }
                if(bp/100 || flag)
                {
                   flag = 1;                    
                   len += VOS_Sprintf(str+len, "%d", bp/100);
                   bp %= 100;
                }
                if(bp/10 || flag)
                {
                    flag = 1;                    
                    len += VOS_Sprintf(str+len, "%d", bp/10);
                    bp %= 10;
                }
                len += VOS_Sprintf(str+len, "%d,", bp);
            }
            else
            {
                if(bp/1000)
                {
                    flag = 1;
                    len += VOS_Sprintf(str+len, "%d", bp/1000);
                    bp %= 1000;
                }
                if(bp/100 || flag)
                {
                   flag = 1;                    
                   len += VOS_Sprintf(str+len, "%d", bp/100);
                   bp %= 100;
                }
                if(bp/10 || flag)
                {
                    flag = 1;                    
                    len += VOS_Sprintf(str+len, "%d", bp/10);
                    bp %= 10;
                }
                len += VOS_Sprintf(str+len, "%d-", bp);
                flag = 0;
                if(ep/1000)
                {
                    flag = 1;
                    len += VOS_Sprintf(str+len, "%d", ep/1000);
                    ep %= 1000;
                }
                if(ep/100 || flag)
                {
                    flag = 1;                    
                    len += VOS_Sprintf(str+len, "%d", ep/100);
                    ep %= 100;
                }
                if(ep/10 || flag)
                {
                    flag = 1;                    
                    len += VOS_Sprintf(str+len, "%d", ep/10);
                    ep %= 10;
                }
                len += VOS_Sprintf(str+len, "%d,", ep);
            }
            bp = 0;
            ep = 0;
        }
    }
    if(bp*ep)
    {
        flag = 0;
            if(bp == ep)
            {
                if(bp/1000)
                {
                    flag = 1;
                    len += VOS_Sprintf(str+len, "%d", bp/1000);
                    bp %= 1000;
                }
                if(bp/100 || flag)
                {
                   flag = 1;                    
                   len += VOS_Sprintf(str+len, "%d", bp/100);
                   bp %= 100;
                }
                if(bp/10 || flag)
                {
                    flag = 1;                    
                    len += VOS_Sprintf(str+len, "%d", bp/10);
                    bp %= 10;
                }
                len += VOS_Sprintf(str+len, "%d", bp);
            }
            else
            {
                if(bp/1000)
                {
                    flag = 1;
                    len += VOS_Sprintf(str+len, "%d", bp/1000);
                    bp %= 1000;
                }
                if(bp/100 || flag)
                {
                   flag = 1;                    
                   len += VOS_Sprintf(str+len, "%d", bp/100);
                   bp %= 100;
                }
                if(bp/10 || flag)
                {
                    flag = 1;                    
                    len += VOS_Sprintf(str+len, "%d", bp/10);
                    bp %= 10;
                }
                len += VOS_Sprintf(str+len, "%d-", bp);
                flag = 0;
                if(ep/1000)
                {
                    flag = 1;
                    len += VOS_Sprintf(str+len, "%d", ep/1000);
                    ep %= 1000;
                }
                if(ep/100 || flag)
                {
                    flag = 1;                    
                    len += VOS_Sprintf(str+len, "%d", ep/100);
                    ep %= 100;
                }
                if(ep/10 || flag)
                {
                    flag = 1;                    
                    len += VOS_Sprintf(str+len, "%d", ep/10);
                    ep %= 10;
                }
                len += VOS_Sprintf(str+len, "%d", ep);
            }
    }
    return VOS_OK;
}

static int generateDefaultOnuConfClMemFile(struct vty *vty, char *pfile)
{
    vty_out(vty, "!GROS system config file\r\n"); /*解决无法初始化默认配置的问题 2011-06-29*/
    vty_out(vty, "config onu-profile %s\r\n", DEFAULT_ONU_CONF);
    /*vty_out(vty, "vlan dot1q 1\r\n");*/
#if 0
    vty_out(vty, "port en 1-24 1\r\n");
    vty_out(vty, "vlan pvid 1-24 1\r\n");
#endif

    vty_out(vty, "exit\r\n");
	if(!cl_flush_vty_obuf_to_memfile(vty->obuf, pfile))
		return 0;

	return 1;
}

int initDefaultOnuConfFile()
{
    int ret = 0;
    uchar *mfile = cl_config_mem_file_init();

    if(mfile)
    {
        struct vty * file_vty;
        file_vty = vty_new ();
        if ( !file_vty )
        {
#ifdef _ROUTER_
		      g_free(mfile);
#else /* platform */
#ifndef _DISTRIBUTE_PLATFORM_
		    VOS_Free(mfile);
#else
			VOS_ASSERT(0);
		    g_free(mfile);		/* aaaaaa modified by xieshl 20141121 */
#endif
#endif /* #ifdef _ROUTER_ */
            return 0;
        }
        file_vty->fd = _CL_MEMFILE_FD_;
        file_vty->type = VTY_FILE;

        if(generateDefaultOnuConfClMemFile( file_vty, mfile ))
        {

            extern ULONG g_ulFeiTaskId;/*FEI task id*/
        /*zhoucheng added: increase the task priority of FEI_rxPacket during execute startup config*/
            if(g_ulFeiTaskId)
            {
                VOS_TaskPrioritySet(g_ulFeiTaskId,TASK_PRIORITY_HIGHEST - 20);
            }

            if(cl_run_memfile_config_strict_quietly(mfile) == 1)
            {
                ret = 1;
#ifdef CDP_DEBUG
                ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL,("\r\n\r\n%s\r\n\r\n", mfile));
#endif
            }
            else
                ret = 0;

            if(g_ulFeiTaskId)
            {
                VOS_TaskPrioritySet(g_ulFeiTaskId,TASK_PRIORITY_HIGHER - 30);
            }

        }

        vty_free ( file_vty );

#ifdef _ROUTER_
      g_free(mfile);
#else /* platform */
#ifndef _DISTRIBUTE_PLATFORM_
    VOS_Free(mfile);
#else
    g_free(mfile);
#endif
#endif /* #ifdef _ROUTER_ */
    }

    return ret;
}

int generateOnuConfUndoClMemFile( char *name, unsigned char flags, struct vty *vty, char *pfile)
{
    int ret = 0;
    char portlist_str[80]="";
    ONUConfigData_t * pd = NULL;
    ONUConfigData_t * def = NULL;

	if(!name)
		return ret;

    if(!VOS_StriCmp(name, DEFAULT_ONU_CONF))
    {
    	if(pfile)
    	{
        if(!cl_flush_vty_obuf_to_memfile(vty->obuf, pfile))
            ret = 0;
        else
            ret = 1;
    	}
        else
            ret = 1;

        return ret;
    }
    pd = getOnuConfFromHashBucket(name);
    def = OnuConfigProfile_init();

    if(pd && def)
    {
        int i;

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if ( flags & OnuProfile_Part_CTC )
#endif
        {
            /*vlan*/

    		if(pd->vlanconf.portIsolateEnable!= def->vlanconf.portIsolateEnable)
    		    vty_out(vty, "vlan port_isolate %d\r\n",def->vlanconf.portIsolateEnable);


    		vty_out(vty, "!onu config\r\n");
            
    		if(pd->holdover != def->holdover)
    		    vty_out(vty, "ctc holdover time %d\r\n", def->holdover);

    		if(pd->fecenable!= def->fecenable)
    		    vty_out(vty, "fec-mode %s\r\n", def->fecenable?"enable":"disable");

    		vty_out(vty, "!L2 config\r\n");
    		/*atu config*/

    		if(pd->atuAging != def->atuAging)
    		    vty_out(vty, "atu aging %d\r\n", def->atuAging);

    		if(pd->atuLimit != def->atuLimit)
    		    vty_out(vty, "atu limit %d\r\n", def->atuLimit);

            {
    /*            int j = 0;
                int rmask[32];
                for(i=0;i<ONU_MAX_PORT;i++)
                {
                    if(pd->portconf[i].atuLearnEnable!=def->portconf[i].atuLearnEnable)
                    {
                        rmask[j]=i;
                        j++;
                    }
                }
                if(portListLongToString(rmask[j], portlist_str))
                    vty_out(vty, "atu learning %s %d\r\n", portlist_str, def->portconf[0].atuLearnEnable);*/
                    int rmask = 0;
    				vty_out_port_var_undo_list(pd->portconf, def->portconf, atuLearnEnable, rmask);
    				if(portListLongToString(rmask, portlist_str))
    					vty_out(vty, "atu learning %s %d\r\n", portlist_str, def->portconf[0].atuLearnEnable);
            }
     /*                             */       
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, vlanIngressFilter, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "vlan ingress_filtering %s %d\r\n", portlist_str, def->portconf[0].vlanIngressFilter);
            }

            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, vlanAcceptFrameType, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "vlan acceptable_frame_types %s %s\r\n", portlist_str, def->portconf[0].vlanAcceptFrameType?"all":"tagged");
            }
            
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, atuFloodEnable, rmask);
                if(portListLongToString(rmask, portlist_str))
                   vty_out(vty, "atu flood %s %d\r\n", portlist_str, def->portconf[0].atuFloodEnable);
            }
            
            vty_out(vty, "!\r\n\r\n");

            vty_out(vty, "!vlan config\r\n");

            vty_out(vty, "vlan dot1q 0\r\n");

            vty_out(vty, "vlan dot1q 1\r\n");

            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, defaultVid&0x0fff, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "vlan pvid %s %d\r\n", portlist_str, (def->portconf[0].defaultVid)&0x0fff);
            }

            vty_out(vty, "!\r\n\r\n");

            vty_out(vty, "!port config\r\n");

            /*port disable*/
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, enable, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "port en %s %d\r\n", portlist_str, def->portconf[0].enable);
            }

        /*vlan-transparent disable*/
        {
            int rmask = 0;
			vty_out_port_var_undo_list(pd->portconf, def->portconf, vlanTransparentEnable, rmask);
            if(portListLongToString(rmask, portlist_str))
                vty_out(vty, "vlan-transparent %s %d\r\n", portlist_str, def->portconf[0].vlanTransparentEnable);
        }
            /*port auto negotination disable*/
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, mode, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "port mode %s %d\r\n", portlist_str, def->portconf[0].mode);
            }

            /*port fec enable*/
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, fecEnable, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "port fc %s %d\r\n", portlist_str, def->portconf[0].fecEnable);
            }

            /*port pause enable*/

            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, pauseEnable, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "port pause %s %d\r\n", portlist_str, def->portconf[0].pauseEnable);
            }

            /*port ingress*/
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, ingressRateLimit, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "port ingress_rate %s %d %d\r\n", portlist_str, def->portconf[0].ingressRateType, def->portconf[0].ingressRateLimit);
            }

            /*port egress*/
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, egressRateLimit, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "port egress_rate %s %d\r\n", portlist_str, def->portconf[0].egressRateLimit);
            }

    		/*port mirror undo*/
    		if(pd->ingressMirrorFromList || pd->egressMirrorFromList)
    		{
    			int type = 'a';
    			if(!pd->ingressMirrorFromList)
    			{
    				portListLongToString(pd->egressMirrorFromList, portlist_str);
    				type = 'e';
    			}
    			else if(!pd->egressMirrorFromList)
    			{
    				portListLongToString(pd->ingressMirrorFromList, portlist_str);
    				type = 'i';
    			}
    			else
    				portListLongToString(pd->ingressMirrorFromList, portlist_str);

    			
    			vty_out(vty, "port mirror_from %c %s 0\r\n", type, portlist_str);
    		}

    		if(pd->egressMirrorToList)
    			vty_out(vty, "port mirror_to e %d\r\n", 0);

    		if(pd->ingressMirrorToList)
    			vty_out(vty, "port mirror_to i %d\r\n", 0);

            vty_out(vty, "!\r\n\r\n");

            /*qinq*/
            vty_out(vty, "!QinQ Config\r\n");
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, qinqEntry.qinqMode, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "qinq-config disable %s \r\n", portlist_str);
            }

            vty_out(vty, "!QoS config\r\n");

            /*qos default priority*/
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, defaultVid>>13, rmask);
                if(portListLongToString(rmask, portlist_str))
                   vty_out(vty, "qos def_pri %s %d\r\n", portlist_str, (def->portconf[0].defaultVid)>>13);
            }
    /*port Qos Rule*/
#if 1
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, qosRule, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "qos Rule %s %s\r\n", portlist_str, def->portconf[0].qosRule?"ip":"user");
            }
#endif
    /*Qos IP-DSCP priority*/
#if 1
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, qosIpEnable, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "qos ip_pri_en %s %d\r\n", portlist_str, def->portconf[0].qosIpEnable);
            }
#endif
    /*Qos 802.1p priority*/
#if 1
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, qosUserEnable, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "qos user_pri_en %s %d\r\n", portlist_str, def->portconf[0].qosUserEnable);
            }
#endif
    /*Qos Algorithm*/
#if 1
    		if(pd->qosAlgorithm!= def->qosAlgorithm)
    		    vty_out(vty, "qos algorithm %s\r\n", def->qosAlgorithm?"spq":"wrr");
#endif

        if(pd->qos_rules[QOS_RULE_UP_DIRECTION].priority_mode!=QOS_MODE_PRIO_TRANS)
            vty_out(vty, "qos vlan priority_mode up priority_translation\r\n");
        
        if(pd->qos_rules[QOS_RULE_DOWN_DIRECTION].priority_mode!=QOS_MODE_PRIO_TRANS)
            vty_out(vty, "qos vlan priority_mode down priority_translation\r\n");
        
        {
            int index = 0;
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                gw_qos_classification_rule_t *qos_rule = &(pd->qos_rules[QOS_RULE_UP_DIRECTION].class_entry[index]);
                if(qos_rule->valid)
                {
                    if(qos_rule->mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "qos classifier %s %x %s %d %d %d\r\n", "ether-type", qos_rule->value, "up", qos_rule->priority_mark, qos_rule->queue_mapped, 0);
                    else
                        vty_out(vty, "qos classifier %s %d %s %d %d %d\r\n", qos_rule->mode == BASE_ON_VLAN_ID?"vlan-id":"ip-protocol", qos_rule->value, "up", qos_rule->priority_mark, qos_rule->queue_mapped, 0);
                }
            }
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                gw_qos_classification_rule_t *qos_rule = &(pd->qos_rules[QOS_RULE_DOWN_DIRECTION].class_entry[index]);                
                if(qos_rule->valid)
                {
                    if(qos_rule->mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "qos classifier %s %x %s %d %d %d\r\n", "ether-type", qos_rule->value, "down", qos_rule->priority_mark, qos_rule->queue_mapped, 0);
                    else
                        vty_out(vty, "qos classifier %s %d %s %d %d %d\r\n", qos_rule->mode == BASE_ON_VLAN_ID?"vlan-id":"ip-protocol", qos_rule->value, "down", qos_rule->priority_mark, qos_rule->queue_mapped, 0);
                }
            }   
        }
        
        {
            int index = 0;
            for(index=0;index<MAX_PAS_RULES_FOR_SINGLE_PATH;index++)
            {
                gw_pas_rule_t *pas_rule = &(pd->pas_rules[PAS_RULE_UP_DIRECTION].rule_entry[index]);
                if(pas_rule->valid)
                {
                    if(pas_rule->mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "vlan pas-rule %s %0x %s %s %d %x %s %d\r\n", "ether-type", pas_rule->value, "up", 
                        pas_rule->action == PAS_RULE_ACTION_NONE?"none":pas_rule->action==PAS_RULE_ACTION_ATTACH?"attach":"exchange",
                        pas_rule->new_vid, pas_rule->vlan_type, pas_rule->prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 0);
                    else
                        vty_out(vty, "vlan pas-rule %s %d %s %s %d %x %s %d\r\n", pas_rule->mode == BASE_ON_VLAN_ID?"vlan-id":"ip-protocol", pas_rule->value, "up", 
                        pas_rule->action == PAS_RULE_ACTION_NONE?"none":pas_rule->action==PAS_RULE_ACTION_ATTACH?"attach":"exchange",
                        pas_rule->new_vid, pas_rule->vlan_type, pas_rule->prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 0);
                }
            }
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                gw_pas_rule_t *pas_rule = &(pd->pas_rules[PAS_RULE_DOWN_DIRECTION].rule_entry[index]);
                if(pas_rule->valid)
                {
                    if(pas_rule->mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "vlan pas-rule %s %0x %s %s %d %x %s %d\r\n", "ether-type", pas_rule->value, "down", 
                        pas_rule->action == PAS_RULE_ACTION_NONE?"none":pas_rule->action==PAS_RULE_ACTION_ATTACH?"attach":"exchange",
                        pas_rule->new_vid, pas_rule->vlan_type, pas_rule->prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 0);
                    else
                        vty_out(vty, "vlan pas-rule %s %d %s %s %d %x %s %d\r\n", pas_rule->mode == BASE_ON_VLAN_ID?"vlan-id":"ip-protocol", pas_rule->value, "down", 
                        pas_rule->action == PAS_RULE_ACTION_NONE?"none":pas_rule->action==PAS_RULE_ACTION_ATTACH?"attach":"exchange",
                        pas_rule->new_vid, pas_rule->vlan_type, pas_rule->prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 0);
                }
            }   
        }

    /*ingressRateLimitBase*/
#if 1
    		if(pd->ingressRateLimitBase!=def->ingressRateLimitBase)
    		    vty_out(vty, "port ingress_rate_limit_base %d\r\n", def->ingressRateLimitBase);
#endif
    /*Qos FE port replace 802.1p priority*/
#if 1
            {
                int j = 0;
                int rmask = 0;
                for(i=0;i<ONU_MAX_PORT;i++)
                {
                    for(j=0;j<8;j++)
                    {
                        if((pd->portconf[i].qosPrioReplace[j])!=def->portconf[i].qosPrioReplace[j])
                        {
                            rmask |=1<<i;
                            break;
                        }
                    }
                }
                if(portListLongToString(rmask, portlist_str))
                {
                	for(j=0;j<8;j++)
                    {
                        vty_out(vty, "qos user_pri_reg %s %d %d\r\n",portlist_str,j,def->portconf[0].qosPrioReplace[j]-1);      
                	}
                }
            }
#endif

    /*802.1p priority associated with traffic-class queue*/
#if 1
            for(i=0;i<8;i++)
            {
                if((pd->qosMap.queue[i])!=def->qosMap.queue[i])
                {
                   vty_out(vty, "qos user_pri_tc %d %d\r\n",i,def->qosMap.queue[i]-1);      
                }
            }
#endif

    /*IP-DSCP associated with traffi-class queue*/
#if 1
            for(i=0;i<8;i++)
            {
                int k = 0;
                if(pd->qosMap.qosDscpQueue[i][i] != 0xff)
                {
                    for(k=0;k<8;k++)
                    {
                        if(def->qosMap.qosDscpQueue[i][i]&(1<<k))
                        {
                            vty_out(vty, "qos dscp_tc %d %d\r\n",i*8+k,i);
                        }
                    }
                }
            }
#endif

            /*QOS set undo*/

            for(i=0; i<ONU_MAX_PORT; i++)
            {
            	if(pd->portconf[i].qosSetSel)
            		vty_out(vty, "service-policy port %d policy %d\r\n", i+1, 0);
            }

            vty_out(vty, "!\r\n\r\n");

            vty_out(vty, "!IGMP config\r\n");

            /*igmpsnooping set*/


                
                /*max response time*/
                if(pd->igmpMaxResponseTime != def->igmpMaxResponseTime)
                    vty_out(vty, "igmpsnooping max_response_time %d\r\n", def->igmpMaxResponseTime);

                /*host aging time for igmp*/
                if(pd->igmpHostAgeTime != def->igmpHostAgeTime)
                    vty_out(vty, "igmpsnooping host_aging_time %d\r\n", def->igmpHostAgeTime);

                /*group aging time*/
                if(pd->igmpGroupAgeTime != def->igmpGroupAgeTime)
                    vty_out(vty, "igmpsnooping group_aging_time %d\r\n", def->igmpGroupAgeTime);

                /*port fastleave enable*/
                {
                    int rmask = 0;
        			vty_out_port_var_undo_list(pd->portconf, def->portconf, igmpFastLeaveEnable, rmask);
                    if(portListLongToString(rmask, portlist_str))
                        vty_out(vty, "igmpsnooping fast_leave %s %d\r\n", portlist_str, def->portconf[0].igmpFastLeaveEnable);
                }

                /*igmp gda_del*/
                {
                    int i=0;
                    ULONG addr = 0;
                    for(i=0;i<ONU_MAX_IGMP_GROUP;i++)
                    {
                        if(VOS_OK == sfun_getOnuConf_igmp_groupdata(pd, i, sv_enum_igmp_group_gda, &addr))
                        {
                            if(addr)
                            {
                                vty_out(vty, "igmpsnooping gda_del %d.%d.%d.%d\r\n", addr>>24, (addr>>16)&0xff,
                                                (addr>>8)&0xff, addr&0xff);
                            }
                        }
                    }
                }

                /*port igmpsnooping max group*/
                {
                    int rmask = 0;
        			vty_out_port_var_undo_list(pd->portconf, def->portconf, igmpMaxGroup, rmask);
                    if(portListLongToString(rmask, portlist_str))
                       vty_out(vty, "igmpsnooping group-num %s %d\r\n", portlist_str, def->portconf[0].igmpMaxGroup);
                }
                
        	    {
                    /*multicast vlan*/
                    for(i=0;i<ONU_MAX_PORT;i++)
                    {
                        if(pd->portconf[i].igmpVlanNum)
                        {
                            vty_out(vty, "multicast vlan %d %d\r\n", i+1, 0); 
                        }
                    }
        	    }
                
            if(pd->igmpEnable != def->igmpEnable)
                vty_out(vty, "igmpsnooping enable %d\r\n",def->igmpEnable);

            /*max igmp gruop*/
            if(pd->igmpMaxGroup != def->igmpMaxGroup)
                vty_out(vty, "igmpsnooping max_group %d\r\n", def->igmpMaxGroup);

            /*global igmp fast leave enable*/
            if(pd->igmpFastLeaveEnable!=def->igmpFastLeaveEnable)
                vty_out(vty, "ctc igmp-fastleave %s\r\n", def->igmpFastLeaveEnable?"enable":"disable");

            /*igmp auth enable*/
            if(pd->igmpAuthEnable != def->igmpAuthEnable)
                vty_out(vty, "igmpsnooping auth enable %d\r\n",def->igmpAuthEnable);

    		/*ctc igmp tag strip set*/
            {
                int rmask = 0;
    			vty_out_port_var_undo_list(pd->portconf, def->portconf, igmpTagStrip, rmask);
                if(portListLongToString(rmask, portlist_str))
                    vty_out(vty, "igmpsnooping tag strip %s %d\r\n", portlist_str, def->portconf[0].igmpTagStrip);
            }		
  
            vty_out(vty, "!\r\n\r\n");

        }

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if ( flags & OnuProfile_Part_CMC )
        {
            ONUCmcConf_t *curr_cfg = &pd->cmcCfg;
            ONUCmcConf_t *default_cfg = &def->cmcCfg;

            default_cfg->cmcIsEnabled = curr_cfg->cmcIsEnabled;
            generateCmcConfCl2vty(curr_cfg, default_cfg, vty);    
        }
#endif            

        if(pfile)
        {
        if(!cl_flush_vty_obuf_to_memfile(vty->obuf, pfile))
            ret = 0;
        else
            ret = 1;
        }
        else
        	ret = 1;
    }
    onuconf_free(def,ONU_CONF_MEM_DATA_ID);
    return ret;
}

char * onuconf_write_showrun_to_buffer(struct vty *vty)
{
	char * pbuf = NULL;

	if(vty != NULL)
	{
		ULONG  length =  vty->obuf->length+1;
		char *p = VOS_Malloc(length, MODULE_RPU_CTC);

		if(p)
		{
			VOS_MemZero(p, length);
			if(cl_flush_vty_obuf_to_memfile(vty->obuf, p))
			{
				ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL,("write profile to buffer: length is %d\r\n", vty->obuf->length));
				pbuf = p;
			}
			else
			{
				ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL,("write profile to buffer:  fail\r\n"));
				VOS_Free(p);
			}
		}
	}

	return pbuf;
}

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
void generateCmcConfCl2vty( ONUCmcConf_t *cmcDftCfg, ONUCmcConf_t *cmcCurrCfg, struct vty *vty )
{
    if ( 0 < cmcCurrCfg->cmcIsEnabled )
    {
        int i, b, n, c;
        ULONG aulQueue[CMC_DSCHANNEL_NUM][2];
        SHORT asQueue[CMC_DSCHANNEL_NUM][2];
        UCHAR aucQueue[CMC_DSCHANNEL_NUM][2];
        ULONG *pulDft;
        ULONG *pulCurr;
        USHORT *pusDft;
        USHORT *pusCurr;
        SHORT *psDft;
        SHORT *psCurr;
        UCHAR *pucDft;
        UCHAR *pucCurr;
        UCHAR ucBitMap;
        UCHAR ucBitMask;
        UCHAR ucBitOff;
        UCHAR ucByteVal;

    
        vty_out(vty, "!cmc config\r\n");


        if ( cmcCurrCfg->maxCm != cmcDftCfg->maxCm )
        {
            if ( CMC_CFG_MAX_CM_DEFAULT != cmcCurrCfg->maxCm )
            {
    		    vty_out(vty, "cmc max-cm %d\r\n", cmcCurrCfg->maxCm);
            }
            else
            {
                if ( 0 < cmcDftCfg->cmcIsEnabled )
                {
        		    vty_out(vty, "undo cmc max-cm\r\n");
                }
            }
        }


        pucDft  = cmcDftCfg->aucUsChannelEnable;
        pucCurr = cmcCurrCfg->aucUsChannelEnable;
        if ( *pucCurr != *pucDft )
        {
            if ( 0xF == (ucBitMap = (*pucDft) ^ (*pucCurr)) )
            {
                if ( 0 == *pucCurr )
                {
        		    vty_out(vty, "shutdown cmc upstream channel all\r\n");
                }
                else
                {
                    if ( 0 < cmcDftCfg->cmcIsEnabled )
                    {
            		    vty_out(vty, "undo shutdown cmc upstream channel all\r\n");
                    }
                }
            }
            else
            {
                for ( i = 0; i < CMC_USCHANNEL_NUM; i++ )
                {
                    if ( ucBitMap & (ucBitMask = 1 << i) )
                    {
                        if ( ucBitMask & *pucCurr )
                        {
                            if ( 0 < cmcDftCfg->cmcIsEnabled )
                            {
                    		    vty_out(vty, "undo shutdown cmc upstream channel %d\r\n", i + 1);
                            }
                        }
                        else
                        {
                		    vty_out(vty, "shutdown cmc upstream channel %d\r\n", i + 1);
                        }
                    }
                }
            }
        }

        pusDft = (USHORT*)cmcDftCfg->aucDsChannelEnable;
        pusCurr = (USHORT*)cmcCurrCfg->aucDsChannelEnable;
        if ( *pusCurr != *pusDft )
        {
            if ( 0xFFFF == (USHORT)((*pusDft) ^ (*pusCurr)) )
            {
                if ( 0xFFFF == *pusCurr )
                {
        		    vty_out(vty, "undo shutdown cmc downstream channel all\r\n");
                }
                else
                {
                    if ( 0 < cmcDftCfg->cmcIsEnabled )
                    {
            		    vty_out(vty, "shutdown cmc downstream channel all\r\n");
                    }
                }
            }
            else
            {
                pucDft  = cmcDftCfg->aucDsChannelEnable;
                pucCurr = cmcCurrCfg->aucDsChannelEnable;
                for ( b = 0; b < 2; b++ )
                {
                    if ( 0 != (ucBitMap = (*pucDft) ^ (*pucCurr)) )
                    {
                        for ( i = 0; i < 8; i++ )
                        {
                            if ( ucBitMap & (ucBitMask = 1 << i) )
                            {
                                if ( ucBitMask & *pucCurr )
                                {
                        		    vty_out(vty, "undo shutdown cmc downstream channel %d\r\n", (b << 3) + i + 1);
                                }
                                else
                                {
                                    if ( 0 < cmcDftCfg->cmcIsEnabled )
                                    {
                            		    vty_out(vty, "shutdown cmc downstream channel %d\r\n", (b << 3) + i + 1);
                                    }
                                }
                            }
                        }
                    }

                    pucDft++;
                    pucCurr++;
                }
            }
        }


        pucDft  = cmcDftCfg->aucUsChannelD30;
        pucCurr = cmcCurrCfg->aucUsChannelD30;
        if ( *pucCurr != *pucDft )
        {
            if ( 0xF == (ucBitMap = (*pucDft) ^ (*pucCurr)) )
            {
                if ( 0 == *pucCurr )
                {
        		    vty_out(vty, "cmc upstream channel all docsis30 disable\r\n");
                }
                else
                {
                    if ( 0 < cmcDftCfg->cmcIsEnabled )
                    {
            		    vty_out(vty, "cmc upstream channel all docsis30 enable\r\n");
                    }
                }
            }
            else
            {
                for ( i = 0; i < CMC_USCHANNEL_NUM; i++ )
                {
                    if ( ucBitMap & (ucBitMask = 1 << i) )
                    {
                        if ( ucBitMask & *pucCurr )
                        {
                            if ( 0 < cmcDftCfg->cmcIsEnabled )
                            {
                    		    vty_out(vty, "cmc upstream channel %d docsis30 enable\r\n", i + 1);
                            }
                        }
                        else
                        {
                		    vty_out(vty, "cmc upstream channel %d docsis30 disable\r\n", i + 1);
                        }
                    }
                }
            }
        }


        pucDft  = cmcDftCfg->aucUsChannelType;
        pucCurr = cmcCurrCfg->aucUsChannelType;
        if ( *pucCurr != *pucDft )
        {
            if ( 0xF == (ucBitMap = (*pucDft) ^ (*pucCurr)) )
            {
                if ( 0xF == *pucCurr )
                {
        		    vty_out(vty, "cmc upstream channel all type atdma\r\n");
                }
                else
                {
                    if ( 0 < cmcDftCfg->cmcIsEnabled )
                    {
            		    vty_out(vty, "undo cmc upstream channel all type\r\n");
                    }
                }
            }
            else
            {
                for ( i = 0; i < CMC_USCHANNEL_NUM; i++ )
                {
                    if ( ucBitMap & (ucBitMask = 1 << i) )
                    {
                        if ( ucBitMask & *pucCurr )
                        {
                		    vty_out(vty, "cmc upstream channel %d type atdma\r\n", i + 1);
                        }
                        else
                        {
                            if ( 0 < cmcDftCfg->cmcIsEnabled )
                            {
                    		    vty_out(vty, "undo cmc upstream channel %d type\r\n", i + 1);
                            }
                        }
                    }
                }
            }
        }


        pusDft = (USHORT*)cmcDftCfg->aucDsChannelAnnex;
        pusCurr = (USHORT*)cmcCurrCfg->aucDsChannelAnnex;
        if ( *pusCurr != *pusDft )
        {
            if ( 0xFFFF == (USHORT)((*pusDft) ^ (*pusCurr)) )
            {
                if ( 0xFFFF == *pusCurr )
                {
        		    vty_out(vty, "cmc downstream channel all annex b\r\n");
                }
                else
                {
                    if ( 0 < cmcDftCfg->cmcIsEnabled )
                    {
            		    vty_out(vty, "undo cmc downstream channel all annex\r\n");
                    }
                }
            }
            else
            {
                pucDft  = cmcDftCfg->aucDsChannelAnnex;
                pucCurr = cmcCurrCfg->aucDsChannelAnnex;
                for ( b = 0; b < 2; b++ )
                {
                    if ( 0 != (ucBitMap = (*pucDft) ^ (*pucCurr)) )
                    {
                        for ( i = 0; i < 8; i++ )
                        {
                            if ( ucBitMap & (ucBitMask = 1 << i) )
                            {
                                if ( ucBitMask & *pucCurr )
                                {
                        		    vty_out(vty, "cmc downstream channel %d annex b\r\n", (b << 3) + i + 1);
                                }
                                else
                                {
                                    if ( 0 < cmcDftCfg->cmcIsEnabled )
                                    {
                            		    vty_out(vty, "undo cmc downstream channel %d annex\r\n", (b << 3) + i + 1);
                                    }
                                }
                            }
                        }
                    }

                    pucDft++;
                    pucCurr++;
                }
            }
        }


        if ( *(ULONG*)cmcCurrCfg->aucDsChannelModulation != *(ULONG*)cmcDftCfg->aucDsChannelModulation )
        {
            char modulation_val[16];
            
            pucDft  = cmcDftCfg->aucDsChannelModulation;
            pucCurr = cmcCurrCfg->aucDsChannelModulation;
            for ( b = 0, n = 0, c = 0; b < 4; b++ )
            {
                if ( *pucDft != *pucCurr )
                {
                    for ( i = 0; i < 4; i++ )
                    {
                        ucBitOff  = i << 1;
                        ucBitMask = 3 << ucBitOff;
                        
                        if ( (*pucDft & ucBitMask) != (ucByteVal = (*pucCurr & ucBitMask)) )
                        {
                            ucByteVal >>= ucBitOff;
                            aucQueue[n][1] = ucByteVal;

                            if ( (CMC_CFG_DOWN_CHANNEL_MODULATION_DEFAULT != ucByteVal) || (cmcDftCfg->cmcIsEnabled) )
                            {
                                aulQueue[n][0] = (b << 2) + i + 1;
                            }
                            else
                            {
                                aulQueue[n][0] = 0;
                            }

                            if ( (0 == c) && (n > 0) )
                            {
                                if ( aulQueue[n][1] != aulQueue[n-1][1] )
                                {
                                    c = 1;
                                }
                            }

                            n++;
                        }
                    }
                }

                pucDft++;
                pucCurr++;
            }

            if ( 0 == c )
            {
                if ( (0 < n) && (0 < aucQueue[0][0]) )
                {
                    if ( CMC_CFG_DOWN_CHANNEL_MODULATION_DEFAULT == aucQueue[0][1] )
                    {
            		    vty_out(vty, "undo cmc downstream channel all modulation\r\n");
                    }
                    else
                    {
                        if ( 0 < CMCUtil_GetDownstreamModulationNameByCode(aucQueue[0][1], modulation_val) )
                        {
                		    vty_out(vty, "cmc downstream channel all modulation %s\r\n", modulation_val);
                        }
                    }
                }
            }
            else
            {
                for ( i = 0; i < n; i++ )
                {
                    if ( 0 < aucQueue[i][0] )
                    {
                        if ( CMC_CFG_DOWN_CHANNEL_MODULATION_DEFAULT == aucQueue[i][1] )
                        {
                		    vty_out(vty, "undo cmc downstream channel %d modulation\r\n", aucQueue[i][0]);
                        }
                        else
                        {
                            if ( 0 < CMCUtil_GetDownstreamModulationNameByCode(aucQueue[i][1], modulation_val) )
                            {
                    		    vty_out(vty, "cmc downstream channel %d modulation %s\r\n", aucQueue[i][0], modulation_val);
                            }
                        }
                    }
                }
            }
        }


        if ( *(long long*)cmcCurrCfg->aucDsChannelInterleaver != *(long long*)cmcDftCfg->aucDsChannelInterleaver )
        {
            char interleaver_val[16];
            
            pucDft  = cmcDftCfg->aucDsChannelInterleaver;
            pucCurr = cmcCurrCfg->aucDsChannelInterleaver;
            for ( b = 0, n = 0, c = 0; b < 8; b++ )
            {
                if ( *pucDft != *pucCurr )
                {
                    for ( i = 0; i < 2; i++ )
                    {
                        ucBitOff  = i << 4;
                        ucBitMask = 0xF << ucBitOff;
                        
                        if ( (*pucDft & ucBitMask) != (ucByteVal = (*pucCurr & ucBitMask)) )
                        {
                            ucByteVal >>= ucBitOff;
                            aucQueue[n][1] = ucByteVal;

                            if ( (CMC_CFG_DOWN_CHANNEL_INTERLEAVER_DEFAULT != ucByteVal) || (cmcDftCfg->cmcIsEnabled) )
                            {
                                aulQueue[n][0] = (b << 1) + i + 1;
                            }
                            else
                            {
                                aulQueue[n][0] = 0;
                            }

                            if ( (0 == c) && (n > 0) )
                            {
                                if ( aulQueue[n][1] != aulQueue[n-1][1] )
                                {
                                    c = 1;
                                }
                            }

                            n++;
                        }
                    }
                }

                pucDft++;
                pucCurr++;
            }

            if ( 0 == c )
            {
                if ( (0 < n) && (0 < aucQueue[0][0]) )
                {
                    if ( CMC_CFG_DOWN_CHANNEL_INTERLEAVER_DEFAULT == aucQueue[0][1] )
                    {
            		    vty_out(vty, "undo cmc downstream channel all interleaver\r\n");
                    }
                    else
                    {
                        if ( 0 < CMCUtil_GetDownstreamInterleaverNameByCode(aucQueue[0][1], interleaver_val) )
                        {
                		    vty_out(vty, "cmc downstream channel all interleaver %s\r\n", interleaver_val);
                        }
                    }
                }
            }
            else
            {
                for ( i = 0; i < n; i++ )
                {
                    if ( 0 < aucQueue[i][0] )
                    {
                        if ( CMC_CFG_DOWN_CHANNEL_INTERLEAVER_DEFAULT == aucQueue[i][1] )
                        {
                		    vty_out(vty, "undo cmc downstream channel %d interleaver\r\n", aucQueue[i][0]);
                        }
                        else
                        {
                            if ( 0 < CMCUtil_GetDownstreamInterleaverNameByCode(aucQueue[i][1], interleaver_val) )
                            {
                    		    vty_out(vty, "cmc downstream channel %d interleaver %s\r\n", aucQueue[i][0], interleaver_val);
                            }
                        }
                    }
                }
            }
        }


        if ( *(USHORT*)cmcCurrCfg->aucUsChannelProfile != *(USHORT*)cmcDftCfg->aucUsChannelProfile )
        {
            char profile_str[64];
            
            pucDft  = cmcDftCfg->aucUsChannelProfile;
            pucCurr = cmcCurrCfg->aucUsChannelProfile;
            for ( b = 0; b < 2; b++ )
            {
                if ( *pucDft != *pucCurr )
                {
                    for ( i = 0; i < 2; i++ )
                    {
                        ucBitOff  = i << 4;
                        ucBitMask = 0xF << ucBitOff;
                        
                        if ( (*pucDft & ucBitMask) != (ucByteVal = (*pucCurr & ucBitMask)) )
                        {
                            ucByteVal >>= ucBitOff;
                            aucQueue[n][1] = ucByteVal;

                            if ( (CMC_CFG_UP_CHANNEL_PROFILE_DEFAULT != ucByteVal) || (cmcDftCfg->cmcIsEnabled) )
                            {
                                aulQueue[n][0] = (b << 1) + i + 1;
                            }
                            else
                            {
                                aulQueue[n][0] = 0;
                            }

                            if ( (0 == c) && (n > 0) )
                            {
                                if ( aulQueue[n][1] != aulQueue[n-1][1] )
                                {
                                    c = 1;
                                }
                            }

                            n++;
                        }
                    }
                }

                pucDft++;
                pucCurr++;
            }

            if ( 0 == c )
            {
                if ( (0 < n) && (0 < aucQueue[0][0]) )
                {
                    if ( CMC_CFG_UP_CHANNEL_PROFILE_DEFAULT == aucQueue[0][1] )
                    {
            		    vty_out(vty, "undo cmc upstream channel all profile\r\n");
                    }
                    else
                    {
                        if ( 0 < CMCUtil_GetUpstreamProfileStrByCode(aucQueue[0][1], profile_str) )
                        {
                		    vty_out(vty, "cmc upstream channel all %s\r\n", profile_str);
                        }
                    }
                }
            }
            else
            {
                for ( i = 0; i < n; i++ )
                {
                    if ( 0 < aucQueue[i][0] )
                    {
                        if ( CMC_CFG_UP_CHANNEL_PROFILE_DEFAULT == aucQueue[i][1] )
                        {
                		    vty_out(vty, "undo cmc upstream channel %d profile\r\n", aucQueue[i][0]);
                        }
                        else
                        {
                            if ( 0 < CMCUtil_GetUpstreamProfileStrByCode(aucQueue[i][1], profile_str) )
                            {
                    		    vty_out(vty, "cmc upstream channel %d %s\r\n", aucQueue[i][0], profile_str);
                            }
                        }
                    }
                }
            }
        }


        pulDft  = cmcDftCfg->aulDsChannelFreq;
        pulCurr = cmcCurrCfg->aulDsChannelFreq;
        for ( i = 0; i < CMC_DSCHANNEL_NUM; i++ )
        {
            if ( pulDft[i] != pulCurr[i] )
            {
                if ( CMC_CFG_DOWN_CHANNEL_FREQ_DEFAULT(i) == pulCurr[i] )
                {
                    if ( 0 < cmcDftCfg->cmcIsEnabled )
                    {
            		    vty_out(vty, "undo cmc downstream channel %d freq\r\n", i + 1);
                    }
                }
                else
                {
        		    vty_out(vty, "cmc upstream downstream %d freq %lu\r\n", i + 1, pulCurr[i]);
                }
            }
        }
        

        pulDft  = cmcDftCfg->aulUsChannelFreq;
        pulCurr = cmcCurrCfg->aulUsChannelFreq;
        for ( i = 0; i < CMC_USCHANNEL_NUM; i++ )
        {
            if ( pulDft[i] != pulCurr[i] )
            {
                if ( CMC_CFG_UP_CHANNEL_FREQ_DEFAULT(i) == pulCurr[i] )
                {
                    if ( 0 < cmcDftCfg->cmcIsEnabled )
                    {
            		    vty_out(vty, "undo cmc upstream channel %d freq\r\n", i + 1);
                    }
                }
                else
                {
        		    vty_out(vty, "cmc upstream channel %d freq %lu\r\n", i + 1, pulCurr[i]);
                }
            }
        }


        pulDft  = cmcDftCfg->aulUsChannelFreqWidth;
        pulCurr = cmcCurrCfg->aulUsChannelFreqWidth;
        for ( i = 0, n = 0, c = 0; i < CMC_USCHANNEL_NUM; i++ )
        {
            if ( pulCurr[i] != pulDft[i] )
            {
                aulQueue[n][1] = pulCurr[i];

                if ( (CMC_CFG_UP_CHANNEL_FREQ_WIDTH_DEFAULT != pulCurr[i]) || (cmcDftCfg->cmcIsEnabled) )
                {
                    aulQueue[n][0] = i + 1;
                }
                else
                {
                    aulQueue[n][0] = 0;
                }

                if ( (0 == c) && (n > 0) )
                {
                    if ( aulQueue[n][1] != aulQueue[n-1][1] )
                    {
                        c = 1;
                    }
                }

                n++;
            }
        }

        if ( 0 == c )
        {
            if ( (0 < n) && (0 < aulQueue[0][0]) )
            {
                if ( CMC_CFG_UP_CHANNEL_FREQ_WIDTH_DEFAULT == aulQueue[0][1] )
                {
        		    vty_out(vty, "undo cmc upstream channel all width\r\n");
                }
                else
                {
        		    vty_out(vty, "cmc upstream channel all width %lu\r\n", aulQueue[0][1]);
                }
            }
        }
        else
        {
            for ( i = 0; i < n; i++ )
            {
                if ( 0 < aulQueue[i][0] )
                {
                    if ( CMC_CFG_UP_CHANNEL_FREQ_WIDTH_DEFAULT == aulQueue[i][1] )
                    {
            		    vty_out(vty, "undo cmc upstream channel %d width\r\n", aulQueue[i][0]);
                    }
                    else
                    {
            		    vty_out(vty, "cmc upstream channel %d width %lu\r\n", aulQueue[i][0], aulQueue[i][1]);
                    }
                }
            }
        }


        psDft  = cmcDftCfg->asUsChannelPower;
        psCurr = cmcCurrCfg->asUsChannelPower;
        for ( i = 0, n = 0, c = 0; i < CMC_USCHANNEL_NUM; i++ )
        {
            if ( psDft[i] != psCurr[i] )
            {
                asQueue[n][1] = psCurr[i];
                
                if ( (CMC_CFG_UP_CHANNEL_POWER_LEVEL_DEFAULT != pulCurr[i]) || (cmcDftCfg->cmcIsEnabled) )
                {
                    asQueue[n][0] = i + 1;
                }
                else
                {
                    asQueue[n][0] = 0;
                }

                if ( (0 == c) && (n > 0) )
                {
                    if ( asQueue[n][1] != asQueue[n-1][1] )
                    {
                        c = 1;
                    }
                }

                n++;
            }
        }

        if ( 0 == c )
        {
            if ( (0 < n) && (0 < asQueue[0][0]) )
            {
                if ( CMC_CFG_UP_CHANNEL_POWER_LEVEL_DEFAULT == asQueue[0][1] )
                {
        		    vty_out(vty, "undo cmc upstream channel all power-level\r\n");
                }
                else
                {
        		    vty_out(vty, "cmc upstream channel all power-level %d\r\n", asQueue[0][1]);
                }
            }
        }
        else
        {
            for ( i = 0; i < n; i++ )
            {
                if ( 0 < asQueue[i][0] )
                {
                    if ( CMC_CFG_UP_CHANNEL_POWER_LEVEL_DEFAULT == asQueue[i][1] )
                    {
            		    vty_out(vty, "undo cmc upstream channel %d power-level\r\n", asQueue[i][0]);
                    }
                    else
                    {
            		    vty_out(vty, "cmc upstream channel %d power-level %d\r\n", asQueue[i][0], asQueue[i][1]);
                    }
                }
            }
        }


        psDft  = cmcDftCfg->asDsChannelPower;
        psCurr = cmcCurrCfg->asDsChannelPower;
        for ( i = 0, n = 0, c = 0; i < CMC_DSCHANNEL_NUM; i++ )
        {
            if ( psDft[i] != psCurr[i] )
            {
                asQueue[n][1] = psCurr[i];
                
                if ( (CMC_CFG_DOWN_CHANNEL_POWER_LEVEL_DEFAULT != pulCurr[i]) || (cmcDftCfg->cmcIsEnabled) )
                {
                    asQueue[n][0] = i + 1;
                }
                else
                {
                    asQueue[n][0] = 0;
                }

                if ( (0 == c) && (n > 0) )
                {
                    if ( asQueue[n][1] != asQueue[n-1][1] )
                    {
                        c = 1;
                    }
                }

                n++;
            }
        }

        if ( 0 == c )
        {
            if ( (0 < n) && (0 < asQueue[0][0]) )
            {
                if ( CMC_CFG_DOWN_CHANNEL_POWER_LEVEL_DEFAULT == asQueue[0][1] )
                {
        		    vty_out(vty, "undo cmc downstream channel all power-level\r\n");
                }
                else
                {
        		    vty_out(vty, "cmc downstream channel all power-level %d\r\n", asQueue[0][1]);
                }
            }
        }
        else
        {
            for ( i = 0; i < n; i++ )
            {
                if ( 0 < asQueue[i][0] )
                {
                    if ( CMC_CFG_DOWN_CHANNEL_POWER_LEVEL_DEFAULT == asQueue[0][1] )
                    {
            		    vty_out(vty, "undo cmc downstream channel %d power-level\r\n", asQueue[i][0]);
                    }
                    else
                    {
            		    vty_out(vty, "cmc downstream channel %d power-level %d\r\n", asQueue[i][0], asQueue[i][1]);
                    }
                }
            }
        }

        vty_out(vty, "!\r\n\r\n");
    }
    else
    {
        /* 此ONU不是CMC，则无CMC配置 */
    }

    return;
}
#endif

int generateOnuConfClMemFile( char *name, unsigned char flags, struct vty *vty, char *pfile, short int slot, short int port, short int onuid)
{
    int ret = 0;

    char portlist_str[80]="";
    ONUConfigData_t * pd = NULL;
    ONUConfigData_t * def = NULL;

	if(!name )
	{
		return ret;
	}
    /*默认配置返回空文件*/
    if(!VOS_StriCmp(name, DEFAULT_ONU_CONF))
	{
		if(pfile)
		{
			if(!cl_flush_vty_obuf_to_memfile(vty->obuf, pfile))
				ret = 0;
			else
				ret = 1;
		}
		else
			ret = 1;

		return ret;
	}

    def = OnuConfigProfile_init();
    pd = getOnuConfFromHashBucket(name);

    if(pd && def)
    {
        int i;

        ONUVlanConf_t *pvc = &pd->vlanconf;
        ONUVlanConf_t *pvc_def = &def->vlanconf;

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if ( flags & OnuProfile_Part_CTC )
#endif
        {
            /*vlan*/

    		if(pd->vlanconf.portIsolateEnable!=def->vlanconf.portIsolateEnable)
    		    vty_out(vty, "vlan port_isolate %d\r\n", pd->vlanconf.portIsolateEnable);


    		vty_out(vty, "!onu config\r\n");

    		if(pd->holdover!=def->holdover)
    		vty_out(vty, "ctc holdover time %d\r\n", pd->holdover);

    		if(pd->fecenable!=def->fecenable)
    		    vty_out(vty, "fec-mode %s\r\n", pd->fecenable?"enable":"disable");

    		vty_out(vty, "!L2 config\r\n");
    		/*atu config*/

    		if(pd->atuAging!=def->atuAging)
    		    vty_out(vty, "atu aging %d\r\n", pd->atuAging);

    		if(pd->atuLimit!=def->atuLimit)
    		    vty_out(vty, "atu limit %d\r\n", pd->atuLimit);

            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, atuLearnEnable, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].atuLearnEnable)
                                vty_out(vty, "atu learning %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, vlanIngressFilter, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].vlanIngressFilter)
                                vty_out(vty, "vlan ingress_filtering %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }

            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, vlanAcceptFrameType, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].vlanAcceptFrameType)
                               vty_out(vty, "vlan acceptable_frame_types %s %s\r\n", portlist_str, rv[i]?"all":"tagged");
                        }
                    }
                }
            }
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, atuFloodEnable, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].atuFloodEnable)
                                vty_out(vty, "atu flood %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }
            vty_out(vty, "!\r\n\r\n");

            vty_out(vty, "!vlan config\r\n");

            if((pvc->vmtrunkportmask &&
            		!pvc->vmtranslationportmask &&
            		!pvc->vmtransparentportmask &&
            		!pvc->vmtagportmask &&
            		!pvc->vmaggportmask)!=(pvc_def->vmtrunkportmask &&
            		!pvc_def->vmtranslationportmask &&
            		!pvc_def->vmtransparentportmask &&
            		!pvc_def->vmtagportmask &&
            		!pvc_def->vmaggportmask))/*去使能1Q VLAN*/
        	{
                int mode = sfun_getOnuConfVlanMode(pd);
                if(mode == ONU_CONF_VLAN_MODE_TAG)
                {
                    vty_out(vty, "vlan mode tag\r\n");
  		            if(!(slot||port||onuid))
	                {  
	                    ULONG begin_slot,begin_port,begin_onuid;
	                    ULONG end_slot,end_port,end_onuid;  
	                    
	                    for(i=0;i<pd->rules.number_rules;i++)
	                    {	
	                    	/*tag 模式下只需要关注untag vlan*/
	                        if(pd->rules.rule[i].rule_mode && pd->rules.rule[i].mode == 2)
	                        {
	                            begin_slot = GET_PONSLOT(pd->rules.rule[i].begin_onuid);
	                            begin_port = GET_PONPORT(pd->rules.rule[i].begin_onuid);
	                            begin_onuid = GET_ONUID(pd->rules.rule[i].begin_onuid);
	                            end_slot = GET_PONSLOT(pd->rules.rule[i].end_onuid);
	                            end_port = GET_PONPORT(pd->rules.rule[i].end_onuid);
	                            end_onuid = GET_ONUID(pd->rules.rule[i].end_onuid);
	                            
	                            if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID)
	                                vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d %d %d\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].mode);
	                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID_PORT)
	                            {
	                                portListLongToString(pd->rules.rule[i].portlist, portlist_str);
	                                vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d %d %d %s\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].mode, portlist_str);
	                            }
	                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE)
	                                vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d vlan-range %d %d %d %d\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].end_vid, pd->rules.rule[i].step, pd->rules.rule[i].mode);
	                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_PORT)
	                                vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d vlan-range %d %d %d %d port %d\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].end_vid, pd->rules.rule[i].step, pd->rules.rule[i].mode, pd->rules.rule[i].port_step);
	                        	else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_ONEPORT)
	                        	{
	                            	portListLongToString(pd->rules.rule[i].portlist, portlist_str);
	                            	vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d vlan-range %d %d %d %d port_id %s\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].end_vid, pd->rules.rule[i].step, pd->rules.rule[i].mode, portlist_str);
	                        	}
	                    	}
	                	}
	            	}	
					
	                if(pvc->onuVlanEntryNum>1)
	                {
	                    for(i=1; i<pvc->onuVlanEntryNum; i++)
	                    {
	                        ONUVlanEntry_t *pe = &pd->vlanconf.entryarry[i];
	                        ULONG portall = pe->allPortMask;
	                        ULONG portuntag = pe->untagPortMask;
		                    char portstr[80]="";

	                        vty_out(vty, "vlan dot1q_add %d\r\n", pe->vlanid);
	                        if(portListLongToString(portuntag, portstr))
	                            vty_out(vty, "vlan dot1q_port_add %d %s 2\r\n", pe->vlanid, portstr);
	                    }	                    	    		        				
	                }
					
                }
        		else
    			{
	            	vty_out(vty, "vlan dot1q 0\r\n");
    			}
        	}
            if(pvc->vmtrunkportmask &&
            		!pvc->vmtranslationportmask &&
            		!pvc->vmtransparentportmask &&
            		!pvc->vmtagportmask &&
            		!pvc->vmaggportmask) /*使能1Q VLAN*/
            {

            	/*added by wangxiaoyu 2012-06-19 强制使能1Q VLAN并清除vlan配置*/
            	vty_out(vty, "vlan dot1q 1\r\n");
                /*显示vlan批处理配置命令modi by luh 2011-12*/
                if(!(slot||port||onuid))
                {  
                    ULONG begin_slot,begin_port,begin_onuid;
                    ULONG end_slot,end_port,end_onuid;  
                    
                    for(i=0;i<pd->rules.number_rules;i++)
                    {
                        if(pd->rules.rule[i].rule_mode)
                        {
                            begin_slot = GET_PONSLOT(pd->rules.rule[i].begin_onuid);
                            begin_port = GET_PONPORT(pd->rules.rule[i].begin_onuid);
                            begin_onuid = GET_ONUID(pd->rules.rule[i].begin_onuid);
                            end_slot = GET_PONSLOT(pd->rules.rule[i].end_onuid);
                            end_port = GET_PONPORT(pd->rules.rule[i].end_onuid);
                            end_onuid = GET_ONUID(pd->rules.rule[i].end_onuid);
                            
                            if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID)
                                vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d %d %d\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].mode);
                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID_PORT)
                            {
                                portListLongToString(pd->rules.rule[i].portlist, portlist_str);
                                vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d %d %d %s\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].mode, portlist_str);
                            }
                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE)
                                vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d vlan-range %d %d %d %d\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].end_vid, pd->rules.rule[i].step, pd->rules.rule[i].mode);
                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_PORT)
                                vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d vlan-range %d %d %d %d port %d\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].end_vid, pd->rules.rule[i].step, pd->rules.rule[i].mode, pd->rules.rule[i].port_step);
                        	else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_ONEPORT)
                        	{
                            	portListLongToString(pd->rules.rule[i].portlist, portlist_str);
                            	vty_out(vty, "vlan dot1q_add onu_range %d/%d/%d %d/%d/%d vlan-range %d %d %d %d port_id %s\r\n", begin_slot, begin_port, begin_onuid, end_slot, end_port, end_onuid, pd->rules.rule[i].begin_vid, pd->rules.rule[i].end_vid, pd->rules.rule[i].step, pd->rules.rule[i].mode, portlist_str);
                        	}
                    	}
                	}
            	}

                
                if(pvc->onuVlanEntryNum>1)
                {
                    for(i=1; i<pvc->onuVlanEntryNum; i++)
                    {
                        ONUVlanEntry_t *pe = &pd->vlanconf.entryarry[i];
                        ULONG portall = pe->allPortMask;
                        ULONG portuntag = pe->untagPortMask;
                    ULONG porttag = portall &(~portuntag);
                    char portstr[80]="";

                        vty_out(vty, "vlan dot1q_add %d\r\n", pe->vlanid);

                        if(portListLongToString(porttag, portstr))
                            vty_out(vty, "vlan dot1q_port_add %d %s 1\r\n", pe->vlanid, portstr);
                        if(portListLongToString(portuntag, portstr))
                            vty_out(vty, "vlan dot1q_port_add %d %s 2\r\n", pe->vlanid, portstr);
                    }
                    
    		        {
    		            int rnum = 0;
    		            int rmask[32], rv[32];
    		            vty_out_onuconf_port_var(pd->portconf, defaultVid&0x0fff, rmask, rv, rnum)
    		            if(rnum)
    		            {
    		                for(i=0; i<rnum; i++)
    		                {
    		                    if(portListLongToString(rmask[i], portlist_str))
    		                    {
    		                        if(rv[i]!=((def->portconf[0].defaultVid)&0x0fff))
    		                        vty_out(vty, "vlan pvid %s %d\r\n", portlist_str, rv[i]);
    		                    }
    		                }
    		            }
    		        }				
                }
            }

            {
                int mode = sfun_getOnuConfVlanMode(pd);
                if(mode == ONU_CONF_VLAN_MODE_TRANSLATION)
                {
                    vty_out(vty, "vlan mode translation\r\n");

                    for(i=0; i<ONU_MAX_PORT; i++)
                    {
                        ONUPortConf_t *opc = (ONUPortConf_t*)&pd->portconf[i];

                        if(pvc->vmtranslationportmask&(1<<i))
                        {
                            ONUVlanExtConf_t *ovc = &opc->extVlanConf;
                            int num = ovc->numberOfEntry;
                            int j;
                            for(j=0; j<num; j++)
                            {
                                vty_out(vty, "vlan-translation %d old-vid %d new-vid %d\r\n", i+1, ovc->extEntry.transArry[j].ovid,
                                        ovc->extEntry.transArry[j].svid);
                            }
                        }
                    }

                }
#if 0				
                else if(mode == ONU_CONF_VLAN_MODE_TAG)
                {
                    vty_out(vty, "vlan mode tag\r\n");
                }
#endif				
    			else if(mode == ONU_CONF_VLAN_MODE_AGG)
    			{
    				vty_out(vty, "vlan mode agg\r\n");				
    			}

            }

            vty_out(vty, "!\r\n\r\n");

            vty_out(vty, "!port config\r\n");

            /*port disable*/
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, enable, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].enable)
                                vty_out(vty, "port en %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }

        /*vlan-transparent disable*/
        {
            int rnum = 0;
            int rmask[32], rv[32];
            vty_out_onuconf_port_var(pd->portconf, vlanTransparentEnable, rmask, rv, rnum)
            if(rnum)
            {
                for(i=0; i<rnum; i++)
                {
                    if(portListLongToString(rmask[i], portlist_str))
                    {
                        if(rv[i]!=def->portconf[0].vlanTransparentEnable)
                            vty_out(vty, "vlan-transparent %s %d\r\n", portlist_str, rv[i]);
                    }
                }
            }
        }
            /*port auto negotination disable*/
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, mode, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].mode)
                                vty_out(vty, "port mode %s %d\r\n", portlist_str, rv[i]);
                        }
                        }
                }
            }

            /*port fec enable*/
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, fecEnable, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].fecEnable)
                                vty_out(vty, "port fc %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }

            /*port pause enable*/

            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, pauseEnable, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].pauseEnable)
                                vty_out(vty, "port pause %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }

            /*port ingress*/
            for(i=0; i<ONU_MAX_PORT; i++)
            {
                if(pd->portconf[i].ingressRateLimit)
                {
    				if(pd->portconf[i].ingressBurstMode == ONU_CONF_PORT_INGRESS_BURST_NONE &&
    				pd->portconf[i].ingressRateAction == ONU_CONF_PORT_INGRESS_ACT_NONE)
                    	vty_out(vty, "port ingress_rate %d %d %d\r\n", i+1, pd->portconf[i].ingressRateType, pd->portconf[i].ingressRateLimit);
    				else
    				{
    					char sz[80] = "";
    					VOS_Sprintf(sz, "port ingress_rate %d %d %d", i+1, pd->portconf[i].ingressRateType, pd->portconf[i].ingressRateLimit);

    					if(pd->portconf[i].ingressRateAction != ONU_CONF_PORT_INGRESS_ACT_NONE)
    					{
    						if(pd->portconf[i].ingressRateAction == ONU_CONF_PORT_INGRESS_ACT_DROP)
    							VOS_Sprintf(sz, "%s %s", sz, "drop");
    						else
    							VOS_Sprintf(sz, "%s %s", sz, "pause");
    					}

    					if(pd->portconf[i].ingressBurstMode != ONU_CONF_PORT_INGRESS_BURST_NONE)
    					{
    						switch(pd->portconf[i].ingressBurstMode)
    						{
    							case ONU_CONF_PORT_INGRESS_BURST_12K:
    								VOS_Sprintf(sz, "%s %s", sz, "12k");							
    								break;
    							case ONU_CONF_PORT_INGRESS_BURST_24K:
    								VOS_Sprintf(sz, "%s %s", sz, "24k");							
    								break;
    							case ONU_CONF_PORT_INGRESS_BURST_48K:
    								VOS_Sprintf(sz, "%s %s", sz, "48k");							
    								break;							
    							case ONU_CONF_PORT_INGRESS_BURST_96K:
    								VOS_Sprintf(sz, "%s %s", sz, "96k");							
    								break;														
    						}
    					}

    					vty_out(vty, "%s\r\n", sz);
    				}
                }
            }

            /*port egress*/
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, egressRateLimit, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].egressRateLimit)
                            vty_out(vty, "port egress_rate %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }

    		/*port mirror config*/

    		if(pd->ingressMirrorFromList || pd->egressMirrorFromList)
    		{
    			int type = 'a';
    			if(!pd->ingressMirrorFromList)
    			{
    				portListLongToString(pd->egressMirrorFromList, portlist_str);
    				type = 'e';
    			}
    			else if(!pd->egressMirrorFromList)
    			{
    				portListLongToString(pd->ingressMirrorFromList, portlist_str);
    				type = 'i';
    			}
    			else
    				portListLongToString(pd->ingressMirrorFromList, portlist_str);

    			
    			vty_out(vty, "port mirror_from %c %s 1\r\n", type, portlist_str);
    		}

    		if(pd->egressMirrorToList)
    			vty_out(vty, "port mirror_to e %d\r\n", pd->egressMirrorToList);

    		if(pd->ingressMirrorToList)
    			vty_out(vty, "port mirror_to i %d\r\n", pd->ingressMirrorToList);
    		
            vty_out(vty, "!\r\n\r\n");

            /*qinq*/
            vty_out(vty, "!QinQ Config\r\n");
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, qinqEntry.qinqMode, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].qinqEntry.qinqMode)
                            {
    	                        switch (rv[i])
    	                        {
    								case CTC_QINQ_MODE_PER_PORT_C:
    									vty_out(vty, "qinq-config %s %s \r\n", "pport", portlist_str);
    									break;
    								case CTC_QINQ_MODE_PER_VLAN_C:
    									vty_out(vty, "qinq-config %s %s \r\n",  "pvlan", portlist_str);
    									break;
    								default:
    									vty_out(vty, "qinq-config %s %s \r\n", "disable", portlist_str);
    									break;
    	                        }	
                            	
                            }
                        }
                    }
                }
            }

            for(i=0; i<ONU_MAX_PORT; i++)
            {
                ONUQinqEntry_t *pqinq = &pd->portconf[i].qinqEntry;

                if(pqinq->qinqMode == CTC_QINQ_MODE_PER_VLAN_C)
                {
                    int j=0;

                    for(j=0; j<pqinq->numberOfEntry; j++)
                    {
                        vty_out(vty, "qinq-config vlan-tag-add %d c-vid %d s-vid %d\r\n", i+1,
                                pqinq->qinqEntries[2*j], pqinq->qinqEntries[2*j+1]);
                    }
                }
            }

            vty_out(vty, "!QoS config\r\n");

            /*qos default priority*/
                    {
                        int rnum = 0;
                        int rmask[32], rv[32];
                        vty_out_onuconf_port_var(pd->portconf, defaultVid>>13, rmask, rv, rnum)
                        if(rnum)
                        {
                            for(i=0; i<rnum; i++)
                            {
                                if(portListLongToString(rmask[i], portlist_str))
                                {
                                    if((rv[i]!=(def->portconf[0].defaultVid)>>13) && rv[i])
                                    vty_out(vty, "qos def_pri %s %d\r\n", portlist_str, rv[i]);
                                }
                            }
                        }
                    }

    /*port Qos Rule*/
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, qosRule, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].qosRule)
                                vty_out(vty, "qos Rule %s %s\r\n", portlist_str, rv[i]?"ip":"user");
                        }
                    }
                }
            }
    /*Qos IP-DSCP priority*/
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, qosIpEnable, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].qosIpEnable)
                                vty_out(vty, "qos ip_pri_en %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }
    /*Qos 802.1p priority*/
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, qosUserEnable, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].qosUserEnable)
                                vty_out(vty, "qos user_pri_en %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }
    /*Qos Algorithm*/
    		if(pd->qosAlgorithm != def->qosAlgorithm)
    		    vty_out(vty, "qos algorithm %s\r\n", pd->qosAlgorithm?"spq":"wrr");

        if(pd->qos_rules[QOS_RULE_UP_DIRECTION].priority_mode!=QOS_MODE_PRIO_TRANS)
            vty_out(vty, "qos vlan priority_mode up priority_vid\r\n");
        
        if(pd->qos_rules[QOS_RULE_DOWN_DIRECTION].priority_mode!=QOS_MODE_PRIO_TRANS)
            vty_out(vty, "qos vlan priority_mode down priority_vid\r\n");
        
        {
            int index = 0;
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                gw_qos_classification_rule_t *qos_rule = &(pd->qos_rules[QOS_RULE_UP_DIRECTION].class_entry[index]);
                if(qos_rule->valid)
                {
                    if(qos_rule->mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "qos classifier %s %0x %s %d %d %d\r\n", "ether-type", qos_rule->value, "up", qos_rule->priority_mark, qos_rule->queue_mapped, 1);
                    else
                        vty_out(vty, "qos classifier %s %d %s %d %d %d\r\n", qos_rule->mode == BASE_ON_VLAN_ID?"vlan-id":"ip-protocol", qos_rule->value, "up", qos_rule->priority_mark, qos_rule->queue_mapped, 1);
                }
            }
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                gw_qos_classification_rule_t *qos_rule = &(pd->qos_rules[QOS_RULE_DOWN_DIRECTION].class_entry[index]);
                if(qos_rule->valid)
                {
                    if(qos_rule->mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "qos classifier %s %0x %s %d %d %d\r\n", "ether-type", qos_rule->value, "down", qos_rule->priority_mark, qos_rule->queue_mapped, 1);
                    else
                        vty_out(vty, "qos classifier %s %d %s %d %d %d\r\n", qos_rule->mode == BASE_ON_VLAN_ID?"vlan-id":"ip-protocol", qos_rule->value, "down", qos_rule->priority_mark, qos_rule->queue_mapped, 1);
                }
            }   
        }
        
        {
            int index = 0;
            for(index=0;index<MAX_PAS_RULES_FOR_SINGLE_PATH;index++)
            {
                gw_pas_rule_t *pas_rule = &(pd->pas_rules[PAS_RULE_UP_DIRECTION].rule_entry[index]);
                if(pas_rule->valid)
                {
                    if(pas_rule->mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "vlan pas-rule %s %0x %s %s %d %x %s %d\r\n", "ether-type", pas_rule->value, "up", 
                        pas_rule->action == PAS_RULE_ACTION_NONE?"none":pas_rule->action==PAS_RULE_ACTION_ATTACH?"attach":"exchange",
                        pas_rule->new_vid, pas_rule->vlan_type, pas_rule->prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 1);
                    else
                        vty_out(vty, "vlan pas-rule %s %d %s %s %d %x %s %d\r\n", pas_rule->mode == BASE_ON_VLAN_ID?"vlan-id":"ip-protocol", pas_rule->value, "up", 
                        pas_rule->action == PAS_RULE_ACTION_NONE?"none":pas_rule->action==PAS_RULE_ACTION_ATTACH?"attach":"exchange",
                        pas_rule->new_vid, pas_rule->vlan_type, pas_rule->prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 1);
                }
            }
            for(index=0;index<MAX_CLASS_RULES_FOR_SINGLE_PATH;index++)
            {
                gw_pas_rule_t *pas_rule = &(pd->pas_rules[PAS_RULE_DOWN_DIRECTION].rule_entry[index]);
                if(pas_rule->valid)
                {
                    if(pas_rule->mode == BASE_ON_ETHER_TYPE)
                        vty_out(vty, "vlan pas-rule %s %0x %s %s %d %x %s %d\r\n", "ether-type", pas_rule->value, "down", 
                        pas_rule->action == PAS_RULE_ACTION_NONE?"none":pas_rule->action==PAS_RULE_ACTION_ATTACH?"attach":"exchange",
                        pas_rule->new_vid, pas_rule->vlan_type, pas_rule->prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 1);
                    else
                        vty_out(vty, "vlan pas-rule %s %d %s %s %d %x %s %d\r\n\r\n", pas_rule->mode == BASE_ON_VLAN_ID?"vlan-id":"ip-protocol", pas_rule->value, "down", 
                        pas_rule->action == PAS_RULE_ACTION_NONE?"none":pas_rule->action==PAS_RULE_ACTION_ATTACH?"attach":"exchange",
                        pas_rule->new_vid, pas_rule->vlan_type, pas_rule->prio_source==PAS_RULE_PRIO_SOURCE_CLASSIFIER?"classifier":"original", 1);
                }
            }   
        }
/*ingressRateLimitBase*/
    		if(pd->ingressRateLimitBase != def->ingressRateLimitBase)
    		    vty_out(vty, "port ingress_rate_limit_base %d\r\n", pd->ingressRateLimitBase);
    /*Qos FE port replace 802.1p priority*/
            {
                int rnum = 0;
                int rmask[32], rv[32];
                int j = 0, k = 0;
                int masklist = 0;
            	for(j=0;j<8;j++)
                {
                    rnum = 0;
                    vty_out_onuconf_port_var(pd->portconf, qosPrioReplace[j], rmask, rv, rnum)
                    if(rnum)
                    {
                        for(i=0; i<rnum; i++)
                        {
                            for(k=0;k<ONU_MAX_PORT;k++)
                            {
                                if((rmask[i]>>k)&1)
                                {
                                    if(pd->portconf[k].qosUserEnable)
                                    {
                                        masklist |=(1<<k); 
                                    }
                                }
                            }
                            rmask[i] &= masklist;
                            if(portListLongToString(rmask[i], portlist_str))
                            {
                                if(j!=(rv[i]-1))
                                vty_out(vty, "qos user_pri_reg %s %d %d\r\n",portlist_str,j,rv[i]-1);      
                            }
                        }
                    }           	    
            	}
            }

    /*802.1p priority associated with traffic-class queue*/
            for(i=0;i<8;i++)
            {
                if(pd->qosMap.queue[i]!=0)
                {
                    if(i!=(pd->qosMap.queue[i]-1))
                        vty_out(vty, "qos user_pri_tc %d %d\r\n",i,(pd->qosMap.queue[i])-1);     
                }
            }

    /*IP-DSCP associated with traffi-class queue*/
            {
                int enable = 0;
                for(i=0;i<ONU_MAX_PORT;i++)
                {
                    enable |= pd->portconf[i].qosIpEnable;
                }
                if(enable)
                {
            for(i=0;i<8;i++)
            {
                int j = 0, k = 0;
                for(j=0;j<8;j++)
                {
                    if(pd->qosMap.qosDscpQueue[i][j]!=0&&pd->qosMap.qosDscpQueue[i][i]!=0xFF)
                    {
                        for(k=0;k<8;k++)
                        {
                            if(pd->qosMap.qosDscpQueue[i][j]&(1<<k))   
                            {
                                vty_out(vty, "qos dscp_tc %d %d\r\n",j*8+k,i);      
                                    }
                                }
                            }
                        }
                    }
                }
            }

            /*QOS set*/
            for(i=0; i<QOS_MAX_SET_COUNT; i++)
            {
            	int j,k;
            	qos_classification_rule_t *prule = (qos_classification_rule_t*)&pd->qosset[i];
            	for(j=0; j<QOS_MAX_RULE_COUNT_PER_SET; j++)
            	{
            		if(prule->valid)
            		{
            			vty_out(vty, "policy-map %d %d queue-map %d priority-mark %d\r\n", i+1, j+1, prule->queue_mapped, prule->priority_mark);

            			for(k=0; k<QOS_MAX_CLASS_RULE_PAIRS; k++)
            			{
            				qos_class_rule_entry *pe = &prule->entries[k];
            				if(prule->entrymask&(1<<k))
            				{
            					char vtext[80]="";
            					onuconf_qosRuleFieldValue_ValToStr_parase(pe->field_select, &pe->value, vtext);
            					vty_out(vty, "class-match %d %d %d select %s %s operator %s\r\n", i+1, j+1, k+1, qos_rule_field_sel_str(pe->field_select+1),
            							vtext, qos_rule_field_operator_str(pe->validation_operator+1) );
            				}
            			}
            		}
            		prule++;
            	}
            }

            for(i=0; i<ONU_MAX_PORT; i++)
            {
            	if(pd->portconf[i].qosSetSel)
            		vty_out(vty, "service-policy port %d policy %d\r\n", i+1, pd->portconf[i].qosSetSel);
            }

            vty_out(vty, "!\r\n\r\n");

            vty_out(vty, "!IGMP config\r\n");

            /*igmpsnooping set*/

            if(pd->igmpEnable)
                vty_out(vty, "igmpsnooping enable 1\r\n");

                /*max response time*/
                if(pd->igmpMaxResponseTime != def->igmpMaxResponseTime)
                    vty_out(vty, "igmpsnooping max_response_time %d\r\n", pd->igmpMaxResponseTime);

                /*host aging time for igmp*/
                if(pd->igmpHostAgeTime != def->igmpHostAgeTime)
                    vty_out(vty, "igmpsnooping host_aging_time %d\r\n", pd->igmpHostAgeTime);

                /*group aging time*/
                if(pd->igmpGroupAgeTime != def->igmpGroupAgeTime)
                    vty_out(vty, "igmpsnooping group_aging_time %d\r\n", pd->igmpGroupAgeTime);

                /*port fastleave enable*/
                {
                    int rnum = 0;
                    int rmask[32], rv[32];
                    vty_out_onuconf_port_var(pd->portconf, igmpFastLeaveEnable, rmask, rv, rnum)
                    if(rnum)
                    {
                        for(i=0; i<rnum; i++)
                        {
                            if(portListLongToString(rmask[i], portlist_str))
                            {
                                if(rv[i]!=def->portconf[0].igmpFastLeaveEnable)
                                {
                                    if(rmask[i] == 0xffffffff)
                                        vty_out(vty, "igmpsnooping fast_leave all %d\r\n", rv[i]);
                                    else
                                        vty_out(vty, "igmpsnooping fast_leave %s %d\r\n", portlist_str, rv[i]);
                                }
                            }
                        }
                    }
                }

                /*igmp gda_add*/
                {
                    ULONG addr, portlist, vid;
                    for(i=0;i<ONU_MAX_IGMP_GROUP;i++)
                    {
                        if(VOS_OK == sfun_getOnuConf_igmp_groupdata(pd, i, sv_enum_igmp_group_gda, &addr))
                        {
                            if(addr)
                        {
                            if(!(sfun_getOnuConf_igmp_groupdata(pd, i, sv_enum_igmp_group_vlanId, &vid)||
                                    sfun_getOnuConf_igmp_groupdata( pd, i, sv_enum_igmp_group_portmask, &portlist)))
                            {
                                if(portListLongToString(portlist, portlist_str))
                                        vty_out(vty, "igmpsnooping gda_add %d.%d.%d.%d %d %s\r\n", addr>>24, (addr>>16)&0xff,
                                                (addr>>8)&0xff, addr&0xff, vid, portlist_str);
                                }
                            }
                        }
                    }
                }

                /*port igmpsnooping max group*/
                {
                    int rnum = 0;
                    int rmask[32], rv[32];
                    vty_out_onuconf_port_var(pd->portconf, igmpMaxGroup, rmask, rv, rnum)
                    if(rnum)
                    {
                        for(i=0; i<rnum; i++)
                        {
                            if(portListLongToString(rmask[i], portlist_str))
                            {
                                if(rv[i]!=def->portconf[0].igmpMaxGroup)
                                vty_out(vty, "igmpsnooping group-num %s %d\r\n", portlist_str, rv[i]);
                            }
                        }
                    }
                }
                

            /*multicast vlan*/
            {
                char vlan_list[512] = "";
                ULONG list[ONU_MAX_IGMP_VLAN];
                int k = 0, num = 0, temp = 0;
            for(i = 0; i< ONU_MAX_PORT; i++)
            {
                if(pd->portconf[i].igmpVlanNum)
                {
                    int j;
                        num = pd->portconf[i].igmpVlanNum;
                        if(num>ONU_MAX_IGMP_VLAN)
                        {
                            VOS_ASSERT(0);
                            break;
                        }
                        for(j=0; j<num; j++)
                        {
                            list[j] = pd->portconf[i].igmpVlan[j];
                        }
                        for(j=0;j<num-1;j++)
                        {
                            for(k=0;k<num-1-j;k++)
                            {
                                if(list[k]>list[k+1])
                                {
                                    temp = list[k];
                                    list[k] = list[k+1];
                                    list[k+1] = temp;
                                }
                            }
                        }
                        VlanListArrayToString(list, num, vlan_list);
                        vty_out(vty, "multicast vlan %d %s\r\n", i+1, vlan_list);
                    }
                }
            }
            

            /*max igmp gruop*/
            if(pd->igmpMaxGroup != def->igmpMaxGroup)
                vty_out(vty, "igmpsnooping max_group %d\r\n", pd->igmpMaxGroup);

            /*global igmp fast leave enable*/
            if(pd->igmpFastLeaveEnable!=def->igmpFastLeaveEnable)
                vty_out(vty, "ctc igmp-fastleave %s\r\n", pd->igmpFastLeaveEnable?"enable":"disable");

            /*igmp auth enable*/
            if(pd->igmpAuthEnable!=def->igmpAuthEnable)
                vty_out(vty, "igmpsnooping auth enable 1\r\n");

    		/*ctc igmp tag strip set*/
            {
                int rnum = 0;
                int rmask[32], rv[32];
                vty_out_onuconf_port_var(pd->portconf, igmpTagStrip, rmask, rv, rnum)
                if(rnum)
                {
                    for(i=0; i<rnum; i++)
                    {
                        if(portListLongToString(rmask[i], portlist_str))
                        {
                            if(rv[i]!=def->portconf[0].igmpTagStrip)
                            vty_out(vty, "igmpsnooping tag strip %s %d\r\n", portlist_str, rv[i]);
                        }
                    }
                }
            }		
            vty_out(vty, "!\r\n\r\n");
        }

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        if ( flags & OnuProfile_Part_CMC )
        {
            ONUCmcConf_t *curr_cfg = &pd->cmcCfg;
            ONUCmcConf_t *default_cfg = &def->cmcCfg;

            generateCmcConfCl2vty(default_cfg, curr_cfg, vty);    
        }
#endif            

        if(pfile)
        {
        if(!cl_flush_vty_obuf_to_memfile(vty->obuf, pfile))
            ret = 0;
			else
				ret = 1;
        }
        else
            ret = 1;
    }

	if(def)
	    onuconf_free(def, ONU_CONF_MEM_DATA_ID);
	
    return ret;
}
/*added by luh 2011-12*/
extern int GWONU_Resume_CliCommand(short int olt_id, short int onu_id, char *buffer, int len);
int gpon_bat_debug = 1;
int generateOnuConfClMemFile_Bat(struct vty* vty, char *name, ULONG slot, ULONG port, ULONG onuid)
{
    int ret = VOS_OK;
    int lReturnValue = VOS_OK;    
    int lRlt = VOS_OK;
    ONUConfigData_t * pd = NULL;
    int valid_flag = 0;
    unsigned char ver = 0;
    short int PonPortIdx = GetPonPortIdxBySlot(slot, port);
	if(!name )
		return ret;

    
    pd = getOnuConfFromHashBucket(name);
    if(pd)
    {
        int i;
        char portlist_str[80]="";
        /*vlan bat*/
        {
            ULONG begin_slot,begin_port,begin_onuid;
            ULONG end_slot,end_port,end_onuid;  
            if(slot&&port&&onuid)
            {
                ONUVlanConf_t temp_vlan;
                int j = 0, k =0, m = 0;
                ULONG fenum, deviceIndex = 0,v_portlist = 0;
                int onu_num;
				int portlist_onebit=0;
                short int v_vid;
                short int olt_id = GetPonPortIdxBySlot(slot, port);
                OnuGen_Get_CtcVersion(olt_id, onuid-1, &ver);

                VOS_MemZero(&temp_vlan,sizeof(ONUVlanConf_t));
                deviceIndex = MAKEDEVID(slot, port, onuid);
                if(getDeviceCapEthPortNum(deviceIndex, &fenum) == VOS_OK)
                {    
                    for(k=0;k<fenum;k++)
                        v_portlist |= (1<<k);                    
                    for(i=0;i<pd->rules.number_rules;i++)
                    {
                        if(pd->rules.rule[i].rule_mode)
                        {
                            begin_slot = GET_PONSLOT(pd->rules.rule[i].begin_onuid);
                            begin_port = GET_PONPORT(pd->rules.rule[i].begin_onuid);
                            begin_onuid = GET_ONUID(pd->rules.rule[i].begin_onuid);
                            end_slot = GET_PONSLOT(pd->rules.rule[i].end_onuid);
                            end_port = GET_PONPORT(pd->rules.rule[i].end_onuid);
                            end_onuid = GET_ONUID(pd->rules.rule[i].end_onuid);
                            
                            /*检查onu是否在范围之内*/
                            onu_num = Get_OnuNumFromOnuRange(slot,port,onuid,end_slot,end_port,end_onuid);
                            if(onu_num == VOS_ERROR)
                                continue;
                            onu_num = Get_OnuNumFromOnuRange(begin_slot,begin_port,begin_onuid,slot,port,onuid);
                            if(onu_num == VOS_ERROR)
                                continue;
                            
                            if(!valid_flag)
                                valid_flag = 1;
                            
                            v_vid   = pd->rules.rule[i].begin_vid + (onu_num - 1)*pd->rules.rule[i].step;
                            if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID)
                            {
                                for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                {
                                    if(temp_vlan.entryarry[j].vlanid == pd->rules.rule[i].begin_vid)
                                		break;
                                }
                                if(j < temp_vlan.onuVlanEntryNum)
                                {                                    
                                    if(pd->rules.rule[i].mode == 2)
                                    {
                                        temp_vlan.entryarry[j].untagPortMask |= v_portlist;  
                                    }
                                    temp_vlan.entryarry[j].allPortMask |= v_portlist;
                                }
                                else
                                {
                                    k= temp_vlan.onuVlanEntryNum;
                                    temp_vlan.entryarry[k].vlanid = pd->rules.rule[i].begin_vid;
                                    if(pd->rules.rule[i].mode == 2)
                                    {
                                        temp_vlan.entryarry[j].untagPortMask |= v_portlist;  
                                    }
                                    temp_vlan.entryarry[j].allPortMask |= v_portlist;
                                    temp_vlan.onuVlanEntryNum++;
                                }
								/*if(0SYS_MODULE_IS_GPON(slot))
								{
	        						ret = OnuMgt_AddVlanPort(olt_id, onuid-1,pd->rules.rule[i].begin_vid, v_portlist,pd->rules.rule[i].mode);
								}*/
                            }
                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID_PORT)
                            {
                                for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                {
                                    if(temp_vlan.entryarry[j].vlanid == pd->rules.rule[i].begin_vid)
                                		break;
                                }
                                if(j < temp_vlan.onuVlanEntryNum)
                                {
                                    if(pd->rules.rule[i].mode == 2)
                                    {                                        
                                        temp_vlan.entryarry[j].untagPortMask |= pd->rules.rule[i].portlist & v_portlist;                                                                            
                                    }
                                    temp_vlan.entryarry[j].allPortMask |= pd->rules.rule[i].portlist & v_portlist;
                                }
                                else
                                {
                                    k= temp_vlan.onuVlanEntryNum;
                                    temp_vlan.entryarry[k].vlanid = pd->rules.rule[i].begin_vid;
                                    if(pd->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[k].untagPortMask |= pd->rules.rule[i].portlist & v_portlist;     
                                    temp_vlan.entryarry[k].allPortMask |= pd->rules.rule[i].portlist & v_portlist;
                                    temp_vlan.onuVlanEntryNum++;
                                }
								/*if(0SYS_MODULE_IS_GPON(slot))
								{
	        						ret = OnuMgt_AddVlanPort(olt_id, onuid-1,pd->rules.rule[i].begin_vid, temp_vlan.entryarry[j].allPortMask,pd->rules.rule[i].mode);
								}*/
                            }
                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE)
                            {
                                for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                {
                                    if(temp_vlan.entryarry[j].vlanid == v_vid)
                                		break;
                                }
                                if(j < temp_vlan.onuVlanEntryNum)
                                {
                                    if(pd->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[j].untagPortMask |= v_portlist;                                                                            
                                    temp_vlan.entryarry[j].allPortMask |= v_portlist;
                                }
                                else
                                {
                                    k= temp_vlan.onuVlanEntryNum;
                                    temp_vlan.entryarry[k].vlanid = v_vid;
                                    if(pd->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[k].untagPortMask |= v_portlist;     
                                    temp_vlan.entryarry[k].allPortMask |= v_portlist;
                                    temp_vlan.onuVlanEntryNum++;
                                }
								/*if(0SYS_MODULE_IS_GPON(slot))
								{
	        						ret = OnuMgt_AddVlanPort(olt_id, onuid-1,v_vid, v_portlist,pd->rules.rule[i].mode);
								}*/
                            }
                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_PORT)
                            {	
                                int t_vid = v_vid;
                                for(m=0;m<fenum;m++)
                                {
                                    if(t_vid >= (v_vid + pd->rules.rule[i].step))
                                        t_vid = v_vid;
                                    
                                    for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                    {
                                        if(temp_vlan.entryarry[j].vlanid == t_vid)
                                    		break;
                                    }
                                    if(j < temp_vlan.onuVlanEntryNum)
                                    {
                                        if(pd->rules.rule[i].mode == 2)
                                            temp_vlan.entryarry[j].untagPortMask |= 1<<m;                                                                            
                                        temp_vlan.entryarry[j].allPortMask |= 1<<m;
                                    }
                                    else
                                    {
                                        k= temp_vlan.onuVlanEntryNum;
                                        temp_vlan.entryarry[k].vlanid = t_vid;
                                        if(pd->rules.rule[i].mode == 2)
                                            temp_vlan.entryarry[k].untagPortMask |= 1<<m;     
                                        temp_vlan.entryarry[k].allPortMask |= 1<<m;
                                        temp_vlan.onuVlanEntryNum++;
                                	}   
									/*if(0SYS_MODULE_IS_GPON(slot))
									{
										portlist_onebit = 0;
										portlist_onebit |= (1<<m);
	        							ret = OnuMgt_AddVlanPort(olt_id, onuid-1,t_vid, portlist_onebit,pd->rules.rule[i].mode);
									}*/
									t_vid += pd->rules.rule[i].port_step;  
                                }
                            }
                            else if(pd->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_ONEPORT)
                            {
                                for(j=0;j<temp_vlan.onuVlanEntryNum;j++)
                                {
                                    if(temp_vlan.entryarry[j].vlanid == v_vid)
                                		break;
                                }
                                if(j < temp_vlan.onuVlanEntryNum)
                                {
                                    if(pd->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[j].untagPortMask |= pd->rules.rule[i].portlist;                                                                            
                                    temp_vlan.entryarry[j].allPortMask |= pd->rules.rule[i].portlist;
                                }
                                else
                                {
                                    k= temp_vlan.onuVlanEntryNum;
                                    temp_vlan.entryarry[k].vlanid = v_vid;
                                    if(pd->rules.rule[i].mode == 2)
                                        temp_vlan.entryarry[k].untagPortMask |= pd->rules.rule[i].portlist;     
                                    temp_vlan.entryarry[k].allPortMask |= pd->rules.rule[i].portlist;
                                    temp_vlan.onuVlanEntryNum++;
                                }  
								/*if(0SYS_MODULE_IS_GPON(slot))
								{
	        						ret = OnuMgt_AddVlanPort(olt_id, onuid-1,v_vid, temp_vlan.entryarry[k].allPortMask,pd->rules.rule[i].mode);
								}*/
                            }                            
                        }
                    }
					/*if(0SYS_MODULE_IS_GPON(slot))
					{
						return ret;
					}*/
                }
                
                if(valid_flag)
                {
                    if(temp_vlan.onuVlanEntryNum)
                    {
                        for(i=0;i<fenum;i++)
                        {   
                            short int port_untag = pd->portconf[i].defaultVid & 0xfff;
                            if(port_untag != 1)
                            {
                                for(k=0;k<temp_vlan.onuVlanEntryNum;k++)
                                {
                                    if(temp_vlan.entryarry[k].untagPortMask&(1<<i) && temp_vlan.entryarry[k].vlanid != port_untag)
                                    {
                                        temp_vlan.entryarry[k].untagPortMask &= ~(1<<i);     
                                        temp_vlan.entryarry[k].allPortMask &= ~(1<<i);
                                        break;
                                    }
                                }
                            }
                        } 
                    }
					if(SYS_MODULE_IS_GPON(slot))
					{
						for(k=0;k<temp_vlan.onuVlanEntryNum;k++)
						{
							ULONG tag = temp_vlan.entryarry[k].allPortMask & (~(temp_vlan.entryarry[k].untagPortMask));
							if(temp_vlan.entryarry[k].untagPortMask != NULL)
	        				ret = OnuMgt_AddVlanPort(olt_id, onuid-1,temp_vlan.entryarry[k].vlanid, temp_vlan.entryarry[k].untagPortMask,2);
							if(ret == VOS_ERROR)
								return VOS_ERROR;
							if(tag != NULL)
							ret = OnuMgt_AddVlanPort(olt_id, onuid-1,temp_vlan.entryarry[k].vlanid, tag,1);
						}
						if(ret == VOS_OK)
								return VOS_OK;
							else
								return VOS_ERROR;
					}
                    if(IsCtcOnu(olt_id, onuid-1))
                    {
                        CTC_STACK_vlan_configuration_ports_t ports_info;
                        unsigned char number_of_entries;
                        short int llid;

                        /* B--modified by liwei056@2014-5-28 for D21028 */                    
#if 1
                        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onuid-1)) )
                        {
                            if ( 0 != OnuMgt_GetAllPortVlanConfig (olt_id, onuid-1, &number_of_entries, ports_info) )
                            {
                                number_of_entries = fenum;
                                VOS_MemSet(&ports_info, CTC_STACK_ALL_PORTS, sizeof(ports_info));
                            }
                        }
                        else
                        {
                            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %s(%d, %d) failed, line %d!\r\n", __FUNCTION__, olt_id, onuid-1, __LINE__);                        
                            return VOS_ERROR;
                        }
#else
                        if ( INVALID_LLID != (llid = GetLlidActivatedByOnuIdx(olt_id, onuid-1)) )
    						#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
    						OnuMgt_GetAllPortVlanConfig(olt_id, onuid-1, &number_of_entries, ports_info);
    						#else
                            CTC_STACK_get_vlan_all_port_configuration (olt_id, llid, &number_of_entries, ports_info);
    						#endif
#endif
                        /* E--modified by liwei056@2014-5-28 for D21028 */                    
                        if(temp_vlan.onuVlanEntryNum)
                        {
                            for(i=0;i<fenum;i++)
                            {   
                                int need_set = 0;
                                /* B--added by liwei056@2014-5-28 for D21028 */                    
                                if( CTC_STACK_ALL_PORTS == ports_info[i].port_number )
                                {
                                    if ( 0 == OnuMgt_GetEthPortVlanConfig(olt_id, onuid-1, i+1, &ports_info[i].entry) )
                                    {
                                        ports_info[i].port_number = i+1;
                                    }
                                    else
                                    {
                                        ports_info[i].port_number = 0;
                                        continue;
                                    }  
                                }
                                if(ver >= CTC_2_1_ONU_VERSION)
                                {
                                    /*modi by luh 2013-12-09, 要配置的vlan的端口其模式必须为trunk*/
                                    if(ports_info[i].entry.mode == CTC_VLAN_MODE_TRUNK)
                                    {
                                        for(k=0;k<temp_vlan.onuVlanEntryNum;k++)
                                        {
                                            if(temp_vlan.entryarry[k].untagPortMask&(1<<i))
                                            {
                                                ports_info[i].entry.default_vlan = (0x8100<<16)|temp_vlan.entryarry[k].vlanid;
                                                need_set = 1;
                                                break;
                                            }
                                        }
										
                                        for(k=0;k<temp_vlan.onuVlanEntryNum;k++)
                                        {
                                            ULONG tag = temp_vlan.entryarry[k].allPortMask & (~(temp_vlan.entryarry[k].untagPortMask));
                                            if(!(tag&(1<<i)))
                                                continue;
                                                                        
                                            for(m=0;m<ports_info[i].entry.number_of_entries;m++)
                                            {
                                                if((ports_info[i].entry.vlan_list[m]&0xfff) == temp_vlan.entryarry[k].vlanid)
                                                break;
                                            }
                                            if(m >= ports_info[i].entry.number_of_entries)
                                            {
                                                int num_entry = ports_info[i].entry.number_of_entries;
                                                ports_info[i].entry.vlan_list[num_entry] = temp_vlan.entryarry[k].vlanid;
                                                ports_info[i].entry.number_of_entries++;
                                                need_set = 1;
                                            }
                                        }
                                        if(need_set)
										{
											
        									#if 1/*for 10G EPON of PMC8411 Adapter Extension by jinhl @2012-11-12*/
												ret = OnuMgt_SetEthPortVlanConfig(olt_id, onuid-1, i+1, &ports_info[i].entry);
        									#else
											
                                            ret |= CTC_STACK_set_vlan_port_configuration (olt_id, llid, i+1, ports_info[j].entry);
        									#endif
        									if(ret != VOS_OK)
        									{
                                                lReturnValue = ret;        									
                                                OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d), line %d!\r\n", lReturnValue, __FUNCTION__, olt_id, onuid-1, i+1, __LINE__);                                									
        									}
                                    	}
                                    }
									else if(ports_info[i].entry.mode == CTC_VLAN_MODE_TAG)
									{
										/*tag 模式下只配置untag vlan*/
                                        for(k=0;k<temp_vlan.onuVlanEntryNum;k++)
                                        {
                                            if(temp_vlan.entryarry[k].untagPortMask&(1<<i))
                                            {
                                                ports_info[i].entry.vlan_list[0] = temp_vlan.entryarry[k].vlanid;
                                                ports_info[i].entry.number_of_entries = 1;
                                                break;
                                            }
                                        }
                                        
    									ret = OnuMgt_SetEthPortVlanConfig(olt_id, onuid-1, i+1, &ports_info[i].entry);
                                        if(ret != VOS_OK)
                                        {
                                            lReturnValue = ret;                                        
                                            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d), line %d!\r\n", lReturnValue, __FUNCTION__, olt_id, onuid-1, i+1, __LINE__);                                                                    
                                        }										
									}
									else
									{
                                    }
                                }
                                else
                                {
                                    /*modi by luh 2013-12-09, 要配置的vlan的端口其模式必须为trunk*/
                                    if(ports_info[i].entry.mode == CTC_VLAN_MODE_TAG)
                                    {
                                        for(k=0;k<temp_vlan.onuVlanEntryNum;k++)
                                        {
                                            if(temp_vlan.entryarry[k].untagPortMask&(1<<i))
                                            {
                                                ports_info[i].entry.vlan_list[0] = temp_vlan.entryarry[k].vlanid;
                                                ports_info[i].entry.number_of_entries = 1;
                                                break;
                                            }
                                        }
										ret = OnuMgt_SetEthPortVlanConfig(olt_id, onuid-1, i+1, &ports_info[i].entry);
                                        if(ret != VOS_OK)
                                        {
                                            lReturnValue = ret;                                        
                                            OLT_SYSLOG_DEBUG(LOG_TYPE_ONU, LOG_ERR, "%% %d=%s(%d, %d, %d), line %d!\r\n", lReturnValue, __FUNCTION__, olt_id, onuid-1, i+1, __LINE__);                                                                    
                                        }
                                    }                                    
                                }                                    
                            }
                        }
                    }
                    else
                    {
                        int len = 0;                    
                        char *str = NULL;
                        if(temp_vlan.onuVlanEntryNum)
                        {
                            str = VOS_Malloc(GW_ONU_RESUME_BUF_MAX_SIZE, MODULE_RPU_CTC);/*申请8k 空间，传输 GW -ONU命令行，可以同时下发50条vlan的配置命令 */
                            if(str)
                                VOS_MemZero(str, GW_ONU_RESUME_BUF_MAX_SIZE);
                            else
                                return VOS_ERROR;
                            
                            for(i=0; i<temp_vlan.onuVlanEntryNum; i++)
                            {
                                ONUVlanEntry_t *pe = &temp_vlan.entryarry[i];
                                ULONG portall = pe->allPortMask;
                                ULONG portuntag = pe->untagPortMask;
                                ULONG porttag = portall &(~portuntag);
                                if(len+25 < GW_ONU_RESUME_BUF_MAX_SIZE)
                                    len += VOS_Snprintf(str+len, 25, "vlan dot1q_add %d\r\n", pe->vlanid);
                                if(portListLongToString(porttag, portlist_str)&&(len+50 < GW_ONU_RESUME_BUF_MAX_SIZE))
                                    len += VOS_Snprintf(str+len, 50, "vlan dot1q_port_add %d %s 1\r\n", pe->vlanid, portlist_str);
                                if(portListLongToString(portuntag, portlist_str)&&(len+50 < GW_ONU_RESUME_BUF_MAX_SIZE))
                                    len += VOS_Snprintf(str+len, 50, "vlan dot1q_port_add %d %s 2\r\n", pe->vlanid, portlist_str);
                            }
                        }
                        else/*没有配置数据要恢复*/
                            return VOS_OK;
                        
                        if(len)
                        {
                            if (GWONU_Resume_CliCommand(olt_id,  onuid-1, str, len) != VOS_OK)
                            {
                                lReturnValue = VOS_ERROR;
                            }
                        }
                        if(str)
                            VOS_Free(str);
                    }
                }
            }
        }         
    }	
    return lReturnValue;
}

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
int cmc_show_running_config_all(USHORT pon_id, USHORT onu_id, ONUCmcConf_t *cmcCurrCfg)
{
    short int PonPortIdx;
    short int OnuIdx;
    
    PonPortIdx = pon_id;
    OnuIdx = onu_id - 1;
    
    if ( OnuIsCMC(PonPortIdx, OnuIdx) )
    {
        int iRlt;
        int  i;
        char cmcMac[BYTES_IN_MAC_ADDRESS];
        unsigned long ulByteOffset, ulBitOffset, ulByteMask;
        unsigned long aulValues[CMC_MAX_DS_CH];
        short int *psValues = (short int*)aulValues;
        unsigned char  *pucValues = (unsigned char*)aulValues;

    	if (0 != (iRlt = GetOnuMacAddr(PonPortIdx, OnuIdx, cmcMac, &i)))
    	{
    		return iRlt;
    	}  

    	if (0 == OnuMgt_GetCmcMaxCm(PonPortIdx, OnuIdx, cmcMac, pucValues))
    	{
    	    if ( CMC_CFG_MAX_CM_DEFAULT != *pucValues )
            {
                cmcCurrCfg->maxCm = pucValues;
            }   
    	}

    	if (0 == OnuMgt_GetCmcDownChannelMode(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, pucValues))
    	{
    	    for ( i = 0; i < CMC_MAX_DS_CH; i++ )
            {
                if ( pucValues[i] )
                {
                    ulByteOffset = i >> 3;
                    ulBitOffset  = 1 << (i & 7);
                    cmcCurrCfg->aucDsChannelEnable[ulByteOffset] |= ulBitOffset;
                }
            }   
    	}

    	if (0 == OnuMgt_GetCmcUpChannelMode(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, pucValues))
    	{
    	    for ( i = 0; i < CMC_MAX_US_CH; i++ )
            {
                if ( !pucValues[i] )
                {
                    ulBitOffset = 1 << i;
                    cmcCurrCfg->aucUsChannelEnable[0] &= ~ulBitOffset;
                }
            }   
    	}
        
    	if (0 == OnuMgt_GetCmcDownChannelFreq(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, aulValues))
    	{
    	    for ( i = 0; i < CMC_MAX_DS_CH; i++ )
            {
                if ( CMC_CFG_DOWN_CHANNEL_FREQ_DEFAULT(i) != aulValues[i] )
                {
                    cmcCurrCfg->aulDsChannelFreq[i] = aulValues[i];
                }
            }   
    	}
        
    	if (0 == OnuMgt_GetCmcUpChannelFreq(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, aulValues))
    	{
    	    for ( i = 0; i < CMC_MAX_US_CH; i++ )
            {
                if ( CMC_CFG_UP_CHANNEL_FREQ_DEFAULT(i) != aulValues[i] )
                {
                    cmcCurrCfg->aulUsChannelFreq[i] = aulValues[i];
                }
            }   
    	}
        
    	if (0 == OnuMgt_GetCmcUpChannelWidth(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, aulValues))
    	{
    	    for ( i = 0; i < CMC_MAX_US_CH; i++ )
            {
                if ( CMC_CFG_UP_CHANNEL_FREQ_WIDTH_DEFAULT != aulValues[i] )
                {
                    cmcCurrCfg->aulUsChannelFreqWidth[i] = aulValues[i];
                }
            }   
    	}
        
    	if (0 == OnuMgt_GetCmcDownChannelPower(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, psValues))
    	{
    	    for ( i = 0; i < CMC_MAX_DS_CH; i++ )
            {
                if ( CMC_CFG_DOWN_CHANNEL_POWER_LEVEL_DEFAULT != psValues[i] )
                {
                    cmcCurrCfg->asDsChannelPower[i] = psValues[i];
                }
            }   
    	}
        
    	if (0 == OnuMgt_GetCmcUpChannelPower(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, psValues))
    	{
    	    for ( i = 0; i < CMC_MAX_US_CH; i++ )
            {
                if ( CMC_CFG_UP_CHANNEL_POWER_LEVEL_DEFAULT != psValues[i] )
                {
                    cmcCurrCfg->asUsChannelPower[i] = psValues[i];
                }
            }   
    	}

    	if (0 == OnuMgt_GetCmcUpChannelD30Mode(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, pucValues))
    	{
    	    for ( i = 0; i < CMC_MAX_US_CH; i++ )
            {
                if ( !pucValues[i] )
                {
                    ulBitOffset = 1 << i;
                    cmcCurrCfg->aucUsChannelD30[0] &= ~ulBitOffset;
                }
            }   
    	}

    	if (0 == OnuMgt_GetCmcUpChannelType(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, pucValues))
    	{
    	    for ( i = 0; i < CMC_MAX_US_CH; i++ )
            {
                if ( CMC_CFG_UP_CHANNEL_TYPE_DEFAULT != pucValues[i] )
                {
                    ulBitOffset = 1 << i;
                    cmcCurrCfg->aucUsChannelType[0] |= ulBitOffset;
                }
            }   
    	}

    	if (0 == OnuMgt_GetCmcUpChannelProfile(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, pucValues))
    	{
    	    for ( i = 0; i < CMC_MAX_US_CH; i++ )
            {
                if ( CMC_CFG_UP_CHANNEL_PROFILE_DEFAULT != pucValues[i] )
                {
                    ulByteOffset = i >> 1;
                    ulBitOffset  = (i & 1) << 2;
                    ulByteMask   = ~(0xF << ulBitOffset);
                    cmcCurrCfg->aucUsChannelProfile[ulByteOffset] = (cmcCurrCfg->aucUsChannelProfile[ulByteOffset] & ulByteMask) | (pucValues[i] << ulBitOffset);
                }
            }   
    	}
        
    	if (0 == OnuMgt_GetCmcDownChannelAnnexMode(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, pucValues))
    	{
    	    for ( i = 0; i < CMC_MAX_DS_CH; i++ )
            {
                if ( CMC_CFG_DOWN_CHANNEL_ANNEX_DEFAULT != pucValues[i] )
                {
                    ulByteOffset = i >> 3;
                    ulBitOffset  = 1 << (i & 7);
                    cmcCurrCfg->aucDsChannelAnnex[ulByteOffset] |= ulBitOffset;
                }
            }   
    	}

    	if (0 == OnuMgt_GetCmcDownChannelInterleaver(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, pucValues))
    	{
    	    for ( i = 0; i < CMC_MAX_DS_CH; i++ )
            {
                if ( CMC_CFG_DOWN_CHANNEL_INTERLEAVER_DEFAULT != pucValues[i] )
                {
                    ulByteOffset = i >> 1;
                    ulBitOffset  = (i & 1) << 2;
                    ulByteMask   = ~(0xF << ulBitOffset);
                    cmcCurrCfg->aucDsChannelInterleaver[ulByteOffset] = (cmcCurrCfg->aucDsChannelInterleaver[ulByteOffset] & ulByteMask) | (pucValues[i] << ulBitOffset);
                }
            }   
    	}

    	if (0 == OnuMgt_GetCmcDownChannelModulation(PonPortIdx, OnuIdx, cmcMac, CMC_CHANNELID_ALL, pucValues))
    	{
    	    for ( i = 0; i < CMC_MAX_DS_CH; i++ )
            {
                if ( CMC_CFG_DOWN_CHANNEL_MODULATION_DEFAULT != pucValues[i] )
                {
                    ulByteOffset = i >> 2;
                    ulBitOffset  = (i & 3) << 1;
                    ulByteMask   = ~(3 << ulBitOffset);
                    cmcCurrCfg->aucDsChannelModulation[ulByteOffset] = (cmcCurrCfg->aucDsChannelModulation[ulByteOffset] & ulByteMask) | (pucValues[i] << ulBitOffset);
                }
            }   
    	}

        cmcCurrCfg->cmcIsEnabled = TRUE;
    }
    else
    {
        /* 此ONU不是CMC，则无CMC配置 */
        cmcCurrCfg->cmcIsEnabled = FALSE;
    }

    return 0;
}
#endif

int ctc_show_running_config_all(USHORT pon_id, USHORT onu_id, ONUConfigData_t *pd)
{
    int ret = 0;
    int error_flag = 0;
    unsigned char ver = 0;
    short int ulSlot = GetCardIdxByPonChip(pon_id);
    short int ulPort = GetPonPortByPonChip(pon_id);
    
    OnuGen_Get_CtcVersion(pon_id, onu_id-1, &ver);    
    if(pd)
	{
		int i;
		int maxPortNum = getOnuEthPortNum(pon_id, onu_id - 1);
		if(ThisIsGponOnu(pon_id, onu_id - 1))
		{
		}
		else
		{
			{
				int enable = 0;
				if(VOS_OK == OnuMgt_GetMulticastSwitch(pon_id, onu_id - 1, &enable))
				{
					if(enable == CTC_STACK_PROTOCOL_IGMP_MLD_SNOOPING)
						pd->igmpEnable = ENABLE;
				}
				else
					error_flag = 1;
			}

			/*global igmp fast leave enable*/
			{
				int enable = 0;
				if(VOS_OK == OnuMgt_GetMulticastFastleave(pon_id, onu_id - 1, &enable))
					pd->igmpFastLeaveEnable = enable == CTC_STACK_FAST_LEAVE_ADMIN_STATE_ENABLED?1:0;
			}

			/*igmp auth enable*/
			{
				int enable = 0;
				if(VOS_OK == OnuMgt_GetMulticastSwitch(pon_id, onu_id - 1, &enable))
					pd->igmpAuthEnable = enable;
			}
		}

		{
			CTC_STACK_holdover_state_t holdover;
			if(VOS_OK == OnuMgt_GetHoldOver(pon_id, onu_id-1, &holdover))
			{
				if(holdover.holdover_state == CTC_STACK_HOLDOVER_STATE_ACTIVATED)
					pd->holdover = holdover.holdover_time;
			}
		}

		{
			int port = 0, enable=0;
			int tag = 0, num = 0;
			CTC_STACK_ethernet_port_policing_entry_t policing;   
			CTC_STACK_ethernet_port_ds_rate_limiting_entry_t policing_egress;      
			CTC_STACK_port_qinq_configuration_t pvc; 
			CTC_STACK_multicast_vlan_t mv;    

			if(ThisIsGponOnu(pon_id, onu_id - 1))
			{
				for(port=0;port<maxPortNum;port++)
				{
					enable = 0;
					if(VOS_OK == OnuMgt_GetEthPortAdminStatus(pon_id, onu_id - 1, port+1, &enable))
						pd->portconf[port].enable = enable;
				}		
			}
			else
			{
				for(port=0;port<maxPortNum;port++)
				{
					VOS_MemZero(&policing, sizeof(CTC_STACK_ethernet_port_policing_entry_t));
					VOS_MemZero(&policing_egress, sizeof(CTC_STACK_ethernet_port_ds_rate_limiting_entry_t));
					VOS_MemZero(&pvc, sizeof(CTC_STACK_port_qinq_configuration_t));
					VOS_MemZero(&mv, sizeof(CTC_STACK_multicast_vlan_t));
					enable = 0;
					if(VOS_OK == OnuMgt_GetEthPortAdminStatus(pon_id, onu_id - 1, port+1, &enable))
						pd->portconf[port].enable = enable;
					enable = 0;                
					if(VOS_OK == OnuMgt_GetEthPortAutoNegotiationAdmin(pon_id, onu_id - 1, port+1, &enable))
						pd->portconf[port].mode = enable?0:8;
					enable = 0;
					if(VOS_OK == OnuMgt_GetEthPortPause(pon_id, onu_id - 1, port+1, &enable))
						pd->portconf[port].pauseEnable = enable;
					if(VOS_OK == OnuMgt_GetEthPortPolicing(pon_id, onu_id - 1, port+1, &policing))
						pd->portconf[port].ingressRateLimit = policing.cir;
					if(VOS_OK == OnuMgt_GetEthPortDownstreamPolicing(pon_id, onu_id - 1, port+1, &policing_egress))
						pd->portconf[port].egressRateLimit = policing_egress.CIR;
					if(VOS_OK == get_OnuVlanTagConfig(pon_id, onu_id - 1, port+1, &pvc))
					{
						pd->portconf[port].qinqEntry.qinqMode = pvc.mode;
						pd->portconf[port].qinqEntry.numberOfEntry = pvc.number_of_entries;
						for(i=0;i<pvc.number_of_entries;i++)
							pd->portconf[port].qinqEntry.qinqEntries[i] = pvc.vlan_list[i];  
					}
					tag = 0;
					if(VOS_OK == OnuMgt_GetEthPortMulticastTagStrip(pon_id, onu_id - 1, port+1, &tag))
						pd->portconf[port].igmpTagStrip = tag;
					num = 0;
					if(VOS_OK == OnuMgt_GetEthPortMulticastGroupMaxNumber(pon_id, onu_id - 1, port+1, &num))
						pd->portconf[port].igmpMaxGroup = num;
					if(VOS_OK == OnuMgt_GetEthPortMulticastVlan(pon_id, onu_id - 1, port+1, &mv))
					{
						pd->portconf[port].igmpVlanNum = mv.num_of_vlan_id;
						for(i=0;i<mv.num_of_vlan_id;i++)
							pd->portconf[port].igmpVlan[i]=mv.vlan_id[i];
					}
				}
			}
		}

		{
			int fec_mode = 0;
			if(VOS_OK == CTC_GetLlidFecMode( pon_id, onu_id-1, &fec_mode))
				pd->fecenable = fec_mode == STD_FEC_MODE_ENABLED?1:0;
		}     

		{

			int flag_untag, flag_all;
			int port = 0, j = 0, k = 0, num = 0;

			CTC_STACK_port_vlan_configuration_t Vconfig;
			pd->vlanconf.vmtrunkportmask = 0;
			pd->vlanconf.entryarry[0].allPortMask = 0;
			pd->vlanconf.entryarry[0].untagPortMask = 0;
			for(port=0;port<maxPortNum;port++)
			{
				flag_untag = 0;
				flag_all = 0;
				if(VOS_OK == OnuMgt_GetEthPortVlanConfig(pon_id, onu_id - 1, port+1, &Vconfig))
				{
					num = Vconfig.number_of_entries;
					if(Vconfig.mode == CTC_VLAN_MODE_TRUNK)
					{
						if(Vconfig.default_vlan & 0xfff)
							pd->portconf[port].defaultVid = Vconfig.default_vlan & 0xfff;

						pd->vlanconf.vmtrunkportmask |= 1<<port;
						for(j=0;j<pd->vlanconf.onuVlanEntryNum;j++)
						{
							if(pd->vlanconf.entryarry[j].vlanid == pd->portconf[port].defaultVid)
							{
								flag_untag = 1;
								pd->vlanconf.entryarry[j].untagPortMask |= 1<<port;
								pd->vlanconf.entryarry[j].allPortMask |= 1<<port;
								break;
							}
						}
						if(!flag_untag)
						{
							k = pd->vlanconf.onuVlanEntryNum;
							pd->vlanconf.entryarry[k].vlanid = pd->portconf[port].defaultVid;  
							pd->vlanconf.entryarry[k].untagPortMask |= 1<<port;
							pd->vlanconf.entryarry[k].allPortMask |= 1<<port;
							pd->vlanconf.onuVlanEntryNum++;
						}

						for(k=0;k<num;k++)
						{
							flag_all = 0;
							for(j=0;j<pd->vlanconf.onuVlanEntryNum;j++)
							{
								if(pd->vlanconf.entryarry[j].vlanid == (Vconfig.vlan_list[k]&0xfff))
								{
									flag_all = 1;
									pd->vlanconf.entryarry[j].allPortMask |= 1<<port;
									pd->vlanconf.entryarry[j].untagPortMask &= ~(1<<port);
									break;
								}
							}
							if(!flag_all)
							{
								i = pd->vlanconf.onuVlanEntryNum;
								pd->vlanconf.entryarry[i].vlanid = Vconfig.vlan_list[k]&0xfff;  
								pd->vlanconf.entryarry[i].allPortMask |= 1<<port;
								pd->vlanconf.onuVlanEntryNum++;
							}
						} 
					} 
					else if(ONU_CONF_VLAN_MODE_TRANSLATION == Vconfig.mode)
					{
						pd->vlanconf.vmtranslationportmask |= 1<<port;
						pd->vlanconf.vmtrunkportmask &= ~(1<<port);
						pd->portconf[port].extVlanConf.numberOfEntry = Vconfig.number_of_entries;
						for(i=0;i<num;i++)
						{
							pd->portconf[port].extVlanConf.extEntry.transArry[i].ovid = Vconfig.vlan_list[2*i];
							pd->portconf[port].extVlanConf.extEntry.transArry[i].svid = Vconfig.vlan_list[2*i+1];
						}
					}
					else if(ONU_CONF_VLAN_MODE_TAG == Vconfig.mode)
					{
						if(ver >= CTC_2_1_ONU_VERSION)
						{
							pd->vlanconf.vmtagportmask |= 1<<port;
							pd->vlanconf.vmtrunkportmask &= ~(1<<port);
                            for(j=0;j<pd->vlanconf.onuVlanEntryNum;j++)
                            {
                                if(pd->vlanconf.entryarry[j].vlanid == (Vconfig.vlan_list[0]&0xfff))
                                {
                                    flag_untag = 1;
                                    pd->vlanconf.entryarry[j].untagPortMask |= 1<<port;
                                    pd->vlanconf.entryarry[j].allPortMask |= 1<<port;
                                    break;
                                }
                            }
                            if(!flag_untag)
                            {
                                k = pd->vlanconf.onuVlanEntryNum;
                                pd->vlanconf.entryarry[k].vlanid = (Vconfig.vlan_list[0]&0xfff);  
                                pd->vlanconf.entryarry[k].untagPortMask |= 1<<port;
                                pd->vlanconf.entryarry[k].allPortMask |= 1<<port;
                                pd->vlanconf.onuVlanEntryNum++;
                            }                                                           
 							
                        }
						else
						{
							pd->vlanconf.vmtrunkportmask |= 1<<port;

							for(j=0;j<pd->vlanconf.onuVlanEntryNum;j++)
							{
								if(pd->vlanconf.entryarry[j].vlanid == (Vconfig.vlan_list[0]&0xfff))
								{
									flag_untag = 1;
									pd->vlanconf.entryarry[j].untagPortMask |= 1<<port;
									pd->vlanconf.entryarry[j].allPortMask |= 1<<port;
									break;
								}
							}
							if(!flag_untag)
							{
								k = pd->vlanconf.onuVlanEntryNum;
								pd->vlanconf.entryarry[k].vlanid = (Vconfig.vlan_list[0]&0xfff);  
								pd->vlanconf.entryarry[k].untagPortMask |= 1<<port;
								pd->vlanconf.entryarry[k].allPortMask |= 1<<port;
								pd->vlanconf.onuVlanEntryNum++;
							}                                                           
						}
					}
					else if(ONU_CONF_VLAN_MODE_AGG == Vconfig.mode)
					{
						pd->vlanconf.vmaggportmask |= 1<<port;
						pd->vlanconf.vmtrunkportmask &= ~(1<<port);
					}
					else if(ONU_CONF_VLAN_MODE_TRANSPARENT == Vconfig.mode)/*added by luh 2013-12-04*/
					{
						pd->portconf[port].vlanTransparentEnable = 1;
					}
				}
			}
		}

#if 1
		{

			int port = 0, j =0, k = 0, m =0;
			int num = 0;
			int flag = 0;
			for(port=0;port<maxPortNum;port++)
			{
				CTC_STACK_classification_rules_t cam;
				VOS_MemZero(&cam[0], sizeof(CTC_STACK_classification_rules_t));
				if(VOS_OK == OnuMgt_GetEthPortClassificationAndMarking(pon_id, onu_id - 1, port+1, cam))
				{
					num = sizeof(cam)/sizeof(CTC_STACK_classification_rule_t);
					flag = 0;
					for(j=0;j<QOS_MAX_SET_COUNT;j++)
					{
						qos_classification_rule_t *prule = (qos_classification_rule_t*)&pd->qosset[j];            
						for(k=0;k<QOS_MAX_RULE_COUNT_PER_SET;k++)
						{                        
							if(prule->priority_mark == (cam[k+1].priority_mark) &&
									prule->queue_mapped == (cam[k+1].queue_mapped) &&
									prule->num_of_entries == cam[k+1].num_of_entries &&
									prule->valid == cam[k+1].valid && cam[k+1].valid)
							{  
								for(m=0;m<cam[k+1].num_of_entries;m++)
								{
									if(prule->entries[m].field_select != cam[k+1].entries[m].field_select ||
											prule->entries[m].validation_operator != cam[k+1].entries[m].validation_operator)
									{
										break;
									}
									else
									{
										int flag_temp = 0;
										switch(cam[k+1].entries[m].field_select)
										{
											case FIELD_SEL_DA_MAC:
											case FIELD_SEL_SA_MAC:
												if(VOS_MemCmp(&prule->entries[m].value.mac_address, &cam[k+1].entries[m].value.mac_address, sizeof(mac_address_t))!=0)
													flag_temp = 1;   
												break;
											case FIELD_SEL_DEST_IPV6:
											case FIELD_SEL_SRC_IPV6:
												if(VOS_MemCmp(&prule->entries[m].value.ipv6_match_value, &cam[k+1].entries[m].value.ipv6_match_value, sizeof(PON_ipv6_addr_t))!=0)
													flag_temp = 1;
												break;
											default:
												if(prule->entries[m].value.match_value != cam[k+1].entries[m].value.match_value)
													flag_temp = 1;   
												break;
										}
										if(flag_temp)
											break;
										else 
											continue;
									}
								}
								if(m >= cam[k+1].num_of_entries)
								{
									pd->portconf[port].qosSetSel = j+1;
									flag = 1;
									break;
								}
							}
							prule++;

						}
					}
					if(flag == 0)
					{
						for(i=0;i<4;i++)
						{
							if(!pd->qosset[i][0].valid)
							{
								for(j=0;j<QOS_MAX_RULE_COUNT_PER_SET;j++)
								{
									if(cam[j+1].valid)
									{
										pd->portconf[port].qosSetSel = i+1;
										pd->qosset[i][j].priority_mark = cam[j+1].priority_mark;
										pd->qosset[i][j].queue_mapped = cam[j+1].queue_mapped;
										pd->qosset[i][j].num_of_entries = cam[j+1].num_of_entries;
										pd->qosset[i][j].valid = cam[j+1].valid;
										for(m=0;m<cam[j+1].num_of_entries;m++)
										{
											pd->qosset[i][j].entries[m].field_select = cam[j+1].entries[m].field_select;
											pd->qosset[i][j].entries[m].validation_operator = cam[j+1].entries[m].validation_operator;
											switch(cam[j+1].entries[m].field_select)
											{
												case FIELD_SEL_DA_MAC:
												case FIELD_SEL_SA_MAC:
													VOS_MemCpy(&pd->qosset[i][j].entries[m].value.mac_address, &cam[j+1].entries[m].value.mac_address, sizeof(mac_address_t));
													break;
												case FIELD_SEL_DEST_IPV6:
												case FIELD_SEL_SRC_IPV6:
													VOS_MemCpy(&pd->qosset[i][j].entries[m].value.ipv6_match_value, &cam[j+1].entries[m].value.ipv6_match_value, sizeof(PON_ipv6_addr_t));
													break;
												default:
													pd->qosset[i][j].entries[m].value.match_value = cam[j+1].entries[m].value.match_value;
													break;
											}
											pd->qosset[i][j].entrymask |= 1<<m;
										}
									}
								}
								break;
							}
						}
					}
				}
			}
		}
#endif

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
		ret = cmc_show_running_config_all(pon_id, onu_id, &pd->cmcCfg);
#endif

	}
    if(error_flag)
        ret = VOS_ERROR;
	
    return ret;
}

static char * generateOnuConfClMemFileForRestore(short int pon_id, short int onu_id, const char *filename, unsigned char fileflags, int act, struct vty *vty, char **pfile)
{
    int ret = 0;

    char *name = NULL;

    short int slot = GetCardIdxByPonChip(pon_id);
    short int pon = GetPonPortByPonChip(pon_id);
    if(pfile == NULL || filename == NULL || vty == NULL )
    	return name;

#ifndef ONUID_MAP
    char * name = getOnuConfMacFromHashBucket(mac);


    /*根据MAC找不到，根据ONU位置找*/
    /*添加 isOnuConfExist，解决由于删除onu原先的配置文件后，不能正确使用新预配置的配置文件的问题  wangxy 2011-06-30 */
    if(!(name && isOnuConfExist(name)))
    {
        name = ONU_CONF_NAME_PTR_GET(pon_id, onu_id);
        if(!name[0])
        {
            name = DEFAULT_ONU_CONF;
        }
    }
#else
	name = filename;
#endif

    vty_out(vty, "onu %d/%d/%d\r\n", slot, pon, onu_id+1);

    if(act == ONU_CONF_RES_ACTION_DO)
        ret = generateOnuConfClMemFile(name, fileflags, vty, NULL,slot,pon,onu_id+1);
    else
        ret = generateOnuConfUndoClMemFile(name, fileflags, vty, NULL);

    vty_out(vty, "exit\r\n");

    if(ret)
    {
    	*pfile = onuconf_write_showrun_to_buffer(vty);
    }

    return(ret)?name:NULL;
}

extern ULONG cdsms_file_conf_maxlen;

int getBitMaskString(const char *buf, int buflen, const int bwidth, char *string, int strlen)
{
    int ret = 0;

    if(buf == NULL || string == NULL || buflen*8<bwidth )
        return ret;
    else
    {
        int bp=0, ep=0, i, len=0, total=0;

        for(i=0; i<bwidth; i++)
        {
            int offset = i/8;
            int bitnum = i%8;

            if(buf[offset] & (1<<bitnum))
            {
                if(!(bp*ep))
                {
                    bp = i+1;
                    ep = i+1;
                }
                else
                {
                    ep = i+1;
                }
            }
            else if(bp*ep)
            {
                char sz[80] = "";
                if(bp == ep)
                    len = VOS_Sprintf(sz, "%d,", bp);
                else
                    len = VOS_Sprintf(sz, "%d-%d,", bp, ep);

                if(total+len > strlen-1)
                    break;
                else
                {
                    VOS_StrCat(string, sz);
                    total += len;
                }

                bp = 0;
                ep = 0;
            }
        }

        if(bp*ep)
        {
            char sz[80] = "";
            if(bp == ep)
                len = VOS_Sprintf(sz, "%d,", bp);
            else
                len = VOS_Sprintf(sz, "%d-%d,", bp, ep);

            if(total+len > strlen-1)
                total = 0;
            else
            {
                VOS_StrCat(string, sz);
                total += len;
            }

            bp = 0;
            ep = 0;
        }

        if(i==bwidth && total)
        {
            ret = total-1;
            string[ret] = 0;
        }
    }

    return ret;

}

#if 0
static int getConfFileOnuListByName( const char *name, char *string, int slen )
{
    int ret = VOS_OK;

    char sz[512] = "";

    ONUConfigData_t *pd = getOnuConfFromHashBucket(name);

    if(pd)
    {
        int ponid, glen = 0;
        VOS_MemZero(string, slen);

        for(ponid=0; ponid<SYS_MAX_PON_PORTNUM;ponid++)
        {
            int bp=0, ep=0, i, len=0;

            for(i=0; i < MAXONUPERPON; i++)
            {
                int offset = i/8;
                int bitnum = i%8;

                if(pd->onulist[ponid][offset] & (1<<bitnum))
                {
                    if(!(bp*ep))
                    {
                        bp = i+1;
                        ep = i+1;
                    }
                    else
                    {
                        ep = i+1;
                    }
                }
                else if(bp*ep)
                {
                    if(bp == ep)
                        len += VOS_Sprintf(sz+len, "%d,", bp);
                    else
                    {
                        len += VOS_Sprintf(sz+len, "%d-%d,", bp , ep);
                    }

                    bp = 0;
                    ep = 0;
                }
            }

			if(bp*ep)
			{
                if(bp == ep)
                    len += VOS_Sprintf(sz+len, "%d,", bp);
                else
                {
                    len += VOS_Sprintf(sz+len, "%d-%d,", bp , ep);
                }				
			}

            if(len > 0)
            {
                sz[len-1]=0;

                if(slen < len+1)
                    ret = VOS_ERROR;
                else
                {
                    int slot = GetGlobalCardIdxByPonChip(ponid);
                    int pon = GetGlobalPonPortByPonChip(ponid);

                    glen += VOS_Sprintf(string+glen, "onu-profile associate %d/%d %s %s\r\n", slot, pon, sz, name);
                }
            }
            else
                ret = VOS_ERROR;

        }
    }

    return ret;
}
#else

static int getConfFileOnuListByName(struct vty * vty, const char *name )
{
    int ret = VOS_OK;

    char sz[512] = "";

    ONUConfigData_t *pd = getOnuConfFromHashBucket(name);

    if(pd)
    {
        int ponid, glen = 0;

        for(ponid=0; ponid<SYS_MAX_PON_PORTNUM;ponid++)
        {
            int bp=0, ep=0, i, len=0;
            VOS_MemZero(sz,  512);

            for(i=0; i < MAXONUPERPON; i++)
            {
                int offset = i/8;
                int bitnum = i%8;

                if(pd->onulist[ponid][offset] & (1<<bitnum))
                {
                    if(!(bp*ep))
                    {
                        bp = i+1;
                        ep = i+1;
                    }
                    else
                    {
                        ep = i+1;
                    }
                }
                else if(bp*ep)
                {
                    if(bp == ep)
                        len += VOS_Sprintf(sz+len, "%d,", bp);
                    else
                    {
                        len += VOS_Sprintf(sz+len, "%d-%d,", bp , ep);
                    }

                    bp = 0;
                    ep = 0;
                }
            }

			if(bp*ep)
			{
                if(bp == ep)
                    len += VOS_Sprintf(sz+len, "%d,", bp);
                else
                {
                    len += VOS_Sprintf(sz+len, "%d-%d,", bp , ep);
                }				
			}

            if(len > 0 && len <= 512)
            {

                int slot = GetGlobalCardIdxByPonChip(ponid);
                int pon = GetGlobalPonPortByPonChip(ponid);
                sz[len-1]=0;

                    /*glen += VOS_Sprintf(string+glen, "onu-profile associate %d/%d %s %s\r\n", slot, pon, sz, name);*/
                vty_out(vty, "onu-profile associate %d/%d %s %s\r\n", slot, pon, sz, name);
            }
            else
                ret = VOS_ERROR;

        }
    }

    return ret;
}

#endif

int eraseOnuConfFromFlash(void)
{
    int ret = VOS_OK;

    if (cdsms_file_onu_conf_erase)
    {
        if ((*cdsms_file_onu_conf_erase)(ONU_CONF_SAVE_FILE_HEAD))
            ret = VOS_ERROR;
    }
    else
        ret = VOS_ERROR;

    if(ret == VOS_OK)
    {
        /*added by luh 2012-10-24 解决问题，清除flash中的profile中，再次执行save不能保存*/        
        onu_conf_flash_empty = 1;
        if( SYS_LOCAL_MODULE_ISMASTERACTIVE && (device_standby_master_slotno_get() != 0) )
            config_sync_notify_event();
    }

    return ret;
}

int onuConfShowStartupByVty(struct vty *vty)
{
    int ret = CMD_SUCCESS;

    char *file = NULL;
    char *flashfile = NULL;

    char szName[80] = "";

    long len = ONU_MAX_DATABUF_SIZE, flashfilelen = 0;

    VOS_Sprintf(szName, "%s", ONU_CONF_SAVE_FILE_HEAD);

    flashfilelen = xflash_filesize_get_by_name(szName);

    if (flashfilelen > 0)
    {
        flashfile = (char*)g_malloc(flashfilelen);
        file = (char*)g_malloc(len);

        if (file && flashfile)
        {
			LONG fraglen = 0, bfraglen = 0, offset = 0, length = 0;
            VOS_MemZero(flashfile, flashfilelen);
            VOS_MemZero(file, len);
            if (cdsms_file_onu_conf_read)
            {
#if(PRODUCT_CLASS == EPON3 )
                V2R1_disable_watchdog();
#endif
                (*cdsms_file_onu_conf_read)(szName, flashfile, &flashfilelen);
#if(PRODUCT_CLASS == EPON3 )
                V2R1_enable_watchdog();
#endif

                if(checkCompressConfigFormat(flashfile, flashfilelen) == VOS_OK)
                {
                    do{
                        fraglen = *(LONG*)(flashfile+offset);
                        bfraglen = fraglen;

                        if(fraglen > 0 && fraglen != ONU_CONF_FILE_END_FLAG && fraglen+offset+4 <= flashfilelen)
                        {
                            if(defLzmaUncompress(file, &len, flashfile+offset+4, &fraglen) == SZ_OK)
                            {
                                char buf[BUFSIZ] = "";
                                char *config_buf = file;
                                length += len;
                                while (cl_read_memfile_to_buf(buf, (char**) &config_buf, BUFSIZ))
                                    vty_out(vty, "%s", buf);
                            }

                            VOS_MemZero(file, ONU_MAX_DATABUF_SIZE);
                            offset += bfraglen+4;
                        }
                        else
                        {
                            ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("\r\nonuConfShowStartupByVty: fraglen is %d, flashfilelen is %d\r\n", fraglen, flashfilelen));
                            break;
                        }
                    }while(bfraglen != ONU_CONF_FILE_END_FLAG && offset < flashfilelen);

                    if(length > 0)
                    {
                        vty_out(vty, "\r\n");
                        vty_out(vty, "! *************************\r\n");
                        vty_out(vty, "! Total usage %d bytes \r\n", length);
                        vty_out(vty, "! Maximum     %d bytes \r\n", ONU_MAX_CONF_SIZE);
                        vty_out(vty, "! *************************\r\n");
                    }
                    else
                    {
                        vty_out(vty, "config file is empty!\r\n");
                        ret = CMD_WARNING;
                    }

                }
                else
                {
                    char buf[BUFSIZ] = "";
                    char *config_buf = flashfile;
                    while (cl_read_memfile_to_buf(buf, (char**) &config_buf, BUFSIZ))
                        vty_out(vty, "%s", buf);
                }

            }

            g_free(file);
            g_free(flashfile);
        }
        else
        {
            if (file)
                g_free(file);
            if (flashfile)
                g_free(flashfile);
        }
    }
    else
    {
        vty_out(vty, "config file is empty!\r\n");
        ret = CMD_WARNING;
    }

    return ret;
}

#ifdef ONUID_MAP

static int output_onuconf_for_saving(struct vty *file_vty, ONUConfigData_t *pd, char *file, ULONG *pflen)
{

	LONG length = 0;
	int ret = VOS_ERROR;

	if(pd == NULL || file_vty == NULL || file == NULL || pflen == NULL)
		return ret;

	
    /*ONUConfigData_t *pd = (ONUConfigData_t *)(pe->confdata);*/
 
 
    if (VOS_StrCmp(pd->confname, DEFAULT_ONU_CONF) && VOS_StrCmp(
        pd->confname, DEFAULT_GWD_ONU_CONF) && (!VOS_StrStr(pd->confname, "*auto")))
    {
        /*comment by wangxiaoyu 2011-10-26 for problem 13726*/
        /*if (pd->share)*/
        vty_out(file_vty, "config onu-profile %s\r\n", pd->confname);
        /*
        else
            vty_out(file_vty, "onu-profile %s private\r\n", pd->confname);*/
 
        ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("saving onu conf file %s ... ...\r\n", pd->confname));
 
        if (generateOnuConfClMemFile(pd->confname, -1, file_vty, NULL,0,0,0))
        {
            /*char sz[512] = "";*/
            vty_out(file_vty, "exit\r\n");
 
            /*getConfFileOnuListByName(pd->confname, sz, 512);*/
            getConfFileOnuListByName(file_vty, pd->confname);
 
            /*vty_out(file_vty, "%s\r\n", sz);*/
 
            cl_flush_vty_obuf_to_memfile(file_vty->obuf, file+length);
 
            length += file_vty->obuf->length;
 
            vty_buffer_reset(file_vty);

			*pflen = length;
			ret = VOS_OK;
 
        }
 
    }

	return ret;
}


static int saveOnuConfFragment(char *name, char *frag, ULONG fraglen, char *oldfile, ULONG df_offset, int* wflag)
{
	int ret = VOS_ERROR;

	int len = 4;

	/*如果之前已经判断出需要写入新的配置数据，直接在文件末尾添加*/
	if(*wflag == FILE_WRITE_APPEND)
	{
		ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("save onuconf fragement for length, offset = %d, wflag = %s\r\n", df_offset, *wflag?"FILE_WRITE_APPEND":"FILE_WRITE_NOAPPEND"));
		xflash_file_write2(name, &fraglen, &len, *wflag);
		ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("save onuconf fragement for context, offset = %d, wflag = %s\r\n", df_offset+4, *wflag?"FILE_WRITE_APPEND":"FILE_WRITE_NOAPPEND"));
		xflash_file_write2(name, frag, &fraglen, *wflag);
		ret = VOS_OK;
	}
	else
	{
		if(!oldfile ||
			(oldfile && ((*(ULONG*)(oldfile+df_offset) != fraglen) ||
			VOS_MemCmp(oldfile+df_offset+4, frag, fraglen))))
		{

			/*先清除原来的内容，并写入之前检查结果为没有改变的内容*/
			if(oldfile && df_offset)
			{
				ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("save onuconf fragement for unchanged context, length = %d, wflag = %s\r\n", df_offset, *wflag?"FILE_WRITE_APPEND":"FILE_WRITE_NOAPPEND"));
				xflash_file_write2(name, oldfile, &df_offset, *wflag);
				*wflag = FILE_WRITE_APPEND;
			}

			/*写入本部分分片内容*/
			ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("save onuconf fragement for length, offset = %d, wflag = %s\r\n", df_offset, *wflag?"FILE_WRITE_APPEND":"FILE_WRITE_NOAPPEND"));
			xflash_file_write2(name, &fraglen, &len, *wflag);
			*wflag = FILE_WRITE_APPEND;
			ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("save onuconf fragement for context, offset = %d, wflag = %s\r\n", df_offset+4, *wflag?"FILE_WRITE_APPEND":"FILE_WRITE_NOAPPEND"));
			xflash_file_write2(name, frag, &fraglen, *wflag);
			ret = VOS_OK;
		}
	}

	return ret;
}

int saveOnuConfToFlash(void)
{
    int ret = VOS_OK;

    char szName[80] = "";

    ULONG length = 0;

    char *szFile = NULL, *oldfile = NULL, *file = NULL;
    ULONG filelen = 0, df_offset = 0;
	LONG oldlen = 0;

	int wflag = FILE_WRITE_NOAPPEND;

	const int max_file_num = ONU_MAX_DATABUF_SIZE/ONU_MAX_SIZE_FILE;
	int count = 0, all_count = 0;

	if(onuconf_GetSaveFlag() != VOS_OK && !onu_conf_flash_empty)/*如果flash不为空，且无更改则不再执行保存*/
	    return VOS_ERROR;
    VOS_StrCpy(szName, ONU_CONF_SAVE_FILE_HEAD);

	oldlen = xflash_filesize_get_by_name(szName);
	if(oldlen > 0)
	{
		oldfile = (char*)g_malloc(oldlen);
		if(oldfile)
			xflash_file_read2(szName, oldfile, &oldlen);
		else
			return VOS_ERROR;
	}

    if ((file = (char*)g_malloc(ONU_MAX_DATABUF_SIZE)))
    {
        struct vty * file_vty;

        file_vty = vty_new();
        if (!file_vty)
        {
            goto SAVE_FAIL;
        }
        file_vty->fd = _CL_FLASH_FD_;
        file_vty->type = VTY_FILE;

		VOS_MemZero(file, ONU_MAX_DATABUF_SIZE);

        vty_out(file_vty, "!GROS system config file%s", "\r\n");

        ONU_CONF_SEM_TAKE
        {
            int i;

            for(i=0; i<g_maxPrimeNumber; i++)
            {
                element_onuconfhashbucket_t *pe = (element_onuconfhashbucket_t*) g_onuConfBucket[i];

                while(pe)
                {
					ULONG flen = 0;
					if(output_onuconf_for_saving(file_vty, pe->confdata, file+length, &flen) == VOS_OK)
					{
					    length+= flen;
					    count++;
					    all_count++;

                        if(count == max_file_num)
                        {
                            if(defLzmaCompress(&szFile, &filelen, file, length, lzma_malloc, lzma_free, 0) == SZ_OK)
                            {
                            	if( szFile == NULL )	/* aaaaaa modified by xieshl 20141125 申请不到内存时也会返还OK */
                            	{
                            		VOS_ASSERT(0);
                            	}
								else
								{
	                                saveOnuConfFragment(szName, szFile, filelen, oldfile, df_offset, &wflag);
	                                df_offset += filelen+4;
	                                if (szFile) g_free(szFile); 
	                                szFile = NULL;
								}
                            }

                            count = 0;
                            length = 0;
                            VOS_MemZero(file, ONU_MAX_DATABUF_SIZE);
                        }
					}
                    pe = pe->next;
                }

            }
            ONU_CONF_SEM_GIVE
        }

		if(count )
		{
        	if(defLzmaCompress(&szFile, &filelen, file, length, lzma_malloc, lzma_free, 0) == SZ_OK)
        	{
            	if( szFile == NULL )	/* aaaaaa modified by xieshl 20141125 申请不到内存时也会返还OK */
            	{
            		VOS_ASSERT(0);
            	}
				else
				{
					saveOnuConfFragment(szName, szFile, filelen, oldfile, df_offset, &wflag);
					df_offset += filelen+4;
                                 if (szFile) g_free(szFile); 
					szFile = NULL;
				}
        	}
		}

		/*写入结尾标识*/
		if(wflag == FILE_WRITE_APPEND || (wflag == FILE_WRITE_NOAPPEND && all_count == 0)) /*添加判断防止配置数据无变化，之前没有写入操作时，以结束符覆盖原有数据*/
		{
            filelen = ONU_CONF_FILE_END_FLAG;
            length = 4;
            xflash_file_write2(szName, &filelen, &length, wflag);
		}

SAVE_FAIL:

        if(szFile)
            g_free(szFile);

        if(file_vty)
            vty_free(file_vty);

        g_free(file);

    }

	if(oldfile)
		g_free(oldfile);

    if( SYS_LOCAL_MODULE_ISMASTERACTIVE && (device_standby_master_slotno_get() != 0) )
        config_sync_notify_event();

    if(onu_conf_flash_empty)/*执行save操作时，需要把save标志清空，该标志只用于配置文件的保存*/
        onu_conf_flash_empty = 0;
    return ret;
}

static int checkCompressConfigFormat(const char *buf, const int length)
{
    int ret = VOS_ERROR;

    ULONG offset = 0, fraglen = 0;
    while(offset+4 <= length)
    {
        fraglen = *(ULONG*)(buf+offset);

        if(fraglen != ONU_CONF_FILE_END_FLAG)
        {
            offset += fraglen+4;
        }
        else
            offset+=4;
    }

    if(offset == length)
        ret = VOS_OK;

    return ret;
}

int loadOnuConfFile(void)
{
    int ret = VOS_ERROR;

    char *file = NULL;
    char *file_uncompress = NULL;
    LONG len = 0;
    ULONG file_uncompresslen = ONU_MAX_DATABUF_SIZE;

    char szName[80] = "";

    VOS_Sprintf(szName, "%s", ONU_CONF_SAVE_FILE_HEAD);

    len = xflash_filesize_get_by_name(szName);

    if (len > 0)
    {

		LONG fraglen = 0, offset = 0, bfraglen = 0;
		
        file = (char*)g_malloc(len);
        file_uncompress = (char*)g_malloc(file_uncompresslen);
        if (file && file_uncompress)
        {
            VOS_MemZero(file, len);
            VOS_MemZero(file_uncompress, file_uncompresslen);
            if (cdsms_file_onu_conf_read)
            {
#if(PRODUCT_CLASS == EPON3 )
                V2R1_disable_watchdog();
#endif
                (*cdsms_file_onu_conf_read)(szName, file, &len);
#if(PRODUCT_CLASS == EPON3 )
                V2R1_enable_watchdog();
#endif

                if(checkCompressConfigFormat(file, len) == VOS_OK)
                {
                    do{
                        fraglen = *(LONG*)(file+offset);
                        bfraglen = fraglen;

                        if(fraglen > 0 && fraglen != ONU_CONF_FILE_END_FLAG && fraglen+offset+4 <= len)
                        {
                        	file_uncompresslen = ONU_MAX_DATABUF_SIZE;
                        	VOS_MemZero(file_uncompress, file_uncompresslen);
                            if(defLzmaUncompress(file_uncompress, &file_uncompresslen, file+offset+4, &fraglen) == SZ_OK)
                            {
                                if (cl_run_memfile_config_quietly(file_uncompress) == 1)
                                {
                                    ret = VOS_OK;
                                }
                            }

                            offset += bfraglen+4;
                        }
                        else
                        {
                            /*VOS_ASSERT(0);*/
                            break;
                        }
                    }while(bfraglen != ONU_CONF_FILE_END_FLAG && offset < len);
                }
                else
                {
                    if(cl_run_memfile_config_quietly(file) == 1)
                        ret = VOS_OK;
                }
            }

            g_free(file);
            g_free(file_uncompress);

        }
        else
        {
            if(file)
                g_free(file);
            if(file_uncompress)
                g_free(file_uncompress);
        }

    }

    return ret;
}

#endif

static int compileAndExecuteConfFile(onu_conf_res_queue_e *p)
{

    /*
     * MTODO: 编译并执行配置恢复命令行
     * 1、定位配置文件
     * 2、将配置文件生成可执行命令buffer
     * 3、执行可命令行buffer
     * 4、p->trynum++
     * 4、返回结果
    */
    char *mfile =NULL;

    int ret = 0;
    int lRlt = 0;
    /*setOnuConfRestoreFlag(p->pon_id, p->onu_id, 1);*/

	BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_GENERAL)
	int slot = GetCardIdxByPonChip(p->pon_id);
	int port = GetPonPortByPonChip(p->pon_id);
    ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("\r\nonu %d/%d/%d restore %s: %s\r\n", slot, port, p->onu_id+1, p->action == ONU_CONF_RES_ACTION_DO?"do":"undo", p->filename))
    END_ONUCONF_DEBUG_PRINT()

    /*mfile = cl_config_mem_file_init();*/

    /*if(mfile)*/
    {
        struct vty * file_vty, *run_vty;
        short int slot, pon;
        char *name = NULL;

        file_vty = vty_new ();
        run_vty = vty_new();
        if ( !file_vty || !run_vty)
        {
            goto FAIL;
        }
        file_vty->fd = _CL_MEMFILE_FD_;
        file_vty->type = VTY_FILE;

        run_vty->fd = 0;
        run_vty->type = VTY_TERM;
        run_vty->node = CONFIG_NODE;

        name = generateOnuConfClMemFileForRestore( p->pon_id, p->onu_id, p->filename, p->flags, p->action, file_vty, &mfile );

        if(name && mfile)
        {
            if(p->action == ONU_CONF_RES_ACTION_UNDO)
                setOnuConfUndoFlag(p->pon_id,p->onu_id, 1);
            /*if(cl_run_memfile_config_quietly(mfile) == 1)*/
            if(cl_run_memfile_config_vty(mfile, run_vty) == 0)
            {

                ret = 1;
            }
            else
            {
                ret = 0;                
            }
            /****** 只有执行DO操作的时候进行 批处理数据恢复2011-12-25 by luh***/
            if((p->action == ONU_CONF_RES_ACTION_DO) && (GetOnuOperStatus(p->pon_id,  p->onu_id) == ONU_OPER_STATUS_UP))
            {
                
                slot = GetCardIdxByPonChip(p->pon_id);
                pon = GetPonPortByPonChip(p->pon_id);
                lRlt = generateOnuConfClMemFile_Bat(file_vty, p->filename, slot, pon, p->onu_id+1);
            }
            /*******************************/
            if(ret == 1 && lRlt == VOS_OK)
            {
                if(p->action == ONU_CONF_RES_ACTION_UNDO)
                    setOnuConfResult(p->pon_id, p->onu_id, ONU_CONF_EXEC_RESULT_NULL);
                else
                    setOnuConfResult(p->pon_id, p->onu_id, ONU_CONF_EXEC_RESULT_OK);

                BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)


                char sz[128];
                int len = 0;
                char *text = NULL;

                len = VOS_Sprintf(sz, "\r\n%s restore %d/%d/%d with file %s OK!\r\n", (p->action == ONU_CONF_RES_ACTION_UNDO)?"undo":"do",
                        GetCardIdxByPonChip(p->pon_id), GetPonPortByPonChip(p->pon_id), p->onu_id+1, name);
                sendOnuConfDebugReportMsg(SYS_MASTER_ACTIVE_SLOTNO,SYS_LOCAL_MODULE_SLOTNO, len+1, sz);

                len = VOS_StrLen(mfile);
                text = VOS_Malloc(len+1, MODULE_RPU_CTC);
                if(text)
                {
                    VOS_MemSet(text, 0, len+1);
                    VOS_StrCpy(text, mfile);

                    sendOnuConfDebugReportMsg(SYS_MASTER_ACTIVE_SLOTNO, SYS_LOCAL_MODULE_SLOTNO, len+1, text);
                    VOS_Free(text);
                }

                END_ONUCONF_DEBUG_PRINT()
            }
            else
            {
                if(p->action == ONU_CONF_RES_ACTION_UNDO)
                    setOnuConfResult(p->pon_id, p->onu_id, ONU_CONF_EXEC_RESULT_NULL);
                else                
                    setOnuConfResult(p->pon_id, p->onu_id, ONU_CONF_EXEC_RESULT_ERR);
                
                BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)

                char sz[128]="";
                int len = VOS_Sprintf(sz, "\r\n%s restore %d/%d/%d with file %s ERR!\r\n", (p->action == ONU_CONF_RES_ACTION_UNDO)?"undo":"do",
                        GetCardIdxByPonChip(p->pon_id), GetPonPortByPonChip(p->pon_id), p->onu_id+1, name);
                sendOnuConfDebugReportMsg(SYS_MASTER_ACTIVE_SLOTNO,SYS_LOCAL_MODULE_SLOTNO, len+1, sz);
                ret = 0;

                END_ONUCONF_DEBUG_PRINT()
            }
            
            if(p->action == ONU_CONF_RES_ACTION_UNDO)
                setOnuConfUndoFlag(p->pon_id,p->onu_id, 0);
        }
        if(mfile)
        	VOS_Free(mfile);

        vty_free ( file_vty );
        file_vty = NULL;
        vty_free(run_vty);
        run_vty = NULL;
FAIL:

    if(file_vty)
        vty_free(file_vty);

    if(run_vty)
        vty_free(run_vty);
    }
    /*del by luh 2014-03-28*/
    /*setOnuConfRestoreFlag(p->pon_id, p->onu_id, 0);*/

    p->trynum++;

    return ret;

}

static int lock_onuRestoreCtrlItemList(short int ponid)
{
	if( ponid < 0 || ponid >= MAXPON )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
	if( s_semOnuResConf[ponid] == 0 )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
    return VOS_SemTake(s_semOnuResConf[ponid], WAIT_FOREVER);
}

static int unlock_onuRestoreCtrlItemList(short int ponid)
{
	if( ponid < 0 || ponid >= MAXPON )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
	if( s_semOnuResConf[ponid] == 0 )
		{
			VOS_ASSERT(0);
			return VOS_ERROR;
		}
    return VOS_SemGive(s_semOnuResConf[ponid]);
}

extern int CTCONU_RecoverAlarmData(short int olt_id, short int onu_id);

static int checkOnuRestoreEntry(onu_conf_res_queue_e * entry)
{
	int ret = VOS_ERROR;

	if(entry)
	{
		if(entry->pon_id >= 0 && entry->pon_id < MAXPON &&
				entry->onu_id >= 0 && entry->onu_id < MAXONUPERPON &&
				entry->action >= ONU_CONF_RES_ACTION_UNDO &&
				entry->action <= ONU_CONF_RES_ACTION_NOOP)
			ret = VOS_OK;
		else
			ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL,("task_onuResConf:  onu restore entry invalid! ponid %d, onuid %d, action %d\r\n", entry->pon_id, entry->onu_id, entry->action));
	}
	else
	{
		ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL,("task_onuResConf:  onu restore entry invalid! entry NULL\r\n"));
	}

	return ret;
}


void printOnuRestoreQueue(short int pon_id)
{
	onu_conf_res_queue_list_t *pe = s_waitRestoreOnuQueue[pon_id];

	while(pe && pe->e)
	{
		sys_console_printf("element : pon_id: %d, onu_id %d, act:%s, file:%s\r\n", pe->e->pon_id, pe->e->onu_id, pe->e->action == 0 ? "undo": "do", pe->e->filename);
		pe = pe->next;
	}
}

DECLARE_VOS_TASK( task_OnuResConf )
{
    ULONG ponid = ulArg1;
    int OnuOperstatus = 0;
	int delflag = 0;
    onu_conf_res_queue_e e_res[2];

    while(1)
    {
        int i = 0;

        VOS_MemZero(e_res, sizeof(e_res));
        e_res[0].action = ONU_CONF_RES_ACTION_NOOP;
        e_res[1].action = ONU_CONF_RES_ACTION_NOOP;

        lock_onuRestoreCtrlItemList(ponid);

        /*只找到第一个等待的ONU恢复*/
        {
            onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[ponid];
            /*ONU必须是undo+do或单一的do操作才能执行，不允许单独的undo操作*/
            while(p)
            {
            	if(checkOnuRestoreEntry(p->e) != VOS_OK)
            	{
            		onu_conf_res_queue_list_t * p1 = p->next;
            		delElementFromRestoreQueue(ponid, p);
            		p = p1;
            		continue;
            	}
                if(p->next && checkOnuRestoreEntry(p->next->e) == VOS_OK && p->next->e->onu_id == p->e->onu_id)
                {
                    VOS_MemCpy(&e_res[0], p->e, sizeof(onu_conf_res_queue_e));
                    VOS_MemCpy(&e_res[1], p->next->e, sizeof(onu_conf_res_queue_e));
                    delElementFromRestoreQueue(ponid, p->next);
                    delElementFromRestoreQueue(ponid, p);
                    break;
                }
                else if(p->e->action == ONU_CONF_RES_ACTION_DO)
                {
                    VOS_MemCpy(&e_res[0], p->e, sizeof(onu_conf_res_queue_e));
                    delElementFromRestoreQueue(ponid, p);
                    break;
                }
                else
                	p = p->next;
            }

        }
        unlock_onuRestoreCtrlItemList(ponid);
        /*added by luh 2012-11-6 ;Onu 恢复配制时可能会离线，如果离线则不处理
                后续过程，直接清恢复队列*/
        OnuOperstatus = GetOnuOperStatus(e_res[0].pon_id, e_res[0].onu_id);
		/*只有do操作，认为是ONU上线，恢复告警配置，其它情况不恢复*/
		if(e_res[0].action == ONU_CONF_RES_ACTION_DO && ONU_OPER_STATUS_UP == OnuOperstatus)
			CTCONU_RecoverAlarmData (e_res[0].pon_id, e_res[0].onu_id);

        for(i=0; i<2; i++)
        {
            if(e_res[i].action != ONU_CONF_RES_ACTION_NOOP)
            {
                int rcode = 0;
                if(ONU_OPER_STATUS_UP == OnuOperstatus)
                {
                    setOnuConfRestoreFlag(e_res[i].pon_id, e_res[i].onu_id, 1);                    
                    rcode = compileAndExecuteConfFile(&e_res[i]);
                    rcode = !rcode?ONU_CONF_EXEC_RESULT_ERR:ONU_CONF_EXEC_RESULT_OK;
                    /*执行undo操作时，该onu的恢复状态应该清空，2014-04-09*/
                    if(e_res[i].action == ONU_CONF_RES_ACTION_UNDO)
                        sendOnuConfReportMsg(GetCardIdxByPonChip(e_res[i].pon_id), GetPonPortByPonChip(e_res[i].pon_id), e_res[i].onu_id+1, e_res[i].filename, ONU_CONF_EXEC_RESULT_NULL);
                    else
                        sendOnuConfReportMsg(GetCardIdxByPonChip(e_res[i].pon_id), GetPonPortByPonChip(e_res[i].pon_id), e_res[i].onu_id+1, e_res[i].filename, rcode);
                    setOnuConfRestoreFlag(e_res[i].pon_id, e_res[i].onu_id, 0);                    
                }
                if(VOS_StrStr(e_res[i].filename, "*auto"))
                {
                    int slot = GetCardIdxByPonChip(ponid);
                    int port = GetPonPortByPonChip(ponid);
                    int ponportidx = GetGlobalPonPortIdxBySlot(slot, port);
					ponportidx = onuConfGetPonIdFromPonProtectGroup(ponportidx);
                    int send_flag = 0;
                    if(ponportidx != -1)
                    {
                   
                        lock_onuRestoreCtrlItemList(ponid);
                        {
                            onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[ponid];

                            while(p)
                            {
                                if(!VOS_StrCmp(p->e->filename, e_res[i].filename))
                                {
									if(p->e->pon_id == e_res[i].pon_id && p->e->onu_id ==  e_res[i].onu_id)
									{
										delflag = TRUE;
									}
                                    send_flag = 1;
									break;
								}
                                    
                                p = p->next;
                            }

                        }
                        unlock_onuRestoreCtrlItemList(ponid);

                        /*modi by luh,2012-11-12；保留，为了保证跟配置文件关联的onu全部回复后，才能删除*/
#if 1                        
                        ONU_CONF_SEM_TAKE;
                        {
                            if(!delflag)
                            {
                                sfun_clrOnuConfNameMapByPonId(e_res[i].filename, ponportidx, e_res[i].onu_id);
                                delflag = FALSE;
                            }
                        }
                        ONU_CONF_SEM_GIVE;

                        if(!send_flag)
                        {
                            /*发送PON端口编辑临时文件恢复结束*/
                            int queid;
                            ULONG args[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_RENAME_CHECK_CDP, 0, 0};
                            char *name = VOS_Malloc(ONU_CONFIG_NAME_LEN, MODULE_RPU_CTC);
                            if(name)
                            {
                                    VOS_MemZero(name, ONU_CONFIG_NAME_LEN);
                                    VOS_StrCpy(name, e_res[i].filename);
                                    args[3] = (int)name;

                                    queid = getQueIdBySlotno(slot);

                                if(VOS_ERROR == VOS_QueSend(queid, args, WAIT_FOREVER, MSG_PRI_NORMAL))
                                {
                                	VOS_Free(name);
                                }

                                ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("pon %d notify auto file restore completly (task_OnuResConf)\r\n", ponid+1))
                            }
                        }
                        
#endif

                    }
                }
            }
        }

        VOS_TaskDelayEx(VOS_TICK_SECOND/10);
    }
}

#if 0
void printWaitRestoreOnuList(int ponid)
{
    cl_vty_console_out("onu_list: %08x %08x\r\n", s_waitRestoreOnuList[ponid][0], s_waitRestoreOnuList[ponid][1]);
}
#endif

/*
 * 初始化PON板上ONU配置恢复任务及消息队列， 共初始化与PON端口个数相同数目的任务，每个端口对应一个任务，每个任务一个队列.
 * 仅在PON板上执行此函数。
*/

void init_onuResConf(void)
{
    int i;

    VOS_MemZero(g_rename_list, sizeof(g_rename_list));

	s_waitRestoreOnuQueue = VOS_Malloc(MAXPON*sizeof(int), MODULE_RPU_CTC);
	s_waitRestoreOnuQueueTail = VOS_Malloc(MAXPON*sizeof(int), MODULE_RPU_CTC);
	s_semOnuResConf = VOS_Malloc(MAXPON*sizeof(int), MODULE_RPU_CTC);
	s_taskOnuResConfId = VOS_Malloc(MAXPON*sizeof(int), MODULE_RPU_CTC);

	if(s_waitRestoreOnuQueue && s_waitRestoreOnuQueueTail && s_semOnuResConf && s_taskOnuResConfId)
	{

		VOS_MemZero(s_waitRestoreOnuQueue, MAXPON*sizeof(int));
		VOS_MemZero(s_waitRestoreOnuQueueTail, MAXPON*sizeof(int));
        /*modi by luh @2014-10-11 */
#if 0        
        if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER)
        {
    	    for(i=0; i<MAXPON; i++)
    	    {

    	        char szName[80] ="";

    	        sprintf(szName, "onuresconf%d", i+1);

    	        s_semOnuResConf[i] = VOS_SemMCreate(VOS_SEM_Q_FIFO);

    	        /*s_taskOnuResConfId[i] = VOS_TaskCreate(szName, TASK_PRIORITY_NORMAL, task_OnuResConf, &i);*/
    	        s_taskOnuResConfId[i] = VOS_TaskCreateEx(szName, task_OnuResConf, TASK_PRIORITY_NORMAL, (64*1024), &i);
    	    }
        }
        else
        {
    	    for(i=0; i<MAXPONPORT_PER_BOARD; i++)
    	    {

    	        char szName[80] ="";

    	        sprintf(szName, "onuresconf%d", i+1);

    	        s_semOnuResConf[i] = VOS_SemMCreate(VOS_SEM_Q_FIFO);

    	        /*s_taskOnuResConfId[i] = VOS_TaskCreate(szName, TASK_PRIORITY_NORMAL, task_OnuResConf, &i);*/
    	        s_taskOnuResConfId[i] = VOS_TaskCreateEx(szName, task_OnuResConf, TASK_PRIORITY_NORMAL, (64*1024), &i);
    	    }
        }
#else
        if(SYS_LOCAL_MODULE_TYPE_IS_CPU_PON)
        {
    	    for(i=0; i<MAXPONPORT_PER_BOARD; i++)
    	    {

    	        char szName[80] ="";

    	        sprintf(szName, "onuresconf%d", i+1);

    	        s_semOnuResConf[i] = VOS_SemMCreate(VOS_SEM_Q_FIFO);

    	        /*s_taskOnuResConfId[i] = VOS_TaskCreate(szName, TASK_PRIORITY_NORMAL, task_OnuResConf, &i);*/
    	        s_taskOnuResConfId[i] = VOS_TaskCreateEx(szName, task_OnuResConf, TASK_PRIORITY_NORMAL, (64*1024), &i);
    	    }
        }
        else
        {
    	    for(i=0; i<MAXPON; i++)
    	    {

    	        char szName[80] ="";

    	        sprintf(szName, "onuresconf%d", i+1);

    	        s_semOnuResConf[i] = VOS_SemMCreate(VOS_SEM_Q_FIFO);

    	        /*s_taskOnuResConfId[i] = VOS_TaskCreate(szName, TASK_PRIORITY_NORMAL, task_OnuResConf, &i);*/
    	        s_taskOnuResConfId[i] = VOS_TaskCreateEx(szName, task_OnuResConf, TASK_PRIORITY_NORMAL, (64*1024), &i);
    	    }
        }
#endif
	}

}


static void addElementToRestoreQueue(short int queid, onu_conf_res_queue_list_t *p)
{
    if(s_waitRestoreOnuQueue[queid] == NULL)
        s_waitRestoreOnuQueue[queid] = p;
    else
        s_waitRestoreOnuQueueTail[queid]->next = p;

    s_waitRestoreOnuQueueTail[queid] = p;
}

static void insertElementToRestoreQueue(short int queid, onu_conf_res_queue_list_t *pp, onu_conf_res_queue_list_t *pd)
{
    if(pp && pd)
    {
        pd->next = pp->next;
        pp->next = pd;

        if(pp == s_waitRestoreOnuQueueTail[queid])
            s_waitRestoreOnuQueueTail[queid] = pd;
    }
}

static void delElementFromRestoreQueue(short int queid, onu_conf_res_queue_list_t *pd)
{
    onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[queid];
    onu_conf_res_queue_list_t *ppre = NULL;

    while(p)
    {
        if(p == pd)
        {
            if(ppre)
            {
                ppre->next = p->next;
                if(p == s_waitRestoreOnuQueueTail[queid])
                	s_waitRestoreOnuQueueTail[queid] = ppre;
            }
            else
            {
                s_waitRestoreOnuQueue[queid] = s_waitRestoreOnuQueue[queid]->next;
                if(p == s_waitRestoreOnuQueueTail[queid])
                    s_waitRestoreOnuQueueTail[queid] = NULL;
            }

            VOS_Free(p->e);
            VOS_Free(p);
            p = NULL;
        }
        else
        {
            ppre = p;
            p = p->next;
        }
    }

}

int addOnuToRestoreQueueByName(SHORT pon_id, SHORT onu_id, const char *name, onu_conf_res_act_t act)
{

    int ret = VOS_ERROR;
	char *temp_name = NULL;
    int queid;
    ULONG args[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_RENAME_CHECK_CDP, 0, 0};

    if(!OLT_LOCAL_ISVALID(pon_id))
        return ret;
    if(!name)
        return ret;

    ONU_ASSERT(onu_id);
    {
        onu_conf_res_queue_list_t *pe = NULL;

        /*先生成添加项*/
        onu_conf_res_queue_list_t *plist = VOS_Malloc(sizeof(onu_conf_res_queue_list_t), MODULE_RPU_CTC);
        onu_conf_res_queue_e *pdata = VOS_Malloc(sizeof(onu_conf_res_queue_e), MODULE_RPU_CTC);
        if(pdata && plist)
        {
            pdata->trynum = 0;
#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
            pdata->flags = OnuProfile_Part_ALL;
#endif
            pdata->action = act;
            pdata->pon_id = pon_id;
            pdata->onu_id = onu_id;
            VOS_StrCpy(pdata->filename, name);

            plist->e = pdata;
            plist->next = NULL;

            ret = VOS_OK;
        }
        else
        {
            if(pdata)
                VOS_Free(pdata);
            if(plist)
                VOS_Free(pdata);
        }

        if(ret == VOS_OK)
        {

            ret = VOS_ERROR;

            lock_onuRestoreCtrlItemList(pon_id);

            pe = s_waitRestoreOnuQueue[pon_id];
            /*找到此ONU*/
            while(pe)
            {
                if(pe->e->pon_id == pon_id && pe->e->onu_id == onu_id)
                    break;
                pe = pe->next;
            }

            if(pe)
            {
                int slot = GetCardIdxByPonChip(pon_id);
                int port = GetPonPortByPonChip(pon_id);
                int ponportidx = GetGlobalPonPortIdxBySlot(slot, port);
            
                if(act == ONU_CONF_RES_ACTION_DO)
                {
                    /*要加入一个do操作，队列中有undo操作，如果之后不是此ONU的操作，插入do操作，否则不操作*/
                    if(pe->e->action == ONU_CONF_RES_ACTION_UNDO)
                    {
                        if(!pe->next || (pe->next && pe->next->e->onu_id != onu_id))
                        {
                            if(VOS_StrCmp(pe->e->filename, plist->e->filename) == 0)
                            {
                                delElementFromRestoreQueue(pon_id, pe->next);
                                VOS_Free(pdata);
                                VOS_Free(plist);   
                                ret = VOS_OK;
							}
							else
							{
                                insertElementToRestoreQueue(pon_id, pe, plist);
    							ret = VOS_OK;
							}                            
                        }
                        else
                        {
							if(0 == VOS_StrnCmp(pe->next->e->filename,"*auto",5))
                            {                        
                                if(VOS_StrCmp(pe->next->e->filename, plist->e->filename) == 0)
                                {
                                    /*do 同一个临时文件，后一次操作放弃*/
                                    VOS_Free(pdata);
                                    VOS_Free(plist);
                                    ret = VOS_OK;                                                              
                                }
                                else if(VOS_StrCmp(pe->e->filename, plist->e->filename) == 0)
                            	{
                            		/*本次添加的do操作与上一次的undo操作是同一文件*/
                                    VOS_Free(pdata);
                                    VOS_Free(plist);
                                    ret = VOS_OK;                                                              
                            	}
								else
                                {                                
    								sfun_clrOnuConfNameMapByPonId(pe->next->e->filename, ponportidx, pe->next->e->onu_id);
                                    /*删除临时文件的do操作时，应该把临时文件中的onuid删除，并触发一次把临时文件拷贝到源文件的操作*/
                                    {
                                        /*发送PON端口编辑临时文件恢复结束*/
                                        temp_name = VOS_Malloc(ONU_CONFIG_NAME_LEN, MODULE_RPU_CTC);
                                        if(temp_name)
                                        {
                                            VOS_MemZero(temp_name, ONU_CONFIG_NAME_LEN);
                                            VOS_StrCpy(temp_name, pe->next->e->filename);
                                            args[3] = (int)temp_name;                                    
                                        }
                                    }
    								
                                    delElementFromRestoreQueue(pon_id, pe->next);
                                    insertElementToRestoreQueue(pon_id, pe, plist);
                                    ret = VOS_OK;                                                                                                  
                                }
                            }
                        }
                    }
                    else if(pe->e->action == ONU_CONF_RES_ACTION_DO)
                    {
                        /*此种情况不可能发生*/
                        
                    }
                }

                else if(act == ONU_CONF_RES_ACTION_UNDO)
                {
            		/*modified by liyang @2015-04-14 for undo-do action balance */
					if(pe->e->action == ONU_CONF_RES_ACTION_UNDO)
                    {          
                        if(pe->next && pe->next->e->onu_id == onu_id && pe->next->e->action == ONU_CONF_RES_ACTION_DO)
                        {                            
                            if(VOS_StrCmp(pe->next->e->filename, plist->e->filename) == 0)
                            {     
                                /*上一次do与本次undo是同一个文件可以抵消，否则统一添加在末尾*/                            
                            	delElementFromRestoreQueue(pon_id, pe->next);
                            }
                            else if(VOS_StrCmp(pe->e->filename, plist->e->filename) == 0)
                            {
                                /*前后两次操作undo同一文件，说明上一次操作是配置文件修改，那么此次undo操作放弃*/
                                /*而do操作，在do的case中进行处理*/
                            }  
							else
							{
								/*do nothing*/
							}
                        }
    					VOS_Free(pdata);
                   		VOS_Free(plist);
    					ret = VOS_OK;                          
    					
                    }
                    else if(pe->e->action == ONU_CONF_RES_ACTION_DO)/*added by luh@2015-07-12*/
                    {
                        /*当插入一个undo操作时，且原来的只有一个do操作时，应该先添加在其后添加undo操作并删除掉原来的do操作*/
                        insertElementToRestoreQueue(pon_id, pe, plist);
                        delElementFromRestoreQueue(pon_id, pe);
    					ret = VOS_OK;                    
                    }
                    else
                    {
                        /*不会进入这个case， ret == VOS_ERROR*/
                    }
                }
            }
            else
            {
                /*当前队列中没有此ONU，直接添加到队尾*/
                addElementToRestoreQueue(pon_id, plist);
                ret = VOS_OK;
            }

            unlock_onuRestoreCtrlItemList(pon_id);

            /*没有进行操作时，释放之前申请的内存*/
            if(ret == VOS_ERROR)
            {
                VOS_Free(pdata);
                VOS_Free(plist);
            }
        }
		
        if(temp_name)
        {
            queid = getQueIdBySlotno(GetCardIdxByPonChip(pon_id));
    
            if(VOS_ERROR == VOS_QueSend(queid, args, WAIT_FOREVER, MSG_PRI_NORMAL))
        	{
        		VOS_Free(temp_name);
        	}
    
            ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("pon %d notify auto file restore completly\r\n", pon_id+1))
        }
		
    }

    return ret;
}

int addOnuToRestoreQueue(SHORT pon_id, SHORT onu_id, onu_conf_res_act_t act, UCHAR restore_flags)
{

    int ret = VOS_ERROR;
	char *temp_name = NULL;
    int queid;
    ULONG args[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_RENAME_CHECK_CDP, 0, 0};


    if(!OLT_LOCAL_ISVALID(pon_id))
        return ret;

    ONU_ASSERT(onu_id);

    {
        onu_conf_res_queue_list_t *pe = NULL;

        /*先生成添加项*/
        onu_conf_res_queue_list_t *plist = VOS_Malloc(sizeof(onu_conf_res_queue_list_t), MODULE_RPU_CTC);
        onu_conf_res_queue_e *pdata = VOS_Malloc(sizeof(onu_conf_res_queue_e), MODULE_RPU_CTC);
        if(pdata && plist)
        {
            pdata->flags = restore_flags;
            pdata->trynum = 0;
            pdata->action = act;
            pdata->pon_id = pon_id;
            pdata->onu_id = onu_id;
            VOS_StrCpy(pdata->filename, ONU_CONF_NAME_PTR_GET(pon_id, onu_id));

            plist->e = pdata;
            plist->next = NULL;

            ret = VOS_OK;
        }
        else
        {
            if(pdata)
                VOS_Free(pdata);
            if(plist)
                VOS_Free(pdata);
        }

        if(ret == VOS_OK)
        {

            ret = VOS_ERROR;

            lock_onuRestoreCtrlItemList(pon_id);

            pe = s_waitRestoreOnuQueue[pon_id];
            /*找到此ONU*/
            while(pe)
            {
                if(pe->e->pon_id == pon_id && pe->e->onu_id == onu_id)
                    break;
                pe = pe->next;
            }
            if(pe)
            {
                int slot = GetCardIdxByPonChip(pon_id);
                int port = GetPonPortByPonChip(pon_id);
                int ponportidx = GetGlobalPonPortIdxBySlot(slot, port);
            
                if(act == ONU_CONF_RES_ACTION_DO)
                {
                    /*要加入一个do操作，队列中有undo操作，如果之后不是此ONU的操作，插入do操作，否则不操作*/
                    if(pe->e->action == ONU_CONF_RES_ACTION_UNDO)
                    {
                        if(!pe->next || (pe->next && pe->next->e->onu_id != onu_id))
                        {
                            if(VOS_StrCmp(pe->e->filename, plist->e->filename) == 0)
                            {
                                delElementFromRestoreQueue(pon_id, pe->next);
                                VOS_Free(pdata);
                                VOS_Free(plist);   
                                ret = VOS_OK;
							}					
							else
							{
                                insertElementToRestoreQueue(pon_id, pe, plist);
    							ret = VOS_OK;
							}                            
                        }
                        else
                        {
							if(0 == VOS_StrnCmp(pe->next->e->filename,"*auto",5))
                            {                        
                                if(VOS_StrCmp(pe->next->e->filename, plist->e->filename) == 0)
                                {
                                    /*do 同一个临时文件，后一次操作放弃*/
                                    VOS_Free(pdata);
                                    VOS_Free(plist);
                                    ret = VOS_OK;                                                              
                                }
                                else if(VOS_StrCmp(pe->e->filename, plist->e->filename) == 0)
                            	{
                            		/*本次添加的do操作与上一次的undo操作是同一文件*/
                                    VOS_Free(pdata);
                                    VOS_Free(plist);
                                    ret = VOS_OK;                                                              
                            	}								
                                else
                                {
    								sfun_clrOnuConfNameMapByPonId(pe->next->e->filename, ponportidx, pe->next->e->onu_id);
                                    /*删除临时文件的do操作时，应该把临时文件中的onuid删除，并触发一次把临时文件拷贝到源文件的操作*/
                                    {
                                        /*发送PON端口编辑临时文件恢复结束*/
                                        temp_name = VOS_Malloc(ONU_CONFIG_NAME_LEN, MODULE_RPU_CTC);
                                        if(temp_name)
                                        {
                                            VOS_MemZero(temp_name, ONU_CONFIG_NAME_LEN);
                                            VOS_StrCpy(temp_name, pe->next->e->filename);
                                            args[3] = (int)temp_name;                                    
                                        }                                    
                                    }
    								
                                    delElementFromRestoreQueue(pon_id, pe->next);
                                    insertElementToRestoreQueue(pon_id, pe, plist);
                                    ret = VOS_OK;                                                                                                  
                                }
                            }
							else
							{
								/*do nothing*/
							}
                        }
                    }
                    else if(pe->e->action == ONU_CONF_RES_ACTION_DO)
                    {
                        /*此种情况不可能发生*/
                        
                    }
                }

                else if(act == ONU_CONF_RES_ACTION_UNDO)
                {
            		/*modified by liyang @2015-04-14 for undo-do action balance */
					if(pe->e->action == ONU_CONF_RES_ACTION_UNDO)
                    {          
                        if(pe->next && pe->next->e->onu_id == onu_id && pe->next->e->action == ONU_CONF_RES_ACTION_DO)
                        {                            
                            if(VOS_StrCmp(pe->next->e->filename, plist->e->filename) == 0)
                            {     
                                /*上一次do与本次undo是同一个文件可以抵消，否则统一添加在末尾*/                            
                            	delElementFromRestoreQueue(pon_id, pe->next);
                            }
                            else if(VOS_StrCmp(pe->e->filename, plist->e->filename) == 0)
                            {
                                /*前后两次操作undo同一文件，说明上一次操作是配置文件修改，那么此次undo操作放弃*/
                                /*而do操作，在do的case中进行处理*/
                            }    							                          
                        }
    					VOS_Free(pdata);
                   		VOS_Free(plist);
    					ret = VOS_OK;                          
    					
                    }
                    else if(pe->e->action == ONU_CONF_RES_ACTION_DO)/*added by luh@2015-07-12*/
                    {
                        /*当插入一个undo操作时，且原来的只有一个do操作时，应该先添加在其后添加undo操作并删除掉原来的do操作*/
                        insertElementToRestoreQueue(pon_id, pe, plist);
                        delElementFromRestoreQueue(pon_id, pe);
    					ret = VOS_OK;                    
                    }
                    else
                    {
                        /*不会进入这个case， ret == VOS_ERROR*/
                    }
#if 0	            /*deleted by liyang @2015-04-14 */
                    /*当插入一个undo操作时，要删除之前队列中的任何操作*/
                    insertElementToRestoreQueue(pon_id, pe, plist);
                    delElementFromRestoreQueue(pon_id, pe);
					ret = VOS_OK;
#endif
                }
            }
            else
            {
                /*当前队列中没有此ONU，直接添加到队尾*/
                addElementToRestoreQueue(pon_id, plist);
                ret = VOS_OK;
            }
			

            unlock_onuRestoreCtrlItemList(pon_id);

            /*没有进行操作时，释放之前申请的内存*/
            if(ret == VOS_ERROR)
            {
                VOS_Free(pdata);
                VOS_Free(plist);
            }
        }
		
        if(temp_name)
        {
            queid = getQueIdBySlotno(GetCardIdxByPonChip(pon_id));
    
            if(VOS_ERROR == VOS_QueSend(queid, args, WAIT_FOREVER, MSG_PRI_NORMAL))
        	{
        		VOS_Free(temp_name);
        	}
    
            ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("pon %d notify auto file restore completly\r\n", pon_id+1))
        }

    }


    return ret;
}


int delOnuFromRestoreQueue(SHORT pon_id, SHORT onu_id)
{

    int ret = VOS_ERROR;
    char filename[ONU_CONFIG_NAME_LEN] = {0};
    if(!OLT_LOCAL_ISVALID(pon_id))
        return ret;

    ONU_ASSERT(onu_id);

    lock_onuRestoreCtrlItemList(pon_id);

    {
        /*找到此ONU的操作*/
        onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[pon_id];
        while(p)
        {
            if(p->e->pon_id == pon_id && p->e->onu_id == onu_id)
                break;
            p = p->next;
        }

        if(p)
        {
            VOS_StrCpy(filename, p->e->filename);
            /*当ONU操作为do时，可删除此操作，否则不删除操作*/
            if(p->e->action == ONU_CONF_RES_ACTION_DO)
            {
                delElementFromRestoreQueue(pon_id, p);
                ret = VOS_OK;
            }

        }

    }

    unlock_onuRestoreCtrlItemList(pon_id);

    
    if(getOnuConfRestoreFlag(pon_id, onu_id))
    {
        VOS_TaskDelay(100);
    }
    
    setOnuConfResult(pon_id, onu_id, ONU_CONF_EXEC_RESULT_NULL);
    sendOnuConfReportMsg(GetCardIdxByPonChip(pon_id), GetPonPortByPonChip(pon_id), onu_id+1, filename, ONU_CONF_EXEC_RESULT_NULL);
    return ret;
}


int checkAndRenameOnuConfig(char *name)
{
    int i,j, ret = VOS_ERROR;

    char rename[80] = "";

    if(getFromRenameList(name, rename) == VOS_OK)
    {

        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(name);
            ONUConfigData_t *pp = getOnuConfFromHashBucket(rename);
            if(pd && pp)
            {
                for(i=0; i<MAXPON; i++)
                {
                    int slot = GetCardIdxByPonChip(i);
                    int port = GetPonPortByPonChip(i);
                    int ponportidx = GetGlobalPonPortIdxBySlot(slot, port);
					ponportidx = onuConfGetPonIdFromPonProtectGroup(ponportidx);
                    /*modi by luh 2013-8-20, for Q.18474*/
#if 0
                    if((SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER && slot != SYS_LOCAL_MODULE_SLOTNO) || (SlotCardIsPonBoard(slot) != VOS_OK))
#else
                    if((SYS_LOCAL_MODULE_TYPE_IS_CPU_PON && slot != SYS_LOCAL_MODULE_SLOTNO) || (SlotCardIsPonBoard(slot) != VOS_OK))
#endif
						continue;
					else
                    {
                        for(j=0; j<MAXONUPERPON; j++)
                        {
                            int offset = j/8;
                            int bitnum = j&7;

                            if(pd->onulist[ponportidx][offset] == 0)
                            {
                                j+=7;
                                continue;
                            }

                            if(pd->onulist[ponportidx][offset] & (1<<bitnum))
                                break;
                        }
                        if(j < MAXONUPERPON)
                            break;
                    }
                }

                if(i == MAXPON)
                {
                    ret = sfun_OnuConfRename(name, rename);
                    delFromRenameList(name);
                }
            }
        }
        ONU_CONF_SEM_GIVE

    }

    return ret;
}

int SendCheckOnuRestoreMsg(void *pd)
{  
    ULONG queid;
    ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)pd;
    ULONG ulArgs[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_DEL_CDP, 0, 0};
    queid = getQueIdBySlotno(SYS_LOCAL_MODULE_SLOTNO);
    ulArgs[3] = (ULONG)p;

    return VOS_QueSend(queid, ulArgs,WAIT_FOREVER, MSG_PRI_NORMAL);
}
/*检查ONU 恢复队列中是否有需要删除的profile    added  by luh 2011-10-11*/
int CheckOnuConfRestoreQueueByName(char *name)
{
	char i;
 	char queue_file_flag=0;	
		for(i=0;i<CARD_MAX_PON_PORTNUM;i++)
	    {
			lock_onuRestoreCtrlItemList(i);  
		    {
           		 onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[i];
		        while(p)
		        {
			    	if(VOS_StrCmp((char *)p->e->filename,(char *)name)==0)
			    	{
			        	queue_file_flag = 1;
			            break;
			        }
			        else
			        {
			           	p = p->next;
			        }
		      	}
			}
		    unlock_onuRestoreCtrlItemList(i);  
			if(queue_file_flag)
	        break;
		}
	if(queue_file_flag)
		return VOS_OK;
	else
		return VOS_ERROR;
}
int CheckOnuConfRestoreQueue(ONUCONFSYNDMSGPTR_T pd)
{
    int i = 0;
    onu_conf_res_queue_list_t *ppre = NULL;
    int flag = 0;
    ULONG  pon_timer = 0;
    char * name; 
#if 0    
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER)
    {
	    for(i=0; i<MAXPON; i++)
	    {
            onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[i];
            while(p)
            {
                if(VOS_StrCmp((char *)p->e->filename,(char *)pd->data)==0)
                {
                    flag = 1;
                    break;
                }
                else
                {
                    ppre = p;
                    p = p->next;
                }
            }
            if(flag)
                break;
            
	    }
    }    
    else
    {
        for(i=0;i<CARD_MAX_PON_PORTNUM;i++)
        {
            onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[i];
            while(p)
            {
                if(VOS_StrCmp((char *)p->e->filename,(char *)pd->data)==0)
                {
                    flag = 1;
                    break;
                }
                else
                {
                    ppre = p;
                    p = p->next;
                }
            }
            if(flag)
                break;
        }
    }
#else
    if(SYS_LOCAL_MODULE_TYPE_IS_CPU_PON)
    {
        for(i=0;i<CARD_MAX_PON_PORTNUM;i++)
        {
        
            onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[i];
            while(p)
            {
                if(VOS_StrCmp((char *)p->e->filename,(char *)pd->data)==0)
                {
                    flag = 1;
                    break;
                }
                else
                {
                    ppre = p;
                    p = p->next;
                }
            }
			
            if(flag)
                break;
        }
    }
    else
    {
	    for(i=0; i<MAXPON; i++)
	    {
            onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[i];
            while(p)
            {
                if(VOS_StrCmp((char *)p->e->filename,(char *)pd->data)==0)
                {
                    flag = 1;
                    break;
                }
                else
                {
                    ppre = p;
                    p = p->next;
                }
            }
            if(flag)
                break;
            
	    }
    }    
        
#endif
    if(flag)
    {
        pon_timer = VOS_TimerCreate(MODULE_RPU_CTC, 0, 1000, SendCheckOnuRestoreMsg, (void *)pd, VOS_TIMER_NO_LOOP);
        if( pon_timer == VOS_ERROR)
        {
            VOS_ASSERT(0);
        }
    }
    else
    {
        name = (char*)pd->data;
        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, name, NULL, NULL);                            
        if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER && !SYS_LOCAL_MODULE_ISMASTERACTIVE)
            CDP_FreeMsg(pd);
        else
            VOS_Free(pd);
    }
    return VOS_OK;

}
int GetOnuConfRestoreQueueStatus(char *name)
{
    int i = 0;
    onu_conf_res_queue_list_t *ppre = NULL;
    int flag = 0;

    if(!name)
        return flag;
    
    if(/*SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER*/0)
    {
	    for(i=0; i<MAXPON; i++)
	    {
            lock_onuRestoreCtrlItemList(i);    
            {
                onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[i];
                while(p)
                {
                    if(VOS_StrCmp((char *)p->e->filename,(char *)name)==0)
                    {
                        flag = 1;
                        break;
                    }
                    else
                    {
                        ppre = p;
                        p = p->next;
                    }
                }
            }
            unlock_onuRestoreCtrlItemList(i);            
            if(flag)
                break;            
	    }
    }    
    else
    {
        for(i=0;i<CARD_MAX_PON_PORTNUM;i++)
        {
            lock_onuRestoreCtrlItemList(i);  
            {
                onu_conf_res_queue_list_t *p = s_waitRestoreOnuQueue[i];
                while(p)
                {
                    if(VOS_StrCmp((char *)p->e->filename,(char *)name)==0)
                    {
                        flag = 1;
                        break;
                    }
                    else
                    {
                        ppre = p;
                        p = p->next;
                    }
                }
            }
            unlock_onuRestoreCtrlItemList(i);            
            if(flag)
                break;
        }
    }
    
    return flag;
}
extern int  Build_recoverData_func(eventCtcSyncRecoverMsg_t * data);
extern int Parse_recoverData_func(eventCtcSyncRecoverMsg_t *alconfig);

static void onuconf_event_delprofile_callback(ULONG aulMsg[4])
{

	ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
#if 0    
    /*6100/6700主控板*/
	if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER && SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
		if(SYS_MODULE_ISMASTERSTANDBY(p->slotid))
			sendOnuConfSyndMsg(p->slotid, p, aulMsg[2]);
		else
            CheckOnuConfRestoreQueue(p);		
    }
    /*modi by luh 2012-11-28, 针对6900M 做对应修改，6900 EPON板*/
	else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER&&!SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
		CheckOnuConfRestoreQueue(p);
	}
    /*备用主控*/
    else if(SYS_LOCAL_MODULE_ISMASTERSTANDBY)
    {
        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, (char*)p->data, NULL, NULL);                                    
        CDP_FreeMsg(p);
    }
    /*6900-6900M主控板*/
    else if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        if(p)
        {
            if(SYS_MODULE_IS_PONCARD_LOCAL_MANAGER(p->slotid) ||
				SYS_MODULE_ISMASTERSTANDBY(p->slotid))
                sendOnuConfSyndMsg(p->slotid, p, aulMsg[2]);
            else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)/*69M-69S 主控板应该等恢复完成再执行删除配置文件*/
        		CheckOnuConfRestoreQueue(p);
            else
                VOS_Free(p);/*6900主控，do nothing*/
        }

    }
    else
        VOS_ASSERT(0);
#else
    
	if(SYS_LOCAL_MODULE_TYPE_IS_CPU_PON)/*modi by luh 2014-10-17 所有cpuPon板理论上都都要检查恢复队列*/
    {
        CheckOnuConfRestoreQueue(p);		        
        /*针对6900m 1槽位主控还需要向目的槽位发送cdp消息，通知slave板删除配置文件*/
		/*if( p->slotid && p->slotid != SYS_LOCAL_MODULE_SLOTNO && SYS_LOCAL_MODULE_ISMASTERACTIVE)
			sendOnuConfSyndMsg(p->slotid, p, aulMsg[2]);*/
			/*因8000目前没有没有类似于6900M的设备,此处删去 by zhouzh*/
    }    
	else if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
        if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)/*67.61的主控板删除指定槽位的配置文件时，根据目的槽位进行裁决执行*/
        {
    		if(SYS_MODULE_ISMASTERSTANDBY(p->slotid))
    			sendOnuConfSyndMsg(p->slotid, p, aulMsg[2]);
    		else
                CheckOnuConfRestoreQueue(p);		            
        }
        else/*6900 8000等主控板只需要向特定板卡发送消息即可*/
        {
		    sendOnuConfSyndMsg(p->slotid, p, aulMsg[2]);
        }
	}    
    else if(SYS_LOCAL_MODULE_ISMASTERSTANDBY)/*备用主控*/
    {
        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, (char*)p->data, NULL, NULL);                                    
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);
#endif
}

static void onuconf_event_reqprofile_callback(ULONG aulMsg[4])
{
    ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];

#if 0
    ONUConfigData_t *psrcdata = (ONUConfigData_t *)p->data;

    ONUConfigData_t *pd = getOnuConfFromHashBucket(psrcdata->confname);
#else
    ONUConfigData_t *pd = NULL;
    LONG dstlen = sizeof(ONUConfigData_t), srclen = aulMsg[2]-sizeof(ONUCONFSYNDMSG_T);
    ONUConfigData_t *psrcdata = onuconf_malloc(ONU_CONF_MEM_DATA_ID);

    if(!psrcdata)
    {
    	CDP_FreeMsg(p);
    	return;
    }

    if( defLzmaUncompress((UCHAR*)psrcdata, &dstlen, p->data, &srclen) != VOS_OK)
    {
    	onuconf_free(psrcdata, ONU_CONF_MEM_DATA_ID);
    	CDP_FreeMsg(p);
    	return;
    }
#endif
    pd = getOnuConfFromHashBucket(psrcdata->confname);
    if(!pd)
    {
        /*pd = VOS_Malloc(sizeof(ONUConfigData_t), MODULE_RPU_ONU);*/
        pd = onuconf_malloc(ONU_CONF_MEM_DATA_ID);
        if (pd)
        {

            VOS_MemCpy(pd, psrcdata, sizeof(ONUConfigData_t));

            ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP,("\r\nmsg consynd_req: %d/%d %s\r\n",
                    p->ponid, p->onuid + 1, pd->confname));
            
            OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, pd->confname, NULL, pd);
#ifdef CDP_DEBUG
            BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
            {
                char sz[128];
                int len = 0;
                if (!(p->ponid | p->onuid))
                    len = VOS_Sprintf(
                                    sz,
                                    "\r\nMSG_ONU_CONFSYND_REQ_CDP: slot %d %s\r\n",
                                    p->slotid, pd->confname);
                else
                    len = VOS_Sprintf(
                                    sz,
                                    "\r\nMSG_ONU_CONFSYND_REQ_CDP: %d/%d/%d %s\r\n",
                                    p->slotid, p->ponid,
                                    p->onuid + 1, pd->confname);
                sendOnuConfDebugReportMsg(SYS_MASTER_ACTIVE_SLOTNO,
                        SYS_LOCAL_MODULE_SLOTNO, len + 1, sz);
            }
            END_ONUCONF_DEBUG_PRINT()
#endif

        }
        else
        {
#ifdef CDP_DEBUG
            BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)

            char sz[128];
            int len = VOS_Sprintf(sz,
                    "\r\nnMSG_ONU_CONFSYND_REQ_CDP: g_malloc mem fail!\r\n");
            sendOnuConfDebugReportMsg(SYS_MASTER_ACTIVE_SLOTNO,
                    SYS_LOCAL_MODULE_SLOTNO, len + 1, sz);
            sys_console_printf("%s", sz);

            END_ONUCONF_DEBUG_PRINT()
#endif
        }
    }
    else
    {
        /*ONUConfigData_t *newpd = VOS_Malloc(sizeof(ONUConfigData_t), MODULE_RPU_ONU);*/
        ONUConfigData_t *newpd = onuconf_malloc(ONU_CONF_MEM_DATA_ID);
        if(newpd)
        {
            VOS_MemCpy(newpd, psrcdata, sizeof(ONUConfigData_t));
            OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, pd->confname, NULL, newpd);            
        }

    }
	ONUCONF_DEBUG("\r\n onuconf_event_reqprofile_callback conf_filename:%s\r\n", pd->confname);
#if 1
    /*板间同步，临时文件涉及关联关系的修改*/
    if(VOS_StrnCmp(pd->confname, "*auto", 5) && SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
    {
        short int slot = SYS_LOCAL_MODULE_SLOTNO;
        short int port = 0;
        short int PonPortIdx = 0;
        short int OnuIdx = 0;
        for(port=1;port<=MAXPONPORT_PER_BOARD;port++)
        {
            short int loop = 0;
            int num = MAXONUPERPON/8;
            int i = 0;
            short int pon_id = GetPonPortIdxBySlot(slot, port);
            if(pon_id == RERROR)
                continue;
            
            PonPortIdx = GetGlobalPonPortIdxBySlot(slot, port);
			PonPortIdx = onuConfGetPonIdFromPonProtectGroup(PonPortIdx); 
            if(PonPortIdx == RERROR)
                continue;
            
            for(i=0; i<num; i++)
            {
                if(psrcdata->onulist[PonPortIdx][i])
                {
                    int j=0;
                    for(j=0; j<8; j++)
                    {
                        if(psrcdata->onulist[PonPortIdx][i]&(1<<j))
                        {
                            OnuIdx = i*8+j;
#if 0                            
                            /*只有pon板才执行undo+do,备用主控只执行修改关联关系*/
                            if(VOS_StriCmp(psrcdata->confname, DEFAULT_ONU_CONF)
                                &&GetOnuOperStatus(pon_id, OnuIdx) == ONU_OPER_STATUS_UP)
                            {
                                ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("%d:%d add to wait queue begin!\r\n", pon_id, OnuIdx));
                                OnuProfile_Action_ByCode(OnuAction_Undo_AssociateAndSync, 0, pon_id, OnuIdx, NULL, NULL, NULL);
                                OnuProfile_Action_ByCode(OnuMap_Update, 0, PonPortIdx, OnuIdx, psrcdata->confname, NULL, NULL);
                                OnuProfile_Action_ByCode(OnuAction_Do_AssociateAndSync, 0, pon_id, OnuIdx, NULL, NULL, NULL);
                                /*恢复新的配置文件*/
                                ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("%d:%d add to wait queue end!\r\n", pon_id, OnuIdx));
                            }
                            else
#endif                                
                            {
                                OnuProfile_Action_ByCode(OnuMap_Update, 0, PonPortIdx, OnuIdx, psrcdata->confname, NULL, NULL);
								if(OnuIdx <= MAX_DEBUG_ONU) ONUCONF_DEBUG("\r\n update onu(%d/%d) profile:%s\r\n", PonPortIdx, OnuIdx, psrcdata->confname);
                            }
                        }
                    }
                }
            }   
        }
    }
#endif    
#if 0
    {
        short int ponportidx;
        int onuentry;
#ifndef ONUID_MAP
        ponportidx = GetPonPortIdxBySlot(p->slotid, p->ponid);
#else
        ponportidx = GetPonPortIdxBySlot(p->slotid, p->ponid);
#endif
        if(ponportidx != -1)
        {
            onuentry = ponportidx * MAXONUPERPON + p->onuid;

#ifdef ONUID_MAP
            /*onuconfAssociateOnuId(ponportidx, p->onuid, psrcdata->confname);*/
            OnuProfile_Action_ByCode(OnuMap_Update, 0, ponportidx, p->onuid, psrcdata->confname, NULL, NULL);            
#else
            ONU_MGMT_SEM_TAKE

            VOS_StrCpy(OnuMgmtTable[onuentry].configFileName, psrcdata->confname);

            ONU_MGMT_SEM_GIVE
#endif
#if 0
            sendOnuConfSyndReqAckMsg(SYS_LOCAL_MODULE_SLOTNO, 0);
#endif
        }
#if 0
        else /*未指定具体的ONU ID，则用文件中的ONU ID列表更新映射关系表*/
            updateOnuConfOnuIdMapByName(psrcdata->confname);
#endif
    }
#endif
    onuconf_free(psrcdata, ONU_CONF_MEM_DATA_ID);
    CDP_FreeMsg(p);

}

static void onuconf_event_cardreq_callback(ULONG aulMsg[4])
{
	ULONG semid = getSemIdBySlotno(aulMsg[3]);
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        startOnuConfSyndRequestWitchDstSlot(aulMsg[3]);
    }
    setOnuConfSyndBroadcastCardMask(aulMsg[3]);

    /*VOS_SemGive(semid);*/
    sendOnuConfSyncCardReqFinishedMsg(aulMsg[3]);

    /*备用主控需要在自己的任务中等待同步完毕或超时，置同步结束信号
     * PON板同步结束置信号操作在板激活调用中执行*/
#if 0     
    if(SYS_MODULE_ISMASTERSTANDBY(aulMsg[3]))
    {
    	onuconf_wait_cardsyncfinished_signal(aulMsg[3], 0);
    	onuconf_signal_cardsyncfinished(aulMsg[3]);
    }
#endif
    ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP, ("card %d sync complete, ticks: %u, semid:%u", aulMsg[3], VOS_GetTick(), semid));
}

static void onuconf_event_alarmdata_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER || SYS_LOCAL_MODULE_ISMASTERSTANDBY)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
        eventCtcSyncRecoverMsg_t *psrcdata = (eventCtcSyncRecoverMsg_t *)p->data;
        Parse_recoverData_func(psrcdata);
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	
}

static void onuconf_event_transmissionflag_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T) aulMsg[3];
        setOnuTransmission_flag(p->data[0]);
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	
}
static void onuconf_event_updatependingtime_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T) aulMsg[3];
        SetUpdatePendingTime(p->data[0]);
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	
}
static void onuconf_event_privateptyenable_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_ISMASTERSTANDBY)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T) aulMsg[3];
        SetPrivatePtyEnable(p->data[0]);
        if(p)
            CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	
}

static void onuconf_event_applyreq_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T) aulMsg[3];

        if(p)
        {
            ULONG slot, pon, onuid;

            slot = p->slotid;
            pon = p->ponid;
            onuid = p->onuid;

            onu_profile_apply_by_onuid((const char *)p->data, slot, pon, onuid);

            CDP_FreeMsg(p);
        }
    }
    else
        VOS_ASSERT(0);	
}

static void onuconf_event_renamereq_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        int slen = aulMsg[2];
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T) aulMsg[3];
        sendOnuConfSyndMsg(p->slotid, p, slen);
    }
    else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER || SYS_LOCAL_MODULE_ISMASTERSTANDBY)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T) aulMsg[3];
        OnuProfile_Action_ByCode(OnuProfile_Modify, 0, 0, 0, (char*)p->data, ((char*)p->data)+ONU_CONFIG_NAME_LEN, NULL);        
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	
}

static void onuconf_event_renamecheck_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
        char *name = (char*)aulMsg[3];

        if(name)
        {
            checkAndRenameOnuConfig(name);
            VOS_Free(name);
        }

    }
    else
        VOS_ASSERT(0);	
}

static void onuconf_event_onuidmapreq_callback(ULONG aulMsg[4])
{

    if (SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER || SYS_LOCAL_MODULE_ISMASTERSTANDBY)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T) aulMsg[3];

        int slot = p->slotid;
        int pon = p->ponid;                    
        int ponid = GetGlobalPonPortIdxBySlot(slot, pon);
        int OnuIdx = 0;
        int len = MAXONUPERPON * sizeof(onuConfOnuIdMapEntry_t);

        if (ponid != RERROR)
        {
            ONU_CONF_SEM_TAKE
            {
                onuConfOnuIdMapEntry_t* pd = (onuConfOnuIdMapEntry_t*)(p->data);
                for(OnuIdx=0;OnuIdx<MAXONUPERPON;OnuIdx++)
                {
                    OnuProfile_Action_ByCode(OnuMap_Update, 0, ponid, OnuIdx, pd->name, NULL, NULL);
                    /*VOS_MemCpy(g_onuConfOnuIdMapArry[ponid][OnuIdx].name, pd->name, ONU_CONFIG_NAME_LEN);*/

                    /*备用主控需要恢复onu配置恢复结果，而pon板不需要。Q.24734*/
                    if(SYS_LOCAL_MODULE_ISMASTERSTANDBY)
                        setOnuConfResult(ponid, OnuIdx, pd->configResult);
                    pd++;
                }
            }
            ONU_CONF_SEM_GIVE           
        }

        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);

}

static void onuconf_event_macreq_callback(ULONG aulMsg[4])
{
    /*
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T) aulMsg[3];
        onuConfMacHashEntry_t *pd = (onuConfMacHashEntry_t*)p->data;

        char * name = getOnuConfMacFromHashBucket(pd->mac);

        if(name)
        {
            if(VOS_StriCmp(name, pd->name))
                VOS_StrCpy(name, pd->name);
        }
        else
        {
            onuConfMacHashEntry_t *pp = VOS_Malloc(sizeof(onuConfMacHashEntry_t), MODULE_RPU_ONU);
            if(pp)
            {
                VOS_MemCpy(pp, pd, sizeof(onuConfMacHashEntry_t));
                setOnuConfMacToHashBucket(pd->mac, pp);
            }
        }
#ifdef CDP_DEBUG

        BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
        {
            char sz[128];
            int len = VOS_Sprintf(sz, "\r\nMSG_ONU_CONFSYND_MAC_REQ_CDP: slot(%d) %s %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                    p->slotid, pd->name, (UINT32)pd->mac[0], (UINT32)pd->mac[1],(UINT32)pd->mac[2],
                    (UINT32)pd->mac[3],(UINT32)pd->mac[4],(UINT32)pd->mac[5]);
            sendOnuConfDebugReportMsg(SYS_MASTER_ACTIVE_SLOTNO,
                    SYS_LOCAL_MODULE_SLOTNO, len + 1, sz);
        }
        END_ONUCONF_DEBUG_PRINT()
#endif
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	*/
}

static void onuconf_event_execute_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
        short int ponportidx = GetPonPortIdxBySlot(p->slotid, p->ponid);
        if(addOnuToRestoreQueue(ponportidx, p->onuid, p->data[0], -1) == VOS_OK)
        {
            
#ifdef CDP_DEBUG

            BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
            char sz[128];
            char *addr = OnuMgmtTable[ponportidx*MAXONUPERPON+p->onuid].DeviceInfo.MacAddr;
            int len = VOS_Sprintf(sz, "MSG_ONU_CONFSYND_EXECUTE_CDP: %d/%d/%d %02x:%02x:%02x:%02x:%02x:%02x:\r\n",
                    p->slotid, p->ponid, p->onuid+1, (UINT32)addr[0],(UINT32)addr[1],(UINT32)addr[2],
                    (UINT32)addr[3],(UINT32)addr[4],(UINT32)addr[5]);
            sendOnuConfDebugReportMsg(SYS_MASTER_ACTIVE_SLOTNO, SYS_LOCAL_MODULE_SLOTNO, len+1, sz);
            END_ONUCONF_DEBUG_PRINT()
#endif
        }
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	
}

static void onuconf_event_undoassociationbyname_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        char *name = (char*)aulMsg[2];

        if(name)
        {
            sendOnuConfSyndUndoAssociationByNameMsg(aulMsg[3], name, VOS_StrLen(name));

            VOS_Free(name);
        }
    }
    else if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
        onuconfUndoAssociationByName((char*)p->data);
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	
}

static void onuconf_event_undoassociationbyid_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
        short int ponid = GetPonPortIdxBySlot(p->slotid, p->ponid);

        /*onuconfAssociateOnuId(ponid, p->onuid-1, DEFAULT_ONU_CONF);*/

        CDP_FreeMsg(p);

    }
    else
        VOS_ASSERT(0);	
}

static void onuconf_event_resultreport_callback(ULONG aulMsg[4])
{
#if 0    
    if(SYS_LOCAL_MODULE_TYPE_IS_PON_TOP_MANAGER)
    {

        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
		short int ponportidx = GetPonPortIdxBySlot(p->slotid, p->ponid);

#ifndef ONUID_MAP
		int onuEntry = ponportidx*MAXONUPERPON+p->onuid-1;

		ONU_MGMT_SEM_TAKE
#if 0 /*don't set onu-file map by pon request 2011-07-18 by wangxiaoyu*/
		if(VOS_StriCmp(OnuMgmtTable[onuEntry].configFileName, (char*)(p->data+1)))
		{
#if 0
		    VOS_StrCpy(OnuMgmtTable[onuEntry].configFileName, p->data+1);
		    sendOnuConfMacToAllPonBoard(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, p->data+1);
#else
		    onuconfAssociateOnuId(ponportidx, p->onuid-1, p->data+1);
		    sendOnuConfMacToAllPonBoard(OnuMgmtTable[onuEntry].DeviceInfo.MacAddr, p->data+1);
#endif
		}
#endif
		OnuMgmtTable[onuEntry].configResult = p->data[0];

        sys_console_printf("\r\nmsg consynd_res_report: %d/%d/%d %s %s\r\n", p->slotid, p->ponid, p->onuid, OnuMgmtTable[onuEntry].configFileName,
        p->data[0]==ONU_CONF_EXEC_RESULT_OK?"OK":"ERROR");

        ONU_MGMT_SEM_GIVE
#else
        ONU_CONF_SEM_TAKE
        {
            setOnuConfResult(ponportidx, p->onuid-1, p->data[0]);
            ONU_CONF_SEM_GIVE
        }

        BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
        cl_vty_all_out("\r\nmsg consynd_res_report: %d/%d/%d %s %s\r\n", p->slotid, p->ponid, p->onuid, (char*)(p->data+1), p->data[0]==ONU_CONF_EXEC_RESULT_OK?"OK":"ERROR");
        END_ONUCONF_DEBUG_PRINT()
#endif
		
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	
#else

    ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
	short int ponportidx = GetPonPortIdxBySlot(p->slotid, p->ponid);
    ONUCONFSYNDMSGPTR_T pd = NULL;
    ULONG len = 0;

    if(ponportidx != RERROR)/*modi by luh 2014-05-08, pon 1/1不能恢复结果*/
    {
        ONU_CONF_SEM_TAKE
        {
            setOnuConfResult(ponportidx, p->onuid-1, p->data[0]);
        }
        ONU_CONF_SEM_GIVE
        BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
        cl_vty_all_out("\r\nmsg consynd_res_report: %d/%d/%d %s %s\r\n", p->slotid, p->ponid, p->onuid, (char*)(p->data+1), p->data[0]?(p->data[0]==ONU_CONF_EXEC_RESULT_OK?"OK":"ERROR"):"NULL");
        END_ONUCONF_DEBUG_PRINT()
    }
    
	if( SYS_LOCAL_MODULE_ISMASTERACTIVE )
	{
		LONG to_slotno = device_standby_master_slotno_get();
		if( to_slotno != 0 )
		{
            len = sizeof(ONUCONFSYNDMSG_T)+sizeof(int)+ONU_CONFIG_NAME_LEN;
            pd = VOS_Malloc(len, MODULE_RPU_CTC);
            if (pd)
            {
                pd->slotid = p->slotid;
                pd->ponid = p->ponid;
                pd->onuid = p->onuid;
                pd->msgtype = p->msgtype;
        		pd->data[0] = p->data[0];
        		
        		ONU_CONF_SEM_TAKE;
    		    VOS_StrCpy((char*)(pd->data+1), (char*)(p->data+1));
        		ONU_CONF_SEM_GIVE;
                sendOnuConfSyndMsg(to_slotno, pd, len);
            }
		}
	}
    if(p) CDP_FreeMsg(p);
#endif
}

static void onuconf_event_debugreport_callback(ULONG aulMsg[4])
{
    ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];

    BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
    char buf[BUFSIZ]="";
    char *config_buf = (char*)p->data;
    cl_vty_all_out("\r\n\r\nonuconfsynd debug msg from slot %d:\r\n\r\n", p->slotid);
    while(cl_read_memfile_to_buf(buf, (char**)&config_buf, BUFSIZ))
        cl_vty_all_out("%s\r\n", buf);
    END_ONUCONF_DEBUG_PRINT()

    if(p->slotid != SYS_LOCAL_MODULE_SLOTNO)
        CDP_FreeMsg(p);
    else
        VOS_Free(p);
}

static void onuconf_event_ack_callback(ULONG aulMsg[4])
{

    ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
    VOS_SemGive(getSemIdBySlotno(p->slotid));
    CDP_FreeMsg(p);
}	

static void onuconf_event_setothervendorregistration_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)VOS_Malloc(sizeof(ONUCONFSYNDMSG_T), MODULE_RPU_CTC);
        if(p)
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            VOS_MemZero(p, len);
            p->slotid = aulMsg[2];
            p->data[0] = aulMsg[3];
            p->msgtype = MSG_ONU_OTHER_VENDOR_REGISTRATION_CDP;

            sendOnuConfSyndMsg(p->slotid, p, len);
        }
    }
    else
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
        if(p)
        {
            set_ctcOnuOtherVendorAccept(p->data[0]);
            CDP_FreeMsg(p);
        }
    }
}

static void onuconf_event_card_req_finished_callback(ULONG aulMsg[4])
{
	ONUCONFSYNDMSGPTR_T pmsg = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
	if(pmsg)
	{

		sendOnuConfSyncCardReqFinishedAckMsg(SYS_MASTER_ACTIVE_SLOTNO);

		CDP_FreeMsg(pmsg);

	}

	ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("master active card sync finished!\r\n"));

}

static void onuconf_event_card_req_finished_ack_callback(ULONG aulMsg[4])
{
	ONUCONFSYNDMSGPTR_T pmsg = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
	if(pmsg)
	{
		int slot = pmsg->data[0];

		/*VOS_SemGive(getSemIdBySlotno(slot));*/
		onuconf_signal_cardsyncfinished(slot);

		CDP_FreeMsg(pmsg);

	}

}
static void onuconf_event_delfault_callback(ULONG aulMsg[4])
{
	ONUCONFSYNDMSGPTR_T pmsg = (ONUCONFSYNDMSGPTR_T)aulMsg[3];
	if(pmsg)
	{
		if(pmsg->slotid != SYS_LOCAL_MODULE_SLOTNO)
			CDP_FreeMsg(pmsg);
		else
			VOS_Free(pmsg);
	}
	
}
static void onuconf_event_onu_model_black_list_callback(ULONG aulMsg[4])
{
    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {
        ONUCONFSYNDMSGPTR_T p = (ONUCONFSYNDMSGPTR_T) aulMsg[3];
		if(p->slotid == 1)
	        add_black_onu_model(p->data[0]);
		else if(p->slotid == 2)
	        del_black_onu_model(p->data[0]);
		else
		{
			/*do nothing*/
		}
        CDP_FreeMsg(p);
    }
    else
        VOS_ASSERT(0);	
}

int registerOnuConfEventCallback(ULONG msgtype, onuconf_event_callback data)
{
	if(msgtype < MSG_ONU_CONFSYND_CDP_MAX)
	{
		g_onuconf_event_callback[msgtype] = data;
		return VOS_OK;
	}

	return VOS_ERROR;
}

onuconf_event_callback getOnuConfEventCallback(ULONG msgtype)
{
	if(msgtype < MSG_ONU_CONFSYND_CDP_MAX && g_onuconf_event_callback[msgtype])
		return g_onuconf_event_callback[msgtype];
	else
		return onuconf_event_delfault_callback;
}


void init_onuconfEventCallbackArray()
{

	VOS_MemSet(g_onuconf_event_callback, 0, sizeof(g_onuconf_event_callback));
	
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_CARD_REQ, onuconf_event_cardreq_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_REQ_CDP, onuconf_event_reqprofile_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_MAC_REQ_CDP, onuconf_event_macreq_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_DEL_CDP, onuconf_event_delprofile_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_EXECUTE_CDP, onuconf_event_execute_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_UNDO_ASSOCIATION_BY_NAME, onuconf_event_undoassociationbyname_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_UNDO_ASSOCIATION_BY_ONUID, onuconf_event_undoassociationbyid_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_ONUID_MAP_REQ_CDP, onuconf_event_onuidmapreq_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_RESULT_REPORT_CDP, onuconf_event_resultreport_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_DEBUG_REPORT_CDP, onuconf_event_debugreport_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_APPLY_REQ, onuconf_event_applyreq_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_RENAME_REQ_CDP, onuconf_event_renamereq_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_RENAME_CHECK_CDP, onuconf_event_renamecheck_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_REQ_ACK_CDP, onuconf_event_ack_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_TRANSMISSION_FLAG_CDP, onuconf_event_transmissionflag_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_PENDING_UPDATETIME_CDP, onuconf_event_updatependingtime_callback);  
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_PRIVATE_PTY_ENABLE_CDP, onuconf_event_privateptyenable_callback);    
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_ALARM_DATA_CDP, onuconf_event_alarmdata_callback);
	registerOnuConfEventCallback(MSG_ONU_OTHER_VENDOR_REGISTRATION_CDP, onuconf_event_setothervendorregistration_callback);
	
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_CARD_REQ_FINISHED_CDP, onuconf_event_card_req_finished_callback);
	registerOnuConfEventCallback(MSG_ONU_CONFSYND_CARD_REQ_FINISHED_ACK_CDP, onuconf_event_card_req_finished_ack_callback);

	registerOnuConfEventCallback(MSG_ONU_CONFSYND_ONU_MODEL_BLACK_LIST_CDP, onuconf_event_onu_model_black_list_callback);	
}

/*
 * onu配置数据CDP管理任务
 */

DECLARE_VOS_TASK(task_onuConfSynd)
{
    ULONG queid = s_onuConfSyndQueId[ulArg1];
    ULONG aulMsg[4];

    while(VOS_QueReceive(queid, aulMsg, WAIT_FOREVER) != VOS_ERROR)
    {	
		onuconf_event_callback pfunc = getOnuConfEventCallback(aulMsg[1]);
		(*pfunc)(aulMsg);
    }	
}

#if 0
int sendOnuConfSyndReqMsg(USHORT slot, USHORT pon, USHORT onu)
{
    int ret = VOS_OK;

#ifndef ONUID_MAP
    int ppidx = GetGlobalPonPortIdxBySlot(slot, pon);
    int onuEntry = ppidx*MAXONUPERPON+onu-1;



    ONUConfigData_t * pd = getOnuConfFromHashBucket(OnuMgmtTable[onuEntry].configFileName);
#else
    int ponid = GetPonPortIdxBySlot(slot, pon);
    int onuid = onu-1;

    if(ponid == -1 || onuid == -1)
    {
        ret = VOS_ERROR;
        return ret;
    }

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
#endif

    if(pd)
    {
#if 0
        ULONG len = sizeof(ONUCONFSYNDMSG_T)+sizeof(ONUConfigData_t);
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
        if(p)
        {
            p->slotid = slot;
            p->ponid = pon;
            p->onuid = onu-1;
            p->msgtype = MSG_ONU_CONFSYND_REQ_CDP;
            VOS_MemCpy(p->data, pd, sizeof(ONUConfigData_t));

            ret = sendOnuConfSyndMsg(slot, p, len);

        }
        else
            ret = VOS_ERROR;
#else
        char *data = NULL;
        ULONG datalen = onuconf_build_lzma_data((char*)pd, sizeof(ONUConfigData_t), &data);

        if(datalen)
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T)+datalen;
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = slot;
                p->ponid = pon;
                p->onuid = onu-1;
                p->msgtype = MSG_ONU_CONFSYND_REQ_CDP;
                VOS_MemCpy(p->data, data, datalen);
                free(data);

                ret = sendOnuConfSyndMsg(slot, p, len);

            }
            else
                ret = VOS_ERROR;
        }
        else
        	ret = VOS_ERROR;
#endif
    }
    else
        ret = VOS_ERROR;

#ifdef ONUID_MAP
    }
    ONU_CONF_SEM_GIVE
#endif

    return ret;
}
int sendOnuConfSyndOnuIdMapReqMsg(ULONG dstSlot, ULONG ponid)
{
    int ret = VOS_OK;

    if(ponid == -1 )
    {
        ret = VOS_ERROR;
        return ret;
    }

    ONU_CONF_SEM_TAKE
    {

        int slot = GetCardIdxByPonChip(ponid);
        int pon = GetPonPortByPonChip(ponid);

        int length = MAXONUPERPON * sizeof(onuConfOnuIdMapEntry_t);

        ULONG len = sizeof(ONUCONFSYNDMSG_T) + length;
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);

        if (p)
        {
            p->slotid = slot;
            p->ponid = pon;
            p->onuid = 0;
            p->msgtype = MSG_ONU_CONFSYND_ONUID_MAP_REQ_CDP;
            VOS_MemCpy(p->data, &g_onuConfOnuIdMapArry[ponid][0], length);

            ret = sendOnuConfSyndMsg(dstSlot, p, len);

        }
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int
sendOnuConfSyndUndoAssociationByOnuIdMsg(USHORT slot, USHORT pon, USHORT onu)
{

    int ret = VOS_OK;

    ULONG len = sizeof(ONUCONFSYNDMSG_T);

    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
    if (p)
    {
        p->slotid = slot;
        p->ponid = pon;
        p->onuid = onu;
        p->msgtype = MSG_ONU_CONFSYND_UNDO_ASSOCIATION_BY_ONUID;

        sendOnuConfSyndMsg(slot, p, len);

    }
    else
        ret = VOS_ERROR;

    return ret;
}

int
sendOnuConfSyndUndoAssociationByNameMsg(USHORT slot, char *name, int nlen)
{

    int ret = VOS_OK;

    ULONG len = sizeof(ONUCONFSYNDMSG_T)+nlen;

    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
    if (p)
    {
        VOS_MemZero(p, len);
        p->slotid = slot;
        p->ponid = 0;
        p->onuid = 0;
        p->msgtype = MSG_ONU_CONFSYND_UNDO_ASSOCIATION_BY_NAME;
        VOS_MemCpy(p->data, name, nlen);

        sendOnuConfSyndMsg(slot, p, len);

    }
    else
        ret = VOS_ERROR;

    return ret;
}

int sendOnuConfSyndDelReqMsg(int slot, const char *name)
{
    ULONG ulArgs[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_DEL_CDP, 0, 0};
    ULONG len = sizeof(ONUCONFSYNDMSG_T) + VOS_StrLen(name), queid = 0;
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
    if (p)
    {
        p->slotid = slot;
        p->ponid = 0;
        p->onuid = 0;
        p->msgtype = MSG_ONU_CONFSYND_DEL_CDP;
        VOS_StrCpy((char*)p->data, name);
#if 1
        /*return sendOnuConfSyndMsg(slot, p, len);*/
        ulArgs[2] = len;
        ulArgs[3] = (ULONG)p;
        queid = getQueIdBySlotno(slot);
        return VOS_QueSend(queid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
#else
        if(VOS_SemTake(getSemIdBySlotno(slot), VOS_TICK_SECOND*s_semTimeOutSeconds) == VOS_OK)
        {
            sendOnuConfSyndMsg(slot, p, len);
            if(VOS_SemTake(getSemIdBySlotno(slot), VOS_TICK_SECOND*s_semTimeOutSeconds) == VOS_OK)
            {
                VOS_SemGive(getSemIdBySlotno(slot));
                return VOS_OK;
            }
            else
                return VOS_ERROR;
        }
        else
        	return VOS_ERROR;
#endif
    }
    else
        return VOS_ERROR;
}
void startOnuConfSyndBroadcastDelMsg(const char *name)
{

    int i;

    for(i=1;i<=SYS_CHASSIS_SLOTNUM;i++)
    {
        if(SYS_MODULE_IS_PONCARD_MANAGER(i))
        {
            /*
            int len = VOS_StrLen(name);
            char *parg = VOS_Malloc(len+1, MODULE_RPU_CTC);
            if(parg)
            {
                VOS_StrCpy(parg, name);
            }
            ulArgs[2] = (ULONG)parg;
            ulArgs[3] = i;
            queid = getQueIdBySlotno(i);
            VOS_QueSend(queid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
            */
            sendOnuConfSyndDelReqMsg(i,  name);
        }
    }
}
void startOnuConfSyndBroadcastUndoAssociationByNameMsg(const char *name)
{

    int i;
    ULONG queid;
    ULONG ulArgs[4] =
        { MODULE_RPU_CTC, MSG_ONU_CONFSYND_UNDO_ASSOCIATION_BY_NAME, 0, 0 };

    for(i=1;i<=SYS_CHASSIS_SLOTNUM;i++)
    {
        if(SYS_MODULE_IS_PON(i))
        {
            int len = VOS_StrLen(name);
            char *parg = VOS_Malloc(len+1, MODULE_RPU_CTC);
            if(parg)
            {
                VOS_StrCpy(parg, name);
            }
            ulArgs[2] = (ULONG)parg;
            ulArgs[3] = i;
            queid = getQueIdBySlotno(i);
            VOS_QueSend(queid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
        }
    }
}
#endif

void setOtherCtcOnuRegistrationForAllPonCard(ULONG v)
{
    int i;
    ULONG queid;
    ULONG ulArgs[4] =
        { MODULE_RPU_CTC, MSG_ONU_OTHER_VENDOR_REGISTRATION_CDP, 0, 0 };

    ulArgs[3] = v;

    for(i=1;i<=SYS_CHASSIS_SLOTNUM;i++)
    {
        if(SlotCardIsPonBoard(i) == VOS_OK)
        {
            ulArgs[2] = i;
            queid = getQueIdBySlotno(i);
            VOS_QueSend(queid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
        }
    }
}
#if 0
int sendOnuConfMacSyndReqMsg(USHORT slot, const char *mac, const char *name)
{
    int ret = VOS_OK;
    ULONG len = sizeof(ONUCONFSYNDMSG_T) + sizeof(onuConfMacHashEntry_t);
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);

    if(VOS_StrLen(name) >= ONU_CONFIG_NAME_LEN)
        ret = VOS_ERROR;
    else if (p)
    {
        onuConfMacHashEntry_t *pd = (onuConfMacHashEntry_t*)p->data;
        VOS_MemSet(pd, 0, sizeof(onuConfMacHashEntry_t));
        VOS_StrCpy(pd->name, name);
        VOS_MemCpy(pd->mac, mac, 6);
        p->slotid = slot;
        p->ponid = 0;
        p->onuid = 0;
        p->msgtype = MSG_ONU_CONFSYND_MAC_REQ_CDP;
        VOS_MemCpy(p->data, pd, sizeof(onuConfMacHashEntry_t));

#if 1
        sendOnuConfSyndMsg(slot, p, len);
#else
        if(VOS_SemTake(getSemIdBySlotno(slot), VOS_TICK_SECOND*s_semTimeOutSeconds) == VOS_OK)
        {
            sendOnuConfSyndMsg(slot, p, len);
            if(VOS_SemTake(getSemIdBySlotno(slot), VOS_TICK_SECOND*s_semTimeOutSeconds) == VOS_OK)
            {
                VOS_SemGive(getSemIdBySlotno(slot));
                ret = VOS_OK;
            }
            else
            {
                cl_vty_console_out("\r\n------ get ack timeout ------\r\n");
                ret = VOS_ERROR;
            }
        }
        else
        {
            cl_vty_console_out("\r\n------ take semphere timeout ------\r\n");
            ret = VOS_ERROR;
        }
#endif

    }
    else
        ret = VOS_ERROR;

    return ret;
}

int sendOnuConfSyndReqAckMsg(int slot, int result)
{
    ULONG len = sizeof(ONUCONFSYNDMSG_T);
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
    if (p)
    {
        p->slotid = slot;
        p->ponid = 0;
        p->onuid = 0;
        p->msgtype = MSG_ONU_CONFSYND_REQ_ACK_CDP;
        p->data[0] = result;
        return sendOnuConfSyndMsg(SYS_MASTER_ACTIVE_SLOTNO, p, len);
    }
    else
        return VOS_ERROR;
}

int sendOnuConfMacToAllPonBoard(const char *mac, const char* name)
{
    int i, ret = 0;

    for(i=1;i<=SYS_CHASSIS_SLAVE_SLOTNUM;i++)
        if(SYS_MODULE_IS_PON(i))
            ret |= sendOnuConfMacSyndReqMsg(i, mac, name);

    return ret?VOS_ERROR:VOS_OK;
}

int sendOnuConfSyndBroadcastReqMsg(const char *name)
{

        int i,ret=0;

        for (i = 1; i <= SYS_CHASSIS_SLOTNUM; i++)
        {
            if (SYS_MODULE_IS_PON(i))
            {
                ret |= sendOnuConfSyndReqMsgByName(i, name);
            }
        }
        return ret ? VOS_ERROR : VOS_OK;

}

int sendOnuConfExecuteMsg(USHORT slot, USHORT pon, USHORT onu, int act)
{

    ULONG len = sizeof(ONUCONFSYNDMSG_T);
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
    if (p)
    {
        p->slotid = slot;
        p->ponid = pon;
        p->onuid = onu - 1;
        p->msgtype = MSG_ONU_CONFSYND_EXECUTE_CDP;
        p->data[0] = act;

        return sendOnuConfSyndMsg(slot, p, len);

    }
    else
        return VOS_ERROR;
}
#endif
int sendOnuConfReportMsg(USHORT slot, USHORT pon, USHORT onu, char * name, int rcode)
{

	short int lponid = GetPonPortIdxBySlot(slot, pon);
	short int ponportidx = GetGlobalPonPortIdxBySlot(slot, pon);
    int ret = VOS_OK;
	int partner_slot, partner_port, partner_ponid;

#ifndef ONUID_MAP
	int onuEntry = ponportidx*MAXONUPERPON+onu-1;
#endif

	/*如果当前PON口处于保护倒换，则使用较小的PON ID*/
	if(PonPortSwapSlotQuery(lponid, &partner_slot, &partner_port) == VOS_OK)
	{
	    partner_ponid = GetGlobalPonPortIdxBySlot(partner_slot, partner_port);

	    if(partner_ponid < ponportidx)
	        ponportidx = partner_ponid;
	}

	{
	    ULONG len = sizeof(ONUCONFSYNDMSG_T)+sizeof(int)+ONU_CONFIG_NAME_LEN;

	    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
	    if (p)
	    {
	        p->slotid = slot;
	        p->ponid = pon;
	        p->onuid = onu;
	        p->msgtype = MSG_ONU_CONFSYND_RESULT_REPORT_CDP;
			p->data[0] = rcode;
			
#ifdef ONUID_MAP
			ONU_CONF_SEM_TAKE
			{
			    VOS_StrCpy((char*)(p->data+1), name);
			}
			ONU_CONF_SEM_GIVE
#else
			ONU_MGMT_SEM_TAKE
				VOS_StrCpy((char*)(p->data+1), OnuMgmtTable[onuEntry].configFileName);
			ONU_MGMT_SEM_GIVE;
#endif
            /*Q.21083  集中式设备，状态要同步到备用主控，之前一直存在bug。 2014-6-11*/
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
            {
        		LONG to_slotno = device_standby_master_slotno_get();
        		if( to_slotno != 0 )                
	                ret = sendOnuConfSyndMsg(to_slotno, p, len);
            }
            else
            {
	            ret = sendOnuConfSyndMsg(SYS_MASTER_ACTIVE_SLOTNO, p, len);
            }
            return ret;
	    }
	    else
	        return VOS_ERROR;
	}

	return VOS_OK;
}

int sendOnuConfDebugReportMsg(USHORT dstslot, USHORT srcslot, int len, char *text)
{

    ULONG slen = sizeof(ONUCONFSYNDMSG_T)+len+4;
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(slen, MODULE_RPU_CTC);
    if (p)
    {
        p->slotid = srcslot;
        p->ponid = 0;
        p->onuid = 0;
        p->msgtype = MSG_ONU_CONFSYND_DEBUG_REPORT_CDP;
        VOS_MemCpy(p->data, text, len);

        if(srcslot != dstslot)
            return sendOnuConfSyndMsg(dstslot, p, slen);
        else
        {
            ULONG ulargs[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_DEBUG_REPORT_CDP, 0, 0};
            ULONG queid = getQueIdBySlotno(dstslot);

            ulargs[3] = (ULONG)p;
            return VOS_QueSend(queid, ulargs, WAIT_FOREVER, MSG_PRI_NORMAL);
        }

    }
    else
        return VOS_ERROR;
}

int sendOnuConfSyncApplyMsg(const char * filename, ULONG slot, ULONG pon, ULONG onuid)
{
    int len = VOS_StrLen(filename);
    ULONG slen = sizeof(ONUCONFSYNDMSG_T)+ONU_CONFIG_NAME_LEN;
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(slen, MODULE_RPU_CTC);
    if (p)
    {
        VOS_MemZero(p, slen);
        p->slotid = slot;
        p->ponid = pon;
        p->onuid = onuid;
        p->msgtype = MSG_ONU_CONFSYND_APPLY_REQ;
        VOS_MemCpy(p->data, filename, len);

        return sendOnuConfSyndMsg(slot, p, slen);

    }
    else
        return VOS_ERROR;
}
#if 0
int sendOnuConfSyncRenameMsg(ULONG slot, const char * filename, const char *rename)
{
    int len = 2*ONU_CONFIG_NAME_LEN;
    ULONG slen = sizeof(ONUCONFSYNDMSG_T)+len;
    ONUCONFSYNDMSGPTR_T p = VOS_Malloc(slen, MODULE_RPU_CTC);
    if (p)
    {
        ULONG queid = 0;
        ULONG ulArgs[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_RENAME_REQ_CDP, 0, 0};

        p->slotid = slot;
        p->msgtype = MSG_ONU_CONFSYND_RENAME_REQ_CDP;
        VOS_MemCpy(p->data, filename, ONU_CONFIG_NAME_LEN);
        VOS_MemCpy(((char*)p->data)+ONU_CONFIG_NAME_LEN, rename, ONU_CONFIG_NAME_LEN);

        queid = getQueIdBySlotno(slot);
        ulArgs[2] = slen;
        ulArgs[3] = (ULONG)p;

        return VOS_QueSend(queid, ulArgs,WAIT_FOREVER, MSG_PRI_NORMAL);

    }
    else
        return VOS_ERROR;
}

int startOnuConfSyndReqByCard(int slotid)
{

    ULONG queid;
    ULONG ulArgs[4] = {MODULE_RPU_CTC, MSG_ONU_CONFSYND_CARD_REQ, 0, 0};
	/*ULONG semid = getSemIdBySlotno(slotid);*/
    ulArgs[3] = slotid;

	if(!(SYS_LOCAL_MODULE_ISMASTERACTIVE && SYS_MODULE_ISHAVECPU(SYS_MODULE_TYPE(slotid))))
		return VOS_ERROR;

    if( ( slotid == SYS_LOCAL_MODULE_SLOTNO ) || (!(SYS_MODULE_IS_PON(slotid) || SYS_MODULE_ISMASTERSTANDBY(slotid))) )
        return VOS_ERROR;

    queid = getQueIdBySlotno(slotid);

    /*if(VOS_SemTake(semid, ONU_CONF_CARD_SYNC_WAIT_TIME*VOS_TICK_SECOND) != VOS_OK)
    {
    	VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "card_insert_callback_sync_onuconf (slot %d) sem take fail\r\n", slotid);
    	return VOS_ERROR;
    }
    else
    */
    if(onuconf_wait_cardsyncfinished_signal(slotid, 1) != VOS_OK)
    {
    	VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "startOnuConfSyndReqByCard (slot %d) sem take fail\r\n", slotid);
    	return VOS_ERROR;
    }
    else
    {
    	ULONG semid = getSemIdBySlotno(slotid);
    	ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP, ("card sync begin: slot %d, ticks: %u, semid:%u\r\n", slotid, VOS_GetTick(), semid));
        return VOS_QueSend(queid, ulArgs,WAIT_FOREVER, MSG_PRI_NORMAL);
    }

}
#endif
int g_onuConfLoad = 0;

int onuconf_devsm_card_register_request_callback(int slotid)
{
    if(!g_onuConfLoad)
    {
        initDefaultOnuConfFile();
        loadOnuConfFile();
        g_onuConfLoad = 1;
    }
    return VOS_OK;
}

void startOnuConfSyndBroadcastRequest()
{
    int icard = 0;

    ONU_CONF_SEM_TAKE
    s_onuConfBroadcastCardMask = 0;
    ONU_CONF_SEM_GIVE

    for(icard=1; icard <= SYS_CHASSIS_SLOTNUM; icard++)
        if(SYS_MODULE_IS_PON(icard) || SYS_MODULE_ISMASTERSTANDBY(icard))
            startOnuConfSyndReqByCard(icard);
 
}

void setOnuConfSyndBroadcastCardMask(int slot)
{
    int icard = 0;

    for(icard=1; icard <= SYS_CHASSIS_SLOTNUM; icard++)
    {
        if(icard == slot && SYS_MODULE_IS_PON(icard))
        {
            ONU_CONF_SEM_TAKE
            s_onuConfBroadcastCardMask |= 1<<(icard-1);
            ONU_CONF_SEM_GIVE
        }
    }

}

void endOnuConfSyndBroadcastReques()
{
    int icard = 0, ctrl = 1;
    ULONG mask = 0;

    while(ctrl)
    {
        for(icard=1; icard <= SYS_CHASSIS_SLOTNUM; icard++)
        {
            if(SYS_MODULE_IS_PON(icard))
            {
                ONU_CONF_SEM_TAKE
                mask = s_onuConfBroadcastCardMask;
                ONU_CONF_SEM_GIVE
                if(!(mask & (1<<(icard-1))))
                    break;
            }
        }

        if(icard > SYS_CHASSIS_SLOTNUM)
            ctrl = 0;
        VOS_TaskDelay(10);
    }

    ONU_CONF_SEM_TAKE
    s_onuConfBroadcastCardMask = 0;
    ONU_CONF_SEM_GIVE
}

int sendOnuAlarmConfSyndDataBySlot(USHORT slot)
{

	int ret = VOS_OK;
    ULONG length = sizeof(eventCtcSyncRecoverMsg_t);
    eventCtcSyncRecoverMsg_t * alconfig = (eventCtcSyncRecoverMsg_t *)VOS_Malloc(length , MODULE_RPU_CTC);
    if(!alconfig)
    {
        VOS_ASSERT(0);
        return VOS_ERROR;
    }
    VOS_MemZero(alconfig,length);
    if(Build_recoverData_func(alconfig) != VOS_OK)
    {
        VOS_Free(alconfig);
        return VOS_ERROR;
    }
    if(alconfig)
    {
        ULONG len = sizeof(ONUCONFSYNDMSG_T)+sizeof(eventCtcSyncRecoverMsg_t);
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
        if(p)
        {
            p->slotid = slot;
            p->ponid = 0;
            p->onuid = 0;
            p->msgtype = MSG_ONU_CONFSYND_ALARM_DATA_CDP;
            VOS_MemCpy(p->data, alconfig, sizeof(eventCtcSyncRecoverMsg_t));
            ret = sendOnuConfSyndMsg(slot, p, len);
        }
        else
           ret = VOS_ERROR;
        VOS_Free(alconfig);
    }
    else
        ret = VOS_ERROR;
    return ret;
}

#if 0
int sendOnuConfSyndReqMsgByName(USHORT slot, const char *name)
{

	int ret = VOS_OK;

    ONUConfigData_t * pd = getOnuConfFromHashBucket(name);

    if(pd)
    {
#if 0
        ULONG len = sizeof(ONUCONFSYNDMSG_T)+sizeof(ONUConfigData_t);
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
        if(p)
        {

            p->slotid = slot;
            p->ponid = 0;
            p->onuid = 0;
            p->msgtype = MSG_ONU_CONFSYND_REQ_CDP;
            VOS_MemCpy(p->data, pd, sizeof(ONUConfigData_t));
            ret = sendOnuConfSyndMsg(slot, p, len);

        }
        else
           ret = VOS_ERROR;
#else
        char *data = NULL;
        ULONG datalen = onuconf_build_lzma_data((char*)pd, sizeof(ONUConfigData_t), &data);

        if(datalen)
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T)+datalen;
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = slot;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_REQ_CDP;
                VOS_MemCpy(p->data, data, datalen);
                free(data);

                ret = sendOnuConfSyndMsg(slot, p, len);

            }
            else
                ret = VOS_ERROR;
        }
        else
        	ret = VOS_ERROR;
#endif
    }
    else
        ret = VOS_ERROR;

    return ret;
}

int sendOnuConfToAllPonBoard(const char *name)
{
   int i, ret = 0;

   for(i=1; i<=SYS_CHASSIS_SLOTNUM; i++)
       if(SYS_MODULE_IS_PON(i))
           ret |= sendOnuConfSyndReqMsgByName(i, name);

   return ret?VOS_ERROR:VOS_OK;
}
void deleteAllDisusedConfigFile()
{
    int i = 0;
    ONU_CONF_SEM_TAKE
    {
        for(i=0; i<g_maxPrimeNumber; i++)
        {
            element_onuconfhashbucket_t *pe = (element_onuconfhashbucket_t*) g_onuConfBucket[i];
            while (pe)
            {
                ONUConfigData_t * pd = (ONUConfigData_t *) pe->confdata;
                if(!onuconfHaveAssociatedOnu(pd->confname))
                {
                    deleteOnuConfFromHashBucket(pd->confname);
                    if(SYS_LOCAL_MODULE_ISMASTERACTIVE && (!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER))
                        startOnuConfSyndBroadcastDelMsg(pd->confname);
                }
                pe = pe->next;
            }
        }
    }
    ONU_CONF_SEM_GIVE
}

int startOnuConfSyndRequestWitchDstSlot(int slotno)
{
    int i = 0;

    /*added by luh 2012-8-20增加透传命令行在热插拔时的恢复*/
    if(ONU_TRANSMISSION_ENABLE != OnuTransmission_flag)
        ResumeOnuTransmissionflagBySlot(slotno, OnuTransmission_flag);
#ifdef CDP_DEBUG
    BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
    cl_vty_console_out("\r\n------broadcasting onu conf files to slot %d------\r\n", slotno);
    END_ONUCONF_DEBUG_PRINT()
#endif
    /*for (j = 0; j < 1000; j++)*/
    {
        for (i = 0; i < g_maxPrimeNumber; i++)
        {
            ONU_CONF_SEM_TAKE
            {
                element_onuconfhashbucket_t *pe = (element_onuconfhashbucket_t*) g_onuConfBucket[i];

                while (pe)
                {
                    ONUConfigData_t * pd = (ONUConfigData_t *) pe->confdata;
                    sendOnuConfSyndReqMsgByName(slotno, pd->confname);
                    pe = pe->next;
                }
            }
            ONU_CONF_SEM_GIVE

        }
        /*cl_vty_console_out("\r\n------%d bucket send complete!------\r\n", j+1);*/
    }

#ifdef CDP_DEBUG
    BEGIN_ONUCONF_DEBUG_PRINT(ONU_CONF_DEBUG_LVL_CDP)
    cl_vty_console_out("\r\n------broadcast onu conf files to slot %d done!------\r\n", slotno);
    END_ONUCONF_DEBUG_PRINT()
#endif

#ifndef ONUID_MAP

#ifdef CDP_DEBUG
    cl_vty_console_out("\r\n------broadcast onu mac-conf map to slot %d------\r\n", slotno);
#endif

    for (i = 0; i < g_maxPrimeNumber; i++)
    {
        element_onuconfhashbucket_t *pe = (element_onuconfhashbucket_t*) g_onuConfBucket[i];
#if 0
        if (pe)
            sys_console_printf("broadcast config-mac from hash bucket %d on slot %d\r\n", i + 1, slotno);
#endif
        while (pe)
        {
            onuConfMacHashEntry_t *pd = pe->confdata;
            sendOnuConfMacSyndReqMsg(slotno, pd->mac, pd->name);
            pe = pe->next;
        }
    }

#ifdef CDP_DEBUG
    cl_vty_console_out("\r\n------broadcast onu mac-conf map to slot %d done!------\r\n", slotno);
#endif

#else

    for(i=0; i<MAXPON; i++)
    {
        sendOnuConfSyndOnuIdMapReqMsg(slotno, i);
    }

#endif
    sendOnuAlarmConfSyndDataBySlot(slotno);

    return VOS_OK;
}
#endif

/*
 * CDP通道的回调函数
 * 在PON板中将配置同步信息分发到PON的同步任务队列中
 * 在主控板板侧要根据同步信息中的板信息将同步信息分发到对应的同步任务队列中
*/

void onuConfSyndCdpCallback(ULONG ulFlag, ULONG ulSrcPath, ULONG ulDstSlot, ULONG ulDstPath, VOID *pData, ULONG ulLen)
{

    if(pData)
    {
        ONUCONFSYNDMSGPTR_T pmsg = (ONUCONFSYNDMSGPTR_T) pData;
        ULONG ulArgMsg[4] = { MODULE_RPU_CTC, 0, 0, 0 };

        ULONG queid = getQueIdBySlotno(pmsg->slotid);

        switch (ulFlag)
        {

        case CDP_NOTI_FLG_RXDATA:
            ulArgMsg[1] = pmsg->msgtype;
            ulArgMsg[2] = ulLen;
            ulArgMsg[3] = (ULONG) pData;

            if(VOS_QueSend(queid, ulArgMsg, 5*VOS_TICK_SECOND, MSG_PRI_NORMAL) != VOS_OK)
            {
            	CDP_FreeMsg(pData);
                ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP, ("send onu config sync callback msg fail!\r\n"));
            }
            break;
        case CDP_NOTI_FLG_SEND_FINISH:
            CDP_FreeMsg(pData);
            break;
        default:
            CDP_FreeMsg(pData);
            break;
        }
    }
    else
        VOS_ASSERT(pData);

}

int onuconf_syncStandbyMaster(void *pd)
{
    int i=0;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for(i=1; i <= SYS_CHASSIS_MASTER_SLOTNUM; i++)
            if(SYS_CHASSIS_MASTER_SLOTNO(i) != SYS_LOCAL_MODULE_SLOTNO)
                /*return startOnuConfSyndRequestWitchDstSlot(SYS_CHASSIS_MASTER_SLOTNO(i));*/
                return startOnuConfSyndReqByCard(SYS_CHASSIS_MASTER_SLOTNO(i));
    }

        return VOS_ERROR;
}

/*
 * ntodo:初始化ONU配置同步过程
 *
 *
*/

int init_onuConfSyndProcedure(void)
{
    int i = 0;

	/*申请内存*/

    if(ONU_MAX_CONF_NUM*sizeof(onuConfDataItem_t) > (0x5000000/2))
    {
        VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "%s", "onu config file operation record table create fail!\r\n");
		sys_console_printf("onu config memory is (%d k)too large!(only reserved 40M)\r\n", (ONU_MAX_CONF_NUM*sizeof(onuConfDataItem_t))/1024);        
    }
	if((sysPhysMemTop() <= 0x10000000))
	{
		g_onuConfDataMem = (onuConfDataItem_t*)g_malloc(ONU_MAX_CONF_NUM*sizeof(onuConfDataItem_t));
	}
	else
	{
		g_onuConfDataMem = (onuConfDataItem_t*)GetOnuProfileMemStart();/*g_malloc(ONU_MAX_CONF_NUM*sizeof(onuConfDataItem_t));*/
	}
	
	if(!g_onuConfDataMem)
	{
		sys_console_printf("onu config memory g_malloc fail!\r\n");
		return VOS_ERROR;
	}
	else
	{
	    memset(g_onuConfDataMem, 0, ONU_MAX_CONF_NUM*sizeof(onuConfDataItem_t));
	}


    g_onuConfHashMem = (onuconfhashitem_t*)g_malloc(ONU_MAX_CONF_NUM*sizeof(onuconfhashitem_t));
    if(!g_onuConfHashMem)
    {
        sys_console_printf("onu config hash item memory g_malloc fail!\r\n");
        return VOS_ERROR;
    }
    else
    {
        memset(g_onuConfHashMem, 0, ONU_MAX_CONF_NUM*sizeof(onuconfhashitem_t));
    }

    g_onuConfMemSemId = VOS_SemMCreate(VOS_SEM_Q_FIFO);
    if(!g_onuConfMemSemId)
    {
        sys_console_printf("onu config memory semaphore init fail!\r\n");
        return VOS_ERROR;
    }

    initOnuConfHashBucket(HASH_BUCKET_DEPTH);

	init_onuconfEventCallbackArray();

	initEventCallbackFunctions();
	if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
	{
	    initAuoOnuProfileCheckTask();
        registerEventCallback(EVENT_ONUPROFILE_CREATE, fcb_onuProfileCreation);
        registerEventCallback(EVENT_ONUPROFILE_MODIFIED, fcb_onuProfileModified);
        registerEventCallback(EVENT_ONUPROFILE_DELETE, fcb_onuProfileDelete);
        g_onuConfFileOpRecord = cl_vector_init(_CL_VECTOR_MIN_SIZE_);
        if(!g_onuConfFileOpRecord)
            VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "%s", "onu config file operation record table create fail!\r\n");
	}
    for(i=0; i<SYS_MAX_PON_PORTNUM; i++)
    {
        int j = 0;
        for(; j<MAXONUPERPONNOLIMIT; j++)
            VOS_StrCpy(g_onuConfOnuIdMapArry[i][j].name, DEFAULT_ONU_CONF);
    }
#if 0
    if (/*SYS_LOCAL_MODULE_WORKMODE_ISMASTER*/(__SYS_MODULE_TYPE__(SYS_LOCAL_MODULE_SLOTNO) == MODULE_E_GFA6900_SW))
#else
    if(SYS_LOCAL_MODULE_TYPE_IS_6900_SW || SYS_LOCAL_MODULE_TYPE_IS_8000_SW)
#endif
    {
        s_onuConfSyndTaskId = VOS_Malloc(sizeof(ULONG) * (MAX_PON_BOARD_NUM+2), MODULE_RPU_CTC);
        s_onuConfSyndQueId = VOS_Malloc(sizeof(ULONG) * (MAX_PON_BOARD_NUM+2), MODULE_RPU_CTC);

        s_onuConfSyncSemId = VOS_Malloc(sizeof(ULONG)*(MAX_PON_BOARD_NUM+2), MODULE_RPU_CTC);

        if (!(s_onuConfSyndTaskId && s_onuConfSyndQueId && s_onuConfSyncSemId))
        {

            if (s_onuConfSyndTaskId)
                VOS_Free(s_onuConfSyndTaskId);

            if (s_onuConfSyndQueId)
                VOS_Free(s_onuConfSyndQueId);

            if(s_onuConfSyncSemId)
                VOS_Free(s_onuConfSyncSemId);

            return VOS_ERROR;
        }

        /*
         * added by wangxiaoyu 2011-08-09
         * 定时同步备主控上的ONU配置数据，防止出现由于RPC失败导致的备主控上配置数据与当前主控不一致的问题
         */
        s_timer_sync_standby_master = VOS_TimerCreate(MODULE_RPU_CTC, 0, g_lBACBatchTimer*1000, onuconf_syncStandbyMaster, NULL, VOS_TIMER_LOOP);
        if(s_timer_sync_standby_master == VOS_ERROR)
            return VOS_ERROR;

        for (i=0; i < MAX_PON_BOARD_NUM+2; i++)
        {

            char szTaskName[20];
            VOS_Sprintf(szTaskName, "tCardSync_%d", i);

            s_onuConfSyndQueId[i] = VOS_QueCreate(2048, VOS_MSG_Q_FIFO);
            s_onuConfSyncSemId[i] = VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_FULL);

            if (!s_onuConfSyndQueId[i])
            {
                sys_console_printf("queue for card %d init fail!\r\n",
                        i);
                s_onuConfSyndTaskId[i] = 0;
                continue;
            }

            s_onuConfSyndTaskId[i] = VOS_TaskCreate(szTaskName, TASK_PRIORITY_HIGH, task_onuConfSynd, &i);

            if (!s_onuConfSyndTaskId[i])
            {
                VOS_QueDelete(s_onuConfSyndQueId[i]);
                s_onuConfSyndQueId[i] = 0;
                sys_console_printf("task for card %d init fail!\r\n",
                        i);
            }
            VOS_QueBindTask(s_onuConfSyndTaskId[i], s_onuConfSyndQueId[i]);

        }
        
    }
    else if(/*SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER*/SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
    {

        char szTaskName[20];


    	/*
    	 * ntodo:配置数据恢复任务、队列初始化调用
    	*/

  		init_onuResConf();

        s_onuConfSyndTaskId = VOS_Malloc(sizeof(ULONG) ,
                        MODULE_RPU_CTC);

        s_onuConfSyndQueId = VOS_Malloc(sizeof(ULONG) ,
                        MODULE_RPU_CTC);
        s_onuConfSyncSemId = VOS_Malloc(sizeof(ULONG), MODULE_RPU_CTC);

        if (!(s_onuConfSyndTaskId && s_onuConfSyndQueId && s_onuConfSyncSemId))
        {

            if (s_onuConfSyndTaskId)
                VOS_Free(s_onuConfSyndTaskId);

            if (s_onuConfSyndQueId)
                VOS_Free(s_onuConfSyndQueId);

            if(s_onuConfSyncSemId)
                VOS_Free(s_onuConfSyncSemId);

            return VOS_ERROR;
        }

        *s_onuConfSyndQueId = VOS_QueCreate(100, VOS_MSG_Q_FIFO);
        if(!*s_onuConfSyndQueId)
        {
            VOS_Free(s_onuConfSyncSemId);
            VOS_Free(s_onuConfSyndQueId);
            VOS_Free(s_onuConfSyndTaskId);
            return VOS_ERROR;
        }

        *s_onuConfSyncSemId = VOS_SemBCreate(VOS_SEM_Q_FIFO, VOS_SEM_FULL);
        if(!*s_onuConfSyncSemId)
        {
            VOS_Free(s_onuConfSyncSemId);
            VOS_Free(s_onuConfSyndQueId);
            VOS_Free(s_onuConfSyndTaskId);
            return VOS_ERROR;
        }

        VOS_StrCpy(szTaskName, "tOnuConfSynd");
        *s_onuConfSyndTaskId = VOS_TaskCreate(szTaskName, TASK_PRIORITY_HIGH, task_onuConfSynd, NULL);
        if(!*s_onuConfSyndTaskId)
        {
            VOS_Free(s_onuConfSyncSemId);
            VOS_Free(s_onuConfSyndQueId);
            VOS_Free(s_onuConfSyndTaskId);
            return VOS_ERROR;
        }
    }

    if(CDP_Create(RPU_TID_CDP_ONU_CONFSYND, CDP_NOTI_VIA_FUNC, 0, onuConfSyndCdpCallback) != VOS_OK)
    {
        return VOS_ERROR;
    }

    g_onuConfSemId = VOS_SemMCreate(VOS_SEM_Q_FIFO/*, VOS_SEM_FULL*/);
    if(!g_onuConfSemId)
        return VOS_ERROR;

    return VOS_OK;

}

int onuDumpConfData(short int ponid, short int onuid, const char *name)
{

    if (isOnuConfExist(name))
        return VOS_ERROR;
    else
    {

        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));

        if (GetOnuOperStatus(ponid, onuid) == ONU_OPER_STATUS_UP)
        {
            ONUConfigData_t *pd = VOS_Malloc(sizeof(ONUConfigData_t), MODULE_RPU_CTC);

            if (pd)
            {
                VOS_MemSet(pd, 0, sizeof(ONUConfigData_t));

                if (p)
                {
                    int slot = GetCardIdxByPonChip(ponid);
                    int port = GetPonPortByPonChip(ponid);

                    ONU_CONF_SEM_TAKE
                    {
                        VOS_MemCpy(pd, p, sizeof(ONUConfigData_t));
                    }
                    ONU_CONF_SEM_GIVE

                    pd->share = 0;
                    VOS_StrCpy(pd->confname, name);

                    OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, pd->confname, NULL, pd);
#ifdef ONUID_MAP
                    ONU_CONF_SEM_TAKE
                    {
#if 1                        
                        OnuProfile_Action_ByCode(OnuMap_Update, 0, ponid, onuid, name, NULL, NULL);
#else
                        setOnuConfOnuIdMapByPonId(ponid, onuid, name);
                        setOnuConfNameMapByPonId(name, ponid, onuid);
#endif                        
                    }
                    ONU_CONF_SEM_GIVE;

#else
                    ONU_MGMT_SEM_TAKE
                    VOS_StrCpy(OnuMgmtTable[onuEntry].configFileName, name);
                    ONU_MGMT_SEM_GIVE;
#endif                    
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, 0, 0, 0, name, NULL, NULL);

                }
                else
                {
                    VOS_Free(pd);
                    return VOS_ERROR;
                }
            }
            else
                return VOS_ERROR;
        }
        else
            return VOS_ERROR;
    }

    return VOS_OK;
}


#define SHOW_ONU_CONF_LIST_FORMAT "%-8d\t%-20s\t%-20s\t%-20s\r\n"
void show_onu_conf_Dataheader(struct vty *vty,short int slot,short int pon)
{
    vty_out(vty, "\r\n");

    vty_out(vty, "pon%d/%d:\r\n", slot, pon);

    vty_out(vty, "%-8s\t%-20s\t%-9s\t%-9s%-18s%s\r\n", "onuid", "name", "share",  "restore", "mac addr/SN", "device name");

}
void show_onu_temp_conf_list(struct vty *vty)
{
    int i=0, c = 0;

    vty_out(vty, "\r\n");

    vty_out(vty, "%-8s\t%-20s\t%-20s\t%-20s\r\n", "idx", "name", "share", "used");

    ONU_CONF_SEM_TAKE
    {
        for(i=0; i<g_maxPrimeNumber; i++)
        {
            element_onuconfhashbucket_t *p = (element_onuconfhashbucket_t*)g_onuConfBucket[i];
            while(p)
            {
                ONUConfigData_t *pd = p->confdata;
                /*问题单16102，临时配置文件不再显示*/
                if(pd && !VOS_StrnCmp(pd->confname, "*auto", 5))
                    vty_out(vty, SHOW_ONU_CONF_LIST_FORMAT, ++c, pd->confname, pd->share?"yes":"no", onuconfHaveAssociatedOnu(pd->confname)?"yes":"no");
                p = p->next;
            }
        }
    }
    ONU_CONF_SEM_GIVE

    vty_out(vty, "\r\n");
}
void show_onu_conf_list(struct vty *vty)
{
    int i=0, c = 0;

    vty_out(vty, "\r\n");

    vty_out(vty, "%-8s\t%-20s\t%-20s\t%-20s\r\n", "idx", "name", "share", "used");

    ONU_CONF_SEM_TAKE
    {
        for(i=0; i<g_maxPrimeNumber; i++)
        {
            element_onuconfhashbucket_t *p = (element_onuconfhashbucket_t*)g_onuConfBucket[i];
            while(p)
            {
                ONUConfigData_t *pd = p->confdata;
                /*问题单16102，临时配置文件不再显示*/
                if(pd && VOS_StrnCmp(pd->confname, "*auto", 5))
                    vty_out(vty, SHOW_ONU_CONF_LIST_FORMAT, ++c, pd->confname, pd->share?"yes":"no", onuconfHaveAssociatedOnu(pd->confname)?"yes":"no");
                p = p->next;
            }
        }
    }
    ONU_CONF_SEM_GIVE

    vty_out(vty, "\r\n");
}

int show_onu_conf_list_by_onu(struct vty *vty, int ponid, int onuid)
{
	int slot = GetCardIdxByPonChip(ponid);
    ONU_CONF_SEM_TAKE
    {
        int onuentry = ponid*MAXONUPERPON+onuid;
        UCHAR *mac = OnuMgmtTable[onuentry].DeviceInfo.MacAddr;
		UCHAR *SN = OnuMgmtTable[onuentry].DeviceInfo.DeviceSerial_No;
        char *devname = OnuMgmtTable[onuentry].DeviceInfo.DeviceName;
        char * name = ONU_CONF_NAME_PTR_GET(ponid, onuid);
        UCHAR code = getOnuConfResult(ponid, onuid);
        /*if(name[0] && ThisIsValidOnu(ponid, i) == VOS_OK)*/
        if(name)
			if(SYS_MODULE_IS_GPON(slot))
			{
				vty_out(vty, "%-8d\t%-20s\t%-9s\t%-09s%16s    %s\r\n", onuid+1, name, onuConfIsShared(ponid, onuid)?"yes":"no",
                    code?(code==ONU_CONF_EXEC_RESULT_OK?"yes":"no"):"null", SN, devname);
			}
			else
			{
            	vty_out(vty, "%-8d\t%-20s\t%-9s\t%-09s%02x%02x.%02x%02x.%02x%02x    %s\r\n", onuid+1, name, onuConfIsShared(ponid, onuid)?"yes":"no",
                    code?(code==ONU_CONF_EXEC_RESULT_OK?"yes":"no"):"null", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], devname);
			}
	}
    ONU_CONF_SEM_GIVE

    return 0;
}

int show_onu_conf_list_by_pon(struct vty *vty, int ponid)
{
    int i=0;

    int slot = GetCardIdxByPonChip(ponid);
    int pon = GetPonPortByPonChip(ponid);
    int MaxOnu = GetMaxOnuByPonPort(ponid)&0xff;


    if(slot == -1 || pon == -1)
    {
        vty_out(vty, "invalid pon port id\r\n");
        return CMD_WARNING;
    }

	show_onu_conf_Dataheader(vty,slot, pon);
    for(i=0; i</*MAXONUPERPON*/MaxOnu; i++)
    {
#ifdef ONUID_MAP
        show_onu_conf_list_by_onu(vty, ponid, i);
#else
        int onuentry = ponid*MAXONUPERPON+i;

        if(OnuMgmtTable[onuentry].configFileName[0] &&
                ThisIsValidOnu(ponid, i) == VOS_OK)
            vty_out(vty, "%-8d\t%-20s\t%-20s\r\n", i+1, OnuMgmtTable[onuentry].configFileName, onuConfIsShared(ponid, i)?"yes":"no");
#endif
    }

    vty_out(vty, "\r\n");

    return CMD_SUCCESS;
}

void show_onu_conf_qosset_vty(struct vty *vty, short int olt_id, short int onu_id, int idx)
{

	ONUConfigData_t *pd = NULL;

	OLT_LOCAL_ASSERT(olt_id);
	ONU_ASSERT(onu_id);

	if(idx < 1 || idx > QOS_MAX_SET_COUNT)
	{
	    vty_out(vty,"\r\ninvalid qos set index (%d)!\r\n", idx);
	    return;
	}

	pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(olt_id, onu_id));

	if(pd)
	{
		ONU_CONF_SEM_TAKE
		{
			int i=0, vc = 0, banner = 0;
			qos_classification_rules_t *qset = &pd->qosset[idx-1];

			qos_classification_rule_t *prule = (qos_classification_rule_t*)qset;

			for(i=0; i<QOS_MAX_RULE_COUNT_PER_SET; i++)
			{
				int j;

				if(prule->valid)
				{
					char vtext[80] = "";

					vc++;

					vty_out(vty, "\r\nclassid %d:\tqueuemap: %d\tprioritymap: %d\r\n", i+1, prule->queue_mapped, prule->priority_mark);

					if(!banner)
					{
						vty_out(vty, "\r\nclassid\tfieldid\t%-20s%-20s%-20s\r\n\r\n", "field", "operation", "value");
						banner++;
					}

					for(j=0; j<QOS_MAX_CLASS_RULE_PAIRS; j++)
					{

						if(prule->entrymask & (1<<(j)))
						{
						    qos_class_rule_entry *pe = &prule->entries[j];
							onuconf_qosRuleFieldValue_ValToStr_parase(pe->field_select, &pe->value, vtext);
							vty_out(vty, "\r\n%-7d\t%-7d\t%-20s%-20s%-20s\r\n", i+1, j+1, qos_rule_field_sel_str(pe->field_select+1),
									qos_rule_field_operator_str(pe->validation_operator+1), vtext );
						}
					}
				}

				prule++;
			}

			if(!vc)
				vty_out(vty, "\r\nno valid rule config!\r\n");
		}
		ONU_CONF_SEM_GIVE
	}
}

void show_onu_conf_qosset_vty_by_ptr(struct vty *vty, int suffix, void *pdata, int idx)
{

    ONUConfigData_t *pd = (ONUConfigData_t*) pdata;

    if (idx < 1 || idx > QOS_MAX_SET_COUNT)
    {
        vty_out(vty, "\r\ninvalid qos set index (%d)!\r\n", idx);
        return;
    }

    ONU_CONF_SEM_TAKE
    {
        if (onuConfCheckByPtr(suffix, pd) == VOS_OK)
        {
            int i = 0, vc = 0, banner = 0;
            qos_classification_rules_t *qset = &pd->qosset[idx-1];

            qos_classification_rule_t *prule = (qos_classification_rule_t*) qset;

            for (i = 0; i < QOS_MAX_RULE_COUNT_PER_SET; i++)
            {
                int j;

                if (prule->valid)
                {
                    char vtext[80] = "";

                    vc++;

                    vty_out(vty, "\r\nclassid %d:\tqueuemap: %d\tprioritymap: %d\r\n", i + 1, prule->queue_mapped,
                            prule->priority_mark);

                    if (!banner)
                    {
                        vty_out(vty, "\r\nclassid\tfieldid\t%-20s%-20s%-20s\r\n\r\n", "field", "operation", "value");
                        banner++;
                    }

                    for (j = 0; j < QOS_MAX_CLASS_RULE_PAIRS; j++)
                    {

                        if (prule->entrymask & (1 << (j)))
                        {
                            qos_class_rule_entry *pe = &prule->entries[j];
                            onuconf_qosRuleFieldValue_ValToStr_parase(pe->field_select, &pe->value, vtext);
                            vty_out(vty, "\r\n%-7d\t%-7d\t%-20s%-20s%-20s\r\n", i + 1, j + 1,
                                    qos_rule_field_sel_str(pe->field_select + 1),
                                    qos_rule_field_operator_str(pe->validation_operator + 1), vtext);
                        }
                    }
                }

                prule++;
            }

            if (!vc)
                vty_out(vty, "\r\nno valid rule config!\r\n");
        }
    }
    ONU_CONF_SEM_GIVE

}

int onu_profile_apply_by_onuid(const char *filename, ULONG slot, ULONG pon, ULONG onuid)
{
    int ret = VOS_ERROR;

    short int pon_id = GetPonPortIdxBySlot(slot, pon);

    if(pon_id >= 0 && pon_id < MAXPON)
    {

        if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER)
        {
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE && !SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER )
                sendOnuConfSyncApplyMsg(filename, slot, pon, onuid);
            /*关联新的配置文件或重新关联文件*/
            /*onuconfAssociateOnuId(pon_id, onuid-1, filename);*/
        }

        if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_MANAGER)
        {
            /*先执行原关联文件的去配置操作*/
            addOnuToRestoreQueue(pon_id, onuid-1, ONU_CONF_RES_ACTION_UNDO, -1);

            /*关联新的配置文件或重新关联文件*/
            /*onuconfAssociateOnuId(pon_id, onuid-1, filename);*/

            /*恢复新的配置文件*/
            addOnuToRestoreQueue(pon_id, onuid-1, ONU_CONF_RES_ACTION_DO, -1);
        }

        ret = VOS_OK;

    }

    return ret;

}

#if 0
#ifndef ONUID_MAP
int onuconfHaveAssociatedOnu(const char *name)
{
    int ret = FALSE;
    int slot = 0, pon=0, onu=0;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for(slot = 1; slot <= SYS_CHASSIS_SLOTNUM; slot++)
        {
            if(SYS_MODULE_IS_PON(slot))
            {
#ifndef ONUID_MAP
                ONU_MGMT_SEM_TAKE
#endif

                for(pon=1; pon<=MAX_PONPORT_PER_BOARD; pon++)
                {
                    for(onu=0; onu<MAXONUPERPON; onu++)
                    {
                        short int ponid = GetGlobalPonPortIdxBySlot(slot, pon);
                        if(ponid != -1)
                        {
                            if(!VOS_StrCmp(name, ONU_CONF_NAME_PTR_GET(ponid, onu)))
                            {
                                ret = TRUE;
                                break;
                            }
                        }
                    }
                    if(onu < MAXONUPERPON)
                        break;
                }

#ifndef ONUID_MAP
                ONU_MGMT_SEM_GIVE
#endif
                if(pon <= MAX_PONPORT_PER_BOARD)
                     break;
            }
        }
    }
    else
    {
#ifndef ONUID_MAP
        ONU_MGMT_SEM_TAKE
#endif
        for(pon=1; pon<=MAX_PONPORT_PER_BOARD; pon++)
        {
            for(onu=0; onu<MAXONUPERPON; onu++)
            {
                short int ponid = GetPonPortIdxBySlot(SYS_LOCAL_MODULE_SLOTNO, pon);
                if(ponid != -1)
                {
                    if(!VOS_StrCmp(name, ONU_CONF_NAME_PTR_GET(ponid, onu)))
                    {
                        ret = TRUE;
                        break;
                    }
                }
            }
            if(onu < MAXONUPERPON)
                break;
        }
#ifndef ONUID_MAP
        ONU_MGMT_SEM_GIVE
#endif
    }

    return ret;
}
#else

int onuconfHaveAssociatedOnu(const char *name)
{
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(name);

        if(pd)
        {
            int i = 0, j=0 , n = MAXONUPERPON/8+1;
            for(i=0; i<SYS_MAX_PON_PORTNUM;i++)
                for(j=0; j<n; j++)
                    if(pd->onulist[i][j])
                    {
                        ONU_CONF_SEM_GIVE
                        return TRUE;
                    }

        }
    }
    ONU_CONF_SEM_GIVE

    return FALSE;
}

#endif

#ifdef ONUID_MAP
int onuconfUndoAssociationByName(const char *name)
{
    int ret = VOS_OK;

    if(name)
    {
        ONU_CONF_SEM_TAKE
        {
            ONUConfigData_t *pd = getOnuConfFromHashBucket(name);
            if(pd)
            {
                int i,j;

                for(i=0; i<SYS_MAX_PON_PORTNUM; i++)
                {
                    for(j=0; j<MAXONUPERPON; j++)
                    {
                        int offset = j/8;
                        int bitnum = j%8;
                        if(pd->onulist[i][offset]&(1<<bitnum))
                        {
                            setOnuConfOnuIdMapByPonId(i, j, DEFAULT_ONU_CONF);
                            /*setOnuConfOnuIdMapByPonId(DEFAULT_ONU_CONF, i, j);*/
                            setOnuConfNameMapByPonId(DEFAULT_ONU_CONF, i, j);
                            pd->onulist[i][offset] &= ~(1<<bitnum);
                        }
                    }
                }
            }
            else
                ret = VOS_ERROR;
        }
        ONU_CONF_SEM_GIVE
    }
    else
        ret = VOS_ERROR;

    return ret;
}
#else
void onuconfUndoAssociationByName(const char *name)
{
    int slot = 0, pon=0, onu=0;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for(slot = 1; slot <= SYS_CHASSIS_SLOTNUM; slot++)
        {
            if(SYS_MODULE_IS_PON(slot))
            {
                ONU_MGMT_SEM_TAKE
                for(pon=1; pon<=MAX_PONPORT_PER_BOARD; pon++)
                {
                    for(onu=0; onu<MAXONUPERPON; onu++)
                    {
                        short int ponid = GetPonPortIdxBySlot(slot, pon);
                        if(ponid != -1)
                        {
                            onuconfAssociateOnuId(ponid, onu, DEFAULT_ONU_CONF);
                            if(!VOS_StrCmp(name, ONU_CONF_NAME_PTR_GET(ponid, onu)))
                            {
                                onuconfAssociateOnuId(ponid, onu, DEFAULT_ONU_CONF);
                            }
                        }
                    }
                }
                ONU_MGMT_SEM_GIVE
            }
        }
    }
    else
    {
        ONU_MGMT_SEM_TAKE
        for(pon=1; pon<=MAX_PONPORT_PER_BOARD; pon++)
        {
            for(onu=0; onu<MAXONUPERPON; onu++)
            {
                short int ponid = GetPonPortIdxBySlot(SYS_LOCAL_MODULE_SLOTNO, pon);
                if(ponid != -1)
                {
                    onuconfAssociateOnuId(ponid, onu, DEFAULT_ONU_CONF);
                    if(!VOS_StrCmp(name, ONU_CONF_NAME_PTR_GET(ponid, onu)))
                    {
                        onuconfAssociateOnuId(ponid, onu, DEFAULT_ONU_CONF);
                    }
                }
            }
        }
        ONU_MGMT_SEM_GIVE
    }

}
#endif
#endif

void onu_conf_card_sync_wait(ULONG slotno, ULONG seconds)
{

	if(!(SYS_LOCAL_MODULE_ISMASTERACTIVE && SYS_MODULE_ISHAVECPU(SYS_MODULE_TYPE(slotno))))
		return;

	if(SYS_MODULE_ISMASTERSTANDBY(slotno) || SlotCardIsPonBoard(slotno) == ROK)
	{
		ULONG wtime = (seconds > 0)?seconds:ONU_CONF_CARD_SYNC_WAIT_TIME;
		ULONG semid = getSemIdBySlotno(slotno);
		ULONG ticks = VOS_GetTick(), ticks1=0;

		ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP, ("card sync wait slot %d\r\n", slotno));
		ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP, ("onu_conf_card_sync_wait sem id :%u\r\n", semid));
		/*
		if(VOS_SemTake(semid, wtime*VOS_TICK_SECOND) == VOS_OK)
		{
			VOS_SemGive(semid);
		}
		*/
		if(  onuconf_wait_cardsyncfinished_signal(slotno, 0) == VOS_OK)
			onuconf_signal_cardsyncfinished(slotno);
		else
		{
			/*强制释放信号量*/
			/*VOS_SemGive(semid);*/
			onuconf_signal_cardsyncfinished(slotno);
			VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "onu_conf_card_sync_wait (slot %d) sem take fail\r\n", slotno);
		}
		ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP,("card sync wait ok slot %d\r\n", slotno));
		ticks1 = VOS_GetTick();
		ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_CDP,("card sync takes %d ticks\r\n", ticks1-ticks));
	}
}
extern int vty_quit_flag;

int cli_onuDumpConfigForOnuNodeVty(struct vty * vty, ULONG ulSlot, ULONG ulPort, ULONG ulOnuId)
{
    int ret = VOS_OK;
    int ponid = GetPonPortIdxBySlot(ulSlot, ulPort);

    if (ponid != -1&&SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        char sz[80]="";
        char *name = getOnuConfNamePtrByPonId(ponid, ulOnuId - 1);
        int share = 0;

        VOS_Sprintf(sz, "onu%d/%d/%d", ulSlot, ulPort, ulOnuId);

        /*如果是恢复配置数据过程，则不再生成临时的私有文件*/
        ONU_CONF_SEM_TAKE
        {
            if(getOnuConfRestoreFlag(ponid, ulOnuId-1))
            {
                ONU_CONF_SEM_GIVE
                return ret;
            }
            ONU_CONF_SEM_GIVE
        }

        if (onuConfIsShared(ponid, ulOnuId - 1))
        {

            /*ONUConfigData_t *pd = VOS_Malloc(sizeof(ONUConfigData_t), MODULE_RPU_ONU);*/
            ONUConfigData_t *pd = openOnuConfigFile(sz, ONU_CONF_OPEN_FLAG_WRITE);

            if (pd)
            {
                ONU_CONF_SEM_TAKE
                {
                    /*2012-11-15，共享文件的权限不需要独享，只需要获取私有文件权限即可*/
                    ONUConfigData_t *p = getOnuConfFromHashBucket(name);
                    if(p)
                    {
                        /*modified by luh 2012-11-21; 解决关联共享配置文件的onu，可以有2个vty同时进入onu节点的问题*/
                        onuconfCopy(pd, p);/*VOS_MemCpy(pd, p, sizeof(ONUConfigData_t));*/                        
                        VOS_MemZero(pd->onulist, sizeof(pd->onulist));
                        VOS_MemZero(pd->confname, sizeof(pd->confname));
                        VOS_StrCpy(pd->confname, sz);
                        pd->share = share;

                        /*纪录当前共享文件的指针以便退出ONU命令节点时需要恢复原来的映射关系时使用*/
                        if(vty)
                            vty->onuconfptr = p;
                    }
                    else
                    {
                        ONU_CONF_SEM_GIVE
                        if(vty)
                        vty_out(vty, "dump private config file fail!\r\n");
                        ret = VOS_ERROR;
                        closeOnuConfigFile(pd->confname);
                        return ret;
                    }
                    ONU_CONF_SEM_GIVE
                }
                /*update onu-map to private profile*/
                OnuProfile_Action_ByCode(OnuMap_Update, 0, ponid, ulOnuId-1, sz, NULL, NULL);
                if(SYS_LOCAL_MODULE_ISMASTERACTIVE/* && SYS_MODULE_IS_PON(ulSlot)*/)
                {
                    int swap_slot, swap_port;
                    int to_slot = 0;
                    OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, ulSlot, ponid, ulOnuId-1, sz, NULL, NULL);
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, 0, 0, 0, sz, NULL, NULL);
                    to_slot= device_standby_master_slotno_get();
                    if(to_slot)
                    {
                        OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, ponid, ulOnuId-1, sz, NULL, NULL);
                    }
                    /*sendOnuConfSyndReqMsg(ulSlot, ulPort, ulOnuId);*/

                    /*
                                 * added by wangxiaoyu 2011-08-29
                                 * PON端口保护时，两个PON端口上相同ONU ID的配置关联同时被更新
                                 * */
                }
            }
            else
                ret = VOS_ERROR;

        }
        else if(!onuConfIsShared(ponid, ulOnuId - 1) && SYS_LOCAL_MODULE_ISMASTERACTIVE) /*备份配置文件内容方便退出时比较有无修改*/
        {

            ONUConfigData_t *pd = NULL;
            VOS_MemZero(sz, sizeof(sz));
            VOS_Sprintf(sz, "*auto%s", name);
			ONUCONF_DEBUG("\r\n cli_onuDumpConfigForOnuNodeVty(%d,%d,%d) open: %s \r\n", ulSlot, ulPort, ulOnuId, sz);
            pd = openOnuConfigFile(sz, ONU_CONF_OPEN_FLAG_WRITE);
            if(pd)
            {
                ONUConfigData_t *p = openOnuConfigFile(name, ONU_CONF_OPEN_FLAG_WRITE);

                if(p)
                {
                    onuconfCopy(pd, p);
                }
                else
                {
                    closeOnuConfigFile(pd->confname);
                    /*deleteOnuConfFromHashBucket(pd->confname);*/
                    OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, pd->confname, NULL, NULL);
                    ret =VOS_ERROR;
                }
            }
            else
                ret = VOS_ERROR;

        }
        else if(vty)
            vty->onuconfptr = 0; /*不需要生成私有配置时，强制将缓存指针清0，防止退出操作时误判*/

    }

    return ret;
}

int cl_onuNodeVtyExitConfirmNoForTransparentGwOnu(struct vty *vty)
{
    ONUConfigData_t *pd = vty->onuconfptr;
    char sz[80] = "";
    CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
    ULONG   ulIfIndex = 0;
        /*ULONG   ulFeIndex=0;
        ULONG   ulState = 0;*/
    ULONG   ulSlot, ulPort, ulOnuid,ulSlot_swap = 0, ulPort_swap = 0;
    CHAR    prompt[64] = { 0 };

    if (PON_GetSlotPortOnu((ULONG)vty->index, &ulSlot, &ulPort, &ulOnuid) == VOS_OK)
    {
        int ponid = GetPonPortIdxBySlot(ulSlot, ulPort);
        int onuid = ulOnuid-1;

		VOS_StrCpy(sz, ONU_CONF_NAME_PTR_GET(ponid, onuid));

		if(!onuConfIsShared(ponid, onuid))
		{
			
         	ONU_CONF_SEM_TAKE
	        {
	            if (onuConfCheckByPtrNoSuffix(pd) == VOS_OK)
	            {
	                VOS_StrCpy(sz, ONU_CONF_NAME_PTR_GET(ponid, onuid));
                    OnuProfile_Action_ByCode(OnuMap_Update, 0, ponid, onuid, pd->confname, NULL, NULL);
	            }
	        }
            ONU_CONF_SEM_GIVE
            /*问题单16237，67背板没有删除私有文件*/
			if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
			{
                LONG to_slot= device_standby_master_slotno_get();
                if(to_slot)
                {
                    OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, ponid, onuid, pd->confname, NULL, NULL);
                }                
                OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, ulSlot, ponid, onuid, pd->confname, NULL, NULL);
				/*added by wangjiah@2017-03-17 : begin 
				sync to swap slot when hotswap is configured*/
				if(PonPortSwapSlotQuery(ponid, &ulSlot_swap, &ulPort_swap) == VOS_OK && ulSlot_swap != ulSlot/*no need to sync when inner-card protect configured*/)
				{
					OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, ulSlot_swap, GetPonPortIdxBySlot(ulSlot_swap, ulPort_swap), onuid, pd->confname, NULL, NULL);
				}
				/*added by wangjiah@2017-03-17: end*/
                OnuProfile_Action_ByCode(OnuProfile_Delete_SyncBroadcast, 0, 0, 0, sz, NULL, NULL);                
			}
            OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, sz, NULL, NULL);
            
		}

    }

	if( vty->prev_node == PON_PORT_NODE )
	{
		ulIfIndex =(ULONG) vty->index;
		PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
		ulIfIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, 0, 0);
		vty->index = ( VOID * )ulIfIndex;
		vty->node = PON_PORT_NODE;
		vty->prev_node = MAX_NODE;

		VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
		VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d", "pon", ulSlot, ulPort );
		VOS_StrCpy( prompt, "%s(epon-" );
		VOS_StrCat( prompt, ifName );
		VOS_StrCat( prompt, ")#" );
		vty_set_prompt( vty, prompt );
	}
    else
    {
        vty->node = CONFIG_NODE;
        vty->prev_node = MAX_NODE;
    }

    vty->onuconfptr = 0;

    return VOS_OK;
}

int cl_onuNodeVtyExitConfirmNo(struct vty *vty)
{
    ONUConfigData_t *pd = vty->onuconfptr;
    char sz[80] = "";
    ULONG ulIfidx = 0;
    CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
    ULONG   ulIfIndex = 0;
    ULONG   ulSlot, ulPort, ulOnuid;
    CHAR    prompt[64] = { 0 };

    if (PON_GetSlotPortOnu((ULONG)vty->index, &ulSlot, &ulPort, &ulOnuid) == VOS_OK)
    {
        int ponid = GetPonPortIdxBySlot(ulSlot, ulPort);
        int onuid = ulOnuid-1;

        ONU_CONF_SEM_TAKE
        {
            ponid = onuConfGetPonIdFromPonProtectGroup(ponid);

            VOS_StrCpy(sz, ONU_CONF_NAME_PTR_GET(ponid, onuid));

            if(!onuConfIsShared(ponid, onuid))
            {
                /*对ONU 实际配置进行更新，问题单13591 modi by luh 2011-10-10*/
                /*ulIfidx = IFM_PON_CREATE_INDEX(ulSlot, ulPort, ulOnuid, 0);*/
                onu_profile_associate_by_index(vty, ulSlot, ulPort, ulOnuid, pd->confname);
                /*end*/
                OnuProfile_Action_ByCode(OnuProfile_Delete_SyncBroadcast, 0, 0, 0, sz, NULL, NULL);
                /*只有在6900 sw上才可以直接删除，否则需要等恢复队列清空再删除2013-7-18*/
                if(SYS_LOCAL_MODULE_TYPE_IS_6900_SW || SYS_LOCAL_MODULE_TYPE_IS_8000_SW)
                    OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, sz, NULL, NULL);
                else
                    sendOnuConfLocalDelReqMsg(sz);                    
            }
        }
        ONU_CONF_SEM_GIVE

    }

    if( vty->prev_node == PON_PORT_NODE )
    {
	ulIfIndex =(ULONG) vty->index;
	PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	ulIfIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, 0, 0);
	vty->index = ( VOID * )ulIfIndex;
	vty->node = PON_PORT_NODE;
	vty->prev_node = MAX_NODE;

	VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
	VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d", "pon", ulSlot, ulPort );
	VOS_StrCpy( prompt, "%s(epon-" );
	VOS_StrCat( prompt, ifName );
	VOS_StrCat( prompt, ")#" );
	vty_set_prompt( vty, prompt );
    }
    else
    {
        vty->node = CONFIG_NODE;
        vty->prev_node = MAX_NODE;
    }

    vty->onuconfptr = 0;
    if(vty_quit_flag)
    {
        vty_out( vty, "Quit.%s", VTY_NEWLINE );
        vty->status = VTY_CLOSE;
        vty_quit_flag = 0;
    }
    /*end*/

    return VOS_OK;
}

int cl_onuNodeVtyExitConfirmYes(struct vty *vty)
{

    CHAR    ifName[IFM_NAME_SIZE + 1] = { 0 };
	CHAR    prompt[64] = { 0 };
    ULONG   ulIfIndex = 0;
    /*ULONG   ulFeIndex=0;
    ULONG   ulState = 0;*/
    ULONG   ulSlot, ulPort, ulOnuid;
	short int ponid = 0, onuid = 0;
    char sz[80]="";
    short int to_slot = 0;
	ulIfIndex =(ULONG) vty->index;
	PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );
	ponid = GetPonPortIdxBySlot(ulSlot, ulPort);
	onuid = ulOnuid-1;
	ONUCONF_DEBUG("\r\n cl_onuNodeVtyExitConfirmYes onu%d/%d/%d\r\n", ulSlot, ulPort, ulOnuid);
	/*avoid standby master write config file error, sync current config file*/
	if(ponid != RERROR && onuid != RERROR && SYS_LOCAL_MODULE_ISMASTERACTIVE)
	{
        unsigned int swap_slot = 0, swap_port = 0;
        OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, 0, ponid, onuid, ONU_CONF_NAME_PTR_GET(ponid, onuid), NULL, NULL);        
		ONUCONF_DEBUG("\r\n OnuProfile_Add_Syncbroadcast %s (%d, %d)\r\n", ONU_CONF_NAME_PTR_GET(ponid, onuid), ponid, onuid);
        /*确认修改，以保证save config时正确执行*/
        {
            ULONG uv[4] = {0,0,0,0};
            /*short int ponid = 0;*/
            char *name = NULL;
            /*ulIfIndex =(ULONG) vty->index;*/
            /*PON_GetSlotPortOnu( ulIfIndex, &ulSlot, &ulPort, &ulOnuid );*/
            /*ponid = GetPonPortIdxBySlot(ulSlot, ulPort);*/
            name = ONU_CONF_NAME_PTR_GET(ponid, onuid);
            uv[2] = name;
            uv[3] = EM_ONU_PROFILE_MODIFIED;
			/*modified by liyang 2014-08-25*/
            processEventCallbackList(EVENT_ONUPROFILE_MODIFIED, uv);
        }
        
		/*added by wangxiaoyu 2011-11-28 保持备用主控上的配置文件与当前主控一致*/
		to_slot = device_standby_master_slotno_get();
		if(to_slot)
		{
            OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, ponid, onuid, NULL, NULL, NULL);
		}	
//removed by wangjiah@2017-03-08
//Not necessary to sync onu map because OnuProfile_Add_SyncBroadcast operation has updated onu profile map
#if 0 
        /*对pon保护倒换的兼容实现2012-10-18*/
        if(PonPortAutoProtectPortQuery(ulSlot, ulPort, &swap_slot, &swap_port) == VOS_OK)
        {
            ONUConfigData_t *pd = NULL, *po= NULL; 
            char s_name[80] = "";
            short int swap_ponid = GetPonPortIdxBySlot(swap_slot, swap_port);
            VOS_StrCpy(s_name, g_onuConfOnuIdMapArry[ponid][onuid].name/*getOnuConfNamePtrByPonId(ponid, onuid)*/);
            
            OnuProfile_Action_ByCode(OnuMap_Update, 0, ponid, onuid, DEFAULT_ONU_CONF, NULL, NULL);
            OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, ulSlot, swap_ponid, onuid, sz, NULL, NULL);
            if(to_slot)
                OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, swap_ponid, onuid, sz, NULL, NULL);

            OnuProfile_Action_ByCode(OnuMap_Update, 0, swap_ponid, onuid, s_name, NULL, NULL);
            OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, swap_slot, swap_ponid, onuid, sz, NULL, NULL);
            if(to_slot)
                OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, swap_ponid, onuid, sz, NULL, NULL);
            
        }
#endif        
	}

    if( vty->prev_node == PON_PORT_NODE )
	{
		ulIfIndex = IFM_PON_CREATE_INDEX( ulSlot, ulPort, 0, 0);
		vty->index = ( VOID * )ulIfIndex;
		vty->node = PON_PORT_NODE;
		vty->prev_prev_node = MAX_NODE;

		VOS_MemZero( ifName, IFM_NAME_SIZE + 1 );
		VOS_Snprintf( ifName, IFM_NAME_SIZE, "%s%d/%d", "pon", ulSlot, ulPort );
		VOS_StrCpy( prompt, "%s(epon-" );
		VOS_StrCat( prompt, ifName );
		VOS_StrCat( prompt, ")#" );
		vty_set_prompt( vty, prompt );
	}
    else
    {
        vty->node = CONFIG_NODE;
        vty->prev_prev_node = MAX_NODE;
    }

    vty->onuconfptr = 0;
    if(vty_quit_flag)
    {
        vty_out( vty, "Quit.%s", VTY_NEWLINE );
        vty->status = VTY_CLOSE;
        vty_quit_flag = 0;
    }
    /*end*/

    return VOS_OK;
}

void cli_onuNodeVtyExitCallback(struct vty * vty)
{
    ULONG ulSlot, ulPort, ulOnuId;
    int ret = 0;
    int flag = 0;
    /*added master active state for master standby can't process confirm action node wangxiaoyu 2011-08-31*/
    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER && PON_GetSlotPortOnu((ULONG)vty->index, &ulSlot, &ulPort, &ulOnuId) == VOS_OK)
    {
        int ponid = GetGlobalPonPortIdxBySlot(ulSlot, ulPort);
        int onuid = ulOnuId-1;

        ONU_CONF_SEM_TAKE
        {

            /*如果是恢复数据的过程，则不进行文件比较，且不要求用户确认*/
            if(getOnuConfRestoreFlag(ponid, onuid))
            {
                ONU_CONF_SEM_GIVE;
                return;
            }

			ONUCONF_DEBUG("\r\n exit callback:onuConfCheckByPtrNoSuffix : %d\r\n", onuConfCheckByPtrNoSuffix(vty->onuconfptr));
            if(onuConfCheckByPtrNoSuffix(vty->onuconfptr) == VOS_OK)
            {
                ONUConfigData_t *p = vty->onuconfptr;
                ret = check_OnuNodeProfile(p, ponid, onuid);
                switch(ret)
                {
                    case ONU_NODE_EXIT_MODE_SAVE_NONE:
						ONUCONF_DEBUG("\r\n cli_onuNodeVtyExitCallback: ONU_NODE_EXIT_MODE_SAVE_NONE \r\n");
                        cl_onuNodeVtyExitConfirmNoForTransparentGwOnu(vty);
                        break;
                    case ONU_NODE_EXIT_MODE_SAVE_WITHOUT_NOTICE:
						ONUCONF_DEBUG("\r\n cli_onuNodeVtyExitCallback: ONU_NODE_EXIT_MODE_SAVE_WITHOUT_NOTICE\r\n");
                        cl_onuNodeVtyExitConfirmYes(vty);
                        break;
                    case ONU_NODE_EXIT_MODE_SAVE_NOTICE:
                        vty_out(vty, "the onu's configuration has changed, will you agree to save it? [Y/N]");
                        vty->node = CONFIRM_ACTION_NODE;
                        vty->no_action_func = cl_onuNodeVtyExitConfirmNo;
                        vty->action_func = cl_onuNodeVtyExitConfirmYes;
                        break;
                    default:
                        break;
                }
                flag = 1;
            }
            else
            {
                char *name = getOnuConfNamePtrByPonId(ponid, onuid);
                char sz[80] = "";
				int swap_slot = 0, swap_port = 0;

                if(name)
                {
                    ONUConfigData_t *p = NULL, *pd = NULL;
                    VOS_Sprintf(sz, "*auto%s", name);
                    p = getOnuConfFromHashBucket(name);
                    pd = getOnuConfFromHashBucket(sz);

                    if(pd && p)
                    {
                        closeOnuConfigFile(p->confname);
                        closeOnuConfigFile(pd->confname);
                        if(onuconfCompare(pd, p) != VOS_OK)
                        {
                        
                            /*ULONG uv[4] = {0, 0, name, EM_ONU_PROFILE_MODIFIED};*/
							ULONG uv[4] = {0, 0, 0, 0};
							uv[2] = (ULONG)name;
            				uv[3] = EM_ONU_PROFILE_MODIFIED;
                            processEventCallbackList(EVENT_ONUPROFILE_MODIFIED, uv);
							/* added by wangjiah@2017-03-08 sync to swap_slot when swap_slot is not equal to ulSlot:begin */
							if(PonPortSwapSlotQuery(ponid, &swap_slot, &swap_port) == VOS_OK)
							{
								ONUCONF_DEBUG("\r\nulSlot : %d, swap_slot : %d\r\n", ulSlot, swap_slot);
								if(swap_slot != ulSlot)
								{
									ONUCONF_DEBUG("\r\n sync onuconf: %s update to slot : %d\r\n", name, swap_slot);
									OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, swap_slot, 0, 0, name, NULL, NULL);
								}
							}
							/* added by wangjiah@2017-03-08 :end */
                        }
                        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, sz, NULL, NULL);
                    }

                }
                
                /*added by wangxiaoyu 2011-11-28 保持备用主控上的配置文件与当前主控一致*/
        		ulSlot = device_standby_master_slotno_get();
        		if(ulSlot && SYS_LOCAL_MODULE_ISMASTERACTIVE)
        		{
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, ulSlot, 0, 0, ONU_CONF_NAME_PTR_GET(ponid, onuid), NULL, NULL);                    
        		}
                
                if(vty->node != CONFIRM_ACTION_NODE)
                {
                    CHAR ifName[IFM_NAME_SIZE + 1] = { 0 };
                    ULONG ulIfIndex = 0;
                    ULONG ulSlot, ulPort, ulOnuid;
                    CHAR prompt[64] = { 0 };

                    if (vty->prev_prev_node == PON_PORT_NODE)
                    {
                        ulIfIndex = (ULONG) vty->index;
                        PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
                        ulIfIndex = IFM_PON_CREATE_INDEX(ulSlot, ulPort, 0, 0);
                        vty->index = (VOID *) ulIfIndex;
                        vty->node = PON_PORT_NODE;
                        vty->prev_prev_node = MAX_NODE;

                        VOS_MemZero(ifName, IFM_NAME_SIZE + 1);
                        VOS_Snprintf(ifName, IFM_NAME_SIZE, "%s%d/%d", "pon", ulSlot, ulPort);
                        VOS_StrCpy(prompt, "%s(epon-");
                        VOS_StrCat(prompt, ifName);
                        VOS_StrCat(prompt, ")#");
                        vty_set_prompt(vty, prompt);
                    }
                    else
                    {
                        vty->node = CONFIG_NODE;
                        vty->prev_prev_node = MAX_NODE;
                    }
                }
            }
        }
        ONU_CONF_SEM_GIVE        
    }
    /*解决pon板onu退不出onu节点的问题*/
    else
    {
        if(vty->node != CONFIRM_ACTION_NODE)
        {
            CHAR ifName[IFM_NAME_SIZE + 1] = { 0 };
            ULONG ulIfIndex = 0;
            ULONG ulSlot, ulPort, ulOnuid;
            CHAR prompt[64] = { 0 };

            if (vty->prev_prev_node == PON_PORT_NODE)
            {
                ulIfIndex = (ULONG) vty->index;
                PON_GetSlotPortOnu(ulIfIndex, &ulSlot, &ulPort, &ulOnuid);
                ulIfIndex = IFM_PON_CREATE_INDEX(ulSlot, ulPort, 0, 0);
                vty->index = (VOID *) ulIfIndex;
                vty->node = PON_PORT_NODE;
                vty->prev_prev_node = MAX_NODE;

                VOS_MemZero(ifName, IFM_NAME_SIZE + 1);
                VOS_Snprintf(ifName, IFM_NAME_SIZE, "%s%d/%d", "pon", ulSlot, ulPort);
                VOS_StrCpy(prompt, "%s(epon-");
                VOS_StrCat(prompt, ifName);
                VOS_StrCat(prompt, ")#");
                vty_set_prompt(vty, prompt);
            }
            else
            {
                vty->node = CONFIG_NODE;
                vty->prev_prev_node = MAX_NODE;
            }
        }
    }
    
    /*if(!flag)*/
    {
        if(vty_quit_flag)
        {
            vty_out( vty, "Quit.%s", VTY_NEWLINE );
            vty->status = VTY_CLOSE;
            vty_quit_flag = 0;
        }
    }
}


#if 1
int cl_profileNodeVtyExitConfirmNo(struct vty *vty)
{

    int slot = device_standby_master_slotno_get();

    ONUConfigData_t *pd = (ONUConfigData_t *)vty->onuconfptr;

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtrNoSuffix(pd) == VOS_OK)
        {
#if 0            
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE && slot != 0)
                sendOnuConfSyndDelReqMsg(slot, pd->confname);
			deleteOnuConfFromHashBucket(pd->confname);
#else
            OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, pd->confname, NULL, NULL);
#endif
        }
    }
    ONU_CONF_SEM_GIVE

    vty->orig_profileptr = NULL;
    vty->onuconfptr = NULL;
    if(vty_quit_flag)
    {
        vty_out( vty, "Quit.%s", VTY_NEWLINE );
        vty->status = VTY_CLOSE;
        vty_quit_flag = 0;
    }

    return 0;

}

int cl_profileNodeVtyExitConfirmYes(struct vty *vty)
{
    int ret = VOS_OK;
    ONUConfigData_t *pd = (ONUConfigData_t *)vty->onuconfptr;
    ONUConfigData_t *pp = (ONUConfigData_t *)vty->orig_profileptr;/*old profile*/

    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtrNoSuffix(pd) == VOS_OK &&
                    onuConfCheckByPtrNoSuffix(pp) == VOS_OK)
        {
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
            {
                OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, 0, 0, 0, pd->confname, NULL, NULL);
            }
            OnuProfile_Action_ByCode(OnuProfile_Modify, 0, 0, 0, pd->confname, pp->confname, NULL);
            OnuProfile_Action_ByCode(OnuProfile_Modify_SyncBroadcast, 0, 0, 0, pd->confname, pp->confname, NULL);
        }
        else
            ret = VOS_ERROR;
    }
    ONU_CONF_SEM_GIVE

    vty->orig_profileptr = NULL;
    vty->onuconfptr = NULL;
    if(vty_quit_flag)
    {
        vty_out( vty, "Quit.%s", VTY_NEWLINE );
        vty->status = VTY_CLOSE;
        vty_quit_flag = 0;
    }
    return ret;
}

void cli_profileNodeVtyExitCallback(struct vty *vty)
{
    ULONG  flag = 1;

    if(onuConfCheckByPtrNoSuffix(vty->onuconfptr) == VOS_OK &&
            onuConfCheckByPtrNoSuffix(vty->orig_profileptr) == VOS_OK)
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        ONUConfigData_t *pp = (ONUConfigData_t*)vty->orig_profileptr;
        int ret = 0;

        closeOnuConfigFile(pd->confname);
        closeOnuConfigFile(pp->confname);

        ret = onuconfCompare(pd, pp);

        if(!ret)
        {
#if 0            
            deleteOnuConfFromHashBucket(pd->confname);
#else
            OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, pd->confname, NULL, NULL);
#endif
        }
        else
        {
            if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
            {
                vty_out(vty, "the file has been modified, will you agree to save and apply it to all associated ONUs? [Y/N]");
                /* vty->prev_node = vty->node; */
                vty->node = CONFIRM_ACTION_NODE;
                vty->no_action_func = cl_profileNodeVtyExitConfirmNo;
                vty->action_func = cl_profileNodeVtyExitConfirmYes;
				vty->prev_node = CONFIG_NODE;
                flag = 0;
            }
        }
    }
    else if(onuConfCheckByPtrNoSuffix(vty->onuconfptr) == VOS_OK)
    {
        ONUConfigData_t *pd = (ONUConfigData_t*)vty->onuconfptr;
        closeOnuConfigFile(pd->confname);
        if(SYS_LOCAL_MODULE_ISMASTERACTIVE && g_SystemLoadConfComplete == _VOS_SYSTEM_RUNNING_)        
            OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, 0, 0, 0, pd->confname, NULL, NULL);        
    }

    if(flag)
    {
        vty->node = CONFIG_NODE;
        vty->onuconfptr = NULL;
        vty->orig_profileptr = NULL;
    }  
    
    if(vty_quit_flag)
    {
        vty_out( vty, "Quit.%s", VTY_NEWLINE );
        vty->status = VTY_CLOSE;
        vty_quit_flag = 0;
    }

}
#endif

int onuconfSwitchPonPort(short int src_slot, short int src_port, short int dst_slot, short int dst_port)
{
    int ret = VOS_ERROR, i;

    char s_name[80] = "", d_name[80]="";

    short int ponid = GetGlobalPonPortIdxBySlot(src_slot, src_port);
    short int dponid = GetGlobalPonPortIdxBySlot(dst_slot, dst_port);
    short int to_slot= device_standby_master_slotno_get();

    if(ponid == RERROR || dponid == RERROR)
        return ret;

    ONU_CONF_SEM_TAKE
    {
        for(i=0; i<MAXONUPERPON; i++)
        {
            short int onuid = i;
            short int donuid = i;
			int isSrcOnuConfDefault = isOnuConfDefault(ponid, onuid);
			int isDstOnuConfDefault = isOnuConfDefault(dponid, donuid);
#if 1
			/*bind non-defualt config to protect pair when reset pon port or restart olt*/
			if(isSrcOnuConfDefault && !isDstOnuConfDefault)
			{
				VOS_StrCpy(s_name, g_onuConfOnuIdMapArry[dponid][onuid].name);
				VOS_StrCpy(d_name, g_onuConfOnuIdMapArry[ponid][donuid].name);
			}
			else 
			{
				VOS_StrCpy(s_name, g_onuConfOnuIdMapArry[ponid][onuid].name);
				VOS_StrCpy(d_name, g_onuConfOnuIdMapArry[dponid][donuid].name);
			}	
            /*clrOnuConfNameMapByPonId(s_name, ponid, onuid);
            clrOnuConfNameMapByPonId(d_name, dponid, donuid);
            
            setOnuConfOnuIdMapByPonId(ponid, onuid, DEFAULT_ONU_CONF);
			setOnuConfNameMapByPonId(DEFAULT_ONU_CONF, ponid, onuid);

            setOnuConfOnuIdMapByPonId(dponid, donuid, s_name);
            setOnuConfNameMapByPonId(s_name, dponid, donuid);*/
            
            OnuProfile_Action_ByCode(OnuMap_Update, 0, ponid, onuid, DEFAULT_ONU_CONF, NULL, NULL);
			ONUCONF_ONU_LIMIT_DEBUG("\r\n update(%d,%d) conf to %s\r\n", ponid, onuid, DEFAULT_ONU_CONF);
            OnuProfile_Action_ByCode(OnuMap_Update, 0, dponid, onuid, s_name, NULL, NULL);
			ONUCONF_ONU_LIMIT_DEBUG("\r\n update(%d,%d) conf from %s to %s\r\n", dponid, onuid, d_name, s_name);
            
#else
            ONUConfigData_t *pd = NULL, *po= NULL, *pdata = NULL; 
            short int swap_slot = 0, swap_port = 0;
            char sz[80]="";
            short int swap_ponid = GetPonPortIdxBySlot(swap_slot, swap_port);
            VOS_Sprintf(sz, "onu%d/%d/%d", dst_slot, dst_port, onuid+1);            
            po = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));            
            if(po->share)
            {
                OnuProfile_Action_ByCode(OnuMap_Update, 0, dponid, onuid, po->confname, NULL, NULL);
                if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
                {
        			if(SYS_MODULE_IS_LOCAL(swap_slot)&&!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER)
        			{
                        OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, dst_slot, dponid, onuid, NULL, NULL, NULL);                
        			}      
            		if(to_slot)
            		{
                        OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, dponid, onuid, NULL, NULL, NULL);                
            		}
                }
                
            }
            else
            {
                pd = getOnuConfFromHashBucket(sz);       
                if(pd)
                {
                    onuconfCopy(pd, po);
                }
                else
                {
                    pd = onuconf_malloc(ONU_CONF_MEM_DATA_ID);
                    if(pd)
                    {
                        VOS_MemZero(pd, sizeof(ONUConfigData_t));
                        onuconfCopy(pd, po);
                        VOS_StrCpy(pd->confname, sz);
                        pd->share = 0;
                        OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, sz, NULL, pd);
                    }
                    else
                        VOS_ASSERT(0);	
                }
                OnuProfile_Action_ByCode(OnuMap_Update, 0, swap_ponid, onuid, sz, NULL, NULL);
                if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
                {
        			if(SYS_MODULE_IS_LOCAL(swap_slot)&&!SYS_LOCAL_MODULE_TYPE_IS_PONCARD_REMOTE_MANAGER)
        			{
                        OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, swap_slot, 0, 0, sz, NULL, NULL);                
                        OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, swap_slot, swap_ponid, onuid, sz, NULL, NULL);
        			}      
            		if(to_slot)
            		{
                        OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, to_slot, 0, 0, sz, NULL, NULL);                
                        OnuProfile_Action_ByCode(OnuMap_Update_SyncUnicast, to_slot, swap_ponid, onuid, sz, NULL, NULL);
            		}
                }
            }
#endif
        }
        ONU_CONF_SEM_GIVE
        ret = VOS_OK;
    }
/*
    ret = startOnuConfSyndReqByCard(dst_slot);
    ret |= startOnuConfSyndReqByCard(src_slot);
    ret |= onuconf_syncStandbyMaster(NULL);
*/
    return ret;
}

int onuconfSwitchOnPonPort(short int src_slot, short int src_port, short int dst_slot, short int dst_port)
{
    int ret = VOS_ERROR;

    int reg_flag = 1, onuEntry = 0, i;

    short int s_pon = GetPonPortIdxBySlot(src_slot, src_port);
    short int d_pon =  GetPonPortIdxBySlot(dst_slot, dst_port);
	UCHAR MacAddrInit[BYTES_IN_MAC_ADDRESS];

	VOS_MemSet(MacAddrInit, 0xff, BYTES_IN_MAC_ADDRESS );

    if(s_pon == RERROR || d_pon == RERROR)
        return ret;

    for(i=0; i<MAXONUPERPON; i++)
    {
        short int onu_id = 0;
		char sz[80] = "";
		
        DeviceInfo_S *pDevInfo = NULL;

        ONUConfigData_t *pd = NULL, *po = NULL;

        onuEntry = s_pon*MAXONUPERPON+i;

        pDevInfo = &OnuMgmtTable[onuEntry].DeviceInfo;

		/*added by liyang 2014-08-29  问题单20882*/
		if(MAC_ADDR_IS_EQUAL(pDevInfo->MacAddr,MacAddrInit))
			continue;

        /*获取目标PON口上的onuid*/

		ONU_MGMT_SEM_TAKE
        {
	        if((onu_id = SearchFreeOnuIdxForRegister(d_pon, pDevInfo->MacAddr, &reg_flag)) == RERROR)
	        {
				ONU_MGMT_SEM_GIVE
	            continue;
	        }
			else
			{
				OnuMgt_AddOnuByManual(d_pon, onu_id, pDevInfo->MacAddr); /*problem 13685*/
			}
        }
		ONU_MGMT_SEM_GIVE

        ONU_CONF_SEM_TAKE
        {
            pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(d_pon, onu_id));
            po = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(s_pon, i));

            if(po && pd)
            {
				if(!po->share)
				{
                    ONUConfigData_t * pdata = NULL;

                    VOS_Sprintf(sz, "onu%d/%d/%d", GetCardIdxByPonChip(d_pon), GetPonPortByPonChip(d_pon), onu_id+1);
                    pdata = onuconf_malloc(ONU_CONF_MEM_DATA_ID);

                    if(pdata)
                    {
                        VOS_MemZero(pdata, sizeof(ONUConfigData_t));
                        onuconfCopy(pdata, po);
                        VOS_StrCpy(pdata->confname, sz);
                        pdata->share = 0;
                        OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, sz, NULL, pdata);            
                    }
                    else
                        VOS_ASSERT(0);					
				}
				else
				    VOS_StrCpy(sz, po->confname);
								
                OnuProfile_Action_ByCode(OnuMap_Update, 0, d_pon, onu_id, sz, NULL, NULL);
            }
        }
        ONU_CONF_SEM_GIVE
    }

	ret = VOS_OK;

    return ret;

}

int onuconfSwitchOnuOverPon(struct vty *vty, short int src_slot, short int src_port, short int src_onu, short int dst_slot, short int dst_port, short int dst_onu)
{
    int ret = VOS_ERROR;
    char sz[80] = "";
    		
    ULONG ulIfidx = 0;
    ONUConfigData_t *pd = NULL, *po = NULL, *pauto = NULL;
	short int to_slot = device_standby_master_slotno_get();
    short int s_pon = GetPonPortIdxBySlot(src_slot, src_port);
    short int d_pon =  GetPonPortIdxBySlot(dst_slot, dst_port);

    if(s_pon == RERROR || d_pon == RERROR)
        return ret;


    ONU_CONF_SEM_TAKE
    {
        pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(d_pon, dst_onu));
        po = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(s_pon, src_onu));

        if(po && pd)
        {
            if(!po->share && pd->share)
            {
                ONUConfigData_t * pdata = NULL;

                VOS_Sprintf(sz, "onu%d/%d/%d", GetCardIdxByPonChip(d_pon), GetPonPortByPonChip(d_pon), dst_onu+1);
                pdata = getOnuConfFromHashBucket(sz);
                if(!pdata)
                {
                    pdata = onuconf_malloc(ONU_CONF_MEM_DATA_ID);
                    VOS_MemZero(pdata, sizeof(ONUConfigData_t));
                    onuconfCopy(pdata, po);
                    VOS_StrCpy(pdata->confname, sz);
                    pdata->share = 0;
                    OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, sz, NULL, pdata);
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, dst_slot, 0, 0, sz, NULL, NULL);  
                    if(to_slot)
                        OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, to_slot, 0, 0, sz, NULL, NULL);  
                }
                else
                {
                    onuconfCopy(pdata, po);
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, dst_slot, 0, 0, sz, NULL, NULL);   
                    if(to_slot)
                        OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, to_slot, 0, 0, sz, NULL, NULL);  
                    
                }
                /*ulIfidx = IFM_PON_CREATE_INDEX(dst_slot, dst_port, dst_onu+1, 0);*/
                onu_profile_associate_by_index(vty, dst_slot, dst_port, dst_onu+1, sz);	
            }
            else if(po->share)
            {
                VOS_StrCpy(sz, po->confname);
                /*ulIfidx = IFM_PON_CREATE_INDEX(dst_slot, dst_port, dst_onu+1, 0);*/
                onu_profile_associate_by_index(vty, dst_slot, dst_port, dst_onu+1, sz);	

            }
            else if(!pd->share)
            {                
                VOS_Sprintf(sz, "*auto%s", pd->confname);
    			pauto = onuconf_malloc(ONU_CONF_MEM_DATA_ID);

    			if(pauto)
    			{
    				VOS_StrCpy(pauto->confname, sz);
    				VOS_MemCpy(pauto->onulist, pd->onulist, sizeof(pd->onulist));
                    OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, sz, NULL, pauto);

    				if(VOS_OK == onuconfCopy(pauto, po))
    				{
                        OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, dst_slot, 0, 0, sz, NULL, NULL);
                        if(to_slot)
                            OnuProfile_Action_ByCode(OnuProfile_Add_SyncUnicast, to_slot, 0, 0, sz, NULL, NULL);                                
                        OnuProfile_Action_ByCode(OnuProfile_Modify_SyncUnicast, dst_slot, 0, 0, sz, pd->confname, NULL);
                        if(to_slot)
                            OnuProfile_Action_ByCode(OnuProfile_Modify_SyncUnicast, to_slot, 0, 0, sz, pd->confname, NULL);                           
                            
                        if(OnuProfile_Action_ByCode(OnuProfile_Modify, 0, 0, 0, sz, pd->confname, NULL)!=VOS_OK)
                            OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, sz, NULL, NULL);
    					ret = CMD_SUCCESS;
    				}
    				else
                        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, sz, NULL, NULL);
    				
    			}
    			else
    			{
    				vty_out(vty, "copy onu configuration fail!\r\n");
    			}
            }
        }
        ret = VOS_OK;
    }
    ONU_CONF_SEM_GIVE

    return ret;

}
#define onu_resconf_section_end

short int  onuConfGetPonIdFromPonProtectGroup(short int ponid)
{

    int slot = 0, port = 0, glponid = 0, llponid = 0;
    int swap_slot = 0, swap_port = 0, swap_ponid = 0;

	slot = GetCardIdxByPonChip(ponid);
	port = GetPonPortByPonChip(ponid);
	glponid = GetGlobalPonPortIdxBySlot(slot, port);
    llponid = GetPonPortIdxBySlot(slot, port);
    /*modi by luh 2013-3-7 如果使用全局ponid，在pon板上会打印断言，此处做个转换*/
	if(PonPortSwapSlotQuery(llponid, &swap_slot, &swap_port) == VOS_OK)
	{
		swap_ponid = GetGlobalPonPortIdxBySlot(swap_slot, swap_port);

		if(OLT_ISLOCAL(swap_ponid))
			return (swap_ponid > glponid)?glponid:swap_ponid;
		else
			return glponid;
	}

	return glponid;
}


void *onuconf_malloc(int module)
{
    void * ret = NULL;

    VOS_SemTake(g_onuConfMemSemId, VOS_WAITFOREVER);

    switch(module)
    {
    case ONU_CONF_MEM_DATA_ID:
        if(g_onuConfDataMem && g_onuConfDataItemNum < ONU_MAX_CONF_NUM)
        {
            int i = 0;
            for(i=0; i<ONU_MAX_CONF_NUM; i++)
            {
                if(!g_onuConfDataMem[i].used)
                {
                    ret = &g_onuConfDataMem[i].data;;
                    g_onuConfDataMem[i].used = 1;
                    g_onuConfDataItemNum++;
                    break;
                }
            }
        }
        break;
    case ONU_CONF_MEM_HASH_ID:
        if(g_onuConfHashMem && g_onuConfHashItemNum < ONU_MAX_CONF_NUM)
        {
            int i = 0;
            for(i=0; i<ONU_MAX_CONF_NUM; i++)
            {
                if(!g_onuConfHashMem[i].used)
                {
                    ret = &g_onuConfHashMem[i].data;;
                    g_onuConfHashMem[i].used = 1;
                    g_onuConfHashItemNum++;
                    break;
                }
            }
        }
        break;
    default:
        break;
    }

    VOS_SemGive(g_onuConfMemSemId);

    return ret;
}
void onuconf_free(void *data, int module)
{

    VOS_SemTake(g_onuConfMemSemId, VOS_WAITFOREVER);

    switch(module)
    {
    case ONU_CONF_MEM_DATA_ID:
        if(g_onuConfDataMem)
        {
            int i;
            for(i=0; i<ONU_MAX_CONF_NUM; i++)
            {
                if(&g_onuConfDataMem[i].data == data)
                {
                    g_onuConfDataMem[i].used = 0;
                    if(g_onuConfDataItemNum > 0)
                        g_onuConfDataItemNum--;
                    break;
                }
            }
        }
        break;
    case ONU_CONF_MEM_HASH_ID:
        if(g_onuConfHashMem)
        {
            int i;
            for(i=0; i<ONU_MAX_CONF_NUM; i++)
            {
                if(&g_onuConfHashMem[i].data == data)
                {
                    g_onuConfHashMem[i].used = 0;
                    if(g_onuConfHashItemNum > 0)
                        g_onuConfHashItemNum--;
                    break;
                }
            }
        }
        break;
    default:
        break;
    }

    VOS_SemGive(g_onuConfMemSemId);

}

int isOnuConfFileTableFull()
{
	int ret = FALSE;
	VOS_SemTake(g_onuConfMemSemId, VOS_WAITFOREVER);
	
	if(g_onuConfDataItemNum >= ONU_MAX_CONF_NUM_USE)
		ret = TRUE;
	VOS_SemGive(g_onuConfMemSemId);

	return ret;
}
int SetPrivatePtyEnable(int enable)
{
    short int slotno = 0;
    int ret = 0;   
    g_gwonu_pty_flag = enable;
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        slotno=device_standby_master_slotno_get();
        if(slotno)
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = 0;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_PRIVATE_PTY_ENABLE_CDP;
                p->data[0] = enable;

                if((slotno != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slotno)))
                    ret = sendOnuConfSyndMsg(slotno, p, len);
                else
                    VOS_Free(p);
            }
            else
                ret = VOS_ERROR;            
        }
    }
    return ret;
}

int setOnuTransmission_flag(int enable)
{
    int ret = VOS_OK;
    int slot = 0;
    OnuTransmission_flag = enable;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for( slot = 1; slot <= SYS_CHASSIS_SLOTNUM; slot++ )
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = 0;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_TRANSMISSION_FLAG_CDP;
                p->data[0] = enable;

                if((slot != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slot))&&(SYS_MODULE_SLOT_ISHAVECPU(slot)))
                    ret = sendOnuConfSyndMsg(slot, p, len);
                else
                    VOS_Free(p);
            }
            else
            ret = VOS_ERROR;
        }
    }

    return ret;
}
int SetUpdatePendingTime(int time)
{
    int ret = VOS_OK;
    int slot = 0;
    UpdatePendingQueueTime = time;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for( slot = 1; slot <= SYS_CHASSIS_SLOTNUM; slot++ )
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = 0;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_PENDING_UPDATETIME_CDP;
                p->data[0] = time;

                if((slot != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slot))&&(SYS_MODULE_SLOT_ISHAVECPU(slot)))
                    ret = sendOnuConfSyndMsg(slot, p, len);
                else
                    VOS_Free(p);
            }
            else
            ret = VOS_ERROR;
        }
    }

    return ret;
}
int ResumeOnuTransmissionflagBySlot(USHORT slot, int enable)
{
    int ret = VOS_OK;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = 0;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_TRANSMISSION_FLAG_CDP;
                p->data[0] = enable;
                if((slot != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slot))&&(SYS_MODULE_SLOT_ISHAVECPU(slot)))
                    ret = sendOnuConfSyndMsg(slot, p, len);
                else
                    VOS_Free(p);
            }
            else
            ret = VOS_ERROR;
    }

    return ret;
}

int ResumeOtherCtcOnuRegistrationBySlot(USHORT slot, ULONG v)
{
    int ret = VOS_OK;
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = 0;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_OTHER_VENDOR_REGISTRATION_CDP;
                p->data[0] = v;

                if((slot != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slot))&&(SYS_MODULE_SLOT_ISHAVECPU(slot)))
                    ret = sendOnuConfSyndMsg(slot, p, len);
                else
                    VOS_Free(p);
            }
            else
	            ret = VOS_ERROR;
    	}
	return ret;			
}

int ResumeUpdatePendingPeriodBySlot(USHORT slot, int period)
{
    int ret = VOS_OK;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = 0;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_PENDING_UPDATETIME_CDP;
                p->data[0] = period;

                if((slot != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slot))&&(SYS_MODULE_SLOT_ISHAVECPU(slot)))
                    ret = sendOnuConfSyndMsg(slot, p, len);
                else
                    VOS_Free(p);
            }
            else
            ret = VOS_ERROR;
    }

    return ret;
}
#if 1
int check_black_onu_model(short int PonPortIdx, short int OnuIdx)
{	
	int i = 0;
	int onu_model = 0;
	ONU_MGMT_SEM_TAKE;
	onu_model = OnuMgmtTable[PonPortIdx*MAXONUPERPON +OnuIdx].onu_model;
	ONU_MGMT_SEM_GIVE;

	for(i=0;i<MAX_ONUMODEL_BLACKLIST;i++)
	{
		if(onuModelBlacklList[i] == onu_model && onu_model)
			return 1;
	}
	return 0;
}

int add_black_onu_model(int onu_model)
{
	int i = 0;
	int insert_num = VOS_ERROR;
	for(i=0;i<MAX_ONUMODEL_BLACKLIST;i++)
	{
		if(onuModelBlacklList[i] == onu_model && onu_model)
			break;
		else if(VOS_ERROR == insert_num && onuModelBlacklList[i] == 0)
			insert_num = i;
		else
		{
			/*do nothing*/
		}
	}
	if(i == MAX_ONUMODEL_BLACKLIST && insert_num != VOS_ERROR && onu_model)
	{
		onuModelBlacklList[insert_num] = onu_model;
	}
	return VOS_OK;
}

int del_black_onu_model(int onu_model)
{
	int i = 0;
	for(i=0;i<MAX_ONUMODEL_BLACKLIST;i++)
	{
		if(onuModelBlacklList[i] == onu_model && onu_model)
		{
			onuModelBlacklList[i] = 0;
			break;
		}
	}
	return VOS_OK;
}

void show_black_onu_model(struct vty *vty)
{
	int i = 0;
	vty_out(vty, "ctc onu-model black-list:\r\n");
	vty_out(vty, "-----------------------------\r\n");
	for(i=0;i<MAX_ONUMODEL_BLACKLIST;i++)
	{
		if(onuModelBlacklList[i])
		{
			vty_out(vty, "  0x%08x%s", onuModelBlacklList[i], "  ");
			if((i+1)%2 == 0)
				vty_out(vty, "\r\n");
		}
	}	
	vty_out(vty, "\r\n");
}
void show_run_onu_model(struct vty *vty)
{
	int i = 0;
	for(i=0;i<MAX_ONUMODEL_BLACKLIST;i++)
	{
		if(onuModelBlacklList[i])
		{
			vty_out(vty, " add onu-model blacklist 0x%08x\r\n", onuModelBlacklList[i]);
		}
	}	
}
int AddOnuModelBlackList(int model)
{
    int ret = VOS_OK;
    int slot = 0;
	
	add_black_onu_model(model);
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for( slot = 1; slot <= SYS_CHASSIS_SLOTNUM; slot++ )
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = 1;/*1代表添加|  2代表删除*/
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_ONU_MODEL_BLACK_LIST_CDP;
                p->data[0] = model;

                if((slot != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slot))&&(SYS_MODULE_SLOT_ISHAVECPU(slot)))
                    ret = sendOnuConfSyndMsg(slot, p, len);
                else
                    VOS_Free(p);
            }
            else
            ret = VOS_ERROR;
        }
    }
}
int DelOnuModelBlackList(int model)
{
    int ret = VOS_OK;
    int slot = 0;

	del_black_onu_model(model);
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for( slot = 1; slot <= SYS_CHASSIS_SLOTNUM; slot++ )
        {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = 2;/*1代表添加|  2代表删除*/
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_ONU_MODEL_BLACK_LIST_CDP;
                p->data[0] = model;

                if((slot != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slot))&&(SYS_MODULE_SLOT_ISHAVECPU(slot)))
                    ret = sendOnuConfSyndMsg(slot, p, len);
                else
                    VOS_Free(p);
            }
            else
            ret = VOS_ERROR;
        }
    }
}

int ResumeOneOnuModelBlackListBySlot(USHORT slot, int model)
{
    int ret = VOS_OK;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        ULONG len = sizeof(ONUCONFSYNDMSG_T);
        ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
        if(p)
        {
            p->slotid = 1;
            p->ponid = 0;
            p->onuid = 0;
            p->msgtype = MSG_ONU_CONFSYND_ONU_MODEL_BLACK_LIST_CDP;
            p->data[0] = model;

            if((slot != SYS_LOCAL_MODULE_SLOTNO)&&(!SYS_MODULE_ISMASTERSTANDBY(slot))&&(SYS_MODULE_SLOT_ISHAVECPU(slot)))
                ret = sendOnuConfSyndMsg(slot, p, len);
            else
                VOS_Free(p);
        }
        else
	        ret = VOS_ERROR;
    }

    return ret;
}
int ResumeOnuModelBlackListBySlot(USHORT slot)
{
    int ret = VOS_OK;
	int i = 0;
	
	for(i=0;i<MAX_ONUMODEL_BLACKLIST;i++)
	{
		if(onuModelBlacklList[i])
		{
			ResumeOneOnuModelBlackListBySlot(slot, onuModelBlacklList[i]);			
		}
	}    
    return ret;
}

#endif
int sendOnuConfSyncCardReqFinishedMsg(int dstslot)
{
    int ret = VOS_OK;

    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = dstslot;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_CARD_REQ_FINISHED_CDP;
                p->data[0] = 0;

                ret = sendOnuConfSyndMsg(dstslot, p, len);

            }
    }

    return ret;
}

int sendOnuConfSyncCardReqFinishedAckMsg(int dstslot)
{
    int ret = VOS_OK;

    if(SYS_LOCAL_MODULE_TYPE_IS_PONCARD_LOCAL_MANAGER || SYS_LOCAL_MODULE_ISMASTERSTANDBY)
    {
            ULONG len = sizeof(ONUCONFSYNDMSG_T);
            ONUCONFSYNDMSGPTR_T p = VOS_Malloc(len, MODULE_RPU_CTC);
            if(p)
            {
                p->slotid = dstslot;
                p->ponid = 0;
                p->onuid = 0;
                p->msgtype = MSG_ONU_CONFSYND_CARD_REQ_FINISHED_ACK_CDP;
                p->data[0] = SYS_LOCAL_MODULE_SLOTNO;

                ret = sendOnuConfSyndMsg(dstslot, p, len);
            }
    }

    return ret;
}

int getOnuTransmission_flag()
{
    return OnuTransmission_flag;
}

int check_OnuNodeProfile(ONUConfigData_t *p, ULONG pon_id, ULONG onu_id)
{
    int ret = ONU_NODE_EXIT_MODE_NOOP;
    ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(pon_id, onu_id));

    if(pd)
    {
        if(p->share && !pd->share)
        {
            if (onuconfCompare(p, pd))
            {
                if (VOS_StriCmp(p->confname, DEFAULT_ONU_CONF) == 0)
                {
                    ret = ONU_NODE_EXIT_MODE_SAVE_WITHOUT_NOTICE;
            }
            else
                {
                    ret = ONU_NODE_EXIT_MODE_SAVE_NOTICE;
                }
            }
            else
                ret = ONU_NODE_EXIT_MODE_SAVE_NONE;
        }
        else if(p->share && pd->share)
            ret = ONU_NODE_EXIT_MODE_SAVE_NONE;
        
        closeOnuConfigFile(p->confname);
        closeOnuConfigFile(pd->confname);
    }
    
    return ret;
}


#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
void CmcConfigProfile_init(ONUConfigData_t *p_profile)
{
    int i;
    ONUCmcConf_t *pCmcCfg;

    VOS_ASSERT(p_profile);
    if ( NULL != (pCmcCfg = &p_profile->cmcCfg) )
    {
        VOS_MemZero(pCmcCfg, sizeof(ONUCmcConf_t));
    
        pCmcCfg->maxCm = CMC_CFG_MAX_CM_DEFAULT;

        pCmcCfg->aucUsChannelEnable[0] = 0xF;
        pCmcCfg->aucUsChannelD30[0] = 0xF;

        for ( i = 0; i < CMC_USCHANNEL_NUM; i++ )
        {
            pCmcCfg->aulUsChannelFreq[i] = CMC_CFG_UP_CHANNEL_FREQ_DEFAULT(i);
            pCmcCfg->aulUsChannelFreqWidth[i] = CMC_CFG_UP_CHANNEL_FREQ_WIDTH_DEFAULT;

            pCmcCfg->asUsChannelPower[i] = CMC_CFG_UP_CHANNEL_POWER_LEVEL_DEFAULT;
        }
        
        for ( i = 0; i < CMC_DSCHANNEL_NUM; i++ )
        {
            pCmcCfg->aulDsChannelFreq[i] = CMC_CFG_DOWN_CHANNEL_FREQ_DEFAULT(i);

            pCmcCfg->asDsChannelPower[i] = CMC_CFG_DOWN_CHANNEL_POWER_LEVEL_DEFAULT;
        }
    }   

    return;
}
#endif

ONUConfigData_t *OnuConfigProfile_init()
{
    int i = 0, j = 0;
    ONUConfigData_t *p = (ONUConfigData_t *)onuconf_malloc(ONU_CONF_MEM_DATA_ID);

	if(p)
	{
	    VOS_MemSet(p, 0, calcOnuConfigSize());
		
	    p->atuAging = 330;
		p->holdover = 200;
		p->igmpFastLeaveEnable = ENABLE;
	    for(i=0;i<ONU_MAX_PORT;i++)
	    {
	        p->portconf[i].enable = ENABLE;
	        p->portconf[i].igmpFastLeaveEnable = ENABLE;            
	        p->portconf[i].atuLearnEnable = ENABLE;
	        p->portconf[i].atuFloodEnable = 0;
	        p->portconf[i].mode = 0;/*0-enable 2012-10-31 modified by luh*/
	        p->portconf[i].ingressRateAction = ONU_CONF_PORT_INGRESS_ACT_NONE;
		    p->portconf[i].ingressBurstMode = ONU_CONF_PORT_INGRESS_BURST_NONE;
	        p->portconf[i].defaultVid = 1;
	        p->portconf[i].vlanAcceptFrameType = 1;
	        p->portconf[i].igmpMaxGroup = 255;
	        p->portconf[i].igmpTagStrip = DISABLE;
			p->portconf[i].qinqEntry.qinqMode = CTC_QINQ_MODE_NONE_C;
	        for(j=0;j<8;j++)
	        {
	            p->portconf[i].qosPrioReplace[j] = j+1;
	        }
	    }

	    p->igmpHostAgeTime = 135;
	    p->igmpGroupAgeTime = 125;
	    p->igmpMaxResponseTime = 10;
	    p->igmpMaxGroup = 256;
		
	    for(i=0;i<8;i++)
	    {
	        p->qosMap.queue[i] = i+1;
	    }
		
	    for(i=0;i<8;i++)
	    {
	        p->qosMap.qosDscpQueue[i][i] = 0xFF;
	    }
		p->vlanconf.portIsolateEnable = 1;
	    p->vlanconf.onuVlanEntryNum = 1;
	    p->vlanconf.vmtrunkportmask = 0xFFFFFFFF;
		p->vlanconf.vmtransparentportmask = 0;
		p->vlanconf.vmtagportmask = 0;
		p->vlanconf.vmtranslationportmask = 0;
		p->vlanconf.vmaggportmask = 0;
		
	    p->vlanconf.entryarry[0].vlanid = 1;
	    p->vlanconf.entryarry[0].allPortMask= 0xFFFFFFFF;
	    p->vlanconf.entryarry[0].untagPortMask = 0xFFFFFFFF;

#if (EPON_MODULE_DOCSIS_PROFILE == EPON_MODULE_YES)
        CmcConfigProfile_init(p);
#endif
	}
	
    return p;        
}

ULONG getOnuConfFileCounter()
{
    ULONG counter = 0;
    ONU_CONF_SEM_TAKE
    {
        counter = g_onuConfHashItemNum>g_onu_temp_profile_num?g_onuConfHashItemNum-g_onu_temp_profile_num:0;
    }
    ONU_CONF_SEM_GIVE
    return counter;
}
ONUConfigData_t *test_OnuProfile_full_init()
{
    int i = 0, j = 0, k = 0;
    ONUConfigData_t *p = (ONUConfigData_t *)onuconf_malloc(ONU_CONF_MEM_DATA_ID);
    char aaa[6] = {0,1,2,3,4,5};
 	if(p)
	{
	    VOS_MemSet(p, 0, calcOnuConfigSize());
	    p->atuAging = 350;
		p->holdover = 220;
		p->linkMonEnable = ENABLE;
        p->modeMonEnable = ENABLE;
        p->fecenable = ENABLE;
        p->ingressMirrorFromList = 0xFF;
        p->egressMirrorFromList = 0xFF00;
        p->ingressMirrorToList = 0x1;
        p->egressMirrorToList = 0x2;
        p->igmpEnable = ENABLE;
        p->igmpAuthEnable = ENABLE;
        p->igmpFastLeaveEnable = ENABLE;
        p->qosAlgorithm = 1;
        p->ingressRateLimitBase = 1;
        for(i=0;i<QOS_MAX_SET_COUNT;i++)
        {
            for(j=0;j<QOS_MAX_CLASS_RULE_PAIRS;j++)
            {            
                p->qosset[i][j].valid = 1;
                p->qosset[i][j].queue_mapped = 2;
                p->qosset[i][j].priority_mark = 3;
                p->qosset[i][j].num_of_entries = 8;
                for(k=0;k<QOS_MAX_CLASS_RULE_PAIRS;k++)
                {
                    p->qosset[i][j].entries[k].field_select = FIELD_SEL_DA_MAC;
                    VOS_MemCpy(p->qosset[i][j].entries[k].value.mac_address, aaa ,6);
                    p->qosset[i][j].entries[k].validation_operator = VALIDATION_OPERS_EQUAL;
                }
            }
        }
	    for(i=0;i<16;i++)
	    {
	        p->portconf[i].enable = DISABLE;
	        p->portconf[i].atuLearnEnable = DISABLE;
	        p->portconf[i].atuFloodEnable = ENABLE;
            p->portconf[i].fecEnable = ENABLE;
            p->portconf[i].loopDetectEnable = ENABLE;
            p->portconf[i].pauseEnable = ENABLE;
            p->portconf[i].ingressRateType = 1;
            p->portconf[i].ingressRateLimit = 1;
            p->portconf[i].egressRateLimit = 1;
            p->portconf[i].vlanIngressFilter = 1;
            p->portconf[i].igmpFastLeaveEnable = ENABLE;
            p->portconf[i].igmpVlanNum = 16;
            p->portconf[i].qosIpEnable = ENABLE;
            p->portconf[i].qosRule = 1;
            p->portconf[i].qosSetSel = 4;
            p->portconf[i].qinqEntry.qinqMode = 1;
            p->portconf[i].qinqEntry.numberOfEntry = 8;
            for(j=0;j<16;j++)
            {
                p->portconf[i].qinqEntry.qinqEntries[j] = j+1;
            }
            for(j=0;j<16;j++)
            {
                p->portconf[i].igmpVlan[j] = j+2;
            }
	        p->portconf[i].mode = 1;
	        p->portconf[i].ingressRateAction = ONU_CONF_PORT_INGRESS_ACT_DROP;
		    p->portconf[i].ingressBurstMode = ONU_CONF_PORT_INGRESS_BURST_24K;
	        p->portconf[i].defaultVid = 100;
	        p->portconf[i].vlanAcceptFrameType = 1;
	        p->portconf[i].igmpMaxGroup = 4;
	        p->portconf[i].igmpTagStrip = DISABLE;
	        for(j=0;j<8;j++)
	        {
	            p->portconf[i].qosPrioReplace[j] = (j+3)%7+1;
	        }
	    }
        for(i=16;i<ONU_MAX_PORT;i++)
	    {
	        p->portconf[i].enable = ENABLE;
	        p->portconf[i].atuLearnEnable = ENABLE;
	        p->portconf[i].atuFloodEnable = 0;
	        p->portconf[i].mode = 0;
	        p->portconf[i].ingressRateAction = ONU_CONF_PORT_INGRESS_ACT_NONE;
		    p->portconf[i].ingressBurstMode = ONU_CONF_PORT_INGRESS_BURST_NONE;
	        p->portconf[i].defaultVid = 1;
	        p->portconf[i].vlanAcceptFrameType = 1;
	        p->portconf[i].igmpMaxGroup = 4;
	        p->portconf[i].igmpTagStrip = ENABLE;
			p->portconf[i].qinqEntry.qinqMode = CTC_QINQ_MODE_NONE_C;
	        for(j=0;j<8;j++)
	        {
	            p->portconf[i].qosPrioReplace[j] = j+1;
	        }
	    }
	    p->igmpHostAgeTime = 150;
	    p->igmpGroupAgeTime = 135;
	    p->igmpMaxResponseTime = 15;
	    p->igmpMaxGroup = 244;
		
	    for(i=0;i<8;i++)
	    {
	        p->qosMap.queue[i] = (i+3)%8;
	    }
		
	    for(i=0;i<8;i++)
	    {
	        p->qosMap.qosDscpQueue[i][(i+2)%8] = 0xFF;
	    }

    	    p->vlanconf.onuVlanEntryNum = 64;
    	    p->vlanconf.vmtrunkportmask = 0xFFFFFFFF;
    		p->vlanconf.vmtransparentportmask = 0;
    		p->vlanconf.vmtagportmask = 0;
    		p->vlanconf.vmtranslationportmask = 0;
    		p->vlanconf.vmaggportmask = 0;
            p->vlanconf.entryarry[0].vlanid = 1;
    	    p->vlanconf.entryarry[0].allPortMask= 0xFFFFFFFF;
    	    p->vlanconf.entryarry[0].untagPortMask = 0xFFFFFFFF;           
		for(i=1;i<64;i++)
		{
            p->vlanconf.entryarry[i].vlanid = i+100;
    	    p->vlanconf.entryarry[i].allPortMask= 0xFFFFFFFF;
    	    p->vlanconf.entryarry[i].untagPortMask = 0xFFFFFFFF;
		}

        for(i=0;i<4;i++)
        {
		    p->igmpGroupArry[i].gda = 0xE1020101+i;
            p->igmpGroupArry[i].mapPortMask = 0xffffffff;
            p->igmpGroupArry[i].multicastVlanId = 100+i;
        }
	}
	
    return p;        
}
ONUConfigData_t *test_CTC_OnuProfile_full_init()
{
    int i = 0, j = 0, k = 0;
    ONUConfigData_t *p = (ONUConfigData_t *)onuconf_malloc(ONU_CONF_MEM_DATA_ID);
    char aaa[6] = {0,1,2,3,4,5};
 	if(p)
	{
	    VOS_MemSet(p, 0, calcOnuConfigSize());
		
	    p->atuAging = 350;
		p->holdover = 220;
		p->linkMonEnable = ENABLE;
        p->modeMonEnable = ENABLE;
        p->fecenable = ENABLE;
        p->igmpEnable = ENABLE;
        p->igmpAuthEnable = ENABLE;
        p->igmpFastLeaveEnable = ENABLE;
        p->qosAlgorithm = 1;
        p->ingressRateLimitBase = 1;
        for(i=0;i<QOS_MAX_SET_COUNT;i++)
        {
            for(j=0;j<QOS_MAX_RULE_COUNT_PER_SET;j++)
            {            
                p->qosset[i][j].valid = 1;
                p->qosset[i][j].queue_mapped = 2;
                p->qosset[i][j].priority_mark = 3;
                p->qosset[i][j].num_of_entries = 8;
                for(k=0;k<QOS_MAX_CLASS_RULE_PAIRS;k++)
                {
                    p->qosset[i][j].entrymask |= 1<<k;
                    p->qosset[i][j].entries[k].field_select = FIELD_SEL_DA_MAC;
                    VOS_MemCpy(p->qosset[i][j].entries[k].value.mac_address, aaa ,6);
                    p->qosset[i][j].entries[k].validation_operator = VALIDATION_OPERS_EQUAL;
                }
            }
        }
	    for(i=0;i<16;i++)
	    {
	        p->portconf[i].enable = DISABLE;
	        p->portconf[i].atuLearnEnable = DISABLE;
	        p->portconf[i].atuFloodEnable = ENABLE;
            p->portconf[i].fecEnable = ENABLE;
            p->portconf[i].loopDetectEnable = ENABLE;
            p->portconf[i].pauseEnable = ENABLE;
            p->portconf[i].ingressRateType = 1;
            p->portconf[i].ingressRateLimit = 1;
            p->portconf[i].egressRateLimit = 1;
            p->portconf[i].vlanIngressFilter = 1;
            p->portconf[i].igmpVlanNum = 16;
            p->portconf[i].qosIpEnable = ENABLE;
            p->portconf[i].qosRule = 1;
            p->portconf[i].qosSetSel = 4;
            
            p->portconf[i].qinqEntry.qinqMode = 1;
            p->portconf[i].qinqEntry.numberOfEntry = 8;
            for(j=0;j<16;j++)
            {
                p->portconf[i].qinqEntry.qinqEntries[j] = j+1;
            }  
            for(j=0;j<16;j++)
            {
                p->portconf[i].igmpVlan[j] = j+2;
            }
	        p->portconf[i].mode = 1;
	        p->portconf[i].defaultVid = 100;
	        p->portconf[i].vlanAcceptFrameType = 1;
	        p->portconf[i].igmpMaxGroup = 4;
	        p->portconf[i].igmpTagStrip = DISABLE;
	    }
        for(i=16;i<ONU_MAX_PORT;i++)
	    {
	        p->portconf[i].enable = ENABLE;
	        p->portconf[i].atuLearnEnable = ENABLE;
	        p->portconf[i].atuFloodEnable = 0;
	        p->portconf[i].mode = 0;
	        p->portconf[i].ingressRateAction = ONU_CONF_PORT_INGRESS_ACT_NONE;
		    p->portconf[i].ingressBurstMode = ONU_CONF_PORT_INGRESS_BURST_NONE;
	        p->portconf[i].defaultVid = 1;
	        p->portconf[i].vlanAcceptFrameType = 1;
	        p->portconf[i].igmpMaxGroup = 4;
	        p->portconf[i].igmpTagStrip = ENABLE;
			p->portconf[i].qinqEntry.qinqMode = CTC_QINQ_MODE_NONE_C;
	        for(j=0;j<8;j++)
	        {
	            p->portconf[i].qosPrioReplace[j] = j+1;
	        }
	    }
	    p->igmpHostAgeTime = 150;
	    p->igmpGroupAgeTime = 135;
	    p->igmpMaxResponseTime = 15;
	    p->igmpMaxGroup = 244;
		
    	    p->vlanconf.onuVlanEntryNum = 64;
    	    p->vlanconf.vmtrunkportmask = 0xFFFFFFFF;
    		p->vlanconf.vmtransparentportmask = 0;
    		p->vlanconf.vmtagportmask = 0;
    		p->vlanconf.vmtranslationportmask = 0;
    		p->vlanconf.vmaggportmask = 0;
            p->vlanconf.entryarry[0].vlanid = 1;
    	    p->vlanconf.entryarry[0].allPortMask= 0xFFFFFFFF;
    	    p->vlanconf.entryarry[0].untagPortMask = 0xFFFFFFFF;           
		for(i=1;i<64;i++)
		{
            p->vlanconf.entryarry[i].vlanid = i+200;
    	    p->vlanconf.entryarry[i].allPortMask= 0xFFFFFFFF;
    	    p->vlanconf.entryarry[i].untagPortMask = 0xFFFFFFFF;
		}
	}
	
    return p;        
}

ONUConfigData_t *test_GW_OnuProfile_full_init()
{
    int i = 0, j = 0;
    ONUConfigData_t *p = (ONUConfigData_t *)onuconf_malloc(ONU_CONF_MEM_DATA_ID);
 	if(p)
	{
	    VOS_MemSet(p, 0, calcOnuConfigSize());
		
	    p->atuAging = 350;
		p->holdover = 220;
		p->linkMonEnable = ENABLE;
        p->modeMonEnable = ENABLE;
        p->fecenable = ENABLE;
        p->ingressMirrorFromList = 0xFF;
        p->egressMirrorFromList = 0xFF00;
        p->ingressMirrorToList = 0x1;
        p->egressMirrorToList = 0x2;
        p->igmpEnable = ENABLE;
        p->igmpAuthEnable = ENABLE;
        p->qosAlgorithm = 1;
        p->ingressRateLimitBase = 1;
	    for(i=0;i<16;i++)
	    {
	        p->portconf[i].enable = DISABLE;
	        p->portconf[i].atuLearnEnable = DISABLE;
	        p->portconf[i].atuFloodEnable = ENABLE;
            p->portconf[i].fecEnable = ENABLE;
            p->portconf[i].loopDetectEnable = ENABLE;
            p->portconf[i].pauseEnable = ENABLE;
            p->portconf[i].ingressRateType = 1;
            p->portconf[i].ingressRateLimit = 1;
            p->portconf[i].egressRateLimit = 1;
            p->portconf[i].vlanIngressFilter = 1;
            p->portconf[i].igmpFastLeaveEnable = ENABLE;
            p->portconf[i].igmpVlanNum = 0;
            p->portconf[i].qosIpEnable = ENABLE;
            p->portconf[i].qosRule = 1;
            
            p->portconf[i].qinqEntry.qinqMode = 0;
            p->portconf[i].qinqEntry.numberOfEntry = 0;

	        p->portconf[i].mode = 1;
	        p->portconf[i].ingressRateAction = ONU_CONF_PORT_INGRESS_ACT_DROP;
		    p->portconf[i].ingressBurstMode = ONU_CONF_PORT_INGRESS_BURST_24K;
	        p->portconf[i].defaultVid = 100;
	        p->portconf[i].vlanAcceptFrameType = 1;
	        p->portconf[i].igmpMaxGroup = 4;
	        p->portconf[i].igmpTagStrip = DISABLE;
	        for(j=0;j<8;j++)
	        {
	            p->portconf[i].qosPrioReplace[j] = (j+3)%7+1;
	        }
	    }
        for(i=16;i<ONU_MAX_PORT;i++)
	    {
	        p->portconf[i].enable = ENABLE;
	        p->portconf[i].atuLearnEnable = ENABLE;
	        p->portconf[i].atuFloodEnable = 0;
	        p->portconf[i].mode = 0;
	        p->portconf[i].ingressRateAction = ONU_CONF_PORT_INGRESS_ACT_NONE;
		    p->portconf[i].ingressBurstMode = ONU_CONF_PORT_INGRESS_BURST_NONE;
	        p->portconf[i].defaultVid = 1;
	        p->portconf[i].vlanAcceptFrameType = 1;
	        p->portconf[i].igmpMaxGroup = 4;
	        p->portconf[i].igmpTagStrip = ENABLE;
			p->portconf[i].qinqEntry.qinqMode = CTC_QINQ_MODE_NONE_C;
	        for(j=0;j<8;j++)
	        {
	            p->portconf[i].qosPrioReplace[j] = j+1;
	        }
	    }
	    p->igmpHostAgeTime = 150;
	    p->igmpGroupAgeTime = 135;
	    p->igmpMaxResponseTime = 15;
	    p->igmpMaxGroup = 244;
		
	    for(i=0;i<8;i++)
	    {
	        p->qosMap.queue[i] = (i+3)%8;
	    }
		
	    for(i=0;i<8;i++)
	    {
	        p->qosMap.qosDscpQueue[i][(i+2)%8] = 0xFF;
	    }

    	    p->vlanconf.onuVlanEntryNum = 64;
    	    p->vlanconf.vmtrunkportmask = 0xFFFFFFFF;
    		p->vlanconf.vmtransparentportmask = 0;
    		p->vlanconf.vmtagportmask = 0;
    		p->vlanconf.vmtranslationportmask = 0;
    		p->vlanconf.vmaggportmask = 0;
            p->vlanconf.entryarry[0].vlanid = 1;
    	    p->vlanconf.entryarry[0].allPortMask= 0xFFFFFFFF;
    	    p->vlanconf.entryarry[0].untagPortMask = 0xFFFFFFFF;           
		for(i=1;i<64;i++)
		{
            p->vlanconf.entryarry[i].vlanid = i+100;
    	    p->vlanconf.entryarry[i].allPortMask= 0xFFFFFFFF;
    	    p->vlanconf.entryarry[i].untagPortMask = 0xFFFFFFFF;
		}

        for(i=0;i<4;i++)
        {
		    p->igmpGroupArry[i].gda = 0xE1020101+i;
            p->igmpGroupArry[i].mapPortMask = 0xffffffff;
            p->igmpGroupArry[i].multicastVlanId = 100+i;
        }
	}
	
    return p;        
}
#endif
#if 1
/*供命令行调用，仅在onu 节点产生作用added by luh 2011-12*/
int Get_OnuNumFromOnuRange(ULONG begin_slot, ULONG begin_port, ULONG begin_onuid, ULONG end_slot, ULONG end_port, ULONG end_onuid)
{
    int onu_Num = 0;

    if(begin_slot > PONCARD_LAST || begin_port > MAX_PONPORT_PER_BOARD || begin_onuid > MAXONUPERPON)
        return VOS_ERROR;
    if(end_slot > PONCARD_LAST||end_port > MAX_PONPORT_PER_BOARD || end_onuid > MAXONUPERPON)
        return VOS_ERROR;
    
    if(begin_slot != end_slot)
        return VOS_ERROR;
    if(begin_port > end_port)
        return VOS_ERROR;
    if(begin_port == end_port && begin_onuid > end_onuid)
        return VOS_ERROR;
    onu_Num = (end_port-begin_port)*MAXONUPERPON + (end_onuid - begin_onuid + 1);
    return onu_Num;
}

int Get_OnuNumFromOnuRangeByVty(struct vty* vty, ULONG begin_slot, ULONG begin_port, ULONG begin_onuid, ULONG end_slot, ULONG end_port, ULONG end_onuid)
{
    int onu_Num = 0;
   
    if(SlotCardMayBePonBoardByVty(begin_slot, vty))
    {
        vty_out(vty, "\r\nslot %d is not a PonCard!\r\n", begin_slot);
        return VOS_ERROR;
    }
    if(SlotCardMayBePonBoardByVty(end_slot, vty))
    {
        vty_out(vty, "\r\nslot %d is not a PonCard!\r\n", end_slot);
        return VOS_ERROR;
    }    
    if(begin_port > MAX_PONPORT_PER_BOARD || end_port > MAX_PONPORT_PER_BOARD)
    {
        vty_out(vty, "\r\nPon port id must be less then %d\r\n", MAX_PONPORT_PER_BOARD);
        return VOS_ERROR;
    }
    if(begin_onuid > MAXONUPERPON || end_onuid > MAXONUPERPON)
    {
        vty_out(vty, "\r\nOnu id must be less then %d\r\n", MAXONUPERPON);
        return VOS_ERROR;
    }
    
    if(begin_slot != end_slot)
    {
        vty_out(vty, "\r\nThe command only support single poncard!\r\n");
        return VOS_ERROR;
    }
    if(begin_port > end_port)
    {
        vty_out(vty, "\r\nBegin Onu's id Must be less then the End one!\r\n");        
        return VOS_ERROR;
    }
    if(begin_port == end_port && begin_onuid > end_onuid)
    {
        vty_out(vty, "\r\nBegin Onu's id Must be less then the End one!\r\n");        
        return VOS_ERROR;
    }
    /*暂不支持跨pon 的vlan 配置*/
    /*for(i=begin_slot;i<=end_slot;i++)
            {
            if(SlotCardMayBePonBoardByVty(i, vty) == ROK)
            {
                onu_Num += GetSlotCardPonPortNum(i)*MAXONUPERPON;
        
        GetSlotPonPortPhyRange(end_slot, &begin_ponport, &end_ponport);
        onu_exceptNum += (end_ponport - end_port)*MAXONUPERPON + (MAXONUPERPON - end_onuid);
        if(onu_Num<onu_exceptNum)
            return VOS_ERROR;
        onu_Num -= onu_exceptNum;*/
    onu_Num = (end_port-begin_port)*MAXONUPERPON + (end_onuid - begin_onuid + 1);
    if(!onu_Num)
        VOS_ASSERT(0);
    return onu_Num;
}

int Set_OnuProfile_VlanBat_Mode_Vid(struct vty* vty, int suffix, void *pdata, ULONG begin_onuid, ULONG end_onuid, ULONG vid, ULONG portlist, USHORT mode)
{
    int ret = VOS_OK;
    int i = 0;
               
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = (ONUConfigData_t *)pdata;
        if(onuConfCheckByPtr(suffix, pdata))
        {
            vty_out(vty, "  %%executed command error!\r\n");            
            ret = VOS_ERROR;
        }
        else
        {
            if(p->rules.number_rules < ONU_VLAN_RULE)
            {
                i = p->rules.number_rules;
                if(portlist)
                {
                    p->rules.rule[i].rule_mode = VLAN_BAT_MODE_VLANID_PORT; 
                    p->rules.rule[i].portlist = portlist;
                }
                else
                    p->rules.rule[i].rule_mode = VLAN_BAT_MODE_VLANID;
                p->rules.rule[i].begin_onuid = begin_onuid;
                p->rules.rule[i].end_onuid   = end_onuid;
                p->rules.rule[i].begin_vid   = vid;
                p->rules.rule[i].end_vid     = 0;
                p->rules.rule[i].mode        = mode;
                p->rules.number_rules++;
            }
            else
            {
                vty_out(vty, "Add Error,The Onu-Profile Vlan Rule Number is 16!\r\n");
                ret = VOS_ERROR;
            }
        }
    }
    ONU_CONF_SEM_GIVE
    return ret;
}
int Set_OnuProfile_VlanBat_Mode_Range(struct vty* vty, int suffix, void *pdata, ULONG begin_onuid, ULONG end_onuid, ULONG begin_vid,ULONG end_vid, ULONG onu_step, ULONG mode, ULONG port_step)
{
    int ret = VOS_OK;
    int i = 0;
               
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = (ONUConfigData_t *)pdata;
        if(onuConfCheckByPtr(suffix, pdata))
        {
            vty_out(vty, "  %%executed command error!\r\n");            
            ret = VOS_ERROR;            
        }
        else
        {
            if(p->rules.number_rules < ONU_VLAN_RULE)
            { 
                i = p->rules.number_rules;
                if(port_step)
                {
                    p->rules.rule[i].rule_mode = VLAN_BAT_MODE_VLANRANGE_PORT; 
                    p->rules.rule[i].port_step = port_step;
                }
                else
                    p->rules.rule[i].rule_mode = VLAN_BAT_MODE_VLANRANGE;
                p->rules.rule[i].step        = onu_step;
                p->rules.rule[i].begin_onuid = begin_onuid;
                p->rules.rule[i].end_onuid   = end_onuid;
                p->rules.rule[i].begin_vid   = begin_vid;
                p->rules.rule[i].end_vid     = end_vid; 
                p->rules.rule[i].mode        = mode;
                p->rules.number_rules++;
            }
            else
            {
                vty_out(vty, "Add Error,The Onu-Profile Vlan Rule Number is 16!\r\n");
                ret = VOS_ERROR;
            }
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int Set_OnuProfile_VlanBat_Mode_RangeByPortId(struct vty* vty, int suffix, void *pdata, ULONG begin_onuid, ULONG end_onuid, ULONG begin_vid,ULONG end_vid, ULONG onu_step, ULONG mode, ULONG port_id)
{
    int ret = VOS_OK;
    int i = 0;
               
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = (ONUConfigData_t *)pdata;
        if(onuConfCheckByPtr(suffix, pdata))
        {
            vty_out(vty, "  %%executed command error!\r\n");            
            ret = VOS_ERROR;            
        }
        else
        {
            if(p->rules.number_rules < ONU_VLAN_RULE)
            { 
                i = p->rules.number_rules;
                p->rules.rule[i].rule_mode = VLAN_BAT_MODE_VLANRANGE_ONEPORT; 
                p->rules.rule[i].portlist = 1 << (port_id-1);
                p->rules.rule[i].step        = onu_step;
                p->rules.rule[i].begin_onuid = begin_onuid;
                p->rules.rule[i].end_onuid   = end_onuid;
                p->rules.rule[i].begin_vid   = begin_vid;
                p->rules.rule[i].end_vid     = end_vid; 
                p->rules.rule[i].mode        = mode;
                p->rules.number_rules++;
            }
            else
            {
                vty_out(vty, "Add Error,The Onu-Profile Vlan Rule Number is 16!\r\n");
                ret = VOS_ERROR;
            }
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int Show_OnuProfile_VlanBat_RuleList(struct vty* vty, ULONG slot, ULONG port, ULONG onuid)
{
    int i;
    ULONG   begin_slot, begin_port, begin_onuid;
    ULONG   end_slot, end_port, end_onuid;
    char portlist_str[80] = "";
    char beginonuid_str[20] = "";
    char endonuid_str[20] = "";
    	short int ponID = GetPonPortIdxBySlot((short int)slot, (short int)port);
    
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponID, onuid-1));        
        vty_out(vty, "\r\nVlan Configuration Rules List:\r\n");
        vty_out(vty, "%3s%12s%10s%10s%8s%9s%10s%9s%8s%10s\r\n", "No", "begin_onuid", "end_onuid", "begin_vid", "end_vid", "onu_step", "port_step", "  mode  ", " Status ", "portlist");
        vty_out(vty, "%3s%12s%10s%10s%8s%9s%10s%9s%8s%10s\r\n", "--", "-----------", "---------", "---------", "-------", "--------", "---------", "--------", "--------", "--------");

        for(i=0;i<p->rules.number_rules;i++)
        {
            if(p->rules.rule[i].rule_mode)
            {
                int valid = 0;
                begin_slot = GET_PONSLOT(p->rules.rule[i].begin_onuid);
                begin_port = GET_PONPORT(p->rules.rule[i].begin_onuid);
                begin_onuid = GET_ONUID(p->rules.rule[i].begin_onuid);
                end_slot = GET_PONSLOT(p->rules.rule[i].end_onuid);
                end_port = GET_PONPORT(p->rules.rule[i].end_onuid);
                end_onuid = GET_ONUID(p->rules.rule[i].end_onuid);
                if(Get_OnuNumFromOnuRange(slot,port,onuid,end_slot,end_port,end_onuid) != VOS_ERROR&&
                    Get_OnuNumFromOnuRange(begin_slot,begin_port,begin_onuid,slot,port,onuid) != VOS_ERROR)
                    valid = 1;
                VOS_Sprintf(beginonuid_str, "%d/%d/%d", begin_slot, begin_port, begin_onuid);
                VOS_Sprintf(endonuid_str, "%d/%d/%d", end_slot, end_port, end_onuid);
                if(p->rules.rule[i].portlist)
                {
                    portListLongToString(p->rules.rule[i].portlist, portlist_str);
                    vty_out(vty, "%3d%12s%10s%10d%8d%9d%10d%9s%8s%10s\r\n", i+1, beginonuid_str, endonuid_str, p->rules.rule[i].begin_vid, p->rules.rule[i].end_vid, p->rules.rule[i].step, p->rules.rule[i].port_step, p->rules.rule[i].mode == 1?"  tag   ":"untagged", valid?" Valid ":"Invalid",portlist_str);
                }                 
                else
                    vty_out(vty, "%3d%12s%10s%10d%8d%9d%10d%9s%8s%10s\r\n", i+1, beginonuid_str, endonuid_str, p->rules.rule[i].begin_vid, p->rules.rule[i].end_vid, p->rules.rule[i].step, p->rules.rule[i].port_step, p->rules.rule[i].mode == 1?"  tag   ":"untagged", valid?" Valid ":"Invalid", "all");
            }
        }
        vty_out(vty, "\r\n");
    }
    ONU_CONF_SEM_GIVE

    return VOS_OK;
}

int Show_OnuProfile_VlanBat_RuleListByPtr(struct vty* vty, int suffix, void *pd)
{
    int ret = VOS_OK;
    int i;
    ULONG   begin_slot, begin_port, begin_onuid;
    ULONG   end_slot, end_port, end_onuid;
    char portlist_str[80] = "";
    char beginonuid_str[20] = "";
    char endonuid_str[20] = "";
    
    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd))
            ret = VOS_ERROR;
        else
        {
            ONUConfigData_t *p = (ONUConfigData_t *)pd;
            vty_out(vty, "\r\nVlan Configuration Rules List:\r\n");
            vty_out(vty, "%3s%12s%10s%10s%8s%9s%10s%9s%10s\r\n", "No", "begin_onuid", "end_onuid", "begin_vid", "end_vid", "onu_step", "port_step", "  mode  ",  "portlist");
            vty_out(vty, "%3s%12s%10s%10s%8s%9s%10s%9s%10s\r\n", "--", "-----------", "---------", "---------", "-------", "--------", "---------", "--------",  "--------");

            for(i=0;i<p->rules.number_rules;i++)
            {
                if(p->rules.rule[i].rule_mode)
                {
                    begin_slot = GET_PONSLOT(p->rules.rule[i].begin_onuid);
                    begin_port = GET_PONPORT(p->rules.rule[i].begin_onuid);
                    begin_onuid = GET_ONUID(p->rules.rule[i].begin_onuid);
                    end_slot = GET_PONSLOT(p->rules.rule[i].end_onuid);
                    end_port = GET_PONPORT(p->rules.rule[i].end_onuid);
                    end_onuid = GET_ONUID(p->rules.rule[i].end_onuid);
                    VOS_Sprintf(beginonuid_str, "%d/%d/%d", begin_slot, begin_port, begin_onuid);
                    VOS_Sprintf(endonuid_str, "%d/%d/%d", end_slot, end_port, end_onuid);
                    if(p->rules.rule[i].portlist)
                    {
                        portListLongToString(p->rules.rule[i].portlist, portlist_str);
                        vty_out(vty, "%3d%12s%10s%10d%8d%9d%10d%9s%10s\r\n", i+1, beginonuid_str, endonuid_str, p->rules.rule[i].begin_vid, p->rules.rule[i].end_vid, p->rules.rule[i].step,p->rules.rule[i].port_step,p->rules.rule[i].mode == 1?"  tag   ":"untagged", portlist_str);
                    }                 
                    else
                        vty_out(vty, "%3d%12s%10s%10d%8d%9d%10d%9s%10s\r\n", i+1, beginonuid_str, endonuid_str, p->rules.rule[i].begin_vid, p->rules.rule[i].end_vid, p->rules.rule[i].step,p->rules.rule[i].port_step,p->rules.rule[i].mode == 1?"  tag   ":"untagged", "all");
                }
            }
            vty_out(vty, "\r\n");
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int Delete_onuProfile_VlanBat_Rule(struct vty* vty, int suffix, void *pd, ULONG num)
{
    int i, ret = VOS_ERROR;
        
    ONU_CONF_SEM_TAKE
    {
        if(onuConfCheckByPtr(suffix, pd))
        {
            vty_out(vty, "  %%executed command error!\r\n");
        }
        else
        {
            ONUConfigData_t *p =  (ONUConfigData_t *)pd;
            if(num)
            {
                if(num <= p->rules.number_rules)
                {
                    if(p->rules.rule[num-1].rule_mode)
                    {
                        int num_temp = p->rules.number_rules;
                        for(i=num-1;i<num_temp-1;i++)
                        {
                            p->rules.rule[i].rule_mode = p->rules.rule[i+1].rule_mode;
                            p->rules.rule[i].begin_onuid = p->rules.rule[i+1].begin_onuid;
                            p->rules.rule[i].begin_vid = p->rules.rule[i+1].begin_vid;
                            p->rules.rule[i].end_onuid = p->rules.rule[i+1].end_onuid;
                            p->rules.rule[i].end_vid = p->rules.rule[i+1].end_vid;
                            p->rules.rule[i].portlist = p->rules.rule[i+1].portlist;
                            p->rules.rule[i].step = p->rules.rule[i+1].step;
                            p->rules.rule[i].mode = p->rules.rule[i+1].mode;
                            p->rules.rule[i].port_step = p->rules.rule[i+1].port_step;/*拷贝过程中丢配置，问题单16445*/
                        }
                        VOS_MemZero(&p->rules.rule[num_temp-1],sizeof(onu_vlan_bat_rule_t));
                        p->rules.number_rules--;
                        ret = VOS_OK;
                    }
                }
                else
                {
                    vty_out(vty, "Rule %d is not exist!\r\n", num);
                }
            }
            else
            {
                VOS_MemZero(&p->rules, sizeof(onu_vlan_bat_rules_t));
                ret = VOS_OK;
            }
        }

    }
    ONU_CONF_SEM_GIVE
    return ret;
}
int IsBelongToVlanRules(ULONG vid, ULONG slot, ULONG port, ULONG onuid)
{
    int ret = 0, i, onu_num = 0;
    short int olt_id = GetPonPortIdxBySlot(slot, port);
    short int onu_id = onuid - 1;
    ULONG begin_slot,begin_port,begin_onuid;
    ULONG end_slot,end_port,end_onuid;  
    ULONG v_vid = 0, e_vid = 0;
    
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(olt_id, onu_id));
        for(i=0;i<p->rules.number_rules;i++)
        {
            if(p->rules.rule[i].rule_mode)
            {
                begin_slot = GET_PONSLOT(p->rules.rule[i].begin_onuid);
                begin_port = GET_PONPORT(p->rules.rule[i].begin_onuid);
                begin_onuid = GET_ONUID(p->rules.rule[i].begin_onuid);
                end_slot = GET_PONSLOT(p->rules.rule[i].end_onuid);
                end_port = GET_PONPORT(p->rules.rule[i].end_onuid);
                end_onuid = GET_ONUID(p->rules.rule[i].end_onuid);
                onu_num = Get_OnuNumFromOnuRange(slot,port,onuid,end_slot,end_port,end_onuid);
                if(onu_num == VOS_ERROR)
                    continue;
                onu_num = Get_OnuNumFromOnuRange(begin_slot,begin_port,begin_onuid,slot,port,onuid);
                if(onu_num == VOS_ERROR)
                    continue;
                switch(p->rules.rule[i].rule_mode)
                {
                    case VLAN_BAT_MODE_VLANID:
                    case VLAN_BAT_MODE_VLANID_PORT:                        
                        if(p->rules.rule[i].begin_vid == vid)
                            ret = 1;
                        break;
                    case VLAN_BAT_MODE_VLANRANGE:
                    case VLAN_BAT_MODE_VLANRANGE_ONEPORT:                        
                        v_vid = p->rules.rule[i].begin_vid + (onu_num - 1)*p->rules.rule[i].step;
                        if(v_vid == vid)
                            ret = 1;
                        break;
                    case VLAN_BAT_MODE_VLANRANGE_PORT:
                        v_vid = p->rules.rule[i].begin_vid + (onu_num - 1)*p->rules.rule[i].step;
                        e_vid = v_vid + p->rules.rule[i].step;
                        for(;v_vid<e_vid;)
                        {
                            if(vid == v_vid)
                            {
                                ret = 1;
                                break;
                            }
                            v_vid += p->rules.rule[i].port_step;
                        }
                        break;
                    default:
                        break;
                }
                if(ret)
                    break;
            }
        }
    }
    ONU_CONF_SEM_GIVE
    return ret;
}
int SearchOnuPortByVlanRules(ULONG vid, ULONG slot, ULONG port, ULONG onuid, ULONG *portIdx)
{
    int ret = VOS_ERROR, i, onu_num = 0;
    short int olt_id = GetPonPortIdxBySlot(slot, port);
    short int onu_id = onuid - 1;
    ULONG begin_slot,begin_port,begin_onuid;
    ULONG end_slot,end_port,end_onuid;  
    ULONG v_vid = 0, e_vid = 0;
    ULONG portlist= 0;
    ULONG portNum = 1;
    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(olt_id, onu_id));
        for(i=0;i<p->rules.number_rules;i++)
        {
            if(p->rules.rule[i].rule_mode)
            {
                begin_slot = GET_PONSLOT(p->rules.rule[i].begin_onuid);
                begin_port = GET_PONPORT(p->rules.rule[i].begin_onuid);
                begin_onuid = GET_ONUID(p->rules.rule[i].begin_onuid);
                end_slot = GET_PONSLOT(p->rules.rule[i].end_onuid);
                end_port = GET_PONPORT(p->rules.rule[i].end_onuid);
                end_onuid = GET_ONUID(p->rules.rule[i].end_onuid);
                onu_num = Get_OnuNumFromOnuRange(slot,port,onuid,end_slot,end_port,end_onuid);
                if(onu_num == VOS_ERROR)
                    continue;
                onu_num = Get_OnuNumFromOnuRange(begin_slot,begin_port,begin_onuid,slot,port,onuid);
                if(onu_num == VOS_ERROR)
                    continue;
                switch(p->rules.rule[i].rule_mode)
                {
                    case VLAN_BAT_MODE_VLANID:
                        if(p->rules.rule[i].begin_vid == vid)
                        {
                            portNum = 1;
                            ret = VOS_OK;                                                                
                        }
                        break;
                    case VLAN_BAT_MODE_VLANID_PORT:                        
                        if(p->rules.rule[i].begin_vid == vid)
                        {
                            int loop = 1;
                            portlist = p->rules.rule[i].portlist;
                            while(portlist)
                            {
                                if(portlist&1)
                                {
                                    portNum = loop;
                                    ret = VOS_OK;                                    
                                    break;
                                }
                                loop++;
                                portlist>>=1;
                            }                            
                        }
                        break;
                    case VLAN_BAT_MODE_VLANRANGE:
                        v_vid = p->rules.rule[i].begin_vid + (onu_num - 1)*p->rules.rule[i].step;
                        if(v_vid == vid)
                        {
                            portNum = 1;
                            ret = VOS_OK;                                                                
                        }
                        break;
                    case VLAN_BAT_MODE_VLANRANGE_PORT:
                        v_vid = p->rules.rule[i].begin_vid + (onu_num - 1)*p->rules.rule[i].step;
                        e_vid = v_vid + p->rules.rule[i].step;
                        for(;v_vid<e_vid;)
                        {
                            if(vid == v_vid)
                            {
                                ret = VOS_OK;
                                break;
                            }
                            portNum++;
                            v_vid += p->rules.rule[i].port_step;
                        }
                        break;
                    case VLAN_BAT_MODE_VLANRANGE_ONEPORT:
                        v_vid = p->rules.rule[i].begin_vid + (onu_num - 1)*p->rules.rule[i].step;
                        if(vid == v_vid)
                        {
                            int loop = 1;
                            portlist = p->rules.rule[i].portlist;
                            while(portlist)
                            {
                                if(portlist&1)
                                {
                                    portNum = loop;
                                    ret = VOS_OK;                                    
                                    break;
                                }
                                loop++;
                                portlist>>=1;
                            }                            
                        }
                        break;                        
                    default:
                        break;
                }
                if(ret == VOS_OK)
                {
                    *portIdx = portNum;
                    break;
                }
            }
        }
    }
    ONU_CONF_SEM_GIVE
    return ret;
}

#endif
#if 1
int search_onu_vlan(short int PonPortIdx, short int OnuIdx, short int vid)
{
    ULONG allmask = 0, untagmask = 0;
    if(get_onuconf_vlanPortlist(PonPortIdx, OnuIdx, vid, &allmask, &untagmask) != RERROR)
        return VOS_OK;
    else if(IsBelongToVlanRules(vid, GetCardIdxByPonChip(PonPortIdx), GetPonPortByPonChip(PonPortIdx), OnuIdx+1))
        return VOS_OK;
    else
        return VOS_ERROR;
}
int get_onu_allvlanstr(short int PonPortIdx, short int OnuIdx, USHORT *vlan, int *vlan_no)
{
    int i = 0, j = 0;
    int ret = VOS_ERROR;
    ULONG slot = GetCardIdxByPonChip(PonPortIdx);
    ULONG port = GetPonPortByPonChip(PonPortIdx);
    ULONG onuid = OnuIdx + 1;
    ULONG   begin_slot, begin_port, begin_onuid;
    ULONG   end_slot, end_port, end_onuid;
    USHORT vlanlist[ONU_MAX_VLAN] = {0};
    int vlan_num = 0;
    int onu_num = 0;
    short int v_vid = 0;
    int temp_vlan = 0;
    ULONG fenum, devidx = 0;

    devidx = MAKEDEVID(slot, port, onuid);
    /*如果获取不到端口，则默认32个口*/
    if(getDeviceCapEthPortNum(devidx, &fenum) != VOS_OK)
        fenum = 32;
    
	ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *p = getOnuConfFromHashBucket(
                ONU_CONF_NAME_PTR_GET(PonPortIdx, OnuIdx));

        if (p && (sfun_getOnuConfVlanMode(p) == ONU_CONF_VLAN_MODE_TRUNK))
        {
            /*Get general vlan configuration*/
            int VlanentryNum = p->vlanconf.onuVlanEntryNum;
            for (i = 0; i < VlanentryNum; i++)
            {
                vlanlist[vlan_num] = p->vlanconf.entryarry[i].vlanid;
                vlan_num++;
            }
            
            /*Get bat vlan configuration*/
            for(i=0;i<p->rules.number_rules;i++)
            {
                if(p->rules.rule[i].rule_mode)
                {
                    int valid = 0;
                    begin_slot = GET_PONSLOT(p->rules.rule[i].begin_onuid);
                    begin_port = GET_PONPORT(p->rules.rule[i].begin_onuid);
                    begin_onuid = GET_ONUID(p->rules.rule[i].begin_onuid);
                    end_slot = GET_PONSLOT(p->rules.rule[i].end_onuid);
                    end_port = GET_PONPORT(p->rules.rule[i].end_onuid);
                    end_onuid = GET_ONUID(p->rules.rule[i].end_onuid);
                    onu_num = Get_OnuNumFromOnuRange(slot,port,onuid,end_slot,end_port,end_onuid);
                    if(onu_num == VOS_ERROR)
                        continue;
                    onu_num = Get_OnuNumFromOnuRange(begin_slot,begin_port,begin_onuid,slot,port,onuid);
                    if(onu_num == VOS_ERROR)
                        continue;
                                                        
                    v_vid   = p->rules.rule[i].begin_vid + (onu_num - 1)*p->rules.rule[i].step;
                     
                    if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID || p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANID_PORT)
                    {
                        for(j=0;j<vlan_num;j++)
                        {
                            if(vlanlist[j] == p->rules.rule[i].begin_vid)
                        		break;
                        }
                        if(j >= vlan_num)
                        {
                            vlanlist[vlan_num] = p->rules.rule[i].begin_vid;
                            vlan_num++;
                        }
                    }                            
                    else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE)
                    {
                        for(j=0;j<vlan_num;j++)
                        {
                            if(vlanlist[j] == p->rules.rule[i].begin_vid)
                        		break;
                        }
                        if(j >= vlan_num)
                        {
                            vlanlist[vlan_num] = v_vid;
                            vlan_num++;
                        }
                    }
                    else if(p->rules.rule[i].rule_mode == VLAN_BAT_MODE_VLANRANGE_PORT)
                    {
                        int t_vid = v_vid;
                        int m = 0;
                        for(m=0;m<fenum;m++)
                        {
                            if(t_vid >= (v_vid + p->rules.rule[i].step))
                                t_vid = v_vid;

                            for(j=0;j<vlan_num;j++)
                            {
                                if(vlanlist[j] == t_vid)
                            		break;
                            }
                            if(j >= vlan_num)
                            {
                                vlanlist[vlan_num] = t_vid;
                                vlan_num++;                                        
                            }
                            t_vid += p->rules.rule[i].port_step;                 
                        }
                    }

                }
            }

            /*sort the vlan config*/
            for(i=0;i<vlan_num-1;i++)
            {
                for(j=0;j<vlan_num-1-i;j++)
                {
                    if(vlanlist[j]>vlanlist[j+1])
                    {
                        temp_vlan = vlanlist[j];
                        vlanlist[j] = vlanlist[j+1];
                        vlanlist[j+1] = temp_vlan;
                    }
                }
            }

            /*copy the config*/
            if(vlan_num)
            {
                VOS_MemCpy(vlan, vlanlist, sizeof(USHORT)*vlan_num);    
                *vlan_no = vlan_num;
                ret = VOS_OK;
            }
        }
    }
    ONU_CONF_SEM_GIVE
    return ret;
}
#endif
#if 1
int snmp_access_prepare(int type, snmp_acc_record_info_t info)
{
    int ret = VOS_ERROR;
    short int PonPortIdx = 0;
    short int OnuIdx = 0;
    switch (type)
    {
        case SNMP_ACC_ONU:
            /*modified by luh 2012-11-21, 解决关联共享文件的onu，在线修改后其关联关系从共
                        享文件中删除，当该共享文件没有关联onu时，不能手动删除，原因是由于
                        网管在线修改权限没有释放*/
#if 1
            /*added by luh 2012-12-11, 私有Onu下挂交换机时不能执行post，且网管对私有onu产生的私有文件实际是原共享文件的拷贝，没有任何意义，此处不再产生*/
            PonPortIdx = GetPonPortIdxBySlot(info.onuinfo.slot, info.onuinfo.port);
            OnuIdx = info.onuinfo.onuid-1;
            if(IsCtcOnu(PonPortIdx, OnuIdx))
                ret = cli_onuDumpConfigForOnuNodeVty(NULL, info.onuinfo.slot, info.onuinfo.port, info.onuinfo.onuid);
            else
                ret = VOS_OK;
#else
            {
                int type, argv0, argv1;
                type = sfun_getSnmpAccessType(&argv0, &argv1);
                if(openOnuConfigFile(ONU_CONF_NAME_PTR_GET(argv0, argv1), ONU_CONF_OPEN_FLAG_WRITE))
                {
                    ret = cli_onuDumpConfigForOnuNodeVty(NULL, info.onuinfo.slot, info.onuinfo.port, info.onuinfo.onuid);
                }
            }
#endif
        break;
        case SNMP_ACC_PROFILE:
        {
#if 0
            ONUConfigData_t *pdata = getOnuConfFromHashBucket(info.profile), *pnew = NULL;
#else
            ONUConfigData_t *pdata = openOnuConfigFile(info.profile, ONU_CONF_OPEN_FLAG_WRITE), *pnew = NULL;
#endif
            if(pdata)
            {
                if(VOS_StrCmp(pdata->confname, DEFAULT_ONU_CONF))
                {
                    char filename[80] = "";
                    VOS_Sprintf(filename, "*auto%s", pdata->confname);

                    if(!isOnuConfExist(filename))
                    {
                        pnew = onuconf_malloc(ONU_CONF_MEM_DATA_ID);
                        if(pnew)
                        {
                            VOS_MemCpy(pnew, pdata, sizeof(ONUConfigData_t));
                            VOS_StrCpy(pnew->confname, filename);

                            if(OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, pnew->confname, NULL, pnew)!=VOS_OK)
                            {
                                onuconf_free(pnew, ONU_CONF_MEM_DATA_ID);
                            }
                            else
                            {
                                /*
                                vty->index = IFM_ONU_PROFILE_CREATE_INDEX(pnew->confname);
                                vty->onuconfptr = pnew;
                                vty->orig_profileptr = pdata;
                                */
                                ret = VOS_OK;
                                sendAutoOnuProfileAddMsg(filename);
                            }
                        }
                    }
                    else
                        ret = VOS_OK;
                }
            }
        }
            break;
        default:
        break;
    }

    return ret;
}

int snmp_access_post(int type, snmp_acc_record_info_t info)
{
    int ret = VOS_OK;

    int ltype, argv0, argv1;


    ltype = sfun_getSnmpAccessType(&argv0, &argv1);

    if(!ltype)
        return ret;
/*
    g_snmp_acc_record.msg_type = 0;
    g_snmp_acc_record.snmpacctype = 0;
    VOS_MemZero( &g_snmp_acc_record.snmpaccinfo, sizeof(snmp_acc_record_info_t));
*/
    if(ltype == SNMP_ACC_ONU )
    {                    
        ONU_CONF_SEM_TAKE
        {
            char sz[80] = "";
            char *name = NULL;
            ONUConfigData_t *pd = NULL, *pb = NULL;

            name = ONU_CONF_NAME_PTR_GET(argv0, argv1);
            /*modi by luh@2014-10-31*/
            if(name)
            {
                if(!onuConfIsSharedByName(name))
                {
                    VOS_Sprintf(sz, "*auto%s", name);
                    pd = getOnuConfFromHashBucket(name);
                    pb = getOnuConfFromHashBucket(sz);
                    closeOnuConfigFile(name);

                    if(pd && (!pd->share) && pb )
                    {
                        if(onuconfCompare(pb, pd))
                        {
                        /*ULONG uv[4] = {0, 0, pd->confname, EM_ONU_PROFILE_MODIFIED};*/
						ULONG uv[4] = {0, 0, 0, 0};
						uv[2] = (ULONG)pd->confname;
        				uv[3] = EM_ONU_PROFILE_MODIFIED;
                            processEventCallbackList(EVENT_ONUPROFILE_MODIFIED, uv);
                        }
                    }

                    if(pb)
                    {
                        closeOnuConfigFile(pd->confname);
                        OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, pb->confname, NULL, NULL);                                    
                    }   
                    if(!pd)
                        ret = VOS_ERROR;
                }
                else
                {
                    /*do nothing*/
                }
            }
            else
                ret = VOS_ERROR;

            if(ret == VOS_ERROR)
            {
                if(name)
                    sys_console_printf("profile name : %s share: %s\r\n", name, onuConfIsSharedByName(name)?"yes":"no");
                else
                    sys_console_printf("wrong profile name : NULL, ponid %d onuid %d\r\n", argv0, argv1);
            }
        }
        ONU_CONF_SEM_GIVE
    }
    else
    {
        closeOnuConfigFile(((ONUConfigData_t*)argv1)->confname);
        closeOnuConfigFile(info.profile);
    }

    return ret;
}

void init_snmp_access_record()
{
    registerSnmpAccApis(snmp_access_prepare, snmp_access_post);
}

LONG onuconf_CalculateFileId(const char *filename)
{
    long idx = getOnuConfHashIndex( filename ), ret = 0;
    int i = 0;
    element_onuconfhashbucket_t *p = NULL;

    p = (element_onuconfhashbucket_t*) (*(g_onuConfBucket + idx));

    while (p)
    {
        ONUConfigData_t * pd = (ONUConfigData_t *) p->confdata;
        if (pd && (!VOS_StriCmp(pd->confname, filename)))
            break;
        p = p->next;
        i++;
    }

    if(p)
        ret = ((1<<24)|(idx<<8)|i);

    return ret;
}
#define T_BUF_SIZE 0x2000000

int init_fullOnuConfs()
{
    int i = 0;
    for (i = 0; i < ONU_MAX_CONF_NUM_USE; i++)
    {
        ONUConfigData_t *pd = test_CTC_OnuProfile_full_init();
        if (pd)
        {
            /*   char *pfile = VOS_Malloc(128*1024, MODULE_RPU_CTC);*/
            VOS_Sprintf(pd->confname, "test_ctc%d", i + 1);
            pd->share = 1;
            setOnuConfToHashBucket(pd->confname, pd);
        }
    }

    return VOS_OK;
}

int test_saveConfs()
{
    int i=0;
    unsigned long length = 0;
    struct vty *vty = NULL;

    char *szsrc = (char*)g_malloc(T_BUF_SIZE);

    vty = vty_new();
    vty->fd = _CL_FLASH_FD_;
    vty->type = VTY_FILE;

    if(szsrc)
    {

        VOS_MemZero(szsrc, T_BUF_SIZE);

        for(i=0; i<ONU_MAX_CONF_NUM_USE;i++)
        {
            ONUConfigData_t *pd = test_CTC_OnuProfile_full_init();
            if(pd)
            {
             /*   char *pfile = VOS_Malloc(128*1024, MODULE_RPU_CTC);*/
                VOS_Sprintf(pd->confname, "test_ctc%d", i+1);
                pd->share = 1;
                setOnuConfToHashBucket(pd->confname, pd);

                generateOnuConfClMemFile(pd->confname, -1, vty, szsrc+length,0,0,0);
                length += vty->obuf->length;

                vty_buffer_reset(vty);

            }
        }

        {
            char *dest = NULL;
            long int destlen = 0, bdestlen = 0;
            int ret = 0;
            ret = defLzmaCompress(&dest, &destlen, szsrc, length, lzma_malloc, lzma_free, 0);
            if(ret == SZ_OK)
            {
                char * newsrc = NULL;
                long newlen = 0;

                g_free(szsrc);

                newsrc = g_malloc(T_BUF_SIZE);
                VOS_MemZero(newsrc, T_BUF_SIZE);
                newlen = T_BUF_SIZE;
                bdestlen = destlen;

                ret = defLzmaUncompress(newsrc, &newlen, dest, &destlen);

                if(ret == SZ_OK && bdestlen == destlen)
                {

                }

                if (dest) g_free(dest);
                g_free(newsrc);
            }
        }
    }

    vty_free(vty);

    return ROK;

}

/*--------------------------------- 事件回调处理相关API的实现 -------------------------------*/
event_callback_func g_event_callbacks[EVENT_MAX_NUM][MAX_EVENT_CALLBACK_CLIENTS];

void initEventCallbackFunctions()
{
    VOS_MemSet(g_event_callbacks, 0, sizeof(g_event_callbacks));
}

int registerEventCallback(int eventid, event_callback_func  pfunc)
{
    int ret = VOS_ERROR;

    if(CHECK_EVENTID_VALID(eventid))
    {
        int i = 0;
        for(;i<MAX_EVENT_CALLBACK_CLIENTS; i++)
        {
            if(!g_event_callbacks[eventid][i])
            {
                g_event_callbacks[eventid][i] = pfunc;
                ret = i;
                break;
            }
        }
    }

    return ret;
}

int unregisterEventCallback(int eventid, int callback_handle)
{
    int ret = VOS_ERROR;

    if(CHECK_EVENTID_VALID(eventid) && CHECK_EVENT_HANDLE_VALID(callback_handle))
    {
        int i=0;
        for(;i<MAX_EVENT_CALLBACK_CLIENTS; i++)
        {
            g_event_callbacks[eventid][callback_handle] = 0;
            ret = VOS_OK;
            break;
        }
    }

    return ret;
}

void processEventCallbackList(int eventid, ULONG argv[4])
{
    if(CHECK_EVENTID_VALID(eventid))
    {
        int i = 0;
        for(; i<MAX_EVENT_CALLBACK_CLIENTS; i++)
            if(g_event_callbacks[eventid][i])
                (*g_event_callbacks[eventid][i])(argv);
    }
}


int fcb_autoOnuProfileApplyEvent(ULONG argv[4])
{
    switch(argv[2])
    {
        case MIB_ONU_PROFILE_APPLY:     /*apply*/
        {
            char *name = NULL;
            sendAutoOnuProfileCancelCheckMsg(argv[3]);
            name = VOS_StrStr(argv[3], "*auto");
            if(name)
            {
                if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
                {
                    OnuProfile_Action_ByCode(OnuProfile_Add_SyncBroadcast, 0, 0, 0, name, NULL, NULL);                    
                }

                name += 5;
                OnuProfile_Action_ByCode(OnuProfile_Modify, 0, 0, 0, (char*)argv[3], name, NULL);        
                OnuProfile_Action_ByCode(OnuProfile_Modify_SyncBroadcast, 0, 0, 0, (char*)argv[3], name, NULL);        
            }
        }
            break;
        case MIB_ONU_PROFILE_CANCEL:     /*cancel*/
            sendAutoOnuProfileDeleteMsg(argv[3]);
            break;
        default:
            break;
    }

    return VOS_OK;
}

int fcb_onuProfileCreation(ULONG argv[4])
{
    int ret = VOS_ERROR;

    const char *name = (const char*)argv[2];
    onu_profile_op_code_t code = argv[3];
    cl_vector v = g_onuConfFileOpRecord;

    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER&&
            g_SystemLoadConfComplete == _VOS_SYSTEM_RUNNING_ &&
            v && name && !VOS_StrStr(name, "*auto")
            && VOS_StrCmp(name, DEFAULT_ONU_CONF) && code < EM_ONU_PROFILE_INVALID)
    {
        int i;
        onu_profile_op_record_t * p = NULL, *pnew = NULL;

        pnew = VOS_Malloc(sizeof(onu_profile_op_record_t), MODULE_RPU_CTC);

        if(pnew)
        {
            VOS_MemZero(pnew->filename, sizeof(pnew->filename));
            pnew->op = code;
            VOS_StrnCpy(pnew->filename, name, ONU_CONFIG_NAME_LEN);

            for(i=0; i<cl_vector_max(v); i++)
            {
                p = cl_vector_slot(v, i);
                if(p)
                {
                    if(!VOS_StrCmp(p->filename, name))
                    {
                        VOS_Free(p);
                        cl_vector_unset(v, i);
                    }
                }
            }

            cl_vector_set(v, pnew);

            ret = VOS_OK;
        }

    }

    return ret;
}

int fcb_onuProfileModified(ULONG argv[4])
{
    int ret = VOS_ERROR;

    const char *name = (const char*)argv[2];
    onu_profile_op_code_t code = argv[3];
    cl_vector v = g_onuConfFileOpRecord;

    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER &&
            g_SystemLoadConfComplete == _VOS_SYSTEM_RUNNING_ &&
            v && name && !VOS_StrStr(name, "*auto")
            && VOS_StrCmp(name, DEFAULT_ONU_CONF) && code < EM_ONU_PROFILE_INVALID)
    {
        int i;
        onu_profile_op_record_t * p = NULL, *pnew = NULL;

        pnew = VOS_Malloc(sizeof(onu_profile_op_record_t), MODULE_RPU_CTC);

        if(pnew)
        {
            int fc = 0;
            VOS_MemZero(pnew->filename, sizeof(pnew->filename));
            pnew->op = code;
            VOS_StrnCpy(pnew->filename, name, ONU_CONFIG_NAME_LEN);

            VOS_TaskLock();
            for(i=0; i<cl_vector_max(v); i++)
            {
                p = cl_vector_slot(v, i);
                if(p)
                {
                    if(!VOS_StrCmp(p->filename, name) && (p->op == EM_ONU_PROFILE_MODIFIED))
                    {
                        fc = 1;
                        break;
                    }
                }
            }

            if(!fc)
                cl_vector_set(v, pnew);
            else
                VOS_Free(pnew);

            VOS_TaskUnlock();

            ret = VOS_OK;
        }

    }

    return ret;
}

int fcb_onuProfileDelete(ULONG argv[4])
{
    int ret = VOS_ERROR;

    const char *name = (const char*)argv[2];
    onu_profile_op_code_t code = argv[3];
    cl_vector v = g_onuConfFileOpRecord;

    if(SYS_LOCAL_MODULE_WORKMODE_ISMASTER &&
            g_SystemLoadConfComplete == _VOS_SYSTEM_RUNNING_ && v && name && !VOS_StrStr(name, "*auto")
            && VOS_StrCmp(name, DEFAULT_ONU_CONF) && code < EM_ONU_PROFILE_INVALID)
    {
        int i;
        onu_profile_op_record_t * p = NULL, *pnew = NULL;

        pnew = VOS_Malloc(sizeof(onu_profile_op_record_t), MODULE_RPU_CTC);

        if(pnew)
        {
            int fc = 0;

            VOS_MemZero(pnew->filename, sizeof(pnew->filename));
            pnew->op = code;
            VOS_StrnCpy(pnew->filename, name, ONU_CONFIG_NAME_LEN);

            for(i=0; i<cl_vector_max(v); i++)
            {
                VOS_TaskLock();
                p = cl_vector_slot(v, i);
                if(p)
                {
                    if(!VOS_StrCmp(p->filename, name))
                    {
                        if(p->op == EM_ONU_PROFILE_CREATED)
                        {                                                       
                            fc = 1;
                        }
                        VOS_Free(p);                         
                        cl_vector_unset(v, i);
                    }
                }
                VOS_TaskUnlock();
            }

            if(!fc)
            {
                VOS_TaskLock();
                cl_vector_set(v, pnew);
                VOS_TaskUnlock();
            }
            else
                VOS_Free(pnew);

            ret = VOS_OK;
        }

    }

    return ret;
}

int onuconf_SetDirtyFlag(const char *name)
{
    int ret = VOS_ERROR;

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(name);
        if(pd)
        {
            pd->dirty = 1;
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int onuconf_ClrDirtyFlag(const char *name)
{
    int ret = VOS_ERROR;

    ONU_CONF_SEM_TAKE
    {
        ONUConfigData_t *pd = getOnuConfFromHashBucket(name);
        if(pd)
        {
            pd->dirty = 0;
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}

int onuconf_GetSaveFlag(void)
{
    int i;
    int ret = VOS_ERROR;

    cl_vector v;

    if ( NULL != (v = g_onuConfFileOpRecord) )
    {
        VOS_TaskLock();
        if(cl_vector_count(v))
        {
            int j, c = 0;
            for(j=0; j<cl_vector_max(v); j++)
            {
                onu_profile_op_record_t *p = cl_vector_slot(v, j);
                if(p)
                {
                    switch(p->op)
                    {
                    case EM_ONU_PROFILE_CREATED:
                        ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("op %d:    %s    created\r\n", ++c, p->filename));
                        break;
                    case EM_ONU_PROFILE_MODIFIED:
                    	ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("op %d:    %s    modified\r\n", ++c, p->filename));
                        break;
                    case EM_ONU_PROFILE_DELETED:
                    	ONUCONF_TRACE(ONU_CONF_DEBUG_LVL_GENERAL, ("op %d:    %s    deleted\r\n", ++c, p->filename));
                        break;
                    default:
                        break;
                    }
                }
            }
            ret = VOS_OK;
        }

        for(i=0; i<cl_vector_max(v); i++)
        {
            if(cl_vector_slot(v, i))
            {
                VOS_Free(cl_vector_slot(v, i));
                cl_vector_unset(v, i);
            }
        }

        VOS_TaskUnlock();
    }
    else
    {
        VOS_ASSERT(0);
    }

    return ret;
}

/*-----------------------------------------------------------------------------------*/

/*--------------------------------------- 临时配置文件管理相关实现 ---------------------------*/

#define AUTO_ONU_PROFILE_TIMEOUT 300

typedef struct{
    char name[ONU_CONFIG_NAME_LEN];
    int time;
}auto_onu_profile_mon_element_t;

static LONG autoOnuProfile_timer = 0;
static ULONG autoOnuProfile_msgid = 0;
static LONG autoOnuProfile_taskid = 0;
static dataarray_t *autoOnuProfile_dataarray = NULL;

void fcb_auto_onu_profile_add(const char *name,  int namelen)
{
    auto_onu_profile_mon_element_t * pe = VOS_Malloc(sizeof(auto_onu_profile_mon_element_t), MODULE_RPU_CTC);
    if(pe)
    {
        pe->time = AUTO_ONU_PROFILE_TIMEOUT;
        VOS_MemCpy(pe->name, name, namelen);
        pe->name[namelen]=0;

        data_array_add(autoOnuProfile_dataarray, pe);
    }
}

void fcb_auto_onu_profile_time()
{
    dataarray_t *pa = autoOnuProfile_dataarray;

    if(pa)
    {
        int c = data_array_count(pa);
        while(c--)
        {
            auto_onu_profile_mon_element_t * pe = (auto_onu_profile_mon_element_t*)data_array_getat(pa, c);

            if(!pe)
                continue;

            if(pe->time > 1)
            {
                pe->time--;
            }
            else
            {
                ONU_CONF_SEM_TAKE
                {
                    OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, pe->name, NULL, NULL);                                    
                }
                ONU_CONF_SEM_GIVE

                VOS_Free(pe);
                data_array_removeat(pa, c);
            }
        }
    }
}

void fcb_auto_onu_profile_delete(const char *name, const int namelen)
{
    dataarray_t *pa = autoOnuProfile_dataarray;

    if(pa)
    {
        int c = dataarray_count(pa);
        while(c--)
        {
            auto_onu_profile_mon_element_t * pe = (auto_onu_profile_mon_element_t*)data_array_getat(pa, c);
            if(!VOS_StrCmp(pe->name, name))
            {
                ONU_CONF_SEM_TAKE
                {
                    OnuProfile_Action_ByCode(OnuProfile_Delete, 0, 0, 0, pe->name, NULL, NULL);                                    
                }
                ONU_CONF_SEM_GIVE

                VOS_Free(pe);
                data_array_removeat(pa, c);
                break;
            }
        }
    }
}

void fcb_auto_onu_profile_cancel_check(const char *name, const int namelen)
{
    dataarray_t *pa = autoOnuProfile_dataarray;

    if(pa)
    {
        int c = dataarray_count(pa);
        while(c--)
        {
            auto_onu_profile_mon_element_t * pe = (auto_onu_profile_mon_element_t*)data_array_getat(pa, c);
            if(!VOS_StrCmp(pe->name, name))
            {
                VOS_Free(pe);
                data_array_removeat(pa, c);
                break;
            }
        }
    }
}

DECLARE_VOS_TASK(task_autoonuprofile_check)
{
    ULONG ulArgs[4];

    while(1)
    {
        if(VOS_QueReceive(autoOnuProfile_msgid, ulArgs, WAIT_FOREVER) != VOS_ERROR)
        {
            switch(ulArgs[1])
            {
                case auto_onu_profile_add_msg:
                    if(ulArgs[3])
                    {
                        fcb_auto_onu_profile_add(ulArgs[3], ulArgs[2]);
                        VOS_Free(ulArgs[3]);
                    }
                    break;
                case auto_onu_profile_delete_msg:
                    if(ulArgs[3])
                    {
                        fcb_auto_onu_profile_delete(ulArgs[3], ulArgs[2]);
                        VOS_Free(ulArgs[3]);
                    }
                    break;
                case auto_onu_profile_cancle_check_msg:
                    if(ulArgs[3])
                    {
                        fcb_auto_onu_profile_cancel_check(ulArgs[3], ulArgs[2]);
                        VOS_Free(ulArgs[3]);
                    }
                    break;
                case auto_onu_profile_time_msg:
                    fcb_auto_onu_profile_time();
                    break;
                default:
                    break;
            }
        }
        VOS_TaskDelay((VOS_Ticks_Per_Seconds/10));
    }
}

int sendAutoOnuProfileTimerMsg()
{
    ULONG ulArgs[4] = {MODULE_RPU_CTC, auto_onu_profile_time_msg, 0 , 0};
    return VOS_QueSend(autoOnuProfile_msgid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
}

int autoOnuProfileTimerCheck()
{
    return sendAutoOnuProfileTimerMsg();
}

void initAuoOnuProfileCheckTask()
{
    autoOnuProfile_dataarray = dataarray_create(5);

    if(!autoOnuProfile_dataarray)
    {
        VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "%s", "auto onu profile data array create fail!\r\n");
        return;
    }

    autoOnuProfile_msgid = VOS_QueCreate(200, VOS_MSG_Q_FIFO);

    if(!autoOnuProfile_msgid)
    {
        VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "%s", "auto onu profile msg queue create fail!\r\n");
        dataarray_destroy(autoOnuProfile_dataarray);
        return;
    }

    autoOnuProfile_taskid = VOS_TaskCreate("onuprofilecheck",TASK_PRIORITY_NORMAL, task_autoonuprofile_check, NULL);

    if(!autoOnuProfile_taskid)
    {
        VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "%s", "auto onu profile man task create fail!\r\n");
        dataarray_destroy(autoOnuProfile_dataarray);
        VOS_QueDelete(autoOnuProfile_msgid);
        return;
    }

    autoOnuProfile_timer = VOS_TimerCreate(MODULE_RPU_CTC, 0, 1000, autoOnuProfileTimerCheck, NULL, VOS_TIMER_LOOP);

    if(autoOnuProfile_timer == VOS_ERROR)
    {
        VOS_SysLog(LOG_TYPE_ONU, LOG_EMERG, "%s", "auto onu profile check timer create fail!\r\n");
        dataarray_destroy(autoOnuProfile_dataarray);
        VOS_QueDelete(autoOnuProfile_msgid);
        VOS_TaskDelete(autoOnuProfile_taskid);
        return;
    }

    /*注册事件：应用配置文件*/
    registerEventCallback(EVENT_ONUPROFILE_ACTION, fcb_autoOnuProfileApplyEvent);
}

int sendAutoOnuProfileAddMsg(const char *filename)
{
    ULONG ulArgs[4] = {MODULE_RPU_CTC, auto_onu_profile_add_msg, 0, 0};

    char * buf = NULL;

    ulArgs[2] = VOS_StrLen(filename);

    buf = VOS_Malloc(ulArgs[2]+1, MODULE_RPU_CTC);

    if(buf)
    {
        VOS_MemSet(buf, 0, ulArgs[2]+1);
        VOS_StrCpy(buf, filename);
        ulArgs[3] = (ULONG)buf;
        return VOS_QueSend(autoOnuProfile_msgid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
    }
    else
        return VOS_ERROR;

}

int sendAutoOnuProfileDeleteMsg(const char *filename)
{
    ULONG ulArgs[4] = {MODULE_RPU_CTC, auto_onu_profile_delete_msg, 0, 0};

    char * buf = NULL;

    ulArgs[2] = VOS_StrLen(filename);

    buf = VOS_Malloc(ulArgs[2]+1, MODULE_RPU_CTC);

    if(buf)
    {
        VOS_MemSet(buf, 0, ulArgs[2]+1);
        VOS_StrCpy(buf, filename);
        ulArgs[3] = (ULONG)buf;
        return VOS_QueSend(autoOnuProfile_msgid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
    }
    else
        return VOS_ERROR;
}

int sendAutoOnuProfileCancelCheckMsg(const char *filename)
{
    ULONG ulArgs[4] = {MODULE_RPU_CTC, auto_onu_profile_cancle_check_msg, 0, 0};

    char * buf = NULL;

    ulArgs[2] = VOS_StrLen(filename);

    buf = VOS_Malloc(ulArgs[2]+1, MODULE_RPU_CTC);

    if(buf)
    {
        VOS_MemSet(buf, 0, ulArgs[2]+1);
        VOS_StrCpy(buf, filename);
        ulArgs[3] = (ULONG)buf;
        return VOS_QueSend(autoOnuProfile_msgid, ulArgs, WAIT_FOREVER, MSG_PRI_NORMAL);
    }
    else
        return VOS_ERROR;
}


int getFirstOnuProfileEntryIndex(char * name, int *namelen)
{
    int rc = VOS_ERROR;

    int i = 0;

    while(i<g_maxPrimeNumber)
    {
        ONU_CONF_SEM_TAKE
        {
            element_onuconfhashbucket_t *pe = (element_onuconfhashbucket_t*) g_onuConfBucket[i];

#if 0
            if(pe)
            {
                ONUConfigData_t * pd = (ONUConfigData_t *) pe->confdata;
                if(pd)
                {
                    VOS_StrCpy(name, pd->confname);
                    *namelen = VOS_StrLen(pd->confname);
                    rc = VOS_OK;
                }
            }
#endif
            while(pe)
            {
                ONUConfigData_t * pd = (ONUConfigData_t *) pe->confdata;
                if(!VOS_StrStr(pd->confname, "*auto"))
                {
                    VOS_StrCpy(name, pd->confname);
                    *namelen = VOS_StrLen(pd->confname);
                    rc = VOS_OK;
                    break;
                }
                pe = pe->next;
            }

            i++;
        }
        ONU_CONF_SEM_GIVE

        if(rc == VOS_OK)
            break;
    }

    return rc;
}

int getNextOnuProfileEntryIndex(const char *curName, char *name, int *namelen)
{
    int rc = VOS_ERROR;

    int fok = 0;

    int i = getOnuConfHashIndex(curName);

    while(i<g_maxPrimeNumber)
    {
        ONU_CONF_SEM_TAKE
        {
            element_onuconfhashbucket_t *pe = (element_onuconfhashbucket_t*) g_onuConfBucket[i];

#if 0
            if(fok)
            {
                if(pe)
                {
                    ONUConfigData_t *pd = (ONUConfigData_t *) pe->confdata;
                    VOS_StrCpy(name, pd->confname);
                    *namelen = VOS_StrLen(name);
                    rc = VOS_OK;
                }

            }
            else
            {
                while(pe)
                {
                    ONUConfigData_t * pd = (ONUConfigData_t *) pe->confdata;

                    if(fok)
                    {
                        VOS_StrCpy(name, pd->confname);
                        *namelen = VOS_StrLen(name);
                        rc = VOS_OK;
                        break;
                    }
                    else
                    {
                        if(!VOS_StrCmp(pd->confname, curName))
                            fok = 1;
                    }

                    pe = pe->next;

                }

            }
#endif

            while(pe)
            {
                ONUConfigData_t *pd = (ONUConfigData_t *) pe->confdata;
                if(fok)
                {
                    if(!VOS_StrStr(pd->confname, "*auto"))
                    {
                        VOS_StrCpy(name, pd->confname);
                        *namelen = VOS_StrLen(name);
                        rc = VOS_OK;
                        break;
                    }
                }
                else
                {
                    if(!VOS_StrCmp(pd->confname, curName))
                        fok = 1;
                }
                pe = pe->next;
            }

            i++;
        }
        ONU_CONF_SEM_GIVE

        if(rc == VOS_OK)
            break;
    }

    return rc;
}

int checkOnuProfileEntryIndex(const char * name)
{
    int rc = (getOnuConfFromHashBucket(name) == NULL)?VOS_ERROR:VOS_OK;
    return rc;
}

int dataarray_add(void *data, void *e)
{
	int rc = -1;
	dataarray_t * p = (dataarray_t*)data;

	if(p)
	{
		if(!p->data)
		{
			p->data = VOS_Malloc(p->growth*sizeof(int), MODULE_RPU_CTC);
			if(!p->data)
				return rc;
			else
				p->maxnum = p->growth;
		}

		if(p->number < p->maxnum)
			p->data[p->number] = (int)e;
		else
		{
			p->data = VOS_Realloc(p->data, (p->maxnum+p->growth)*sizeof(int), MODULE_RPU_CTC);
			p->maxnum += p->growth;
			if(p->data)
				p->data[p->number] = (int)e;
			else
				return rc;
		}
		p->number++;
		rc = 0;
	}

	return rc;

}

int dataarray_remove_at(void *data, int pos)
{
	int rc = -1;
	dataarray_t * p = (dataarray_t*)data;

	if(p && p->data && pos >= 0 && pos < p->number)
	{
		memmove(p->data+pos, p->data+pos+1, (p->number-pos-1)*sizeof(int));
		p->number--;
		rc = 0;
	}

	return rc;
}

int dataarray_remove_all(void *data)
{

	int rc = -1;
	dataarray_t * p = (dataarray_t*)data;

	if(p && p->data )
	{
		p->number = 0;
		/*deleted by liyang 2014-08-14*/
		/*p->maxnum = 0;*/
		rc = 0;
	}

	return rc;
}

void* dataarray_get_at(void *data, int pos)
{
	void * prc = NULL;
	dataarray_t *p = (dataarray_t*)data;

	if(p)
	{
		if(pos < p->number && pos >= 0)
			prc = (void*)(p->data[pos]);
	}

	return prc;
}

int dataarray_count(void *data)
{
	return data?((dataarray_t*)data)->number:(0);
}

dataarray_t * dataarray_create(const int growth)
{
	dataarray_t * parray = VOS_Malloc(sizeof(dataarray_t), MODULE_RPU_CTC);
	if(parray)
	{
		parray->data = NULL;
		parray->maxnum = 0;
		parray->number = 0;
		parray->growth = growth>0?growth:16;

		parray->add = dataarray_add;
		parray->removeall = dataarray_remove_all;
		parray->removeat = dataarray_remove_at;
		parray->getat = dataarray_get_at;
		parray->count = dataarray_count;
	}

	return parray;
}

void dataarray_destroy(dataarray_t *parray)
{
	if(parray)
	{
		if(parray->data)
			VOS_Free(parray->data);

		VOS_Free(parray);
	}
}
/*-----------------------------------------------------------------------------------*/

int onuconf_get_profile_text(const char *confname, char *text, const ULONG buflen, ULONG *len)
{
    int rc = VOS_ERROR;

    if(confname && text && len)
    {
        struct vty * file_vty;
        ONUConfigData_t *pd = getOnuConfFromHashBucket(confname);

        file_vty = vty_new();
        if (file_vty && pd)
        {

            char *file = NULL;

            file_vty->fd = _CL_FLASH_FD_;
            file_vty->type = VTY_FILE;

            file = VOS_Malloc(8192, MODULE_RPU_SNMP);

            if(file)
            {
                ULONG length = 0, copylen = 0;
                VOS_MemZero(file, 8192);
                if(VOS_OK == output_onuconf_for_saving(file_vty, pd, file, &length))
                {
                    copylen = (length<=buflen)?length:buflen;
                    VOS_MemCpy(text, file, copylen);
                    *len = copylen;
                    rc = VOS_OK;
                }

                VOS_Free(file);
            }

            vty_free(file_vty);
        }
    }

    return rc;
}

ONUConfigData_t * openOnuConfigFile(const char * name, onu_conf_op_flag_t mode )
{
    ONUConfigData_t *pd = NULL;

    ULONG thandle = VOS_GetCurrentTask();

    VOS_TASKCB * ptcb = (VOS_TASKCB*)thandle;

    ULONG owner = ptcb->ulOSId;

    ONU_CONF_SEM_TAKE
    {
        pd = getOnuConfFromHashBucket(name);

        switch(mode)
        {
            case ONU_CONF_OPEN_FLAG_READ:
 
                break;
            case ONU_CONF_OPEN_FLAG_WRITE:
                if(pd )
                {
                    if(!pd->owner || pd->owner == owner ||
                            ((pd->owner != owner) && (taskIdVerify(pd->owner) == VOS_ERROR)))
                    {
                        pd->owner = owner;
                    }
                    else
                        pd = NULL;
                }
                else
                {
                    pd = OnuConfigProfile_init();
                    if(pd)
                    {
                        pd->owner = owner;
                        VOS_StrnCpy(pd->confname, name, ONU_CONFIG_NAME_LEN);
                        if(OnuProfile_Action_ByCode(OnuProfile_Add, 0, 0, 0, name, NULL, pd)!= VOS_OK)
                        {
                            onuconf_free(pd, ONU_CONF_MEM_DATA_ID);
                            pd = NULL;
                        }
                    }
                }
                break;
            default:
                break;
        }

    }
    ONU_CONF_SEM_GIVE

    return pd;
}

int closeOnuConfigFile(const char * name)
{
    int ret = VOS_ERROR;
    ONUConfigData_t *pd = NULL;
    ULONG thandle = VOS_GetCurrentTask();

    VOS_TASKCB * ptcb = (VOS_TASKCB*)thandle;

    ULONG owner = ptcb->ulOSId;

    ONU_CONF_SEM_TAKE
    {
        pd = getOnuConfFromHashBucket(name);
        if(pd && pd->owner == owner)
        {
            pd->owner = 0;
            ret = VOS_OK;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int checkOnuConfigFile(const char * name)
{
    int ret = VOS_OK;
    ONUConfigData_t *pd = NULL;

    ONU_CONF_SEM_TAKE
    {
        pd = getOnuConfFromHashBucket(name);
        if(pd && pd->owner)
        {
            ret = VOS_ERROR;
        }
    }
    ONU_CONF_SEM_GIVE

    return ret;
}
int checkOnuIsIntoOnuNode(short int slot, short int port, short int onuid)
{
	short int PonPortIdx = GetPonPortIdxBySlot(slot, port);
	short int OnuIdx = onuid - 1;
	char sz[80]="";
	char sz_temp[80]="";
    int iRlt = VOS_OK;

    VOS_Sprintf(sz, "onu%d/%d/%d", slot, port, onuid);
    VOS_Sprintf(sz_temp, "*autoonu%d/%d/%d", slot, port, onuid);
	/*在onu节点时，关联关系必为私有*/
	if (!onuConfIsShared(PonPortIdx, OnuIdx))
	{
		if(checkOnuConfigFile(sz) == VOS_ERROR || checkOnuConfigFile(sz_temp) == VOS_ERROR)
			iRlt = VOS_ERROR;
	}
	return iRlt;
}

/*---------------------------------------------------------mib接口------------------------------------------------------*/
static ONUVlanConf_t s_onuvlanconf;
static ULONG s_mibvlantick = 0;
static short int s_mibvlanponid =0, s_mibvlanonuid=0;

static int sfun_addPortToVlan(ONUVlanConf_t *pconf, int vid , int portid, int tag)
{
	int ret = VOS_ERROR, idx = -1;

	if(pconf && vid >=1 && vid <=4094 && portid <= ONU_MAX_PORT && portid >=1)
	{
		int i = 0;
		for(i=0; i<pconf->onuVlanEntryNum; i++)
		{
			if(pconf->entryarry[i].vlanid == vid)
			{
				idx = i;
				break;
			}
		}

		if(i==pconf->onuVlanEntryNum && i+1 < ONU_MAX_VLAN)
		{
			idx = i;
			pconf->onuVlanEntryNum++;
		}

		if(idx != -1)
		{
			pconf->entryarry[idx].vlanid = vid;
			pconf->entryarry[idx].allPortMask |= (1<<(portid-1));

			if(!tag)
				pconf->entryarry[idx].untagPortMask |= (1<<(portid-1));

			ret = VOS_OK;
		}

	}

	return ret;
}

static int copyOnuVlanConfByModel(ONUVlanConf_t *dst, ONUVlanConf_t *src, ONUVlanConf_t model)
{
	int ret = VOS_ERROR;

	if(dst && src )
	{

		int i, j;
        UCHAR num = src->onuVlanEntryNum;
		VOS_MemZero(dst, sizeof(ONUVlanConf_t));
        /*added by luh 2012-11-26*/
        if (num > 0)
        {
            short int temp1 = 0; 
            ULONG temp2 = 0, temp3 = 0;
            for (i=0;i<num-1;i++)
            {   
                for(j=0;j<num-1-i;j++)
                {
                    if(src->entryarry[j].vlanid>src->entryarry[j+1].vlanid)
                    {
                        temp1 = src->entryarry[j].vlanid;
                        temp2 = src->entryarry[j].allPortMask;
                        temp3 = src->entryarry[j].untagPortMask;
                        src->entryarry[j].vlanid = src->entryarry[j+1].vlanid;
                        src->entryarry[j].allPortMask = src->entryarry[j+1].allPortMask;
                        src->entryarry[j].untagPortMask = src->entryarry[j+1].untagPortMask;                                
                        src->entryarry[j+1].vlanid = temp1;
                        src->entryarry[j+1].allPortMask = temp2;
                        src->entryarry[j+1].untagPortMask = temp3;   
                    }
                }

            }
            VOS_MemCpy(dst, src, sizeof(ONUVlanConf_t));
        }
        /*moved by luh 2012-11-26*/
#if 0        
		/*按model的顺序copy vlan数据*/
		for(i=0; i<model.onuVlanEntryNum; i++)
		{
			for(j=0; j<src->onuVlanEntryNum; j++)
			{
				if(src->entryarry[j].vlanid == model.entryarry[i].vlanid)
				{
					dst->entryarry[i] = src->entryarry[j];
					dst->onuVlanEntryNum++;
					src->entryarry[j].vlanid = 0;
				}
			}
		}

		/*添加src有，但model没有的vlan*/

		for(j=0; j<src->onuVlanEntryNum; j++)
		{
			if(src->entryarry[j].vlanid && (dst->onuVlanEntryNum < ONU_MAX_VLAN-1))
				dst->entryarry[dst->onuVlanEntryNum++] = src->entryarry[j];
		}
#endif
		ret = VOS_OK;
	}

	return ret;
}

int getOnuVlanConfig(short int ponid, short int onuid, ONUVlanConf_t *conf)
{
	int ret = VOS_ERROR;
	ONUVlanConf_t ovc;

	if(conf)
	{
		if(ponid == s_mibvlanponid && onuid == s_mibvlanonuid && VOS_GetTick()-s_mibvlantick < VOS_TICK_SECOND/10)
		{
			VOS_MemCpy(conf, &s_onuvlanconf, sizeof(ONUVlanConf_t));
		}
		else
		{
			int num = getOnuEthPortNum(ponid, onuid);
			int i=0;

			VOS_MemZero(&ovc, sizeof(ONUVlanConf_t));

			/*获取全部的VLAN数据*/
			for(i=1; i<=num; i++)
			{
				CTC_STACK_port_vlan_configuration_t vc;
				if(OnuMgt_GetEthPortVlanConfig(ponid, onuid, i, &vc) == VOS_OK)
				{
					if(vc.mode != CTC_VLAN_MODE_TRUNK)
						continue;
					else
					{
						int j = 0;
						sfun_addPortToVlan(&ovc, vc.default_vlan&0x0fff, i, 0);
						for(j=0; j<vc.number_of_entries; j++)
							sfun_addPortToVlan(&ovc, vc.vlan_list[j]&0xfff, i, 1);
					}
				}
			}

			ONU_CONF_SEM_TAKE
			{
				ONUConfigData_t *pd = getOnuConfFromHashBucket(ONU_CONF_NAME_PTR_GET(ponid, onuid));
				if(pd)
				{
					copyOnuVlanConfByModel(conf, &ovc, pd->vlanconf);
					VOS_MemCpy(&s_onuvlanconf, conf, sizeof(ONUVlanConf_t));
				}
			}
			ONU_CONF_SEM_GIVE

		}

		ret = VOS_OK;
	}

	s_mibvlantick = VOS_GetTick();
	s_mibvlanponid = ponid;
	s_mibvlanonuid = onuid;

	return ret;
}

int getOnuVlanConfigByPtr(short int suffix, void * pdata, ONUVlanConf_t *conf)
{
    int ret = VOS_OK;

    if(conf)
    {
        ONU_CONF_SEM_TAKE
        {
            if(onuConfCheckByPtr(suffix, pdata))
                ret = VOS_ERROR;
            else
            {
                ONUConfigData_t * p = (ONUConfigData_t *)pdata;
                if(p)
                {
                	ONUVlanConf_t *pvc = &p->vlanconf;
                	VOS_MemCpy(conf, pvc, sizeof(ONUVlanConf_t));
                	ret = VOS_OK;
                }
            }
        }
        ONU_CONF_SEM_GIVE;
    }
    else
        ret = VOS_ERROR;

    return ret;
}
/*-------------------------------------------------------------------------------------------------------------------------*/
#endif

ULONG onuconf_build_lzma_data(char * data, const int length, char **outptr)
{
	ULONG ret = 0;

	if(data && outptr && length > 0)
	{
		char *pdata = NULL;
		ULONG outlen = 0;
		char * p = VOS_Malloc(length, MODULE_RPU_CTC);
		if(p)
		{
			VOS_MemCpy(p, data, length);
			((ONUConfigData_t*)p)->owner = 0;

			if(defLzmaCompress(&pdata, &outlen,  p, length, lzma_malloc, lzma_free, 0) == VOS_OK)
			{
				ret = outlen;
				*outptr = pdata;
			}

			VOS_Free(p);
		}
	}

	return ret;
}


int onuconf_signal_cardsyncfinished(int slot)
{
	ULONG semid = getSemIdBySlotno(slot);
	VOS_SemGive(semid);
	return VOS_OK;
}

int onuconf_wait_cardsyncfinished_signal(int slot, int timeout)
{

	int ret = VOS_ERROR;
	int wtime = 0;
	ULONG semid = getSemIdBySlotno(slot);

	if(timeout < 0)
		wtime = WAIT_FOREVER;
	else
		wtime = (timeout > 0)?timeout:ONU_CONF_CARD_SYNC_WAIT_TIME;

	ret = VOS_SemTake(semid, wtime*VOS_TICK_SECOND);

	return ret;
}

void testOnuRestore(void)
{
	int ipon = 0;
	for(ipon=0; ipon<MAXPON;ipon++)
	{
		int onu;
		for(onu=0; onu<MAXONUPERPON; onu++)
		addOnuToRestoreQueue(ipon, onu, ONU_CONF_RES_ACTION_DO, -1);
	}

	for(ipon=0; ipon<MAXPON; ipon++)
	{
		int onu;
		for(onu=0; onu<10; onu++)
		{
			int a = rand();
			a &= 63;
			delOnuFromRestoreQueue(ipon, a);
		}
	}
}

void testOnuAssociation( const char *name)
{
	int i, j;
	for(i=0; i<MAXPON; i++)
	{
		int s = rand();
		s &= 63;

		for(j=0; j<MAXONUPERPON; j++)
			/*onuconfAssociateOnuId(i, j, DEFAULT_ONU_CONF);*/

		for(j=s; j<MAXONUPERPON; j++)
		{
			/*onuconfAssociateOnuId(i, j, name);*/
		}
	}
}

#ifdef __cplusplus
}
#endif
