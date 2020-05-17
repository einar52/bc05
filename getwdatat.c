#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <evlib.h>
#include "getwdata.h"
#include "silread.h"
void plotz( SilChannel *s)
{
	float *x,*y ;
	static float para[30] ;
	
}
int main( int ac , char **av)
{
	time_t tt,tr ;
	int d,t,len ;
	SilChannel *sd ;
	int data[10000],n ;
	char *s ;
	s = "sol" ;
	if(ac > 1 ) s = av[1] ;
	d = 19980922 ; t = 152310 ; len = 100 ;
	if( ac > 2 ) d = atoi(av[2]) ;
	if( ac > 3 ) t = atoi(av[3]) ;
	if( ac > 4 ) len = atoi(av[4]) ;
	tt = evTime(d,t) ;
	sd = gwGetWData(s,tt,len,100)  ;
	if(sd) sRPrintSilC(sd,stdout) ;
	else printf("no data\n") ;
	return(0) ;
}
