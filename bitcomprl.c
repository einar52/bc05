
/*
© Copyright 1998,2005,2020 Einar Kjartansson

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
/*
 * This does currently only work on 32 bit Linux systems 
 *
 * Changes made 2. oct 2005 :
 * Use 40 bit workbuffer when shifting and packing bits, allows for
 * full 32 bits of data instead of 24. Changes in bcPutBits() bcGetBits()
 * and bitcomprl.h
 *
 * Changed in bcGetMag() from
 * v = 1 ; while( v <= a ) { r++ ; v += v ; }
 * to
 * v = 0 ; while( v < a ) { r++ ; v += v + 1; }
 * Older version resulted in infinte loops for a >= 2^30 
 * as range for int is -2^31 to 2^31 - 1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitcomprl.h"
int bcLittleEnd  = 0 ;
void bcResetBits( BitBuffer *b ) 
{
	b->bytes = 0 ; b->bits = 0 ; b->work = 0 ; 
}
int bcLittleEndian() /* return true (1) if on little endain machine */
{
	int j ;
	char *cp ;
	cp = ( char *) & j ;
	j = 57 ;
	return ( *cp == 57 ) ;
}
void bcSwapL( long *val) /* convert bytes in longint, big vs. little endian  */
{
        char *p,t ;
        p = (char *)val ;
        t = *p   ; *p   = p[3] ; p[3] = t ;
        t = p[1] ; p[1] = p[2] ; p[2] = t ;
}
void bcSwapIf( long *p, int n ) /* swap array of n long, if on little end */
{
	if ( bcLittleEndian() ) while(n-- ) bcSwapL(p++) ;
}
void bcInitBits( BitBuffer *b,  char *data)
{
	int i,r,l ;
	bcResetBits(b) ;
	r = 1 ;  l = -1 ;
	bcLittleEnd = bcLittleEndian() ;
	for( i = 0 ; i < 40 ; i++ ) { 
		bcBitRight[i] = r ;
		bcBitLeft[i] = l ;	 
		bcBit[i] = l & r ;
		r += r + 1 ;
		l <<= 1 ;
	}
/*	b->data = ( char *) calloc(size,1) ; */
	b->data = data ;
/*	b->size = size ; */
}
void bcPutBits(  BitBuffer *b, int val, int nbits) 
{
	char *cp ;
	b->work |= ((val & bcBitRight[nbits-1]) << ( 40 - b->bits -nbits )) ;
	b->bits += nbits ;
	if( bcLittleEnd )  
		cp = (char *) &(b->work) +4  ;
	else
		cp = (char *) &(b->work) +3; /* líklega +3 */
	while ( b->bits >= 8 ) {
		b->data[ b->bytes++ ] = *cp ;
		b->work  <<= 8 ;
		b->bits -= 8 ;	
	}
}
int bcFlushBits(BitBuffer *b ) 
{
	if (b->bits) bcPutBits(b,0,8) ;
	return(b->bytes) ;
}
int bcGetBits( BitBuffer *b, int nbits ) 
{
	long long  res ;
	char *cp ;
	if( bcLittleEnd) 
		cp = ( char *) &(b->work) ;
	else
		cp = ( char *) &(b->work) + 7 ; /* á líklega að vera +7 */
	while( b->bits < 32 ) {
		b->work <<= 8 ;
		*cp = b->data[b->bytes++] ;
/*
		if( bcLittleEnd ) 
			* ( (char* ) &(b->work)) = b->data[b->bytes++] ;
		else 
			* ( (char* ) &(b->work)+3) = b->data[b->bytes++] ;
*/
/*		b->work |= (b->data[b->bytes++] & 255) ; */
		b->bits += 8 ;
	}
	b->bits -= nbits ;
	res = bcBitRight[nbits-1] & (b->work >> b->bits) ;
	if( res & bcBit[nbits-1] ) res |= bcBitLeft[nbits-1] ;
	return (res) ;
/*
	fprintf(stdout," w=%x ",b->work) ;
	fprintf(stdout,"b=%d w=%x ",b->bits,b->work) ;
	fprintf(stdout,"w=%x b=%d res=%d\n",b->work, b->bits, res) ;
*/
}
int bcComprArr(int *data,int *mag,int n,char *buff)
{
	BitBuffer bb ;
	int om,i,m,d ;
	om = -1 ;
	bcInitBits(&bb,buff) ;
	for( i = 0 ; i < n ; i++ ) {
		m = mag[i] ;
		d = data[i];
		if( om != m ) {
			if(i) bcPutBits(&bb,bcBit[om-1],om) ;
			bcPutBits(&bb,m,6) ;
			om = m ;
		}
		bcPutBits(&bb,d,m) ;
	}
	return( bcFlushBits(&bb) ) ;
}
void bcDeComprArr(char *buff, int *data, int n	) 
{
	BitBuffer bb ;
	int m,i,d ;
	bcInitBits(&bb,buff) ;
	m = bcGetBits(&bb,6) ;
	for ( i = 0 ; i < n ; i++ ) {
		d = bcGetBits(&bb,m) ;
		if( d == bcBitLeft[m-1] ) {
			m = bcGetBits(&bb,6) ;
			d = bcGetBits(&bb,m) ;
		} 
		data[i] = d ;
	}
}
void bcDiff(int *inp, int *outp, int n)  
		/* inp and outp may be same */
{
	int old,d ;
	if( n < 1 ) return ;
	old = 0 ;
	while(n--) {
		d = *inp++ ;
		*outp++ = d - old ;
		old = d ;
	}
}
void bcMagFilt( int *inp,int *out,int n,int len )
{					/* fill in lows in curve */
	int i,ll,a,aold,amax,j,aa,jen ;	/* writes one value at end of input */
	i = 0 ;
	aold = 999 ;
	jen = inp[n] ;
	inp[n] = 999 ;
	while( i < n ) {
		a = inp[i] ;
		if( aold <= a ) {
			out[i++] = a ;
			aold = a ;
		} else {
			ll = 1+ n - i ;
			if( ll > len ) ll = len ;
			amax = a ;
			for( j = 0 ; j < ll ; j++ ) {
				aa = inp[i+j] ;
				if( aa > amax) amax = aa ;
			}	
			if( amax > a) {
				 if( aold > amax) aold = amax ;
				 while(inp[i] <= a) out[i++] = aold ;
			} else {
				out[i++] = a ;
				aold = a ;
			}
		}
	}	
	inp[n] = jen ;
}
void bcMagFilt2( int *inp, int *out, int n, int len )
		/* Fill in on slopes  bcMagFilt should be caled first */
{
	int i2,a1,a2,i,a,ii ;
	a1 = 0 ;
	a2 = inp[0] ; i2 = 0 ;
/*	fprintf(stdout,"i=%2d i2=%2d a1=%d a2=%d a=%d\n",i,i2,a1,a2,a) ; */
	for( i = 0 ; i <= n ; i++) {
		if(i < n ) {
			a= inp[i] ;
			out[i] = a ;
		} else a = 0 ;
		if( a != a2 ) {
			if( ((i - i2) < len ) && ( a>a2 )) {
					for( ii = i2 ; ii < i ; ii++) 
						out[ii] = a ;
					a2 = a ;
			} else {
				a1 = a2 ; a2 = a ;
				i2 = i ;
			}
		}
	}
	a1 = 0 ;
	a2 = out[n-1] ; i2 = n-1 ;
	for( i = n-1 ; i > -2 ; i--) {
		if(i >= n ) {
			a= out[i] ;
		} else a = 0 ;
		if( a != a2 ) {
			if( ((i2 - i) < len ) && ( a>a2 )) {
					for( ii = i+1 ; ii <= i2 ; ii++) 
						out[ii] = a ;
					a2 = a ;
			} else {
				a1 = a2 ; a2 = a ;
				i2 = i ;
			}
		}
	}
}
void bcGetMag(int *inp,int *out, int n) /* get number of bits in data */
{
	int i,v,r,a ;
	for( i = 0 ; i < n ; i++ ) {
		a = abs(inp[i]) ;
		v = 0 ;
		r = 1 ;
		while( v < a ) { r++ ; v += v + 1; }
		out[i] = r ;
	}
}
void bcIntegrate(int *inp,int *outp,int n) 
{
	int sum ;
	if( n < 1 ) return ;
	sum = 0 ;
	while(n--) {
		sum += *inp++ ;
		*outp++ =  sum ;
	}	
}
int bcCompareI( int *a, int *b, int n) 
{
	int d ;
	int *aa ;
	aa = a ;
	if(n < 1 ) return(0) ;
/* for( d = 0 ; d < n ; d++) printf("%3d %4d %4d\n",d,a[d],b[d]) ;  */
	while(n--) if(d = *a++ - *b++) return(a-aa) ;
	return(0) ;
}
void bcDumpArr( int *d, int n)
{
	int i ;
	for(i=0 ; i < n ; i++ ) {
		fprintf(stderr,"%2d ",d[i]) ;
	}
	fprintf(stderr,"\n") ;
}
int bcCompBit( int *ind, int n, CData *outd ,int nDiff)
{
	int *d1,*d2,*d3,*dp ,dd,nb,ss,j ;
	d1 = ( int *) calloc(3*n,4) ;
	d2 = d1 + n ;
	d3 = d2 + n ;
	dp = ind ;
	for( j = 0 ; j < nDiff ; j++ ) { bcDiff(dp,d1,n) ; dp = d1 ; }
	bcGetMag(dp,d2,n) ;
/*	bcDumpArr(ind+270,25) ; bcDumpArr(d1+270,25) ; bcDumpArr(d2+270,25);*/
	 bcMagFilt(d2,d2,n,20) ;
	bcMagFilt2(d2,d2,n,20)  ; /* this makes little difference */ 
/*	bcDumpArr(d2+270,25) ;*/
	nb = bcComprArr(dp,d2,n,outd->data) ;  
	ss = (CDATAHL + nb+1)/2 ;
	*((short *)outd +ss) = BcMagic - 169 ;
	outd->magic = BcMagic ;		/* put magic at both ends */
	outd->size =  ss+ss+2  ; 
	outd->nData = n ;
	outd->preProc = nDiff ;
	outd->type = 2 ;

	bcDeComprArr(outd->data,d3,n) ;  /* check result */
/*	bcDumpArr(d3+270,25) ; */
	for(j = 0 ; j < nDiff ; j++) bcIntegrate(d3,d3,n) ;
/*	bcDumpArr(d3+270,25) ; */
	dd = bcCompareI(ind,d3,n) ;
	if( dd ) {
		fprintf(stderr,
			" Internal error in compression routine dd=%d\n",dd);
		fflush(stderr) ;
		abort() ;
	}
	free(d1) ;
	return(outd->size) ;
}
int bcCheckCData( CData *data)
{
	int ss ;
	if( data->magic != BcMagic ) return(1) ;
	ss = data->size ;
	if( *((short *)data +(ss/2 -1)) != BcMagic - 169 ) return(2) ; 
	if( data->preProc < 0  ) return(3) ;
	if( data->type    != 2 ) return(4) ;
	return(0) ;  					 /* data ok */
}
int bcDeCompr( CData *data, int *out) 
/* This takes about 0.6 us per data point on ultrasparc 170 */
{
	int n,j ;
	if( bcCheckCData( data ) ) return(0) ;
	n = data->nData ;
	bcDeComprArr(data->data,out,n) ;
	for(j = 0 ; j < data->preProc ; j++) bcIntegrate(out,out,n) ;
	return(n) ;
}
int bcWrite( FILE *fd, CData *d )   /* write CData to stdio file */
{			/* return true if successful  */
	int nw ,nb ;
	char *buff ;
	nb = d->size ;
	if( bcLittleEndian() ) {
		buff = (char *) malloc(nb) ;
		memcpy(buff,d,nb) ;
		bcSwapL((long*)buff) ;
		bcSwapL((long*)buff+1) ;
		swab( buff+8 , buff+8 ,2) ;
		swab( buff+nb-2,buff+nb-2,2) ; 
		nw = fwrite( buff,1,nb,fd) ;
		free(buff) ;
	} else nw = fwrite( d,1,nb,fd) ;
	return( nw == nb ) ;
}
CData *bcRead( FILE *fd) /* read CData from stdio stream, calls malloc */
{
	long nb;
	int nr ;
	CData *d ;
	char *cp ;
	if( 1 !=  fread( &nb,4,1,fd)) return(0) ;
	bcSwapIf( (long *) &nb,1) ;
	d = (CData *) malloc(nb+8) ;
	cp = ( char *)d + 4  ;
	d->size = nb ;
	nr = fread( cp ,1,nb-4,fd) ;
	if( bcLittleEndian() ) {
		bcSwapL( (long *) cp) ;
		swab(cp+4,cp+4,2) ;
		swab(cp+nr-2,cp+nr-2,2) ; 
	}
/*	fprintf(stderr,"nr=%d nb=%d \n",nr,nb ) ;  */
	if( (nr != nb -4 ) || bcCheckCData(d) )  {
		free(d) ;
		return(0) ;
	}
	return(d) ;
}
