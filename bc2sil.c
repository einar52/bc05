#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "silread.h"
int logLevel = 4 ;
char *bcFile, *silFiles ;


void bclog( int level , char *s1 , void *s2 )
{
        if( logLevel < level  ) return ;
        fprintf(stdout,"bc2sil %d: ",level ) ;
        fprintf(stdout,s1,s2) ;
        fprintf(stdout,"\n")  ;
        fflush(stdout) ;
        if( level < -1 ) abort() ;
        if( level <  0 ) exit(0) ;
}

int doit()
{
	FILE *fd ;
	SilChannel *sC ;
	sC = sRGetBc(bcFile) ;
	bclog(5,"einar","x") ;

/*	sRPrintSilC( sC,stdout ) ;   */
	sRPutSil(sC) ;
	return(0) ;
}
int main( int ac , char **av )
{
	extern char *optarg ;
	int cc ;
	bcFile = "000311.bc" ;
	doit() ;
	return(0) ;
}	
