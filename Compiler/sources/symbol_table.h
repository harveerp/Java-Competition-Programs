#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

typedef struct st_bucket{		/* table entry */
	struct st_bucket *next;		/* next entry in chain */
	char *lexeme;			/* the key */
	int index;			/* index for different identifiers */
}ST_BUCKET;

typedef ST_BUCKET *ST_TABLE;

ST_BUCKET *symboltable_lookup( char* );
ST_BUCKET *symboltable_install( char* );

#endif

