/*
 * error_reporting.c
 * 
 * 
 *
 * 
 * ERROR REPORTING MODULE:
 * 	Keeps a list of error strings, limited to 10 strings. Uses one function to add an entry, 
 *	and other three functions for printing the added strings.
 * 
 *
 * DEPENDENCIES:
 *	c12.h error_reporting.h
 *
 *   	
 */

#include "c12.h"
#include "error_reporting.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct err_str{ /* error data structure */
	char *msg;			/* message */
	long int lineno;		/* line number */
}ErrorList;

static ErrorList error_list[10];	/* the error list. Limited to 10 elements... it's useless to have more */
static int list_index;			/* index for the list */


int error_list_add( char* message, long int line_number ){
	/* 
	   adds an entry to the error list. returns the number of elements in the list
	*/

	if( list_index < 10 ){
		//copy string error
		error_list[list_index].msg = ( char* )malloc( sizeof(message)*strlen(message) ); //avails space
		memcpy( error_list[list_index].msg, message, strlen(message) );			 //copy
		//copy line number
		error_list[list_index].lineno = line_number;
		
		list_index++;
	}

	return list_index;
}

char *get_error_msg( int i ){
	return error_list[i].msg;
}

long int get_error_lineno( int i ){
	return error_list[i].lineno;
}

int num_errors_found(){
	return list_index;
}

