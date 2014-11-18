#ifndef _AST_H_
#define _AST_H_

typedef struct leafrecord{
        int token_name;
	int type;
	bool is_array;				/* true for identifiers that are arrays */
	ST_TABLE table_entry;

	/* annotated attributes */
	struct treerecord *decnode;		/* for identifiers, it contains a pointer to its declaration node. It's filled during semantic analisys */
	struct treerecord *paramnode;		/* for function declarations. it constains a pointer to its parameters */	
	struct treerecord *arrayinode;
}leafrecord,*leafpointer;

typedef struct treerecord{
        int op;					/* label for the node */
	int type;				/* type */
	int lineno;
        struct treerecord *leftptr;
        struct treerecord *centerptr;
	struct treerecord *rightptr;
	struct leafrecord *leaf;		/* if leaf field != NULL the current node is considered a LEAF, otherwise a NODE.  */
						/* This means that the same data structure is used for leaves and nodes, but  only leaves will have */
						/* this field assigned. */ 
}noderecord,*nodepointer;


typedef nodepointer 	  astnode;	/* for more clarity */
typedef leafpointer	  astleaf;

astnode maketree( int , astnode, astnode, astnode, astleaf, long int );
astleaf makeleaf( int, ST_TABLE );
astnode left(astnode);
astnode center(astnode);
astnode right(astnode);
OP op(astnode);
void setop(astnode,int);
void settoken_name( astleaf, int );
void setright( astnode, astnode );
void setcenter( astnode, astnode );
void setleft( astnode ,astnode );
void setleaf( astnode, astleaf );
void settableentry( astleaf, ST_TABLE );
void settype_node( astnode, int );
void settype_leaf( astleaf, int );
void printtree();


#endif

