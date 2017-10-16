/*
**
**  This software is licensed for use according to the terms set by
**  the Passave API license agreement.
**  Copyright (c) 2001 - 2006 Passave Ltd.
**  Kohav Herzelia bldg. 4 Hasadna'ot st.
**  POB 2089, Herzliya Pituach, 46120, ISRAEL
**
**
**	Project:	CTC Stack
**  File:		CTC_STACK_frame_struct.h - Frame structure of various frames of the CTC Stack
**
**  This file was written by Adi Shilo, adi.shilo@passave.com, 05-mar-2006
**
**
**  Changes:
**
**  Version   |  Date       |    Change               |    Author
**  ----------|-------------|-------------------------|------------
**     1.0    | 05-mar-2006 | Creation                | Adi Shilo
*/
#ifndef __CTC_STACK_FRAME_STRUCT_H__
#define __CTC_STACK_FRAME_STRUCT_H__

#define ETHERNET_MTU 1518	/* The length (in bytes) of an Ethernet MTU */
#define CTC_OVER_LENGTH_FRAME 1536	/* The maximum length (in bytes) of CTC over-length frame */

#define OAM_ETHERNET_HEADER_SIZE 18	/* The size of the Ethernet and OAM header in bytes */
#define OAM_ETHERNET_STD_TLV_SIZE 16	/* The size of a TLV (local & remote) of the OAM-INF */
#define OAM_ETHERNET_EX_TLV_STATIC_SIZE 7	/* The size of the static fields in the extended TLV of the OAM-INF (without the version list, which is dynamic in size) */
#define OAM_ETHERNET_VEND_EXT_HEADER_SIZE 4	/* The size of the OAM vendor extension header */
#define OAM_ETHERNET_VEND_EXT_DBA_HEADER_SIZE 3	/* The size of the header of CTC DBA OAM vendor extension */


#define OAM_STANDARD_REQUEST 0x02			/* OAM standard type for request */
#define OAM_STANDARD_RESPONSE 0x03			/* OAM standard type for response */

#define MAX_TLV_SIZE 128

#define EX_OAM_INF_TYPE 0xfe					/* Extended OAM Discovery, InfoType field value */
#define OAM_VENDOR_EXT_GET_REQUEST 0x01			/* OAM Vendor Extension type for CTC get request */
#define OAM_VENDOR_EXT_GET_RESPONSE 0x02		/* OAM Vendor Extension type for CTC get response */
#define OAM_VENDOR_EXT_SET_REQUEST 0x03			/* OAM Vendor Extension type for CTC set request */
#define OAM_VENDOR_EXT_SET_RESPONSE 0x04		/* OAM Vendor Extension type for CTC set response */
#define OAM_VENDOR_EXT_DBA 0x0a					/* OAM Vendor Extension type for CTC DBA reports configuration */
#define OAM_VENDOR_EXT_ENC 0x09					/* OAM Vendor Extension type for CTC ENC (Churning) configuration */
#define OAM_VENDOR_EXT_UPDATE_FIRMWARE 0x06	    /* OAM Vendor Extension type for CTC update ONU Firmware */
#define OAM_VENDOR_EXT_ONU_EVENT 0x0F           /* OAM Vendor Extension type for CTC ONU Event notification */
#define OAM_VENDOR_EXT_ONU_AUTHENTICATION 0x05  /* OAM Vendor Extension type for CTC ONU authentication */


/* DBA OAM vendor extension sub-types */
#define DBA_VENDOR_EXT_GET_REQUEST 0x00		/* get_DBA_request */
#define DBA_VENDOR_EXT_GET_RESPONSE 0x01	/* get_DBA_response */
#define DBA_VENDOR_EXT_SET_REQUEST 0x02		/* set_DBA_request */
#define DBA_VENDOR_EXT_SET_RESPONSE 0x03	/* set_DBA_response */

/* ENC OAM vendor extension sub-types */
#define ENC_VENDOR_EXT_KEY_REQUEST 0x00		/* new_key_request */
#define ENC_VENDOR_EXT_KEY_RESPONSE 0x01	/* new_churning_key */

/* Auth OAM vendor extension sub-types */
#define AUTH_VENDOR_EXT_REQUEST  0x01		/* request ONU its ONU_ID and Password */
#define AUTH_VENDOR_EXT_RESPONSE 0x02	    /* response of ONU to the Auth_Request */
#define AUTH_VENDOR_EXT_SUCCESS  0x03		/* indicate the ONU has passed authentication */
#define AUTH_VENDOR_EXT_FAILURE  0x04	    /* indicate ONU failed in authentication */


/* TFTP vendor extension sub-types */
typedef enum
{
	TFTP_UPDATE_FIRMWARE_OPCODE_READ=1,
	TFTP_UPDATE_FIRMWARE_OPCODE_WRITE,
	TFTP_UPDATE_FIRMWARE_OPCODE_FILE_TRANSFER,
	TFTP_UPDATE_FIRMWARE_OPCODE_FILE_ACK,
	TFTP_UPDATE_FIRMWARE_OPCODE_ERROR,
	TFTP_UPDATE_FIRMWARE_OPCODE_END_DOWNLOAD_REQUEST,
	TFTP_UPDATE_FIRMWARE_OPCODE_END_DOWNLOAD_RESPONSE,
	TFTP_UPDATE_FIRMWARE_OPCODE_ACTIVATE_IMAGE_REQUEST,
	TFTP_UPDATE_FIRMWARE_OPCODE_ACTIVATE_IMAGE_RESPONSE,
	TFTP_UPDATE_FIRMWARE_OPCODE_COMMIT_IMAGE_REQUEST,
	TFTP_UPDATE_FIRMWARE_OPCODE_COMMIT_IMAGE_RESPONSE,
}TFTP_update_firmware_opcode_t;


typedef enum
{
	TFTP_DATA_TYPE_DATA=1,
	TFTP_DATA_TYPE_FILE_INTEGRITY,
	TFTP_DATA_TYPE_ACTIVE_IMAGE,
	TFTP_DATA_TYPE_COMMIT_IMAGE,

}TFTP_data_type_t;

typedef enum
{
	TFTP_RPS_CODE_VERIFICATION_SUCCESS=0,
	TFTP_RPS_CODE_STILL_WRITING_SOFTWARE,
	TFTP_RPS_CODE_VERIFICATION_ERROR,
 	TFTP_RPS_CODE_PARAMETER_ERROR,
 	TFTP_RPS_CODE_NOT_SUPPORTED,

}TFTP_rps_code_t;

typedef enum
{
	TFTP_ACTIVE_IMAGE_ACK_SUCCESS=0,
	TFTP_ACTIVE_IMAGE_ACK_PARAMETER_ERROR,
	TFTP_ACTIVE_IMAGE_ACK_NOT_SUPPORTED,
 	TFTP_ACTIVE_IMAGE_ACK_LOAD_FAILED = 5,

}TFTP_activate_image_ack_t;

typedef enum
{
    TFTP_ACTIVE_BACKUP_IMAGE=0,                      /*Active the image in backup storage*/
    TFTP_ACTIVE_RESERVED_FLAG,

}TFTP_activate_image_request_t;

typedef enum
{
	TFTP_COMMIT_IMAGE_ACK_SUCCESS=0,
	TFTP_COMMIT_IMAGE_ACK_PARAMETER_ERROR,
	TFTP_COMMIT_IMAGE_ACK_NOT_SUPPORTED,
 	TFTP_COMMIT_IMAGE_ACK_LOAD_FAILED = 5,

}TFTP_commit_image_ack_t;

typedef enum
{
    TFTP_COMMIT_BACKUP_IMAGE=0,                /*Commit the image in backup storage*/
    TFTP_COMMIT_RESERVED_FLAG,

}TFTP_commit_image_request_t;

#define COMPARSER_HEADER_PLACE        OAM_ETHERNET_HEADER_SIZE+OAM_OUI_SIZE

#define CTC_TFTP_TRANSFER_DATA_BLOCK_SIZE		1400

#define TFTP_DATA_TYPE_SIZE			1
#define TFTP_LENGTH_SIZE			2
#define TFTP_TID_SIZE				2
#define TFTP_COMMON_HEADER	TFTP_DATA_TYPE_SIZE+TFTP_LENGTH_SIZE+TFTP_TID_SIZE/*1+2+2=5*/
  
#define TFTP_TIME_TO_RESEND_END_DOWNLOAD_REQUEST 30000 /*30sec*/ 
#define TFTP_NUM_OF_RESEND_END_DOWNLOAD_REQUEST  6 


#define TFTP_GEN_ERROR								0 
#define TFTP_FILE_NOT_FOUND							1
#define TFTP_ACCESS_VIOLATION						2
#define TFTP_DISK_FULL_OR_ALLOCATION_EXCEEDS		3
#define TFTP_ILLEGAL_OPERATION						4
#define TFTP_UNKNOWN_TRANSFER_ID					5
#define TFTP_FILE_ALREADY_EXIST						6
#define TFTP_NO_SUCH_USER							7

#endif	/* __CTC_STACK_FRAME_STRUCT_H__ */
