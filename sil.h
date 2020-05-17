
/*	record definitions for sil data base */
/* header for bit_compress data files */
typedef struct {
	time_t	sTime ;		/* 4 */
	char	station[4] ;	/* 8 */
	short	freq ;		/*10 */
	short	nData ;		/*12 */
} SilBcHead ;

typedef struct {
        char station[4], tail[20], id[20],statype[40] ;
        int frac, freq,n ;
        time_t sTime ;
        int *data ;
} SilData ;

