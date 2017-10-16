/*  
**  OAM_MODULE.c
**
**  
**  This software is licensed for use according to the terms set by the Passave API license agreement.
**  Copyright Passave Ltd. 
**  Ackerstein Towers - A, 9 Hamenofim St.
**  POB 2089, Herziliya Pituach 46120 Israel
**
**
**  This file was written by Meital Levy, meital.levy@passave.com, 14/03/2006
**  
**  Changes:
**
**  Version	  |  Date	   |    Change	             |    Author	  
**  ----------+------------+-------------------------+------------
**	1.00	  | 14/03/2006 |	Creation		     | Meital Levy
**	2.00	  | 28/07/2014 |	Modified		     | liwei056
*/


#define __MODULE__ "OAM_MODULE" /* File name for log headers */

#define ___FILE___ "" /* File name for log headers */


#include "oam/efm/efm.h"
#include "PonOam.h"
#include "OnuGeneral.h"
#include "ponEventHandler.h"
#include "onu/Onu_manage.h"

/*============================== Data Types =================================*/                             


#define MAX_OAM_FRAME_PRINTING_SIZE (MAX_ETHERNET_FRAME_SIZE_STANDARDIZED*4)


typedef struct  
{
    unsigned long                        oui;
    VENDOR_OAM_frame_received_handler_t  handler_function;
    PON_oam_code_t                       oam_code;
    bool                                 occupied;
}oam_database_entry_t;

/*========================== External Variables =============================*/

/*========================== External Functions =============================*/
/*============================== Variables ==================================*/
/*=========================== Internal variables =============================*/

static bool            internal_oam_module_is_init = FALSE;

static oam_database_entry_t   oam_internal_database[OAM_MODULE_DB_MAX_SIZE];

static unsigned short registration_handler_id;
static unsigned short deregistration_handler_id;
static unsigned short ext_registration_handler_id;


/*============================== Constants ==================================*/


#define OAM_FRAME_OUI_BEGINNING_PLACE       (ETHERNET_FRAME_CODE_END_PLACE+1)
#define OAM_FRAME_OUI_END_PLACE             (OAM_FRAME_OUI_BEGINNING_PLACE+OAM_OUI_SIZE)

#define OAM_SN_SIZE                         2 /* sequence_number size measured in Bytes */
#define OAM_FRAME_SN_BEGINNING_PLACE        (ETHERNET_FRAME_CODE_END_PLACE+1)
#define OAM_FRAME_SN_END_PLACE              (OAM_FRAME_SN_BEGINNING_PLACE+OAM_SN_SIZE)

/*  Ethernet layer */
static mac_address_t oam_destination_address = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x02 };
static unsigned short const_oam_ethernet_type = 0x8809;


/*  OAM layer */
static unsigned char const_oam_sub_type  =  0x03 ;
static unsigned short const_oam_flags    =  0x50 ;

/*================================ Macros ===================================*/

/*======================== Internal Functions prototype =====================*/

static short int Convert_oui_type_to_number( 
                    const OAM_oui_t   oui,
                    unsigned long    *oui_as_number );


static short int Insert_to_db
             ( const PON_oam_code_t            	     oam_code,
               const OAM_oui_t  					 oui,
               VENDOR_OAM_frame_received_handler_t   handler_func );

static short int Get_handler_from_db
             ( const short int                       olt_id,
               const PON_llid_t                      llid,
               const PON_oam_code_t            	     oam_code,
               const OAM_oui_t  					 oui,
               VENDOR_OAM_frame_received_handler_t  *handler_func );

static int oam_pon_event_handler(PON_event_handler_index_t event_id, void *event_data);


/*============================== Data Types =================================*/
/*================================ Functions ================================*/


#define PONLOG_FUNCTION_NAME  "OAM_MODULE_init"
PON_STATUS  PON_OAM_MODULE_init(void)
{

    short int result = 0;


    /* if it already initialize, ignore  */
    if(internal_oam_module_is_init)
    {
        return OAM_EXIT_OK;
    }


    result = Pon_event_assign_handler_function(PON_EVT_HANDLER_LLID_END_STD_OAM_DISCOVERY, (void*)oam_pon_event_handler, &registration_handler_id);

    if (result != EXIT_OK) 
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: Pon_event_assign_handler_function failed with code %d. \n", result); 
	}

    result = Pon_event_assign_handler_function(PON_EVT_HANDLER_LLID_DEREGISTER_EVENT,(void*)oam_pon_event_handler, &deregistration_handler_id);

    if (result != EXIT_OK) 
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: Pon_event_assign_handler_function failed with code %d. \n", result); 
	}

    result = Pon_event_assign_handler_function(PON_EVT_HANDLER_LLID_END_EXT_OAM_DISCOVERY, (void*)oam_pon_event_handler, &ext_registration_handler_id);

    if (result != EXIT_OK) 
    {
        PONLOG_ERROR_1(PONLOG_FUNC_REMOTE_MGMT_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: Pon_event_assign_handler_function failed with code %d. \n", result); 
	}
    

    internal_oam_module_is_init = TRUE; 

    return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "OAM_MODULE_terminate"
PON_STATUS PON_OAM_MODULE_terminate(void)
{
    short int result = 0;
 
    internal_oam_module_is_init = FALSE; 

    return result;

}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME  "OAM_MODULE_assign_handler"

PON_STATUS PON_OAM_MODULE_assign_handler( 
                    const PON_oam_code_t            	  oam_code,
                    const OAM_oui_t  					  oui,
                    VENDOR_OAM_frame_received_handler_t   handler_func )
{

    unsigned char  entry;
    unsigned long       oui_as_number;

    short int result=OAM_EXIT_OK;

    if(handler_func != NULL)
    {
    
        result = Insert_to_db(oam_code, oui, handler_func);

        if(result != OAM_EXIT_OK)
        {
            PONLOG_ERROR_4( PONLOG_FUNC_OAM_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: can not registrate oui:0x%X oam_code:0x%x. all entries occupied\n",
                            oui[0],oui[1],oui[2], oam_code);

            return result;
        }
    }
    else
    {
        for(entry = 0; entry < OAM_MODULE_DB_MAX_SIZE; entry++)
        {
            if(oam_internal_database[entry].occupied)
            {
                if (oam_internal_database[entry].oam_code == oam_code)
                {
                    switch(oam_code)
                    {
                    case PON_OAM_CODE_OAM_IMFORMATION:
                    case PON_OAM_CODE_EVENT_NOTIFICATION:

                        oam_internal_database[entry].occupied = FALSE;

                        oam_internal_database[entry].handler_function = NULL;

                        return OAM_EXIT_OK;
                        break;
                    case PON_OAM_CODE_VENDOR_EXTENSION:
                

                        result = Convert_oui_type_to_number(oui, &oui_as_number);

                        if( result != OAM_EXIT_OK )
                        {
                            PONLOG_ERROR_3(PONLOG_FUNC_OAM_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: can not convert OUI: 0x%x%x%x \n. discarding frame", oui[0],oui[1],oui[2]);
        
                        }

                        if( oam_internal_database[entry].oui == oui_as_number) 
                        {
                            oam_internal_database[entry].handler_function = NULL;
                            oam_internal_database[entry].occupied = FALSE;

                            return OAM_EXIT_OK;
                        }


                        default:
                            PONLOG_ERROR_1(PONLOG_FUNC_OAM_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: Unsupported OAM code: 0x%x\n. discarding request", oam_code);
                    }
                }
            }
        }
    }

    

    
    
    return result;
        
    
}
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME  "OAM_MODULE_send_frame"

PON_STATUS PON_OAM_MODULE_send_frame(      
                  const PON_olt_id_t             olt_id,
                  const PON_llid_t               llid,
                  const PON_oam_code_t           oam_code,
                  const OAM_oui_t				 oui,
                  const unsigned short           length,
                  const unsigned char           *content )
{
    short int       result;
#if 0    
	char		    hex_str[MAX_ETHERNET_FRAME_SIZE_STANDARDIZED*4 + 100/* for \n */]; 

    unsigned char   ethernet_frame[MAX_ETHERNET_FRAME_SIZE_STANDARDIZED] = {0};
    unsigned short  send_length = length + ETHERNET_FRAME_HEADER_SIZE + 1;
    mac_address_t   olt_mac_address;
    int            *piMacAddr;
    unsigned char   oam_ethernet_type = 0x88;
    unsigned char   oam_length        = 0x09;
    unsigned char   oam_sub_type      = 0x03;
    unsigned char   oam_flags         = 0x50;

    
    piMacAddr = GetPonChipMgmtPathMac ( olt_id ); 
    if( piMacAddr != NULL )
    {
        VOS_MemCpy(olt_mac_address, piMacAddr, sizeof(mac_address_t));
    }
    else
    {
        PONLOG_ERROR_1(PONLOG_FUNC_OAM_COMM, olt_id, PONLOG_ONU_IRRELEVANT,
                       "Error %d on Get_olt_mac_address\n", result);
        return (result);
    }
    
    /*DA*/
    memcpy( (ethernet_frame+ETHERNET_FRAME_DESTINATION_ADDRESS_BEGINNING_PLACE),
        (unsigned char*)(oam_destination_address), BYTES_IN_MAC_ADDRESS );
    /*SA*/
    memcpy( (ethernet_frame+ETHERNET_FRAME_SOURCE_ADDRESS_BEGINNING_PLACE),
        (unsigned char*)(olt_mac_address), BYTES_IN_MAC_ADDRESS );
    
    /*type*/
    ethernet_frame[ETHERNET_FRAME_TYPE_BEGINNING_PLACE] = oam_ethernet_type;
    
    /*lenght*/                                            
    ethernet_frame[ETHERNET_FRAME_TYPE_END_PLACE] = oam_length;
    
    /*sub type*/
    ethernet_frame[ETHERNET_FRAME_SUB_TYPE_BEGINNING_PLACE] = oam_sub_type;
    
    /*flags*/
    ethernet_frame[ETHERNET_FRAME_FLAGS_END_PLACE] = oam_flags;
    
    /*code*/
    ethernet_frame[ETHERNET_FRAME_CODE_BEGINNING_PLACE] = oam_code;
    
    /*content*/
    memcpy( (ethernet_frame+ETHERNET_FRAME_HEADER_SIZE+1),
            (unsigned char*)(content), length );
    
    
    /* padding pdu data with zeros if smaller than minimum */
    if ( send_length < MIN_ETHERNET_FRAME_SIZE ) 
    {
        memset( (unsigned char *)(ethernet_frame+send_length), 0, 
                (MIN_ETHERNET_FRAME_SIZE - send_length) );
        
        send_length = MIN_ETHERNET_FRAME_SIZE; 
    }
    
#if 0
    if(send_length <= MAX_ETHERNET_FRAME_SIZE_STANDARDIZED)
    {
        if (PONLOG_check_configuration_for_print(PONLOG_SEVERITY_INFO, PONLOG_FUNC_OAM_COMM, olt_id, llid))
        {
            PON_general_hex_bytes_to_string((unsigned char *)ethernet_frame, send_length, hex_str, sizeof(hex_str));   
            PONLOG_INFO_4(PONLOG_FUNC_OAM_COMM, olt_id, llid, "OLT %d ONU %d TX: length: %d, ethernet_frame: %s\n\n", olt_id, llid, send_length, hex_str);
        }
    }
    else
    {
        PONLOG_INFO_4( PONLOG_FUNC_OAM_COMM, olt_id, llid,
                       "OLT %d ONU %d oam frame length length is %d, max size for priniting is %d\n",
                       olt_id, llid, send_length, MAX_ETHERNET_FRAME_SIZE_STANDARDIZED );
    }
#endif

    result = OLT_SendFrame2PON(olt_id, llid, ethernet_frame, (int)send_length);

#else

    switch ( oam_code )
    {
        case PON_OAM_CODE_VENDOR_EXTENSION:
            result = PonExtOamPktTx(olt_id, llid, content, (int)length);
        break;
        case PON_OAM_CODE_OAM_IMFORMATION:
            result = PonExtOamInfoTx(olt_id, llid, content, (int)length);
        break;
        case PON_OAM_CODE_EVENT_NOTIFICATION:
            result = PonExtOamEvtTx(olt_id, llid, content, (int)length);
        break;
        default:
            VOS_ASSERT(0);
            
    }

#endif
    
    return result;
}
                  
#undef PONLOG_FUNCTION_NAME




#define PONLOG_FUNCTION_NAME  "OAM_MODULE_recv_frame"

PON_STATUS PON_OAM_MODULE_receive_frame(  
                 const short int                 olt_id,
                 const PON_llid_t                llid,
				 const PON_oam_code_t		     oam_code, 
				 const OAM_oui_t                 oam_oui,
                 const unsigned short            length,
                 const unsigned char            *content)
{
#if 0
    mac_address_t                         destination_mac_address;
#endif
    unsigned char                         index;
#if 0
    bool                                  valid_address=TRUE;
    unsigned short                        received_ethernet_type;
    OAM_oui_t							  received_oui;
    unsigned char                         received_sub_type;  
    unsigned short                        received_flags;
    unsigned char                         received_oam_code; 
#endif
    VENDOR_OAM_frame_received_handler_t   handler_function;
    short int                             result;
#if 0
	PON_onu_id_t						  onu_id = llid; 
	char		                          hex_str[MAX_ETHERNET_FRAME_SIZE_STANDARDIZED*4 + 100/* for \n */]; 
    

    unsigned char                         *received_buffer = (unsigned char *)content;
#endif


#if 0
	if(length <= MAX_ETHERNET_FRAME_SIZE_STANDARDIZED)
	{
		if (PONLOG_check_configuration_for_print(PONLOG_SEVERITY_INFO, PONLOG_FUNC_OAM_COMM, olt_id, onu_id))
		{
            PON_general_hex_bytes_to_string((unsigned char *)content, length, hex_str, sizeof(hex_str));   
            PONLOG_INFO_4(PONLOG_FUNC_OAM_COMM, olt_id, onu_id, "OLT %d ONU %d RX: length: %d, content: %s\n\n", olt_id, onu_id, length, hex_str);
		}
	}
	else
	{
		PONLOG_INFO_4( PONLOG_FUNC_OAM_COMM, olt_id, onu_id,
					   "OLT %d ONU %d oam frame length length is %d, max size for priniting is %d\n",
					   olt_id, onu_id, length, MAX_ETHERNET_FRAME_SIZE_STANDARDIZED );
	}
    

    /* ETHERNET header */

    /* check illegal frame size */
    if ( (length < MIN_ETHERNET_FRAME_SIZE) || (length > MAX_ETHERNET_FRAME_SIZE_STANDARDIZED) )
    {

        PONLOG_ERROR_3(PONLOG_FUNC_OAM_COMM, olt_id, onu_id, "ERROR: OLT:%d ONU:%d received wrong size frame (%d bytes )\n",olt_id, llid, length);
        return;
    }


    /* extract destination mac address */
    memcpy( destination_mac_address, (unsigned char *)(received_buffer+ETHERNET_FRAME_DESTINATION_ADDRESS_BEGINNING_PLACE), BYTES_IN_MAC_ADDRESS);

    /* check destination mac address */
    for(index = 0; index < BYTES_IN_MAC_ADDRESS; index++)
    {
        if(destination_mac_address[index] != oam_destination_address[index])
        {
            valid_address = FALSE;
        }
    }

    if( !valid_address )
    {
		char received_mac_string[44];
		char expected_mac_string[44];
		unsigned char mac_counter;

		for (mac_counter=0; mac_counter < sizeof(mac_address_t); mac_counter ++) 
		{
			sprintf (&received_mac_string[5*mac_counter],"0x%02x-",(destination_mac_address[mac_counter]));
		}
		received_mac_string[5*mac_counter-1]=0;

		for (mac_counter=0; mac_counter < sizeof(mac_address_t); mac_counter ++) 
		{
			sprintf (&expected_mac_string[5*mac_counter],"0x%02x-",(oam_destination_address[mac_counter]));
		}
		expected_mac_string[5*mac_counter-1]=0;


		PONLOG_INFO_4( PONLOG_FUNC_OAM_COMM, olt_id, onu_id,
					"OLT %d ONU %d expected mac_address %s, retrieved mac_address %s\n",
					olt_id, onu_id, expected_mac_string, received_mac_string);

        return;/* not a relevant frame */
    }

    
    /* extract ethernet type */
    received_ethernet_type = (unsigned short)MAKE_UNSIGNED_SHORT_PARAM(*(received_buffer+ETHERNET_FRAME_TYPE_BEGINNING_PLACE),*(received_buffer+ETHERNET_FRAME_TYPE_BEGINNING_PLACE+1)); 
 
    /* check ethernet type */
    if( received_ethernet_type != const_oam_ethernet_type )
    {
        PONLOG_INFO_4( PONLOG_FUNC_OAM_COMM, olt_id, onu_id, "OLT %d ONU %d: received wrong ether type 0x%x, expected 0x%x \n",
                        olt_id, onu_id, received_ethernet_type, const_oam_ethernet_type );


        return;/* not a relevant frame */
    }


    /* OAM header */

    /* extract sub type */
    received_sub_type      = (unsigned char)  received_buffer[ETHERNET_FRAME_SUB_TYPE_BEGINNING_PLACE];

    /* check sub type */
    if( received_sub_type != const_oam_sub_type )
    {

        PONLOG_ERROR_4( PONLOG_FUNC_OAM_COMM, olt_id, onu_id,"OLT %d ONU %d: received wrong OAM sub type. expected :0x%x , received 0x%x )\n",
                        olt_id, onu_id, const_oam_sub_type, received_sub_type);
        
        return;
    }

    
    /* extract flags */
    received_flags = (unsigned short)(MAKE_UNSIGNED_SHORT_PARAM(*(received_buffer+ETHERNET_FRAME_FLAGS_BEGINNING_PLACE),*(received_buffer+ETHERNET_FRAME_FLAGS_END_PLACE))); 

    /* extract code */
    received_oam_code         = (unsigned char)  received_buffer[ETHERNET_FRAME_CODE_BEGINNING_PLACE];
#endif


    /* check OAM code */
    switch(oam_code)
    {
    case PON_OAM_CODE_VENDOR_EXTENSION:
#if 0
        /* extract OUI */
        for(index=0; index < OAM_OUI_SIZE; index++ )
        {
            received_oui[index]  = (unsigned char)  received_buffer[OAM_FRAME_OUI_BEGINNING_PLACE+index];

        }
#endif
      
        result = Get_handler_from_db(olt_id, llid, PON_OAM_CODE_VENDOR_EXTENSION, oam_oui, &handler_function);

        if( (result == OAM_EXIT_OK) && (handler_function != NULL) )/* pre-assigned handler found*/
        {
#if 0
            handler_function(olt_id, llid, PON_OAM_CODE_VENDOR_EXTENSION, received_oui, (unsigned short)(length - OAM_FRAME_OUI_END_PLACE), received_buffer+OAM_FRAME_OUI_END_PLACE);
#else
            handler_function(olt_id, llid, PON_OAM_CODE_VENDOR_EXTENSION, oam_oui, (unsigned short)length, content);
#endif

        }
    break;
    case PON_OAM_CODE_OAM_IMFORMATION:

        result = Get_handler_from_db(olt_id, llid, PON_OAM_CODE_OAM_IMFORMATION, oam_oui, &handler_function);

        if( (result == OAM_EXIT_OK) && (handler_function != NULL) )/* pre-assigned handler found*/
        {
#if 0
            handler_function(olt_id, llid, PON_OAM_CODE_OAM_IMFORMATION, received_oui, (unsigned short)(length-ETHERNET_FRAME_CODE_END_PLACE), received_buffer+ETHERNET_FRAME_CODE_END_PLACE+1);
#else
            handler_function(olt_id, llid, PON_OAM_CODE_OAM_IMFORMATION, oam_oui, (unsigned short)length, content);
#endif
        }
    break;
    case PON_OAM_CODE_EVENT_NOTIFICATION:
        
        result = Get_handler_from_db(olt_id, llid, PON_OAM_CODE_EVENT_NOTIFICATION, oam_oui, &handler_function);
        
        if( (result == OAM_EXIT_OK) && (handler_function != NULL) )/* pre-assigned handler found*/
        {
#if 0
            handler_function(olt_id, llid, PON_OAM_CODE_EVENT_NOTIFICATION, oam_oui, (unsigned short)(length - OAM_FRAME_SN_END_PLACE), received_buffer+OAM_FRAME_SN_END_PLACE);
#else
            handler_function(olt_id, llid, PON_OAM_CODE_EVENT_NOTIFICATION, oam_oui, (unsigned short)length, content);
#endif
        }
    break;
    default:
        return OAM_EXIT_ERROR;

    }
   
    return OAM_EXIT_OK;
}

#undef PONLOG_FUNCTION_NAME



static short int Find_empty_entry( unsigned char  *entry )
{

    for((*entry) = 0; (*entry) < OAM_MODULE_DB_MAX_SIZE; (*entry)++)
    {
        if(oam_internal_database[(*entry)].occupied != TRUE)
        {
            return OAM_EXIT_OK;
            
        }
    }

    return OAM_DB_FULL;

    
}

#define PONLOG_FUNCTION_NAME  "Insert_to_db"

static short int Insert_to_db
             ( const PON_oam_code_t            	     oam_code,
               const OAM_oui_t  					 oui,
               VENDOR_OAM_frame_received_handler_t   handler_func )
{
    short int           result=OAM_EXIT_OK;
    unsigned char       entry;
    unsigned long       oui_as_number;

   

    result = Find_empty_entry(&entry);
    if(result != OAM_EXIT_OK)
    {
        PONLOG_ERROR_4( PONLOG_FUNC_OAM_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: can not registrate oui:0x%x%x%x oam_code:0x%x. all entries occupied\n",
                        oui[0],oui[1],oui[2], oam_code );

        return result;
    }

    oam_internal_database[entry].handler_function = handler_func;
    oam_internal_database[entry].oam_code = oam_code;
    oam_internal_database[entry].occupied = TRUE;


    switch(oam_code)
    {
    case PON_OAM_CODE_OAM_IMFORMATION:
    case PON_OAM_CODE_EVENT_NOTIFICATION:
        break;
    case PON_OAM_CODE_VENDOR_EXTENSION:

        result = Convert_oui_type_to_number(oui, &oui_as_number);
        if( result != OAM_EXIT_OK )
        {
            PONLOG_ERROR_3(PONLOG_FUNC_OAM_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: can not convert OUI: 0x%x%x%x \n. discarding request", oui[0],oui[1],oui[2]);

            return OAM_EXIT_ERROR;
        }

        oam_internal_database[entry].oui = oui_as_number;
        break;
    default:
        PONLOG_ERROR_1(PONLOG_FUNC_OAM_COMM, PONLOG_OLT_IRRELEVANT, PONLOG_ONU_IRRELEVANT, "ERROR: Unsupported OAM code: 0x%x\n. discarding request", oam_code);
    }

    return result;
 
}
#undef PONLOG_FUNCTION_NAME



#define PONLOG_FUNCTION_NAME  "Get_handler_from_db"

static short int Get_handler_from_db
             ( const short int                       olt_id,
               const PON_llid_t                      llid,
               const PON_oam_code_t            	     oam_code,
               const OAM_oui_t  					 oui,
               VENDOR_OAM_frame_received_handler_t  *handler_func )
{
    short int           result=OAM_EXIT_OK;
    unsigned char       entry;
    unsigned long       oui_as_number;
	PON_onu_id_t		onu_id = llid; 

   
  
    for(entry = 0; entry < OAM_MODULE_DB_MAX_SIZE; entry++)
    {
        if(oam_internal_database[entry].occupied)
        {
            if (oam_internal_database[entry].oam_code == oam_code)
            {
                switch(oam_code)
                {
                case PON_OAM_CODE_OAM_IMFORMATION:
                case PON_OAM_CODE_EVENT_NOTIFICATION:
                    (*handler_func) = oam_internal_database[entry].handler_function;

                    return OAM_EXIT_OK;
                    break;
                case PON_OAM_CODE_VENDOR_EXTENSION:
                
                    result = Convert_oui_type_to_number(oui, &oui_as_number);

                    if( result != OAM_EXIT_OK )
                    {
                        PONLOG_ERROR_5( PONLOG_FUNC_OAM_COMM, olt_id, onu_id, "OLT %d ONU %d: can not convert OUI: 0x%x%x%x \n. discarding frame", 
										olt_id, onu_id, oui[0],oui[1],oui[2]);
        
                    }

                    if( oam_internal_database[entry].oui == oui_as_number) 
                    {
                        (*handler_func) = oam_internal_database[entry].handler_function;

                        return OAM_EXIT_OK;
                    }

                    break;
                    default:
                        PONLOG_ERROR_3(PONLOG_FUNC_OAM_COMM, olt_id, onu_id, "OLT %d ONU %d: Unsupported OAM code: 0x%x\n. discarding request", 
										olt_id, onu_id, oam_code);
                }
            }
        }
    }

    return OAM_QUERY_FAILED;
    
}
#undef PONLOG_FUNCTION_NAME



static short int Convert_oui_type_to_number( 
                    const OAM_oui_t   oui,
                    unsigned long    *oui_as_number )
{

    short int result=OAM_EXIT_OK;

    (*oui_as_number) = oui[2]+(oui[1] << BITS_IN_BYTE)+(oui[0] << (BYTES_IN_WORD*BITS_IN_BYTE));


    return result;
    
}


#if 1

#define PONLOG_FUNCTION_NAME "OAM_OnuRegister_EventNotify"
int OAM_OnuRegister_EventNotify(short int olt_id, short int onu_id, mac_address_t onu_mac)
{
    int result;
    unsigned short send_size, recv_size;
    unsigned char send_data[64];
    unsigned char recv_data[64];

    send_size = 64;
    recv_size = 64;
    if ( 0 == (result = PonStdOamGetHeartInfos(olt_id, onu_id, &send_size, send_data, &recv_size, recv_data)) )
    {
        if ( 0 == (result = OLT_SetLLIDHeartbeatOam(olt_id, onu_id, EFM_OAM_DEFAULT_INFO_TIMER / 10, send_size, send_data, EFM_OAM_DEFAULT_TIMER_OUT / 10, recv_size, recv_data, FALSE)) )
        {
            if ( 0 != (result = PonStdOamSetHeartMode(olt_id, onu_id, EFM_OAM_HARDWARE_MODE)) )
            {
                PONLOG_ERROR_3(PONLOG_FUNC_OAM_COMM, olt_id, onu_id, "OLT %d ONU %d: Failed(%d) to set OAM HeartMode: hardware", 
    							olt_id, onu_id, result);
            }
        }
        else
        {
            PONLOG_ERROR_3(PONLOG_FUNC_OAM_COMM, olt_id, onu_id, "OLT %d ONU %d: Failed(%d) to setup OAM HeartInfo", 
    						olt_id, onu_id, result);
        }
    }
    else
    {
        PONLOG_ERROR_3(PONLOG_FUNC_OAM_COMM, olt_id, onu_id, "OLT %d ONU %d: Failed(%d) to get OAM HeartInfo", 
						olt_id, onu_id, result);
    }

    return result;
}
#undef PONLOG_FUNCTION_NAME

#define PONLOG_FUNCTION_NAME "OAM_OnuDeregister_EventNotify"
int OAM_OnuDeregister_EventNotify(short int olt_id, short int onu_id, PON_onu_deregistration_code_t deregistration_code)
{
    int result;
    
    if ( 0 != (result = PonStdOamSetHeartMode(olt_id, onu_id, EFM_OAM_SOFTWARE_MODE)) )
    {
        PONLOG_ERROR_3(PONLOG_FUNC_OAM_COMM, olt_id, onu_id, "OLT %d ONU %d: Failed(%d) to set OAM HeartMode: software", 
						olt_id, onu_id, result);
    }

	return result;
}
#undef PONLOG_FUNCTION_NAME


#define PONLOG_FUNCTION_NAME "oam_pon_event_handler"
static int oam_pon_event_handler(PON_event_handler_index_t event_id, void *event_data)
{
    int result = 0;
    
    switch ( event_id )
    {
        case PON_EVT_HANDLER_LLID_END_STD_OAM_DISCOVERY:
        {
            ONURegisterInfo_S *OnuRegisterData;

            if ( NULL != (OnuRegisterData = (ONURegisterInfo_S*)event_data) )
            {
                result = OAM_OnuRegister_EventNotify(OnuRegisterData->olt_id, OnuRegisterData->onu_id, OnuRegisterData->mac_address);
            }
        }
        
        break;
        case PON_EVT_HANDLER_LLID_END_EXT_OAM_DISCOVERY:
        {
            OnuEventData_s *OnuRegisterExtData;

            if ( NULL != (OnuRegisterExtData = (OnuEventData_s*)event_data) )
            {
                result = OAM_OnuRegister_EventNotify(OnuRegisterExtData->PonPortIdx, OnuRegisterExtData->llid, OnuRegisterExtData->onu_mac);
            }
        }
        
        break;
        case PON_EVT_HANDLER_LLID_DEREGISTER_EVENT:
        {
            ONUDeregistrationInfo_S *OnuDeregistrationData;

            if ( NULL != (OnuDeregistrationData = (ONUDeregistrationInfo_S*)event_data) )
            {
                result = OAM_OnuDeregister_EventNotify(OnuDeregistrationData->olt_id, OnuDeregistrationData->onu_id, OnuDeregistrationData->deregistration_code);
            }
        }
        
        break;
        default:
            VOS_ASSERT(0);
    }

    return result;
}
#undef PONLOG_FUNCTION_NAME
#endif


