
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

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "evlib.h"
#include "silread.h"
#include "getwdata.h"

int isbcdata_(char *station, int *sTime, int *nSec, int *sFreq, int *aT, int *aS, int stal) 
/* 	This is a replacement for function of same name that is in isdata.c
	If this version is used with lokimp, gwGetWData will merge data from
	more than one bc file.
		Einar Kjartansson, july 2008
*/
{
	SilChannel *sd ;
	char sta[8],buff[100] ;
	int i ;
	strncpy(sta,station,3) ; sta[3] = 0 ;
	sprintf(buff,"isbcdata(%s, %d %d, %d....",sta,evDay(*sTime),evDTime(*sTime),*nSec) ;
	bclog(6,buff,NULL) ;
	sd = gwGetWData(sta, *sTime, *nSec , *sFreq) ; 
	if( NULL == sd ) return ( 0 )  ;
	for( i = 0 ; i < 3 ; i++) {
		aT[i] = sd->sTime ;
		aS[i] = sd->nData/sd->freq ;
	}
	sprintf(buff,"isbcdata return: %s %d, %d....",sta,evDTime(sd->sTime),*aS)  ;
        bclog(6,buff,NULL) ;
	free(sd) ;
	return(1) ;
}

