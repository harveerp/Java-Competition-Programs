/*
 * ast.c
 * 
 * 
 *
 * 
 * AST MODULE:
 * 	contains the root node of the AST, therefore contains the AST. It provides interface 
 * 	functions for build it, modify it, and print it.
 * 
 *
 * DEPENDENCIES:
 *	ast.h, c12.h
 *
 *DESCRIPTION:
 *   
 */

#include <stdlib.h>
#include <stdio.h>
#include "c12.h" 
#include "ast.h"

/* ------------------------------------------------------------------------------------------------- */
/* ---------------------------------------      MACROS     ----------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

#define IS_EMPTYNODE(x)		  ( ((x)->leftptr) == NULL && ((x)->centerptr) == NULL && ((x)->rightptr) == NULL ) ? true : false


/* ------------------------------------------------------------------------------------------------- */
/* -----------------------------------      FUNCTION DEC      -------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

 
static void traversal( astnode );	/* ast depth-first traversal function*/
static astnode availnode();
static astleaf availleaf();
static void setlineno( astnode, long int );


/* ------------------------------------------------------------------------------------------------- */
/* -------------------------------------      VARIABLES     ---------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

static FILE *file;
static char ast_ofile[14] = "ast_output.txt";	/* the output file's name */
astnode ast_root;				/* this is the root node of the syntax tree */

char *node_labels[NUM_OF_NODE_LABELS] = 	/* contains the text form of all node labels... for printing purposes */
			{ "e", "program", "void", "int", "bool", "id", "array", "multivar", "var_name", 				   
			 "fun_dec_tail", "params",  "multiparam", "ref_bool", "ref_int", ":=", "array_assign", "routine_call", "compound-stmt", 	    
			 "compound-stmt'","compound-stmt''", "compound-stmt'''", "if-stmt", "loop-stmt", "loop-stmt'", "exit", "continue", "return",
			 "num", "blit", "/=", "=", ">=", ">", "<=", "<", "/", 
			 "*", "-", "+", "orelse", "or", "not", "mod","and", "andthen", 
			 "-", "multiparams", "arguments'", "null", "declaration", "param", "compound-declaration", "+(main)", "array-or-call", 
			 "no arguments", "*(main)", "-(main)", "or(main)","orelse(main)", "/(main)", "mod(main)", "and(main)", "andthen(main)",
			 "ERROR!!"
			}; /* for easy counting there are 9 elements in each line */

char *types[NUM_OF_TYPES] = 			/* contains the text form of all types... for printing purposes */
			{ "int", "bool", "void", "ref int", "ref bool", "no type", "universal", "e", "error" };



/* ------------------------------------------------------------------------------------------------- */
/* -------------------------------------      FUNCTION DEF's  -------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

astnode maketree( int op, astnode leftson, astnode centerson, astnode rightson, astleaf optional_leaf, long int line_num ){
	/* Creates a new node, then attaches child subtrees
 
	   that might be empty or not. 
	
	   Therefore, makes a tree 
	*/ 
	astnode p;
	p = availnode();
	setop( p, op );
	setleft( p, leftson );
	setcenter( p, centerson );
	setright( p, rightson );
	setleaf( p, optional_leaf );
	setlineno( p, line_num );

	return p;
}

astleaf makeleaf( int tok_name, ST_TABLE entry ){
	/* 
	    creates a new leaf data structure 
	*/
	astleaf p;
	p = availleaf();
	settoken_name( p, tok_name );
	settableentry( p, entry );

	return p;
}

astnode left(astnode p){
	 /* Returns a copy of the left

	    pointer of the node p points to.

	*/
      return(p->leftptr);
}

astnode center(astnode p){

	/* Returns a copy of the center

	pointer of the node p points to.

	*/
	return(p->centerptr);
}

astnode right(astnode p){
	/* Returns a copy of the right

	pointer of the node p points to.

	*/
	return(p->rightptr);

}

int op(astnode p){

	/* Returns a copy of the right

	pointer of the node p points to.

	*/
	return(p->op);

}

astnode availnode( )
{
	/* Returns a pointer to storage

	allocated for a new node.

	*/
	  astnode p;
	  p = ( astnode )malloc( sizeof(noderecord) );
	  p->op = -1;	// init node
	  p->type = NO_TYPE;
	  p->lineno = -1;

	 return( p );
}

astleaf availleaf( )
{
	/* Returns a pointer to storage

	allocated for a new leaf.

	*/
	astleaf p;
	p = ( astleaf )malloc( sizeof(leafrecord) );		
	p->token_name = -1;	//init node
	p->type = NO_TYPE;
	p->is_array = false;
	//p->paramnode = NULL;
	return( p );
}

void setop(astnode p,int value)
{
	/* Copies the contents of value

	into the record p points to.

	*/
	p->op = value;
}

void setlineno( astnode p, long int line_no ){
	/* 
		sets the line number
	*/
	p->lineno = line_no;
	
}

void settoken_name( astleaf p, int value ){
	/* 
	copies content of value int the

	record p points to
	*/
	p->token_name = value;
}

void settype_node( astnode p, int value ){
	/* 
	copies content of value int the

	record p points to
	*/
	p->type = value;
}

void settype_leaf( astleaf p, int value ){
	/* 
	copies content of value int the

	record p points to
	*/
	p->type = value;
}


void setleft(astnode p,astnode q)
{
	/* Copies q into the left pointer

	of the record p points to.

	*/
	p->leftptr = q;

}

void setcenter( astnode p, astnode q)
{

	/* Copies q into the center pointer

	of the record p points to.

	*/
	p->centerptr = q;
}

void setright( astnode p, astnode q)
{
	/* Copies q into the right pointer

	of the record p points to.

	*/
	p->rightptr = q;
}

void setleaf( astnode p, astleaf q)
{
	/* Copies q into the right pointer

	of the record p points to.

	*/

	p->leaf = q;
}

void settableentry( astleaf p, ST_TABLE q ){

	/* Copies q into the table entry field

	of the record p points to.

	*/
	p->table_entry = q;
}

astnode get_ast_root(){
	astnode ptree;

	ptree = ast_root;

	return ptree;
}

void printtree(){
	/* 
	   prints the tree by calling the
	
	   traversal function */
	
	file = fopen( ast_ofile, "w" );

	astnode ptree;	
	ptree = ast_root;

	fprintf( file, "\nABSTRACT SYNTAX TREE ( depth-first traversal in preorder )"		/* output file's title */
		       "\n----------------------------------------------------------\n" );

	if( ptree != NULL ) traversal( ptree ); 						/* this avoids trying to print an empty tree */
	
	fclose( file );
}

void traversal( astnode p ){

	/* 
	   Performs a depth First Traversal in preorder. 

	   See the syntax tree implementation file for more info
 	*/


	static int counter;
	
	if( p->leaf == NULL ){ /* IT'S A NODE */
	

		/* prints identation */
		for(int i=0; i<counter; i++)
			fprintf( file, "     |" );	

		if( IS_EMPTYNODE(p) ){ /* IT'S A LEAVE IMPLEMENTED AS NODE */
			fprintf( file, ">[%s]\n", node_labels[ ( p->op ) + NODE_LABELS_OFFSET ] );	
			fflush( file );	
		}
		else{ 			/* IT'S A REAL NODE */
			fprintf( file, ">(%s)\n", node_labels[ ( p->op ) + NODE_LABELS_OFFSET ] );	
			fflush( file );	

			/* traverses childs */
			if( ( p->leftptr ) != NULL  ){
				counter++;		
				traversal( p->leftptr );
				counter--;
			}

			if( ( p->centerptr )  != NULL  ){
				counter++;
				traversal( p->centerptr );
				counter--;
			}
			if( ( p->rightptr )  != NULL  ){
				counter++;
				traversal( p->rightptr );
				counter--;
			}
		}
		
	}	
	else{ /* IT'S A LEAF */

		/* prints leaf */
		for(int i=0; i<counter; i++)
			fprintf( file, "     |" );		/* identation */

		if( p->leaf->token_name == LBL_ID ){
				if( p->leaf->type == NO_TYPE )
					fprintf( file, ">[%s]index:%d\n", node_labels[ ( p->leaf->token_name ) +NODE_LABELS_OFFSET ],
									  p->leaf->table_entry->index );
				else
					fprintf( file, ">[%s]index:%d, type: %s\n", node_labels[ ( p->leaf->token_name ) +NODE_LABELS_OFFSET ], 
									            p->leaf->table_entry->index, types[ (p->leaf->type) + TYPES_OFFSET ] );
		}
		else if( p->leaf->token_name == LBL_NUM )
				fprintf( file, ">[%s]lex:%s type: %s\n", node_labels[ ( p->leaf->token_name ) + NODE_LABELS_OFFSET ], 
                                                                         p->leaf->table_entry->lexeme, types[ (p->leaf->type) + TYPES_OFFSET ] );
		else if( p->leaf->token_name == LBL_BLIT )
				fprintf( file, ">[%s]lex:%s type:%s\n", node_labels[ ( p->leaf->token_name ) + NODE_LABELS_OFFSET ], 
                                                                        p->leaf->table_entry->lexeme, types[ (p->leaf->type) + TYPES_OFFSET ] );
		else
				fprintf( file, ">[¡¡ERROR!!]\n" );

		fflush( file ); /* when segfault caused it's good to have the partial printed tree */
	}	
}
