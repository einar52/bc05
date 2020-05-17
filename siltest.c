#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "silread.h"
int logLevel = 3 ;
int main()
{
	FILE *fd ;
	SilChannel *sC ;
/*
	fd = fopen("07074800Z00","r") ;
	sC = sRReadSil(fd) ;
	sRPrintSilC( sC,stdout ) ;
	free(sC) ;
*/
	sC = sRGetSil3Channel("00031100E00.Z") ;
	sRPutBCFile(sC,"junk.bc") ;
	sRPrintSilC( sC,stdout ) ;
	return(0) ;
}
