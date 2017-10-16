/****************************************************************************
*
*     Copyright (c) 2005 Mellon Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Mellon Corporation
*
*     All information contained in this document is Mellon Corporation
*     company private, proprietary, and trade secret.
*
*****************************************************************************/
#ifndef __IFM_XXX_H__
#define __IFM_XXX_H__

#ifdef __cplusplus
extern "C"{
#endif

#define  ONU_GT861_NODE  ONU_RESERVED1_NODE		/*ONU_GT865_NODE*/
#define  ONU_GT831B_NODE  ONU_RESERVED2_NODE
#define  NEW_ONU_TYPE_CLI_NODE   ONU_RESERVED3_NODE

ULONG * V2R1_Parse_Port_List( CHAR * pcPort_List );
ULONG * V2R1_Parse_Vlan_List( CHAR * pcvlan_List );
/**这一段可以放到相应的头文件,便于其他文件引用**/
#define BEGIN_PARSE_ONUID_LIST_TO_ONUID_1(onuid_list, onuid) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = V2R1_Parse_OnuId_List_1(onuid_list);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0;_i++)\
        {\
            onuid = _pulIfArray[_i];\
            if(onuid>MAXONUPERPON) continue;
            

#define END_PARSE_ONUID_LIST_TO_ONUID_1() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}



/**这一段可以放到相应的头文件,便于其他文件引用**/
#define BEGIN_PARSE_PORT_LIST_TO_PORT(port_list, port) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = V2R1_Parse_Port_List(port_list);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0;_i++)\
        {\
            port = _pulIfArray[_i];

#define END_PARSE_PORT_LIST_TO_PORT() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}

/* added by xieshl 20120906, 在宏BEGIN_PARSE_PORT_LIST_TO_PORT 和END_PARSE_PORT_LIST_TO_PORT之间不能直接调用
    return，统一用下面的宏替换，防止内存泄露 */
#define RETURN_PARSE_PORT_LIST_TO_PORT(x) {\
        VOS_Free(_pulIfArray);\
        return (x);\
}


/*add by shixh 20110614*/
#define BEGIN_PARSE_VLAN_LIST_TO_VLAN(vlan_list, vlanList) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = V2R1_Parse_Vlan_List(vlan_list);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0;_i++)\
        {\
            vlanList[_i] = _pulIfArray[_i];

#define END_PARSE_VLAN_LIST_TO_VLAN() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}
#define BEGIN_PARSE_VLAN_LIST_TO_VLAN_NUM(port_list, vlan) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = V2R1_Parse_Vlan_List(port_list);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0;_i++)\
        {\
            vlan = _pulIfArray[_i];
#define END_PARSE_VLAN_LIST_TO_VLAN_NUM() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}
#ifdef	__cplusplus
}
#endif/* __cplusplus */

#endif/* __IFM_XXX_H__ */
