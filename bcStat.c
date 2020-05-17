
/*
© Copyright 1998, 2005, 2020 Einar Kjartansson

This file is part of BC

    BC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BC.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "silread.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

BcIndex *ind ;
FILE *out ;
int nIn ;
char  iPath[100] ;
int list, allStations , *sortb ;

void help()
{
	fprintf(stderr,"\n\
	bcStat   - statistics for bc waveform files, from index file\n\
	options: \n\
	-f file		index file, default is file for current day. May\n\ 
				include idecies for more than one day.\n\
	-y 		use indes file for previous day, may be used more than once.\n\
	-V		print subversion revision information.\n\
	-h		print brief instructions.\n\
\n\
	Information is only listed for stations running ring2bc\n\
	Default indexfile is at $BC_BASE/yyyy/mmm/dd/index\n\
	Default for BC_BASE is /sil/bc\n\
	See also bctool\n\
\n") ;
	exit(0) ;
}

void swapL( void  *val) /* convert bytes in longint, big vs. little endian  */
{
        char *p,t ;
        p = (char *)val ;
        t = *p   ; *p   = p[3] ; p[3] = t ;
        t = p[1] ; p[1] = p[2] ; p[2] = t ;
}
int iComp( int *p1, int *p2 ) 
{
	return ( *p1 - *p2 ) ;
}
int gwComp( BcIndex *p1, BcIndex *p2 ) 
{
        int d ;
        d = p2->freq - p1->freq ; if(d) return(d) ;
        d = strcmp(p1->station,p2->station) ; if(d) return(d) ;
        return( p1->sTime - p2->sTime ) ;
}


void readIndex( char *file )
{
	int fd,n,i,nr ;
	time_t tt ;
	BcIndex *ip ;
	fd = open(file,O_RDONLY) ;
	n = lseek(fd,0,SEEK_END) ;
	i = lseek(fd,0,SEEK_SET) ;
	ind = (BcIndex*) malloc(n+sizeof(BcIndex)) ;
	sortb = ( int *) malloc(n) ;
	nr = read(fd, ( void * ) ind, n ) ;
	nIn = n/ sizeof(BcIndex) ;
	for( i = 0 ; i < nIn ; i++) {
		ip = ind + i ;
		tt = ip->sTime ;
		swapL(&ip->sTime) ;
		swapL(&ip->nSamp) ;
		tt = ip->sTime ;
/*		printf("%s %d %s",ip->station,tt,ctime( & tt)) ; */
	}
        qsort(ind,nIn,sizeof(BcIndex),(int(*)())gwComp)  ;
	fprintf(out,"Read %d records from index file %s\n",nIn,file) ;
/*
	for( i = 0 ; i < nIn ; i++) {
		ip = ind + i ;
		tt = ip->sTime ;
		swapL(&ip->sTime) ;
		printf("%s %d %s",ip->station,tt,ctime( & tt)) ;
	}
*/
}
void listGaps( char *stationX ) 
{
	int i,d, nGap,lenGap,average, ringFlag ;
	time_t tlast,tt ;
	char *sta ;
	int q1,q5,q9,qq ;
	BcIndex  *ip ;
	sta = "_" ;
	tlast = 0 ;
/*              ksk  34761    71341      2      1      1      1 */
	fprintf(out,"_ -1  nGap    total    mean    Q.1  median  Q.9 \n") ;
	for( i=0 ; i <= nIn ; i++ ){
		ip = ind + i ;	
		if( strcmp(ip->station,sta)) {
			if( i > 0 ) {
				if( nGap > 0 ) {
					average = lenGap/nGap ;
					qsort(sortb,nGap,sizeof(int),(int(*)())iComp) ;
					q5 = sortb[(nGap)/2 ];
					qq = nGap/10 ;
					q1 = sortb[qq] ;
					q9 = sortb[nGap-1-qq] ;
				} else { average = 0 ; q1 = 0 ; q5 = 0 ; q9 = 0  ; }
				if(  ringFlag )
				   fprintf(out,"%s  %5d  %7d %6d %6d %6d %6d\n",sta,nGap,lenGap,average,q1,q5,q9) ;
/*					printf("%s %4d gaps, total %5d seconds, average %4d seconds\n",
						sta,nGap,lenGap,average) ; */
			}
			sta = ip->station ;
			tlast = 0 ; nGap = 0 ; lenGap = 0 ;
			ringFlag = 0 ;
			if( i == nIn ) return ;
		}
		tt = ip->sTime ;
/*		printf("%s %d %s",ip->station,tt,ctime( & tt)) ; */
		if( tt % 120 ) ringFlag = 1 ;
		d = tt - tlast ;
		if( (d>0) &&  tlast ) {
			if( list && ringFlag) 
				fprintf(out,"%s %5d %s",sta,d,ctime( & tlast ));  
			sortb[nGap++] = d ;
			lenGap += d ;
		} 
		tlast = tt + ip->nSamp/100 ;
	}
}
void init(time_t tt)
{
	struct tm *tm ;
	char *base,*cp ;
	base = getenv("BC_BASE") ;
	if(NULL == base) base = "/sil/bc" ;
	cp =  strcpy(iPath,base) ;
	cp = iPath+ strlen(base) ;
	tm = gmtime(&tt) ;
	strftime(cp,80,"/%Y/%b/%d/index",tm) ;
	cp[6] ^= 32 ;
	
}
int main( int ac , char **av)
{
	time_t tt ;
	int cc ;
	extern char *optarg ;
	char *file ;
	unsetenv("LANG") ;
	time(&tt) ;
	file = NULL ;
	while( EOF != (cc = getopt(ac,av,"f:lyaVhH?"))){
	    switch(cc) {
		case 'l' : list = 1 ; break ;
		case 'f' : file = optarg ; break ;
		case 'y' : tt -= 24*3600 ; break ;
		case 'a' : allStations = 1 ; break ;
		case 'V' : fprintf(stderr,"%s\n",SVN_REV) ; break ;
		default : help() ; break  ;
	}}
	if( list ) out = stdout ;
	else {
		out = popen("sort -n -k2","w") ;
	}
	if( NULL == file ) {
		init(tt) ;
		file = iPath ;
	}
	readIndex(file) ;
	listGaps("-" ) ;
	if( list == 0 ) pclose(out) ;
}
