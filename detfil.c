/* filter utilities useful for detection */
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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "detfil.h"
#define A0	0
#define A1	1
#define A2	2
#define Y1	3
#define Y2	4
#define A3	5
#define X1	6
#define X2	7
double gFilt( double *f, double x )
/* apply general filter 
	f contains sizes, coefficients and work data */
{
	double *xw,*yw,*a,*b ;
	double y ;
	int n,m,j ;
	n = *f ;
	m = f[1] ;
	a = f+2 ;
	b = a+n+1 ;
	xw = b+m ;
	yw = xw+n ;
	y = *a * x ;
	for( j = 0 ; j < n ; j++ ) y += xw[j]*a[j+1] ;
	for( j = 0 ; j < m ; j++ ) y += yw[j]*b[j] ;
	for( j = n-1 ; j > 0 ; j-- ) xw[j] = xw[j-1] ;
	for( j = m-1 ; j > 0 ; j-- ) yw[j] = yw[j-1] ;
	*xw = x ;
	*yw = y ;
/*	fprintf(stderr,"n=%d m=%d y=%12.4f",n,m,y) ; */
	return(y) ;
}
double *tGFilt( double scale, int size ) 
{
	double *f ;
	f = ( double * ) calloc(size,8) ;
	f[0] = 0 ;
	f[1] = 0 ;
	f[2] = scale ;
	return(f) ;
}
void addZero( double *f , double *zf)
{
        int n,m,j ;
	double *a ;
        n = *f ;
        m = f[1] ;
        a = f+2 ;
	for( j = n+m ; j >= 0 ; j-- ) a[j+2] = a[j] ;
	a[0] = 0.0 ; a[1] = 0.0 ;
	for( j = 0 ; j <= n ; j++)
		a[j] += a[j+2] - *zf*a[j+1]  ;
	a[n+1] -=  *zf * a[n+2] ;
	*f += 2.0 ;
}
void addPole( double *f , double *pf)
{
        double *xw,*yw,*a,*b ;
        double y,p1,p2 ;
        int n,m,j ;
        n = *f ;
        m = f[1] ;
        a = f+2 ;
        b = a+n+1 ;
	p1 = pf[A1] ;
	p2 = -pf[A2] ;
/*	printf("p1=%10.4f p2=%10.4f\n",p1,p2) ; */
	for( j = m ; j > 0 ; j-- ) b[j+1] = b[j-1] ;
	b[0] = 0 ;
	b[1] = -1 ;
	for(j = 0 ; j < m  ; j++ ) 
		b[j] = -p1*b[j+1] - p2*b[j] + b[j+2]; 
	b[m] = -p1*b[m+1] - p2*b[m] ;
	b[m+1] = -p2*b[m+1] ;
	f[1] += 2 ;
}
double zFilt( double *f, double x ) 
/*	apply filter with to zeros on unit circle */
{
	double y ;
	y =  x - f[A0]*f[X1] + f[X2] ;
        f[X2] = f[X1] ;
        f[X1] = x ;
	return(y) ;
}
double * mkZFilt( double dt, double freq )
/*	make filter with a zero pair on unit circle */
{
        double *f ;
        double om,co ;
        f = ( double *) calloc(8,sizeof( double )) ;
        om = freq * 2.0 * M_PI *dt ;
        co = cos( om ) ;
        f[A0] = 2.0*co ;
        return( f ) ;
}
double nFilt( double *f, double x )
	/* apply notch filter, seep mkNFilt  */
{
	double y ;
	y = f[A1]*f[Y1] - f[A2]*f[Y2] + x - f[A0]*f[X1] + f[X2] ;
	f[X2] = f[X1] ;
	f[X1] = x ;
	f[Y2] = f[Y1] ;
	f[Y1] = y ;
	return( y ) ;
}
double * mkNFilt( double dt, double freq, double df) 
	/* make notchfiler, pair of complex conjugate poles
		and zeroes
		returns a pointer to work array of 8 double values
		allocated using malloc */
{
	double *f,eps ;
	double eps1,eps2,om,co ;
	f = ( double *) calloc(8,sizeof( double )) ;
	eps = df*dt ;
	eps1 = 1.0 - eps ;
	eps2 = eps1 * eps1 ;
	om = freq * 2.0 * M_PI *dt ;
	co = cos( om ) ;
	f[A2] = eps2 ;
	f[A1] = 2.0*eps1*co ;
	f[A0] = 2.0*co ;
	return( f ) ;
}
double pFilt( double *f, double x )
/* apply pole filter, f is a pointer from mkPFilt 	*/
{
	double y ;
	y = f[A0]*x + f[A1]*f[Y1] - f[A2]*f[Y2] ;	
	f[Y2] = f[Y1] ;
	f[Y1] = y ;
	return( y ) ;
}
double envelope( double *f )
/* get envelope of function, should be called after pFilt
	f is a pointer from mkpfilt */
{	
	double y,y1 ;
	y =  0.5*( f[Y1] + f[Y2] ) ;
	y1 = ( f[Y1] - f[Y2])*f[A3] ; 
	return( sqrt( y1*y1 + y * y ) ) ;  
}
double * mkPFilt( double dt, double freq, double df) 
{	/* make polefiler, pair of complex conjugate poles
		returns a pointer to work array of 6 double values
		allocated using malloc */
	double *f,eps ;
	double eps1,eps2,om,co,re,im ;
/*	fprintf(stdout,"dt=%10.4f freq=%10.4f df=%10.4f\n",dt,freq,df) ; */
	f = ( double *) calloc(6,sizeof( double )) ;
	eps = df*dt ;
	eps1 = 1.0 - eps ;
	eps2 = eps1 * eps1 ;
	om = freq * 2.0 * M_PI *dt ;
	co = cos( om ) ;
	f[A0] = eps ;
	f[A0] *= 2.0 ;
	re = 1.0 - 2.0*eps1*co*co + eps2*cos(2.0*om) ;
	im = eps*eps1*sin(2.0*om) ;
	f[A0] = sqrt(re*re + im*im) ;
	f[A2] = eps2 ;
	f[A1] = 2.0*eps1*co ;
	f[A3] = 1.0 / om ;
	return( f ) ;
}
double *readFilter( char *fname ) 
{
	FILE *ff ;
	double *fil ;
	int n,m,i,j ;
	ff = fopen( fname,"r") ;
	fscanf(ff,"%d %d",&n,&m ) ;
	fil = calloc( (n+m)*2+8,8) ;
	fil[0] = n ;
	fil[1] = m ;
	for( i = 0 ; i <= n+m ; i++) {
		j = fscanf(ff,"%lf",fil+2+i) ;
	}
	if( j == 1 ) return(fil) ;
	else return(NULL) ;
}
double *initFilter( double *fin )
/* initialize a new filter using fin */
{
	double *fout ;
	int n,m,j ;
	n = fin[0] ; m = fin[1] ;
	fout = calloc( (n+m)*2+8,8) ;
	for( j = 0 ; j < n+m+3 ; j++ ) fout[j] = fin[j] ;
	return fout ;
}
void saveFilter( double *filter, char *name )
{
	FILE *ff ;
	int n,i ;
	ff = fopen(name,"w") ;
	n = *filter + filter[1] + 3 ;
	for( i = 0 ; i < n ; i++ ) fprintf(ff,"%1.15g\n",filter[i]) ;
}
