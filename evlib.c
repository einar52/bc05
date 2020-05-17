#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include  "evlib.h"
char **evGetLine( int *nField, FILE *input ) 
{
	char  buffer[2008] ,*ip,**obuf,*op  ;
	int n,l,i,flag ;
	ip = fgets( buffer,1000,input) ;
	if( NULL == ip) { *nField = 0  ;  return( NULL)  ; }
	l = strlen(ip) ;
	if( ip[l-1] == '\n'  ) { l-- ; ip[l] = 0  ; }
	n = 0 ;
	flag = 1 ;
	for( i = 0 ; i < l ; i++ )  {
		if( isspace( *ip++))  flag = 1 ;
		else {
				if( flag == 1  ) n++ ;
			flag = 0 ;
		}
	}
	obuf = (char **) malloc( sizeof( ip )*(n+1)+l+l+2)  ;
	if( NULL == obuf)  { *nField = 0 ;   return(NULL) ; }
	*nField = n ;
	op  =  sizeof(ip)*(n+1) +  (char *) obuf ;
	(void) strcpy( op,buffer) ;
	obuf[0] = op ;
	op += l ; 
	i = 1 ;
	ip = buffer ;
	flag  = 1 ;
	while(*ip) {
		if( isspace(*ip)) {flag = 1 ;  ip++ ; }
		else {
			if( flag ) {
				*op++ = 0 ;
				obuf[i++] = op ;		
			}
			*op++ = *ip++  ;
			flag = 0 ;
		}
	}
	*op = 0 ;
	return(obuf) ;
}
