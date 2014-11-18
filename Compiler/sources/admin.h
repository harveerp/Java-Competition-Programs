#ifndef _ADMIN_H_
#define _ADMIN_H_

typedef char CHARACTER;

CHARACTER  getcharacter();
bool add_spellingtable( char*, int );
char *get_spellingtable( int );
int size_spellingtable();

#endif
