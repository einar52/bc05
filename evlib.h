#include <sys/types.h>
char **evGetLine( int *nField, FILE *input )  ;
/*	Read one line  from input allocate memory for the line and return
	a arry of string pointest that points to the fields in the input file
*/

/* filter utilities useful for detection */
double evFilt( double *f, double x ) ;
/* apply general filter 
	f contains sizes, coefficients and work data */
double *evInitFilter(  double *fin) ;
/* initialize a new filter using fin */
 
int evGetTremVal( char *sta, time_t tt, int comp , time_t *ta  ) ;
/* Return tremor value. Arguments:
	sta	station
	tt	unixtime
	comp	trem component, 0-9 (see sjatrem -h) or
		10,13,16 which gives max of 0,1,2 or 3,4,5 or 6,7,8
	*ta	actual starttime of the minute returned.
The routine maintains a cache in memory of data at least 5 min before
and after time requested. It is therefore faster to request tremor data
in order of time rather than by stations.
The value returned is 1000*ln(tremval).
*/	
int evTremStaList( char ***staList) ;
/*              Get a list of tremor stations in buffer.
                Space for the list is allocated an put in statlist.
                Statioin names are sorted in alphabetical order.
                Routine returns number of station.
*/


double evDist( double b1, double b2, double ll ) ;
/*	compute distance between two points ( in km)
	with latitudes b1 and b2 and longitude difference of ll 
	input en degress, output in km */

int evMax3(int i1,int i2, int i3) ;
/*	return the largerst of 3 integers */

int evTime(int d, int t)  ;
/*	compte unixtime from day and time where
	d = yyyymmdd and t= hhmmss
        d = yymmdd works also for year 2000 +-50
*/

int evDTime (time_t unixTime ) ;
/* 	compute time (hhmmss) from unixtime */

int evDay( time_t unixTime ) ;
/* 	compute day (yyyymmdd) from unixtime */

int evToday() ;
/* return today on form yyyymmdd */
