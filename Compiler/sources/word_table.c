/*
 * word_table.c
 * 
 *
 *
 * WORD-TABLE MODULE:
 *	contains the declaration of the word table data structure and all the functions used to insert, lookup and hash the key. 
 *	In contrary with the symbol table, it contains an adittional function wich fills the word table with all the reserved 
 *	words in c12 specification.
 *
 * DEPENDENCIES	
 * 	word_table.h c12.h
 *   	
 */

#include <string.h>
#include <stdlib.h>
#include "c12.h"
#include "word_table.h"


static char *strdup( const char* );
static unsigned hash( char* );

static TABLE reserved_words[ NUM_OF_RESERVED_WORDS ]; 	/* it's the table:  array of entries */

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

	return hashval % NUM_OF_RESERVED_WORDS;
	
}


/*lookup for s in hashtab */
BUCKET *wordtable_lookup( char *s )
{
	BUCKET *np;
	
	for( np = reserved_words[ hash(s) ]; np != NULL; np = np->next ){
		if( strcmp( s, np->lexeme ) == 0 )
			return np;	/* found */
	}
	return NULL;
}


BUCKET *wordtable_install( char *lexeme, int token_name )
{
	BUCKET *np;
	unsigned hashval;

	if( ( np = wordtable_lookup(lexeme) ) == NULL )	/* not found */
	{
		np = (BUCKET*)malloc( sizeof(*np) );
		if( np == NULL  || ( np->lexeme = strdup( lexeme ) ) == NULL )
			return NULL;
		hashval = hash( lexeme );
		np->next = reserved_words[ hashval ];
		reserved_words[ hashval ] = np;
	}
	else{ 						/* already there */
	}
		
	np->token_name = token_name;
		
	return np;
}

void wordtable_init()
{
	/* Install all the reserved words in the word table. 

	   New reserved words require adding a "install" line here, adding the token name to the enum in c12.h,
	   and increasing NUMBER_OF_RESERVED_WORDS by one in c12.h as well  
	*/
	
	wordtable_install( "and", AND );
	wordtable_install( "andthen", ANDTHEN );
	wordtable_install( "bool", BOOL );
	wordtable_install( "continue", CONTINUE );
	wordtable_install( "else", ELSE );
	wordtable_install( "end", END );
	wordtable_install( "exit", EXIT );
	wordtable_install( "if", IF );
	wordtable_install( "int", INT );
	wordtable_install( "loop", LOOP );
	wordtable_install( "mod", MOD );
	wordtable_install( "not", NOT );
	wordtable_install( "or", OR );
	wordtable_install( "orelse", ORELSE );
	wordtable_install( "ref", REF );
	wordtable_install( "return", RETURN ); 
	wordtable_install( "void", VOID );
	wordtable_install( "true", BLIT );
	wordtable_install( "false", BLIT );
}
