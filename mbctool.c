
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "evlib.h"
#include "bitcomprl.h"
#define MAXL 90000
#define NCOL 6

int data[MAXL*NCOL] ;
char **cp, **head ;
char *sta ;
int n ;
time_t tt ;
int logLevel=2 ;
void help()
{
	fprintf(stderr,"\
mbctool c	< mtfile > compressed_mtfile \n\
mbctool u	< compressed_mtfile > mtfile \n\
") ;
exit(0) ;
}
void bclog( int level , char *s1 , void *s2 )
{
        if( logLevel >= level  ) {
                fprintf(stderr,"mbctool %d: ",level ) ;
                fprintf(stderr,s1,s2) ;
                fprintf(stderr,"\n")  ;
                fflush(stderr) ;
        }
        if( level <= -1 ) abort() ;
        if( level <=  0 ) exit(1) ;
}

void readMt()
{
	int i,j,*f,nf ;
	head = evGetLine( &nf,stdin) ;
	sta = head[1] ;
	cp = evGetLine( &nf,stdin) ;
	tt = evTime( atoi(head[2])*10000+atoi(head[3])*100+atoi(head[4]),
			atoi(head[5])*10000) ;
	bclog(4,"%s",ctime(&tt)) ;
	for ( n = 0 ; n < MAXL ; n++ ) {
		free(cp) ;
		cp = evGetLine( &nf,stdin) ;
		while( nf != NCOL ) {
			if(nf <= 0 ) {
				bclog(4,"Read %d lines", ( void *) n) ;
				return ;
			}
			free(cp) ;
			cp = evGetLine( &nf,stdin) ;
			
		}
		for( j = 0 ; j < NCOL ; j++) {
			data[j*MAXL + n] = atoi(cp[j+1]) ;
		}
	}
}
writeMBc()
{
	int j ;
	CData d ;
	fwrite(sta,4,1,stdout) ;
	fwrite(&tt,4,1,stdout) ;
	for(j = 0 ; j < NCOL ; j++) {
		fflush(stdout) ;
		bclog(4,"j=%d",(void *)j) ;
		bcCompBit(data+j*MAXL,n,&d,1) ;
		bcWrite(stdout,&d) ;	
	}
}
void readMBc()
{
	static char s[4] ;
	CData *dp ;
	int j ;
	sta = s ;
	fread(sta,4,1,stdin) ;
	bclog(4,"station %s",sta) ;
	fread(&tt,4,1,stdin) ;
	bclog(4,"%s",ctime(&tt)) ;
	for(j = 0 ; j < NCOL ; j++ ) {
		dp = bcRead(stdin) ;
		n = bcDeCompr(dp,data+j*MAXL) ;
		bclog(4,"j=%d",(void *)j) ;
	}
}
void writeMt()
{
	int i,j ;
	char buff[100] ;
	cftime(buff,"%Y %m %d %H",&tt) ;
	fprintf(stdout,"%s %s\n+data+\n",sta,buff ) ;
	for( i = 0 ; i < n ; i++ ) {
		for( j = 0 ; j < NCOL-1 ; j++) {
			fprintf(stdout,"%7d.0",data[i+MAXL*j]) ;
		}
		fprintf(stdout," %06d\n",data[i+MAXL*NCOL-MAXL] ) ;
	}
}
main(int ac , char **av)
{
	if( ac < 2 ) help() ;
/*bclog(0,"hmbctool c|u < input > output "," ") ; */
	if( *av[1] == 'c' ) {
		readMt() ;
		writeMBc() ;
	} else {
		readMBc() ;
		writeMt() ;
	}
	return(0) ;
}
