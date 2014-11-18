/*
 * ident_table.c
 * 
 * 
 *
 * IDENTIFICATION TABLE MODULE:
 *	The identification table described in the guidelines. It's implemented as a stack
 *
 *
 *
 * DEPENDENCIES
 * 	ident_table.h, access_table.o, ast.o, c12.h
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "c12.h"
#include "ast.h"
#include "ident_table.h"
#include "access_table.h"

static ITABLE create( int, int, astnode, int, int, int );
static int get_idindex( ITABLE );	   
static int get_level( ITABLE );	    
static astnode get_decptr( ITABLE );
static int get_lexindex( ITABLE );
static int get_next( ITABLE );


static ITABLE itable;		/* the identification table */
static ITABLE top_of_stack;	/* top of stack of the identification table at any moment */ 


int get_idindex( ITABLE p ){
	/* 
		returns the idindex of the given entry
	*/
	return p->id_index;
}
int get_level( ITABLE p ){
	/* 
		returns the frame number of the given entry
	*/
	return p->level;
}
astnode get_decptr( ITABLE p ){
	/* 
		returns the declaration pointer of the given entry
	*/
	return p->decptr;
}
int get_lexindex( ITABLE p ){
	/* 
		returns the lexical index of the given entry
	*/
	return p->lexical_index;
}

int get_next( ITABLE p ){
	/* 
		returns the field "next" of the given entry... see ident_table.h to see what it is
	*/
	return p->next;
}

ITABLE  previous_element( ITABLE p ){
	/* 
		returns the field "previous_element" of the given entry... see ident_table.h to see what it is
	*/
	return p->prev_elem;
}

int get_num_elem_frame( int frame ){

	/* 
		Returns the number of elements for a given frame. But this frame can only be the last one (I know, perhaps i should rename the function)

		Since we will be only using it for counting it the last frame's number of elements, then it starts counting
		from top_of_stack until a change of frame is reached
	*/
	ITABLE p;
	p = top_of_stack;
	int c = 0;

	while( p->id_index != -1 ){
		if( p->level == frame ){
			c++;
			p = previous_element(p);
		}
		else
			break;
	}

	return c;
}

int find_closest_previous( int lex_i ){
	/* 
		returns the id_index of the closest previous record for the identifier with lexical index lex_i 
		within the stack, or zero if it does not exists.

		NOTE: the algorithm assumes that the search must start from the top of the stack!! therefore it can only be called
		to calculate the field previous (or Next in c11 guidelines) before inserting the new element on stack
	*/
	ITABLE p;
	
	/* searches */
	p = top_of_stack;

	if( p->id_index == -1 )
		return 0;
	
	do{
		if( p->lexical_index == lex_i )
			return p->id_index;
		p = previous_element(p);
	}while( p->id_index != -1 );

	return 0;
}

ITABLE find( int lex_i, int lvl ){
	/* 
	   Searches for an identifier with the same lexical index within the same frame throughout the identification table. 
	   If it finds it returns the pointer to the entry, otherwise NULL.

	   NOTE: comparison is also made for every level 0 entry, as all level 0 identifiers have global scope and cannot be redeclared. 
	*/
	ITABLE p;
	
	/* searches */
	p = top_of_stack;

	while( p->id_index != -1 ){
		if( ( p->level == lvl && p->lexical_index == lex_i ) || ( p->level == 0 && p->lexical_index == lex_i )  )
			return p;
		p = previous_element(p);
	}
	return NULL;
}

astnode retrieve_decnode_itable( int IdI ){
	/* 
		returns a pointer to the declaration node for the entry with Index IdI... used for linkinf applied ocurrences with definitions
	*/
	ITABLE p;
	astnode pnode = NULL;

	p = top_of_stack;
	
	while( p->id_index != IdI && p->id_index != -1 ){
		p = previous_element(p);
	}
	if( p->id_index != -1 ){
		pnode = p->decptr;
	}	

	return 	pnode;
}

ITABLE create( int idindex, int lvl, astnode decp, int nxt, int lexindex, int coit ){
	/* 
		allocates memory for a new entry for the ident table, and initializes it
	*/
	ITABLE p;	

	p = ( ITABLE )malloc( sizeof( identification_table ) );

	p->id_index = idindex;
	p->level = lvl;
	p->decptr = decp;
	p->next = nxt;
	p->lexical_index = lexindex;
	p->created_on_init_trav = coit;
	p->num_of_redefs = 0;
	p->prev_elem = top_of_stack;	/* watch for segfault if itable_init is not called */

	return p;
}

void itable_init(){
	/* 
		initializes ident table by allocating memory for the first element in the table (at top of the stack), 
		and setting initial values.
	*/

	itable = create( -1, -1, NULL, -1, -1, false );	/* first element of the stack */
	top_of_stack = itable;

}

void push( int idindex, int lvl, astnode decp, int nxt, int lexindex, int coit ){
	/* 
		pushes a new element into the identification table 
	*/
	ITABLE p;

	p = create( idindex, lvl, decp, nxt, lexindex, coit );

	top_of_stack = p;
}

void pop(){
	/* 
		pops the last element pushed into the identification table 
	*/
	ITABLE temp;
	temp = top_of_stack->prev_elem;
	
	free( top_of_stack );			/* frees the space pointed by top of stack */
	top_of_stack = temp;	 		/* top of the stack now points to the prev element on the list */
}


void print_stack(){
	/* 
		prints the identification table ( it's a stack... that the reason of the name ) 
	*/
	ITABLE traveler;

	traveler = top_of_stack;
	
	printf( "\n");
	printf( "IdI - L - DecPtr - Next - LexI - string form\n" );
	printf( "--------------------------------------------\n" );
	printf( "             [ TOP OF STACK ]               \n" );


	while( true ){
		if( get_idindex(traveler) != -1 ){ /* prints elements except the first one. the first one is all setted to -1 (equivalent to say null) */
			printf( " %d    %d              %d      %d    %s\n", get_idindex(traveler), get_level(traveler), get_next(traveler) 
								   , get_lexindex(traveler), get_decptr(traveler)->leaf->table_entry->lexeme );
		}
		if(  previous_element(traveler) == NULL )
			break;	
		else
			traveler =  previous_element(traveler);
	}
}
