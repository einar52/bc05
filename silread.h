
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
#define BCMAGIC 7*19*23*113*511+8

typedef struct {
	int  *data ;	/* pointer to waveform data, nData or
			   3*nData, depending on value of *component */
			/* in disk files this contains BCMAGIC   */
	time_t sTime ;		/* unix time for start of data, whole sec */
	int nData ;	/* number of samples, shold be a multiple of freq */
	time_t cTime ;	/* time at creation of file */
	char station[4] ;	/* name of station, 3 chars */
	char component[2] ; 	/* z,n,e or 3 (3 components in order) */
	short freq ;		/* sample rate  (Hz) */
	short ascHeadSize ;	/* Number of bytes in ascii header (multiple of 4) */
	short ascHeadN ;	/* Number of lines in ascii header  -- */
/* 
	Datafiles with .bc  suffix start with a header 
	that is	a big-endian version of above.

	The following is not written to disk.
*/

	char *ascHead ;	/* pointer to ascii header data */
} SilChannel ;

#define N_BCS_HEAD sizeof(SilChannel)-4 

typedef struct {
	char station[4] ;
	time_t sTime ;
	int nSamp, freq ;
}  BcIndex ;


/* The functions that return pointer to SilChannel allacate memory 
that syould be freed after use */

time_t sRPath2utime( char *s, char *year)  ;
SilChannel *sRReadSil( FILE *fd ) ;  
	/*read one channel of sil data into
	structure which is allocated and must be free'ed after use */
void sRPrintSilC( SilChannel *sC ,FILE *fd) ;
	/* print out head information from SilChannel structure */
SilChannel *sRGetSilChannel( char *name, char comp, int compressed ) ;
/*      read one channel of sildata */
SilChannel *sRGetSil3Channel( char *name, int compressed ) ;
/* read 3 channels of sil data */

int sRPutBCFile(  SilChannel  *sd , char *fname) ;
/* write bc data to file fname */

int sRPutBC( SilChannel *sd ) ;
/*  write bc data do default file name */

SilChannel *sRGetBc( char *fname )  ;
/* read bc data from disk file */

SilChannel *sRGetBcHeader( char *fname ) ;
/* read bc header from disk file  */

void sRPutSilChannel ( SilChannel *sC , FILE *fd , int comp ) ;
/* write sildata to disk */

void sRPutSil( SilChannel *sC ) ;
/* write sil data to standard filename */

SilChannel *sRSilCut( SilChannel *s, int skip, int length)  ;
/* cut part of data */


SilChannel *sRSilCopy( SilChannel *s) ;
/* allocate and space and copy data to it */

SilChannel *sRSilCat( SilChannel *sc1, SilChannel *sc2 ) ;
/*
        Check is s1 and s2 touch or overlap, if so they
        are merged and the result returned. Otherwise NULL is
        returned. Space for result is allocated, sc1 and sc2 are
        not changed.
*/


void sRPutIndex(FILE *ifd, SilChannel *sd) ;

void sRMkIndex( int date) ;

void sRSil2BcDay( int date ) ;
/*
Read all files on /eq0 for a given day, convert to bc and write to BC_BASE
*/
void sRMoveBc( char *file ) ;
/*
	Move file to BC_BASE tree and update index.
*/
void sRCheckSpace(char *fs, int kb, int mfiles) ;
/*  quit if there are less than kb kb or mfiles inodes free on file system */

void sRSetDay( int day, char *days ) ;

void sRMoveList(char *base, int day , int silFlag ) ;
/* call sRMoveBc for all files found for today-day and path base */

void shShift55_8( float *data,  int n) ;
/*      the contents of array data are shifted 6.875 samples back in time
        0 values are shifted in at the end.
*/
void gwShiftRD3( SilChannel *sd , float *data, int n) ;
/*
        Apply 68.75 ms time shift to data in SilChannel if second word in 
        ascii header of sd is "RD3"           - Einar Kjartansson, aug 1999.
	If environment variable BC_DIGITIZER is set, that is used instead.
*/


SilChannel *gwGetWData( char *station, time_t sTime, int nSec, int sFreq ) ;
/*
	This function is used to look for waveform data for a specified
	time (sTime) and station. 
	nSec is ignored by the present version but will probably be used
	in the future.
	Two variables from environment are also used by gwGetWData:
	BC_BASE	is the base directory for storage of waveform data,
		default is /sil/bc
	BC_DEBUG  controls printout of diagnostig messages. Default is 3
		BC_DEBUG should be between 1 and 9.

	gwGetWData returns a pointer to a SilChannel structure that
	contains data that includes sTime. Currently all data from
	one file (save) is returned but in the future data from more
	than one file will be combined as needed.
	If data is not found, a null pointer is returned.

 */
time_t unixtime_(int *year, int *month, int *day, 
                int *hour, int *min, int *sec ) ;
/*      Fortran interface for mktime(3C) */

int getwdata_( char *station, int *sTime, int *channel, int *nSec, int *sFreq,
        float *data, int *nRetSamp,
        char *sensor, char *digitizer, char *timeSync, 
        int lsta, int lsens, int ldig, int lsync ) ;

/*
This a FORTRAN interface to gwGetWData. Following is an example
of fortran usage:

        implicit none
        character*20 sta,sens,dig,sync
        real data(20000)
        real x(20000),para(22)
        integer n,stime,nsec ,i,getwdata,rtime,freq,comp
        sta = 'asm'
        stime=906477810
        freq = 100 
        comp = 2 
        nsec = 15
        n = getwdata(sta,stime,comp,nsec,freq,data,rtime,sens,dig,sync)

Channel is 0 for Z, 1 for N-Z and 2 for E-W
If data is found, the number samples placed in data is returned.
If not data is found,  zero is returned.
rtime is unixtime for start of data returned.
On return send and dig contain fields from  4th line from the sil 
format file where names
of sensor and digitizer are placed, example: LE1 RD3
The 6th line is placed  sync, example: 171 399 -4 0 0

*/




