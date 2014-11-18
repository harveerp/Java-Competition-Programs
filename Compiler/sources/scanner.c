 /*
 * scanner.c
 * 
 * 
 *
 * SCANNER MODULE:
 *	Performs token recognition. Provides a stream of tokens when asked through the gettoken() function.
 *
 *
 * DEPENDENCIES:
 * 	scanner.c, scanner.h, c12.h, admin.o (although it seems to be circular), word_table.o, symbol_table.o error_reporting.o
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "c12.h"
#include "admin.h"
#include "scanner.h"
#include "word_table.h"
#include "symbol_table.h"
#include "error_reporting.h"

/* States */
#define START	     200
#define	PUNCTUATION  202	/* punctuation signs, including valid and invalid ones +,-, <, $, /, etc */
#define NUM_1	     208	
#define NUM_2	     209
#define NUM_3        210
#define NUM_4        211
#define NUM_5        212
#define COMM	     213
#define COMM_1	     214
#define WEIRD_COMM   215
#define COMM_2	     216
#define COMM_3	     217

/* Macros */
#define IS_LINEFEED(x)   	(x) == '\n'
#define IS_FIRSTRUN()    	(lineno == 0) ? true: false
#define IS_FIRSTLINE()	 	(lineno == 1) ? true: false
#define IS_DOT(x) 	 	(x=='.') ? true: false
#define IS_IGNORED(x)  	 	iscntrl(x) || IS_SPACE(x)   
#define IS_SPACE(x) 	     	( x==' ' ) ? true: false
#define IS_SPECIAL_SYMBOL(x) 	( special_symbol_table[x] != 0 )? true: false
#define IS_EQUAL(x) 		( x == '=' ) ? true: false
#define IS_UNDERSCORE(x) 	( x == '_' ) ? true: false
#define IS_STAR(x)		( x == '*' ) ? true: false
#define NEXT_STATE_IS(x) 	state = (x)					 /* defines the next state */
#define END_OF_FILE(x)   	(x) == EOF	 
#define INSERT(x,y) 	 	{ *x++ = (y); }    				 /* inserts the character the lexeme data structure */
#define BACK(x,y) 	  	x -= (y)					 /* takes the pointer 1 position back */
#define REWIND(x)  	 	x = lexeme;					 /* makes the pointer point to the beginning of lexeme */

typedef char LEXEME;

static char GetCharacter( char );
static void token_define( int, LEXEME* );

TOKEN token;		 				/* data structure for token */
static int   lineno; 	 				/* line number */
static bool  non_consuming;				/* flag for stating non-consuming states */

static const char special_symbol_table[256] = { 0,	/* given a symbol in the character stream, maps special symbols to their values */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	LPAREN, /* '('  position 40 */
	RPAREN, /* ')'  position 41 */
	MULT, 	/* '*'  position 42 */
 	PLUS, 	/* '+'  position 43 */
	COMMA,  /* ','  position 44 */
	MINUS, 	/* '-'  position 45 */
	0, 
	DIV, 	/* '/'  position 47 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	ASSIGN, /* ':'  position 58 */
	SEMI, 	/* ';'  position 59 */
	LT,	/* '<'  position 60 */
	EQ,	/* '='  position 61 */
	GT,	/* '>'  position 62 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	LSQR,	/* '['  position 91 */
	0, 
	RSQR, 	/* ']'  position 93 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	LCRLY,	/* '{'  position 123 */
	0,
	RCRLY, 	/* '}'  position 125 */	
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0  	/* position 255 */
	};

TOKEN gettoken()
{
	/* TOKEN RECOGNITION - 

		It's an implementation of the transitions diagrams.

		After identifying a token (good or bad), deletes the last character read from the lexeme (goes back one position 
		and inserts null-character). The character keeps queued in "character" variable for the next pass. 
	
	*/			
	short int    state = START; 			/* the current state */
	LEXEME 	     lexeme[MAXLEN_STRLIT] = { SNUL };  /* the lexeme read so far */
	LEXEME       *plexeme = NULL;			/* to navigate through lexeme */
	static       CHARACTER  character; 		/* contains the new character taken from the stream-of-characters  */  	
	
	REWIND( plexeme ); 				/* plexeme points to the beginning of the lexeme array */

	/* Transition diagrams */
	while(1)
	{			
		switch( state )
		{	
			case START: 

				/* reads character from stream and adds it to the lexeme read so far */
				character = GetCharacter( character );
				INSERT( plexeme, character );
	

				if( isdigit( character ) ){ 			/* is digit [0-9] */
					NEXT_STATE_IS( NUM );
				}
				else if( isalpha( character ) ){ 		/* is a letter [A-Za-z] */
					NEXT_STATE_IS( ID );
				}
				else if( IS_DOT( character ) ){ 		/* is dot [.] */ 
					NEXT_STATE_IS( NUM_1 );
								
				}
				else if( END_OF_FILE( character ) ){		/* is end-of-file */
					token_define( ENDFILE, "" );
					return token;
				}
				else if( IS_IGNORED( character ) )		/* is space or control characters */
				{	
					INSERT( plexeme, SNUL );					
					REWIND( plexeme );

					if( character == '\n' )		
						lineno++;	
				}
				else if( IS_SPECIAL_SYMBOL( character ) ){	/* is special symbol */
					NEXT_STATE_IS( special_symbol_table[ character ] );
				}	
				else{						/* is an unknown character */
					NEXT_STATE_IS( UNKNOW );
				}

				break;
			case UNKNOW:
					/* UNKNOW MATCHED */
					INSERT( plexeme, SNUL);
					token_define( UNKNOW, lexeme );
					return token;
				break;
			case ID:

				character = GetCharacter( character );
				INSERT( plexeme, character );
				
				
				if( isalnum( character ) || IS_UNDERSCORE( character ) ){
					NEXT_STATE_IS( ID );
				}		
				else /* other */
				{	
					/* IDENTIFIER MATCHED */	
 					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL);
					token_define( ID, lexeme );
					non_consuming = true;			
					return token; 						
				}
			
			        break;
			case LT:
				character = GetCharacter( character );
				INSERT( plexeme, character );

				if( IS_EQUAL( character ) ){
					/* LTEQ MATCHED */
					INSERT( plexeme, SNUL);	
					token_define( LTEQ, "" );
				}				
				else{	
					/* LT MATCHED */	
					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL);	
					token_define( LT, "" );
					non_consuming = true;
				}				
				
				return token;
 		
				break;
			case GT:
				character = GetCharacter( character );
				INSERT( plexeme, character );
			
				if( IS_EQUAL( character ) ){	
					/* GTEQ MATCHED */
					INSERT( plexeme, SNUL);	
					token_define( GTEQ, "" );
				}
				else{	
					/* GT MATCHED */	
					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL);						
					token_define( GT, "" );
					non_consuming = true;				
				}				
				
				return token;
							
				break;
			case PLUS:
				token_define( PLUS, "" );
				return token;

				break;
			case MINUS:
				character = GetCharacter( character );
				INSERT( plexeme, character );
			
				if( character == '-' ){	
					/* POSSIBLE WEIRD COMMENTS */
					INSERT( plexeme, SNUL);	
					token_define( GTEQ, "" );
					NEXT_STATE_IS( WEIRD_COMM );
				}
				else{
					/* MINUS MATCHED */	
					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL);						
					token_define( MINUS, "" );
					non_consuming = true;	
					return token;			
				}
							
				break;
			case WEIRD_COMM:
				character = GetCharacter( character );
				INSERT( plexeme, character );
				
				if( END_OF_FILE( character ) ){
					/* EOF */
					token_define( ENDFILE, "" );
					return token;
				}
				else if( IS_LINEFEED( character ) ){
					/* END OF LINE */
					INSERT( plexeme, SNUL);	
					REWIND( plexeme );
					NEXT_STATE_IS( START );
				}
				else{
					/* ignores */
					INSERT( plexeme, SNUL); /* it's important to rewind. otherwise comments cannot belonger than MAX_STRLEN */	
					REWIND( plexeme );					
				}

				break;			
			case MULT:
				token_define( MULT, "" );
				return token;	
				
				break;
			case EQ:
				token_define( EQ, "" );
				return token;
			
				break;
			case DIV:
				character = GetCharacter( character );
				INSERT( plexeme, character );

				if( IS_EQUAL( character ) ){	
					/* NEQ MATCHED */
					INSERT( plexeme, SNUL);	
					token_define( NEQ, "" );
					return token;
				}
				else if( IS_STAR( character ) ){
					/* COMMENTS BEGINNING */
					INSERT( plexeme, SNUL);	
					REWIND( plexeme );					
					NEXT_STATE_IS( COMM );	
				}
				else{	
					/* DIV MATCHED */	
					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL);	
					token_define( DIV, "" );					
					non_consuming = true;
					return token;
				}		

				break;
			case COMM:
				character = GetCharacter( character );
				INSERT( plexeme, character );


				if( END_OF_FILE( character ) ){
					non_consuming = true;
					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL);	
					token_define( ERROR_MISSING, " missing */");
					return token;
				}
				else if( IS_STAR( character ) ){				
					/* POSSIBLE COMMENT CLOSE */
					INSERT( plexeme, SNUL);	
					REWIND( plexeme );
					NEXT_STATE_IS( COMM_1 );
				}
				else if( character == '-' ){
					/* POSSIBLE NESTED COMMENT */
					INSERT( plexeme, SNUL);
					REWIND( plexeme );
					NEXT_STATE_IS( COMM_2 );
				}
				else{
					/* ignores characteres */
					INSERT( plexeme, SNUL); 	/* it's important to rewind. otherwise comment are limited to MAX_STRLIT size */
					REWIND( plexeme );
					NEXT_STATE_IS ( COMM );					
				}
				break;
			case COMM_1:
				character = GetCharacter( character );
				INSERT( plexeme, character );
			
				if( character == '/' ){
					/* COMMENT FINISHED */
					INSERT( plexeme, SNUL);
					REWIND( plexeme );
					NEXT_STATE_IS( START );
				}
				else{
					non_consuming = true;
					INSERT( plexeme, SNUL);
					REWIND( plexeme );
					NEXT_STATE_IS( COMM );
				}

				break;
			case COMM_2:
				character = GetCharacter( character );
				INSERT( plexeme, character );
				if( character == '-' ){
					/* NESTED COMMENT */
					INSERT( plexeme, SNUL);
					REWIND( plexeme );
					NEXT_STATE_IS( COMM_3 );
				}
				else{
					/* GO BACK */
					INSERT( plexeme, SNUL);
					REWIND( plexeme );
					NEXT_STATE_IS( COMM );
					
				}
				break;
			case COMM_3:
				character = GetCharacter( character );
				INSERT( plexeme, character );
				
				if( END_OF_FILE( character ) ){
					/* EOF */
					INSERT( plexeme, SNUL);
					REWIND( plexeme );
					non_consuming = true; 
					token_define( ERROR_MISSING, "missing */" );
					
					return token;
				}
				else if( IS_LINEFEED( character ) ){
					/* END OF LINE */
					INSERT( plexeme, SNUL);	
					REWIND( plexeme );
					NEXT_STATE_IS( COMM );
				}
				else{
					/* ignores */
					INSERT( plexeme, SNUL); 	/* it's important to rewind. otherwise comments cannot belonger than MAX_STRLEN */
					REWIND( plexeme );	
					NEXT_STATE_IS( COMM_3 );				
				}
		
				break;
			case ASSIGN:
				character = GetCharacter( character );
				INSERT( plexeme, character );

				if( IS_EQUAL( character ) ){	
					/* ASSIGN MATCHED */
					INSERT( plexeme, SNUL);	
					token_define( ASSIGN, "" );
				}
				else{
					/* ':' UNKNOW CHARACTER  MATCHED */	
					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL);	
					token_define( UNKNOW, lexeme );
					non_consuming = true;					
				}		
				return token;

				break;
			case LSQR:
				token_define( LSQR, "" );
				return token;
							
				break;
			
			case RSQR:
				token_define( RSQR, "" );
				return token;					
	
				break;
			case SEMI:
				token_define( SEMI, "" );
				return token;	
				
				break;
			case COMMA:
				token_define( COMMA, "" );
				return token;
	
				break;
			case LPAREN:
				token_define( LPAREN, "" );
				return token;	
				break;
			case RPAREN:
				token_define( RPAREN, "" );
				return token;
	
				break;
			case LCRLY:
				token_define( LCRLY, "" );
				return token;
	
				break;
			case RCRLY:
				token_define( RCRLY, "" );
				return token;
	
				break;
			case NUM:
				/* --  NUMERAL TD  -- */		
				character = GetCharacter( character );
				INSERT( plexeme, character );

				if( isdigit( character ) )
					NEXT_STATE_IS( NUM );
				else if( IS_DOT( character ) )
					NEXT_STATE_IS( NUM_1 );
				else if( character == 'E' || character == 'e' )
					NEXT_STATE_IS( NUM_3 );
				else{
					/* NUMERAL MATCHED */	
					if( isalpha(character) )
					{
						/* BAD TOKEN  "nnnx" (n = digit, x = non-number )*/
						token_define( BAD_NUM_K, lexeme );
						non_consuming = false;
					}
					else{
						BACK( plexeme, 1 );
						INSERT( plexeme, SNUL);
						token_define( NUM, lexeme );
						non_consuming = true;

					}
					return token; 
				}
				
				break;
			case NUM_1:
				character = GetCharacter( character );	
				INSERT( plexeme, character );	
	
				if( isdigit( character ) )
					NEXT_STATE_IS( NUM_2 );		
				else{
					/* BAD TOKEN  "n." */
					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL);
					token_define( BAD_NUM_1, lexeme );
					non_consuming  = true;
					return token; 						
				}	
			    
			    break;
			case NUM_2:
				character = GetCharacter( character );	
				INSERT( plexeme, character );	
	
				if( isdigit( character ) )
					NEXT_STATE_IS( NUM_2 ); 
				else if( character == 'E' || character == 'e')
					NEXT_STATE_IS( NUM_3 );				
				else{
					/* NUMERAL MATCHED */	
					if( isalpha(character) )
					{
						/* BAD TOKEN  "nnn.nnnx" */
						INSERT( plexeme, SNUL);
						non_consuming = false;
						token_define( BAD_NUM_K, lexeme );
					}
					else{
						BACK( plexeme, 1 );
						INSERT( plexeme, SNUL);
						non_consuming = true;
						token_define( NUM, lexeme );
					}
					return token; 
		         	}
			    break;
			case NUM_3:				
				character = GetCharacter( character );	
				INSERT( plexeme, character );

				if( character == '+' || character == '-' )
					NEXT_STATE_IS( NUM_4 );
				else if( isdigit( character ) )
					NEXT_STATE_IS( NUM_5 );
				else{
					/* BAD TOKEN  "n[.n]E" */
					non_consuming = true;
						
					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL);						
			
					token_define( BAD_NUM_K, lexeme );
					return token; 
				}				
			     break;	
			case NUM_4:
				character = GetCharacter( character );	
				INSERT ( plexeme, character );

				if ( isdigit( character ) )
					NEXT_STATE_IS( NUM_5 );
				else{
					/* BAD TOKEN n[.n]E[+|-] */ 
				
					non_consuming = true;
					BACK( plexeme, 1 );
					INSERT( plexeme, SNUL );
					
					token_define( BAD_NUM_4, lexeme );
					return token; 
				}
			    break;
			case NUM_5:
				character = GetCharacter( character );	
				INSERT ( plexeme, character );

				if( isdigit( character ) )
					NEXT_STATE_IS( NUM_5 ); 
				else{
					/* NUMERAL MATCHED */
					if( isalpha(character) )
					{
						/* BAD TOKEN  */
						INSERT( plexeme, SNUL );
						non_consuming = false;						
						token_define( BAD_NUM_K, lexeme );
					}
					else{
						BACK( plexeme, 1 );
						INSERT( plexeme, SNUL );
						non_consuming = true;	
						token_define( NUM, lexeme );
					}

					return token; 
				 }		
			    break;
			
		}//end switch
	
	}//end while

} 
	

char GetCharacter( char current_char )
{
	/*  
	    This function wraps the function given bu admin module, which gets a new character from the stream. Uses the non-consuming
	    flag to determine wether to get a new character from stream or not.

	    NOTE: If the pointer has no entry in the symbol table, then it's value is NULL. Thus, whenever a token is manipulated is important to
	    check if the pointer is NULL before trying to use it.
	*/

	char c;
	c = current_char;

	if( IS_FIRSTRUN() )
	{				/* fist character in stream */
		c = getcharacter(); 	
		lineno = 1; 	    	
	}
	else if( non_consuming ){
		non_consuming = false;  /* a character is already in queue */
	}
	else{   			
		c = getcharacter();	/* gets new chjaracter */	
	}
	
	return c;
}

void token_define(int name, LEXEME* lexeme)
{
	/* 
	   Defines the TOKEN data structure for each token matched.
	*/ 
		
	
	TABLE pword = NULL;	 	/* pointer to search through the word table */
	ST_TABLE psymbol = NULL;	/* pointer to manipulate the symbol table */
	static int counter;		/* index for new identifiers added to the symbol table */

	
	if( name == ID ) 
	{
		if( ( pword = wordtable_lookup( lexeme ) ) != NULL ){
			/* is a RESERVED WORD */	
			token.name = pword->token_name;	 			/* obtains token name from word table */
			
			if( token.name == BLIT ){
				/* its a BLIT */
				psymbol = symboltable_install( lexeme );	/* adds the lexeme to the symbol table */
				token.table_entry = psymbol;
			}
			else{
				token.table_entry = NULL;			/* no attribute */
			}
		}
		else{
			/* is an IDENTIFIER */
			token.name = name;

			if( ( psymbol = symboltable_lookup(lexeme) ) == NULL )
			{
				/* NEW identifier */
				psymbol = symboltable_install( lexeme );	/* adds the lexeme to the symbol table */
				
				psymbol->index = counter; 			/*it assigns a differnt index to every different identifier */ 
			
				if( add_spellingtable( lexeme, counter ) == false )
					printf( "_ERROR: space with index %d in spelling table is occupied (it should not happen!)_", counter ); 

				counter++;
			}
			else
			{	/* EXISTING identifier */
				psymbol = symboltable_install( lexeme );	
			}

			token.table_entry = psymbol;				/* copies the "pointer-to-symbol-table" to the token data strcuture*/	
			
		}			
	}
	else if( name == NUM || name == ERROR_MISSING  || name == UNKNOW || name == BAD_NUM_1 || name == BAD_NUM_3 || name == BAD_NUM_4 || 
		 name == BAD_NUM_K )
	{	/* is a NUMERAL, ERROR, UNKNOW or any BAD TOKENS */
		token.name = name;
		psymbol = symboltable_install( lexeme );
		token.table_entry = psymbol;
		
	}
	else
	{	/* all the remaining tokens with no attributes (; , < >, etc) */
		token.name = name;
		token.table_entry = NULL;
	}
	
	token.lineno = lineno;
	
	return;
}

