/*
 * interm_codegen.c
 * 
 *
 *
 * 
 * INTERMEDIATE CODE GENERATOR MODULE:
 * 	Traverses the annotated syntax tree and generates an output file containing the corresponding executable three-address code
 *      in the form of quadruples
 *
 * DEPENDENCIES:
 *	c12.h interm_codegen.h ast.o
 *
 *   	
 */

#include <stdio.h>
#include <stdlib.h>
#include "c12.h"
#include "ast.h"
#include "interm_codegen.h"

#define IS_EMPTYNODE(x)		  ( ((x)->leftptr) == NULL && ((x)->centerptr) == NULL && ((x)->rightptr) == NULL ) ? true : false

static void traversal( astnode );	/* ast depth-first traversal function*/

extern astnode ast_root;			/* The syntax tree defined in ast.c*/
static char ast_ofile[] = "interm_code.tac";	/* the output file's name... tac stand for three-address code */
static FILE *file;

void generate_interm_code(){
	/* 
	   traverses the ast and fill an output file with the corresponding 
	   executable quadruples representing three-address code
	*/
	
	file = fopen( ast_ofile, "w" );

	astnode ptree;	
	ptree = ast_root;

	if( ptree != NULL ) traversal( ptree ); /* this avoids trying to traverse an empty tree */
	
	fclose( file );
}

void traversal( astnode p ){

	/* 
	   Performs a depth First Traversal in preorder. 

	   See the syntax tree implementation file for more info
 	*/


	if( p->leaf == NULL ){ /* IT'S A NODE */

		if( IS_EMPTYNODE(p) ){ /* IT'S A LEAVE IMPLEMENTED AS NODE */
	
			fflush( file ); /* when segfault caused it's good to have the partial printed tree */
		}
		else{ 			/* IT'S A REAL NODE */

			/* --visits node-- */



			fflush( file ); /* when segfault caused it's good to have the partial printed tree */
			/* --end of node visiting-- */

			
			/* traverses childs */
			if( ( p->leftptr ) != NULL  )
				traversal( p->leftptr );

			if( ( p->centerptr )  != NULL  )
				traversal( p->centerptr );

			if( ( p->rightptr )  != NULL  )
				traversal( p->rightptr );

		}
		
	}	
	else{ /* IT'S A LEAF */


		fflush( file ); /* when segfault caused it's good to have the partial printed tree */
	}	
}
