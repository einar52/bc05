
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

#include <sys/types.h>
#include <time.h>
int evTime(int d, int t) 
/*	compte unixtime from day and time where
	d = yyyymmdd and t= hhmmss
	d = yymmdd works also for year 2000 +-50
*/
{
	time_t tt ;
	struct tm tp ;
	if( d < 19000000 ) {
		d += 19000000 ;
		if( d < 19500000 ) 
			d+=1000000 ;
	}
	tp.tm_year = ( d / 10000 ) - 1900 ;
	tp.tm_mon  = (( d /100 ) % 100 ) -1 ;
	tp.tm_mday =  d % 100 ;
	tp.tm_hour = t / 10000 ;
	tp.tm_min = (t / 100) % 100  ;
	tp.tm_sec = t % 100 ;
	tt = mktime(&tp ) ;
	return(tt) ;
}
int evDTime (time_t unixTime )
/* 	compute time (hhmmss) from unixtime */
{
	struct tm *ts ;
	int d ;
	ts = localtime( &unixTime ) ;
	d = ts->tm_hour *10000 + ts->tm_min*100 + ts->tm_sec ;
	return(d) ;
}
int evDay( time_t unixTime )
/* 	compute day (yyyymmdd) from unixtime */
{
	struct tm *ts ;
	int d ;
	ts = localtime( &unixTime ) ;
	d = (1900+ts->tm_year) *10000 + (ts->tm_mon+1)*100 + ts->tm_mday ;
	return(d) ;
}
int evToday()
/* return today on form yyyymmdd */
{
	time_t tt ;
	time( &tt ) ;
	return(evDay(tt)) ;
}
/*
main (int ac, char **av )
{
	int x,d,t,y;
	time_t u ;
	d=atoi(av[1]) ;
	t=atoi(av[2]) ;
	u=evTime(d,t) ;
	printf("%d %d %d %s",d,t,u,ctime(&u)) ;
	x=evToday() ;
	y = evDTime(u) ;
	printf("x=%d y=%06d\n",x,y) ;
}
*/	
