#ifndef _IDENT_TABLE_H_
#define _IDENT_TABLE_H_

typedef struct symbol_table_records{ /* each element correspond to a declaration of an identifier */

	int id_index;					/* identification index. the access table maps this value to the spelling table*/
	int level;					/* it define frames */
	astnode decptr;					/* pointer to declaration node */
	int next;					/* IdI of the closest next record for 'x' within the stack | x = the identifier for current entry */ 
	int lexical_index;				/* the index for 'x' in the spelling table (in the parser it is called simply index) */
	bool created_on_init_trav;			/* says if the current entry was created on the initial traversal or not */
	int num_of_redefs;				/* tells how man redefinitons have been found in the identification table for the current entry */
	struct symbol_table_records*	prev_elem;	/* pointer to next element in table */

}identification_table, *identification_table_ptr;

typedef identification_table_ptr ITABLE;

void itable_init();
void push( int, int, astnode, int, int, int );
void pop();
void print_stack();
int get_num_elem_frame( int );
ITABLE find( int, int );
int find_closest_previous( int );
astnode retrieve_decnode_itable( int );

#endif
