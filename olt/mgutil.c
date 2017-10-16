/***********************************************************************
*     Copyright(C), 2000-2016 GW TECHNOLOGIES CO.,LTD.
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*              GW Technologies Co.,Ltd.
*
*     All information contained in this document is    GW Technologies Co.,Ltd.
*     company private, proprietary, and trade secret.

*  mgutil.c -- 
*     contains functions that is used for parsing sysfile.ini 
*  
*   Date: 			2009/05/20
*   Author:			chen fujun
*   content:
**  History:
**   Date          |    Name         |     Description
**---- ----- |-----------|------------------ 
**  09/05/20  |   chenfj          |     create 
**----------|-----------|------------------
*
***********************************************************************/
#ifdef __cplusplus
extern "C"
 {
#endif

#include "vos.h"
#include "vos_base.h"
#include "vos_task.h"
#include "vos_que.h"
#include "vos_sem.h"
#include "vos_time.h"
#include "vos_string.h"
#include "vos_io.h"
#include "vos_timer.h"
#include "sys/console/sys_console.h"
#include "cli/cli.h"
#include "sys/main/sys_main.h"
#include "vos/vospubh/vos_ctype.h"
#include  "cli/cl_vty.h"

#include  "OltGeneral.h"
#include  "PonGeneral.h"
#include  "OnuGeneral.h"
#include  "V2R1_product.h"

#define  COMMENT_CHAR 			';'    /* ; */
#define  BLOCK_DELIMITER_START  	'{'	/* { */
#define  BLOCK_DELIMITER_END    	 '}'	/* } */
#define  LINE_DELIMITER_START  	'\"'	/* " */
#define  LINE_DELIMITER_END  		'\"'	/* " */
#define  EVALUATE_DELIMITER		"="   /* = */

/* olt vendor information */
#define  KEY_OLT_VENDOR_INFORMATION   "OLT_VENDOR_INFORMATION"

#define  KEY_VENDOR_INFO   		"VENDOR_INFO"
#define  KEY_VENDOR_LOCATION 	"VENDOR_LOCATION"
#define  KEY_VENDOR_COPYRIGHT 	"VENDOR_COPYRIGHT"
#define  KEY_VENDOR_NAME			"VENDOR_NAME"
#define  KEY_VENDOR_NAME_SHORT  "VENDOR_NAME_SHORT"
#define  KEY_VENDOR_NAME_AB  	"VENDOR_NAME_AB"
#define  KEY_PRODUCT_SERIES  		"PRODUCT_SERIES"
#define  KEY_PRODUCT_SOFTWARE   	"PRODUCT_SOFTWARE"

/* olt product info */
#define  KEY_OLT_PRODUCT_INFOMATION		"OLT_PRODUCT_INFOMATION"


#define  KEY_PRODUCT_TYPE_8000  			"PRODUCT_TYPE_8000"
#define  KEY_PRODUCT_DESCRIPTION_8000 	"PRODUCT_DESCRIPTION_8000"
#define  KEY_PRODUCT_TYPE_8000M  			"PRODUCT_TYPE_M8000"
#define  KEY_PRODUCT_DESCRIPTION_8000M 	"PRODUCT_DESCRIPTION_M8000"
#define  KEY_PRODUCT_TYPE_8000S  			"PRODUCT_TYPE_S8000"
#define  KEY_PRODUCT_DESCRIPTION_8000S 	"PRODUCT_DESCRIPTION_S8000"

#define  KEY_PRODUCT_TYPE_6900  			"PRODUCT_TYPE_6900"
#define  KEY_PRODUCT_DESCRIPTION_6900 	"PRODUCT_DESCRIPTION_6900"
#define  KEY_PRODUCT_TYPE_6900M  			"PRODUCT_TYPE_M6900"
#define  KEY_PRODUCT_DESCRIPTION_6900M 	"PRODUCT_DESCRIPTION_M6900"
#define  KEY_PRODUCT_TYPE_6900S  			"PRODUCT_TYPE_S6900"
#define  KEY_PRODUCT_DESCRIPTION_6900S 	"PRODUCT_DESCRIPTION_S6900"

#define  KEY_PRODUCT_TYPE_6700  			"PRODUCT_TYPE_6700"
#define  KEY_PRODUCT_DESCRIPTION_6700 	"PRODUCT_DESCRIPTION_6700"

#define  KEY_PRODUCT_TYPE_6100 			"PRODUCT_TYPE_6100"
#define  KEY_PRODUCT_DESCRIPTION_6100 	"PRODUCT_DESCRIPTION_6100"

#define  KEY_PRODUCT_TYPE_8100 			"PRODUCT_TYPE_8100"
#define  KEY_PRODUCT_DESCRIPTION_8100 	"PRODUCT_DESCRIPTION_8100"


/* board */
#define  KEY_OLT_BOARD					"OLT_BOARD"
#define  KEY_ONU_BOARD					"ONU_BOARD"
#define  KEY_BOARD_SHORT                          "_SHORT"

/* onu vendor */
#define  KEY_ONU_VENDOR_INFO			"ONU_VENDOR_INFO"

#define  KEY_ONU_VENDOR 					"ONU_VENDOR"

/* onu type */
#define  KEY_ONU_TYPE						"ONU_TYPE"


#define  KEY_ONU_APP_STRING_PREFIX		"ONU_APP_STRING_PREFIX"
#define  KEY_TOTAL_TYPE					"TOTAL_TYPE"
#define  KEY_TYPE     						"TYPE"

#define  KEY_TYPE_INTEGER                          "TYPE_INTEGER"
#define  KEY_TYPE_STRING 					"TYPE_STRING"
#define  KEY_DESC_STRING 					"DESC_STRING"
#define  KEY_MAX_SLOT 					"MAX_SLOT"
#define  KEY_FE_PORT_NUM 				"FE_PORT_NUM"
#define  KEY_GE_PORT_NUM 				"GE_PORT_NUM"
#define  KEY_POTS_PORT_NUM 				"POTS_PORT_NUM"
#define  KEY_VOIP_PORT_NUM 				"VOIP_PORT_NUM"
#define  KEY_E1_PORT_NUM 				"E1_PORT_NUM"
#define  KEY_APP_EPON_ID 					"APP_EPON_ID"
#define  KEY_APP_VOICE_ID 				"APP_VOICE_ID"
#define  KEY_APP_FPGA_ID 					"APP_FPGA_ID"
#define  KEY_CTC_REGISTER_ID 				"CTC_REGISTER_ID"

/*ctc onu model*/
#define KEY_ONU_MODEL                   "ONU_MODEL"
#define KET_GONU_EQUIPMENT              "ONU_EQUIPMENT"
#define KEY_TOTAL_MODEL                 "TOTAL_MODELS"
#define KEY_MODEL                       "MODEL"
#define KEY_MODEL_ID                    "MODEL_ID"
#define KEY_MODE_STR                    "MODEL_STRING"

/*gpon onu equipment*/
#define KEY_EQUIPMENT_ID                "EQUIPMENT_ID"
#define KEY_EQUIPMETMODE_STR            "EQUIPMENT_STRING"
/*ctc onu chipset*/
#define KEY_CHIPSET                     "ONU_CHIPSET"

#define STRING_RETURN_AND_LINE            0x5c725c6e

int  SYS_FILE_DEBUG= V2R1_DISABLE;

/***********************************************************************
*
*  isEOL
*
*  PURPOSE:
*     Check end of line (EOL) character
*
*  PARAMETERS:
*     ch - [in] character to be checked
*
*  RETURNS:
*     BOOL - true if input character is an EOL character, else false.
*
*  NOTE:
* 
*
***********************************************************************/
BOOL  isEOL ( const char ch )
{
   return (( ch == '\r') || (ch == '\n' ));
}

/***********************************************************************
*
*  isWhiteSpace
*
*  PURPOSE:
*     Check white space
*
*  PARAMETERS:
*     ch - [in] character to be checked
*
*  RETURNS:
*     BOOL - true if input character is a white space, else false.
*
*  NOTE:
*
***********************************************************************/
BOOL isWhiteSpace ( const char ch )
{
   return ( ch == ' ' || ch == '\t' );
}

/***********************************************************************
*
*  isRestChar
*
*  PURPOSE:
*     Check rest character
*
*  PARAMETERS:
*     ch - [IN] character to be checked
*
*  RETURNS:
*     BOOL - true if input character is a rest char, else false.
*
*  NOTE:
*    
*
***********************************************************************/
BOOL isRestChar ( const char ch )
{
   return ( ch == ';' || ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
            ch == ':' || ch == ',' || ch == '#' || ch == '<' || ch == '>' ||
            ch == '=' || ch == '\"' );
}

/***********************************************************************
*
*  isSafeChar
*
*  PURPOSE:
*     Check safe character
*
*  PARAMETERS:
*     ch - [in] character to be checked
*
*  RETURNS:
*     BOOL - true if input character is a safe char, else false.
*
*  NOTE:
*  
*
***********************************************************************/
BOOL isSafeChar ( const char ch)
{
   return ( ch == '+' || ch == '-' || ch == '&' || ch == '!' || ch == '_' ||
            ch == '/' || ch == '\''|| ch == '?' || ch == '@' || ch == '^' ||
            ch == '`' || ch == '~' || ch == '*' || ch == '$' || ch == '\\'||
            ch == '(' || ch == ')' || ch == '%' || ch == '.' /*|| isalnum((int)ch)*/);
}

/***********************************************************************
*
*  isComment
*
*  PURPOSE:
*     Check beginning tag of a comment
*
*  PARAMETERS:
*     ch - [IN] character to be checked
*
*  RETURNS:
*     BOOL - true if input character is a comment tag, else false
*
*  NOTE:
* 
*
***********************************************************************/
BOOL isComment ( const char ch )
{
   return ( ch == COMMENT_CHAR );
}


/***********************************************************************
*
*  skipComment
*
*  PURPOSE:
*     Skip comments
*
*  PARAMETERS:
*     str - [in] ptr to a string where the comments are skipped
*
*  RETURNS:
*     BOOL - true if input character is a comment, else false.
*
*  NOTE:
*  
*
***********************************************************************/
char * skipComment ( char * const str )
{
   char *p;

   p = str;

   while( *p)
   	{
	if( isEOL( *p))
		break;
	p++;
   	}

   while ( *p )
   {
      if ( !(isEOL ( *p )) )
      {
         break;
      }
      p++;
   }

   return p;
}

/***********************************************************************
*
*  FindNextBlock -- 
*
*  PARAMETERS:
*     str          - [IN] string
*     currentlevel - [OUT] Number of delimiters that has not been paired
*                    such as (number of '{' - number of '}' encountered)
*
*  RETURNS:
*     char* - Ptr to the first char after the end of the current block
*             (terminated by the } character)
*
***********************************************************************/
char   *FindNextBlock(char *str, int currentlevel)
{
       int     i = currentlevel;
       char   *p = str; 

       /* Find the first delimiter */
       while (i == 0 && *p)
       {
              if (*p == BLOCK_DELIMITER_START)
              {
                     i++;
              }
              else if (*p == '}')
              {
                     return p;
              }
			  
              p++;
       }

 

       while (i && *p)
       {
              if (*p == BLOCK_DELIMITER_END )
              {
                     i--;
              }

              else if (*p == BLOCK_DELIMITER_START )
              {
                     i++;
              }

              p++;
       }

       return p;
}
/***********************************************************************
*
*  FindNextLineHeader -- 
*
*  PARAMETERS:
*     str          - [IN] string
*
*  RETURNS:
*     char* - Ptr to the first char after the start of the current line
*             (terminated by the \" character)
*
***********************************************************************/
char   *FindNextLineHeader(char *str)
{
       char   *p = str; 

       /* Find the first delimiter " */
       while (*p)
       {
              if (*p == LINE_DELIMITER_START)
              {
			p++;
			break;
              }
              p++;
       }
	
       return p;
}


/***********************************************************************
*
*  FindNextLineTail -- 
*
*  PARAMETERS:
*     str          - [IN] string
*
*  RETURNS:
*     char* - Ptr to the first char after the end of the current line
*             (terminated by the \" character)
*
***********************************************************************/
char   *FindNextLineTail(char *str)
{
       char   *p = str; 

       /* Find the first delimiter " */
       while (*p)
       {
              if (*p == LINE_DELIMITER_START)
              {
			p++;
			break;
              }
              p++;
       }

       while (*p)
       {
              if (*p == LINE_DELIMITER_END )
              {
			return (p-1);
              }

              p++;
       }

       return p;
}


/***********************************************************************
*
*  SkipLineWhiteSpace
*
*  PURPOSE:
*     Skip white spaces on a line including comments, white space and
*     EOL characters
*
*  PARAMETERS:
*     str - [in] ptr to a string where the comments are skipped
*
*  RETURNS:
*      char* - Ptr to the first char after the end of the white space
*
*  NOTE:
* 
*
***********************************************************************/
char * SkipLineWhiteSpace (char *str)
{
   char *p;

   p = str;
   while (*p)
   {
      if (isComment (*p))
      {
         /* comments */
         p = skipComment (++p);
      }
      else if (isWhiteSpace (*p) || isEOL (*p))
      {
         /* white space / EOL characters */
         p++;
      }
	else if( *p == 0)
	{
		p++;
	}
      else
      {
         /* does not match */
         return p;
      }      
   }
   return p;
}

/***********************************************************************
*
*  SkipLine
*
*  PURPOSE:
*     Skip a line including EOL characters
*
*  PARAMETERS:
*     str - [in] ptr to a string where the comments are skipped
*
*  RETURNS:
*      char* - Ptr to the first char after the end of the line
*
*  NOTE:
* 
*
***********************************************************************/
char * SkipLine (char *str)
{
   char *p;

   p = str;
   while (*p)
   {
      if (isComment (*p))
      {
         /* comments */
         p = skipComment (++p);
      }
      else if (isWhiteSpace (*p))
      {
         /* white space */
         p++;
      }
      else if( isEOL (*p))
      {
         /*  EOL characters */
         p = SkipLineWhiteSpace(++p);
         return p;
      }
	else 
		p++;
   }
   return p;
}

char *ParseStringLine(char *p_line, char **p_line_head, char **p_line_tail)
{ 
	p_line = SkipLineWhiteSpace(p_line);
	*p_line_head = NULL;
	*p_line_tail = NULL;
	
	*p_line_head = VOS_StrStr(p_line, EVALUATE_DELIMITER);
	if(p_line_head != NULL)
		{
		p_line = ++(*p_line_head);
		p_line = SkipLineWhiteSpace(p_line);
		
		*p_line_head = FindNextLineHeader(p_line);
		*p_line_tail = FindNextLineTail(p_line);
		}

	return(SkipLine(p_line));
}

int ParseDigitalLine(char *p_line)
{ 
	int  isDigit_all;
	char *p_line_head;
	int digitalNum;
	char *pToken;
	
	p_line = SkipLineWhiteSpace(p_line);
	isDigit_all = 1;
	digitalNum = 0;
	
	p_line = VOS_StrStr(p_line, EVALUATE_DELIMITER);
	if(p_line != NULL)
		{
		p_line++;
		p_line = SkipLineWhiteSpace(p_line);
		p_line_head = p_line;
		
		while( !isEOL(*p_line_head))
			{
			if(!(vos_isdigit(*p_line_head)))
				{
				isDigit_all = 0;
				}
			p_line_head++;
			}
		if(isDigit_all == 1 )
			digitalNum = (int) VOS_AtoL(p_line);
		else 
			digitalNum = ( int )VOS_StrToUL(p_line, &pToken, 16 );
		}
	
	return(digitalNum);
}


/***********************************************************************
*
*  ParseOltVendorInformation
*
*  PURPOSE:
*     parse OLT vendor infomation, and config the olt vendor info to database
*
*  PARAMETERS:
*     VendorInfo - [in] start ptr to OLT vendor infomation
*     VendorInfo_end - [in] end ptr to OLT vendor infomation
*  RETURNS:
*   
*
*  NOTE:
* 
*
***********************************************************************/
void ParseOltVendorInformation(char *VendorInfo, char *VendorInfo_end)
{
	char *p;
	char *p_line_head;
	char *p_line_tail;
	char temp[256+1];
	int len, temp_len;
	
	if(VendorInfo_end == NULL) 
		return;
	if((int)VendorInfo_end <=  (int)VendorInfo )
		return;
	
	p = VOS_StrStr(VendorInfo, "{");
	if(p == NULL ) 
		return;
	
	p++;
	p = SkipLineWhiteSpace(p);

	while((int)p < (int)VendorInfo_end)
		{
		/* olt venfor information */
		if(VOS_StrnCmp(p, KEY_VENDOR_INFO, VOS_StrLen(KEY_VENDOR_INFO)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_StrnCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_VENDOR_INFO, &temp[0]);
					}

				temp_len = 0;
				do{
					if(((temp_len+3) < len) && (*(int *)&temp[temp_len]== STRING_RETURN_AND_LINE))
						{
						temp[temp_len] = 0x20;
						temp[temp_len+1]=0x20;
						temp[temp_len+2] = 0xD;
						temp[temp_len+3] = 0xA;
						}
					temp_len++;
					}while( temp_len < (len -1));

				InitOltVendorInfoBySysfile(temp, len);
				
				}
			}

		/* olt vendor location */
		else if(VOS_StrnCmp(p, KEY_VENDOR_LOCATION, VOS_StrLen(KEY_VENDOR_LOCATION)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_StrnCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltVendorLocationBySysfile(&temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_VENDOR_LOCATION, &temp[0]);
					}
				}
			}

		/* olt vendor copyright */
		else if(VOS_StrnCmp(p, KEY_VENDOR_COPYRIGHT, VOS_StrLen(KEY_VENDOR_COPYRIGHT)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_StrnCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltVendorCopyrightBySysfile(&temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_VENDOR_COPYRIGHT, &temp[0]);
					}
				}
			}

		/* olt vendor name */
		else if(VOS_StrnCmp(p, KEY_VENDOR_NAME_SHORT, VOS_StrLen(KEY_VENDOR_NAME_SHORT)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_StrnCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltVendorShortNameBySysfile(&temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_VENDOR_NAME_SHORT, &temp[0]);
					}
				}
			}
		
		else if(VOS_StrnCmp(p, KEY_VENDOR_NAME_AB, VOS_StrLen(KEY_VENDOR_NAME_AB)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_StrnCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltVendorABNameBySysfile(&temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_VENDOR_NAME_AB, &temp[0]);
					}
				}
			}

		else if(VOS_StrnCmp(p, KEY_VENDOR_NAME, VOS_StrLen(KEY_VENDOR_NAME)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_StrnCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltVendorNameBySysfile(&temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_VENDOR_NAME, &temp[0]);
					}
				}
			}
		

		/* product series */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_SERIES, VOS_StrLen(KEY_PRODUCT_SERIES)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_StrnCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltProductSeriesBySysfile(&temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_SERIES, &temp[0]);
					}
				}
			}

		/* product software */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_SOFTWARE, VOS_StrLen(KEY_PRODUCT_SOFTWARE)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_StrnCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltProductSoftwareBySysfile(&temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_SOFTWARE, &temp[0]);
					}
				}
			}
		/* other else */
		else {
			p = SkipLine(p);
			}
		
		p = SkipLineWhiteSpace(p);
		}
	
	return;
}

/***********************************************************************
*
*  ParseOltProductInformation
*
*  PURPOSE:
*     parse OLT product infomation, and config the olt product info to database
*
*  PARAMETERS:
*     PorductInfo - [in] start ptr to OLT product infomation
*     ProductInfo_end -- [in] end ptr to OLT product infomation
*  RETURNS:
*   
*
*  NOTE:
* 
*
***********************************************************************/
void ParseOltProductInformation(char *PorductInfo, char *ProductInfo_end)
{
	char *p;
	char *p_line_head;
	char *p_line_tail;
	char temp[256+1];
	int len;
	
	if(ProductInfo_end == NULL) 
		return;
	if((int)ProductInfo_end <=  (int)PorductInfo )
		return;
	
	p = VOS_StrStr(PorductInfo, "{");
	if(p == NULL ) 
		return;
	
	p++;
	p = SkipLineWhiteSpace(p);

	while((int)p < (int)ProductInfo_end)
		{
		/* 6900 product type */
		if(VOS_StrnCmp(p, KEY_PRODUCT_TYPE_6900, VOS_StrLen(KEY_PRODUCT_TYPE_6900)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceTypeStringBySysfile(V2R1_OLT_GFA6900, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_TYPE_6900, &temp[0]);
					}
				}
			}

		/* 6900M product type */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_TYPE_6900M, VOS_StrLen(KEY_PRODUCT_TYPE_6900M)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceTypeStringBySysfile(V2R1_OLT_GFA6900M, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_TYPE_6900M, &temp[0]);
					}
				}
			}
		/* 6900S product type */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_TYPE_6900S, VOS_StrLen(KEY_PRODUCT_TYPE_6900S)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceTypeStringBySysfile(V2R1_OLT_GFA6900S, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_TYPE_6900S, &temp[0]);
					}
				}
			}

		/* 8000 product type */
		    else if(VOS_StrnCmp(p, KEY_PRODUCT_TYPE_8000, VOS_StrLen(KEY_PRODUCT_TYPE_8000)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceTypeStringBySysfile(V2R1_OLT_GFA8000, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_TYPE_8000, &temp[0]);
					}
				}
			}

		/* 8000M product type */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_TYPE_8000M, VOS_StrLen(KEY_PRODUCT_TYPE_8000M)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceTypeStringBySysfile(V2R1_OLT_GFA8000M, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_TYPE_8000M, &temp[0]);
					}
				}
			}
		/* 8000S product type */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_TYPE_8000S, VOS_StrLen(KEY_PRODUCT_TYPE_8000S)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceTypeStringBySysfile(V2R1_OLT_GFA8000S, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_TYPE_8000S, &temp[0]);
					}
				}
			}


		/* 6700 product type */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_TYPE_6700, VOS_StrLen(KEY_PRODUCT_TYPE_6700)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceTypeStringBySysfile(V2R1_OLT_GFA6700, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_TYPE_6700, &temp[0]);
					}
				}
			}

		/* 6100 product type */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_TYPE_6100, VOS_StrLen(KEY_PRODUCT_TYPE_6100)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceTypeStringBySysfile(V2R1_OLT_GFA6100, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_TYPE_6100, &temp[0]);
					}
				}
			}

		/* 8100 product type */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_TYPE_8100, VOS_StrLen(KEY_PRODUCT_TYPE_8100)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceTypeStringBySysfile(V2R1_OLT_GFA8100, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_TYPE_8100, &temp[0]);
					}
				}
			}

		/* 6900 product description */
		if(VOS_StrnCmp(p, KEY_PRODUCT_DESCRIPTION_6900, VOS_StrLen(KEY_PRODUCT_DESCRIPTION_6900)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceDescStringBySysfile(V2R1_OLT_GFA6900, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_DESCRIPTION_6900, &temp[0]);
					}
				}
			}
		/* 6900M product description */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_DESCRIPTION_6900M, VOS_StrLen(KEY_PRODUCT_DESCRIPTION_6900M)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceDescStringBySysfile(V2R1_OLT_GFA6900M, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_DESCRIPTION_6900M, &temp[0]);
					}
				}
			}
		/* 6900S product description */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_DESCRIPTION_6900S, VOS_StrLen(KEY_PRODUCT_DESCRIPTION_6900S)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceDescStringBySysfile(V2R1_OLT_GFA6900S, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_DESCRIPTION_6900S, &temp[0]);
					}
				}
			}
		else if(VOS_StrnCmp(p, KEY_PRODUCT_DESCRIPTION_8000, VOS_StrLen(KEY_PRODUCT_DESCRIPTION_8000)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceDescStringBySysfile(V2R1_OLT_GFA8000, &temp[0]);
				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_DESCRIPTION_8000, &temp[0]);
					}
				}
			}
		else if(VOS_StrnCmp(p, KEY_PRODUCT_DESCRIPTION_8000M, VOS_StrLen(KEY_PRODUCT_DESCRIPTION_8000M)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceDescStringBySysfile(V2R1_OLT_GFA8000M, &temp[0]);
				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_DESCRIPTION_8000M, &temp[0]);
					}
				}
			}
		else if(VOS_StrnCmp(p, KEY_PRODUCT_DESCRIPTION_8000S, VOS_StrLen(KEY_PRODUCT_DESCRIPTION_8000S)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceDescStringBySysfile(V2R1_OLT_GFA8000S, &temp[0]);
				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_DESCRIPTION_8000S, &temp[0]);
					}
				}
			}

		/* 6700 product description */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_DESCRIPTION_6700, VOS_StrLen(KEY_PRODUCT_DESCRIPTION_6700)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceDescStringBySysfile(V2R1_OLT_GFA6700, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_DESCRIPTION_6700, &temp[0]);
					}
				}
			}

		/* 6100 product description */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_DESCRIPTION_6100, VOS_StrLen(KEY_PRODUCT_DESCRIPTION_6100)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceDescStringBySysfile(V2R1_OLT_GFA6100, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_DESCRIPTION_6100, &temp[0]);
					}
				}
			}
		/* 8100 product description */
		else if(VOS_StrnCmp(p, KEY_PRODUCT_DESCRIPTION_8100, VOS_StrLen(KEY_PRODUCT_DESCRIPTION_8100)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOltDeviceDescStringBySysfile(V2R1_OLT_GFA8100, &temp[0]);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_PRODUCT_DESCRIPTION_8100, &temp[0]);
					}
				}
			}
		else
			{
			p = SkipLine(p);
			}
		
		p = SkipLineWhiteSpace(p);
		}
	
	return;
}

/***********************************************************************
*
*  ParseOltBoardName
*
*  PURPOSE:
*     parse OLT board name, and config the olt board name to database
*
*  PARAMETERS:
*     boardName - [in] start ptr to OLT board name
*     boardName_end -- [in] end ptr to OLT board name
*  RETURNS:
*   
*
*  NOTE:
* 
*
***********************************************************************/
int  BoardNameIsShortFormat( char *boardName)
{
	char *p = boardName;

	while( *p != '\0')
		p++;	
	p -= VOS_StrLen(KEY_BOARD_SHORT);
	
	if(VOS_StrnCmp(p, KEY_BOARD_SHORT, VOS_StrLen(KEY_BOARD_SHORT)) == 0)
		return V2R1_ENABLE;
	else 
		return V2R1_DISABLE;
}

void ParseOltBoardName(char *boardName, char *boardName_end)
{
	char *p;
	char *p_line_head;
	char *p_line_tail;
	char temp[256+1];
	char keyword[64];
	int len;
	int boardType;
	
	if(boardName_end == NULL) 
		return;
	if((int)boardName_end <=  (int)boardName )
		return;
	
	p = VOS_StrStr(boardName, "{");
	if(p == NULL ) 
		return;
	
	p++;
	p = SkipLineWhiteSpace(p);

	while((int)p < (int)boardName_end)
		{
		p_line_tail = VOS_StrStr(p, EVALUATE_DELIMITER);

		if(p_line_tail != NULL)
			{
			len = (int)p_line_tail - (int)p;
			VOS_StrnCpy(&keyword[0], p, len);
			while(isWhiteSpace(keyword[len-1]) && (len >1))
				len--;
			keyword[len] = '\0';
			
			boardType = OltBoardTypeKeywordToInteger(&keyword[0]);
			
			if(boardType  >  MODULE_TYPE_UNKNOW)
				{
				p = ParseStringLine(p, &p_line_head, &p_line_tail);

				if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
					{
					len = (int)p_line_tail-(int)p_line_head+1;
					if(len > 256)
						len = 256;
					
					VOS_MemCpy(&temp[0], p_line_head, len);
					temp[len] = '\0';

					if(BoardNameIsShortFormat(keyword) == V2R1_DISABLE)
						InitOltBoardTypeStringBySysfile(boardType, &temp[0]);
					else 
						InitOltBoardTypeStringShortBySysfile(boardType, &temp[0]);

					if(SYS_FILE_DEBUG == 1 )
						{
						sys_console_printf("%s=\"%s\"\r\n", &keyword[0], &temp[0]);
						}
					}
				}
			else 
				p = SkipLine(p);
			}
		else 
			p = SkipLine(p);
		
		p = SkipLineWhiteSpace(p);
		}
	
	return;
}

/***********************************************************************
*
*  ParseOnuBoardName
*
*  PURPOSE:
*     parse onu board name, and config the onu board name to database
*
*  PARAMETERS:
*     boardName - [in] start ptr to onu board name
*     boardName_end -- [in] end ptr to onu board name
*  RETURNS:
*   
*
*  NOTE:
* 
*
***********************************************************************/
void ParseOnuBoardName(char *boardName, char *boardName_end)
{
	char *p;
	char *p_line_head;
	char *p_line_tail;
	char temp[256+1];
	char keyword[64];
	int len;
	int boardType;
	
	if(boardName_end == NULL) 
		return;
	if((int)boardName_end <=  (int)boardName )
		return;
	
	p = VOS_StrStr(boardName, "{");
	if(p == NULL ) 
		return;
	
	p++;
	p = SkipLineWhiteSpace(p);

	while((int)p < (int)boardName_end)
		{
		p_line_tail = VOS_StrStr(p, EVALUATE_DELIMITER);

		if(p_line_tail != NULL)
			{
			len = (int)p_line_tail - (int)p;
			VOS_MemCpy(&keyword[0], p, len);
			while(isWhiteSpace(keyword[len-1]) && (len >1))
				len--;
			keyword[len] = '\0';
			
			boardType = OnuBoardTypeKeywordToInteger(&keyword[0]);

			if( boardType > SUB_BOARD_NULL)
				{
				p = ParseStringLine(p, &p_line_head, &p_line_tail);

				if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
					{
					len = (int)p_line_tail-(int)p_line_head+1;
					if(len > 256)
						len = 256;
					
					VOS_MemCpy(&temp[0], p_line_head, len);
					temp[len] = '\0';
					InitOnuBoardTypeStringBySysfile(boardType, &temp[0]);

					if(SYS_FILE_DEBUG == 1 )
						{
						sys_console_printf("%s=\"%s\"\r\n", &keyword[0], &temp[0]);
						}
					}
				}
			else 
				p = SkipLine(p);
			}
		else 
			p = SkipLine(p);
		
		p = SkipLineWhiteSpace(p);
		}
	
	return;
}

/***********************************************************************
*
*  ParseOnuVendorInformation
*
*  PURPOSE:
*     parse Onu vendor infomation, and config the Onu vendor info to database
*
*  PARAMETERS:
*     VendorInfo - [in] start ptr to Onu vendor infomation
*     VendorInfo_end - [in] end ptr to Onu vendor infomation
*  RETURNS:
*   
*
*  NOTE:
* 
*
***********************************************************************/
void ParseOnuVendorInformation(char *VendorInfo, char *VendorInfo_end)
{
	char *p;
	char *p_line_head;
	char *p_line_tail;
	char temp[256+1];
	int len;
	
	if(VendorInfo_end == NULL) 
		return;
	if((int)VendorInfo_end <=  (int)VendorInfo )
		return;
	
	p = VOS_StrStr(VendorInfo, "{");
	if(p == NULL ) 
		return;
	
	p++;
	p = SkipLineWhiteSpace(p);

	while((int)p < (int)VendorInfo_end)
		{
		if(VOS_StrnCmp(p, KEY_ONU_VENDOR, VOS_StrLen(KEY_ONU_VENDOR)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > 256)
					len = 256;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOnuVendorInfoBySysfile(temp);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_ONU_VENDOR, &temp[0]);
					}
				}
			}
		
		else 
			p = SkipLine(p);
		
		p = SkipLineWhiteSpace(p);
		}
	
	return;
}

/***********************************************************************
*
*  ParseOnetype
*
*  PURPOSE:
*     parse one Onu type infomation, and config the Onu type info to database
*
*  PARAMETERS:
*     typeInfo - [in] start ptr to Onu type infomation
*     typeInfo_end - [in] end ptr to Onu type infomation
*  RETURNS:
*   
*
*  NOTE:
* 
*
***********************************************************************/
void ParseOneType(char *typeInfo, char *typeInfo_end)
{
	char *p;
	char *p_line_head;
	char *p_line_tail;
	
	int  Type_integer=0;
	int len;

	char Type_String[ONU_TYPE_LEN+1]={0};
	char Type_Desc[MAXDEVICEDESCLEN+1]={0};
	int  slotNum=0;
	int  fePortNum=0;
	int  gePortNum=0;
	int  potsPortNum=0;
	int  voipPortNum=0;
	int  e1PortNum=0;
	int  ctcRegisterId = 0;

	char  eponApp[ONU_TYPE_LEN+1]={0};
	char  voiceApp[ONU_TYPE_LEN+1]={0};
	char  fpgaApp[ONU_TYPE_LEN+1]={0};

	if(typeInfo_end == NULL)
		return;
	if((int)typeInfo_end <= (int)typeInfo )
		return;

	p = VOS_StrStr(typeInfo, "{");
	if(p == NULL ) 
		return;
	
	p++;
	p = SkipLineWhiteSpace(p);

	while((int)p < (int)typeInfo_end)
		{
		/* onu type */
		if(VOS_StrnCmp(p, KEY_TYPE_INTEGER, VOS_StrLen(KEY_TYPE_INTEGER)) == 0 )
			{
			Type_integer = ParseDigitalLine(p);
			p = SkipLine(p);

			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("%s=%d\r\n",KEY_TYPE_INTEGER, Type_integer);
			}

		/* onu type-string */
		else if(VOS_StrnCmp(p, KEY_TYPE_STRING, VOS_StrLen(KEY_TYPE_STRING)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail - (int)p_line_head +1;
				if(len > ONU_TYPE_LEN)
					len = ONU_TYPE_LEN;

				VOS_MemCpy(&Type_String[0], p_line_head, len);
				Type_String[len] = '\0';

				if(SYS_FILE_DEBUG == 1 )
					sys_console_printf("%s=\"%s\"\r\n", KEY_TYPE_STRING, &Type_String[0]);
				}
			}

		/* onu description */
		else if(VOS_StrnCmp(p, KEY_DESC_STRING, VOS_StrLen(KEY_DESC_STRING)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail - (int)p_line_head +1;
				if(len > MAXDEVICEDESCLEN)
					len = MAXDEVICEDESCLEN;

				VOS_MemCpy(&Type_Desc[0], p_line_head, len);
				Type_Desc[len] = '\0';

				if(SYS_FILE_DEBUG == 1 )
					sys_console_printf("%s=\"%s\"\r\n", KEY_DESC_STRING, &Type_Desc[0]);
				}
			}

		/* onu slot num */
		else if(VOS_StrnCmp(p, KEY_MAX_SLOT, VOS_StrLen(KEY_MAX_SLOT)) == 0 )
			{
			slotNum = ParseDigitalLine(p);
			p = SkipLine(p);

			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("%s=%d\r\n", KEY_MAX_SLOT, slotNum);	
			}

		/* onu FE port num */
		else if(VOS_StrnCmp(p, KEY_FE_PORT_NUM, VOS_StrLen(KEY_FE_PORT_NUM)) == 0 )
			{
			fePortNum = ParseDigitalLine(p);
			p = SkipLine(p);

			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("%s=%d\r\n", KEY_FE_PORT_NUM, fePortNum);	
			}

		/* onu GE port num */
		else if(VOS_StrnCmp(p, KEY_GE_PORT_NUM, VOS_StrLen(KEY_GE_PORT_NUM)) == 0 )
			{
			gePortNum = ParseDigitalLine(p);
			p = SkipLine(p);

			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("%s=%d\r\n", KEY_GE_PORT_NUM, gePortNum);	
			}

		/* onu pots port num */
		else if(VOS_StrnCmp(p, KEY_POTS_PORT_NUM, VOS_StrLen(KEY_POTS_PORT_NUM)) == 0 )
			{
			potsPortNum = ParseDigitalLine(p);
			p = SkipLine(p);

			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("%s=%d\r\n", KEY_POTS_PORT_NUM, potsPortNum);	
			}

		/* onu voip port num */
		else if(VOS_StrnCmp(p, KEY_VOIP_PORT_NUM, VOS_StrLen(KEY_VOIP_PORT_NUM)) == 0 )
			{
			voipPortNum = ParseDigitalLine(p);
			p = SkipLine(p);

			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("%s=%d\r\n", KEY_VOIP_PORT_NUM, voipPortNum);	
			}

		/* onu e1 port num */
		else if(VOS_StrnCmp(p, KEY_E1_PORT_NUM, VOS_StrLen(KEY_E1_PORT_NUM)) == 0 )
			{
			e1PortNum = ParseDigitalLine(p);
			p = SkipLine(p);

			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("%s=%d\r\n", KEY_E1_PORT_NUM, e1PortNum);	
			}

		/* onu ctc register id */
		else if(VOS_StrnCmp(p, KEY_CTC_REGISTER_ID, VOS_StrLen(KEY_CTC_REGISTER_ID)) == 0 )
			{
			ctcRegisterId = ParseDigitalLine(p);
			p = SkipLine(p);

			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("%s=0x%X\r\n", KEY_CTC_REGISTER_ID, ctcRegisterId);	
			}

		/* onu epon app file  */
		else if(VOS_StrnCmp(p, KEY_APP_EPON_ID, VOS_StrLen(KEY_APP_EPON_ID)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);
	
			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail - (int)p_line_head +1;
				if(len > ONU_TYPE_LEN)
					len = ONU_TYPE_LEN;

				VOS_MemCpy(&eponApp[0], p_line_head, len);
				eponApp[len] = '\0';

				if(SYS_FILE_DEBUG == 1 )
					sys_console_printf("%s=\"%s\"\r\n", KEY_APP_EPON_ID, &eponApp[0]);
				}
			}

		/* onu voice app file  */
		else if(VOS_StrnCmp(p, KEY_APP_VOICE_ID, VOS_StrLen(KEY_APP_VOICE_ID)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);
	
			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail - (int)p_line_head +1;
				if(len >  ONU_TYPE_LEN)
					len = ONU_TYPE_LEN;

				VOS_MemCpy(&voiceApp[0], p_line_head, len);
				voiceApp[len] = '\0';

				if(SYS_FILE_DEBUG == 1 )
					sys_console_printf("%s=\"%s\"\r\n", KEY_APP_VOICE_ID, &voiceApp[0]);
				}
			}

		/* onu fpga app file  */
		else if(VOS_StrnCmp(p, KEY_APP_FPGA_ID, VOS_StrLen(KEY_APP_FPGA_ID)) == 0 )
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);
	
			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail - (int)p_line_head +1;
				if(len > ONU_TYPE_LEN)
					len = ONU_TYPE_LEN;

				VOS_MemCpy(&fpgaApp[0], p_line_head, len);
				fpgaApp[len] = '\0';

				if(SYS_FILE_DEBUG == 1 )
					sys_console_printf("%s=\"%s\"\r\n", KEY_APP_FPGA_ID, &fpgaApp[0]);
				}
			}

		else 
			p = SkipLine(p);

		p = SkipLineWhiteSpace(p);
		}

	InitOnuTypeInfoBySysfile(Type_integer, &Type_String[0], &Type_Desc[0]);
	InitOnuUniPortBySysfile(Type_integer, slotNum, fePortNum, gePortNum, potsPortNum, voipPortNum, e1PortNum);
	InitOnuAppStringBySysfile(Type_integer, &eponApp[0], &voiceApp[0], &fpgaApp[0]);
	InitOnuCtcRegisterIdBySysfile(Type_integer,  ctcRegisterId );
	
	return;
}

/***********************************************************************
*
*  ParseOnutype
*
*  PURPOSE:
*     parse Onu type infomation, and config the Onu type info to database
*
*  PARAMETERS:
*     typeInfo - [in] start ptr to Onu type infomation
*     typeInfo_end - [in] end ptr to Onu type infomation
*  RETURNS:
*   
*
*  NOTE:
* 
*
***********************************************************************/
static char *JudgeOnuType(char *p)
{
	char *temp_ptr;
	
	if(p == NULL)
		return(NULL);

	if(VOS_StrnCmp(p, KEY_TYPE, VOS_StrLen(KEY_TYPE)) == 0)
		{
		temp_ptr =  p + VOS_StrLen(KEY_TYPE);
		if(vos_isdigit(*temp_ptr) ||vos_isdigit(*(temp_ptr+1)))
			{
			temp_ptr = VOS_StrStr(temp_ptr, EVALUATE_DELIMITER);
			if(temp_ptr != NULL)
				return(temp_ptr);
			}
		}
	return NULL;	
}

static char *judgeOneElement(char *p, const char *pe)
{
    char *temp_ptr;
    int len = VOS_StrLen(pe);

    if(p == NULL)
        return(NULL);

    if(VOS_StrnCmp(p, pe, len) == 0)
        {
        temp_ptr =  p + len;
        if(vos_isdigit(*temp_ptr) ||vos_isdigit(*(temp_ptr+1)))
            {
            temp_ptr = VOS_StrStr(temp_ptr, EVALUATE_DELIMITER);
            if(temp_ptr != NULL)
                return(temp_ptr);
            }
        }
    return NULL;
}

void ParseOneModel(char *typeInfo, char *typeInfo_end)
{
    char *p;
    char *p_line_head;
    char *p_line_tail;

    int  model_id=0;
    int  typeid = V2R1_ONU_CTC;
    int len;

    char model_str[80]={0};
    int  slotNum=0;
    int  fePortNum=0;
    int  gePortNum=0;
    int  potsPortNum=0;
    int  voipPortNum=0;
    int  e1PortNum=0;
    int  ctcRegisterId = 0;

    char  eponApp[80]={0};

    if(typeInfo_end == NULL)
        return;
    if((int)typeInfo_end <= (int)typeInfo )
        return;

    p = VOS_StrStr(typeInfo, "{");
    if(p == NULL )
        return;

    p++;
    p = SkipLineWhiteSpace(p);

    while((int)p < (int)typeInfo_end)
        {
    	/*type integer*/
    	if(VOS_StrnCmp(p, KEY_TYPE_INTEGER, VOS_StrLen(KEY_TYPE_INTEGER)) == 0)
    	{
    		typeid = ParseDigitalLine(p);
    		p = SkipLine(p);

    		if(SYS_FILE_DEBUG == 1)
    			sys_console_printf("%s=%d\r\n", KEY_TYPE_INTEGER, typeid);
    	}
        /* onu type */
    	else if(VOS_StrnCmp(p, KEY_MODEL_ID, VOS_StrLen(KEY_MODEL_ID)) == 0 )
            {
            model_id = ParseDigitalLine(p);
            p = SkipLine(p);

            if(SYS_FILE_DEBUG == 1 )
                sys_console_printf("%s=%d\r\n",KEY_MODEL_ID, model_id);
            }

        /* onu type-string */
        else if(VOS_StrnCmp(p, KEY_MODE_STR, VOS_StrLen(KEY_MODE_STR)) == 0 )
            {
            p = ParseStringLine(p, &p_line_head, &p_line_tail);

            if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
                {
                len = (int)p_line_tail - (int)p_line_head +1;
                if(len > ONU_TYPE_LEN)
                    len = ONU_TYPE_LEN;

                VOS_MemCpy(model_str, p_line_head, len);
                model_str[len] = '\0';

                if(SYS_FILE_DEBUG == 1 )
                    sys_console_printf("%s=\"%s\"\r\n", KEY_MODE_STR, model_str);
                }
            }

        /* onu epon app file  */
        else if(VOS_StrnCmp(p, KEY_APP_EPON_ID, VOS_StrLen(KEY_APP_EPON_ID)) == 0 )
            {
            p = ParseStringLine(p, &p_line_head, &p_line_tail);

            if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
                {
                len = (int)p_line_tail - (int)p_line_head +1;
                if(len > ONU_TYPE_LEN)
                    len = ONU_TYPE_LEN;

                VOS_MemCpy(&eponApp[0], p_line_head, len);
                eponApp[len] = '\0';

                if(SYS_FILE_DEBUG == 1 )
                    sys_console_printf("%s=\"%s\"\r\n", KEY_APP_EPON_ID, &eponApp[0]);
                }
            }

        else
            p = SkipLine(p);

        p = SkipLineWhiteSpace(p);
        }

    CTC_addOnuModel(model_id, typeid, model_str, eponApp);

    return;
}

void ParseOneEquipment(char *typeInfo, char *typeInfo_end)
{
    char *p;
    char *p_line_head;
    char *p_line_tail;

    int  model_id=0;
    int  typeid = V2R1_ONU_GPON;
    int len;

    char equipment[80]={0};
    char model_str[80]={0};
    int  slotNum=0;
    int  fePortNum=0;
    int  gePortNum=0;
    int  potsPortNum=0;
    int  voipPortNum=0;
    int  e1PortNum=0;
    int  ctcRegisterId = 0;

    char  eponApp[80]={0};

    if(typeInfo_end == NULL)
        return;
    if((int)typeInfo_end <= (int)typeInfo )
        return;

    p = VOS_StrStr(typeInfo, "{");
    if(p == NULL )
        return;

    p++;
    p = SkipLineWhiteSpace(p);

    while((int)p < (int)typeInfo_end)
        {
    	/*type integer*/
    	if(VOS_StrnCmp(p, KEY_TYPE_INTEGER, VOS_StrLen(KEY_TYPE_INTEGER)) == 0)
    	{
    		typeid = ParseDigitalLine(p);
    		p = SkipLine(p);

    		if(SYS_FILE_DEBUG == 1)
    			sys_console_printf("%s=%d\r\n", KEY_TYPE_INTEGER, typeid);
    	}
        /* onu type */
    	else if(VOS_StrnCmp(p, KEY_EQUIPMENT_ID, VOS_StrLen(KEY_EQUIPMENT_ID)) == 0 )
        {
            p = ParseStringLine(p, &p_line_head, &p_line_tail);
            
            if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
            {
                len = (int)p_line_tail - (int)p_line_head +1;
                if(len > 41)
                    len = 41;
            
                VOS_MemCpy(equipment, p_line_head, len);
                equipment[len] = '\0';
            
                if(SYS_FILE_DEBUG == 1 )
                    sys_console_printf("%s=\"%s\"\r\n", KEY_MODE_STR, equipment);
            }
            
        }

        /* onu type-string */
        else if(VOS_StrnCmp(p, KEY_EQUIPMETMODE_STR, VOS_StrLen(KEY_EQUIPMETMODE_STR)) == 0 )
            {
            p = ParseStringLine(p, &p_line_head, &p_line_tail);

            if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
                {
                len = (int)p_line_tail - (int)p_line_head +1;
                if(len > ONU_TYPE_LEN)
                    len = ONU_TYPE_LEN;

                VOS_MemCpy(model_str, p_line_head, len);
                model_str[len] = '\0';

                if(SYS_FILE_DEBUG == 1 )
                    sys_console_printf("%s=\"%s\"\r\n", KEY_MODE_STR, model_str);
                }
            }

        /* onu epon app file  */
        else if(VOS_StrnCmp(p, KEY_APP_EPON_ID, VOS_StrLen(KEY_APP_EPON_ID)) == 0 )
            {
            p = ParseStringLine(p, &p_line_head, &p_line_tail);

            if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
                {
                len = (int)p_line_tail - (int)p_line_head +1;
                if(len > ONU_TYPE_LEN)
                    len = ONU_TYPE_LEN;

                VOS_MemCpy(&eponApp[0], p_line_head, len);
                eponApp[len] = '\0';

                if(SYS_FILE_DEBUG == 1 )
                    sys_console_printf("%s=\"%s\"\r\n", KEY_APP_EPON_ID, &eponApp[0]);
                }
            }

        else
            p = SkipLine(p);

        p = SkipLineWhiteSpace(p);
        }

    GPON_addOnuEquipment(equipment, typeid, model_str, eponApp);

    return;
}

void ParseOnutype(char *typeInfo, char *typeInfo_end)
{
	char *p;
	char *temp_p;
	int Total_type=0xaa55;
	int Parse_Num=0;

	char *p_line_head;
	char *p_line_tail;
	char temp[ONU_TYPE_LEN+1];
	int len;

	
	if(typeInfo_end == NULL) 
		return;
	if((int)typeInfo_end <=  (int)typeInfo )
		return;
	
	p = VOS_StrStr(typeInfo, "{");
	if(p == NULL ) 
		return;
	
	p++;
	p = SkipLineWhiteSpace(p);


	while(((int)p < (int)typeInfo_end) && ( Total_type > Parse_Num))
		{
		/* 解析onu 程序名前缀*/
		if(VOS_StrnCmp(p, KEY_ONU_APP_STRING_PREFIX, VOS_StrLen(KEY_ONU_APP_STRING_PREFIX)) == 0)
			{
			p = ParseStringLine(p, &p_line_head, &p_line_tail);

			if((p_line_head != NULL) && ( p_line_tail != NULL) && ((int)p_line_tail >= (int)p_line_head))
				{
				len = (int)p_line_tail-(int)p_line_head+1;
				if(len > ONU_TYPE_LEN)
					len = ONU_TYPE_LEN;
				
				VOS_MemCpy(&temp[0], p_line_head, len);
				temp[len] = '\0';
				InitOnuAppPrefixBySysfile(&temp[0], len);

				if(SYS_FILE_DEBUG == 1 )
					{
					sys_console_printf("%s=\"%s\"\r\n", KEY_ONU_APP_STRING_PREFIX, &temp[0]);
					}
				}
			}

		/* 解析TOTAL_TYPE=30 */
		else if(VOS_StrnCmp(p, KEY_TOTAL_TYPE, VOS_StrLen(KEY_TOTAL_TYPE)) == 0)
			{		
			Total_type = ParseDigitalLine(p);
			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("%s = %d\r\n", KEY_TOTAL_TYPE, Total_type);

			if(Total_type == 0 )
				return;
			
			p = SkipLine(p);
			}	

		/*  解析TYPEx={......} */
		else if((temp_p = JudgeOnuType(p)) != NULL)
			{
			Parse_Num++;
			
			if(SYS_FILE_DEBUG == 1 )
				{
				sys_console_printf("\r\n");
				sys_console_printf("type%d={\r\n",Parse_Num);
				}
			
			temp_p++;
			p = SkipLineWhiteSpace(temp_p);
			temp_p = FindNextBlock(p,0);
			ParseOneType(p, temp_p);
			p = temp_p;
			
			if(SYS_FILE_DEBUG == 1 )
				sys_console_printf("}\r\n");
			}
		else 
			p = SkipLine(p);

		p = SkipLineWhiteSpace(p);
		}

	return;
}

void ParseOnuModel(char *typeInfo, char *typeInfo_end)
{
    char *p;
    char *temp_p;
    int Total_model=0xaa55;
    int Parse_Num=0;

    char *p_line_head;
    char *p_line_tail;
    char temp[ONU_TYPE_LEN+1];
    int len;


    if(typeInfo_end == NULL)
        return;
    if((int)typeInfo_end <=  (int)typeInfo )
        return;

    p = VOS_StrStr(typeInfo, "{");
    if(p == NULL )
        return;

    p++;
    p = SkipLineWhiteSpace(p);
    /*added by luh 2013-1-31。在升级sysfile时需要把原来的sysfile产生的数据清空*/
    CTC_ClearOnuModel();
    while(((int)p < (int)typeInfo_end) && ( Total_model > Parse_Num))
        {

        /* 解析TOTAL_MODEL=30 */
        if(VOS_StrnCmp(p, KEY_TOTAL_MODEL, VOS_StrLen(KEY_TOTAL_MODEL)) == 0)
            {
            Total_model = ParseDigitalLine(p);
            if(SYS_FILE_DEBUG == 1 )
                sys_console_printf("%s = %d\r\n", KEY_TOTAL_MODEL, Total_model);

            if(Total_model == 0 )
                return;

            p = SkipLine(p);
            }

        /*  解析Modelx={......} */
        else if((temp_p = judgeOneElement(p, KEY_MODEL)) != NULL)
            {
            Parse_Num++;

            if(SYS_FILE_DEBUG == 1 )
                {
                sys_console_printf("\r\n");
                sys_console_printf("type%d={\r\n",Parse_Num);
                }

            temp_p++;
            p = SkipLineWhiteSpace(temp_p);
            temp_p = FindNextBlock(p,0);
            ParseOneModel(p, temp_p);
            p = temp_p;

            if(SYS_FILE_DEBUG == 1 )
                sys_console_printf("}\r\n");
            }
        else
            p = SkipLine(p);

        p = SkipLineWhiteSpace(p);
        }

    return;
}

void ParseGponOnuEquipment(char *typeInfo, char *typeInfo_end)
{
    char *p;
    char *temp_p;
    int Total_model=0xaa55;
    int Parse_Num=0;

    char *p_line_head;
    char *p_line_tail;
    char temp[ONU_TYPE_LEN+1];
    int len;


    if(typeInfo_end == NULL)
        return;
    if((int)typeInfo_end <=  (int)typeInfo )
        return;

    p = VOS_StrStr(typeInfo, "{");
    if(p == NULL )
        return;

    p++;
    p = SkipLineWhiteSpace(p);
    /*added by luh 2013-1-31。在升级sysfile时需要把原来的sysfile产生的数据清空*/
    /*CTC_ClearOnuModel();*/
    while(((int)p < (int)typeInfo_end) && ( Total_model > Parse_Num))
        {

        /* 解析TOTAL_MODEL=30 */
        if(VOS_StrnCmp(p, KEY_TOTAL_MODEL, VOS_StrLen(KEY_TOTAL_MODEL)) == 0)
            {
            Total_model = ParseDigitalLine(p);
            if(SYS_FILE_DEBUG == 1 )
                sys_console_printf("%s = %d\r\n", KEY_TOTAL_MODEL, Total_model);

            if(Total_model == 0 )
                return;

            p = SkipLine(p);
            }

        /*  解析Modelx={......} */
        else if((temp_p = judgeOneElement(p, KEY_MODEL)) != NULL)
            {
            Parse_Num++;

            if(SYS_FILE_DEBUG == 1 )
                {
                sys_console_printf("\r\n");
                sys_console_printf("type%d={\r\n",Parse_Num);
                }

            temp_p++;
            p = SkipLineWhiteSpace(temp_p);
            temp_p = FindNextBlock(p,0);
            ParseOneEquipment(p, temp_p);
            p = temp_p;

            if(SYS_FILE_DEBUG == 1 )
                sys_console_printf("}\r\n");
            }
        else
            p = SkipLine(p);

        p = SkipLineWhiteSpace(p);
        }

    return;
}

/***********************************************************************
*
*  ParseSysfile
*
*  PURPOSE:
*     parse sysfile.ini, and config the mgmt info to database
*
*  PARAMETERS:
*     filedata - [in] ptr to a string, include mgmt info. and is uncompressed, the head-struct is excluded.
*     len - [in] length of the filedata
*
*  RETURNS:
*   
*
*  NOTE:
* 
*
***********************************************************************/
void ParseSysfile(char *filedata, int len)
{
	char *p;
	char *p_block_start;
	char *p_block_end;
	
	
	if((filedata == NULL ) || (len == 0))
		return;

	p = SkipLineWhiteSpace(filedata);

	while((len +(int)filedata) > (int)p)
		{
		/* 1 OLT venfor information */
		if(VOS_StrnCmp(p,KEY_OLT_VENDOR_INFORMATION, VOS_StrLen(KEY_OLT_VENDOR_INFORMATION)) == 0)
			{
			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("%s={\r\n", KEY_OLT_VENDOR_INFORMATION);

			/* 查找赋值符*/
			p_block_start = VOS_StrStr(p, EVALUATE_DELIMITER);
			if(p_block_start != NULL )
				{
				p = ++p_block_start;
				p = SkipLineWhiteSpace(p);
				p_block_end = FindNextBlock(p,0);
				ParseOltVendorInformation(p, p_block_end);
				p = p_block_end;
				}
			else 
				p = SkipLine(p);

			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("}\r\n\r\n");
			}
		
		/* 2 OLT product info */
		else if(VOS_StrnCmp(p,KEY_OLT_PRODUCT_INFOMATION, VOS_StrLen(KEY_OLT_PRODUCT_INFOMATION)) == 0)
			{
			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("%s={\r\n", KEY_OLT_PRODUCT_INFOMATION);
			
			/* 查找赋值符*/
			p_block_start = VOS_StrStr(p, EVALUATE_DELIMITER);
			if(p_block_start != NULL )
				{
				p = ++p_block_start;
				p = SkipLineWhiteSpace(p);
				p_block_end = FindNextBlock(p,0);
				ParseOltProductInformation(p, p_block_end);
				p = p_block_end;
				}
			else 
				p = SkipLine(p);

			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("}\r\n\r\n");
			}

		/* 3 OLT board info */
		else if(VOS_StrnCmp(p,KEY_OLT_BOARD, VOS_StrLen(KEY_OLT_BOARD)) == 0)
			{
			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("%s={\r\n", KEY_OLT_BOARD);

			/* 查找赋值符*/
			p_block_start = VOS_StrStr(p, EVALUATE_DELIMITER);
			if(p_block_start != NULL )
				{
				p = ++p_block_start;
				p = SkipLineWhiteSpace(p);
				p_block_end = FindNextBlock(p,0);
				ParseOltBoardName(p, p_block_end);
				p = p_block_end;
				}
			else 
				p = SkipLine(p);

			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("}\r\n\r\n");
			}

		/* 4 onu vendor info */
		else if(VOS_StrnCmp(p,KEY_ONU_VENDOR_INFO, VOS_StrLen(KEY_ONU_VENDOR_INFO)) == 0)
			{
			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("%s={\r\n", KEY_ONU_VENDOR_INFO);

			/* 查找赋值符*/
			p_block_start = VOS_StrStr(p, EVALUATE_DELIMITER);
			if(p_block_start != NULL )
				{
				p = ++p_block_start;
				p = SkipLineWhiteSpace(p);
				p_block_end = FindNextBlock(p,0);
				ParseOnuVendorInformation(p, p_block_end);
				p = p_block_end;
				}
			else 
				p = SkipLine(p);

			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("}\r\n\r\n");
			}
		
		/* 5 onu board info */
		else if(VOS_StrnCmp(p,KEY_ONU_BOARD, VOS_StrLen(KEY_ONU_BOARD)) == 0)
			{
			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("%s={\r\n", KEY_ONU_BOARD);
			
			/* 查找赋值符*/
			p_block_start = VOS_StrStr(p, EVALUATE_DELIMITER);
			if(p_block_start != NULL )
				{
				p = ++p_block_start;
				p = SkipLineWhiteSpace(p);
				p_block_end = FindNextBlock(p,0);
				ParseOnuBoardName(p, p_block_end);
				p = p_block_end;
				}
			else 
				p = SkipLine(p);

			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("}\r\n\r\n");
			}

		/* 6 onu type info */
		else if(VOS_StrnCmp(p,KEY_ONU_TYPE, VOS_StrLen(KEY_ONU_TYPE)) == 0)
			{
			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("%s={\r\n", KEY_ONU_TYPE);
			
			/* 查找赋值符*/
			p_block_start = VOS_StrStr(p, EVALUATE_DELIMITER);
			if(p_block_start != NULL )
				{
				p = ++p_block_start;
				p = SkipLineWhiteSpace(p);
				p_block_end = FindNextBlock(p,0);
				ParseOnutype(p, p_block_end);
				p = p_block_end;
				}
			else 
				p = SkipLine(p);

			if(SYS_FILE_DEBUG ==1 )
				sys_console_printf("}\r\n\r\n");
			}
		
		/*7 ctc onu model*/
		else if(VOS_StrnCmp(p, KEY_ONU_MODEL, VOS_StrLen(KEY_ONU_MODEL)) == 0)
		{
		    if(SYS_FILE_DEBUG ==1 )
		        sys_console_printf("%s={\r\n", KEY_ONU_TYPE);

		    p_block_start = VOS_StrStr(p, EVALUATE_DELIMITER);
		    if(p_block_start != NULL)
		    {
		        p = ++p_block_start;
		        p = SkipLineWhiteSpace(p);
		        p_block_end = FindNextBlock(p, 0);

		        ParseOnuModel(p, p_block_end);

		        p = p_block_end;
		    }
		    else
		        p = SkipLine(p);

		    if(SYS_FILE_DEBUG ==1 )
		                    sys_console_printf("}\r\n\r\n");
		}
        else if(VOS_StrnCmp(p, KET_GONU_EQUIPMENT, VOS_StrLen(KET_GONU_EQUIPMENT)) == 0)
        {
		    if(SYS_FILE_DEBUG ==1 )
		        sys_console_printf("%s={\r\n", KEY_ONU_TYPE);

		    p_block_start = VOS_StrStr(p, EVALUATE_DELIMITER);
		    if(p_block_start != NULL)
		    {
		        p = ++p_block_start;
		        p = SkipLineWhiteSpace(p);
		        p_block_end = FindNextBlock(p, 0);

		        ParseGponOnuEquipment(p, p_block_end);

		        p = p_block_end;
		    }
		    else
		        p = SkipLine(p);

		    if(SYS_FILE_DEBUG ==1 )
		                    sys_console_printf("}\r\n\r\n");
		}
		else{
			p = SkipLine(p);
			}

		if(SYS_FILE_DEBUG ==1 )
			sys_console_printf("current pointer 1= 0x%x\r\n",(int)p);
		p = SkipLineWhiteSpace(p);
		if(SYS_FILE_DEBUG ==1 )
			sys_console_printf("current pointer 2= 0x%x\r\n",(int)p);
		}

	if(SYS_FILE_DEBUG ==1 )
		sys_console_printf(" parse sysFile.ini ok\r\n");

	return;
}
/* added by xieshl 20100207 */
int v2r1_printf( struct vty *vty, const char * format, ... )
{
	int rc;
	va_list args;
	int len;
	char buf[ SYS_SCREEN_WIDTH + 1 ];

	va_start( args, format );
	memset( buf, 0, ( SYS_SCREEN_WIDTH + 1 ) );
	len = linux_vsnprintf( buf, SYS_SCREEN_WIDTH, format, args );
	if ( ( len < 0 ) || ( len > SYS_SCREEN_WIDTH ) )
	{
		buf[SYS_SCREEN_WIDTH] = 0;
	}
	va_end( args );

	/*if( vty && SYS_LOCAL_MODULE_WORKMODE_ISMASTER )
	{
		rc = vty_out( vty, buf );
		vty_event( VTY_WRITE, vty->fd, vty );
	}
	else
		rc = sys_console_printf( buf );*/
	rc = cl_vty_all_out( "%s", buf );

	return rc;
}

#ifdef __cplusplus

}

#endif


