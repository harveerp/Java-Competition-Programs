#ifndef _ERROR_REPORTING_H_
#define _ERROR_REPORTING_H_


int error_list_add( char*, long int );
char *get_error_msg( int );
long int get_error_lineno( int );
int num_errors_found();

#endif
