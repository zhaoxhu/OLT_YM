/*  
**  micro_parser.h - definitions/declarations of micro parsing functions
**
**  This software is licensed for use according to the terms set by the Passave API license agreement.
**  Copyright Passave Ltd. 
**  Ackerstein Towers - A, 9 Hamenofim St.
**  POB 2089, Herziliya Pituach 46120 Israel
**
**
**  This file was written by Ran Ne'man, ran.neeman@passave.com, 16/02/2003
**  This file contains the former PASCOMM_micro_parser.h/c files, and some macros from PON_general_expo.h
**  
**
**  Changes:
**
**  Version	  |  Date	   |    Change	             |    Author	  
**  ----------|------------|-------------------------|------------
**	1.00	  | 16/02/2003 |	creation		     | Ran Ne'man
**	1.01	  | 29/05/2003 |BOOL_2_WORD_UBUFFER added| Ran Ne'man
**	4.00	  | 29/06/2003 |double type parse update | Ran Ne'man
**	4.01	  | 11/11/2003 |WORD_UBUFFER_2_BOOL added| Ran Ne'man
*/

#ifndef _PON_MICRO_PARSER_H__
#define _PON_MICRO_PARSER_H__


#ifndef __PASSAVE_ONU__
# define INT64S signed__int64
# define INT64U unsigned__int64
#endif

#define PON_SYSTEM_ENDIANITY_CHANGE

 /* modified by chenfj : ½«ºêx ¸ÄÎª(x)  */


/* Macro for translation from bool variable to 2 unsigned chars buffer
**
** Input parameters:
**				__x__	 - bool parameter
**
** Output parameters:
**				buffer  - Pointer to unsigned char array of two chars or more
**								 
*/
#define BOOL_2_WORD_UBUFFER( __x__, buffer )                {if (__x__ == TRUE) buffer[1] = 0x1; else buffer[1] = 0x0; buffer[0] = 0x0;}

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define BOOL_2_ENDIANITY_WORD_UBUFFER( __x__, buffer )  {if (__x__ == TRUE) buffer[0] = 0x1; else buffer[0] = 0x0; buffer[1] = 0x0;}
#else
	#define BOOL_2_ENDIANITY_WORD_UBUFFER BOOL_2_WORD_UBUFFER
#endif


/* Macro for translation from unsigned chars buffer to bool variable
**
** Input parameters:
**				buffer  - Pointer to unsigned char array of two chars 
**
** Output parameters:
**				__x__	 - bool parameter
**								 
*/
#define WORD_UBUFFER_2_BOOL(buffer, __x__)                  {if ((buffer[0] == 0x0) && (buffer[1] == 0x0)) __x__ = FALSE; else __x__ = TRUE;}

#ifdef PON_SYSTEM_ENDIANITY_CHANGE /* doesn't affect the implementation */
	#define ENDIANITY_WORD_UBUFFER_2_BOOL  WORD_UBUFFER_2_BOOL
#else
	#define ENDIANITY_WORD_UBUFFER_2_BOOL  WORD_UBUFFER_2_BOOL
#endif


/* Macro for translation from unsigned short number to unsigned char buffer
**
** Input parameters:
**				x	    - unsigned short number
**
** Output parameters:
**				buffer  - Pointer to unsigned char array
**								 
*/
#define USHORT_2_UBUFFER( x, buffer )                {buffer[1] = (unsigned char) (x & (0x00ff)); buffer[0] = (unsigned char)((x & (0xff00))>>BITS_IN_BYTE);}

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define USHORT_2_ENDIANITY_UBUFFER( x, buffer )  {buffer[0] = (unsigned char) (x & (0x00ff)); buffer[1] = (unsigned char)((x & (0xff00))>>BITS_IN_BYTE);}
#else
	#define USHORT_2_ENDIANITY_UBUFFER USHORT_2_UBUFFER
#endif


/* Macro for translation from unsigned char buffer to unsigned short number
**
** Input parameters:
**				buffer  - Pointer to unsigned char array
**
** Output parameters:
**				x	    - unsigned short number
**								 
*/
#define UBUFFER_2_USHORT( buffer, x )               x = (unsigned short)(buffer[1] + (buffer[0] << BITS_IN_BYTE));

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UBUFFER_2_ENDIANITY_USHORT( buffer, x ) x = (unsigned short)(buffer[0] + ((buffer[1] << BITS_IN_BYTE)));
#else
	#define UBUFFER_2_ENDIANITY_USHORT UBUFFER_2_USHORT
#endif


/* Macro for translation from unsigned char (represent by word ) buffer to unsigned char
**
** Input parameters:
**				buffer  - Pointer to unsigned char array
**
** Output parameters:
**				x	    - unsigned char number
**								 
*/
#define WORD_UBUFFER_2_UCHAR( buffer, x )               x = buffer[1];

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define WORD_UBUFFER_2_ENDIANITY_UCHAR( buffer, x ) x = buffer[0];
#else
	#define WORD_UBUFFER_2_ENDIANITY_UCHAR  WORD_UBUFFER_2_UCHAR
#endif


/* Macro for translation from unsigned char to buffer
**
** Input parameters:
**              x	    - unsigned char number
**
** Output parameters:
**              buffer  - Pointer to unsigned char array
**												 
*/
#define UCHAR_2_UBUFFER( x, buffer )               buffer[1] = x;

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UCHAR_2_ENDIANITY_UBUFFER( x, buffer ) buffer[0] = x;
#else
	#define UCHAR_2_ENDIANITY_UBUFFER  UCHAR_2_UBUFFER
#endif


/* Macro for translation from __int64 number to unsigned char buffer
**
** Input parameters:
**				x	    - __int64 number
**
** Output parameters:
**				buffer  - Pointer to unsigned char array
**								 
*/
#define ULONG_LONG_2_UBUFFER( x, buffer )                {buffer[7] = (unsigned char) (x & (0x00000000000000ffLL)); \
														  buffer[6] = (unsigned char)((x & (0x000000000000ff00LL))>>BITS_IN_BYTE); \
														  buffer[5] = (unsigned char)((x & (0x0000000000ff0000LL))>>(BITS_IN_BYTE*2)); \
														  buffer[4] = (unsigned char)((x & (0x00000000ff000000LL))>>(BITS_IN_BYTE*3)); \
														  buffer[3] = (unsigned char)((x & (0x000000ff00000000LL))>>(BITS_IN_BYTE*4)); \
														  buffer[2] = (unsigned char)((x & (0x0000ff0000000000LL))>>(BITS_IN_BYTE*5)); \
														  buffer[1] = (unsigned char)((x & (0x00ff000000000000LL))>>(BITS_IN_BYTE*6)); \
														  buffer[0] = (unsigned char)((x & (0xff00000000000000LL))>>(BITS_IN_BYTE*7));}

#ifdef __PASSAVE_ONU__

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define ULONG_LONG_2_ENDIANITY_UBUFFER ULONG_LONG_2_UBUFFER
#else
	#define ULONG_LONG_2_ENDIANITY_UBUFFER( x, buffer )  {buffer[0] = (unsigned char) (x & (0x00000000000000ffLL)); \
														  buffer[1] = (unsigned char)((x & (0x000000000000ff00LL))>>BITS_IN_BYTE); \
														  buffer[2] = (unsigned char)((x & (0x0000000000ff0000LL))>>(BITS_IN_BYTE*2)); \
														  buffer[3] = (unsigned char)((x & (0x00000000ff000000LL))>>(BITS_IN_BYTE*3)); \
														  buffer[4] = (unsigned char)((x & (0x000000ff00000000LL))>>(BITS_IN_BYTE*4)); \
														  buffer[5] = (unsigned char)((x & (0x0000ff0000000000LL))>>(BITS_IN_BYTE*5)); \
														  buffer[6] = (unsigned char)((x & (0x00ff000000000000LL))>>(BITS_IN_BYTE*6)); \
														  buffer[7] = (unsigned char)((x & (0xff00000000000000LL))>>(BITS_IN_BYTE*7));}
#endif

#else
    #define ULONG_LONG_2_ENDIANITY_UBUFFER INT64_2_ENDIANITY_UBUFFER
#endif


/* Macro for translation from unsigned char buffer to __int64 number
**
** Input parameters:
**				buffer  - Pointer to unsigned char array
**
** Output parameters:
**				
**              x	    - __int64 number
**								 
*/
#define UBUFFER_2_ULONG_LONG( buffer, x )               x = (INT64U)buffer[7] + \
														  ((INT64U)buffer[6] << BITS_IN_BYTE) + \
														  ((INT64U)buffer[5] << (BITS_IN_BYTE*2)) + \
														  ((INT64U)buffer[4] << (BITS_IN_BYTE*3)) + \
														  ((INT64U)buffer[3] << (BITS_IN_BYTE*4)) + \
														  ((INT64U)buffer[2] << (BITS_IN_BYTE*5)) + \
														  ((INT64U)buffer[1] << (BITS_IN_BYTE*6)) + \
														  ((INT64U)buffer[0] << (BITS_IN_BYTE*7));
#ifdef __PASSAVE_ONU__

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UBUFFER_2_ENDIANITY_ULONG_LONG UBUFFER_2_ULONG_LONG
#else
	#define UBUFFER_2_ENDIANITY_ULONG_LONG( buffer, x ) x = (INT64U)buffer[0] + \
														  ((INT64U)buffer[1] << BITS_IN_BYTE) + \
														  ((INT64U)buffer[2] << (BITS_IN_BYTE*2)) + \
														  ((INT64U)buffer[3] << (BITS_IN_BYTE*3)) + \
														  ((INT64U)buffer[4] << (BITS_IN_BYTE*4)) + \
														  ((INT64U)buffer[5] << (BITS_IN_BYTE*5)) + \
														  ((INT64U)buffer[6] << (BITS_IN_BYTE*6)) + \
														  ((INT64U)buffer[7] << (BITS_IN_BYTE*7));
#endif

#else
    #define UBUFFER_2_ENDIANITY_ULONG_LONG UBUFFER_2_ENDIANITY_INT64
#endif


/* Macro for translation from unsigned long number to unsigned char buffer
**
** Input parameters:
**				x	    - unsigned long number
**
** Output parameters:
**				buffer  - Pointer to unsigned char array
**								 
*/
#define ULONG_2_UBUFFER( x, buffer )                {buffer[3] = (unsigned char) (x & (0x000000ff)); buffer[2] = (unsigned char)((x & (0x0000ff00))>>BITS_IN_BYTE); buffer[1] = (unsigned char)((x & (0x00ff0000))>>(BITS_IN_BYTE*2)); buffer[0] = (unsigned char)((x & (0xff000000))>>(BITS_IN_BYTE*3));}

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define ULONG_2_ENDIANITY_UBUFFER(x, buffer )  {buffer[0] = (unsigned char) (x & (0x000000ff)); buffer[1] = (unsigned char)((x & (0x0000ff00))>>BITS_IN_BYTE); buffer[2] = (unsigned char)((x & (0x00ff0000))>>(BITS_IN_BYTE*2)); buffer[3] = (unsigned char)((x & (0xff000000))>>(BITS_IN_BYTE*3));}
#else
	#define ULONG_2_ENDIANITY_UBUFFER ULONG_2_UBUFFER
#endif


/* Macro for translation from limited unsigned long number(only 3 bytes) to unsigned char buffer
**
** Input parameters:
**				x	    - limited unsigned long number
**
** Output parameters:
**				buffer  - Pointer to unsigned char array
**								 
*/
#define LIMITED_ULONG_2_UBUFFER( x, buffer )                {buffer[2] = (unsigned char) (x & (0x000000ff)); buffer[1] = (unsigned char)((x & (0x0000ff00))>>BITS_IN_BYTE); buffer[0] = (unsigned char)(((x) & (0x00ff0000))>>(BITS_IN_BYTE*2));}

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define LIMITED_ULONG_2_ENDIANITY_UBUFFER( x, buffer )  {buffer[0] = (unsigned char) (x & (0x000000ff)); buffer[1] = (unsigned char)((x & (0x0000ff00))>>BITS_IN_BYTE); buffer[2] = (unsigned char)((x & (0x00ff0000))>>(BITS_IN_BYTE*2));}
#else
	#define LIMITED_ULONG_2_ENDIANITY_UBUFFER LIMITED_ULONG_2_UBUFFER
#endif



/* Macro for translation from unsigned char buffer to unsigned long number
**
** Input parameters:
**				x	    - unsigned long number
**
** Output parameters:
**				buffer  - Pointer to unsigned char array
**								 
*/
#define UBUFFER_2_ULONG( buffer, x )               x = buffer[3] + (buffer[2] << BITS_IN_BYTE) + (buffer[1] << (BITS_IN_BYTE*2)) + (buffer[0] << (BITS_IN_BYTE*3));

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UBUFFER_2_ENDIANITY_ULONG( buffer, x ) x = buffer[0] + (buffer[1] << BITS_IN_BYTE) + (buffer[2] << (BITS_IN_BYTE*2)) + (buffer[3] << (BITS_IN_BYTE*3));
#else
	#define UBUFFER_2_ENDIANITY_ULONG UBUFFER_2_ULONG
#endif


/* Macro for translation from unsigned char buffer to limited(only 3 bytes) unsigned long number
**
** Input parameters:
**				x	    - limited unsigned long number
**
** Output parameters:
**				buffer  - Pointer to unsigned char array
**								 
*/

#define UBUFFER_2_LIMITED_LONG( buffer, x )               x = buffer[2] + (buffer[1] << BITS_IN_BYTE) + (buffer[0] << (BITS_IN_BYTE*2));

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UBUFFER_2_ENDIANITY_LIMITED_LONG( buffer, x ) x = buffer[0] + (buffer[1] << BITS_IN_BYTE) + (buffer[2] << (BITS_IN_BYTE*2));
#else
	#define UBUFFER_2_ENDIANITY_LIMITED_LONG UBUFFER_2_LIMITED_LONG
#endif




/* Macro for translation from unsigned char buffer to int24 (maybe casted into long)
**
** Input parameters:
**				x	    - unsigned int24 number
**
** Output parameters:
**				buffer  - Pointer to unsigned char array
**								 
*/
#define UBUFFER_2_UINT24( buffer, x )     x = buffer[2] + (buffer[1] << BITS_IN_BYTE) + (buffer[0] << (BITS_IN_BYTE*2));

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UBUFFER_2_ENDIANITY_UINT24( buffer, x ) x = buffer[0] + (buffer[1] << BITS_IN_BYTE) + (buffer[2] << (BITS_IN_BYTE*2));
#else
	#define UBUFFER_2_ENDIANITY_UINT24 UBUFFER_2_UINT24
#endif


/* Macro for translation from unsigned char buffer to double number
**
** Input parameters:
**				x	    - double number
**
** Output parameters:
**				buffer  - Pointer to unsigned char array
**								 
*/
#define UBUFFER_2_DOUBLE( buffer, x )				{short int __counter___; x = 0; for (__counter___=0;__counter___ < BYTES_IN_DOUBLE;__counter___ ++)      x = ((x * (2<<(BITS_IN_BYTE-1))) + buffer[__counter___]);}

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UBUFFER_2_ENDIANITY_DOUBLE( buffer, x ) {short int __counter___; x = 0; for (__counter___=(BYTES_IN_DOUBLE-1);__counter___ >= 0;__counter___ --) x = ((x * (2<<(BITS_IN_BYTE-1))) + buffer[__counter___]);}
#else
	#define UBUFFER_2_ENDIANITY_DOUBLE UBUFFER_2_DOUBLE
#endif


/* Macro for translation from 64-bit unsigned integer (unsigned__int64) to unsigned char buffer 
**
** Input parameters:
**				x	    - 64-bit unsigned integer
**
** Output parameters:
**				buffer  - Pointer to unsigned char array
**								 
*/
#define INT64_2_UBUFFER( x, buffer )				{ ULONG_2_UBUFFER(x.msb, buffer) ULONG_2_UBUFFER(x.lsb, (buffer+BYTES_IN_LONG)) }

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define INT64_2_ENDIANITY_UBUFFER( x, buffer )	{ ULONG_2_ENDIANITY_UBUFFER(x.lsb, buffer) ULONG_2_ENDIANITY_UBUFFER(x.msb, (buffer+BYTES_IN_LONG)) }
#else
	#define INT64_2_ENDIANITY_UBUFFER  INT64_2_UBUFFER
#endif


/* Macro for translation from unsigned char buffer to 64-bit unsigned integer (unsigned__int64) 
**
** Input parameters:
**				buffer  - Pointer to unsigned char array
**
** Output parameters:
**				x	    - 64-bit unsigned integer
**
*/
#define UBUFFER_2_INT64( buffer, x )				{ UBUFFER_2_ULONG		   (buffer, x.msb) UBUFFER_2_ULONG          ((buffer+BYTES_IN_LONG), x.lsb) }

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UBUFFER_2_ENDIANITY_INT64( buffer, x) 	{ UBUFFER_2_ENDIANITY_ULONG(buffer, x.lsb) UBUFFER_2_ENDIANITY_ULONG((buffer+BYTES_IN_LONG), x.msb) }
#else
	#define UBUFFER_2_ENDIANITY_INT64  UBUFFER_2_INT64
#endif


/* Macro for translation from unsigned char buffer to another unsigned char buffer
**
** Input parameters:
**				src_buffer    - Pointer to unsigned char array
**				size		  - Size of chars (bytes) to copy
**
** Output parameters:
**				dest_buffer   - Pointer to unsigned char array
**								 
*/
#define UBUFFER_2_UBUFFER( dest_buffer, src_buffer, size )                        {  long int          __counter___; for (__counter___=0; __counter___<size; dest_buffer[__counter___]       =src_buffer[__counter___],__counter___ ++);}

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UBUFFER_2_ENDIANITY_UBUFFER( dest_buffer, src_buffer, size )          {  long int          __counter___; for (__counter___=0; __counter___<size; dest_buffer[size-__counter___-1]=src_buffer[__counter___],__counter___ ++);}
	#define UBUFFER_2_ENDIANITY_UBUFFER_UNSIGNED( dest_buffer, src_buffer, size ) {  unsigned long int __counter___; for (__counter___=0; __counter___<size; dest_buffer[size-__counter___-1]=src_buffer[__counter___],__counter___ ++);}
#else
	#define UBUFFER_2_ENDIANITY_UBUFFER  UBUFFER_2_UBUFFER
	#define UBUFFER_2_ENDIANITY_UBUFFER_UNSIGNED( dest_buffer, src_buffer, size ) {  unsigned long int __counter___; for (__counter___=0; __counter___<size; dest_buffer[__counter___]       =src_buffer[__counter___],__counter___ ++);}   
#endif


#define UBUFFER_2_BIGGER_UBUFFER( dest_buffer, dest_buffer_size, src_buffer, src_buffer_size, padd ) \
		{  long int __counter___,__diff__; \
		   __diff__ = dest_buffer_size - src_buffer_size; \
		   if (__diff__>=0) { for (__counter___=0; __counter___<src_buffer_size; dest_buffer[__counter___+__diff__] = src_buffer[__counter___],__counter___ ++); \
						      memset( dest_buffer, padd, __diff__ );}}

#ifdef PON_SYSTEM_ENDIANITY_CHANGE
	#define UBUFFER_2_BIGGER_ENDIANITY_UBUFFER( dest_buffer, dest_buffer_size, src_buffer, src_buffer_size, padd ) \
		{  long int __counter___,__diff__; \
		   __diff__ = dest_buffer_size - src_buffer_size; \
		   if (__diff__>=0) { for (__counter___=0; __counter___<src_buffer_size; dest_buffer[dest_buffer_size-__counter___-1-__diff__] = src_buffer[__counter___],__counter___ ++); \
						      memset( dest_buffer+src_buffer_size, padd, __diff__ );}}
#else
	#define UBUFFER_2_BIGGER_ENDIANITY_UBUFFER  UBUFFER_2_BIGGER_UBUFFER
#endif


/* Macro for translation from unsigned char array to unsigned char buffer
**
** Input parameters:
**				src_buffer	    - unsigned char array
**				size		  -   Size of chars (bytes) to copy
**
** Output parameters:
**				dest_buffer		- Pointer to unsigned char array
**								 
*/

/*#define UARRAY_2_ENDIANITY_UBUFFER( dest_buffer, src_buffer, size ) {  long int __counter___; for (__counter___=0; __counter___<(size/4);  ULONG_2_ENDIANITY_UBUFFER((unsigned long)src_buffer[__counter___*4], dest_buffer + (__counter___*4)) ,__counter___ ++);} */
/*#define UARRAY_2_ENDIANITY_UBUFFER( dest_buffer, src_buffer, size )\
{\  
	int temp=0;\
}
	for(__count___=0; (__count___) < 4;  __count___++)\
	{\
		dest_buffer[__count___] = src_buffer[__count___];\
	ULONG_2_ENDIANITY_UBUFFER((*(src_buffer+(__counter___*4))), dest_buffer + (__counter___*4)); 
	}\  
} */


/* Copy macro optimized for MAC address */
#define  PON_MAC_ADDRESS_ENDIANITY_COPY( dst_buffer, src_buffer )  UBUFFER_2_ENDIANITY_UBUFFER( dst_buffer, src_buffer, BYTES_IN_MAC_ADDRESS )


#endif /* _PON_MICRO_PARSER_H__ */


