/*
 * semanalyzer.c
 * 
 *
 *
 * 
 * SEMANTIC ANALYSIS MODULE:
 * 	Checks the semantic consistency of a syntactically correct intermediate representation of the input program. It creates 
 *	an annotated AST from the received AST from the syntactic analyzer. Annotations are created by filling fields of the data estructure that
 * 	up to this points used to have no significant value.
 * 
 *
 * DEPENDENCIES:
 *	c12.h semanalyzer.h ast.o admin.o ident_table.o access_table.o
 *
 *   	
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "c12.h"
#include "semanalyzer.h"
#include "admin.h"
#include "ast.h"
#include "ident_table.h"
#include "access_table.h"
#include "error_reporting.h"

/* ------------------------------------------------------------------------------------------------- */
/* ---------------------------------------  DEBUG CONTROL  ----------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

/* NOTE: Maximize the terminal window to see all information properly. Printed in stdout */


//#define SEE_ITABLE_CONST			/* uncoment to see step by step the construction of the ident table */
//#define PARAM_CONSIST_DEBUG			/* uncoment to see the param consistency-checking process */



/* ------------------------------------------------------------------------------------------------- */
/* --------------------------------     CONSTANT DEFINITION     ------------------------------------ */
/* ------------------------------------------------------------------------------------------------- */

/*traverse modes */
#define INITIAL			  	34
#define NORMAL			 	35

/* semantic error defintions */
#define FUNC_TYPE_MISMATCH	  	38
#define FUNC_NUM_MISMATCH	  	39
#define FUNC_SUCC		  	40
#define IN_ASSIGN		  	41
#define STAND_ALONE		  	42
#define MAIN_NOT_FOUND		  	43
#define MAIN_TYPE_ERR		  	44
#define MAIN_PARAMS_ERR		  	45
#define MAIN_SUCC		  	46
#define TYPE_VOID_ERROR		  	47
#define TYPE_FUNC_ERROR		  	48
#define FUNC_TYPE_ARRAY_EXPECTED  	49
#define FUNC_TYPE_REF_MISMATCH	  	50
#define FUNC_TYPE_ARRAY_CANNOT_BE_REF 	51
#define FUNC_TYPE_ARRAY_AS_ARGUMENT 	52
#define FUNC_TYPE_ARRAY_REF_PARAMS 	53
#define FUNC_NO_DEF_FOUND		54
#define FUNC_NO_PARAM_FOUND		55

/* auxiliar */
#define CHANGED_IN_THIS_NODE		900
#define UNCHANGED			901
#define CONST_VAL			902
#define VAR_VAL				903


/* ------------------------------------------------------------------------------------------------- */
/* ------------------------------------   FUNCTION DECLARATIONS  ----------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

static void visit( astnode, int );					/* For Scope analysis. Depth-First traversal in pre-order recursive function.*/ 
static void visit_postorder( astnode );					/* For Type analysis. Depth-First traversal in post-order recursive function.*/
static void do_scope_analysis();					/* performs scope analysis */
static void do_type_analysis();						/* performs type analysis */
#ifdef SEE_ITABLE_CONST
static void print_itable_const();					/* prints in stdout the construction of the Ident Table... for debugging */
#endif
static int check_main();						/* checks main definition as the last function */
static TYPE type_synthesis_arithmetic( astnode, astnode, astnode );	/* performs type synthesis for nodes that are arithmetic and logical operators */
static TYPE type_synthesis_assign( astnode, astnode );			/* performs type synthesis for nodes that are assignments */ 		
static int check_param_consistency( astnode, astnode, int );		/* checks parameters consistency when visitng an applied ocurrence of a routine */



/* ------------------------------------------------------------------------------------------------- */
/* ---------------------------------------      MACROS       --------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */

#define IS_EMPTYNODE(x)  ( ((x)->leftptr) == NULL && ((x)->centerptr) == NULL && ((x)->rightptr) == NULL ) ? true : false




/* ------------------------------------------------------------------------------------------------- */
/* ---------------------------------------     VARIABLES     --------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */


extern astnode ast_root;			/* The syntax tree defined in ast.c*/
extern char *node_labels[NUM_OF_NODE_LABELS];	/* string form of labels... for printing. defined in ast.c */
extern char *types[NUM_OF_TYPES];		/* string form of types... for printing. defined in ast.c */


/* used during scope analysis */
static int IdI = 1;				/* Identifier Index. Used for filling identification table. Must start in 1!*/
static int level = 0;				/* frame number field. Used for filling IT */
static int next = 0;				/* next	field. For filling IT */
static int LexI = 0;				/* lexical index. For Filling IT */
static astnode DecPtr = NULL;			/* Pointer to declaration node. For filling IT */
static int current_frame = 0;			/* tells the current level when creating the IT */
static astnode last_node_visited = NULL;	/* during the traversal, points to the last node visited, not the current one */
static astnode last_function_nodeptr = NULL;	/* contains a pointer to the last function definition. determined during the initial traversal
						   used to check "main" function */
static bool inside_loop_stmt = false;		/* during traversal, tells wether currently the node visited is within a loop statement. init val: false */

/* used during type analysis */
static TYPE return_type;			
static TYPE func_type;
static bool empty_compound_stmt = true;		/* when reached a function definition, helps to determine if both list of variables and 
						   list of statements are empty. Must start in true! */ 
static int const_or_var = CONST_VAL;
static int last_expr_val = CONST_VAL;





/* ------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */
/* ------------------------------------   FUNCTION DEFINITIONS   ----------------------------------- */
/* ------------------------------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------------------------------- */



void do_semantic_analysis()
{	
	/* 	Performs The Complete Semantic Analysis.
		
		0. Initializes identification table and access table.
		1. Performs Scope analysis
		2. Checks main consistency with language rules		
		3. Do type analysis

		MUST: Must be specifically in that order: 0, 1, 2, 3.
	*/

	int main_state;
	
	/* step 0 */ 
	itable_init();
	atable_init();		
	
	/* step 1 */
	do_scope_analysis();
	

	/* step 2 */
	main_state = check_main();	
	if( main_state == MAIN_NOT_FOUND ){
		/* Semantic Error: No main definition */
		error_list_add( "No \"main\" definition found as the last declaration", -1 );
	}
	else if( main_state == MAIN_TYPE_ERR ){
		/* Semantic Error: No main definition */
		error_list_add( "\"main\" definition must be of type 'int'", -1 );
	}
	else if( main_state == MAIN_PARAMS_ERR ){
		/* Semantic Error: No main definition */
		error_list_add( "\"main\" definition does not accept parameters", -1 );
	}

	/* step 3 */
	do_type_analysis();
	
}


#ifdef SEE_ITABLE_CONST
void print_itable_const()
{
	/* 
	   Prints the current states of the identification table and the access table. Before printing clears the screen, and after printing
	   pauses. For debugging purposes
	   As this function si called several times during the construction of the IT,  it shows how the table is constructed 
	*/

	/* prints stack of IT */
	system("clear");
	print_stack();

	/* prints access table */
	print_atable();
	getc(stdin);
}
#endif


int check_main(){
	/* 	
		Checks Main Consystency with the language semantic rules: must exist as the last definiton; must be of type int; must 
		have no parameters (void).

		can return: 

			MAIN_SUCC:	  All fine
			MAIN_NOT_FOUND:	  Main not found
			MAIN_TYPE_ERR:	  type is not int
			MAIN_PARAMS_ERR:  params are not void
	*/


	/* check existence */
	if( last_function_nodeptr->leaf->table_entry->lexeme != NULL ){
		if( strcmp( last_function_nodeptr->leaf->table_entry->lexeme, "main") != 0 )		
			return MAIN_NOT_FOUND;
	}
	/* check return type and param type */
	if( last_function_nodeptr->leaf->type != TYPE_INT )
		return MAIN_TYPE_ERR;
	if( last_function_nodeptr->leaf->paramnode->op != LBL_VOID )
		return MAIN_PARAMS_ERR;	
	
	return MAIN_SUCC;
}



void do_scope_analysis(){
	/* 	
		Performs the scope analysis. 

		It makes it in two parts: initial scan and full traversal. Both traversals are recursive depth-first in preorder.
		
	*/


	astnode ptree = NULL;
	
	/* Initial Scan 
	   Traverses the global declaration nodes and adds all function names to identification table.
	   See guidelines for more details.
	*/

	current_frame = 0;
	
	ptree = ast_root;
	if( ptree != NULL ) 
		visit( ptree, INITIAL ); 
	
	/* One unique full traverse of the AST */
	current_frame = 0;
	
	ptree = ast_root;
	if( ptree != NULL ) 
		visit( ptree, NORMAL ); 
}

void do_type_analysis(){
	/* 
		Performs type analysis.

		At this point all semantic bindings have been solved during scope analysis. It performs a recursive depth-first 
		post-order traversal			 
	
		
	*/
	 
	astnode ptree = NULL;
	
	ptree = ast_root;	
	if( ptree != NULL )
		visit_postorder( ptree );
}



void visit( astnode pnode, int mode ){
	/* 
	   Visit the syntax tree nodes in a  first-traversal in preorder at the time it checks for semantic coherence. 

	   It's part of the scope anlysis.

	   1. visit node (i.e. do things)
	   2. traverses the left subtree
	   3  traverses the center subtree
	   4. traverses th right subtree

	   If INITIAL mode is selected, it only goes as deep as fun_dec_tail.
	   
 	*/

	int frame_state = UNCHANGED;		/* tells if the frame number was incremented */

	if( pnode->leaf == NULL ){ 	   /* IT'S A NODE */
		

		if( IS_EMPTYNODE(pnode) ){ /* IT'S A LEAVE IMPLEMENTED AS NODE */


			/* checks for continue's and exit's statements outside loop statements */
			if( mode != INITIAL ){
				if( pnode->op == LBL_CONTINUE ){
					if( inside_loop_stmt == false ){
						/* semantic error */
						error_list_add( "'continue' statement outside loop statement", pnode->lineno );
					}
				}
				else if( pnode->op == LBL_EXIT ){
					if( inside_loop_stmt == false ){
						/* semantic error */
						error_list_add( "'exit' statement outside loop statement", pnode->lineno );
					}
				}
			}

		}
		else{ 			   /* IT'S A REAL NODE */
			
			if( mode == INITIAL ){	/* Initial treverse mode, that is, treverses only on a global level. This includes 
			  			   functions and global variables 
						*/
				if( pnode->op == LBL_DECLARATION ){
					if( pnode->centerptr->op == LBL_FUN_DEC_TAIL )
						last_function_nodeptr = pnode->leftptr;
				}			

				/*traverses childs globally, no further than fun_dec_tail*/

				/* updates last node visited jsut before traversing childs*/
				last_node_visited = pnode;
				if( pnode->op != LBL_FUN_DEC_TAIL ){
					if( ( pnode->leftptr ) != NULL  ){	
						visit( pnode->leftptr, mode );
					}
					if( ( pnode->centerptr )  != NULL  ){
						visit( pnode->centerptr, mode );
					}
					if( ( pnode->rightptr )  != NULL  ){
						visit( pnode->rightptr, mode );
					}
				}	
			}
			else{/* Normal mode */
				if( pnode->op == LBL_LOOP_STMT ){
					/* It's for determining when does a loop stmt begins.
					    */
					inside_loop_stmt = true;

				}
				if( pnode->op == LBL_COMPOUND_STMT_PPP ){
					/* It's for determining when does a loop stmt finishes.
					   Even though the ast does not include the "end" token, every loop statement will always be followed by a 
					   this node */
					inside_loop_stmt = false;

				}

				/*
				  Frame changing operates under the assumption that the order in which nodes that require to start a new frame 
				  are visited, correspond to the opposite order in which they are leaved. (like a stack, first in, last out ).
				*/	

				if( pnode->op == LBL_DECLARATION ){
					
					if(pnode->centerptr->op == LBL_FUN_DEC_TAIL ){ //parameters
						pnode->leftptr->leaf->paramnode = pnode->centerptr->leftptr; /* links parameters with their identifier */
													     /* or identifier with parameters          */
						#ifdef SEE_ITABLE_CONST
						printf( "linked %s with its parameters       ", pnode->leftptr->leaf->table_entry->lexeme );
						#endif
						if( pnode->leftptr->leaf->paramnode->op == LBL_VOID ){
							#ifdef SEE_ITABLE_CONST
							printf("no parameters\n");
							getc(stdin);
							#endif
						}
						else{
							#ifdef SEE_ITABLE_CONST
							printf("proof... op of paramnode: %s\n", 
								node_labels[ ( pnode->leftptr->leaf->paramnode->op )  +NODE_LABELS_OFFSET ] );
							getc(stdin);
							#endif
						}
					}
	
				}
				/* links id's definitions that are arrays with their index value */
				if( pnode->op == LBL_DECLARATION || pnode->op == LBL_COMPOUND_DECLARATION  ){
						if( pnode->centerptr->op == LBL_ARRAY ){
							pnode->leftptr->leaf->is_array = true;			      // official en array
							pnode->leftptr->leaf->arrayinode = pnode->centerptr->leftptr; //[id] -- linked --> [num]|(add-exp)
						}

				}
				else if( pnode->op == LBL_PARAM ){
						/* links id's definitions that are array with their index values */
						if( pnode->centerptr->op == LBL_ARRAY ){
							pnode->leftptr->leaf->is_array = true;	// official en array
												//although it has no index
						}
				}

				// new frame at entering this nodes
				// the last condition is for the case when the current node it's in a compound statement and it comes from a 
				//function beginning
				if( pnode->op == LBL_FUN_DEC_TAIL || 
				       ( pnode->op == LBL_COMPOUND_STMT &&
					 last_node_visited->op != LBL_FUN_DEC_TAIL && 
					 last_node_visited->op != LBL_PARAMS && 
				         last_node_visited->op != LBL_MULTIPARAM &&
                     			 last_node_visited->op != LBL_PARAM  &&
					 last_node_visited->op != LBL_ARRAY_OR_CALL ) || 
				    	 pnode->op == LBL_IF_STMT || 
				    	 pnode->op == LBL_LOOP_STMT || 
				    	 pnode->op == LBL_IF_STMT  ){
					 current_frame++;
					 frame_state = CHANGED_IN_THIS_NODE;
				}
				else{
					frame_state = UNCHANGED;	
				}

				/* updates last node visited jsut before traversing childs*/
				last_node_visited = pnode;

				/* traverses childs */
				if( ( pnode->leftptr ) != NULL  ){		
					visit( pnode->leftptr, mode );
				}
				if( ( pnode->centerptr )  != NULL  ){
					visit( pnode->centerptr, mode );
				}
				if( ( pnode->rightptr )  != NULL  ){
					visit( pnode->rightptr, mode );
				}

				// delete frame at exiting the same nodes
				if( frame_state == CHANGED_IN_THIS_NODE ){
					int n = get_num_elem_frame( current_frame );
					#ifdef SEE_ITABLE_CONST
					printf(" elements in frame %d: %d \n", current_frame, n );
					getc(stdin);
					#endif
					for( int i=0; i < n; i++ ){
						pop(); /* pop top_of_stack */

						#ifdef SEE_ITABLE_CONST
						printf( "(popped 1 element)" );
						getc(stdin);
						#endif
					}
					current_frame--; 
					/* see table construction */
					#ifdef SEE_ITABLE_CONST
					print_itable_const();
					#endif
				}
			}// END NORMAL MODE
		}
		
	}	
	else{ /* IT'S A LEAF */


		if( mode == INITIAL ){		
			/* adds function and global variable names to identification table */
			if( pnode->leaf->token_name == LBL_ID ){

				if( last_node_visited->op == LBL_DECLARATION ||
				    last_node_visited->op == LBL_PARAM  || 
				    last_node_visited->op == LBL_VAR_NAME || last_node_visited->op == LBL_MULTIPARAMS
				   ){ 		/* global variable and function declaration */

					/* sets values */
					level = current_frame;
					DecPtr = pnode;
					LexI = pnode->leaf->table_entry->index;
					next = find_closest_previous( LexI );

					/* updates access table */
					insert_atable( LexI, IdI );

					/* pushes on ident table stack */
					push( IdI, level, DecPtr, next, LexI, true );	
					IdI++;

					#ifdef SEE_ITABLE_CONST
					print_itable_const();
					#endif
				}
			}
		}// end initial mode
		else{ /* NORMAL MODE */
			if( pnode->leaf->token_name == LBL_ID ){
				
				if( last_node_visited->op == LBL_DECLARATION || last_node_visited->op == LBL_COMPOUND_DECLARATION ||
				    last_node_visited->op == LBL_PARAM  || 
				    last_node_visited->op == LBL_VAR_NAME || last_node_visited->op == LBL_MULTIPARAMS
				   ){ 		/* variable and function declaration */

					/* sets values */
					level = current_frame;
					DecPtr = pnode;
					LexI = pnode->leaf->table_entry->index;
					next = find_closest_previous( LexI );

					ITABLE pentry;
					if( ( pentry = find( LexI, level ) ) == NULL ){	/* identifier not found in the iden table*/	
						
						/* updates access table */
						insert_atable( LexI, IdI );
						
						 /* pushes on stack */
						push( IdI, level, DecPtr, next, LexI, false );
						IdI++;
						
						/* see table construction */
						#ifdef SEE_ITABLE_CONST
						print_itable_const();
						#endif
					}
					else{						 /* Identifier found within ident table */
		
						bool redef = false;
						if( pentry->created_on_init_trav == true ){
							pentry->num_of_redefs++;

							if( pentry->num_of_redefs > 1 )
								redef = true;
						}
						else{
							redef = true;
						}
		
						if( redef == true ){
							/* Semantic Error: redefinition of identifier */

							//builds error string 
							char *s = ( char* )calloc( strlen( "redefiniton of identifier ''" ) + 										   strlen( pnode->leaf->table_entry->lexeme ) + 1, sizeof(char) );
							sprintf( s, "redefiniton of identifier '%s'", pnode->leaf->table_entry->lexeme );
							//adds the error the list							
							error_list_add( s, pnode->lineno  );
						}
				
					}
			
				}
				else{ /* APPLIED OCURRENCE OF A VARIABLE, OR A FUNCTION CALL */
					int temp_IdI;
					astnode temp_decnode;

					/* links the current node with its declaration node */
					temp_IdI = retrieve_atable( pnode->leaf->table_entry->index );/*gets the corresponding entry in the ident table */
					temp_decnode = retrieve_decnode_itable( temp_IdI );      /* retrieve node from the entry in the ident table*/
					
					if( temp_decnode == NULL ){
						/* Semantic Error: Identifier Without previous definition*/

						//builds error string 
						char *s = ( char* )calloc( strlen( "indentifier '' without previous definition" ) + 
										   strlen( pnode->leaf->table_entry->lexeme ) + 1,  sizeof(char) );
						sprintf( s, "indentifier '%s' without previous definition", pnode->leaf->table_entry->lexeme );
						//adds the error the list							
						error_list_add( s, pnode->lineno  );

						/* sets itself as universal type and as its own declaration node. to sumplify operations in later steps */
						settype_leaf( pnode->leaf, TYPE_UNIVERSAL ); 
						pnode->leaf->decnode = pnode;	
					}
					else{
						pnode->leaf->decnode = temp_decnode; /* link them */

						#ifdef SEE_ITABLE_CONST
						printf("LINKED %s  \n", pnode->leaf->table_entry->lexeme);
						printf("proof... here is its lexeme on the dec node: %s \n", 
							pnode->leaf->decnode->leaf->table_entry->lexeme);
						getc(stdin);
						#endif
					}
				}
			}//end variable and function declaration or applied ocurrence
		}//end normal mode
		
	}	
}	



void visit_postorder( astnode pnode ){

	/* 
	   Visit the syntax tree nodes in a  first-traversal in preorder at the time it checks for semantic coherence. 

	   It's part of the type analysis

	  1. Traverse left subtree
	  2. traverse center subtree
      	  3. traverse right subtree
          4. Visit node (i.e. do stuff!)
 	*/

	int frame_state = UNCHANGED;		/* tells if the frame number was incremented */

	if( pnode->leaf == NULL ){ 	   /* IT'S A NODE */
		
		

		if( IS_EMPTYNODE(pnode) ){ /* IT'S A LEAVE IMPLEMENTED AS NODE */ /* node visiting */

			/* set types for epsilons */
			if( pnode->op == LBL_EPSILON )
				pnode->type = TYPE_EPSILON;	//pnode->type = TYPE_UNIVERSAL;

		}
		else{ 			   /* IT'S A REAL NODE */
			
				/* traverses childs */
				if( ( pnode->leftptr ) != NULL  ){	
					visit_postorder( pnode->leftptr );
				}
				if( ( pnode->centerptr )  != NULL  ){
					visit_postorder( pnode->centerptr );
				}
				if( ( pnode->rightptr )  != NULL  ){
					visit_postorder( pnode->rightptr );
				}
				
				/* Node visiting for nodes: DO THE TYPE CHECKING */

				if( pnode->op == LBL_ARRAY_OR_CALL )
					pnode->type = pnode->leftptr->leaf->decnode->leaf->type;
				
				TYPE t;
				int res;

				//printf("current node: %s\n", node_labels[ pnode->op + NODE_LABELS_OFFSET ] );

				switch( pnode->op ){
					case LBL_IF_STMT: //it includes both if_stmt and else's
						if( pnode->leftptr->leaf == NULL ){ //it's a node
							if( pnode->leftptr->type != TYPE_BOOL ){
								/* semantic error */						
								error_list_add( "test expressions within 'if' statements must be of type boolean", 
									         pnode->lineno  );	
							}
						}
						else{ //it's a leaf
							if( pnode->leftptr->leaf->decnode->leaf->type != TYPE_BOOL ){
								/* semantic error */
								error_list_add( "test expressions within 'if' statements must be of type boolean", 
									         pnode->lineno  );	
							}
						}

						//checks that stand alone routine calls within if stmts's are void
						if( pnode->centerptr != NULL ){
							if( pnode->centerptr->op == LBL_ROUTINE_CALL ){
								//stand alone routine calls that do not include neither assignments nor comparison
								//... such contructs are the left childs of COMP_STMT_PP, COMP_STMT_PPP and LOOP_STMT
								// and centerchilds of  IF_STMT	
								//this routine calls can only be void!
								if( pnode->centerptr->leftptr->leaf->decnode->leaf->type != TYPE_VOID ){
									/* semantic error */
									error_list_add( "stand-alone call statements can only involve functions of type 'void'", pnode->lineno  );
								}
							}
						}
						
						break;
					case LBL_LT:
					case LBL_LTEQ:
					case LBL_GT:
					case LBL_GTEQ:
					case LBL_EQ:
					case LBL_NEQ:
						pnode->type = TYPE_BOOL;
						break;	
					case LBL_PLUS_MAIN:
					case LBL_MULT_MAIN:
					case LBL_MINUS_MAIN:
					case LBL_OR_MAIN:
					case LBL_ORELSE_MAIN:
					case LBL_DIV_MAIN:
					case LBL_MOD_MAIN:
					case LBL_AND_MAIN:
					case LBL_ANDTHEN_MAIN:
						t = type_synthesis_arithmetic( pnode->leftptr, pnode->centerptr, pnode->rightptr );
						pnode->type = t;
			
						last_expr_val = const_or_var; //saves the vlaue of the current expression, 
									      //which becomes the value for the last expression

						//if( last_expr_val == VAR_VAL )
						//	printf("last expression is VARIABLE\n" );
						//else
						//	printf("last expression is CONSTANT\n" );
						//getc(stdin);
						const_or_var = CONST_VAL;//resets value for next expression evaluation
						break;
					case LBL_PLUS:
					case LBL_MULT:
					case LBL_MINUS:
					case LBL_OR:
					case LBL_ORELSE:
					case LBL_DIV:
					case LBL_MOD:
					case LBL_AND:
					case LBL_ANDTHEN:
						t = type_synthesis_arithmetic( pnode->leftptr, pnode->centerptr, NULL );
						pnode->type = t;
						break;
					case LBL_ASSIGN:
						if( pnode->leftptr->leaf != NULL ){//l-value	
							if( pnode->leftptr->leaf->decnode->leaf->is_array == true ){
								// semantic error

								//builds error string 
								char *s = ( char* )calloc( 
										   strlen( "missing index for array '' in left side of assignment" ) +
										   strlen(  pnode->leftptr->leaf->decnode->leaf->table_entry->lexeme  ) + 1,  
										   sizeof(char) 
										   );
								sprintf( s, "missing index for array '%s' in left side of assignment", 	
									 pnode->leftptr->leaf->decnode->leaf->table_entry->lexeme );
								//adds the error the list							
								error_list_add( s, pnode->lineno  );	
							}
						}
						if( pnode->centerptr->leaf != NULL ){//r-value						
							if( pnode->centerptr->leaf->decnode->leaf->is_array == true ){
								// semantic error
								

								//builds error string 
								char *s = ( char* )calloc( 
										   strlen( "missing index for array '' in right side of assignment" ) +
										   strlen(  pnode->centerptr->leaf->decnode->leaf->table_entry->lexeme  ) + 
										   1,  
										   sizeof(char) 
										   );
								sprintf( s, "missing index for array '%s' in right side of assignment", 	
									 pnode->centerptr->leaf->decnode->leaf->table_entry->lexeme );
								//adds the error the list							
								error_list_add( s, pnode->lineno  );
							}
						}

						t = type_synthesis_assign( pnode->leftptr, pnode->centerptr );
						pnode->type = t;
						break;
					case LBL_ARRAY_OR_CALL:
						if( pnode->centerptr->op == LBL_ROUTINE_CALL || pnode->centerptr->op == LBL_NO_ARGUMENTS ){//func call
							//for assignments using functions as R-Value
							res = check_param_consistency( pnode->leftptr, pnode->centerptr, IN_ASSIGN );//id leaf, node of parameters in call		
						}
						else{ //applied use of an array
							if( pnode->centerptr->leaf->decnode->leaf->type != TYPE_INT ){
								//semantic error
								error_list_add( "array indices must be of type 'int'", pnode->lineno  );
							}
						}
						break;
					case LBL_ROUTINE_CALL: //for stand-alone routine call
						if( pnode->rightptr != NULL ){ /* this differentiates stand alone routine calls and routines calls tha
										  are childs of an assignemnet... assignemtns have rightptr null */
								//note: calls used for logical comparations e.g. if( a = bigger(a,b) )...
								//      are also seen as stand alone. in fact all of them act as stand alone, except
								//	for those in assignments

							//check param consistency
							res = check_param_consistency( pnode->leftptr, pnode, STAND_ALONE );
					
						}
						break;
					case LBL_NO_ARGUMENTS: //for stand-alone routine call...without parameteres

						//check param consistency
						res = check_param_consistency( pnode->leftptr, pnode, STAND_ALONE );
						break;
					case LBL_RETURN:
						if( pnode->leftptr->op == LBL_EPSILON )
							return_type = TYPE_VOID;
						else // num, identifier, blit 
							return_type = pnode->leftptr->leaf->decnode->leaf->type; 
						
						break;
					case LBL_NULL:
							return_type = TYPE_VOID;
						break;
					case LBL_DECLARATION:
						/* links id's definitions that are arrays with their index value */
						/*
						if( pnode->centerptr->op == LBL_ARRAY ){
							pnode->leftptr->leaf->is_array = true;			      // official en array
							pnode->leftptr->leaf->arrayinode = pnode->centerptr->leftptr; //[id] -- linked --> [num]|(add-exp)
						}*/
							
						/* checks for empty compound statement */
						if( pnode->centerptr->op == LBL_FUN_DEC_TAIL ){//it's a function
							 if(empty_compound_stmt){
								/* semantic error */
		
								//builds error string 
								char *s = ( char* )calloc( 
										   strlen( "function '' cannot be empty" ) +  
										   strlen( pnode->leftptr->leaf->table_entry->lexeme  ) + 
										   1,  
										   sizeof(char) 
										   );
								sprintf( s, "function '%s' cannot be empty", 	
									 pnode->leftptr->leaf->table_entry->lexeme );
								//adds the error the list							
								error_list_add( s, pnode->lineno  );
							}
							 empty_compound_stmt = true; //reinits var for next function
						}
			
						/* 1 checks type matching on return */
						if( pnode->centerptr->op == LBL_FUN_DEC_TAIL ){ // it's a function declaration 
								
							func_type = pnode->leftptr->leaf->type;

							if( func_type != return_type ){								

								//builds error string 
								char *s = ( char* )calloc( 
										   strlen( "mismatch on return type in function ''" ) +  
										   strlen( pnode->leftptr->leaf->table_entry->lexeme  ) + 
										   1,  
										   sizeof(char) 
										   );
								sprintf( s, "mismatch on return type in function '%s'", 	
									 pnode->leftptr->leaf->table_entry->lexeme );
								//adds the error the list							
								error_list_add( s, pnode->lineno  );



							} 
							//reset type
							return_type = TYPE_VOID;


						}
						break;
					case LBL_COMPOUND_STMT:
					case LBL_COMPOUND_STMT_PP:
					case LBL_COMPOUND_STMT_PPP:
						if( pnode->leftptr != NULL ){ //leftptr being null should never happen, but if somehow it does... 
									     //would take me hours to get here
							if( pnode->leftptr->op != LBL_EPSILON ){
								 empty_compound_stmt = false; //with one comp_stmt or comp_stmt_pp or comp_stmt_ppp
											      //not deriving epsilon is enough to determine that the
											      // function does not have the form "...func(...){}"
											      //the variable is tested later at declaration node
							}
						}	
						
						//break; note: there is no break on pourpose!
					case LBL_LOOP_STMT:
						//checks that stand alone routine calls are void
						if( pnode->leftptr != NULL ){
							if( pnode->leftptr->op == LBL_ROUTINE_CALL ){
								//stand alone routine calls that do not include neither assignments nor comparison
								//... such contructs are the left childs of COMP_STMT_PP, COMP_STMT_PPP and LOOP_STMT
								// and centerchilds of  IF_STMT	
								//this routine calls can only be void!
								if( pnode->leftptr->leftptr->leaf->decnode->leaf->type != TYPE_VOID ){
									/* semantic error */
									error_list_add( "stand-alone call statements can only involve functions of type 'void'", pnode->lineno  );
								}
							}
						}
						break;

					case LBL_ARRAY: //array declarations, both global and within a compound statement
						if( pnode->leftptr->leaf != NULL ){//leaf

							if( pnode->leftptr->leaf->token_name == LBL_ID ){
								//semantic error
								error_list_add( "array sizes must be constant", pnode->lineno  );
							}
							else if( pnode->leftptr->leaf->decnode->leaf->type != TYPE_INT ){
								//semantic error
								error_list_add( "array sizes in definitions must be type 'int'", pnode->lineno  );
							}

						}
						else{//node
			
							//if( last_expr_val == VAR_VAL )
							//	printf("array index is VARIABLE\n" );
							//else
							//	printf("array index is CONSTANT\n" );

							//getc(stdin);
							if( last_expr_val == VAR_VAL ){//last expression evaluated contained a variable
								//semantic error
								error_list_add( "array sizes must be constant", pnode->lineno  );

							}
							if( pnode->leftptr->op == LBL_ARRAY_OR_CALL ){ //trying to use an array or a 
												       //function to define array size
								//semantic error
								error_list_add( "array sizes must be constant", pnode->lineno  );
							}	
							else if( pnode->leftptr->type != TYPE_INT ){ //an expression
								//semantic error
								error_list_add( "array sizes in definitions must be type 'int'", pnode->lineno  );
							}

						}
						break;
					case LBL_ARRAY_ASSIGN: //array assign
						if( pnode->rightptr->leaf != NULL ){
							if( pnode->rightptr->leaf->decnode->leaf->is_array == true ){
								//semantic error

								//builds error string 
								char *s = ( char* )calloc( 
										   strlen( "missing index for array '' in right side of assignment" ) +
										   strlen(  pnode->rightptr->leaf->decnode->leaf->table_entry->lexeme  ) +
										   1,  
										   sizeof(char) 
										   );
								sprintf( s, "missing index for array '%s' in right side of assignment", 	
									 pnode->rightptr->leaf->decnode->leaf->table_entry->lexeme );
								//adds the error the list							
								error_list_add( s, pnode->lineno  );
							}	
					
						}
					

						t = type_synthesis_assign( pnode->leftptr, pnode->rightptr );
						pnode->type = t;

						if( pnode->centerptr->op == LBL_ARRAY_OR_CALL ){ //the index is an array
							if( pnode->centerptr->leftptr->leaf->decnode->leaf->type != TYPE_INT ){
								//semantic error
								error_list_add( "array sizes in definitions must be type 'int'", pnode->lineno  );
							}
							
						}
						else if( pnode->centerptr->op == LBL_ID || pnode->centerptr->op == LBL_NUM || 
							 pnode->centerptr->op == LBL_BLIT ){ //the index is a num, id, blit
							if( pnode->centerptr->leaf->decnode->leaf->type != TYPE_INT ){
								//semantic error
								error_list_add( "array sizes in definitions must be type 'int'", pnode->lineno  );
							}
						}
						else{ //add-exp
							if( pnode->centerptr->type != TYPE_INT ){
								//semantic error
								error_list_add( "array sizes in definitions must be type 'int'", pnode->lineno  );
							}

						}
							 
						break;
					default:
						break;


				}//------------------- END SWITCH -----------------------

			
				//printf("current node: %s\n", node_labels[ pnode->op + NODE_LABELS_OFFSET ] );
		
				// ERROR FOUND NOTIFICATION
				if( res == FUNC_NUM_MISMATCH ) {
					res = FUNC_SUCC;
					/* ERROR: semantic error */

					//builds error string 
					char *s = ( char* )calloc( strlen( "function '' -> number of parameters in definition and call does not match" ) +
								   strlen(  pnode->leftptr->leaf->table_entry->lexeme  ) +
				  				   1,  
								   sizeof(char) 
							         );
					sprintf( s, "function '%s' -> number of parameters in definition and call does not match", 
					         pnode->leftptr->leaf->table_entry->lexeme );
					//adds the error the list							
					error_list_add( s, pnode->lineno  );

				}
				else if( res == FUNC_TYPE_MISMATCH ){
					res = FUNC_SUCC;
					/* ERROR: semantic error */

					//builds error string 
					char *s = ( char* )calloc( strlen( "function '' -> parameters type mismatch in definition and call" ) +
								   strlen(  pnode->leftptr->leaf->table_entry->lexeme  ) +
				  				   1,  
								   sizeof(char) 
							         );
					sprintf( s, "function '%s' -> parameters type mismatch in definition and call", 
					        pnode->leftptr->leaf->table_entry->lexeme );
					//adds the error the list							
					error_list_add( s, pnode->lineno  );
				}
				else if( res == FUNC_TYPE_ARRAY_EXPECTED ){
					res = FUNC_SUCC;
					/* ERROR: semantic error */
					//builds error string 
					char *s = ( char* )calloc( strlen( "function '' -> parameters type mismatch in definition and call; an array was expected" ) +
								   strlen(  pnode->leftptr->leaf->table_entry->lexeme  ) +
				  				   1,  
								   sizeof(char) 
							         );
					sprintf( s, "function '%s' -> parameters type mismatch in definition and call; an array was expected", 
					        pnode->leftptr->leaf->table_entry->lexeme );
					//adds the error the list							
					error_list_add( s, pnode->lineno  );
				}
				else if( res == FUNC_TYPE_REF_MISMATCH ){
					res = FUNC_SUCC;
					/* ERROR: semantic error */
					
					//builds error string 
					char *s = ( char* )calloc( strlen( "function '' -> l-expression expected as parameter" ) +
								   strlen(  pnode->leftptr->leaf->table_entry->lexeme  ) +
				  				   1,  
								   sizeof(char) 
							         );
					sprintf( s, "function '%s' -> l-expression expected as parameter", 
					        pnode->leftptr->leaf->table_entry->lexeme );
					//adds the error the list							
					error_list_add( s, pnode->lineno  );

				}
				else if( res == FUNC_TYPE_ARRAY_CANNOT_BE_REF ){
					res = FUNC_SUCC;
					/* ERROR: semantic error */

					//builds error string 
					char *s = ( char* )calloc( strlen( "function '' -> array expressions cannot be passed by reference" ) +
								   strlen(  pnode->leftptr->leaf->table_entry->lexeme  ) +
				  				   1,  
								   sizeof(char) 
							         );
					sprintf( s, "function '%s' -> array expressions cannot be passed by reference", 
					        pnode->leftptr->leaf->table_entry->lexeme );
					//adds the error the list							
					error_list_add( s, pnode->lineno  );
				}
				else if( res == FUNC_TYPE_ARRAY_REF_PARAMS ){
					res = FUNC_SUCC;
					/* ERROR: semantic error */
					//builds error string 
					char *s = ( char* )calloc( strlen( "function '' -> array parameters cannot be passed by reference" ) +
								   strlen(  pnode->leftptr->leaf->table_entry->lexeme  ) +
				  				   1,  
								   sizeof(char) 
							         );
					sprintf( s, "function '%s' -> array parameters cannot be passed by reference", 
					        pnode->leftptr->leaf->table_entry->lexeme );
					//adds the error the list							
					error_list_add( s, pnode->lineno  );
				}
				else if( res == FUNC_TYPE_ARRAY_AS_ARGUMENT ){
					res = FUNC_SUCC;
					/* ERROR: semantic error */
					error_list_add( "Warning: This is not implemented yet! A value is expected, just make sure you are not passing the array pointer", pnode->lineno  );
				}
				else if( res == FUNC_NO_DEF_FOUND ){
					res = FUNC_SUCC;
					/* ERROR: program error */
					printf("INTERNAL CODE MISTAKE... no definition found\n");
					getc(stdin);
				}
				else if( res == FUNC_NO_PARAM_FOUND ){
					res = FUNC_SUCC;
					/* ERROR: program error */
					printf("INTERNAL CODE MISTAKE... no params found\n");
					getc(stdin);
				}
				if( pnode->type == TYPE_FUNC_ERROR ){
					pnode->type = TYPE_UNIVERSAL;
					error_list_add( "a function cannot be used as l-value", pnode->lineno  );
				}
				else if( pnode->type == TYPE_VOID_ERROR ){
					pnode->type = TYPE_UNIVERSAL;
					error_list_add( "a 'void' function cannot be used as r-value", pnode->lineno  );
				
				}
				else if( pnode->type == TYPE_ERROR ){
					pnode->type = TYPE_UNIVERSAL;
					/* ERROR: type mismatch */
					
					//builds error string--------------------------- 	
					//--------------------------------------------------------------
					char *s = ( char* )calloc( 121, sizeof(char) );					

					if( pnode->leftptr != NULL ){
						if( pnode->leftptr->op == LBL_NOT ){
							sprintf( s, "identifier '%s' does not match type of expression", 
					        		 pnode->leftptr->leftptr->leaf->decnode->leaf->table_entry->lexeme );
						}
						else if( pnode->leftptr->leaf != NULL ){ //it's leaf
							sprintf( s, "identifier '%s' does not match type of expression", 
					        		 pnode->leftptr->leaf->decnode->leaf->table_entry->lexeme );
						}
						else{//it's node
							if( pnode->leftptr->op  == LBL_ARRAY_OR_CALL ){
								sprintf( s, "identifier '%s' does not match type of expression", 
					        		 pnode->leftptr->leftptr->leaf->decnode->leaf->table_entry->lexeme );
							}
							else{
								//sprintf( s, "subexpression does not match type of expression" );
								sprintf( s, "an identifier within the expression has different type" ); 
							}
						}
					}
					if( pnode->centerptr != NULL ){
						if( pnode->centerptr->op == LBL_NOT ){
							sprintf( s, "identifier '%s' does not match type of expression", 
					        		 pnode->centerptr->leftptr->leaf->decnode->leaf->table_entry->lexeme );
						}
						else if( pnode->centerptr->leaf != NULL ){ //it's leaf
							sprintf( s, "identifier '%s' does not match type of expression", 
					        		 pnode->centerptr->leaf->decnode->leaf->table_entry->lexeme );
						}
						else{//it's node	
							if( pnode->centerptr->op == LBL_ARRAY_OR_CALL ){
								sprintf( s, "identifier '%s' does not match type of expression", 
					        		 pnode->centerptr->leftptr->leaf->decnode->leaf->table_entry->lexeme );
							}
							else{
								//sprintf( s, "subexpression does not match type of expression" );
								sprintf( s, "an identifier within the expression has different type" ); 
							}
						}
					}
					if( pnode->rightptr != NULL ){
						if( pnode->rightptr->op == LBL_NOT ){
							sprintf( s, "identifier '%s' does not match type of expression", 
					        		pnode->rightptr->leftptr->leaf->decnode->leaf->table_entry->lexeme );

						}
						if( pnode->rightptr->leaf != NULL ){ //it's leaf
							sprintf( s, "identifier '%s' does not match type of expression", 
					        		pnode->rightptr->leaf->decnode->leaf->table_entry->lexeme );
						}
						else{//it's node	
							if( pnode->rightptr->op == LBL_ARRAY_OR_CALL ){
								sprintf( s, "identifier '%s' does not match type of expression", 
					        		pnode->rightptr->leftptr->leaf->decnode->leaf->table_entry->lexeme );
							}
							else{
								//sprintf( s, "subexpression does not match type of expression" );
								sprintf( s, "an identifier within the expression has different type" ); 
					  
							}
						}
					}



					//adds the error the list------------------------------
					//--------------------------------------------------------------------							
					error_list_add( s, pnode->lineno  );



				}//END ERROR TYPE NOTIFICATION

		}//end else

	}	
	else{ /* IT'S A LEAF */ /* node visiting*/

		/* set nums and blits decnodes as themselves. allows to treat all tokens without distinction */
		if( pnode->leaf->token_name == LBL_NUM || pnode->leaf->token_name == LBL_BLIT)
			pnode->leaf->decnode = pnode;
	}	
}	

int check_param_consistency( astnode id, astnode params, int mode ){
	/* checks consistency in number of parameters, and type of parameters between definition and call
	   of a function  
	   maximum number of arguments setted arbitrarily to 31
	*/	
	int num_in_def, num_in_call;
	TYPE types_in_def[31], types_in_call[31];
	bool isarray_in_def[31], isarray_in_call[31];
	astnode params_in_definition;
	astnode params_in_call;
	int j = 0;

	/*initializes arrays */
	for( int i=0; i<31; i++ ){ 
		types_in_def[i] = -1;
		types_in_call[i] = -1;
		isarray_in_def[i] = false;
		isarray_in_call[i] = false;
	}

	params_in_definition = id->leaf->decnode->leaf->paramnode;
	params_in_call = params;
	//printf("lexeme: %s\n", id->leaf->decnode->leaf->table_entry->lexeme );

	if( params_in_definition == NULL )
		return FUNC_NO_DEF_FOUND;


	/* gets number and types from definition of function */
		if( params_in_definition->op == LBL_VOID ){

			num_in_def = 0;

		}
		else{

			for( num_in_def=1; num_in_def<32; num_in_def++ ){
				//obtans types of parameters
				types_in_def[num_in_def-1] = params_in_definition->leftptr->leftptr->leaf->type;

				//is it array?
				if( params_in_definition->leftptr->leftptr->leaf->token_name == LBL_ID )
					isarray_in_def[num_in_def-1] = params_in_definition->leftptr->leftptr->leaf->is_array;

				//arrays parameters cannot be passed by reference
				if( isarray_in_def[num_in_def-1] == true && 
			            ( types_in_def[num_in_def-1] == TYPE_REF_INT || types_in_def[num_in_def-1] == TYPE_REF_BOOL ) 
				   ){
					return FUNC_TYPE_ARRAY_REF_PARAMS; //here... return the line number of the function definition, not the call!
				}

				if( params_in_definition->centerptr->op == LBL_MULTIPARAMS )//more params?
					params_in_definition = params_in_definition->centerptr;	//next param
				else
					break;			
			}
		}
	
		#ifdef PARAM_CONSIST_DEBUG
		printf("IN DEFINITION\n");
		printf("identifier: %s  ", id->leaf->table_entry->lexeme );
		printf("number of parameters :%d   ", num_in_def );
		printf("   types:" );
	
		j=0;
		while( types_in_def[j] != -1 ){
			printf("%s, ", types[ ( types_in_def[j] ) + TYPES_OFFSET ] );
			j++;
		}
		printf("\n");
		getc(stdin);
		#endif

		/* gets number and types from function call */
		
		if( params_in_call->op == LBL_NO_ARGUMENTS ){
			num_in_call = 0;

		}
		else{
			if( mode == IN_ASSIGN ){
				/* in assign mode */
				//printf("_a_\n");
				for( num_in_call=1; num_in_call<32; num_in_call++ ){
					//obtans types of parameters
					if( params_in_call->leftptr->op == LBL_ARRAY_OR_CALL ){//the argument is an array or a function
						//printf("_c_\n");
						//printf( "node labels: %s --> %s \n", node_labels[ params_in_call->leftptr->op + NODE_LABELS_OFFSET ], 
						//   node_labels[ params_in_call->leftptr->leftptr->leaf->decnode->leaf->token_name + NODE_LABELS_OFFSET ] );
						types_in_call[num_in_call-1] = params_in_call->leftptr->leftptr->leaf->decnode->leaf->type;

						//is it array?? 
						if( params_in_call->leftptr->leftptr->leaf->decnode->leaf->token_name == LBL_ID )
							isarray_in_call[num_in_call-1] = params_in_call->leftptr->leftptr->leaf->decnode->leaf->is_array;
					}
					else{ //the argument is an id, num or blit
						//printf("_b_\n");
						types_in_call[num_in_call-1] = params_in_call->leftptr->leaf->decnode->leaf->type;

						//is it array?
						//if( params_in_call->leftptr->leaf->decnode->leaf->token_name == LBL_ID ){
						//	isarray_in_call[num_in_call-1] = params_in_call->leftptr->leaf->decnode->leaf->is_array;
						//}

						//if the corresponding parameter in the func definition is intended to be passed by reference "ref"
						//then, this argument in the routine call must be an l-value
						if( types_in_def[num_in_call-1] == TYPE_REF_INT || types_in_def[num_in_call-1] == TYPE_REF_BOOL ){
							if( params_in_call->leftptr->leaf->token_name != LBL_ID ){ 
								//it's not an id (l-expression)		
								return FUNC_TYPE_REF_MISMATCH;
							}
							else{ //it's an id
								if( isarray_in_call[num_in_call-1] == true )
									return FUNC_TYPE_ARRAY_CANNOT_BE_REF;
							}
						}


					}
					//printf("salio =)\n");
					

					if( params_in_call->centerptr->op == LBL_ARGUMENTS_P )//more params?
						params_in_call = params_in_call->centerptr;	//next param
					else
						break;						// no more params
				}
			}
			else{
				/* stand-alonde mode */

				//the first argument
				types_in_call[0] = params_in_call->centerptr->leaf->decnode->leaf->type;
				num_in_call = 1;

				//is it array?
				if( params_in_call->centerptr->leaf->decnode->leaf->token_name == LBL_ID ){	
					//printf("c\n"); getc(stdin);
					isarray_in_call[0] = params_in_call->centerptr->leaf->decnode->leaf->is_array;
				}

				//if the corresponding parameter in the definition is intended to be passed by reference "ref"
				//then, this argument in the routine call must be an l-value
				if( types_in_def[0] == TYPE_REF_INT || types_in_def[0] == TYPE_REF_BOOL ){
					if( params_in_call->centerptr->leaf->decnode->leaf->token_name != LBL_ID ){		
							return FUNC_TYPE_REF_MISMATCH;
					}
					else{
						if( isarray_in_call[0] == true )
							return FUNC_TYPE_ARRAY_CANNOT_BE_REF;
					}
				}
				//the rest of the arguments			
				if( params_in_call->rightptr->op == LBL_ARGUMENTS_P )
				{

				
					params_in_call = params_in_call->rightptr;


					for( num_in_call=2; num_in_call<32; num_in_call++ ){
						//obtans types of parameters
						types_in_call[num_in_call-1] = params_in_call->leftptr->leaf->decnode->leaf->type;

						//is it array?
						if( params_in_call->leftptr->leaf->decnode->leaf->token_name == LBL_ID ){	
							isarray_in_call[num_in_call-1] = params_in_call->leftptr->leaf->decnode->leaf->is_array;
						}

						//if the corresponding parameter in the definition is intended to be passed by reference "ref"
						//then, this argument in the routine call must be an l-value
						if( types_in_def[num_in_call-1] == TYPE_REF_INT || types_in_def[num_in_call-1] == TYPE_REF_BOOL ){
							if( params_in_call->leftptr->leaf->token_name != LBL_ID ){	
									return FUNC_TYPE_REF_MISMATCH;
							}
							else{
								if( isarray_in_call[num_in_call-1] == true )
									return FUNC_TYPE_ARRAY_CANNOT_BE_REF;
							}
						}
					

						if( params_in_call->centerptr->op == LBL_ARGUMENTS_P )//more params?
							params_in_call = params_in_call->centerptr;	//next param
						else
							break;						// no more params
					}

				}
			}
		}
		
		#ifdef PARAM_CONSIST_DEBUG
		printf("IT WAS CALLED LIKE THIS: \n");
		printf("number of parameters :%d   ", num_in_call );
		printf("   types:" );
	
		j=0;
		while( types_in_call[j] != -1 ){
			printf("%s", types[ ( types_in_call[j] ) + TYPES_OFFSET ] );
			if( isarray_in_call[j] == true )
				printf( "(array)");

			printf(",   ");
			j++;
		}
		printf("\n");
		getc(stdin);
		#endif

		/*compare */
		if( num_in_def != num_in_call ) //number mismatch
			return FUNC_NUM_MISMATCH;

			//array mismatch

		j=0;
		while( types_in_call[j] != -1 ){

			//converts ref types in defs to normal types, just for comparison... "ref" token is not allowed in function call
			if( types_in_def[j] == TYPE_REF_INT ){ 
				types_in_def[j] = TYPE_INT;
				#ifdef PARAM_CONSIST_DEBUG
				printf("type temporary changed to int");
				getc(stdin);
				#endif
			}
			if( types_in_def[j] == TYPE_REF_BOOL ) {
				types_in_def[j] = TYPE_BOOL; 
				#ifdef PARAM_CONSIST_DEBUG
				printf("type temporary changed to bool");
				getc(stdin);
				#endif
			}

			//compares
			if( types_in_call[j] != types_in_def[j] ) return FUNC_TYPE_MISMATCH;
			if( isarray_in_call[j] != isarray_in_def[j] ){ 
				if( isarray_in_def[j] == true )
					return FUNC_TYPE_ARRAY_EXPECTED;
				else
					return FUNC_TYPE_ARRAY_AS_ARGUMENT;
			}

			j++;
		}

		#ifdef PARAM_CONSIST_DEBUG
		printf("_SALIDA__\n");
		getc(stdin);
		#endif
	
	return FUNC_SUCC;

}

TYPE type_synthesis_assign( astnode leftside, astnode rightside ){

	/* 
		Performs type synthesis for nodes that aassignments, including those following arrays.

	*/

	if( leftside->leaf->decnode->leaf->paramnode != NULL ) //it's a function!
		return TYPE_FUNC_ERROR;		/* functions cannot be l-value */

	


	if( rightside->leaf == NULL ){//node
		if( rightside->op == LBL_ARRAY_OR_CALL ){ /* if r-value is a function or array */
			if( rightside->centerptr->op == LBL_NO_ARGUMENTS || rightside->centerptr->op == LBL_ROUTINE_CALL ){/* if r-value is a function */
				if( rightside->leftptr->leaf->decnode->leaf->type == TYPE_VOID )
					return TYPE_VOID_ERROR;
			}
			
		}

		if( rightside->type == TYPE_UNIVERSAL )
			return TYPE_UNIVERSAL;	

		//changes ref_int just for the sake of comparison
		TYPE temp_left, temp_right;
		temp_left = leftside->leaf->decnode->leaf->type;
		temp_right = rightside->type;
			//left side
		if( temp_left == TYPE_REF_BOOL ) 
			temp_left = TYPE_BOOL;
		else if( temp_left == TYPE_REF_INT )
			temp_left = TYPE_INT;
			//right side
		if( temp_right == TYPE_REF_BOOL ) 
			temp_right = TYPE_BOOL;
		else if( temp_right == TYPE_REF_INT )
			temp_right = TYPE_INT;
		
	
		if( temp_left == temp_right )
			return temp_left;
		else
			return TYPE_ERROR;		

	}
	else{ // leaf
		if( rightside->leaf->decnode->leaf->type == TYPE_UNIVERSAL )
			return TYPE_UNIVERSAL;	


		//changes ref_int just for the sake of comparison
		TYPE temp_left, temp_right;
		temp_left = leftside->leaf->decnode->leaf->type;
		temp_right = rightside->leaf->decnode->leaf->type;
			//left side
		if( temp_left == TYPE_REF_BOOL ) 
			temp_left = TYPE_BOOL;
		else if( temp_left == TYPE_REF_INT )
			temp_left = TYPE_INT;
			//right side
		if( temp_right == TYPE_REF_BOOL ) 
			temp_right = TYPE_BOOL;
		else if( temp_right == TYPE_REF_INT )
			temp_right = TYPE_INT;	

		if( temp_left == temp_right )
			return temp_left;
		else
			return TYPE_ERROR;	

	}

	

}

TYPE type_synthesis_arithmetic( astnode p, astnode q, astnode r ){

	/* 
		Performs type synthesis for nodes that are arithmetic and logical operators 

	*/

	TYPE p_type, q_type, r_type;


	/* if p == q and q == r, then p == r 

	*/
	if( p == NULL || q == NULL )
		return TYPE_ERROR;

	// -- obtains type of p --
	if( p->op == LBL_NOT ){
			p_type = p->leftptr->leaf->decnode->leaf->type;

		}

	else if( p->leaf != NULL ){ //it's a leave
			//checks for arrays without index
			if( p->leaf->decnode->leaf->is_array == true ){
				//semantic error

				//builds error string 
				char *s = ( char* )calloc( strlen( "missing index for array ''" ) + 								   strlen( p->leaf->decnode->leaf->table_entry->lexeme ) + 1, sizeof(char) );
				sprintf( s, "missing index for array '%s'", p->leaf->decnode->leaf->table_entry->lexeme );
				//adds the error the list							
				error_list_add( s, p->lineno  );
			}
		
			//sets if the expression is constant or variable(i.e. at least one lement is a variable)
			if( p->leaf->token_name == LBL_ID ){
				const_or_var = VAR_VAL;
				//printf("variable found: %s\n", p->leaf->decnode->leaf->table_entry->lexeme );
				//getc(stdin);
			}

			//gets type
			p_type = p->leaf->decnode->leaf->type;
	}
	else{//it's a node

			
			
			if( p->op == LBL_ARRAY_OR_CALL ){ /* if r-value is a function or array */

					//checks for void functions
					if( p->centerptr->op == LBL_NO_ARGUMENTS || p->centerptr->op == LBL_ROUTINE_CALL ){
						/* if r-value is a function */
						if( p->leftptr->leaf->decnode->leaf->type == TYPE_VOID )
							return TYPE_VOID_ERROR;
					}
					//sets if the expression is constant or variable(i.e. at least one lement is a variable)
					if( p->leftptr->leaf != NULL ){ //is and id
						if( p->leftptr->leaf->token_name == LBL_ID ){
							const_or_var = VAR_VAL;
							//printf("variable found: %s\n", p->leftptr->leaf->decnode->leaf->table_entry->lexeme );
							//getc(stdin);
						}
					}
			
			}
			p_type = p->type; 

	}
	// -- obtains type of q --

	if( q->op == LBL_NOT ){
			q_type = q->leftptr->leaf->decnode->leaf->type;
		}
	else if( q->leaf != NULL ){ //it's a leave

			//checks for array without index
			if( q->leaf->decnode->leaf->is_array == true ){
				//semantic error

				//builds error string 
				char *s = ( char* )calloc( strlen( "missing index for array ''" ) + 								   strlen( q->leaf->decnode->leaf->table_entry->lexeme ) + 1, sizeof(char) );
				sprintf( s, "missing index for array '%s'", q->leaf->decnode->leaf->table_entry->lexeme );
				//adds the error the list							
				error_list_add( s, q->lineno  );
			}

			//sets if the expression is constant or variable(i.e. at least one lement is a variable)
			if( q->leaf->token_name == LBL_ID ){
				const_or_var = VAR_VAL;
				//printf("variable found: %s\n", q->leaf->decnode->leaf->table_entry->lexeme );
				//getc(stdin);
			}
		
			//gets type
			q_type = q->leaf->decnode->leaf->type;
	}
	else{//it's a node
			if( q->op == LBL_ARRAY_OR_CALL ){ /* if r-value is a function or array */

					//checks for void functions
					if( q->centerptr->op == LBL_NO_ARGUMENTS || q->centerptr->op == LBL_ROUTINE_CALL ){
						/* if r-value is a function */
						if( q->leftptr->leaf->decnode->leaf->type == TYPE_VOID )
							return TYPE_VOID_ERROR;
					}
					//sets if the expression is constant or variable(i.e. at least one lement is a variable)
					if( q->leftptr->leaf != NULL ){ //is and id
						if( q->leftptr->leaf->token_name == LBL_ID ){
							const_or_var = VAR_VAL;
							//printf("variable found: %s\n", q->leftptr->leaf->decnode->leaf->table_entry->lexeme );
							//getc(stdin);
						}
					}


			
			}
			q_type = q->type; 
	}

	// -- obtains type of r --
	if( r != NULL ){

		if( r->op == LBL_NOT ){
				r_type = r->leftptr->leaf->decnode->leaf->type;
			}

		else if( r->leaf != NULL ){ //it's a leave

				//checks for arrays without index
				if( r->leaf->decnode->leaf->is_array == true ){
					//semantic error

					//builds error string 
					char *s = ( char* )calloc( strlen( "missing index for array ''" ) + 								   	   strlen( r->leaf->decnode->leaf->table_entry->lexeme ) + 1, sizeof(char) );
					sprintf( s, "missing index for array '%s'", r->leaf->decnode->leaf->table_entry->lexeme );
					//adds the error the list							
					error_list_add( s, r->lineno  );
				}

				//sets if the expression is constant or variable(i.e. at least one lement is a variable)
				if( r->leaf->token_name == LBL_ID ){
					const_or_var = VAR_VAL;

					//printf("variable found: %s\n", r->leaf->decnode->leaf->table_entry->lexeme );
					//getc(stdin);
				}

				//gets type
				r_type = r->leaf->decnode->leaf->type;
		}
		else{//it's a node
				if( r->op == LBL_ARRAY_OR_CALL ){ /* if r-value is a function or array */

					//checks for voids functions
					if( r->centerptr->op == LBL_NO_ARGUMENTS || r->centerptr->op == LBL_ROUTINE_CALL ){
						/* if r-value is a function */
						if( r->leftptr->leaf->decnode->leaf->type == TYPE_VOID )
							return TYPE_VOID_ERROR;
					}
					//sets if the expression is constant or variable(i.e. at least one lement is a variable)
					if( r->leftptr->leaf != NULL ){ //is and id
						if( r->leftptr->leaf->token_name == LBL_ID ){
							const_or_var = VAR_VAL;
							//printf("variable found: %s\n", r->leftptr->leaf->decnode->leaf->table_entry->lexeme );
							//getc(stdin);
						}
					}
		
			
				}
				r_type = r->type; 
		}
	}
	else{
		r_type = -1;
	}


	//CHANGES "ref" types just for comparison
	if( p_type == TYPE_REF_BOOL )p_type = TYPE_BOOL;
	else if( p_type == TYPE_REF_INT )p_type = TYPE_INT;
	if( q_type == TYPE_REF_BOOL )q_type = TYPE_BOOL;
	else if( q_type == TYPE_REF_INT )q_type = TYPE_INT;
	if( r_type == TYPE_REF_BOOL )r_type = TYPE_BOOL;
	else if( r_type == TYPE_REF_INT )r_type = TYPE_INT;


	// COMPARES
	if( p_type == TYPE_UNIVERSAL || q_type == TYPE_UNIVERSAL || r_type == TYPE_UNIVERSAL ) /* an element down chain was setted universal */
	{
		return TYPE_UNIVERSAL;
 
	}
	else if( r_type != -1 ){// 3 elements
		if( p_type == TYPE_EPSILON ){
			if( q_type == r_type ) return q_type; // q = r and p = e
		}
		else if( q_type == TYPE_EPSILON ){
			if( p_type == r_type ) return p_type; // p = r and q = e
		}
		else if( r_type == TYPE_EPSILON ){
			if( p_type == q_type) return p_type; // p = q and r = e
		}
		else if( p_type == q_type && q_type == r_type )// todos iguales
			return p_type;
	}
	else{//2 elements
		if( p_type == TYPE_EPSILON )	  //p is alone
			return q_type;
		else if( q_type == TYPE_EPSILON ) //q is alone
			return p_type;
		else if( p_type == q_type  )	  //p = q 
			return p_type;	
	}

	return TYPE_ERROR;
}

