
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

/*
	Micellanious  utility routines
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "util.h" 
void utLCase( char *s ) 
{
	while( *s ) {
		if (*s & 64 ) *s |= 32 ;
		s++ ;
	}
}
int utAhPath( char *buf, char *station, time_t *t ) 
{
	time_t tt ;
	struct tm *tp ;
	char s1[200],s2[200] ;
	tp = gmtime(t) ;
	tp->tm_min  -= tp->tm_min % 5 ;
	tt = mktime(tp ) ;
	cftime( s1,"%Y/%b/%d/%H:%M:00",&tt) ;
	utLCase(s1) ;
	cftime( s2,"%H:%M:%S",t ) ;
	sprintf(buf,"%s/%s.00%s",s1,s2,station) ;
}
/*		    1  2  3  4  5   6   7   8   9  10  11  12   */
int utilDays[12] = {0,31,59,90,120,151,181,212,243,273,304,334 } ;
int date2doy( int month, int day,int leap )
{
	int dd ;
	dd = utilDays[month-1]+day-1 ;
	if( leap && month > 2 ) dd++ ;
	return ( dd ) ;
}
int doy2date( int doy, int leap , int *month )
{
	int m,dd,d ;
	m = 11 ;
	dd = doy ;
	if( leap && ( dd > 58)) dd-- ;
	while( dd < utilDays[m] ) m-- ;
	dd = doy ;
	if( leap && ( m > 1) ) dd = doy - 1 ;
	d = dd - utilDays[m] + 1 ;
	*month = m + 1 ;
	return ( d )  ;
}
int utMkPath( char *path, int mode )
{
/*	return 0 if directory exists or was created, -1 otherwise */
	int j,jj ;
	char *cp,*sp ;
/*	printf("path=%s\n",path) ;    */
	j = mkdir(path,mode) ;	
	if( j == 0 ) return(0) ;
	if( errno == EEXIST ) return(0) ; 
	cp = path ;
	sp = NULL ;
	while( *cp ) { if ( *cp == '/' ) sp = cp ; cp++; }
	if( sp ) {
		*sp = 0 ;
		jj=utMkPath(path,mode) ;
		*sp = '/' ;
		if( jj == 0 ) return(mkdir(path,mode)) ;
		else return(-1) ;
	}
	return(-1) ;
}
FILE *utPathOpen( char *name ) 
	/*open file for writing, createdirs as needed*/
{
	char s[200] ; 
	char *p,*sp ;
	int r ;
	strcpy(s,name) ;
	p = s ;
	while( *p ) { if( *p == '/' ) sp = p ; p++ ; } 
	*sp++ = 0 ;
	r = utMkPath(s,0775 ) ;
	if( -1 == r ) return(0 )  ;
	return( fopen(name,"w")) ;
	
/*
	int nslash ;
	char *cp,*sp ;
	nslash = 0 ;
	cp = name ;
	while( *cp ) {if(*cp == '/') {nslash++; sp=cp ;} ; cp++ ; }
*/
	
}

void get_vika(struct tm *tp, int *ar, int *vika ) 
/* get number of veek, and its year, for date in tp, 
not, week is from monday (1) to sunday (7) and week belongs to
the year that the thursday belongs to 
Note:	the tp structure is alterded,on return it points to the thursday*/
{
	int t ;
	t = mktime(tp) ;
	tp->tm_mday += 4 - tp->tm_wday ;
	t = mktime( tp ) ;
/*	printf("year=%d mon=%d mday=%d wday=%d yday=%d\n",
		tp->tm_year,tp->tm_mon,tp->tm_mday,tp->tm_wday,tp->tm_yday); 
*/
	*ar = tp->tm_year ;
	*vika = (tp->tm_yday + 7) / 7 ;
}
int vika2tm( struct tm *tp, int ar, int vika, int vdagur) 
/* reverse of get_vika */
{
	int a,v,x,j ;
	memset( tp,0,sizeof(*tp)) ;
	tp->tm_year = ar ;
	x = 7*vika-3  ;
	tp->tm_mon = x / 31 ;
	tp->tm_mday = x % 31 ;
	get_vika( tp,&a,&v ) ;
	if( a < ar ) {tp->tm_mday += 14 ; get_vika( tp,&a,&v ) ; }
	else if( a > ar ) {tp->tm_mday -= 14 ; get_vika( tp,&a,&v ) ; }
	for(j = 0 ; j < 2 ; j++) {
		if( v < vika) { tp->tm_mday += 7 ; get_vika(tp,&a,&v) ; }
		if( v > vika) { tp->tm_mday -= 7 ; get_vika(tp,&a,&v) ; }
	}
	tp->tm_mday += vdagur - (1+ (tp->tm_wday+6)%7) ;
	j = mktime(tp) ;
	return( (vika == v) && ( vdagur == (1+ (tp->tm_wday+6)%7))) ;
}
/* 
int test2( int ac , char **av)
{
	int a,m,d,ar,vika ;
	static struct tm tt,tx ;
	a = atoi(av[1]) ;	
	m = atoi(av[2]) ;	
	d = atoi(av[3]) ;	
	tt.tm_year = a ;
	tt.tm_mon = m-1 ;
	tt.tm_mday = d ;
	get_vika(&tt,&ar,&vika) ;
	printf("ar=%d vika=%d\n",ar,vika) ;
	if (vika2tm(&tx,ar,vika,2 ) )
	printf("year=%d mon=%d mday=%d wday=%d yday=%d\n",
		tx.tm_year,tx.tm_mon,tx.tm_mday,tx.tm_wday,tx.tm_yday); 
	else printf("ólögleg vika\n") ;
	return(0) ;
}
int test1( int ac , char **av)
{
	int a,v,d,ar,vika ;
	static struct tm tt,tx ;
	a = atoi(av[1]) ;
	v = atoi(av[2]) ;
	d = atoi(av[3]) ; 	
	if( vika2tm(&tx,a,v,d ) )
		printf("year=%d mon=%d mday=%d wday=%d yday=%d\n",
		tx.tm_year,tx.tm_mon,tx.tm_mday,tx.tm_wday,tx.tm_yday); 
	else printf("ólögleg vika\n") ;
	return(0) ;
}
int main(int ac, char **av) 
{
	test1(ac,av) ;
}
*/
/*
main(int ac , char **av ) 
{
	int j ;
	j = utMkPath( av[1], 0775 ) ;
	printf("j=%d errno=%d\n",j,errno) ;
	return(0) ;
}
*/
