/*
 *
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

/* 	Definitions for bitcompression of timeseries data.
	This code should work on big and small endian machines.
	It is assumed that short is 16 bits and long and int 32 bits 
*/

#define MCDATA 500000   
#define CDATAHL 12 
#define BcMagic  13*17*113
typedef struct {
	int size ;
	int nData ; /* number of data samples */
	short magic ;
	char  preProc ;	/* 0: nothing, 1: first bcDiff, 2: second cpDiff  */
	char  type    ; /* 0: nothing, 1: 8,16,32 bits ,2: 1..24 bits ..  */
	char  data[MCDATA] ;
} CData ; 

long long bcBitRight[40],bcBit[40],bcBitLeft[40] ;
typedef struct {
	char *data ;
	int bytes,bits ;
	long long work ;
} BitBuffer ;

int bcLittleEndian() ; /* return true (1) if on little endain machine */
void bcSwapL( long *val); /* convert bytes in longint, big vs. little endian  */
void bcSwapIf( long *p, int n); /* swap array of n long, if on little end */
void bcResetBits( BitBuffer *b ) ;
void bcInitBits( BitBuffer *b, char *data);
void bcPutBits(  BitBuffer *b, int val, int nbits) ;
int bcFlushBits(BitBuffer *b ) ;
int bcGetBits( BitBuffer *b, int nbits ) ;
int bcComprArr(int *data,int *mag,int n,char *buff);
void bcDeComprArr(char *buff, int *data, int n	) ;
void bcDiff(int *inp, int *outp, int n)  ;
void bcMagFilt( int *inp,int *out,int n,int len );
void bcMagFilt2( int *inp, int *out, int n, int len );
		/* Fill in on slopes  bcMagFilt should be caled first */
void bcGetMag(int *inp,int *out, int n) /* get number of bits in data */;
void bcIntegrate(int *inp,int *outp,int n) ;
int  bcCompBit( int *ind, int n, CData *outd, int nDiff );
int bcCompareI( int *a, int *b, int n) ;
int bcCheckCData( CData *data) ;
int bcDeCompr( CData *data, int *out) ;
int bcWrite( FILE *fd, CData *d );   /* write CData to stdio file */
CData *bcRead( FILE *fd);/* read CData from stdio stream, calls malloc */
