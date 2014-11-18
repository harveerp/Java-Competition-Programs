#ifndef _WORD_TABLE_H_
#define _WORD_TABLE_H_

typedef struct bucket{		/*table entry */
	struct bucket *next;	/* next entry in chain */
	char *lexeme;		/* the key */
	int  token_name;	/* the correct token name */
}BUCKET;

typedef BUCKET *TABLE;

BUCKET *wordtable_lookup( char* );
BUCKET *wordtable_install( char *, int);
void	wordtable_init();

#endif
