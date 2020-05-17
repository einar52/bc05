
/*

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bitcomprl.h"
int nDat , *array ;
err( char *s1, char *s2 )
{
	fprintf(stderr,"Error in bcutil: ") ;
	fprintf(stderr,s1,s2) ;
	fprintf(stderr,"\n") ;
	exit(-1) ;
}
void mkExample() 
{
	int i ;
	double dom ;
	dom = 18.0/nDat ;
	if( array ) free(array) ;
	array = (int *) calloc(nDat,4) ;
	for( i = 0 ; i < nDat ; i++ ) {
		array[i] = 100.0*sin(dom*i) ;
	}
}
void printData( char *name )
{
	int i ;
	FILE *ff ;
	ff = fopen(name,"w") ;
	for( i = 0 ; i < nDat ; i++) {
		fprintf(ff,"%3d %6d\n",i,array[i]) ;
	}
	fclose(ff) ;
}
void compOut( char *name) 
{
	CData *cp ;
	FILE *ff ;
	if( nDat < 3 ) return ;
	ff = fopen(name,"w") ;
	cp = (CData *) calloc(nDat+100,4) ;
	bcCompBit(array,nDat,cp,1) ;
	bcWrite(ff,cp) ;
	free(cp) ;
	fclose(ff) ;
}
void compIn( char *name) 
{
	FILE *ff ;
	CData *cp ;
	ff = fopen(name,"r") ;
	if( NULL == ff ) err("Error opening %s",name) ;
	cp = bcRead(ff) ;
	if( NULL == cp) err( "file %s not correct format ",name) ;
	if( array ) free(array) ;
	array = calloc(cp->nData,4) ;
	nDat = bcDeCompr(cp,array ) ; 
}
int main( int ac , char **av) {
	extern char *optarg ;
	int cc ;
	while(EOF != (cc = getopt(ac,av,"m:o:i:p:"))) {
	switch(cc) {
	case 'm' : nDat = atoi(optarg) ; mkExample() ; break ;
	case 'p' : printData(optarg) ; break ;
	case 'o' : compOut(optarg) ; break ;
	case 'i' : compIn(optarg) ; break ;
	} }
	return(0) ;
}
