/*
	This a program for testing methods for ComPressing seismograms
	There is no attempt to support little-endian like ???86 and vax.
 */
#include <stdio.h>
#include <stdlib.h>
#define b8 127
#define b16 077777
#include "bitcompr.h"
int cpCompareT(int *a,int *b,int n)
{
	int d ;
	if(n < 1 ) return(0) ;
	while(n--) if( d = *a++ - *b++) return (d) ;
	return(0) ;
}
void cpComPr( int *ind, int n, CData *outd )
{
	int val,aval,sflag,vflag ;
	char *sp,*cp,*op,*scount ;
	outd->nData = n ;
	if( n < 1 ) return ;
	sp = 2+ (char *) &val ;
	cp = 1 + sp ;
	op = outd->data ;
	sflag = 1 ;			/* start in s (16 bit) mode */
	scount = op++ ;
	*scount = 0 ;
	while(n--) {
		val = *ind++ ;
		vflag = abs(val) > 127 ;
		if( sflag) {
			if(vflag) {
				(*scount)++ ;
				*op++ = *sp ;
				*op++ = *cp ;
			} else {
				sflag = 0 ;
				*op++ = *cp ;
			}
		} else {
			if(vflag) {
				*op++ = -128 ;
				scount = op ;
				*op++ = 1 ;
				*op++ = *sp ;
				*op++ = *cp ;
				sflag = 1 ;
			} else *op++ = *cp ;
		}
	}	
	outd->size = op - ( char *)outd ;
	outd->type = 1 ;
}
int cpDeCompr(  CData *cDat , int *outd )
{
	int count,i,sflag ;
	short sval ;
	char *cp, *sp, *ip ;
	sp = (char *)&sval ;
	cp = 1 + sp ;
	sflag = 1 ;
	ip = cDat->data ;
	count = *ip++ ;
	for(i=0 ; i < cDat->nData ; i++ ) {
	/*	printf("count=%d i=%d sval=%d\n",count,i,sval) ; */
		if ( count ) {
			*sp = *ip++ ;
			*cp = *ip++ ;
			count-- ;
		} else {
			sval = *ip++ ;
			if( sval == -128 ) {
				count = *ip++ - 1 ;
				*sp = *ip++ ;
				*cp = *ip++ ;
			} 
		}	
		*outd++ = sval ;
	}
	return( i ) ;
}
void cpPrData(int *d1,int n)
{
	int i,sum,steps,pen ;
	sum = *d1 ; steps = 0 ;
	for( i = 1 ; i < n ; i++ ) {
		sum += d1[i] ;
		if(d1[i] != d1[i-1]) steps++ ;
	}
	pen = 14*steps + sum ;
/**/
	for( i = 0 ; i < n ; i++ ) {
		fprintf(stdout,"%d ",d1[i] ) ;
		if ((i%30 == 29 ) || ( i == n-1)) fprintf(stdout,"\n") ;
	}
/**/
	fprintf(stdout,"n=%d sum=%d steps=%d avegage=%5.2f pen=%d pen/n=%4.2f\n",
			n,sum,steps,1.0*sum/n,pen,1.0*pen/n ) ;
}
int cpGetstd(int *d1)
{
	int i,n ;
	(void) fscanf(stdin,"%d",&n) ;
	fprintf(stdout,"n=%d\n",n) ;
	for(i = 0 ; i < n ; i++) (void)fscanf(stdin,"%d",d1+i) ;
	return(n) ;
}
void dumpb( char *cp , int n )
{
	int i ;
	for( i = 0 ; i < n ; i++ ) {
		fprintf(stdout,"%4d",cp[i]) ;
		if( ( i%20==19) ||( i == n-1)) fprintf(stdout,"\n") ;
	}
}
#include <math.h>
int cpMkTestD(int *out, int n) 
{
	int i ;
	double f, df,arg,val ;
	f = 0.15 ;
	df = ( 1.5 - f) / n ;
	arg = 1.0 ;
	for( i = 0 ; i < n ; i++ ) {
		val = pow(10.0,cos(arg)+1.0) ;
		*out++ = val ;
		arg += f ;
		f += df ;
	}
	return n ;
}
int getFile(int *out , char *name ) 
{
	FILE *fd ;
	float fv ;
	int i ;
	fd = fopen(name,"r") ;
	i = 0 ;
	while( 1==fscanf(fd,"%f ",&fv)) {
		*out++ = fv ; i++ ;
	}
	return i ;
}
void cpTest1()
{
	int d1[MCDATA],d2[MCDATA] ;
	int n ;
	CData cd1,cd2 ;
	n = cpGetstd(d1);
	cpPrData(d1,n) ;
	cpComPr(d1,n,&cd1) ;
	dumpb( (char*)&cd1,cd1.size) ; 
	cpDeCompr(&cd1,d2) ;
	cpPrData(d2,n) ;
}
void cpTest2()
{
	int d1[MCDATA],d2[MCDATA],d3[MCDATA] ;
	int n,nn,nd,len ;
	CData cd1,cd2 ;
	n = cpGetstd(d1) ;  
	n = cpMkTestD(d1,80) ; 
/*
	n = getFile(d1,"x")  ;
*/
	bcDiff(d1,d2,n) ;
	cpComPr(d2,n,&cd1) ;
	bcDiff(d2,d2,n) ;
	cpComPr(d2,n,&cd2) ;
	nn = cpDeCompr(&cd1,d3) ;	
	bcIntegrate(d3,d3,n) ;
	nd = cpCompareT(d3,d1,n) ;
	fprintf(stdout,"cd1.size=%d cd2.size=%d n=%d nn=%d nd=%d\n",cd1.size,cd2.size,n,nn,nd) ;
	bcDiff(d1,d1,n) ;
	bcGetMag(d1,d3,n) ;
	cpPrData(d1,n) ;
	cpPrData(d3,n) ;
	len = 2 ;
	fprintf(stdout,"len=%d\n",len) ;
	bcMagFilt(d3,d2,n,len) ;
	cpPrData(d2,n) ;
	bcMagFilt2(d2,d2,n,len) ;
	cpPrData(d2,n) ;
}
void cpTest3()
{
        int d1[MCDATA],d2[MCDATA],d3[MCDATA] ;
        int j,n,nn,nd,len,res ;
	FILE *f ;
        CData cd1,cd2,*cdp,*c2 ;
	n = cpMkTestD(d1,40) ;	
/* 	n = getFile(d1,"x") ;  */
	res = bcCompBit(d1,n,&cd1,1) ;
	cdp = ( CData *) malloc(res) ;
	memcpy( cdp, &cd1, res) ;
	for(j = 0 ; j 	< 1 ; j++ ) 
		nn = bcDeCompr(cdp,d3) ;
	f = fopen("junk.bc","w") ;
	j = bcWrite(f,cdp) ;
	fclose(f) ;
	fprintf(stderr,"res=%d n=%d c1=%d c2=%d nn=%d comp=%d j=%d\n",res,n,
		bcCheckCData(&cd1), bcCheckCData(cdp),nn,
		bcCompareI(d1, d3,nn),j);
	f = fopen("junk.bc","r") ;
	c2 = bcRead(f) ;
	fclose(f) ;
	fprintf(stderr," bcRead returns %d \n",c2) ;
	fprintf(stderr,"size=%d nData=%d check=%d\n",
		c2->size,c2->nData,bcCheckCData(c2)) ;
}
void cpBitTest()
{
	BitBuffer bb ;
	char buf[400] ;
	int i ;
	char *cp ;
	bcInitBits( &bb,buf ) ;
	for( i = -16 ; i < 17 ; i++ ) bcPutBits ( &bb,i,5) ; 
	bcFlushBits(&bb) ;
	bcResetBits(&bb) ;
	for( i = 0 ; i < 33 ; i++ ) printf("%4d ",bcGetBits(&bb,5)) ; 
	printf("\n") ;
	
}
int main(int ac, char **av)
{
/*
	cpBitTest() ;
*/
	cpTest3() ;
	return(0) ;
}
