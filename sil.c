#include <stdio.h>
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
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "bitcomprl.h"
#include "util.h"
#include "sil.h"
#define CLIP 4000000

SilData sZ,sN,sE ;

int getmonth( char *s )
{
	static char *mn[] = { "jan", "feb", "mar", "apr", "may",
		"jun","jul","aug","sep","oct","nov","dec" } ;
	int i ;
	for( i = 0 ; i < 12 ; i++ ) 
		if( 0 == strcmp(mn[i],s)) return(i) ;
	return(-1) ;
}
void gethead( FILE *fd, SilData *s )
{
	struct tm t ;
	int j1,j2,j3,j4 ;
	char month[6] ;
	char line[200] ;
	fscanf(fd,"%s %d",s->station,&t.tm_year) ;
	fscanf(fd," /usr/sil/net/%[a-z]/%d/%d:%d:%d.%d%s",
		month,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec,
		&s->frac,&s->tail) ;
	t.tm_mon = getmonth(month) ;
	t.tm_year -= 1900 ;
	s->sTime = mktime( &t ) ;
	fscanf(fd," %d",&s->freq) ;
	j1=fscanf(fd," %s",s->statype) ;
	j2=fscanf(fd," %s",s->id) ;
	*line = 0 ;
	while ( *line - '+' ) {
		if(NULL==fgets(line,200,fd)) 
			Error(" sil2bc, incomplete header", " ") ;
	}
	j3=fscanf(fd,"%d",&s->n) ;
	if( 0 == s->freq) s->freq=100 ;
}
int getdata( FILE *fd , SilData *s)
{
	int i,j,*dp ;
	if( s->data ) free(s->data);
	s->data = (int *) malloc(4*s->n) ;
	dp = s->data ;
	for( i = 0 ; i < s->n ; i++ ) {
		j=fscanf(fd," %d",dp ) ;		
		if( *dp > CLIP ) *dp = CLIP ;
		if( *dp < -CLIP) *dp = -CLIP ;
		dp++ ;
	}
	return(j) ;
}
void pprhead(FILE *ff, SilData *s, char *name)
{
	if( level < 3 ) return ;
	fprintf(ff,"nsamp=%d sfrac=%d sampFreq=%d, %s  ",
			s->n,s->frac,s->freq,name) ;
	fprintf(ff,"\n") ;

	fflush(ff) ;

}
int getSilChannel( int channel, SilData *s, char *inFile ) 
{
	FILE *iFile,*popen() ;	
	int j ;
	char str[200],name[200] ;
	strcpy(name,inFile) ;
	name[cIndex] = channel ;  
	iFile = fopen(name,"r") ;
	if(NULL == iFile) Error("Cannot open file %s",name) ;
	if( compressFlag) {
		fclose(iFile) ;
		sprintf(str," uncompress < %s ",name ) ;
		iFile = popen(str,"r") ;
	}
	gethead(iFile,s );
	j=getdata(iFile,s ) ;
	if( j == 0 ) return(0) ;
	pprhead(stderr,s,name) ;
	if( compressFlag) pclose(iFile); else fclose(iFile) ;
	return(1) ;
}
int getSil( char *fname, FILE *out ) 
{
	int j ;
	CData  d ;
	SilBcHead h ;
	j = 1 ;
	j *= getSilChannel('Z',&sZ,fname) ;
	j *= getSilChannel('N',&sN,fname) ;
	j *= getSilChannel('E',&sE,fname) ;
	if( j == 0 ) { fprintf(stderr,"Missing data, reading %s\n",fname) ;
		return(1) ; }
	h.sTime = sZ.sTime ;
	strncpy(h.station,sZ.station,4) ; 
	h.freq = sZ.freq ;
	h.nData = sZ.n ;
	if( bcLittleEndian() ) {
		bcSwapL( &h.sTime) ;
		swab( ( char *) &h.freq, ( char *) &h.freq, 4 ) ;
	}
	if(1 != fwrite(&h,sizeof(h),1,out) ) Error("Cannot write", fname) ;
	if( sZ.n > 65000 ) {
		fprintf(stderr,"Too many samples %d  > 65000 " ,sZ.n) ;
		return(0) ; }
	if ( sZ.n < 99 ) { fprintf(stderr,"Too few samples: %d ",sZ.n) ;
		return(0) ; }
	bcCompBit( sZ.data,sZ.n, &d,1 );
	bcWrite(out,&d) ;
	bcCompBit( sN.data,sN.n, &d,1 );
	bcWrite(out,&d) ;
	bcCompBit( sE.data,sE.n, &d,1 );
	bcWrite(out,&d) ;
	return(1) ;
}
