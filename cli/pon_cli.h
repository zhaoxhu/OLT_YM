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

ULONG * V2R1_Parse_OnuId_List( CHAR * pcOnuId_List );
enum match_type V2R1_Check_OnuId_List( char * onuid_list );
LONG V2R1_Check_OnuId_ListValid( const char * pcOnuId_List );

extern enum match_type V2R1_Check_Priority_List( char * priority_list );
extern ULONG * V2R1_Parse_Priority_List( CHAR * priority_list );

#define BEGIN_PARSE_PRIORITY_LIST_TO_PRIORITY(priority_list, priority) \
{\
    ULONG * _pulIfArray;\
    ULONG _i = 0;\
    _pulIfArray = V2R1_Parse_Priority_List(priority_list);\
    if(_pulIfArray != NULL)\
    {\
        for(_i=0;_pulIfArray[_i]!=0xaa55;_i++)\
        {\
            priority = _pulIfArray[_i];

#define END_PARSE_PRIORITY_LIST_TO_PRIORITY() \
        }\
        VOS_Free(_pulIfArray);\
    }\
}

#ifdef	__cplusplus
}
#endif/* __cplusplus */

#endif/* __IFM_XXX_H__ */
