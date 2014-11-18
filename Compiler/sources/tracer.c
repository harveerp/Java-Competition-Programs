/*
 * tracer.c
 * 
 * 
 *
 * 
 * TRACER MODULE:
 * 	It traces the parser. The path is builded each time the function parse_trace is called by each nonterminal function in the parser module. 
 *	The printing options can be changed by changing the parameters when calling the function init_trace(). It can also be disabled if commented out.
 * 	The parameter are:
 *
 *	STDOUT_FLAG | FILE_FLAG		--> It prints the output on standar output, or a file specified by the variable "tracer_ofile"
 *	MATCH_FLAG  | COMPLETE_FLAG	--> It printf only matched tokens, or the complete trace, that is, including nonterminals constructs.
 *
 * DEPENDENCIES:
 *	tracer.h c12.h
 *
 *   	
 */

#include <stdio.h>
#include "tracer.h"
#include "c12.h"

/* ------------------------------------------------------------------------------------------------- */
/* ----------------------------------------  VARIABLES     ----------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

			
char *non_terminals[NUM_OF_NONTERMINALS] = { 	/* contains the text form of all non_terminals... for printing purposes */
			    "program", "declaration", "nonvoid-specifier", "dec-tail", "var-dec-tail", "var-name", 
			    "fun-dec-tail", "params", "param", "statement", "id-stmt", "id-stmt-tail", "assign-stmt-tail", 
			    "call-stmt-tail", "call-tail", "arguments", "compound-stmt", "if-stmt", "loop-stmt", 
			    "exit-stmt", "continue-stmt", "return-stmt", "null-stmt", "expression", "add-exp", "term", 
			    "factor", "nid-factor", "id-factor", "id-tail", "var-tail", "relop", 
			    "addop", "multop", "uminus", "[matched]", "!!ERROR", "ROOT", 
			    "id_const", "arguments_p", "compound_stmt''", "compound_stmt'''","loop_stmt'", "assign_stmt", 
			    "routine_call", "KG" }; /* for easy counting there are 6 elements in each line */


char *tokens[NUM_OF_TOKENS] = { 		/* contains the text form of all tokens... for printing purposes */
		     "}", "{", "]", "[", ")", "(", ",", ";", ":=", "/=", "=", //11
		     ">=", ">", "<=", "<", "/", "*", "-", "+", "void", "return", "ref", //22
		     "orelse", "or", "not", "mod", "loop", "int", "if", "exit", "end", "else", "continue", //33 
		     "bool", "andthen", "and", "blit", "unknown token", "id", "num", "endfile", "e", "array", "multivar", //44
 		     "singlevar", "var_dec_tail'", "var_name'", "multiparams", "ref_int", "ref_bool", "array_asign", //51
		     "missing */", "invalid numeral", "invalid numeral", "invalid numeral", "invalid numeral", "syntactic error" };

char tracer_ofile[18] = "tracer_output.txt"; 	/* name of the output file */
static FILE *file;					
static int flag1;
static int flag2;
static bool initialized;			/* indicates if the tracer has been already initialized */


/* ------------------------------------------------------------------------------------------------- */
/* -----------------------------------------  FUNCTIONS     ---------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

void parse_trace( int non_terminal, int token ){
	/*
		After initializated, prints in the specified file, the pair ( non terminal, token ). 
	*/
	
	if( initialized ){
		if( flag2 == MATCH_FLAG ){
			if( non_terminal == MATCH ){
				if( flag1 == FILE_FLAG )
					fprintf( file, "%s, %s \n", non_terminals[ non_terminal + NONTERMINAL_OFFSET ], tokens[ token + TOKEN_OFFSET ]);
				else
					printf( "%s, %s \n", non_terminals[ non_terminal + NONTERMINAL_OFFSET ], tokens[ token + TOKEN_OFFSET ] ); 
			}
		}
		else{
			if( flag1 == FILE_FLAG )
				fprintf( file, "%s, %s \n", non_terminals[ non_terminal + NONTERMINAL_OFFSET ], tokens[ token + TOKEN_OFFSET ] );
			else
				printf( "%s, %s \n", non_terminals[ non_terminal + NONTERMINAL_OFFSET ], tokens[ token + TOKEN_OFFSET ] );
		} 
	}
	fflush( file );
}

void tracer_init( int f1, int f2 ){
	/*
		Initializes the tracer by opening the file specified in tracer_ofile, and printing the header in the same output file.
	*/
	flag1 = f1;
	flag2 = f2;
	initialized = true;

	if( flag1 == FILE_FLAG ){ 		/* flag for writing on file */
		file = fopen( tracer_ofile, "w" );
		fprintf( file, "Parsing tracer: \n\n------------------\nnonterminal, lookahead\n------------------\n\n" );
	}
	else if( flag1 == STDOUT_FLAG ){ 	/*flag for writing on stdout */
		printf( "Parsing tracer: \n\n------------------\nnonterminal, lookahead\n------------------\n\n" );

	}
	else{
		fprintf( stdout, "Unknow flag ( FILE_FLAG or STDOUT_FLAG expected )" );
	}
}
