
#include <stdio.h>
#define MAX 1000
int sRShorten(int freq, int ti, int ni, int td, int nd, int *to )
{
	int ei,ed,eo,d ;
	if( nd <= MAX ) { *to = td ; return (nd) ; }
	if( td > ti ) { *to = td  ; return( MAX) ; }
	ei = ti + ni/freq ;
	ed = td + nd/freq ;
	d = (nd - MAX)/freq ;
	printf("ei = %d ed = %d d=%d\n",ei,ed,d) ;
	if( ed < ei ) { *to = ed - MAX/freq ; return(MAX) ; }
	*to = ti + ((td-ti)*MAX)/nd ;
	return(MAX) ;
}
main( int ac  , char **av ) 
{
	int ti,ni,td,nd,to,no ;
	ti=atoi(av[1]) ;
	ni=atoi(av[2]) ;
	td=atoi(av[3]) ;
	nd=atoi(av[4]) ;
	no = sRShorten(100,ti,ni,td,nd,&to) ;
	printf("%4d %4d   %4d %4d   %4d %4d\n",ti,ni,td,nd,to,no) ;
}
