/*
	Program to test the speed of gwGetWData.
	Results are (real time) about 2-300 waveforms per second 
	from local disk (reitur) or around 100 over NFS
	Oct 1998, Einar Kjartansson
	Average size of bc files is about 7kbytes 
*/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "evlib.h"
#include "getwdata.h"
#include "silread.h"

 char *sta[33] = {"vos" ,"bja" ,"vog" ,"san" ,"haf" ,"kro" ,"asm" 
		,"hei" ,"kri" ,"sol" ,"kud"
	,"sau" ,"nyl" ,"mid" ,"asb" ,"ren" ,"hau" ,"gra" ,"hve" ,"sig"
	,"gyg" ,"skh" ,"snb" ,"gri" ,"hrn" ,"kal" ,"hla" ,"skr" ,"gil"
	,"sva" ,"grs" ,"lei" ,"ada" } ;
int n1,n2 ;
int main( int ac , char **av) 
{
#define M 100 
#define S 15
	char **lp[M],**pp   ;
	int i,j,nf,d,t,s,ns  ;
	SilChannel *sc ;
	time_t tt ;
	for ( j = 0 ; j < M ; ) {
		nf = 0 ;
		pp = evGetLine( &nf,stdin) ;
		if( nf < 4 ) break ;
		lp[j++] = pp ;
	}
	for( i = 0 ; i < j ; i++ ) {
		d = atoi(lp[i][2]) ;
		t = atoi(lp[i][3])  ;
		tt = evTime(d,t) +10 ;
		for( s = 0 ; s < S ; s++ ) {
		 	sc = gwGetWData( sta[s],tt,100,100 ) ;
			if( sc ) n2++ ; else n1++ ;
		}
	} 
	printf("Got %d waveforms, failed for %d\n",n2,n1) ;
	return(0) ;
}
