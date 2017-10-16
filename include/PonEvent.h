/*                   Copyright(c) 2008-2012 GWTT, Inc.                    */
/*  
**  PonEvent.h -  Header file for EVENT interface 
**
**  This file was written by liwei056, 03/09/2012
**  
**  Changes:
**
**  Version       |  Date          |    Change        |    Author	  
**  ----------|-----------|-------------|------------
**	1.00          | 03/09/2012 |	creation	      | liwei056
*/

#if !defined(__PONEVENT_H__)
#define __PONEVENT_H__

#if defined(__cplusplus)
extern "C" {
#endif
 

#ifndef GENERAL_HANDLER_FUNC 
#define GENERAL_HANDLER_FUNC
typedef void (*general_handler_function_t)();
#endif

#define PON_MAX_INDEX_HANDLERS 50
#define PON_MAX_CLIENT	       20

typedef struct PON_handlers_t 
{
    general_handler_function_t functions[PON_MAX_CLIENT][PON_MAX_INDEX_HANDLERS];


} PON_handlers_t;

#define PON_HANDLER_INDEX_ISVALID(idx)  ( ((idx) >= 0) && ((idx) < PON_MAX_INDEX_HANDLERS) )


short int PON_general_init_handler_function( PON_handlers_t  *handlers );

short int PON_general_terminate_handler_function( PON_handlers_t  *handlers );

short int PON_general_assign_handler_function 
                           ( PON_handlers_t					   *handlers,
	                         const unsigned short int           handler_function_index,
                         	 const general_handler_function_t 	handler_function, 
                             unsigned short                    *handler_id );

short int PON_general_delete_handler_function 
                           (       PON_handlers_t                 *handlers,
                             const unsigned short                  handler_id );



short int PON_general_get_handler_function
                           (       PON_handlers_t                 *handlers,
	                         const unsigned short int              handler_function_index,
                             const unsigned short                  client_number ,
                             general_handler_function_t  	      *handler_function );



#if defined(__cplusplus)
}
#endif

#endif /* __PONEVENT_H__ */

