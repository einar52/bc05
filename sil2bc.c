
/*
© Copyright 1998, 2000, 2020 Einar Kjartansson

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
This is program to move data from /home/ved or /home/jar to
/sil/bc/...  and /eq1/...

History :
Jul 1998 Original version.	-eik
Jan 2000 Added -s flag to look for data in path ./mmm/dd/sss   -eik



Input parameters
-n m	500	Maximum number of waveforms
-p path	/home/ved	Path to search for data
-d day  0	day reletive to today (0 is today, 1 is yesterday etc.)
-l logLevel 3	Log events of priority level and lower to stdout
-s 		look in path/mmm/dd/sss instead of  path/sss/mmm/dd 

The program does the following.

Test for available space and inodes on /eq0 and /eq1

Get a list of files (*Z??.Z)

for each file:
	Check if all components are there and ok.
	copy files to /eq0
	convert files bc under /sil/bc
	convert bc file to ah on /eq1
update cat.acc on /eq1

If all above is completed: remove files from source path.

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "silread.h"
#include "util.h"
#include "bitcomprl.h"
#define LineLen 30 ;
int m = 500 ;
int n ;
char *path = "/home/ved" ;
char year[10], days[20] ;
int day ;
int logLevel = 3 ;
int iLine ;
char **lines ;
int *status ;
char *bcBase = "/sil/bc" ;
int solPath ;

void bclog( int level , char *s1 , void *s2 )
{
	if( logLevel < level  ) return ;
	fprintf(stdout,"sil2bc %d: ",level ) ;
	fprintf(stdout,s1,s2) ;
	fprintf(stdout,"\n")  ;
	fflush(stdout) ;
	if( level < -1 ) abort() ;
	if( level <  0 ) exit(0) ;
}
char *bcPath( time_t t, char *station , int freq ) 
{
	static char buff[200] ;
	char b1[40],b2[40] ;
	cftime(b1,"%Y/%b/%d",&t) ;
	b1[5] ^= 32 ;
	cftime(b2,"%H%M%S", &t) ;
	sprintf(buff,"%s/%s/%s/%s%02d.bc",bcBase,b1,station,b2,freq%100) ;
	bclog(5,buff,0) ;
	return(buff ) ;
}
void checkSpace(char *fs, int mb, int mfiles)
{
/*  quit if there are less than mb Mb or mfiles inodes free on file system */
	int j,space,files ;
	struct statvfs buf ;
	j = statvfs(fs,&buf ) ;
	bclog(5,"File system %s",fs) ;
	bclog(9,"statvfs returns %d", ( void *) j ) ;
	space = buf.f_bavail*(buf.f_frsize/1024) ;
	files = buf.f_ffree ;
	bclog(5,"%d blocks available ",( void *) space) ;
	bclog(5,"%d files free",( void *) files ) ;
	if( space < mb ) bclog(-1,"Insufficient space on %s",fs) ;
	if( files < mfiles ) bclog(-1,"Insufficient inode space on %s",fs) ;

}
void setDay()
{
	static char buff[200] ;
	int j ;
	time_t t ;
	time(&t) ;
	t -= 24*3600*day ;
	j = cftime(year,"%Y",&t) ;
	bclog(6,"Year = %s",year) ;
	j = cftime(days,"%b/%d",&t) ;
	*days ^= 32 ;
	bclog(6,"Days = %s",days) ;
}
void getList()
{
	char cmd[400]  ;
	int j ;
	char *cp ;
	FILE *pd ;
	lines = ( char **) calloc( m , sizeof( char *)) ;
	cp = (char *) calloc(m,32) ;
	status = ( int *) calloc(m,4) ;
	for( j = 0 ; j < m ; j++ ) lines[j] = cp + j*32 ;
	if( solPath)
	   sprintf(cmd,"(cd %s ; find %s -name  \"*Z??.Z\" 2>/dev/null)",
		path,days) ;
	else  sprintf(cmd, 
	     "(cd %s ; for i in ??? ; do find $i/%s -name \"*Z??.Z\" ; done \
                2>/dev/null)", path, days) ;
	bclog(6,"%s",cmd) ;
	pd = popen(cmd,"r") ;
	j = 0 ;
	while( fgets(lines[j],26,pd) ) {
		lines[j][24]=0 ;
		bclog(7,lines[j],"_") ;
		j++ ;
		if( j >= m ) break ;
	}
	n = j ;
	bclog(3,"found %d waveforms",( void *) n) ;
	pclose(pd) ;
/*	for( j = 0 ; j < n ; j++ ) printf("%3d %s\n",j,lines[j]) ;  */
	
}
SilChannel *getSilChannel( char *name, char comp ) 
{
	int l ;
	SilChannel *s ;
	char pipe[200] ;
	FILE *pd ;
	sprintf(pipe,"/usr/bin/uncompress < %s ",name) ;
	l = strlen(pipe) ;
	pipe[l-6]=comp ;
	pd = popen(pipe,"r") ;
	s = sRReadSil(pd) ;
	pclose(pd) ;
	if( ( NULL == s ) || (comp != *(s->component)) ) {
		bclog(1,"problem reading %s",pipe) ;
		free(s) ;
		return(NULL) ;
	}
	return(s) ;
}
void doItem( char **line, int  *stat )
{
	CData d ;
	SilChannel *sz,*sn,*se,outh ;
	FILE *bfd ;
	int j ;
	char *bcn,name[200] ;
	bclog(4,*line,0) ;
	sprintf(name,"%s/%s",path,*line) ;
	bclog(4,name,0) ;
	sz = getSilChannel(name,'Z') ;
	sn = getSilChannel(name,'N') ;
	se = getSilChannel(name,'E') ;
	*stat = ( sz && sn && se ) ;
	if( *stat == 0 ) return ;
	bclog(5,"status = %d",(void *) *stat) ;
	bcn = bcPath( sz->sTime,sz->station,sz->freq) ;
	bfd = utPathOpen(bcn) ;
	if( NULL == bfd ) bclog(-1,"cannot write %s",bcn) ;
	bclog(5,"bfd = %d",(void *)bfd) ;
	outh = *sz ;
	j = BCMAGIC ;
	outh.data = (int*)j ;
	*(outh.component) = '3' ;
	bcSwapIf( (long*)&outh,4) ;
	if( bcLittleEndian()) swab( (char*)&(sz->freq),(char*)&(outh.freq),6) ;
	j = fwrite(&outh, N_BCS_HEAD,1,bfd) ;
	j = fwrite(sz->ascHead,sz->ascHeadSize,1,bfd) ;
	bcCompBit(sz->data,sz->nData,&d,1) ;
	bcWrite(bfd,&d) ;
	bcCompBit(sn->data,sn->nData,&d,1) ;
	bcWrite(bfd,&d) ;
	bcCompBit(se->data,se->nData,&d,1) ;
	bcWrite(bfd,&d) ;
	fclose(bfd) ;
	free(se) ; free(sn) ; free(sz) ;
	if( (iLine % 100) == 0 ) bclog(3,"%s",bcn) ;
}
void doList()
{
	for( iLine = 0 ; iLine < n ; iLine++ ) {
		doItem( lines+iLine, status+iLine ) ;
	}
}
void doit()
{
	checkSpace("/eq0",40000,10000) ;
	checkSpace("/eq1",40000,10000) ;
	bclog(3,"Space ok","") ;
	setDay() ;
	getList() ;
	doList() ;
}
int main(int ac , char **av )
{
	int cc ;
	extern char *optarg ;
	while( EOF != (cc = getopt(ac,av,"l:d:p:m:s"))) {
	switch(cc) {
		case 'l': logLevel = atoi(optarg) ; break ;
		case 'd' : day = atoi(optarg) ; break ;
		case 'p' : path = optarg ; break ;
		case 'm' : m = atoi(optarg) ; break ;
		case 's' : solPath = 1 ; break ;
	} }
	doit() ;
	return(0) ;
}
