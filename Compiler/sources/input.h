#ifndef _INPUT_H_
#define _INPUT_H_

typedef	char  *BUFFER;

bool 	   	buffer_init( const char* );
bool 	   	buffer_free();
bool 	   	buffer_update();
const BUFFER    buffer_getptr();

#endif
