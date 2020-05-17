/*	
	The following routines are added to make it possible to use
	the bc files without risking being thrown out when trying to
	access data that is not available. (Note! Low level routines
	should never abort processes but rather return failure!).
	Utilities are also added to make it easy to browse through a
	bc data set. These routines should also ease the implementation
	of concatenate bc-files while reading.

	The routine ThereIsIndex() returns 1 if an index file is available
	for the given time else it returns 0. This routine is mainly for
	internal use and should not be needed at higher levels. 

	The routine ThereIsData() returns 1 if all data is available for 
	the specified time else 0 if non or only part of the data is available;
	It also gives information about the availability of data prior and
	after this time. The last routine is an Fortran interface to the
	routine ThereIsData.

	981231 Reynir Bodvarsson
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "silread.h"
#include "getwdata.h"

extern BcIndex gwIndex[];
extern int gwOldDay, gwNIndex  ;
extern char gwDirName[];
extern int gwLogLevel;
extern int gwFileInMem;

/* routine from the bc library but added the global K */
static int K;

void rbPrintInd()
{
    int i,n ;
    BcIndex *p ;
    n = gwNIndex ;
    for( i = 0 ; i < n ; i++ ) {
        p = gwIndex + i ;
        printf("% 4d %s % 10d % 6d % 4d %s",i,
			p->station,p->sTime,p->nSamp,p->freq,ctime(&(p->sTime)) ) ;
    }
}

int rbBinSearch( BcIndex *p ) 
{
	int i,j,k,r ;
	BcIndex *pp ;
	i = 0 ; j = gwNIndex - 1 ;
	while(i < j ) {
		k = (i+j)/2 ;
		K = k;
		r = gwComp(p, gwIndex+k) ;
		if(r == 0 ) return(k) ;
/*
		printf("%4d %4d %4d %8d   ",i,k,j,r ) ;
		printf("%s %s",gwIndex[k].station,ctime(&(gwIndex[k].sTime)) ) ;
*/
		if( r > 0 ) i = k+1 ; else j = k-1 ;
	}
	k = (i+j)/2 ;
	r = gwComp(p, gwIndex+k) ;
	if(( r < 0 )&& ( k>0 )) {
		k-- ;
		r = gwComp(p, gwIndex+k) ;
	}	
	K = k;
/* 
	printf("%4d %4d %4d %8d   ",i,k,j,r ) ;
	printf("%s %s",gwIndex[k].station,ctime(&(gwIndex[k].sTime)) ) ;
	if( r < 0 ) bclog(1,"This should not happen in binSearch"," ") ;
	if( r*100 >= gwIndex[k].nSamp ) k = -1 ;
*/
	if( r < 0 ) return(-1) ;
	pp = gwIndex + k ;
	if( p->freq - pp->freq ) return(-1) ;
	if( strcmp(pp->station,p->station)) return(-1)   ;
	if( p->sTime >= ( pp->sTime + pp->nSamp/pp->freq )) return(-1) ;
	return(k) ;
}

int ThereIsIndex(BcIndex b)
{
	char *bcbase = getenv("BC_BASE") ;
	char buff[200] ;
	int fd,size,end,j ;
	sRBcPath( evDay( b.sTime) , gwDirName ) ;
	sprintf(buff,"%s/index",gwDirName) ;
	fd = open(buff,O_RDONLY) ;
	if( fd < 0 ) {
		perror(buff);fflush(stderr);
/*		sleep(10);
		printf("K = %d\n",K);
		rbPrintInd(); */
		return(0);
	}
	close(fd);
	return(1);
}

void rbNewDay(time_t tid)
{
	int d;
	static time_t t,indexTime; /* It would be better to have indexTime common to gwGetWdataS()*/
	d = evDay(tid);
	time(&t);
	if(( d != gwOldDay ) || ( (t - indexTime) > 300 )) {
        gwNewDay(tid) ;
        gwOldDay = d ;
        indexTime = t ; /* index is updated if more than 5 min old */
    }
}


/*
	ThereIsData takes the request in the BcIndex struct Req and returns
	the availability information in the BcIndex vector Avail.

	Avail[0] holds the information about the actual data request.
	Avail[1] holds the information about the previous data segment.
	Avail[2] holds the information about the following data segment.

	981231 Reynir Bodvarsson
*/
int ThereIsWData( BcIndex Req, BcIndex *Avail) 
{
	int k;
	BcIndex test ;
	if (!ThereIsIndex(Req)) 
		return(-1);
	rbNewDay(Req.sTime) ;
	k = rbBinSearch(&Req) ;
	memcpy(&test,&Req,sizeof(BcIndex));
	memcpy(&Avail[0],&Req,sizeof(BcIndex));
	memcpy(&Avail[1],&Req,sizeof(BcIndex));
	memcpy(&Avail[2],&Req,sizeof(BcIndex));
	if( k >= 0 ) {
		memcpy(&Avail[0],&gwIndex[k],sizeof(BcIndex));
		K--;
	} else 
		if ( (K+1 <gwNIndex) && (Req.sTime + Req.nSamp/Req.freq > gwIndex[K+1].sTime) &&
		    !strcmp(gwIndex[K+1].station,Req.station)) 
				memcpy(&Avail[0],&gwIndex[K+1],sizeof(BcIndex));
		else {
			Avail[0].sTime = 0;
			Avail[0].nSamp = 0;
		}
	if ((K >= 0) && (!strcmp(gwIndex[K].station,Req.station)) && 
		(gwIndex[K].freq == Req.freq)) {
		memcpy(&Avail[1],&gwIndex[K],sizeof(BcIndex));
	} else 
		Avail[1].sTime = 0;
	K += ((Avail[0].sTime>0));
	do {
		K++;
	if ((K < gwNIndex) && (!strcmp(gwIndex[K].station,Req.station)) &&
		(gwIndex[K].freq == Req.freq)) {
		memcpy(&Avail[2],&gwIndex[K],sizeof(BcIndex));
	} else 
		Avail[2].sTime = 0;
	} while ((K < gwNIndex) && (Avail[0].sTime == Avail[2].sTime));
	if (!Avail[1].sTime) {
		test.sTime = Req.sTime - (Req.sTime % 86400) - 1;
		if (!ThereIsIndex(test)) {
			Avail[1].sTime = 0;
			Avail[1].nSamp = 0;
		} else {
			rbNewDay(test.sTime) ;
			k = rbBinSearch(&test) ;
			if ((K >= 0) && (!strcmp(gwIndex[K].station,Req.station)) &&
				(gwIndex[K].freq == Req.freq)) 
				memcpy(&Avail[1],&gwIndex[K],sizeof(BcIndex));
		} 
	}
	if (!Avail[2].sTime) {
		test.sTime = Req.sTime - (Req.sTime % 86400) + 86400;
		if (!ThereIsIndex(test)) {
			Avail[2].sTime = 0;
			Avail[2].nSamp = 0;
			sRBcPath(evDay(Req.sTime),gwDirName);   /*  sett inn 19/8/04 SSJ    */
		} else {
			rbNewDay(test.sTime) ;
			k = rbBinSearch(&test) ;
			if ((K >= 0) && (!strcmp(gwIndex[K].station,Req.station)) &&
				(gwIndex[K].freq == Req.freq)) 
				memcpy(&Avail[2],&gwIndex[K],sizeof(BcIndex));
		} 
	}
	if (Avail[0].sTime && (Avail[0].sTime <= Req.sTime) && 
	    ((Avail[0].sTime + Avail[0].nSamp/Avail[0].freq) >= (Req.sTime + Req.nSamp/Req.freq)))
		return(1);
	return(0);
}


/*
	ThereIsDataC takes the request in the BcIndex struct Req and returns
	the availability information in the BcIndex vector Avail. This is
	the same routine as ThereIsData exept for that this routine looks
	for data in both directions for more than one day.

	Avail[0] holds the information about the actual data request.
	Avail[1] holds the information about the previous data segment.
	Avail[2] holds the information about the following data segment.

	9908 Reynir Bodvarsson
*/
int ThereIsWDataC( BcIndex Req, BcIndex *Avail) 
{
	int k,IndexFileAvailable;
	BcIndex test ;
	if (!ThereIsIndex(Req)) 
		return(-1);
	rbNewDay(Req.sTime) ;
	k = rbBinSearch(&Req) ;
	memcpy(&test,&Req,sizeof(BcIndex));
	memcpy(&Avail[0],&Req,sizeof(BcIndex));
	memcpy(&Avail[1],&Req,sizeof(BcIndex));
	memcpy(&Avail[2],&Req,sizeof(BcIndex));
	if( k >= 0 ) {
		memcpy(&Avail[0],&gwIndex[k],sizeof(BcIndex));
		K--;
	} else 
		if ( (K+1 <gwNIndex) && (Req.sTime + Req.nSamp/Req.freq > gwIndex[K+1].sTime) &&
		    !strcmp(gwIndex[K+1].station,Req.station)) 
				memcpy(&Avail[0],&gwIndex[K+1],sizeof(BcIndex));
		else {
			Avail[0].sTime = 0;
			Avail[0].nSamp = 0;
		}
	if ((K >= 0) && (!strcmp(gwIndex[K].station,Req.station))) {
		memcpy(&Avail[1],&gwIndex[K],sizeof(BcIndex));
	} else 
		Avail[1].sTime = 0;
	K += ((Avail[0].sTime>0));
	do {
		K++;
	if ((K < gwNIndex) && (!strcmp(gwIndex[K].station,Req.station)) &&
		(gwIndex[K].freq == Req.freq)) {
		memcpy(&Avail[2],&gwIndex[K],sizeof(BcIndex));
	} else 
		Avail[2].sTime = 0;
	} while ((K < gwNIndex) && (Avail[0].sTime == Avail[2].sTime));
	IndexFileAvailable = 1;
	if (!Avail[1].sTime) {
	  test.sTime = Req.sTime;
	  do {
		test.sTime = test.sTime - (Req.sTime % 86400) - 1;
		if (!ThereIsIndex(test)) {
			Avail[1].sTime = 0;
			Avail[1].nSamp = 0;
			IndexFileAvailable = 0;
		} else {
			rbNewDay(test.sTime) ;
			k = rbBinSearch(&test) ;
			if ((K >= 0) && (!strcmp(gwIndex[K].station,Req.station)) &&
				(gwIndex[K].freq == Req.freq)) 
				memcpy(&Avail[1],&gwIndex[K],sizeof(BcIndex));
		} 
	  } while (!Avail[1].sTime && IndexFileAvailable);
	}
/*fprintf(stderr,"Avail[2].sTime = %d\n",Avail[2].sTime);fflush(stderr); */
	IndexFileAvailable = 1;
	if (!Avail[2].sTime) {
	  test.sTime = Req.sTime;
	  do {
		test.sTime = test.sTime - (Req.sTime % 86400) + 86400;
		if (!ThereIsIndex(test)) {
/*fprintf(stderr,"No index file ?  %d\n",Req.sTime);fflush(stderr);*/
			Avail[2].sTime = 0;
			Avail[2].nSamp = 0;
			IndexFileAvailable = 0;
		} else {
			rbNewDay(test.sTime) ;
			k = rbBinSearch(&test) ;
			do {
			if ((K >= 0) && (!strcmp(gwIndex[K].station,Req.station)) &&
				(gwIndex[K].freq == Req.freq)) 
					memcpy(&Avail[2],&gwIndex[K],sizeof(BcIndex));
				K++;
			} while((K<gwNIndex)&&((strcmp(gwIndex[K-1].station,
				Req.station)) || (gwIndex[K].freq != Req.freq)));
/*fprintf(stderr,"Check Next Day: k = %d, K = %d, Avail[2].sTime = %d\n",k,K,Avail[2].sTime);*/
/*		rbPrintInd(); */
		} 
	  } while (!Avail[2].sTime && IndexFileAvailable);
	}
	if (Avail[0].sTime && (Avail[0].sTime <= Req.sTime) && 
	    ((Avail[0].sTime + Avail[0].nSamp/Avail[0].freq) >= (Req.sTime + Req.nSamp/Req.freq)))
		return(1);
	return(0);
}

/*
	isbcdata_ takes the request in the variables sta (station name),
	rT (requested time), rS (requested number of seconds) and rF
	(requested frequency) and returns the availability information
	in the analogue vectors aT[3] and aS[3].

	aT[0] and aS[0] hold the information about the actual data request.
	aT[1] and aS[1] hold the information about the previous data segment.
	aT[2] and aS[2] hold the information about the following data segment.

	981231 Reynir Bodvarsson (Remains to be tested. No FORTRAN available here.)
*/

int isbcdata_(char *sta, int *rT, int *rS, int *rF, int *aT, int *aS, int stal) 
{
	int i,ret;
	BcIndex r,a[3];
	while ((sta[stal] < '0') || (sta[stal] > 'z') && stal) {
		sta[stal] = '\0';
		stal--;
	}
	if (stal < 1) return(-1);
	strcpy(r.station,sta);
	r.sTime = *rT;
	r.nSamp = *rS * *rF;
	r.freq  = *rF;
	ret = ThereIsWData(r,a);
	for (i=0; i<3; i++) {
		aT[i] = a[i].sTime;
		aS[i] = a[i].nSamp/a[i].freq;
	}
	return(ret);
}

/* 
   For some reason f2c seems to have problem with returning function values.
   I therefor make here a subroutine insted of function for the getwdata call.
   2001 06 12  Reynir 
*/

void sgetwdata_( char *station, int *sTime, int *channel, int *nSec, int *sFreq,
    float *data, int *retTime,
    char *sensor, char *digitizer, char *timeSync, int *retSample,
    int lsta, int lsens, int ldig, int lsync)
{
*retSample = getwdata_(station,sTime,channel,nSec,sFreq,
    data, retTime,
    sensor, digitizer, timeSync,
    lsta, lsens, ldig, lsync );
}

/*
	Here is an example on how to use the isbcdata_() routine in FORTRAN.
	Note! Remains to be tested.

       implicit none
       character*20 sta
       integer ok,rt,rs,rf,at(30,as(3);

       sta = 'ada'
       rt  = 913953600
       rs  = 20
       rf  = 100
      		
       ok = isbcdata(sta,rt,rs,rf,at,as)

       write(*,*) sta,rt,rs,rf,ok
       do 10 i=1,3
10     write(*,*) sta,rt,rs,rf,ok
       end
*/

