/*  
**  CTC_STACK_defines.h - CTC stack definition header file
**
**  This header file contains an extension to the PAS5001 system API 
**
**  
**  This software is licensed for use according to the terms set by the Passave API license agreement.
**  Copyright Passave Ltd. 
**  Ackerstein Towers - A, 9 Hamenofim St.
**  POB 2089, Herziliya Pituach 46120 Israel
**
**
**  This file was written by Meital Levy, meital.levy@passave.com, 13/03/2006
**  
*/


#ifndef _CTC_STACK_DEFINES_H__
#define _CTC_STACK_DEFINES_H__

/*============================= Include Files ===============================*/	
/*=============================== Constants =================================*/

#define CTC_OUI_DEFINE                    0x111111
#define CTC_VERSION_DEFINE                1
#define CTC_2_1_ONU_VERSION               0x21

#define CTC_OBJECT_CONSTANT_SIZE		  4

#define MAX_OUI_RECORDS                   62/* Max OUI+Version pairs in Extended OAM information frame*/

#define ONU_RESPONSE_SIZE                 100/* used in send file file_name buffer and send file error_msg buffer */

#define	CTC_STACK_ALL_PORTS		0xFF
#define	CTC_MANAGEMENT_OBJECT_ALL_PORTS		0xFFFF

/*================================ Macros ==================================*/


#define CTC_STACK_CHECK_INIT \
    if ( !ctc_stack_general_information.ctc_stack_is_init ) \
    { \
        PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"ERROR: CTC_STACK package was not initialized. please call CTC_STACK_init first.\n"); \
        return CTC_STACK_EXIT_ERROR; \
    }

#define CTC_STACK_CHECK_INIT_FUNC \
    if ( !ctc_stack_general_information.ctc_stack_is_init ) \
    { \
		PONLOG_ERROR_0( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
							"ERROR: CTC_STACK package was not initialized. please call CTC_STACK_init first.\n" );\
        return CTC_STACK_EXIT_ERROR; \
    }

#define CTC_STACK_CHECK_OLT_VALIDITY\
    { \
        CTC_STACK_CHECK_INIT_FUNC\
        if(!OLTAdv_IsExist(olt_id) ) \
        {\
            PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, \
                        "ERROR: OLT: %d is not available\n", olt_id);\
                        return CTC_STACK_OLT_NOT_EXIST; \
        }\
    }


#define CTC_STACK_CHECK_ONU_VALIDITY\
    if ( discovery_db.state[olt_id][onu_id] != CTC_DISCOVERY_STATE_COMPLETE) \
    { \
        PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, olt_id, onu_id,"ERROR: OLT: %d ONU :%d OAM extended discovery state is not complete, can not apply with this command\n",olt_id,onu_id); \
        return CTC_STACK_ONU_NOT_AVAILABLE; \
    }


#define CTC_STACK_CHECK_ONU_VALIDITY_FUNC\
    if ( discovery_db.state[olt_id][onu_id] != CTC_DISCOVERY_STATE_COMPLETE) \
    { \
		PONLOG_ERROR_2( PONLOG_FUNC_GENERAL, olt_id, onu_id, \
						"ERROR: OLT: %d ONU :%d OAM extended discovery state is not complete, can not apply with this command\n", olt_id, onu_id);\
        return CTC_STACK_ONU_NOT_AVAILABLE; \
    }


#define CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY \
    { \
        CTC_STACK_CHECK_INIT\
        CTC_STACK_CHECK_ONU_VALIDITY\
    }

#define CTC_STACK_CHECK_INIT_AND_CHECK_ONU_VALIDITY_FUNC \
    { \
        CTC_STACK_CHECK_INIT_FUNC\
        CTC_STACK_CHECK_ONU_VALIDITY_FUNC\
    }


#define CTC_STACK_CHECK_INIT_AND_CHECK_OLT_AND_ONU_VALIDITY_FUNC \
    { \
        CTC_STACK_CHECK_INIT_FUNC\
		if(!OLTAdv_IsExist(olt_id) ) \
		{\
		PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, olt_id, PONLOG_ONU_IRRELEVANT, \
						"ERROR: OLT: %d is not available\n", olt_id);\
						return CTC_STACK_OLT_NOT_EXIST; \
		}\
        CTC_STACK_CHECK_ONU_VALIDITY_FUNC\
    }


#define CHECK_COMPARSER_RESULT_AND_RETURN(comparser_result) \
{   result = Convert_comparser_error_code_to_ctc_stack(comparser_result);\
    if ( result != CTC_STACK_EXIT_OK )\
{ \
    PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)\
    return result;\
}\
}


#define CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(__func, comparser_result) \
{   result = Convert_comparser_error_code_to_ctc_stack(comparser_result);\
    if ( result != CTC_STACK_EXIT_OK )\
{ \
	PONLOG_ERROR_3( __func, olt_id, onu_id, \
						"failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);\
    return result;\
}\
}

#define UPDATE_SENT_LEN \
{\
	total_length += sent_length;\
	sent_length = max_length - total_length;\
}

#define UPDATE_RECEIVED_LEN \
{\
	received_total_length += received_left_length;\
}

#define CHECK_CTC_2_1_ONU_VERSION_RESULT_AND_RETURN \
    if ( discovery_db.common_version[olt_id][onu_id] < CTC_2_1_ONU_VERSION ) \
{ \
    PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. Not CTC 2.1 ONU version\n", olt_id, onu_id)\
    return CTC_STACK_NOT_IMPLEMENTED; \
}

#define CHECK_CTC_EXTENSION_RESULT_AND_RETURN_FUNC(__func, ctc_ret_val_from_comparser) \
{\
    if ( ctc_ret_val_from_comparser != COMPARSER_CTC_STATUS_SUCCESS )\
{ \
	PONLOG_ERROR_3( __func, olt_id, onu_id,\
						   "OLT %d ONU %d error returned code 0x%x\n",\
						   olt_id, onu_id, ctc_ret_val_from_comparser );\
    return (Convert_comparser_error_code_to_ctc_stack(ctc_ret_val_from_comparser));\
}\
}


#define CHECK_COMPARSER_RESULT_AND_CTC_EXTENSION_RESULT_RETURN_FUNC(__func, __comparser_result__, __ctc_ret_val__) \
{  CHECK_CTC_EXTENSION_RESULT_AND_RETURN_FUNC(__func, __ctc_ret_val__)\
	CHECK_COMPARSER_RESULT_AND_RETURN_FUNC(__func, __comparser_result__)\
}


#define CHECK_CTC_STACK_RESULT_AND_RETURN \
    if ( result != CTC_STACK_EXIT_OK ) \
{ \
    PONLOG_ERROR_3(PONLOG_FUNC_GENERAL, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)\
    return result; \
}


#define CHECK_CTC_STACK_RESULT_AND_RETURN_FUNC \
    if ( result != CTC_STACK_EXIT_OK ) \
{ \
	PONLOG_ERROR_3( PONLOG_FUNC_GENERAL, olt_id, onu_id, \
					"failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result);\
    return result; \
}


#define CHECK_COMM_RESULT_AND_RETURN \
    if ( result != CTC_STACK_COMMUNICATION_EXIT_OK ) \
{ \
    PONLOG_ERROR_3(PONLOG_FUNC_CTC_COMM, olt_id, onu_id, "failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)\
    return result; \
}

#define CHECK_COMM_RESULT_AND_RETURN_FUNC(__func__) \
    if ( result != CTC_STACK_COMMUNICATION_EXIT_OK ) \
{ \
    PONLOG_ERROR_3(__func__,olt_id, onu_id,"failed in OLT %d ONU %d. error code:%d\n", olt_id, onu_id, result)\
    return result; \
}


#define CHECK_OPCODE_AND_RETURN \
{   if( received_opcode != expected_opcode )\
    {\
      PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"Unexpected opcode: received-0x%x,expected-0x%x \n", received_opcode,expected_opcode );\
      return COMPARSER_STATUS_WRONG_OPCODE;\
    }\
}


#define CHECK_OPCODE_AND_RETURN_FUNC(__func) \
{   if( received_opcode != expected_opcode )\
    {\
		PONLOG_ERROR_2( __func, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
				    "Unexpected opcode: received-0x%x,expected-0x%x \n", received_opcode,expected_opcode); \
      return COMPARSER_STATUS_WRONG_OPCODE;\
    }\
}

#define CHECK_ENTITY_NUMBER_AND_RETURN_FUNC(__func, __received__, __expected__, __entity_name__) \
{   if( __received__ != __expected__ )\
    {\
		PONLOG_ERROR_2( __func, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
				    "Unexpected "#__entity_name__": received-0x%x, expected-0x%x \n", __received__, __expected__); \
      return CTC_STACK_QUERY_FAILED;\
    }\
}


#define CHECK_MANAGEMENT_OBJECT_AND_RETURN_FUNC(__func, __received__, __expected__) \
{\
	CHECK_ENTITY_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, __received__.leaf,			__expected__.leaf,			leaf);\
	CHECK_ENTITY_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, __received__.index.frame_number,	__expected__.index.frame_number,	frame_number);\
	if (__expected__.index.slot_number != CTC_MANAGEMENT_OBJECT_ALL_SLOTS) /* 3F == 63 == 0b111111 == all slots */\
	{\
		CHECK_ENTITY_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, __received__.index.slot_number,	__expected__.index.slot_number,	slot_number);\
	}\
	if (__expected__.index.port_number != CTC_MANAGEMENT_OBJECT_ALL_PORTS) /* 0xFFFF == all ports */\
	{\
		CHECK_ENTITY_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, __received__.index.port_number,	__expected__.index.port_number,	port_number);\
		CHECK_ENTITY_NUMBER_AND_RETURN_FUNC(PONLOG_FUNC_GENERAL, __received__.index.port_type,	__expected__.index.port_type,		port_type);\
	}\
}

#define CHECK_CTC_STACK_OBJECT_ETH_PORT_RANGE(_func, _variable_, _ctc_discovery_version_)\
    if(_ctc_discovery_version_ == CTC_2_1_ONU_VERSION)\
    {\
        PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(_func,  _variable_, port_number, CTC_MANAGEMENT_OBJECT_MIN_PORT, CTC_MANAGEMENT_OBJECT_MAX_PORT) \
    }\
    else\
    {\
        PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(_func, _variable_, port_number,CTC_MIN_ETHERNET_PORT_NUMBER, CTC_MAX_ETHERNET_PORT_NUMBER)  \
    }

#define CHECK_CTC_STACK_OBJECT_E1_PORT_RANGE(_func, _variable_, _ctc_discovery_version_)\
    if(_ctc_discovery_version_ == CTC_2_1_ONU_VERSION)\
    {\
        PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(_func,  _variable_, port_number, CTC_MANAGEMENT_OBJECT_MIN_PORT, CTC_MANAGEMENT_OBJECT_MAX_PORT) \
    }\
    else\
    {\
        PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(_func, _variable_, port_number,CTC_MIN_E1_PORT_NUMBER, CTC_MAX_E1_PORT_NUMBER)  \
    }

#define CHECK_CTC_STACK_OBJECT_VOIP_PORT_RANGE(_func, _variable_, _ctc_discovery_version_)\
    if(_ctc_discovery_version_ == CTC_2_1_ONU_VERSION)\
    {\
        PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(_func,  _variable_, port_number, CTC_MANAGEMENT_OBJECT_MIN_PORT, CTC_MANAGEMENT_OBJECT_MAX_PORT) \
    }\
    else\
    {\
        PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(_func, _variable_, port_number,CTC_MIN_VOIP_PORT_NUMBER, CTC_MAX_VOIP_PORT_NUMBER)  \
    }    

#define CHECK_PORT_NUMBER_AND_RETURN_FUNC(__func, __received_port__, __expected_port__) \
{   if( __received_port__ != __expected_port__ )\
    {\
		PONLOG_ERROR_2( __func, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
				    "Unexpected port number: received-0x%x,expected-0x%x \n", __received_port__,__expected_port__); \
      return CTC_STACK_QUERY_FAILED;\
    }\
}


#define CHECK_BUFFER_SIZE_AND_RETURN( offset, total_length, buffer, size_to_write )\
{\
    if((offset + size_to_write - buffer) > total_length)\
    {\
        PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"failed to compose command. Allocated buffer is too small %d\n", total_length);\
        return COMPARSER_STATUS_MEMORY_ERROR;\
    }\
}


#define CHECK_BUFFER_SIZE_AND_RETURN_FUNC( _func, offset, total_length, buffer, size_to_write )\
{\
    if((offset + size_to_write - buffer) > total_length)\
    {\
	PONLOG_ERROR_1( PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
				    "failed to read or write. Allocated buffer is too small %d\n", total_length); \
        return COMPARSER_STATUS_MEMORY_ERROR;\
    }\
}


#define FILL_USHORT_ENDIANITY_IN_UBUFFER( x, offset, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_WORD)\
    USHORT_2_UBUFFER( (unsigned long) (x), offset )\
    offset+= BYTES_IN_WORD;\
}


#define FILL_USHORT_ENDIANITY_IN_UBUFFER_FUNC( _func, x, offset, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, BYTES_IN_WORD)\
    USHORT_2_UBUFFER( (unsigned long) (x), offset )\
    offset+= BYTES_IN_WORD;\
}


#define FILL_LIMITED_LONG_IN_3_BYTES_ENDIANITY_IN_UBUFFER_FUNC( _func, x, offset, length, buffer )\
{ \
	CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, 3)\
    LIMITED_ULONG_2_UBUFFER( x, offset )\
	offset+= 3;\
}


#define FILL_ULONG_ENDIANITY_IN_UBUFFER_FUNC( _func, x, offset, length, buffer )\
{ \
	CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, BYTES_IN_LONG)\
    ULONG_2_UBUFFER( x, offset )\
	offset+= BYTES_IN_LONG;\
}

#ifdef __PASSAVE_ONU__
#define FILL_ULONG_LONG_ENDIANITY_IN_UBUFFER_FUNC( _func, x, offset, length, buffer )\
{ \
	CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, BYTES_IN_DOUBLE)\
    ULONG_LONG_2_ENDIANITY_UBUFFER( x, offset )\
	offset+= BYTES_IN_DOUBLE;\
}
#else
#define FILL_ULONG_LONG_ENDIANITY_IN_UBUFFER_FUNC( _func, x, offset, length, buffer )\
{ \
	CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, BYTES_IN_DOUBLE)\
    INT64_2_UBUFFER( x, offset )\
	offset+= BYTES_IN_DOUBLE;\
}
#endif

#ifdef __PASSAVE_ONU__
#define EXTRACT_ULONG_LONG_ENDIANITY_FROM_UBUFFER_FUNC( _func, offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, 2*BYTES_IN_LONG)\
    UBUFFER_2_ENDIANITY_ULONG_LONG( offset, x )\
    offset+= 2*BYTES_IN_LONG;\
}
#else
#define EXTRACT_ULONG_LONG_ENDIANITY_FROM_UBUFFER_FUNC( _func, offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, 2*BYTES_IN_LONG)\
    UBUFFER_2_INT64( offset, x )\
    offset+= 2*BYTES_IN_LONG;\
}
#endif


#define FILL_BRANCH_LEAF( _func, branch, leaf, offset, length, buffer )\
{ \
    FILL_UCHAR_ENDIANITY_IN_UBUFFER_FUNC(_func, branch, offset, length, buffer)\
	FILL_USHORT_ENDIANITY_IN_UBUFFER_FUNC(_func, leaf, offset, length, buffer)\
}



#define FILL_UCHAR_ENDIANITY_IN_UBUFFER( x, offset, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_CHAR)\
    UCHAR_2_ENDIANITY_UBUFFER( (unsigned char) (x), offset )\
    offset+= BYTES_IN_CHAR;\
}


#define FILL_UCHAR_ENDIANITY_IN_UBUFFER_FUNC( _func, x, offset, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, BYTES_IN_CHAR)\
    UCHAR_2_ENDIANITY_UBUFFER( (unsigned char) (x), offset )\
    offset+= BYTES_IN_CHAR;\
}



#define FILL_UBUFFER_ENDIANITY_IN_UBUFFER_FUNC( _func, xbuffer, size_to_write, offset, length, buffer )\
{ \
	CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, size_to_write)\
    UBUFFER_2_UBUFFER( offset, xbuffer, size_to_write )\
    offset+= size_to_write;\
}

#define FILL_MAC_ADDRESS_IN_BUFFER_FUNC( _func, x, offset, length, buffer )\
{ \
    memset(offset,0,BYTES_IN_MAC_ADDRESS);\
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, BYTES_IN_MAC_ADDRESS)\
    PON_MAC_ADDRESS_COPY( offset, x )\
    offset+= BYTES_IN_MAC_ADDRESS;\
}

#define FILL_IPV6_ADDRESS_IN_BUFFER_FUNC( _func, x, offset, length, buffer )\
{ \
    memset(offset,0,CTC_MAX_IPV6_STRING_LENGTH);\
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, CTC_MAX_IPV6_STRING_LENGTH)\
    FILL_ULONG_ENDIANITY_IN_UBUFFER_FUNC(_func, x.addr_32[3], offset, length, buffer)\
    FILL_ULONG_ENDIANITY_IN_UBUFFER_FUNC(_func, x.addr_32[2], offset, length, buffer)\
    FILL_ULONG_ENDIANITY_IN_UBUFFER_FUNC(_func, x.addr_32[1], offset, length, buffer)\
    FILL_ULONG_ENDIANITY_IN_UBUFFER_FUNC(_func, x.addr_32[0], offset, length, buffer)\
}

#define FILL_OUI_IN_UBUFFER( oui, offset, length, buffer )\
{ \
    FILL_UCHAR_ENDIANITY_IN_UBUFFER((unsigned char)((oui >>(BYTES_IN_WORD*BITS_IN_BYTE))& 0xff), offset, length, buffer)\
    FILL_UCHAR_ENDIANITY_IN_UBUFFER((unsigned char)((oui >>BITS_IN_BYTE)& 0xff), offset, length, buffer)\
    FILL_UCHAR_ENDIANITY_IN_UBUFFER((unsigned char)(oui & 0xff), offset, length, buffer)\
}


#define EXTRACT_USHORT_ENDIANITY_FROM_UBUFFER( offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_WORD)\
    UBUFFER_2_USHORT( offset, x )\
    offset+= BYTES_IN_WORD;\
}


#define EXTRACT_USHORT_ENDIANITY_FROM_UBUFFER_FUNC( _func, offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, BYTES_IN_WORD)\
    UBUFFER_2_USHORT( offset, x )\
    offset+= BYTES_IN_WORD;\
}



#define EXTRACT_UCHAR_ENDIANITY_FROM_UBUFFER( offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN(offset, length, buffer, BYTES_IN_CHAR)\
    WORD_UBUFFER_2_ENDIANITY_UCHAR( offset, x )\
    offset+= BYTES_IN_CHAR;\
}


#define EXTRACT_UCHAR_ENDIANITY_FROM_UBUFFER_FUNC( _func, offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, BYTES_IN_CHAR)\
    WORD_UBUFFER_2_ENDIANITY_UCHAR( offset, x )\
    offset+= BYTES_IN_CHAR;\
}


#define EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER_FUNC( _func, offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, BYTES_IN_LONG)\
    UBUFFER_2_ULONG( offset, x )\
    offset+= BYTES_IN_LONG;\
}



#define EXTRACT_LIMITED_LONG_ENDIANITY_FROM_UBUFFER_FUNC( _func, offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, 3)\
    UBUFFER_2_LIMITED_LONG( offset, x )\
    offset+= 3;\
}


#define EXTRACT_MAC_ADDRESS_FROM_BUFFER( __func, offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(__func, offset, length, buffer, BYTES_IN_MAC_ADDRESS)\
    PON_MAC_ADDRESS_COPY( x, offset )\
    offset+= BYTES_IN_MAC_ADDRESS;\
}

#define EXTRACT_IPV6_ADDRESS_FROM_BUFFER( __func, offset, x, length, buffer )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(__func, offset, length, buffer, CTC_MAX_IPV6_STRING_LENGTH)\
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER_FUNC(__func, offset, x.addr_32[3], length, buffer)\
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER_FUNC(__func, offset, x.addr_32[2], length, buffer)\
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER_FUNC(__func, offset, x.addr_32[1], length, buffer)\
    EXTRACT_ULONG_ENDIANITY_FROM_UBUFFER_FUNC(__func, offset, x.addr_32[0], length, buffer)\
}

#define EXTRACT_OPCODE_RETURN_IF_ERROR_FUNC(__func, receive_pdu_data,_received_opcode_, length, buffer) \
{\
    EXTRACT_UCHAR_ENDIANITY_FROM_UBUFFER_FUNC(__func, receive_pdu_data, _received_opcode_, length, buffer)\
    CHECK_OPCODE_AND_RETURN_FUNC(__func)\
}



#define CHECK_BRANCH_AND_RETURN_FUNC(__func, __expected_branch__, __received_branch__) \
{   if( __received_branch__ != __expected_branch__ )\
    {\
		PONLOG_ERROR_2( __func, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
				    "Wrong branch: received-0x%x,expected-0x%x \n", __received_branch__,__expected_branch__); \
      return COMPARSER_STATUS_WRONG_OPCODE;\
    }\
}

#define CHECK_LEAF_AND_RETURN_FUNC(__func, __expected_leaf__, __received_leaf__) \
{   if( __received_leaf__ != __expected_leaf__ )\
    {\
		PONLOG_ERROR_2( __func, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
				    "Wrong leaf: received-0x%x,expected-0x%x \n", __received_leaf__,__expected_leaf__); \
      return COMPARSER_STATUS_WRONG_OPCODE;\
    }\
}

#define EXTRACT_BRANCH_LEAF(__func, receive_pdu_data,expected_branch,expected_leaf, length, buffer) \
{\
	unsigned char received_branch; \
	unsigned short received_leaf; \
    EXTRACT_UCHAR_ENDIANITY_FROM_UBUFFER_FUNC(__func, receive_pdu_data, received_branch, length, buffer)\
	EXTRACT_USHORT_ENDIANITY_FROM_UBUFFER_FUNC(__func, receive_pdu_data, received_leaf, length, buffer)\
    CHECK_BRANCH_AND_RETURN_FUNC(__func, expected_branch, received_branch)\
	CHECK_LEAF_AND_RETURN_FUNC(__func, expected_leaf, received_leaf)\
}


#define EXTRACT_BRANCH_LEAF_OR_CHECK_IF_ZERO(__func, receive_pdu_data,expected_branch,expected_leaf, length, buffer) \
{\
	unsigned char received_branch; \
	unsigned short received_leaf; \
    if((receive_pdu_data + BYTES_IN_CHAR - buffer) > length)\
    {\
        return COMPARSER_STATUS_END_OF_FRAME;\
    }\
    WORD_UBUFFER_2_ENDIANITY_UCHAR( receive_pdu_data, received_branch )\
    receive_pdu_data+= BYTES_IN_CHAR;\
	if(received_branch == 0)\
		return COMPARSER_STATUS_END_OF_FRAME;\
    if((receive_pdu_data + BYTES_IN_WORD - buffer) > length)\
    {\
        return COMPARSER_STATUS_END_OF_FRAME;\
    }\
    UBUFFER_2_USHORT( receive_pdu_data, received_leaf )\
    receive_pdu_data+= BYTES_IN_WORD;\
}

#define CHECK_MANAGEMENT_OBJECT_LEAF_AND_RETURN_FUNC(__func, __min_expected_leaf__, __max_expected_leaf__, __received_leaf__) \
{   if( __received_leaf__ < __min_expected_leaf__ || __received_leaf__ > __max_expected_leaf__ )\
    {\
		PONLOG_ERROR_3( __func, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, \
				    "Wrong leaf: received-0x%x,expected-between 0x%x and 0x%x\n", __received_leaf__, __min_expected_leaf__, __max_expected_leaf__); \
      return COMPARSER_STATUS_WRONG_OPCODE;\
    }\
}

#define EXTRACT_MANAGEMENT_OBJECT_BRANCH_LEAF(__func, receive_pdu_data, expected_branch, min_expected_leaf, max_expected_leaf, received_leaf, length, buffer) \
{\
	unsigned char received_branch; \
    EXTRACT_UCHAR_ENDIANITY_FROM_UBUFFER_FUNC(__func, receive_pdu_data, received_branch, length, buffer)\
	EXTRACT_USHORT_ENDIANITY_FROM_UBUFFER_FUNC(__func, receive_pdu_data, received_leaf, length, buffer)\
    CHECK_BRANCH_AND_RETURN_FUNC(__func, expected_branch, received_branch)\
	CHECK_MANAGEMENT_OBJECT_LEAF_AND_RETURN_FUNC(__func, min_expected_leaf, max_expected_leaf, received_leaf)\
}


#define EXTRACT_MANAGEMENT_OBJECT_BRANCH_LEAF_OR_CHECK_IF_ZERO(__func, receive_pdu_data, received_leaf, length, buffer) \
{\
	unsigned char received_branch; \
    if((receive_pdu_data + BYTES_IN_CHAR - buffer) > length)\
    {\
        return COMPARSER_STATUS_END_OF_FRAME;\
    }\
    WORD_UBUFFER_2_ENDIANITY_UCHAR( receive_pdu_data, received_branch )\
    receive_pdu_data+= BYTES_IN_CHAR;\
	if(received_branch == 0)\
		return COMPARSER_STATUS_END_OF_FRAME;\
    if((receive_pdu_data + BYTES_IN_WORD - buffer) > length)\
    {\
        return COMPARSER_STATUS_END_OF_FRAME;\
    }\
    UBUFFER_2_USHORT( receive_pdu_data, received_leaf )\
    receive_pdu_data+= BYTES_IN_WORD;\
}


#define CHECK_ALARM_ID_AND_EXTRACT_MANAGEMENT_OBJECT_LEAF(__func, __alarm_id__, __leaf__) \
{\
	if (__alarm_id__ >= 0x0001 && __alarm_id__ <= 0x00FF)\
	{\
		__leaf__ = CTC_MANAGEMENT_OBJECT_LEAF_NONE; /* ONU alarm has no leaf (not need management object) */\
	}\
	else if (__alarm_id__ >= 0x0101 && __alarm_id__ <= 0x01FF)\
	{\
		__leaf__ = CTC_MANAGEMENT_OBJECT_LEAF_PON_IF;\
	}\
	else if (__alarm_id__ >= 0x0201 && __alarm_id__ <= 0x02FF)\
	{\
		__leaf__ = CTC_MANAGEMENT_OBJECT_LEAF_CARD;\
	}\
	else if (__alarm_id__ >= 0x0301 && __alarm_id__ <= 0x06FF)\
	{\
		__leaf__ = CTC_MANAGEMENT_OBJECT_LEAF_PORT;\
	}\
	else\
	{\
		PONLOG_ERROR_1(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "Invalid alarm id: 0x%x.\n", __alarm_id__);\
		return (COMPARSER_STATUS_BAD_PARAM);\
	}\
}


#define BUILD_MANAGEMENT_OBJECT_STRUCT_FUNC(__func, __management_object__, __leaf__, __management_object_index__)\
{\
	/* check index bounds */ \
	PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(__func, __leaf__, leaf, CTC_MANAGEMENT_OBJECT_LEAF_PORT, (CTC_MANAGEMENT_OBJECT_LEAF_LAST-1))\
	PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(__func, __management_object_index__.frame_number, frame_number, CTC_MIN_MANAGEMENT_OBJECT_FRAME_NUMBER, CTC_MAX_MANAGEMENT_OBJECT_FRAME_NUMBER)\
	if (__management_object_index__.slot_number != CTC_MANAGEMENT_OBJECT_ALL_SLOTS)\
	{\
		PON_HANDLE_ERROR_CHECK_UNSIGNED_IN_BOUNDS_FUNC(__func, __management_object_index__.slot_number, slot_number, CTC_MIN_MANAGEMENT_OBJECT_SLOT_NUMBER, CTC_MAX_MANAGEMENT_OBJECT_SLOT_NUMBER);\
	}\
	/* check index validity */ \
	if ( (__leaf__ == CTC_MANAGEMENT_OBJECT_LEAF_PON_IF) && (__management_object_index__.frame_number != 0 || __management_object_index__.slot_number != 0 || __management_object_index__.port_number != 0) )\
	{\
		PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"Invalid management object index: frame_number, slot_number and port_number must be 0 when leaf equals CTC_MANAGEMENT_OBJECT_LEAF_PON_IF\n");\
		return COMPARSER_STATUS_BAD_PARAM;\
	}\
	if ( (__leaf__ == CTC_MANAGEMENT_OBJECT_LEAF_LLID) && (__management_object_index__.frame_number != 0 || __management_object_index__.slot_number != 0 || __management_object_index__.port_number != 0) )\
	{\
		PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"Invalid management object index: frame_number, slot_number and port_number must be 0 when leaf equals CTC_MANAGEMENT_OBJECT_LEAF_LLID\n");\
		return COMPARSER_STATUS_BAD_PARAM;\
	}\
	if ( (__leaf__ == CTC_MANAGEMENT_OBJECT_LEAF_CARD) && (__management_object_index__.port_number != 0) )\
	{\
		PONLOG_ERROR_0(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"Invalid management object index: port_number must be 0 when leaf equals CTC_MANAGEMENT_OBJECT_LEAF_CARD\n");\
		return COMPARSER_STATUS_BAD_PARAM;\
	}\
	if ( (__leaf__ == CTC_MANAGEMENT_OBJECT_LEAF_PORT) )\
	{\
		/* for port no limit */ \
	}\
	__management_object__.leaf = __leaf__;\
	__management_object__.index.frame_number = __management_object_index__.frame_number;\
	__management_object__.index.slot_number = __management_object_index__.slot_number;\
	__management_object__.index.port_number = __management_object_index__.port_number;\
	__management_object__.index.port_type = __management_object_index__.port_type;\
}

#define EXTRACT_OPCODE_RETURN_IF_ERROR(receive_pdu_data,received_opcode,result, length, buffer) \
{\
    EXTRACT_UCHAR_ENDIANITY_FROM_UBUFFER(receive_pdu_data, received_opcode, length, buffer)\
    CHECK_OPCODE_AND_RETURN\
}

#define EXTRACT_CODE_RETURN_IF_ERROR(receive_pdu_data,received_code,result, length, buffer) \
{\
    EXTRACT_UCHAR_ENDIANITY_FROM_UBUFFER(receive_pdu_data, received_code, length, buffer)\
    if( received_code != expected_code )\
    {\
      PONLOG_ERROR_2(PONLOG_FUNC_GENERAL, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT,"Unexpected code: received-0x%lx,expected-0x%lx \n", received_code,expected_code);\
      return COMPARSER_STATUS_WRONG_OPCODE;\
    }\
}

#define EXTRACT_UBUFFER_ENDIANITY_FROM_UBUFFER_FUNC( _func, offset, x, length, buffer, size_to_read )\
{ \
    CHECK_BUFFER_SIZE_AND_RETURN_FUNC(_func, offset, length, buffer, size_to_read)\
    UBUFFER_2_UBUFFER( x, offset, size_to_read )\
    offset+= size_to_read;\
}

#define STEP_OVER_OBJECT_CONTEXT_TLV(olt_id, onu_id, buffer)\
{\
	unsigned char	branch_code,leaf_size;\
	unsigned short	object_leaf_code;\
	WORD_UBUFFER_2_ENDIANITY_UCHAR( buffer, branch_code )\
	if (CTC_COMPARSER_is_step_over_branch(branch_code))\
	{\
		buffer += BYTES_IN_CHAR;\
		UBUFFER_2_USHORT( buffer, object_leaf_code )\
		buffer += BYTES_IN_WORD;\
		WORD_UBUFFER_2_ENDIANITY_UCHAR( buffer, leaf_size )\
		buffer += BYTES_IN_CHAR;\
		buffer += leaf_size;\
	}\
}
/*============================== Data Types =================================*/

#ifndef __PASSAVE_ONU__

/* contain the OUI and version pairs */
typedef struct 
{
    OAM_oui_t			oui;
    unsigned char       version;
} CTC_STACK_oui_version_record_t;

typedef struct  
{
    bool                state;		/* Is threshold configuration active for the queue */
    unsigned short      threshold;  /* Queue threshold: range 0 - 65535 */
} CTC_STACK_onu_queue_threshold_report_t; 

#define MAX_QUEUE_NUMBER 8
typedef struct  
{
    CTC_STACK_onu_queue_threshold_report_t      queue[MAX_QUEUE_NUMBER];
} CTC_STACK_onu_queue_set_thresholds_t;

typedef struct
{                                                                                                                   
	PON_binary_source_t			  source;	/* Type of device where the binary currently resides	                */
	void					     *location;	/* Location of the binary, different type for different sources:        */
											/* MEMORY source - Pointer to the beginning of the binary               */
											/* FILE   source - The name of the file containing the binary           */
                                            /* FLASH  source - Not required	                                        */
	long int					  size;		/* Size of the binary, stated in bytes. Required only for MEMORY source */
} CTC_STACK_binary_t;

/*========================= Functions Prototype =============================*/


/* Debug function */
short int PON_CTC_STACK_timeout_simulate 
                    ( const PON_olt_id_t  olt_id, 
                      const PON_onu_id_t  onu_id,
                      const bool          timeout_mode );

#endif	/* __PASSAVE_ONU__ */

#endif /* _CTC_STACK_DEFINES_H__ */

