/*
This file contains code to shift data 68.75 ms, to correct for delay
caused by the digital anti-alias filter in the Nanometrics RD3 digitizer.

			Einar Kjartansson, aug 28 1999.

Revisions:	
jan 17. 2000 -eik:
	Use last data value instead of zero past end of data.

*/

/*	this was used to compute coefficients: 
double f1( double x) 
{ return(0.5*x+2.0*x*x-1.5*x*x*x) ; } 
double f2( double x) 
{ return( 0.5*x*(x*x-x)) ; }
main()
{
	double x ;
	x = 0.125 ;
	printf("%15.16f\n",f2(x)) ;
	printf("%15.16f\n",f1(x)) ;
	printf("%15.16f\n",f1(1-x)) ;
	printf("%15.16f\n",f2(1-x)) ;
}
*/
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include "silread.h"
/* coefficients for 4 point interpolator */


#define C1 -0.0068359375 
#define C2  0.0908203125 
#define C3  0.9638671875 
#define C4 -0.0478515625

/*
#define C1 0.00 
#define C2 0.125
#define C3 0.875
#define C4 0.0

#define C1 0.00 
#define C2 0.0
#define C3 1.0
#define C4 0.0
*/

void shShift55_8( float *data,  int n)
/*	the contents of array data are shifted 6.875 samples back in time
	0 values are shifted in at the end.
*/
{
	float y1,y2,y3,y4 ;
	int i,j ;
	y1 = data[5] ;
	y2 = data[6] ;
	y3 = data[7] ;
	for( i = 0 ; i < n ; i++) {
		j = i+8 ;
		if( j < n ) y4 = data[j] ; 
/*		else y4 = 0.0 ;                - eik jan 17.  2000 */
		else y4 = data[n-1] ;
		data[i] = C1*y1 + C2*y2 + C3*y3 + C4*y4 ;
		y1 = y2 ; 
		y2 = y3 ;
		y3 = y4 ;
	}
}
#include <math.h>
#ifdef TEST
main()
{
	static float x[100],y[100],para[30] ;
	int i,n ;
	static float m = 20/6.875 ;
	n = 40 ;
	for( i = 0 ; i < n ; i++ ) {
		x[i] = i ;
		y[i] = sin(M_PI*2.0*i*m/n) ;
	}
	splotc(n,x,y," "," "," ",para);
	shShift55_8(y,n) ,
	splotc(n,x,y," "," "," ",para);
	plotclose() ;
}
#endif
