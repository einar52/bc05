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
#include <sys/types.h>
#include <time.h>

int date2d(y,m,d)	/* compute days after dec 31 1959 */
int y,m,d ;
{
	int dd ;
	static dym[13] = { 0,31,59,90,120,151,181,212,243,273,304,334,365};
	if( y > 1000 ) y -= 1900 ;
	if( y < 60 ) y += 100 ;
	y -= 60 ;
	dd = 365*y + (y+3)/4 +dym [m-1] + d ;
	if (((y & 3) == 0 ) && (m > 2)) dd++ ;
/*	printf("dd=%d %d/%d/%d\n",dd,y,m,d) ;   */
	return dd ;
}
d2date(dd,yp,mp,dp)	/* compute date from days after dec 31 1959 */
int dd,*yp,*mp,*dp ;
{
	int dt,mt,m,y,d ;
	y = (4*dd-4)/1461 ;
	dt = dd - 365*y - (y+3)/4 ;
	y += 60 ;
	m = 1 + dt/30 ;
	if( m > 12 ) m = 12 ;
	d = dd + 1 - date2d(y,m,1)  ;
	while( d < 1 ) {
		m-- ;
		d = dd + 1 - date2d(y,m,1)  ;
	}
	*dp = d ;
	*mp = m ;
	*yp = y ;
}
#ifdef sun
time_t xmktime ( struct tm *tp )
#else
time_t mktime ( struct tm *tp )
#endif
{
	int d,t ;
	d = date2d( tp->tm_year, tp->tm_mon+1 , tp->tm_mday-1) ;
/*	printf("d=%d\n",d) ; */
	t = (((d-3653)*24 + tp->tm_hour)*60 + tp->tm_min)*60 + tp->tm_sec ;
	return(t) ;
}
#ifdef sun
int main(int ac, char **av)
{
	struct tm ts ;
	time_t  j ,jj ;
	ts.tm_year = atoi(av[1]) ;
	ts.tm_mon = atoi(av[2]) ;
	ts.tm_mday = atoi(av[3]) ;
	ts.tm_hour = atoi(av[4]) ;
	ts.tm_min = atoi(av[5]) ;
	ts.tm_sec = atoi(av[6]) ;
	j = xmktime( &ts ) ;
	jj = mktime( &ts ) ;
	printf("%s\n",ctime(&j)) ;
	printf("%s\n",ctime(&jj)) ;
}
#endif
