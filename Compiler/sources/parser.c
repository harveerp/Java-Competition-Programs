/*
 * parser.c
 * 
 * 
 *
 * 
 * PARSER MODULE:
 * 	recognizes the syntactic structure of a syntactically correct program. It uses ad hoc implementation of FIRST sets.
 *  	It does not perform any kind of error recovery technique
 *
 * DEPENDENCIES:
 *	parser.c parser.h c12.h scanner.o ast.o tracer.o
 *
 *   	
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "c12.h"
#include "parser.h"
#include "scanner.h"
#include "tracer.h"
#include "ast.h"
#include "error_reporting.h"

#define DEFERRED -657
		

char *create_err_msg( char* );
TOKEN match( terminal );
astnode program();
astnode program_p();	
astnode declaration();
TYPE nonvoid_specifier();
astnode id_const();
astnode dec_tail();
astnode var_dec_tail();
astnode var_dec_tail_p();
astnode var_name();
astnode var_name_p();
astnode fun_dec_tail();
astnode params();
astnode params_p();
astnode param();
astnode param_p();
astnode statement();
astnode id_stmt();
astnode id_stmt_tail();
astnode assign_stmt_tail();
astnode call_stmt_tail();
astnode call_tail();
astnode call_tail_p();
astnode arguments();
astnode arguments_p();
astnode compound_stmt();
astnode compound_stmt_p();
astnode compound_stmt_pp();
astnode compound_stmt_ppp();
astnode if_stmt();
astnode if_stmt_p();
astnode loop_stmt();
astnode loop_stmt_p();
astnode exit_stmt();
astnode continue_stmt();
astnode return_stmt();
astnode return_stmt_p();
astnode null_stmt();
astnode expression();
astnode expression_p();
astnode add_exp();
astnode add_exp_p();
astnode term();
astnode term_p();
astnode factor();
astnode nid_factor();
astnode id_factor();
astnode id_tail();
astnode var_tail();
OP relop();
OP addop();
OP multop();
OP uminus();

static TOKEN lookahead;			/* the lookahead */
extern astnode ast_root;		/* The actual syntax tree. It's declared on ast.c*/
extern char *tokens[NUM_OF_TOKENS];

static int inh_type = NO_TYPE;			/* used to pass type as inheritted attributes */

 
TOKEN match(terminal t){
	TOKEN last_tkn;	
	/* copies current token to last token data structure*/
	last_tkn.name = lookahead.name;		
	last_tkn.table_entry = lookahead.table_entry;	

	
	if( lookahead.name == t ){
		parse_trace( MATCH, lookahead.name );
		lookahead = gettoken();	


		// Additional error reporting of for lexical errors
		if( lookahead.name == ERROR_MISSING ){
			error_list_add( "missing '*/'", lookahead.lineno );
			// passes end of file to finish parsing, because everything after the unclosed /* was ignored by the scanner
			lookahead.name = ENDFILE;
			return last_tkn;
		}	
		else if( lookahead.name == UNKNOW ){
			//reports the unknown token and gets the next one in stream
			char *s = ( char* )calloc( strlen("unknown token '' found") + strlen( lookahead.table_entry->lexeme) + 1, sizeof(char) );	
			sprintf( s, "unknown token '%s' found", lookahead.table_entry->lexeme );	
			error_list_add( s , lookahead.lineno );	
			lookahead = gettoken();	
		}
		else if( lookahead.name == BAD_NUM_1 || lookahead.name == BAD_NUM_3 || 
	    	     lookahead.name == BAD_NUM_4 || lookahead.name == BAD_NUM_K ){
			//corrupted numerals... all of them predefined in the transition diagrams, reports and gets the next token in stream	
			char *s = ( char* )calloc( strlen("invalid numeral ''") + strlen( lookahead.table_entry->lexeme ) + 1, sizeof(char) );	
			sprintf( s, "invalid numeral '%s'", lookahead.table_entry->lexeme );			
			error_list_add( s, lookahead.lineno );
			lookahead = gettoken();		
		}
		//end of lexical error check

	}
	else{
		/* syntax error */
		parse_trace( PARSE_ERROR, lookahead.name );
		last_tkn.name = PARSE_ERROR;
		last_tkn.table_entry = NULL;
		
	}
	return last_tkn; /* returns the last token, that is, the token before getting the new one */
}

void parse(){
	tracer_init( FILE_FLAG, COMPLETE_FLAG );/* comment to turn tracer off
						   FILE_FLAG | STDOUT_FLAG  toggles the output of the tracer 
						   MATCH_FLAG | COMPLETE_FLAG  toggles wheter all the trace should be printed, or only the
						   matched tokens 
						*/

	/* gets the first token */
	lookahead = gettoken();	

	/* links the ast root declared on ast.c to the subtree generated during parsing... parsing starts here */
	ast_root = maketree( LBL_PROGRAM, program(), NULL, NULL, NULL, lookahead.lineno );
	
	/* print tree in file specified in ast.c*/
	printtree();			

}


astnode program(){
	parse_trace( PROGRAM, lookahead.name );
	astnode pleft = NULL;
	astnode pcenter = NULL;
	int node_name;
	

	if( lookahead.name == VOID || lookahead.name == INT || lookahead.name == BOOL ){			
		pleft = declaration(); 
		pcenter = program();
		node_name = LBL_PROGRAM;	
	}
	else if( lookahead.name == ENDFILE ){
		/* endfile */
		node_name = LBL_EPSILON;
	}	
	else{
		/* syntactic error */
		error_list_add( create_err_msg( "'void', 'int', 'bool' expected" ), lookahead.lineno );
		node_name = LBL_ERROR;
	}
	
	return maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno ) ;
}


astnode declaration(){
	astnode pnode;
	parse_trace( DECLARATION, lookahead.name );
	TYPE type;
	astnode pleft = NULL;
	astnode pcenter = NULL;


	if( lookahead.name == VOID ){
		type = TYPE_VOID;
		match( VOID );  inh_type = type; pleft = id_const(); pcenter = fun_dec_tail();	
	}
	else if( lookahead.name == INT || lookahead.name == BOOL ){ 
		type = nonvoid_specifier(); inh_type = type; pleft = id_const();  inh_type = type; pcenter = dec_tail();
	}
	else{
		/* syntax error */ /* this never happens anyway -_- */
		error_list_add( create_err_msg( "'void', 'int' or 'bool' expected" ), lookahead.lineno ); 
		pnode = maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		return pnode;
	}

	pnode = maketree( LBL_DECLARATION, pleft, pcenter, NULL, NULL, lookahead.lineno );
	settype_node( pnode, type );

	return pnode;
}

TYPE nonvoid_specifier(){
	parse_trace( NONVOID_SPECIFIER, lookahead.name );	
	
	if( lookahead.name == INT ){
		match( INT );
		return TYPE_INT;
	}
	else if( lookahead.name == BOOL ){
		match( BOOL );
		return TYPE_BOOL;
	}
	else{
		/* syntax error - the program flow will never get here anyway*/
		error_list_add( create_err_msg( " 'int' or 'bool' expected" ), lookahead.lineno );
	}
	return NO_TYPE;
}

astnode id_const(){
	parse_trace( ID_CONST, lookahead.name );
	astleaf pleaf;
	TOKEN auxtkn;

	auxtkn = match( ID );

	if( auxtkn.name == PARSE_ERROR ){
		/* syntax error */
		error_list_add( create_err_msg( " 'identifier' expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	pleaf = makeleaf( LBL_ID, auxtkn.table_entry ); /* if this is not really an ID entry, printree will generate segfault */
	
	settype_leaf( pleaf, inh_type ); 
	inh_type = NO_TYPE; 				/* equivalent to popping hehe */
	
	return maketree( LBL_ID, NULL, NULL, NULL, pleaf, lookahead.lineno );
}

astnode dec_tail(){
	parse_trace( DEC_TAIL, lookahead.name );
	astnode pleft = NULL;


	if( lookahead.name == LSQR || lookahead.name == COMMA || lookahead.name == SEMI ){
		pleft = var_dec_tail();
	}
	else if( lookahead.name == LPAREN ){
		pleft = fun_dec_tail();
	}
	else{
		/* syntax error */
		error_list_add( create_err_msg( "'[', ',' or ';' expected" ), lookahead.lineno );
		pleft = maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}	

	return pleft;
}
astnode var_dec_tail(){
	parse_trace( VAR_DEC_TAIL, lookahead.name );
	TYPE type;
	int node_name; 
	astnode pleft = NULL;
	astnode pcenter = NULL;
	TOKEN auxtkn;
	
	type = inh_type; /* it's like a backup. this is necesary as inh value can be changed by any subtree constructed prior to calling the intended 					function for wich
			    we are passing this value to. In other words, other functions might need pass values to. */

	if( lookahead.name == LSQR ){		
		match( LSQR ); pleft = add_exp(); 

		auxtkn = match( RSQR ); 
		if( auxtkn.name == PARSE_ERROR ){
			/* syntactic error */
			error_list_add( create_err_msg( " ']' expected" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}		

		inh_type = type; pcenter = var_dec_tail_p();
		node_name = LBL_ARRAY; 
	}
	else{ 	/* implementes this way because FIRST(var-dec-tail') includes epsilon */
		pleft = var_dec_tail_p();
		node_name =  DEFERRED;	/*no node is created here...
					 done this way because semi is still to be matched
					 cannot leave the routine yet */
	}
	
	
	auxtkn = match( SEMI );
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "';' expected in variable declaration" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}
	
	if( node_name == DEFERRED )
		return pleft; 
	else
		return maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno );
}



astnode var_dec_tail_p(){
	TYPE type;
	parse_trace( VAR_DEC_TAIL, lookahead.name );
	int node_name;
	astnode pleft = NULL;	
	astnode pcenter = NULL;
	TOKEN auxtkn;
	
	type = inh_type;

	if( lookahead.name == COMMA ){
		match( COMMA );
		inh_type = type; pleft = var_name(); inh_type = type; pcenter = var_dec_tail_p();	
		node_name = LBL_MULTIVAR;
	}
	else{  /*epsilon */
		node_name = LBL_EPSILON;
	}
	return maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
	
astnode var_name(){
	parse_trace( VAR_NAME, lookahead.name );	
	astnode pleft = NULL;
	astnode pcenter = NULL;

	pleft = id_const(); pcenter = var_name_p();	

	return maketree( LBL_VAR_NAME, pleft, pcenter, NULL, NULL, lookahead.lineno );
}

astnode var_name_p(){
	parse_trace( VAR_NAME, lookahead.name );
	int node_name;
	astnode pleft = NULL;
	TOKEN auxtkn;

	if( lookahead.name == LSQR ){
		match( LSQR ); pleft = add_exp(); 

		auxtkn = match( RSQR );
		if( auxtkn.name == PARSE_ERROR ){
			/* syntactic error */
			error_list_add( create_err_msg( "']' expected" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}
	
		node_name = LBL_ARRAY;
	}
	else{/* e */	
		node_name = LBL_EPSILON;
	}
	
	return maketree( node_name, pleft, NULL, NULL, NULL, lookahead.lineno );
}

astnode fun_dec_tail(){
	parse_trace( FUN_DEC_TAIL, lookahead.name );
	astnode pleft = NULL;
	astnode pcenter  = NULL;
	TOKEN auxtkn;

	auxtkn = match( LPAREN ); 
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "'(' expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}	
	
	pleft = params(); 

	int x;

	match( RPAREN ); 
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "')' expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	pcenter = compound_stmt();

	return maketree( LBL_FUN_DEC_TAIL, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
astnode params(){
	TYPE type;
	int node_name;
	astnode pnode;
	parse_trace( PARAMS, lookahead.name );
	astnode pleft = NULL;
	astnode pcenter = NULL;

	if( lookahead.name == REF || lookahead.name == INT || lookahead.name == BOOL ){
		pleft = param(); inh_type = pleft->type; pcenter = params_p();	
		type = NO_TYPE;
		node_name = LBL_PARAMS;
	}
	else if( lookahead.name == VOID ){
		match( VOID );
		type = TYPE_VOID;
		node_name = LBL_VOID;
	}
	else{
		/* syntax error */
		error_list_add( create_err_msg( "'ref', 'int', 'bool' or 'void' expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );

	}
	
	pnode = maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno );
	settype_node( pnode, type );	

	return pnode;
}

astnode params_p(){
	parse_trace( PARAMS, lookahead.name );
	astnode pleft = NULL;
	astnode pcenter = NULL;
	int node_name;

	if( lookahead.name == COMMA ){
		match( COMMA ); pleft = param(); pcenter = params_p();	
		node_name = LBL_MULTIPARAMS;
	}
	else{
		node_name = LBL_EPSILON;	
	}
	return maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno );
}

astnode param(){
	astnode pnode;
	parse_trace( PARAM, lookahead.name );
	TYPE type;
	astnode pleft = NULL;
	astnode pcenter = NULL;

	if( lookahead.name ==  REF ){
		match( REF ); type = nonvoid_specifier(); 	
		if( type == TYPE_BOOL ) type = TYPE_REF_BOOL;
		else type = TYPE_REF_INT;

		inh_type = type; pleft = id_const(); pcenter = param_p();
	}
	else if(  lookahead.name == INT || lookahead.name == BOOL ){
		type = nonvoid_specifier(); inh_type = type; pleft = id_const();  pcenter = param_p();	
	}			
	else{
		/* syntactic error... it does not happen */
		error_list_add( create_err_msg( "'ref', 'int' or 'bool' expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}
	
	pnode = maketree( LBL_PARAM, pleft, pcenter, NULL, NULL, lookahead.lineno );
	settype_node( pnode, pleft->type );

	return pnode;
}

astnode param_p(){
	parse_trace( PARAM, lookahead.name );
	int node_name;
	TOKEN auxtkn;	

	if( lookahead.name ==  LSQR ){
		match( LSQR ); 

		auxtkn = match( RSQR );
		if( auxtkn.name == PARSE_ERROR ){
			/* syntactic error... it does not happen */
			error_list_add( create_err_msg( "')' expected" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}

		node_name = LBL_ARRAY;
	}
	else{
		/*epsilon*/ 
		node_name = LBL_EPSILON;
	}
	
	return maketree( node_name, NULL, NULL, NULL, NULL, lookahead.lineno );
}

astnode statement(){
	parse_trace( STATEMENT, lookahead.name );
	astnode pleft = NULL;

	if( lookahead.name == ID )
		pleft = id_stmt();
	else if( lookahead.name == LCRLY )
		pleft = compound_stmt();
	else if( lookahead.name == IF )
		pleft = if_stmt();
	else if( lookahead.name == LOOP )
		pleft = loop_stmt();
	else if( lookahead.name == EXIT )
		pleft = exit_stmt();
	else if( lookahead.name == CONTINUE )
		pleft = continue_stmt();
	else if( lookahead.name == RETURN )
		pleft = return_stmt();
	else if( lookahead.name == SEMI )
		pleft = null_stmt();
	else if( lookahead.name != RCRLY ){
		/* syntax error */
		error_list_add( create_err_msg( "'statement' expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );	
	}
	return pleft;
}
astnode id_stmt(){
	parse_trace( ID_STMT, lookahead.name );
	int node_name;
	astnode pleft = NULL;
	astnode pcenter = NULL;
	astnode pright = NULL;
	astnode ptemp = NULL;
	
	inh_type = NO_TYPE;

	pleft = id_const();
	ptemp = id_stmt_tail();	
		
	/* pcenter is builded */
	node_name = op( ptemp );
	pcenter = left( ptemp );
	pright = center( ptemp );
	free( ptemp );

	return maketree( node_name, pleft, pcenter, pright, NULL, lookahead.lineno );
}
astnode id_stmt_tail(){
	parse_trace( ID_STMT_TAIL, lookahead.name );
	astnode pleft = NULL;

	if( lookahead.name == LSQR || lookahead.name == ASSIGN ){
		pleft = assign_stmt_tail();
	}
	else if( lookahead.name == LPAREN ){
		pleft = call_stmt_tail();	
	}
	else{
		/* syntactic error */
		error_list_add( create_err_msg( "'[', ':=' or '(' expected in identifier statement" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}	
	return pleft;
}
astnode assign_stmt_tail(){
	parse_trace( ASSIGN_STMT_TAIL, lookahead.name );
	int node_name;
	astnode pleft = NULL;
	astnode pcenter = NULL;	
	TOKEN auxtkn;

	if( lookahead.name == LSQR ){
		match( LSQR ); pleft = add_exp(); 

		auxtkn = match( RSQR ); 
		if( auxtkn.name == PARSE_ERROR ){
			/* syntactic error */
			error_list_add( create_err_msg( "']' expected in assign statement" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}

		auxtkn = match( ASSIGN ); 
		if( auxtkn.name == PARSE_ERROR ){
			/* syntactic error */
			error_list_add( create_err_msg( "':=' expected in assign statement" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}

		pcenter = expression();  

		auxtkn = match( SEMI );	
		if( auxtkn.name == PARSE_ERROR ){
			/* syntactic error */
			error_list_add( create_err_msg( "';' expected in assign statement" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}		

		node_name = LBL_ARRAY_ASSIGN;
	}
	else if( lookahead.name ==  ASSIGN ){
		match( ASSIGN ); pleft = expression(); 

		auxtkn = match( SEMI );
		if( auxtkn.name == PARSE_ERROR ){
			/* syntactic error */
			error_list_add( create_err_msg( "';' expected in assign statement" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}

	
		node_name = LBL_ASSIGN;
	}
	else{
		/* syntactic error */
		error_list_add( create_err_msg( "':=' or '[' expected in assign statement" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}
	return maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
astnode call_stmt_tail(){
	parse_trace( CALL_STMT_TAIL, lookahead.name );
	astnode pleft = NULL;
	TOKEN auxtkn;

	pleft = call_tail(); 

	auxtkn = match( SEMI );
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "';' expected in call statement" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	return pleft;
}
astnode call_tail(){
	parse_trace( CALL_TAIL, lookahead.name );
	astnode pleft = NULL;
	TOKEN auxtkn;	

	auxtkn = match( LPAREN ); 
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "'(' expected in call statement" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	pleft = call_tail_p(); 

	auxtkn = match( RPAREN );	
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "')' expected in call statement " ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}


	return pleft;
}

astnode call_tail_p(){
	parse_trace( CALL_TAIL, lookahead.name );
	astnode pleft = NULL;

	if( lookahead.name == MINUS || lookahead.name == NOT || lookahead.name == ID || 
	    lookahead.name == NUM || lookahead.name == BLIT || lookahead.name == LPAREN ){
		pleft = arguments();	
	}
	else{ /*epsilon */
		pleft = maketree( LBL_NO_ARGUMENTS, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	return pleft;
}
astnode arguments(){
	parse_trace( ARGUMENTS, lookahead.name );
	astnode pleft = NULL;
	astnode pcenter = NULL;

	pleft = expression(); pcenter = arguments_p();	

	return maketree( LBL_ROUTINE_CALL, pleft, pcenter, NULL, NULL, lookahead.lineno );
}

astnode arguments_p(){
	parse_trace( ARGUMENTS, lookahead.name );
	int node_name;
	astnode pleft = NULL;
	astnode pcenter = NULL;

	if( lookahead.name == COMMA ){
		match( COMMA ); pleft = expression(); pcenter = arguments_p();	
		node_name = LBL_ARGUMENTS_P;
	}
	else{ /*epsilon */
		node_name = LBL_EPSILON;
	}
	return maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
astnode compound_stmt(){
	parse_trace( COMPOUND_STMT, lookahead.name ); 
	astnode pleft = NULL;
	astnode pcenter = NULL;
	TOKEN auxtkn;	

	auxtkn = match( LCRLY );
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "'{' expected") , lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	pleft = compound_stmt_p();
	pcenter = compound_stmt_pp();	

	auxtkn = match( RCRLY );
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "'}' expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	return maketree( LBL_COMPOUND_STMT, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
astnode compound_stmt_p(){
	parse_trace( COMPOUND_STMT, lookahead.name ); 
	int node_name;
	TYPE type;
	astnode pleft = NULL;
	astnode pcenter = NULL;
	astnode pright = NULL;


	if( lookahead.name == INT || lookahead.name == BOOL ){
		type = nonvoid_specifier(); inh_type = type; pleft = id_const(); inh_type = type; pcenter = var_dec_tail(); pright = compound_stmt_p();	
		node_name = LBL_COMPOUND_DECLARATION;
	}
	else{ /*epsilon*/
		node_name = LBL_EPSILON;
	}
	return maketree( node_name, pleft, pcenter, pright, NULL, lookahead.lineno );
}
astnode compound_stmt_pp(){
	parse_trace( COMPOUND_STMT, lookahead.name ); 
	astnode pleft = NULL;
	astnode pcenter = NULL;	
	
	
	pleft = statement();
	pcenter = compound_stmt_ppp();

	return maketree( LBL_COMPOUND_STMT_PP, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
astnode compound_stmt_ppp(){
	parse_trace( COMPOUND_STMT, lookahead.name ); 
	int node_name;
	astnode pleft = NULL;
	astnode pcenter = NULL;	

	if( lookahead.name == ID || lookahead.name == LCRLY || lookahead.name == IF || 
	       lookahead.name == LOOP || lookahead.name == EXIT || lookahead.name == CONTINUE || 
	       lookahead.name == RETURN || lookahead.name == SEMI ){

		pleft = statement(); 
		pcenter = compound_stmt_ppp();	
		node_name = LBL_COMPOUND_STMT_PPP;
	}
	else{/* e */
		node_name = LBL_EPSILON;
	}
	return maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
astnode if_stmt(){
	parse_trace( IF_STMT, lookahead.name );
	astnode pleft = NULL;
	astnode pcenter = NULL;
	astnode pright = NULL;
	TOKEN auxtkn;

	auxtkn = match( IF ); 

	auxtkn = match( LPAREN ); 
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "'(' missing in if statement" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	pleft = expression(); 

	auxtkn = match( RPAREN ); 
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "')' expected in if statement" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	pcenter = statement(); pright = if_stmt_p();
	
	return maketree( LBL_IF_STMT, pleft, pcenter, pright, NULL, lookahead.lineno );
}

astnode if_stmt_p(){
	parse_trace( IF_STMT, lookahead.name );
	astnode pleft;

	if( lookahead.name == ELSE ){
		match( ELSE ); pleft = statement();	
	}
	else{/* epsilon */
		pleft = maketree( LBL_EPSILON, NULL, NULL, NULL, NULL, lookahead.lineno );
		
	}
	return pleft;
}
astnode loop_stmt(){
	parse_trace( LOOP_STMT, lookahead.name );
	astnode pleft = NULL;
	astnode pcenter = NULL;
	TOKEN auxtkn;

	match( LOOP ); pleft = statement(); pcenter = loop_stmt_p(); 

	auxtkn = match( END ); 
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "'end' token expected in loop statement" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	auxtkn = match( SEMI );
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "';' expected after 'end' token and" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}	

	return maketree( LBL_LOOP_STMT, pleft, pcenter, NULL, NULL, lookahead.lineno );
}

astnode loop_stmt_p(){
	parse_trace( LOOP_STMT, lookahead.name );
	int node_name;	
	astnode pleft =  NULL;
	astnode pcenter = NULL;

	if( lookahead.name == ID || lookahead.name == LCRLY || lookahead.name == IF || 
	       lookahead.name == LOOP || lookahead.name == EXIT || lookahead.name == CONTINUE || 
	       lookahead.name == RETURN || lookahead.name == SEMI ){

		pleft = statement(); pcenter = loop_stmt_p();	
		node_name = LBL_LOOP_STMT_P;
	}
	else{ /*epsilon */
		node_name = LBL_EPSILON;
	}

	return maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
astnode exit_stmt(){
	parse_trace( EXIT_STMT, lookahead.name );
	astnode pleft = NULL;
	TOKEN auxtkn;

	match( EXIT ); 

	auxtkn = match( SEMI );
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "';' expected after exit statement and" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	return maketree( LBL_EXIT, NULL, NULL, NULL, NULL, lookahead.lineno );
}
astnode continue_stmt(){
	parse_trace( CONTINUE_STMT, lookahead.name );
	astnode pleft = NULL;
	TOKEN auxtkn;
	
	match( CONTINUE ); 

	auxtkn = match( SEMI );
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "';' expected after continue statement and" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}	

	return maketree( LBL_CONTINUE, NULL, NULL, NULL, NULL, lookahead.lineno );
}
astnode return_stmt(){
	parse_trace( RETURN_STMT, lookahead.name );
	astnode pleft = NULL;
	TOKEN auxtkn;

	match( RETURN );
	pleft = return_stmt_p();	
	
	auxtkn = match( SEMI );
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg( "';' expected in return statement" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}

	return maketree( LBL_RETURN, pleft, NULL, NULL, NULL, lookahead.lineno );
}

astnode return_stmt_p(){
	parse_trace( RETURN_STMT, lookahead.name );
	astnode pleft = NULL;

	if( lookahead.name == MINUS || lookahead.name == NOT || lookahead.name ==  LPAREN || 
	    lookahead.name == NUM || lookahead.name == BLIT || lookahead.name == ID ){
		pleft = expression();	
	}
	else{ /*epsilon*/
		pleft = maketree( LBL_EPSILON, NULL, NULL, NULL, NULL, lookahead.lineno );
	}
	return pleft;
}
astnode null_stmt(){
	parse_trace( NULL_STMT, lookahead.name );
	
	match( SEMI );

	return maketree( LBL_NULL, NULL, NULL, NULL, NULL, lookahead.lineno );
}
astnode expression(){
	parse_trace( EXPRESSION, lookahead.name );
	int node_name;
	astnode pleft = NULL;
	astnode pcenter = NULL;
	astnode ptemp = NULL;

	pleft = add_exp(); 
	ptemp = expression_p();

	/* pcenter is builded */
	node_name = op( ptemp );
	pcenter = left( ptemp );
	free( ptemp );

	if( node_name != LBL_EPSILON )
		return maketree( node_name, pleft, pcenter, NULL, NULL, lookahead.lineno ); 
	else							
		return pleft;				
}
astnode expression_p(){
	parse_trace( EXPRESSION, lookahead.name );
	astnode pleft =  NULL;
	OP operator;

	if( lookahead.name == LT || lookahead.name == LTEQ || lookahead.name == GT || 
	    lookahead.name == GTEQ || lookahead.name == EQ || lookahead.name == NEQ ){
		operator = relop(); pleft = add_exp();	
	}
	else{ /*epsilon */
		operator = LBL_EPSILON;
	}
	return maketree( operator, pleft, NULL, NULL, NULL, lookahead.lineno );
}
astnode add_exp(){
	parse_trace( ADD_EXP, lookahead.name );
	int node_name;
	astnode pleft = NULL;
	astnode pcenter = NULL;
	astnode pright = NULL;
	astnode ptemp = NULL;

	if( lookahead.name == MINUS ){
		uminus(); pleft = term(); ptemp = add_exp_p();	/* fix this later, there is not enough space for attaching it to the subtree */
	}
	else if( lookahead.name == NOT || lookahead.name == LPAREN || lookahead.name == NUM || lookahead.name == BLIT || lookahead.name == ID ){
		pleft = term(); ptemp = add_exp_p();
	
	}
	else{
		/* syntactic error */
		error_list_add( create_err_msg( "'expression' was expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}		
	
	/* pcenter is builded */
	node_name = op( ptemp );
	pcenter = left( ptemp );
	pright = center( ptemp );
	free( ptemp );

	if( node_name == LBL_EPSILON )
		return pleft;
	else{
		if( node_name == LBL_PLUS ) node_name = LBL_PLUS_MAIN;
		else if( node_name ==  LBL_MINUS ) node_name = LBL_MINUS_MAIN;
		else if( node_name == LBL_OR ) node_name = LBL_OR_MAIN;
		else if( node_name == LBL_ORELSE ) node_name = LBL_ORELSE_MAIN;
		else{ 	
			/* syntactic error */
			error_list_add( create_err_msg( "operator '+','-','or' or 'orelse' expected" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}

		return maketree( node_name, pleft, pcenter, pright, NULL, lookahead.lineno );

	}
}

astnode add_exp_p(){
	parse_trace( ADD_EXP, lookahead.name );
	astnode pleft =  NULL;
	astnode pcenter = NULL;	
	OP operator;

	if( lookahead.name == PLUS || lookahead.name == MINUS || lookahead.name == OR || lookahead.name == ORELSE ){
		operator = addop(); pleft = term(); pcenter = add_exp_p();	
	}
	else{ /*epsilon */
		operator = LBL_EPSILON;
	}

	return maketree( operator, pleft, pcenter, NULL, NULL, lookahead.lineno );

}
astnode term(){
	parse_trace( TERM, lookahead.name );
	int node_name;	
	astnode pleft = NULL;
	astnode pcenter = NULL;
	astnode pright = NULL;
	astnode ptemp = NULL;	

	pleft = factor(); ptemp = term_p();

	/* pcenter is builded */
	node_name = op( ptemp );
	pcenter = left( ptemp );
	pright = center( ptemp );
	free( ptemp );
	
	if( node_name == LBL_EPSILON )
		return pleft;
	else{
		if( node_name == LBL_DIV ) node_name = LBL_DIV_MAIN;
		else if( node_name == LBL_MULT ) node_name = LBL_MULT_MAIN;
		else if( node_name == LBL_MOD ) node_name = LBL_MOD_MAIN;
		else if( node_name == LBL_AND ) node_name = LBL_AND_MAIN;
		else if( node_name == LBL_ANDTHEN ) node_name = LBL_ANDTHEN_MAIN;
		else{
			/* syntactic error */
			error_list_add( create_err_msg( "operator '/', '*', 'mod', 'and' or 'andthen' expected" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}
		return maketree( node_name, pleft, pcenter, pright, NULL, lookahead.lineno );

	}

}

astnode term_p(){
	parse_trace( TERM, lookahead.name );
	OP operator;	
	astnode pleft = NULL;
	astnode pcenter = NULL;

	if( lookahead.name == MULT || lookahead.name == DIV || lookahead.name == MOD || lookahead.name == ANDTHEN || lookahead.name == AND ){
		operator = multop(); pleft = factor(); pcenter = term_p();	
	}
	else{ /* e */
		operator = LBL_EPSILON;
	}
	 
	return maketree( operator, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
astnode factor(){
	parse_trace( FACTOR, lookahead.name );
	astnode pnode;


	if( lookahead.name == NOT || lookahead.name == LPAREN || lookahead.name == NUM || lookahead.name == BLIT )
		pnode = nid_factor();
	else if( lookahead.name == ID )
		pnode = id_factor();
	else{
		/* syntactic error */
		error_list_add( create_err_msg( "'factor' expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}
	return pnode;
}
astnode nid_factor(){
	parse_trace( NID_FACTOR, lookahead.name );
	int node_name;	
	astnode pleft = NULL;	
	astleaf pleaf = NULL;
	TOKEN auxtkn;

	if( lookahead.name == NOT ){
		match( NOT ); pleft = factor();
		node_name = LBL_NOT;
		return maketree( node_name, pleft, NULL, NULL, NULL, lookahead.lineno );		
	}
	else if( lookahead.name == LPAREN ){
		match( LPAREN ); pleft = expression(); 


		auxtkn = match( RPAREN );
		if( auxtkn.name == PARSE_ERROR ){
			/* syntactic error */
			error_list_add( create_err_msg( "')' expected as part of a factor" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}
	
		return pleft;
	}
	else if( lookahead.name == NUM ){
		auxtkn = match( NUM );
		pleaf = makeleaf( LBL_NUM, auxtkn.table_entry );
		settype_leaf( pleaf, TYPE_INT );
		return maketree( LBL_NUM, NULL, NULL, NULL, pleaf, lookahead.lineno );
	}
	else if( lookahead.name == BLIT ){
		auxtkn = match( BLIT ); 
		pleaf = makeleaf( LBL_BLIT, auxtkn.table_entry );
		settype_leaf( pleaf, TYPE_BOOL );
		return maketree( LBL_BLIT, NULL, NULL, NULL, pleaf, lookahead.lineno );
	}
	else{
		/* syntactic error although it should never happen*/
		error_list_add( create_err_msg( "'factor' expected" ), lookahead.lineno );
		return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
	}
	
}
astnode id_factor(){
	parse_trace( ID_FACTOR, lookahead.name );	
	int node_name;	
	astnode pleft = NULL;
	astnode pcenter = NULL;

	inh_type = NO_TYPE; pleft = id_const(); pcenter = id_tail();
	
	/* obtains node name */
	node_name = op( pcenter );

	if( node_name == LBL_EPSILON )
		return pleft;		
	else
		return maketree( LBL_ARRAY_OR_CALL, pleft, pcenter, NULL, NULL, lookahead.lineno );
}
astnode id_tail(){
	parse_trace( ID_TAIL, lookahead.name );
	astnode pnode = NULL;

	
	if( lookahead.name == LPAREN )
		pnode = call_tail();
	else
		pnode = var_tail(); /* implemented this way, because FIRST(var_tail) includes epsilon production see parsing grammar */

	return pnode;
}
astnode var_tail(){
	parse_trace( VAR_TAIL, lookahead.name );
	astnode pnode;
	TOKEN auxtkn;

	if( lookahead.name == LSQR ){
		match( LSQR ); pnode = add_exp(); 

		auxtkn = match( RSQR );
		if( auxtkn.name == PARSE_ERROR ){
			/* syntactic error */
			error_list_add( create_err_msg( "']' expected" ), lookahead.lineno );
			return maketree( LBL_ERROR, NULL, NULL, NULL, NULL, lookahead.lineno );
		}	
	}
	else{ /* e */
		return maketree( LBL_EPSILON, NULL, NULL, NULL, NULL, lookahead.lineno );
	}
	return pnode;
}

OP relop(){
	parse_trace( RELOP, lookahead.name );

	if( lookahead.name == LTEQ )	 { match( LTEQ ); return LBL_LTEQ;  }
	else if( lookahead.name == LT )	 { match( LT );   return LBL_LT; 	}	
	else if( lookahead.name == GT )	 { match( GT );   return LBL_GT; 	}
	else if( lookahead.name == GTEQ ){ match( GTEQ ); return LBL_GTEQ; 	}
	else if( lookahead.name == EQ )	 { match( EQ );   return LBL_EQ; 	}
	else if( lookahead.name == NEQ ) { match( NEQ );  return LBL_NEQ; 	}
	else{
		/* syntactic error */
		error_list_add( create_err_msg( "relational operator '<', '<=', '>', '>=', '=' or '/=' expected" ), lookahead.lineno );
		return LBL_ERROR;
	}

}
OP addop(){
	parse_trace( ADDOP, lookahead.name );
	
	if( lookahead.name == PLUS)	   { match( PLUS );  return LBL_PLUS;    }
	else if( lookahead.name == MINUS ) { match( MINUS ); return LBL_MINUS;   }
	else if( lookahead.name == OR )	   { match( OR );    return LBL_OR;      }
	else if( lookahead.name == ORELSE ){ match( ORELSE );return LBL_ORELSE;  } 
	else{
		/* syntactic error */
		error_list_add( create_err_msg( "operator '+', '-', 'or' or 'orelse' expected" ), lookahead.lineno );
		return LBL_ERROR;
	}
}
OP multop(){
	parse_trace( MULTOP, lookahead.name );


	if( lookahead.name == MULT )         { match( MULT );	return LBL_MULT; 	}
	else if( lookahead.name == DIV )     { match( DIV );	return LBL_DIV;	}
	else if( lookahead.name == MOD )     { match( MOD );	return LBL_MOD;	}
	else if( lookahead.name == AND )     { match( AND );	return LBL_AND;	}
	else if( lookahead.name == ANDTHEN ) { match( ANDTHEN );return LBL_ANDTHEN; }
	else{
		/* syntactic error */
		error_list_add( create_err_msg( "operator '*','/','mod','and' or 'andthen' expected" ), lookahead.lineno );
		return LBL_ERROR;
	}
}
OP uminus(){
	parse_trace( UMINUS, lookahead.name );
	TOKEN auxtkn;

	auxtkn = match( MINUS );
	if( auxtkn.name == PARSE_ERROR ){
		/* syntactic error */
		error_list_add( create_err_msg("symbol '-' expected" ), lookahead.lineno );
		return LBL_MINUS;
	}

	return LBL_MINUS;
}

char *create_err_msg( char* cadena ){
	/* 
	   creates an error message using:
		a) an initial msg
		b) token lexeme if the token has a table entry
		c) token name (which is an int) for those without table entries
	
		e.g.
			intial msg: "expected ';', or '}'
			added string: "before 'if' " <<<--- it is created depending on wheter the current lookahead has a table_entry or not.
		
			it returns initial msg + added string
	*/
	
	char *s;

	if( lookahead.name == ID || lookahead.name == NUM || lookahead.name == BLIT ){ /* is an identifier, numeral or blit  */
		s = ( char* )malloc( strlen( cadena ) + strlen(" before '' ") + strlen( lookahead.table_entry->lexeme ) + 1 );
		sprintf( s, "%s before '%s'", cadena, lookahead.table_entry->lexeme );
	}
	else{	/* is a regular token */
		s = ( char* )malloc( strlen( cadena ) + strlen(" before '' ") + strlen( tokens[ lookahead.name + TOKEN_OFFSET ] ) + 1 );
		sprintf( s, "%s before '%s'", cadena, tokens[ lookahead.name + TOKEN_OFFSET ] );
	}
	return s;
}
