#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <evlib.h>
#include "getwdata.h"
#include "silread.h"
#include "detfil.h"
float para[26] ;
time_t tt ;
SilChannel *s,*s1,*s2 ;
int comp ;
int length = 10 ;
int freq = 100 ;
char *sta = "asm" ;
char *wfile ;
char *comps[3] = {"Vertical","N-S","E-W" } ;
/*
double filt[30] = { 0, 0, 1.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0 } ; 
double filt[30] = { 2, 6, 
	0.019, -0.0379250156802743, 0.019,
	4.69012523744564, -9.76596966232709, 
	11.4871267463075, -8.04237110444662, 3.1800029064931, -0.559031363856 } ;
*/
double filt[30] = { 2, 6, 1, -1.99998578778649, 1,
5.84656498772954, -14.2836860523128, 18.6647404045297, -13.7584053883874,
5.42448399978686, -0.893700045634138, 0.0 } ;
/*
*/

void help()
{
	fprintf(stderr,"\n\
bcplot [ options ]\n\
Plois plot of data from getwdata \n\
The data is filtered with a bandpass filter that passed data from\n\
5 to 12 Hz.\n\
Options:\n\
-s	sta	asm	Station.\n\
-l	len	10	Requested data length in seconds\n\
-d	yyyymmdd today	Day\n\
-t	hhmmss	-	Start time\n\
-f	freq	100	Sample frequency\n\
-n			Plot n-s\n\
-e			Plot e-w\n\
-x			Plot only specified data.\n\
-p			Print header information\n\
-w 	file		Write to file as filter\n\
-h			Print this text\n\
");
}
/* void MAIN__() {}  */
void plotBC( SilChannel *s, int comp)
{
	float *x,*y,x0,y0 ;
	FILE *wf ;
	int i,n, *ip ;
	char *ts ; 
	char cap[80] ;
	sprintf(cap,"%s, %d seconds",sta,s->nData/s->freq) ;
	ts = ctime(&(s->sTime) ) ; 
	n = s->nData ;
	x = ( float *) calloc(n,4) ;
	y = ( float *) calloc(n,4) ;
	ip = s->data + comp * n ;
	x0 = s->sTime % 60 ;
	y0 = *ip ;
	if( wfile ) {
		wf = fopen(wfile,"w") ; 
		if(NULL == wf ) bclog(0,"cannot create %s",wfile) ;
		fprintf(wf,"%d\n0\n",n-1) ;
		for( i = 0 ; i < n ; i++) {
			fprintf(wf,"%g\n",*ip++ - y0) ;
		}
		fclose(wf) ; 	
	}
	ip = s->data + comp * n ;
	for( i = 0 ; i < n ; i++) {
		y[i] = gFilt(filt,*ip++ - y0) ;
		x[i] = x0 + ( 1.0 * i ) / freq ; 
	}
	gwShiftRD3(s,y,n) ;	
	splotc(n,x,y,ts,comps[comp],cap,para) ;
	plotclose() ;
}
int main( int ac , char **av )
{
	int cc ;
	int t,d,N,xflag,pflag ;
	extern char *optarg ;
	N = 1 ;
	pflag=0 ;
	xflag = 0 ;
	d = evToday() ;
	while( EOF != (cc = getopt(ac,av,"s:t:d:l:pnxehH?N:f:w:"))) {	
	switch(cc) {
		case 'l' : length = atoi(optarg) ; break ;	
		case 't' : t = atoi(optarg) ; break ;	
		case 'd' : d = atoi(optarg) ; break ;	
		case 'n' : comp=1 ; break ;
		case 'e' : comp=2 ; break ;
		case 's' : sta = optarg ; break ;
		case 'N' : N = atoi(optarg) ; break ;
		case 'p' : pflag = 1 ; break ;
		case 'x' : xflag = 1 ; break ;
		case 'f' : freq = atoi(optarg) ; break ;
		case 'w' : wfile = optarg ; break ;
		default : help()  ; return(0) ;
	} }
	tt = evTime(d,t) ;
	while(N--) 
		s= gwGetWData(sta,tt,length,freq) ;
	if( s == NULL) bclog(0,"No data found "," ")  ;
	if( pflag && s ) sRPrintSilC(s,stdout) ;
	if( s->sTime > tt ) xflag = 0 ;
	if( xflag ) {
		s1 = sRSilCut( s,tt-s->sTime,length) ;
		plotBC(s1,comp) ; 
	} else plotBC(s,comp) ;
	return(0) ;
}
