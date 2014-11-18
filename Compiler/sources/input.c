/*
 * input.c
 * 
 
 *
 * INPUT MODULE:
 *	Opens the indicated file, and loads a new line of characteres everytime it's asked. For accessing 
 *	the line-lenght buffer, it returns a pointer-to-buffer.
 *
 * DEPENDENCIES	
 * 	input.h, c12.h
 *   	
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "c12.h"
#include "input.h"


#define REALLOCATE	-15	/* flags that indicate wheter the space for the buffer is allocated for the first time */
#define ALLOCATE	-16

typedef char  FLAG;

static char	*fgets_( char*, int, FILE*);
static bool	 loadbfr_( FLAG );


static 	    FILE *source; 	  	 	  /* File-pointer to source file */ 
static 	    BUFFER buffer = NULL;	  	  /* The buffer */
static bool initialized = false;  	  	  /* Initialization flag */

bool buffer_init( const char *file_name )
{		
	/*
	    Opens the source file and loads it in the buffer. On failing returns false.
	*/

	if( !initialized ) /* Protection for more-than-one inizializations */
	{
		initialized = !initialized;

		/* opens source file in read-mode */
		if( ( source = fopen( file_name, "r" ) ) == NULL )
			return false;		
		else{
			/* loads buffer for the first time*/
			if( !loadbfr_( ALLOCATE ) )
				return false;
		}

	}	
	return true;	
}

bool loadbfr_( FLAG flag )
{
	/* 
	   Stores a line of characteres readed from the source file into the buffer
	   using an automatic array of size MAX_LINLEN. If loadbfr_ was called with ALLOCATE flag, then it
	   allocates space for the first time for the buffer, otherwise just reallocates the space.

	   In case of EOF, buffer maintains its last value before calling. On failing returns false.
 	*/
		
	char temp[ MAX_LINLEN ] = { SNUL };	/* automatic variable for reading from the source file */

	/* Loads at most (MAX_LINLEN-1) characters + '/0' from source file   */			
	if( ( fgets_( temp, MAX_LINLEN, source ) ) == NULL )						    
			return false;

	
	/*  Allocates or reallocates space for, at most, MAX_LINLEN characteres, 
	    including the null character. Although fgets_ always inserts the
	    null-character, the allocated or reallocated space is cleaned 
	    (filled with '/0') before copying from the temporary variable     */

	if( flag == ALLOCATE )
	{								
		if ( ( buffer = ( BUFFER )calloc( strlen(temp), sizeof(temp) ) ) == NULL ) 
			return false;  
	}
	else if( flag == REALLOCATE )
	{
		if( ( buffer = ( BUFFER )realloc( buffer, sizeof(temp) ) ) == NULL )
			return false;
		/* cleans the buffer */
		for(int i=0; i <= strlen(temp); i++)				
			buffer[i] = SNUL;
	}
			
	/* Copy at most MAX_LINLEN characters to buffer */
	if( ( strncpy( buffer, temp, strlen(temp) ) ) == NULL )
		return false;		
			
	return true;
}

bool buffer_free()
{
	/* 
	   closes source file and frees buffer. On failing returns false.
	*/
	free( buffer );
	if( fclose( source ) == EOF )
		return false;

	return true;
}

bool buffer_update()
{
	/* 
	   The next line in the source file is loaded in buffer. The former line is replaced. On failing returns false 
        */
	if( !loadbfr_( REALLOCATE ) ){ 		
		return false;
	}
	return true;
}


const BUFFER buffer_getptr() 
{	
	/* 
	   Returns a pointer to the buffer. BUFFER is typedefed as a pointer-to-char. The pointer is 
	   defined as constant, so the buffer cannot be modified through the pointer. 
	*/

	BUFFER aux = NULL;
	aux = buffer;

	return aux;
}

char* fgets_( char *s, int n, FILE *iop )
{
	
	/* 
	   Works just the same as the c99 fgets() defined in stdio. The difference, is that this 
	   function ignore all the null-characters that can make the string to end prematuraly. Aditionally,
	   handles all the extended-ascii characters as the character '`' ( 96 decimal ) to avoid problems
	   storing them in char-type variables

	   The original code  without the null-character-checking was taken from 
	   K&R pg.165, which is said to be copied from the std library.
	*/
		
	register int c;		
	register char *cs;       

	cs = s;
	while( --n > 0 && ( c = getc(iop) ) != EOF ){
		
		if( c > 127 ){ 	/*change all extended-aschii characters */ 
		 	c = 96;
	
		}
		if( c != 0 ) 	/* Ignores null-characters (0x00) */
		{
		     if( ( *cs++ = c ) == '\n' )
			     break;
		}
		
		    
	}
	*cs = '\0';  /* inserts the null-character */

	return ( c == EOF && cs == s)? NULL : s;	
}
