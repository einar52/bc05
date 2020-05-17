
/*
© Copyright 1998, 2020 Einar Kjartansson

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "silread.h"
#include "getwdata.h"

#define MIND 70000
/* BcIndex *gwIndex ; */
BcIndex gwIndex[MIND] ;
SilChannel *gwSp ;
int gwOldDay, gwNIndex  ;
char gwDirName[150] ;
int gwLogLevel = -2 ;
int gwFileInMem  = -1 ;
int logLevel ;
void bclog( int level , char *s1 , void *s2 )
{
	char *lev ;
	if(gwLogLevel == -2) {
		lev = getenv("BC_DEBUG") ;	
		if(lev ) gwLogLevel = atoi(lev) ;
		else gwLogLevel = 3 ;
/*		printf("LogLevel = %d  %s \n",gwLogLevel,lev) ;  */
	}
        if( gwLogLevel < level  ) return ;
        fprintf(stdout,"getwdata %d: ",level ) ;
        fprintf(stdout,s1,s2) ;
        fprintf(stdout,"\n")  ;
        fflush(stdout) ;
        if( level <= -1 ) abort() ;
        if( level <=  0 ) exit(0) ;
}

int gwComp( BcIndex *p1, BcIndex *p2 ) 
{
#define D 100000 
	int d ;
	d = p2->freq - p1->freq ; if(d) return(d*D) ;
	d = strcmp(p1->station,p2->station) ; if(d) return(d*D) ;
	return( p1->sTime - p2->sTime ) ;
}
int gwBinSearch( BcIndex *p ) 
{
	int i,j,k,r ;
	BcIndex *pp ;
	i = 0 ; j = gwNIndex - 1 ;
	while(i < j ) {
		k = (i+j)/2 ;
		r = gwComp(p, gwIndex+k) ;
		if(r == 0 ) return(k) ;
/*
		printf("%4d %4d %4d %8d   ",i,k,j,r ) ;
		printf("%s %s",gwIndex[k].station,ctime(&(gwIndex[k].sTime)) ) ;
*/
		if( r > 0 ) i = k+1 ; else j = k-1 ;
	}
	k = (i+j)/2 ;
	r = gwComp(p, gwIndex+k) ;
	if(( r < 0 )&& ( k>0 )) {
		k-- ;
		r = gwComp(p, gwIndex+k) ;
	}	
/*
	printf("%4d %4d %4d %8d   ",i,k,j,r ) ;
	printf("%s %s",gwIndex[k].station,ctime(&(gwIndex[k].sTime)) ) ;
	if( r < 0 ) bclog(1,"This should not happen in binSearch"," ") ;
	if( r*100 >= gwIndex[k].nSamp ) k = -1 ;
*/
	if( r < 0 ) return(-1) ;
	pp = gwIndex + k ;
	if( p->freq - pp->freq ) return(-1) ;
	if( strcmp(pp->station,p->station)) return(-1)   ;
	if( p->sTime >= ( pp->sTime + pp->nSamp/pp->freq )) return(-1) ;
	return(k) ;
}
void gwPrintInd()
{
	int i,n ;
	BcIndex *p ;
	n = gwNIndex ;
	for( i = 0 ; i < n ; i++ ) {
		p = gwIndex + i ;
		printf("%s %s",p->station,ctime(&(p->sTime)) ) ;
		if( i == 8 ) i = n - 8 ;
	}
}
void gwNewDay( time_t time)
{
	char *bcbase = getenv("BC_BASE") ;
	char buff[200] ;
	int fd,size,end,j ;
/* 	if(gwIndex != 0 ) free(gwIndex) ; */
	sRBcPath( evDay( time) , gwDirName ) ;
	sprintf(buff,"%s/index",gwDirName) ;
	bclog(5,buff," ") ;
	fd = open(buff,O_RDONLY) ;
	if( fd < 0 ) bclog(0,"cannot open %s",buff) ;
	size = lseek(fd,0,SEEK_END) ;
	j = lseek(fd,0,SEEK_SET) ;
	gwNIndex = size/sizeof(*gwIndex) ;
	if(gwNIndex > MIND ) bclog(0,"index to large, n=%d",(void*)gwNIndex) ;
	bclog(5,"index contains %d entries", ( void *) gwNIndex ) ;
/*	gwIndex = ( BcIndex *)malloc(size) ; */
	bclog(5,"gwIndex=%X",(void*)gwIndex) ; 
	j = read(fd,gwIndex,size) ;
	close(fd) ;
	if( j != size ) bclog(0,"problem reading %s",buff) ;
	for(j=0 ; j < gwNIndex ; j++ ) bcSwapIf( &(gwIndex[j].sTime),3) ;
	qsort(gwIndex,gwNIndex,sizeof(BcIndex),(int(*)())gwComp)  ;
/*	gwPrintInd() ; */
	gwFileInMem = -1 ;
}
void gwGaps( time_t tt ) 
{
	int i, flag ;
	static char station[4] ;
	int start, end, n,nGap, gapLen, e ;
	BcIndex *ip ;
	logLevel = 9 ;
	gwNewDay( tt )  ;
	ip = gwIndex ;
	i = 0 ;
	printf("gwGAps %d %s \n",gwNIndex,ctime(&tt)) ;
	while( i < gwNIndex ) {
		flag = 0 ;
		ip = gwIndex+i ;
		if( strncmp( station, ip->station,3 ) )  {
			strncpy( station, ip->station ,3) ;
			flag = 1 ;
			printf("%3s ", station ) ;
			end = 1500000000 ;
			nGap = 0 ; gapLen = 0 ; n=0 ;
		}
		start = ip->sTime ;
		if ( start > end ) { nGap++ ;
			printf(" %d ",start-end) ;
			gapLen += start - end ; }
		e = start + ip->nSamp / 100  ;
		/*if( e > end )*/ end = e ;
		ip++ ; i++ ; n++ ;
		if( strncmp( station, ip->station ,3)) {
			printf("|%6d  %6d %6d\n",n,nGap,gapLen) ; 
		}
	}
}
void gwGetFile( int k )
{
	char tb[10],fname[150] ;
/*	if( gwFileInMem == k ) return ;	  */
	cftime(tb,"%H%M%S",&(gwIndex[k].sTime)) ;
	sprintf(fname,"%s/%s/%s%02d.bc",gwDirName,gwIndex[k].station,tb,gwIndex[k].freq%100) ;
	bclog(5,fname," ") ;
/*	if(gwSp) free(gwSp) ; */
	gwSp = sRGetBc(fname) ;
	gwFileInMem = k ;
}
int gwShorten(int freq, int ti, int ni, int td, int nd, int *to )
{
	int ei,ed,eo,d,padd ;
	if( nd <= MAXSAMPLES ) { *to = td ; return (nd) ; }
	if( td > ti ) { *to = td  ; return( MAXSAMPLES) ; }
	ei = ti + ni/freq ;
	ed = td + nd/freq ;
	d = (nd - MAXSAMPLES)/freq ;
	padd = ( MAXSAMPLES-ni)/freq ;
/*	printf("ei = %d ed = %d d=%d padd=%d\n",ei,ed,d,padd) ; */
	if( ed < ei ) { *to = ed - MAXSAMPLES/freq ; return(MAXSAMPLES) ; }
	*to = ti - (ti-td)*padd/(ti-td+ed-ei) ; ;
/*	printf("%d %d   %d %d   %d %d\n",ti,ni,td,nd,*to,MAXSAMPLES) ; */
	return(MAXSAMPLES) ;
}
SilChannel *gwGetWDataS( char *station, time_t sTime, int nSec, int sFreq ) 
{
	int d,k ;
	static time_t t,indexTime ;
	BcIndex test ;
	d = evDay( sTime ) ;
	t = time(NULL) ;
	if(( d != gwOldDay ) || ( (t - indexTime) > 300 )) {
		gwNewDay(sTime) ;
		indexTime = t ; /* index is updated if more than 5 min old */
	}
	strcpy(test.station,station) ;
	test.sTime = sTime ;
	bclog(7,"unix time is %d",(void*) sTime) ;
	test.freq = sFreq ;
	test.nSamp = nSec*sFreq  ;
	k = gwBinSearch(&test) ;
	bclog(6,"k = %d",(void *)k) ;
	if( k < 0 ) return(0) ;
	gwGetFile(k) ;
	return(gwSp) ;
}
SilChannel *gwGetWData( char *station, time_t sTime, int nSec, int sFreq )
/*	get requests for waveformdata and combine sgments if possible 
	Returns a pointer that should be free'd  by calling program */
{
	SilChannel *s1,*s2,*s3 ;
	char b1[30],b2[30],b3[90] ;
	int nin,nSeg ;
	strftime(b1,20,"%Y%m%d %H%M%S", gmtime(&sTime)) ;
	nin = 0 ;
	nSeg = 1 ;
	s1 = gwGetWDataS( station,sTime,nSec,sFreq) ;
	if( s1 ) while ( 1 ) {
		nin = s1->sTime -sTime + s1->nData/sFreq ;
		if( nin >= nSec ) break ;
		s2 = gwGetWDataS( station,sTime+nin, nSec - nin, sFreq )  ;
		if( s2 == NULL ) break ;
		s3 = sRSilCat(s1,s2 ) ;
		if( s3 == NULL ) break ;
		nSeg++ ;
		free(s1) ; free(s2) ;
		s1 = s3 ;
	} else {
		bclog(4,"gwGetWData did not find data for %s",b1) ;
		return NULL ;
	}
	strftime(b2,20,"%H%M%S", gmtime(&(s1->sTime))) ;
	sprintf(b3,"gwGetWData returns %d seconds starting at %s for request %s, station %s, from %d segments."
		,(s1->nData)/sFreq,b2,b1,station,nSeg  ) ;
	bclog(4,b3,NULL) ;
	bclog(6,"gwGetWData returns spointer %x",s1) ;
	return s1 ;
}
time_t unixtime_(int *year, int *month, int *day, 
		int *hour, int *min, int *sec )
/*	Fortran interface for mktime */
{
	struct tm t ;
	t.tm_sec = *sec ;
	t.tm_min = *min ;
	t.tm_hour = *hour ;
	t.tm_mday = *day ;
	t.tm_mon = *month-1 ;
	t.tm_year = *year - 1900 ;
	return( mktime(&t)) ;
}
void gwShiftRD3( SilChannel *sd , float *data, int n)
/*
	Apply 68.75 ms time shift to data in SilChannel if second word in 
	ascii header is "RD3" 		- Einar Kjartansson, aug 1999.
	If environment variable BC_DIGITIZER is set, that is used instead
	of RD3
	Shift is only applied to data after sep 1 1999. -eik 18/1 2000
*/
{
	char *cp ;
	static char *digitizer ;

	static time_t sep1_1999 ;
	if( 0 ==  sep1_1999 ) sep1_1999 = evTime(19990901,0) ;
	if( sd->sTime < sep1_1999 ) return ; 

	if( NULL == digitizer ) {
		digitizer = getenv("BC_DIGITIZER") ;
		if( NULL == digitizer) digitizer="RD3" ;
	}
	cp = sd->ascHead ;
/*
	while( 0 == isspace(*cp++) ) ;
	while( isspace(*cp++) ) ;
*/
	while( *cp++ > 32 ) ;
	while( *cp++ < 33 ) ;
	cp-- ;
/*	fprintf(stderr,"digitizer=_%s_ BC_DIGITIZER=%s\n",cp,digitizer) ;*/
	if( 0 == strcmp(cp,digitizer)) shShift55_8( data, n) ;
/* NRD3 gerdur ovirkur 14/4/00 ssj */ 
	else if( 0 == strcmp(cp,"NRD3")) shShift55_8( data, n) ;
}
int getwdata_( char *station, int *sTime, int *channel, int *nSec, int *sFreq, 
	float *data, int *retTime, 
	char *sensor, char *digitizer, char *timeSync, 
	int lsta, int lsens, int ldig, int lsync )
/*		Fortran interface, see silreadh.h	*/
#define NMax 12000 ;
{
	SilChannel *sd ;
	char *cp,sta[4] ;
	int i,n,l,*ip,stt ;
	char buff[90] ;
	strncpy(sta,station,3) ;
	sta[3]=0 ;
	if(( *channel>2)||(*channel<0)) { 
		bclog(1,"cannel should be 0,1,2 but is %d",(void*)*channel) ;
		return(0) ;
	}
	sprintf(buff,"%s channel=%d nSec=%d sFreq=%d %s",sta,*channel,
		*nSec,*sFreq,ctime((time_t *)sTime)) ;
	bclog(6,buff," ")  ;
	sd = gwGetWData(sta, *sTime, *nSec , *sFreq) ; 
	if(NULL == sd ) 
		sd = gwGetWData(sta, *sTime+(*nSec/2), *nSec , *sFreq) ;
	if(NULL == sd ) 
		sd = gwGetWData(sta, *sTime+*nSec/2, *nSec , *sFreq) ;
	if( NULL == sd) return(0) ;
	cp = sd->ascHead ;
	sscanf(cp,"%s %s",sensor,digitizer) ;
	if(sd->ascHeadN > 2 ) {
		l =  strlen(cp) ; cp += l+1 ;
		l =  strlen(cp) ; cp += l+1 ;
		strncpy(timeSync,cp,lsync) ;
	}
	n = gwShorten(sd->freq,*sTime,*nSec*sd->freq,sd->sTime,sd->nData,
		retTime) ;
	ip = sd->data + *channel*sd->nData + (*retTime - sd->sTime)*sd->freq  ;
	if( *retTime < sd->sTime ) bclog(0,"data error in getwdata_ "," ") ;
	for(i = 0 ; i < n ; i++) data[i] = *ip++ ;
	gwShiftRD3(sd,data,n) ;
	free(sd) ;
	return(n) ;
}
