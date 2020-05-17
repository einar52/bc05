
/*
This program takes one day of bc data and
converts to ah, and writes on specified directories rooted
on working directory


*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <rpc/rpc.h>
#include <ahhead.h>
#include <ah.h>
#include "bitcomprl.h"
#include "sil.h" 
#include "util.h"
#include "silread.h"
SilBcHead baH ;
ahhed baAhh ;
char *progname = "bc2ahday" ;
char *baInDir,baStation[20],*baBase  ;
char baOName[200] ;
int logLevel = 3 ;
void help()
{
	fprintf(stderr,"%s\n","\
This program takes one day of bc data and\n\
converts to ah, and writes on specified directories rooted\n\
on working directory\n\
") ;
	exit ;
}
void Err( char *s1, char *s2 )
{
	fprintf(stderr,"Error in bc2dayah: ") ;
	fprintf(stderr,s1,s2) ;
	fprintf(stderr,"\n") ;
	exit(-1) ;
}
void bclog( int level , char *s1 , void *s2 )
{
        if( logLevel < level  ) return ;
        fprintf(stdout,"bc2ah %d: ",level ) ;
        fprintf(stdout,s1,s2) ;
        fprintf(stdout,"\n")  ;
        fflush(stdout) ;
        if( level < -1 ) abort() ;
        if( level <  0 ) exit(0) ;
}

void bcGetHead( ) 
{
	FILE *inf ;
	XDR inx ;
	char fname[200] ;
	if( 0==strcmp(baH.station,baStation)) return ;
	strcpy(baStation,baH.station) ;
	sprintf(fname,"/sil/usr/etc/%s.head",baStation) ;
	inf = fopen(fname,"r") ;
	if( NULL == inf) Err("cannot open  %s",fname) ;
	xdrstdio_create(&inx,inf,XDR_DECODE) ;
	xdr_gethead(&baAhh,&inx) ;
	xdr_destroy(&inx) ;
	fclose(inf) ;
	printf("newstation : %s %s %d\n",baStation,
		baAhh.station.stype,baAhh.record.type) ;
}
void bcGetHeadL( SilChannel *sP ) 
{
	FILE *inf ;
	XDR inx ;
	char fname[200] ;
	if( 0==strcmp(sP->station,baStation)) return ;
	strcpy(baStation,sP->station) ;
	sprintf(fname,"/sil/usr/etc/%s.head",baStation) ;
	inf = fopen(fname,"r") ;
	if( NULL == inf) Err("cannot open  %s",fname) ;
	xdrstdio_create(&inx,inf,XDR_DECODE) ;
	xdr_gethead(&baAhh,&inx) ;
	xdr_destroy(&inx) ;
	fclose(inf) ;
	printf("newstation : %s %s %d\n",baStation,
		baAhh.station.stype,baAhh.record.type) ;
}
void bcHeadOut( FILE **outf, XDR *o ) 
{
	char on2[2000] ;
	struct tm *tp ;
	tp = gmtime(&baH.sTime) ;
	baAhh.record.abstime.yr = tp->tm_year ;
	baAhh.record.abstime.mo = tp->tm_mon+1 ;
	baAhh.record.abstime.day = tp->tm_mday ;
	baAhh.record.abstime.hr = tp->tm_hour ;
	baAhh.record.abstime.mn = tp->tm_min ;
	baAhh.record.abstime.sec = tp->tm_sec ;
	baAhh.record.ndata = baH.nData ;
	baAhh.record.delta = 1.0 / baH.freq ;
	utAhPath( baOName , baH.station, &baH.sTime);
	sprintf(on2,"%s/%s",baBase,baOName) ;
	*outf = utPathOpen(on2) ;
	if( NULL == outf) Err("cannot create %s",on2) ;
	xdrstdio_create(o,*outf,XDR_ENCODE) ;
}
void bcHeadOutL( FILE **outf, XDR *o , SilChannel *sp ) 
{
	char on2[2000] ;
	struct tm *tp ;
	tp = gmtime(&(sp->sTime)) ;
	baAhh.record.abstime.yr = tp->tm_year ;
	baAhh.record.abstime.mo = tp->tm_mon+1 ;
	baAhh.record.abstime.day = tp->tm_mday ;
	baAhh.record.abstime.hr = tp->tm_hour ;
	baAhh.record.abstime.mn = tp->tm_min ;
	baAhh.record.abstime.sec = tp->tm_sec ;
	baAhh.record.ndata = sp->nData ;
	baAhh.record.delta = 1.0 / sp->freq ;
	utAhPath( baOName , sp->station, &sp->sTime);
	bclog(6,"name of output file is %s",baOName ) ;
	sprintf(on2,"%s/%s",baBase,baOName) ;
	bclog( 5,"Create file %s",on2) ;
	*outf = utPathOpen(on2) ;
	if( NULL == outf) Err("cannot create %s",on2) ;
	xdrstdio_create(o,*outf,XDR_ENCODE) ;
}
int getRec( FILE *inf, XDR *o, char *comp ) 
{
	CData  *d  ;
	float *data ;
	int i,*idata ;
	d = bcRead( inf ) ;
	if( bcCheckCData(d) ) Err(" error on Z in %s"," ") ;
	data = ( float *) calloc(baH.nData,sizeof( float ) ) ;
	idata = ( int *) calloc(baH.nData,sizeof( int ) ) ;
	bcDeCompr(d,idata) ;
	for( i = 0 ; i < baH.nData ; i++ ) {
		data[i] = idata[i] ;
	}
	strcpy(baAhh.station.chan,comp) ;
	xdr_puthead(&baAhh,o ) ;
	xdr_putdata(&baAhh,data,o) ;
	free(idata) ;
	free(data) ;
	free(d) ;
	return(d->nData) ;
}
void baPutCatAcc( int  flag , SilChannel  *sp ) 
{
	static char accName[200] ;
	static FILE *af ;
	char s1[200],s2[200] ;
	if( flag ) { pclose(af) ; return ; }
	cftime(s1,"%Y/%b/%d/cat.acc",&sp->sTime) ;
	utLCase(s1) ;
	sprintf(s2,"sort > %s/%s",baBase,s1) ;
	if( strcmp(s2,accName) ) {
		bclog(4,"command : %s",accName) ;
		if( af) pclose(af) ;
		af= popen(s2,"w") ;  
		strcpy(accName,s2) ;
	}
	fprintf(af,"%s %d\n",baOName,sp->nData/sp->freq) ;
}
void getFile( char *fname )  /* this is for the old version of bc */
{
	FILE *inf,*of ;
	XDR ofx ;
	int nn ;
	inf = fopen(fname,"r") ;
	if( inf == NULL ) Err("cannot open %s",fname) ;
	fread(&baH,sizeof(baH),1,inf) ;
	bcGetHead() ;
	if( baH.freq == 100 ) {
		bcHeadOut(&of, &ofx ) ;
		nn = getRec(inf,&ofx,"Z") ;
		nn = getRec(inf,&ofx,"N") ;
		nn = getRec(inf,&ofx,"E") ;
/*
		printf("%s station=%s freq=%d nData=%d %d\n",
			fname,baH.station,baH.freq,baH.nData,nn) ;
*/
		xdr_destroy(&ofx) ;
		fclose(of) ;
/* 		baPutCatAcc(0) ; */
	}
	fclose(inf) ;
	
}
SilChannel *getBc( char *fname ) {
	FILE *inf ;
	SilChannel tb, *res ;
	CData *d ;
	int j, *ip  ;
	inf = fopen(fname,"r") ;
	if(inf == NULL ) bclog(0,"Cannot open file %s",fname) ;
	bclog(7,"File %s opened ",fname ) ;
	j = fread(&tb,N_BCS_HEAD,1,inf) ;
	if( (int) tb.data != BCMAGIC ) bclog(-1,"Bad magic number in %s",fname) ;
	tb.data = (int *) 0 ;
	if( j - 1 ) bclog(0,"Cannot read header from %s",fname) ;
	if(logLevel > 7 ) sRPrintSilC(&tb,stdout) ;
	res = ( SilChannel *) malloc( sizeof(tb)+tb.ascHeadSize+12*tb.nData ) ;
	*res = tb ;
	res->ascHead = (char *) res + sizeof(tb) ;
	res->data = (int *)(res->ascHead + res->ascHeadSize) ;
	j = fread(res->ascHead,res->ascHeadSize,1,inf) ;
	if ( j != 1 ) bclog(-1,"problem reading ascii header",NULL) ;
	ip = res->data ;
	for( j = 0 ; j < 3 ; j++ ) {
		d = bcRead(inf) ;
		if( bcCheckCData(d) ) bclog(-1," error data from %s",fname) ;
		bcDeCompr(d,ip) ;	
		ip += res->nData ;
		free(d) ;
	}
	if(logLevel > 7 ) sRPrintSilC(res,stdout) ;
	fclose(inf) ;
	return(res) ;
}
void getFileL( char *fname ) /* get bcl (new version ) bc file */
{	
	FILE *of ;
	float *data ;
	SilChannel *sP ;
	char *clist ;
	int i,j,*dP ;
	XDR ofx ;
	sP = getBc(fname) ;
	clist = "ZNE"  ;
	data = ( float *) calloc(sP->nData,sizeof(*data)) ;
	bcGetHeadL(sP) ;
	bcHeadOutL( &of, &ofx, sP) ;
	dP = sP->data ;
	for(i = 0 ; i < 3 ; i++ ) {
		bclog(6,"work on component %d",(void *)i) ;
		for(j=0 ; j < sP->nData;j++) data[j] = dP[j] ;
		dP += sP->nData ;
		*(baAhh.station.chan) = clist[i] ;
		baAhh.station.chan[1] = 0 ;
		xdr_puthead(&baAhh,&ofx) ;
		xdr_putdata(&baAhh,data,&ofx) ;
	}
	xdr_destroy(&ofx) ;
	fclose(of) ;
	baH.sTime = sP->sTime ;
	baPutCatAcc(0,sP) ;
	free(data) ;
	free(sP) ;
}
void doit()
{	
	FILE *po  ;
	char pp[500],fname[200] ;
	sprintf(pp,"find %s -name \"1[89]*.bc\" -print | sort ",baInDir) ;
	po = popen(pp,"r") ;
	while( 1 == fscanf(po,"%s",fname) ) {
		getFileL( fname) ;
	}
	pclose(po) ;
	baPutCatAcc(1, ( SilChannel *) 0 ) ;
}
void getPars( int ac , char **av )
{
	extern  char *optarg ;
	int c ;
	while(( c = getopt(ac,av,"b:hH?i:f:l:")) != EOF ) 
		switch(c) {
		case 'i' : baInDir = optarg ; break ;
		case 'b' : baBase = optarg ; break ;
		case 'f' : getFileL(optarg) ; break ;
		case 'l' : logLevel = atoi(optarg) ; break ;
		case 'h' :
		case 'H' :
		case '?' :  help() ; break ;
		
	}
}
int main( int ac , char **av )
{
	getPars( ac,av) ;
	if ( baInDir ) doit() ;
	return(0) ;
}
