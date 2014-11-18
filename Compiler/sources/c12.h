
/* IMPORTANT: C12.h must be defined in all modules before any other header */

#ifndef _C12_H_
#define _C12_H_

#include <stdbool.h>
#include "symbol_table.h"


/* ------------------------------------------------------------------------------------------------- */
/* -----------------------------------  CONSTANT DEFINITIONS     ----------------------------------- */
/* ------------------------------------------------------------------------------------------------- */
		
#define SNUL	   	   	'\0'		/* null terminator */ 
#define MAX_NUM_OF_TOKENS 	5000		/* FIX: make it dynamic later */
#define MAX_LINLEN 	   	500		/* FIX: maximum number of character allowed in a line :499 */
#define	MAXLEN_STRLIT	   	509		/* FIX: maximum length of string literals in c12 */	 
#define NUM_OF_RESERVED_WORDS	37		/* number of reserved words in the language */
#define FILE_FLAG		38
#define	STDOUT_FLAG	 	39
#define COMPLETE_FLAG		40
#define MATCH_FLAG		41
#define PARSER_ERR		42
#define SCANNER_ERR		43
#define SEMANALYZER_ERR		44

/* defines used for printing ast labels, tokens, and nonterminals names. 
   They are used together with an enumeration defining the numerical values, and an array containing the 
   string value */ 

#define NUM_OF_TOKENS		58		/* number of elements in "tokens" array in module tracer */
#define NUM_OF_NONTERMINALS	49		/* number of elements in "non_terminals" array in module tracer */
#define NUM_OF_NODE_LABELS	65		/* number of elements in "node_labels" array in module ast */
#define NUM_OF_TYPES		9		/* number of elements in "types" array in module ast */

#define TOKEN_OFFSET		100	/* offset used for printing the numerical values of tokens defined in enumaration "tokens"  */
#define NONTERMINAL_OFFSET	200     /* offset used for printing the numerical values of tokens defined in enumaration "non_terminals"  */
#define NODE_LABELS_OFFSET	300	/* offset used for printing the numerical values of tokens defined in enumaration "node_labels"  */	
#define TYPES_OFFSET		400   	/* offset used for printing the numerical values of tokens defined in enumaration "types"  */

/* ------------------------------------------------------------------------------------------------- */
/* ---------------------------------- ENUMERATIONS AND TYPEDEF's     ------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

typedef int TYPE;		/* type attribute */
typedef int OP;			/* operator attribute */
typedef int terminal;


/* adding a new element to any of theses enums requieres:
	1. add the element to the enum -obviously.
	2. increment the corresponding constant (NUM_OF_NONTERMINALS, NUM_OF_TOKENS or NUM_OF_NODE_LABELS) at c12.h
	3. add the text to the corresponding array (node_labels[] in ast.c, non_terminals[] in tracer.c, and tokens[] in tracer.c 
*/

/* types */
enum types{ TYPE_INT = -400, TYPE_BOOL, TYPE_VOID, TYPE_REF_INT, TYPE_REF_BOOL, NO_TYPE, TYPE_UNIVERSAL, TYPE_EPSILON, TYPE_ERROR };



/* token names */
enum tokens{ RCRLY = -100, LCRLY, RSQR, LSQR, RPAREN, LPAREN, COMMA, SEMI, ASSIGN, NEQ, EQ,  //11
	     GTEQ, GT, LTEQ, LT, DIV, MULT, MINUS, PLUS, VOID, RETURN, REF, ORELSE, OR, NOT, //25
	     MOD, LOOP, INT, IF, EXIT, END, ELSE, CONTINUE, BOOL, ANDTHEN, AND, BLIT, UNKNOW, //38
	     ID, NUM, ENDFILE, EPSILON, ARRAY, MULTIVAR, SINGLEVAR, VAR_DEC_TAIL_P, VAR_NAME_P, MULTIPARAMS, REF_INT, REF_BOOL, ARRAY_ASSIGN, //51
	     ERROR_MISSING, BAD_NUM_1, BAD_NUM_3, BAD_NUM_4, BAD_NUM_K, PARSE_ERROR //57
};					

/* non_terminals names */
enum non_terminals{  PROGRAM = -200, DECLARATION, NONVOID_SPECIFIER, DEC_TAIL, VAR_DEC_TAIL, VAR_NAME, FUN_DEC_TAIL, PARAMS, PARAM, STATEMENT, ID_STMT, 
		     ID_STMT_TAIL, ASSIGN_STMT_TAIL, CALL_STMT_TAIL, CALL_TAIL, ARGUMENTS, COMPOUND_STMT, IF_STMT, LOOP_STMT, EXIT_STMT, CONTINUE_STMT, 
		     RETURN_STMT, NULL_STMT, EXPRESSION, ADD_EXP, TERM, FACTOR, NID_FACTOR, ID_FACTOR, ID_TAIL, VAR_TAIL, RELOP, ADDOP, MULTOP, UMINUS,
		     MATCH, ERR, ROOT, ID_CONST, ARGUMENTS_P, COMPOUND_STMT_PP, COMPOUND_STMT_PPP, LOOP_STMT_P, ASSIGN_STMT, ROUTINE_CALL, KG }; 	//

/* labels names for the ast */
enum node_labels{ LBL_EPSILON = -300, LBL_PROGRAM, LBL_VOID, LBL_INT, LBL_BOOL, LBL_ID, LBL_ARRAY, LBL_MULTIVAR, LBL_VAR_NAME,
		 LBL_FUN_DEC_TAIL, LBL_PARAMS, LBL_MULTIPARAM, LBL_REF_BOOL, LBL_REF_INT, LBL_ASSIGN, LBL_ARRAY_ASSIGN, LBL_ROUTINE_CALL,
		 LBL_COMPOUND_STMT, LBL_COMPOUND_STMT_P, 
		 LBL_COMPOUND_STMT_PP, LBL_COMPOUND_STMT_PPP, LBL_IF_STMT, LBL_LOOP_STMT, LBL_LOOP_STMT_P, LBL_EXIT, LBL_CONTINUE, LBL_RETURN, 
		 LBL_NUM, LBL_BLIT, LBL_NEQ, LBL_EQ, LBL_GTEQ, LBL_GT, LBL_LTEQ, LBL_LT, LBL_DIV, 
		 LBL_MULT, LBL_MINUS, LBL_PLUS, LBL_ORELSE, LBL_OR, LBL_NOT, LBL_MOD, LBL_AND, LBL_ANDTHEN, 
		 LBL_UMINUS, LBL_MULTIPARAMS, LBL_ARGUMENTS_P, LBL_NULL, LBL_DECLARATION, LBL_PARAM, LBL_COMPOUND_DECLARATION, LBL_PLUS_MAIN,
		 LBL_ARRAY_OR_CALL, LBL_NO_ARGUMENTS, LBL_MULT_MAIN, LBL_MINUS_MAIN, LBL_OR_MAIN, LBL_ORELSE_MAIN, LBL_DIV_MAIN, LBL_MOD_MAIN, 
		 LBL_AND_MAIN, LBL_ANDTHEN_MAIN, LBL_ERROR
		};
		

/* ------------------------------------------------------------------------------------------------- */
/* ------------------------------------  VARIABLES DECLARATION-------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

/* TOKEN data structure */
typedef struct ordered_pair{
	int  name;				/* token name */
	long int lineno;			/* line number */
	ST_TABLE   table_entry;			/* pointer to table entry*/
}TOKEN;


#endif










