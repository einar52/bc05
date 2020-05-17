/* bctool - program to perform various operations on seismograms storid using BC */
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "silread.h"
SilChannel *sC ;

int logLevel = 3 ;

void btHelp()
{
	fprintf(stderr,"\
bctool [ options ]\n\
Perform various operations on bc compressed waveform data.\n\
Options include:\n\
-l	loglevel  Set maximum level of debug messages, default is 3\n\
-z	file	read 3 channels of compressed sil-format data.\n\
-s	file	read 3 channels of sil-format data\n\
-S		write 3 channels of silformat data in current directory\n\
-b	file	read bc compressed waveform data\n\
-B	[file]	write bc compressed wavform data to file.\n\
-r	sta day time len \n\
		read bcdata from BC_BASE tree.\n\
-p		print header information from waveform data  in memory\n\
-i	yyyymmdd	Make index for one day\n\
-d	yyyymmdd	read all data for one day from /eq0 to BC_BASE\n\
-m	file	Move bc file to BC_BASE tree and update index\n\
-M	base day  Move all bc files for base and today-day and update index\n\
			for path of form sss/mmm/dd/\n\
-X	base day  Move all bc files for base and today-day and update index\n\
			for path of form mmm/dd/sss\n\
-c	path	quit if there are less than 30Mb or 10000 inodes available on path\n\
-g	[ day ]	Print information about gap in bc data coverage. Optional parameter\n\
		day can either be number of days before today, or date on the\n\
		form yyyymmdd.\n\
") ;
}
int main( int ac , char **av ) 
{
	int cc ;
	extern char *optarg ;
	extern int optind ;
	int day,len,tt ;
	time_t tim ;
	while( EOF != (cc = getopt(ac,av,"hH?z:s:Sb:Br:pl:i:d:m:c:M:X:g"))) {
	switch(cc) {
		case 'l' : logLevel = atoi(optarg) ; break ;
		case 'z': sC = sRGetSil3Channel(optarg,1)  ; break ;
		case 's': sC = sRGetSil3Channel(optarg,0)  ; break ;
		case 'p' : sRPrintSilC( sC,stdout ) ; break ;
		case 'S' : sRPutSil(sC) ; break ;
		case 'b' : sC = sRGetBc(optarg) ; break ;
		case 'B' : 
			if((optind >= ac ) ||(*av[optind] == '-' ) ) {
				sRPutBC(sC) ;
			} else {
				sRPutBCFile(sC,av[optind++]) ;
			}
			break ;
		case 'r' : day = atoi(av[optind++]) ;
				tim = atoi(av[optind++]) ;
				len  = atoi(av[optind++]) ;
				tt = evTime(day,time) ;
				sC = gwGetWData(optarg,tt,len,100) ;
				break ;
		case 'i' : sRMkIndex(atoi(optarg)) ; break ;
		case 'd' : sRSil2BcDay(atoi(optarg)) ; break ;
		case 'm' : sRMoveBc(optarg) ; break ;
		case 'M' : sRMoveList( optarg, atoi(av[optind++]),0 ) ; break ;
		case 'X' : sRMoveList( optarg, atoi(av[optind++]),1 ) ; break ;
		case 'c' : sRCheckSpace(optarg,30000,10000) ; break ;
		case 'g' :  	tim = time(&tim) ;
				if( (optind < ac ) && ( '-' != *av[optind] )) {
					tt = atoi( av[optind++] ) ;
					if( tt < 19700000 ) tim -= tt*24*3600 ;
					else tim = evTime(tt,10) ;
				}
				gwGaps( tim ) ; break ;
			
		default : btHelp() ; break ;
	} }
	return(0) ;
}
