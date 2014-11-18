/*
 * access_table.c
 * 
 * 
 *
 * ACCESS TABLE MODULE:
 *	This module contains the access table, and provides an interface for adding and retrieving elements.
 *
 *
 *
 * DEPENDENCIES
 * 	c12.h, access_table.h
 *
 *
 */

#include <stdio.h>
#include "c12.h"
#include "access_table.h"

static int access_table[MAX_NUM_OF_TOKENS]; /* The access table!... later make it dynamic */

void atable_init(){
	/* initializes table: it's filled with -1's*/
	for( int i=0; i < MAX_NUM_OF_TOKENS; i++ )
		access_table[i] = -1; 
}

void insert_atable( int LexI, int IdI ){
	/* note: insertions must start at LexI = 0. 
		 Otherwise when printing it, it might appear empty, 
		 as the table is prefilled with -1's, and when printing -1 indicates end of table */

	/* note: the guidelines specify that the when popping an element from the stack, it's corresponding access table entry must be updated
                 so, it would be null until another variable is assigned to it.
		 However, I only update it when pushing, so whenever a new push is done. AND IT WORKS!! That means that in the mean time between a pop a a 			 new push of a variable with the same lexical index the entry in the access table would be incorrect since it would contain information of 			 an inexistence variable */ 
	access_table[ LexI ] = IdI;
}

int retrieve_atable( int LexI ){
	/* returns the IdI for a given lexical index */
	
	return access_table[ LexI ];
}

void print_atable(){
	/* prints acces table in std out --for debug purposes*/
	printf( "\n" );	
	printf( "ACCESS TABLE\n" );
	printf( "LexI    IdI\n" );
	printf( "-----------\n" );

	for( int i=0; i<MAX_NUM_OF_TOKENS; i++ ){			/*change this later when changed to a dynamic table */
		if( access_table[i] != -1 )
			printf( "  %d    %d \n", i, access_table[i] );
	}

}
