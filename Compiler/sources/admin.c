/*
 * admin.c
 * 
 * 
 *
 * ADMIN MODULE:
 *	It is the one the human interfaces with, and it serves as  interface between the scanner 
 *	module and the input module. It converts the line-buffer provided by input.c 
 *	into a stream of characters useful for the scanner module.
 *
 *
 *
 * DEPENDENCIES
 * 	admin.h, c12.h, parser.o, word_table.o, input.o semanalyzer.o
 *
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c12.h"
#include "admin.h"
#include "input.h"
#include "parser.h"
#include "word_table.h"
#include "semanalyzer.h"
#include "error_reporting.h"

typedef char LINE;

#define DEF_START	100
#define DEF_INSERTED	101
#define DEF_END		102

static bool getnewline();

static LINE 	  *line = NULL;			/* For accessing the buffer */ 
static CHARACTER  *current_character = NULL;    /* points to the character that will be returned by getcharacter() */	
static char	  *spelling_table[MAX_NUM_OF_TOKENS]; /* spelling table */

static char builtin_defs[] = 			/* built-in definitions provided in one line... */
		"int readint(void) {return 0;} void writeint(int outint) {;} bool readbool(void) {return true;}  void writebool(bool outbool) {;} \0";	  
static int builtin_def_state = DEF_START;	/* states var for inserting built-in functions */
static int size_sptable;			/* size_sptable */ 

int main( int argc, char *argv[] )
{
	
	const char *prog_name = *argv; 	  /* Program name */
	const char *file_name = *++argv;  /* Source file name */

	if( argc == 1 ) /*no args, notify and exit*/
	{
		fprintf( stderr, "%s: Source file was not indicated\n", prog_name );
		return EXIT_SUCCESS;
	}
	else
	{		
		/* Initializes buffer for the source file */
		if( !buffer_init( file_name ) )
		{
			/* buffer init failed*/
			fprintf( stderr, "%s: Failed attempting to open %s\n", prog_name, file_name );
			return EXIT_FAILURE;
		}		
		else
		{
			/* buffer initialized */

			wordtable_init();		/* creates word table */

			/* ACTUAL COMPILATION PROCCESS */
			parse();			/* creates the syntax tree */
			do_semantic_analysis();		/* annotates the syntax tree */


			/* checks for errors generated*/
			if( num_errors_found() > 0 ){
				/* error notification */
				printf("Errors found on %s:\n", file_name );
				for( int i=0; i < num_errors_found(); i++ ){
					printf("   Line %ld: %s\n", get_error_lineno(i), get_error_msg(i) );
				}
				printf("\n");
			}
			
			

		}			
	}

	/* frees buffer and closes file */
	buffer_free();

	return EXIT_SUCCESS;
}

bool getnewline()
{		
	/* 
	   In the first call obtains a pointer to the buffer. If this has already been done, then 
	   updates the buffer (new line). On failing returns false.

	   The first time it's called, before making "line" point to the file buffer,  makes it point to the built-in functions. It
	   can be separated in three steps:

		DEF_START -> DEF_INSERTED -> DEF_END (indicated by the "else")

	   In DEF_END, it would continue in normal operation, making line point to the main buffer. This way line numbers are not affected.
	*/
		
	
	if( builtin_def_state == DEF_START ){
		line = builtin_defs;			/* sets current line to built-in definitions array */
		builtin_def_state = DEF_INSERTED;	/* next state */
	}
	else if( builtin_def_state == DEF_INSERTED ){
		line = NULL;				/* this indicates first line, otherwise segfault */
		 builtin_def_state = DEF_END;		/* next state: finish inserting */
	}
	else{
		/* normal working process... this occurs on DEF_END */
		if ( line == NULL ) /* first line */
		{ 	
			if( ( line = buffer_getptr() ) == NULL ) /* gets pointer to buffer */
				return false; 
		}
		else		   /* update  buffer */
		{	
			if( !buffer_update() )
				return false; 
			
		}
	}	
	current_character = line;  /* restarts current-character position to 0 */
	
	return true;
}


CHARACTER getcharacter()
{
	/*  
	   It provides a stream of characters, returns a new character each time, and automatically 
	   update lines until end-of-file. It does not go back in the list, so it can be seen as 
	   poping characters from the list.
	*/


	CHARACTER c = SNUL;
				
	if( line == NULL )
		getnewline(); /* gets first line */		 

	
	/* end-of-line reached attempts to get newline */
	if( (current_character[0]) == SNUL )
	{		
		if( !getnewline() )
			return EOF;  /* EOF reached, no more lines */
		else
			return SNUL; /* New line is currently avaliable for the next call, reports end-of-line */
	}


	/* returns the current character, and advances the pointer to the next one */
	return c = current_character++[0];	
}

bool add_spellingtable( char *lex, int index ){

	if( spelling_table[index] == NULL ){ /* the space is free */

		/* allocates space for string and null character*/	
		spelling_table[index] = (char*)malloc( sizeof(char)*strlen(lex)+1 );
		/* copies lexeme */		
		memcpy( spelling_table[index], lex, strlen(lex) );
		/* sets last character as null character */
		spelling_table[index][strlen(lex)] = SNUL;
		/* sets size of the table */
		size_sptable = index+1; /* because of the elemnt number 0 */
	}
	else{
		/* the space is already occupied */
		return false;
	}
	return true;
}

char *get_spellingtable( int index ){
	/* returns the lexical value of the element with lexical index (LexI) */

	char *p;
	p =  spelling_table[ index ];

	return p;	
}

int size_spellingtable(){

	return size_sptable;
}
