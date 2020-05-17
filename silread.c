
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
#include <sys/types.h>
#include <time.h>
#include "evlib.h"
#include "silread.h"
#include "bitcomprl.h"
#define MAXHEAD 200 
extern int logLevel ;
/*   Maximum number of header lines in sil format file, currently 8 */ 
int sRGetMonth( char *s )
{
        static char *mn[] = { "jan", "feb", "mar", "apr", "may",
                "jun","jul","aug","sep","oct","nov","dec" } ;
        int i ;
        for( i = 0 ; i < 12 ; i++ )
                if( 0 == strcmp(mn[i],s)) return(i) ;
        return(-1) ;
}
time_t sRPath2utime( char *st, char *year)
{
/*
   3         2         1
3210987654321098765432109876543210
/usr/sil/net/aug/03/07:07:48.00Z00
*/
	int i,s,m,h,d,mo,y ;
	struct tm t ;
	char *sp, *p ;
	static char monn[4] ;
	p = st ; sp = st ;
	while( *p ) if( *p++ == '/' ) sp = p ;
	t.tm_mday = atoi(sp-3) ;
	t.tm_hour = atoi(sp) ;
	t.tm_min =atoi(sp+3) ;
	t.tm_sec = atoi(sp+6) ;
	strncpy(monn,sp-7,3) ;
	t.tm_mon = sRGetMonth(monn) ;
	if ( t.tm_mon < 0 ) return(-1) ;
	t.tm_year = atoi(year) - 1900 ;
/*
	printf("%s\n",st) ;
	printf("%d %d %d %d %d %d %s\n",t.tm_year,t.tm_mon,t.tm_mday,
		t.tm_hour,t.tm_min,t.tm_sec,monn)  ;
*/
	return( mktime( &t )) ;
}
SilChannel *sRReadSil( FILE *fd )
{
	int nf,i,j,nh,ns,sfreq,hsize ;
	int *ip ;
	char *cp ;
	time_t tt ;
	char **lines[MAXHEAD] ;
	SilChannel *sC ;
	char buff[200] ;
	j = 5 ;
	for( i = 0 ; i < MAXHEAD ; i++) {
		lines[i] = evGetLine( &nf,fd) ;
/*		if(nf < 1 ) return(NULL) ;  */
		if(lines[i] == NULL ) return(NULL) ;
/*	printf("%d %d %2d %2d _%s_\n",i,nf,j,strlen(lines[i][0]),lines[i][0]) ; */
		if( j == 0 ) break ;
		j=strcmp(lines[i][0],"+data+") ;
	}
	nh = i + 1 ; 
	ns = atoi(lines[nh-1][1]) ;
	sfreq = atoi(lines[2][1]) ;
	if( sfreq == 0 ) sfreq = 100 ;
	tt = sRPath2utime(lines[1][1], lines[0][2]) ;
	hsize = 0 ;
	for( i = 3 ; i < nh - 2 ; i++ ) {
		hsize += 1 + strlen(lines[i][0]) ;
	}
	hsize = 4*((hsize-1)/4) + 4 ;
	sC = ( SilChannel *) calloc(sizeof( *sC )+4*ns+hsize ,1) ;
	sC->sTime = tt ;
	sC->cTime = time(0) ;
	strncpy(sC->station,lines[0][1],3) ;
	sC->nData = ns ;
	sC->freq = sfreq ;
	sC->ascHead = ( char *) ( sC + 1 ) ;
	sC->data = ( int *) ( sC->ascHead + hsize  ) ;
	sC->ascHeadSize = hsize ;
	j = strlen(lines[1][1]) ;
	*(sC->component) = lines[1][1][j-3] ;
	ip = sC->data ;
	for( i = 0 ; i < ns ; i++ ) 
		if( fscanf(fd,"%d",ip++)  < 1 ) {
		  if( i < sfreq  ) 
			bclog(0,"Out of data reading sample %d",(void*)i) ; 
		  else {
			sC->nData = (i/sfreq)*sfreq ;
			sprintf(buff,"Changing nData from %d to %d for %s %d %06d %c",
				ns,sC->nData,lines[0][1],evDay(tt),
				evDTime(tt),lines[1][1][j-3] ) ;
			bclog(3,buff," ") ;
			break ;
		   }
		}
	cp = sC->ascHead ;
	sC->ascHeadN = nh - 5 ;
	for( i = 3 ; i <  nh - 2 ; i++ ) {
		strcpy( cp, lines[i][0] ) ;
		cp += (strlen(cp) + 1) ;
	}
/*	printf("%d %d %d\n",sC,sC->data,sC->ascHead ) ; */
	for( i = 0 ; i < nh ; i++ ) free(lines[i]) ;
	return(sC) ;
}
void sRPrintSilC( SilChannel *sC , FILE *fd )
{
	int *d,i,n,l,nc,ii ;
	char *cp ;
	fprintf(fd,"%s %s %s",sC->station,sC->component,ctime( &(sC->sTime)) ) ;
	fprintf(fd,"%d samples at %d samples/sec\n",sC->nData,sC->freq ) ;
	fprintf(fd,"Time of creation: %s",ctime(&(sC->cTime)) ) ;
	n = sC->ascHeadN ;
	cp = sC->ascHead ;
	while( n-- ) {
		l = 1+ strlen(cp ) ;
		fprintf(fd,"%s\n",cp ) ;
		cp +=  l  ;
	}
	n = sC->nData ;
	d = sC->data ;
	if(d == (int *) 0 ) return ;
	nc = 1 ;
	if(*(sC->component) == '3') nc = 3 ;
	for( ii = 0 ; ii < nc ; ii++) {
	for( i = 0 ; i < n ; i++ ) {
		fprintf(fd,"%6d ",d[i + n*ii] ) ;
		if( i == 4 ) {
			i = n - 5 ;
			fprintf(fd," -- ") ;
		}
	}
	fprintf(fd,"\n") ;
	}
}
void sRPutSilChannel ( SilChannel *sC , FILE *fd , int comp )
{
	time_t t ;
	char *compNames = "ZNE" ;
	char buff[200],*cp ;
	int i,j,l,n ;
	n = sC->nData ;
	t = sC->sTime ;
	j = cftime(buff,"%Y",&t) ;
	fprintf(fd,"%3s  %s\n",sC->station,buff ) ;
	j = cftime(buff,"/usr/sil/net/%h/%d/%H:%M:%S",&t) ;
	buff[13] ^= 32 ;
	fprintf(fd,"%s.00%c%02d\n",buff,compNames[comp],sC->freq%100) ;
	fprintf(fd,"%d\n",sC->freq) ;
	cp =  sC->ascHead ;
	for(i = 0 ; i < sC->ascHeadN ; i++ ) {
		l = 1+ strlen(cp ) ;	
		fprintf(fd,"%s\n",cp ) ;
		cp += l ;
	}
	fprintf(fd,"+data+\n%6d\n",sC->nData) ;
	for( i = 0 ; i < n ; i++ ) {
		fprintf(fd,"%d\n",sC->data[i+n*comp]) ;
	}
}
void sRPutSil( SilChannel *sC ) 
{
	char ttext[20],fname[60] ;
	int i,j ;
	FILE *fd ;
	char *compNames = "ZNE"  ;
	j = cftime(ttext,"%H%M%S",&(sC->sTime)) ;
	bclog(3,"SilChannel %s",ttext) ;
	for( i = 0 ; i < 3 ; i++ ) {
		sprintf(fname,"%s00%c%02d",ttext,compNames[i],sC->freq%100) ;
		fd = fopen(fname,"w") ;
		if(NULL == fd) bclog(1,"cannot create %s",fname) ;
		sRPutSilChannel ( sC,fd,i) ;
		fclose(fd) ;
	}
}
SilChannel *sRGetSilChannel( char *name, char comp , int compressed )
/*	read one channel of compressed sil data */
{
        int l ;
        SilChannel *s ;
        char pipe[200] ;
        FILE *pd ;
	if( compressed ) {
	        sprintf(pipe,"/usr/local/bin/uncompress < %s ",name) ;
        	l = strlen(pipe) ;
	        pipe[l-6]=comp ;
 	        pd = popen(pipe,"r") ;
		if(pd == NULL) bclog(-1,"cannot open %s",pipe) ;
	} else {
		sprintf(pipe,"%s",name) ;
		l = strlen(pipe) ;
		pipe[l-3] = comp ;
		pd = fopen(pipe,"r") ;
		if(pd == NULL) bclog(-1,"cannot open %s",pipe) ;
	}
        s = sRReadSil(pd) ;
        if(compressed) pclose(pd) ;  else fclose(pd) ;
        if( ( NULL == s ) || (comp != *(s->component)) ) {
                bclog(0,"problem reading %s",pipe) ;
                free(s) ;
                return(NULL) ;
        }
        return(s) ;
}
SilChannel *sRGetSil3Channel( char *name , int compressed )
/* read 3 channels of compressed sil data */
{
	SilChannel *sz,*sn,*se,*res ;
	char *pp ;
	int nd,hsize ;
        sz = sRGetSilChannel(name,'Z',compressed) ;
        sn = sRGetSilChannel(name,'N',compressed) ;
        se = sRGetSilChannel(name,'E',compressed) ;
	res = ( SilChannel *) ( sz && sn && se ) ;
	if(( sz->nData != sn->nData) || (sz->nData != se->nData))
		bclog(0,"compnents for %s have different number of samples",name) ;
	if( NULL == res) return(res) ;
	hsize = sizeof(*sz) + sz->ascHeadSize  ;
	nd = sz->nData ;
	res = ( SilChannel *) malloc( hsize + 12*nd )  ;
	pp = (char*) res ;
	memcpy(pp, sz, hsize+4*nd) ;
	pp += hsize+4*nd ;
	memcpy(pp, sn->data, 4*nd ) ;
	pp += 4*nd ;
	memcpy(pp, se->data, 4*nd ) ;
	free(sz) ; free(sn) ;free(se) ;
	*(res->component) = '3' ;
        res->ascHead = ( char *) (res + 1 ) ;
        res->data = ( int *) ( res->ascHead + res->ascHeadSize )  ;
	return(res) ;
}
int sRPutBCFile(  SilChannel  *sd , char *fname) 
{
	CData d ;
	FILE *bfd ;
	SilChannel outh ;
	int nd,j,i ;
	bfd = fopen(fname,"w") ;
	if(NULL == bfd ) bclog(0,"cannot create %s",fname) ;
	outh = *sd ;
	nd = outh.nData ;
	j = BCMAGIC ;
	outh.data = (int *) j ;
/* make littleendian corrections for header */
        bcSwapIf( (long*)&outh,4) ;
        if( bcLittleEndian()) swab( (char*)&(sd->freq),(char*)&(outh.freq),6) ;
 
	j = fwrite(&outh,N_BCS_HEAD,1,bfd) ;
	j = fwrite(sd->ascHead,sd->ascHeadSize,1,bfd) ;
	bcCompBit(sd->data,nd,&d,1) ;
	bcWrite(bfd,&d) ;
	bcCompBit(sd->data+nd,nd,&d,1) ;
	bcWrite(bfd,&d) ;
	bcCompBit(sd->data+2*nd,nd,&d,1) ;
	j = bcWrite(bfd,&d) ;
	fclose(bfd) ;
	if(j == 0 || nd != d.nData ) {
		unlink(fname) ;
		bclog(0,"error writing %s",fname) ;
	}
	return(0) ;
}
int sRPutBC( SilChannel *sd )
{
	char name[50] ;
	char j[40] ;
	cftime(j,"%H%M%S",&(sd->sTime)) ;
	sprintf(name,"%s%02d.bc",j,sd->freq%100) ;
	return( sRPutBCFile(sd,name)) ;
}
SilChannel *sRGetBc( char *fname ) {
	FILE *inf ;
	SilChannel tb, *res ;
	CData *d ;
	int j, *ip  ;
	inf = fopen(fname,"r") ;
	if(inf == NULL ) bclog(0,"Cannot open file %s",fname) ;
	bclog(7,"File %s opened ",fname ) ;
	j = fread(&tb,N_BCS_HEAD,1,inf) ;
/*
	fprintf(stderr,"j=%X tb=%X srX=%X inf=%X\n",&j,&tb,srX,inf) ;
		if(srX) sRPrintSilC(srX,stdout) ;
*/
	bcSwapIf( ( long*)&tb,4) ; 
	if( bcLittleEndian()) swab( (char*)&(tb.freq),(char*)&(tb.freq),6) ;
	if( (int) tb.data != BCMAGIC ) bclog(-1,"Bad magic number in %s",fname) ;
	tb.data = (int *) 0 ;
	if( j - 1 ) bclog(0,"Cannot read header from %s",fname) ;
/*	if(logLevel > 7 ) sRPrintSilC(&tb,stdout) ;	*/
	res = ( SilChannel *) malloc( sizeof(tb)+tb.ascHeadSize+12*tb.nData ) ;
	*res = tb ;
	res->ascHead = (char *) res + sizeof(tb) ;
	res->data = (int *)(res->ascHead + res->ascHeadSize) ;
	j = fread(res->ascHead,res->ascHeadSize,1,inf) ;
	if ( j != 1 ) bclog(-1,"problem reading ascii header",NULL) ;
	ip = res->data ;
	for( j = 0 ; j < 3 ; j++ ) {
		d = bcRead(inf) ;
		if( d == NULL) {
			bclog(2,"error reading %s",fname) ;
			free( res ) ;
			return(NULL) ;
		}
		if( bcCheckCData(d) ) bclog(0," error data from %s",fname) ;
		bcDeCompr(d,ip) ;	
		ip += res->nData ;
		free(d) ;
	}
/*	if(logLevel > 7 ) sRPrintSilC(res,stdout) ;	*/
	fclose(inf) ;
	return(res) ;
}
SilChannel *sRGetBcHeader( char *fname ) {
        FILE *inf ;
        SilChannel tb, *res ;
        CData *d ;
        int j, *ip  ;
        inf = fopen(fname,"r") ;
        if(inf == NULL ) bclog(0,"Cannot open file %s",fname) ;
        bclog(7,"File %s opened ",fname ) ;
        j = fread(&tb,N_BCS_HEAD,1,inf) ;
        bcSwapIf( ( long*)&tb,4) ;
        if( bcLittleEndian()) swab( (char*)&(tb.freq),(char*)&(tb.freq),6) ;
        if( (int) tb.data != BCMAGIC ) bclog(-1,"Bad magic number in %s",fname) ;
        tb.data = (int *) 0 ;
        if( j - 1 ) bclog(0,"Cannot read header from %s",fname) ;
        res = ( SilChannel *) malloc( sizeof(tb)+tb.ascHeadSize+12*tb.nData ) ;
        *res = tb ;
        if ( j != 1 ) bclog(-1,"problem reading ascii header",NULL) ;

        fclose(inf) ;
        return(res) ;
}
void sRBcPath (int date, char *path )
{
	int day,mon,year ;
	static char *month[] ={ "0","jan","feb","mar","apr","may","jun",
		"jul","aug","sep","oct","nov","dec" };
	char *base ;
	base = getenv("BC_BASE") ;
	if(NULL == base) base = "/sil/bc" ;
	day = date%100 ;
	mon = (date/100)%100 ;
	year = date/10000 ;
	if( year < 1970 ) year += 1900 ;
	sprintf(path,"%s/%04d/%s/%02d",base,year,month[mon],day) ;
	bclog(5,path, " " ) ;
}
int sRPutBCFileTree( SilChannel *sd )
{
	static char oDir[60] ;
	char dir[60],name[80],fn[30] ;
	int day,j ;
	day = evDay( sd->sTime) ;
	sRBcPath(day,dir) ;
	sprintf(dir,"%s/%s",dir,sd->station ) ;
	bclog(6,"dir = %s",dir) ;
	if(strcmp(dir,oDir)) {
		j = utMkPath(dir,0755 ) ;
		if( j < 0 ) bclog(0,"Cannot make directory : %s",dir) ;
		strcpy(oDir,dir) ;
	}
        cftime(fn,"%H%M%S",&(sd->sTime)) ;
        sprintf(name,"%s/%s%02d.bc",dir,fn,sd->freq%100) ;
	bclog(4,"write to  %s",name) ;
	return(sRPutBCFile( sd,name)) ;

}
void sRFixPointers(SilChannel *s)
{
	s->ascHead = ( char * ) (s + 1) ;
	s->data = (int *) (s->ascHead +  s->ascHeadSize ) ;
}
SilChannel *sRSilCut( SilChannel *s, int skip, int length) 
/* cut part of data */
{
	SilChannel *ss ;
	int left,n,nc,i,j,*ip,*op ;
	nc = 1 ;
	if( *(s->component) == '3') nc = 3 ;
	left = s->nData - s->freq * skip ;
	if( left < 0  ) return(0) ;
	n = length * s->freq ;
	if( n > left ) n = left ;
	ss = ( SilChannel *) malloc( sizeof(*ss)+s->ascHeadSize + 12*n) ;
	memcpy(ss,s,sizeof(*ss)+s->ascHeadSize ) ;
	bclog(7,"cut : n = %d",( void *) n ) ;
	ss->nData = n ;
	ss->sTime += skip ;
	sRFixPointers(ss) ;
	for(j = 0 ; j < nc ; j++ ) {
		ip = s->data + s->nData*j + skip*s->freq ;
		op = ss->data + n*j ;
		i = n ;
		while(i--) *op++ = *ip++ ;
	}
	return(ss) ;
}
SilChannel *sRSilCopy( SilChannel *s)
/* allocate and space and copy data to it */
{
	SilChannel *ss ;
	int size ;
	size = 12*s->nData + sizeof(*s) + s->ascHeadSize   ;
	ss = ( SilChannel *) malloc(size) ;
	memcpy(ss,s,size ) ;
	sRFixPointers(ss) ;
	return(ss) ;
}
SilChannel *sRSilCat( SilChannel *sc1, SilChannel *sc2 )
{
/*
	Check is s1 and s2 touch or overlap, if so they
	are merged and the result returned. Otherwise NULL is
	returned. Space for result is allocated, sc1 and sc2 are
	not changed.
*/
	SilChannel *s1,*s2,*sr,*null ;
	int diff,f,n,i,j,*op,hsize, *ip1,*ip2 ;
	if( sc1->sTime < sc2->sTime ) { s1 = sc1 ; s2 = sc2 ; }
		else { s1 = sc2 ; s2 = sc1 ; }
	null = ( SilChannel *) 0 ;
	if( s1->freq != s2->freq ) {
		bclog(3,"Different sample rates in SilCat",ctime(&s1->sTime)) ;
		return(null) ;
	}
	if( strcmp(s1->station,s2->station )) {
		bclog(3,"Different stations in SilCat",s2->station)  ;
		return(null) ;
	}
	if( ( *(s1->component)!='3') || (*(s1->component)!='3') ) {
		bclog(3," Sil cat supports only 3 component data"," ") ;
		return(null) ;
	}
	f = s1->freq ;
	diff = f*(s2->sTime - s1->sTime) ;
	if( diff > s1->nData ) {
		bclog(6,"Gap between waveforms in SilCat",ctime(&s1->sTime)) ;
		return(null) ;
	}
	n = diff+s2->nData ;
	hsize = sizeof(*sr) + s1->ascHeadSize ;
	sr = ( SilChannel *) malloc(hsize + 12*n ) ;
	memcpy(sr,s1,hsize) ;
/*
	sr->ascHead = ( char *) (sr  + 1)  ;
	sr->data = ( int *) (sr->ascHead + sr->ascHeadSize) ;
*/
	sr->nData = n ;
	sRFixPointers(sr) ;
	for( j = 0 ; j < 3 ; j++) {
		op = sr->data + n*j ;
		ip1 = s1->data + j*s1->nData ;
		ip2 = s2->data + j*s2->nData ;
		for( i = 0 ; i < diff      ; i++ ) *op++ = *ip1++ ;
		for( i = 0 ; i < s2->nData ; i++ ) *op++ = *ip2++ ;
	}
	return(sr) ;
}
void sRPutIndex(FILE *ifd, SilChannel *sd) 
{
	static BcIndex index ;
	strncpy(index.station,sd->station,4) ;
	index.sTime = sd->sTime ;
	index.nSamp = sd->nData ;
	index.freq  = sd->freq ;
	bcSwapIf( &index.sTime,3) ;
	fwrite( (void *) &index, sizeof(index),1,ifd) ;
}
void sRMkIndex( int date) 
{
	char path[200],find[200],file[200],iFile[200] ;
	FILE *pd,*ifd ;
	SilChannel *sd ;
	int i ;
	sRBcPath(date,path) ;
	sprintf(find,"find -L %s -name \"????????.bc\" -size +0 -print",path) ;
	bclog(5,find," ")  ;
	sprintf(iFile,"%s/index",path) ;
	ifd = fopen(iFile,"w") ;
	if( ifd == NULL ) bclog(0,"cannot open index file %s",iFile) ;
	pd = popen(find,"r") ;
	if(pd == NULL) bclog(0,"cannot open pipe: %s",find) ;
	while( 1== fscanf(pd,"%s",file) ) {
		bclog(8,file," ") ;
		sd = sRGetBc(file) ;
		if(sd) {
			sRPutIndex(ifd,sd) ;
			free(sd) ;
		}
	}
	pclose(pd) ;
	fclose(ifd) ;
}
void sRSil2BcDay( int date )
/*
Read all files on /eq0 for a given day, convert to bc and write to BC_BASE 
*/
{	
	time_t tt ;
	FILE *pdd ;
	SilChannel *sp ;
	int j,dd ;
	char silpath[100],path[20],days[20],fname[40] ;
	bclog(6,"%d",( void *) date ) ;
	tt = evTime(date,1) ;
	cftime(path,"/eq0/%Y",&tt) ;
	cftime(days,"%b/%d",&tt) ,
	*days ^= 32 ;
	sprintf(silpath,
	"( cd %s ; for i in ??? ; do  find %s/$i/%s -name \"*E??.Z\" ; done 2>/dev/null)",
		path,path,days) ;
	pdd = popen(silpath,"r" ) ;
	while( fgets(fname,46, pdd  )) {
		fname[strlen(fname)-1]= 0 ;
		bclog(6,fname," ") ;
	
		sp = sRGetSil3Channel(fname,1) ;
		dd = evDay(sp->sTime) ;
		if(dd != date ) bclog(3,"got data from different day: %d",(void *) dd) ;
		j = sRPutBCFileTree( sp ) ;
		free(sp) ;
	}
	sRMkIndex(date) ;
}
void sRMoveBc( char *file ) 
/*
        Move file to BC_BASE tree and update index.
*/
{
	SilChannel *s ;
	char path[140] ;
	FILE *ifd ;
	int r ;
	s = sRGetBc(file) ;
	r = sRPutBCFileTree(s) ;
	sRBcPath(evDay(s->sTime),path) ;
	strcat(path,"/index") ;
	bclog(8,"append to %s",path) ;
	ifd = fopen(path,"a") ;
	if(NULL == ifd ) bclog(0,"Cannot append to %s",path) ;
	sRPutIndex(ifd,s) ;
	free(s) ;
	fclose(ifd) ;
	if( unlink(file) ) bclog(0,"Error removing %s",file ) ;
}
void sRCheckSpace(char *fs, int kb, int mfiles)
{
#ifdef sun
#include <sys/statvfs.h>
/*  quit if there are less than kb kb or mfiles inodes free on file system */
	int j,space,files ;
	struct statvfs buf ;
	j = statvfs(fs,&buf ) ;
	bclog(5,"File system %s",fs) ;
	bclog(9,"statvfs returns %d", ( void *) j ) ;
	space = buf.f_bavail*(buf.f_frsize/1024) ;
	files = buf.f_ffree ;
	bclog(5,"%d blocks available ",( void *) space) ;
	bclog(5,"%d files free",( void *) files ) ;
	if( space < kb ) bclog(0,"Insufficient space on %s",fs) ;
	if( files < mfiles ) bclog(0,"Insufficient inode space on %s",fs) ;
#endif
}
void sRSetDay( int day, char *days )
{
	static char buff[200] ;
	int j ;
	time_t t ;
	time(&t) ;
	t -= 24*3600*day ;
	j = cftime(days,"%b/%d",&t) ;
	*days ^= 32 ;
	bclog(7,"Days = %s",days) ;
}
void sRMoveList(char *base, int day, int silFlag )
{
	char cmd[400],path[60],days[50],file[90]   ;
	int j ;
	char *cp ;
	FILE *pd ;
	sRSetDay( day,days) ;
	if ( silFlag ) sprintf(cmd,
	"(cd %s ;  find %s -name \"*.bc\" -size +0   2>/dev/null)",
		base, days) ;
	else sprintf(cmd,
	"(cd %s ; for i in ??? ; do find $i/%s -name \"*.bc\" -size +0 ; done \
2>/dev/null)", base, days) ;
	bclog(6,"%s",cmd) ;
	pd = popen(cmd,"r") ;
	j = 0 ;
	while( fgets(path,56,pd) ) {
		cp = path ;
		while( *cp) {if(*cp == '\n' ) *cp = 0 ; cp++ ;} 
		sprintf(file,"%s/%s",base,path) ;
		bclog(5,file," ") ;
		sRMoveBc(file) ;	
		j++ ;
	}
	bclog(4,"moved %d waveforms",( void *) j) ;
	pclose(pd) ;
}
