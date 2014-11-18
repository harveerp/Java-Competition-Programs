/*
 * symbol_table.c
 * 
 * 
 *
 * SYMBOL-TABLE MODULE
 * 	Contains the declaration of the symbol table data structure and all the functions used to insert, lookup and hash the key.
 *
 * DEPENDENCIES	
 * 	symbol_table.h, c12.h
 *   	
 */

#include <string.h>
#include <stdlib.h>
#include "c12.h"
#include "symbol_table.h"


static char *strdup( const char* );
static unsigned hash( char* );

static ST_TABLE symboltable[ MAX_NUM_OF_TOKENS ]; 	/* The table -> If I have enough time, I'll change it later to be dynamic */

/* an implementation of the POSIX strdup function */
char *strdup( const char *s ){

	char *d = ( char* )( malloc( strlen(s) + 1 ) );	
	if( d == NULL ) return NULL;
	strcpy( d,s );
	return d;
}

/* forms hash value from string s*/
unsigned hash( char *s )
{
	
	unsigned hashval;
	
	for( hashval = 0; *s != '\0'; s++ ){
		hashval = *s + 31 * hashval;
	}

	return hashval % MAX_NUM_OF_TOKENS;
	
}


/*lookup for s in hashtab */
ST_BUCKET *symboltable_lookup( char *s )
{
	ST_BUCKET *np;
	
	for( np = symboltable[ hash(s) ]; np != NULL; np = np->next ){
		if( strcmp( s, np->lexeme ) == 0 )
			return np;				/* found */
	}
	return NULL;
}


ST_BUCKET *symboltable_install( char *lexeme )
{
	ST_BUCKET *np;
	unsigned hashval;

	if( ( np = symboltable_lookup(lexeme) ) == NULL )	/* not found: add */
	{
		np = (ST_BUCKET*)malloc( sizeof(*np) );
		if( np == NULL  || ( np->lexeme = strdup( lexeme ) ) == NULL )
			return NULL;
		hashval = hash( lexeme );
		np->next = symboltable[ hashval ];
		symboltable[ hashval ] = np;
	}
		
	/* return a pointer to the symbol-table entry */	
	return np;
}

