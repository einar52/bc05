/* filter utilities useful for detection */

double gFilt( double *f, double x ) ;
/* apply general filter
        f contains sizes, coefficients and work data */
double *tGFilt( double scale, int size) ;

void addZero( double *f , double *zf) ;

void addPole( double *f , double *pf) ;

double zFilt( double *f, double x ) ;
/*      apply filter with to zeros on unit circle */

double *mkZFilt( double dt, double freq ) ;
/*      make filter with a zero pair on unit circle */

double nFilt( double *f, double x ) ;
	/* apply notch filter, seep mkNFilt  */

double *mkNFilt( double dt, double freq, double df) ;
	/* make notchfiler, pair of complex conjugate poles
		and zeroes
		returns a pointer to work array of 8 double values
		allocated using malloc */

double pFilt( double *f, double x ) ;
/* apply pole filter, f is a pointer from mkPFilt 	*/

double envelope( double *f ) ;
/* get envelope of function, should be called after pFilt
	f is a pointer from mkpfilt */

double * mkPFilt( double dt, double freq, double df)  ;
	/* make polefiler, pair of complex conjugate poles
		returns a pointer to work array of 6 double values
		allocated using malloc */

double *readFilter( char *fname )  ;

void saveFilter( double *filter, char *name ) ;

double *initFilter( double *fin ) ;
/* initialize a new filter using fin */

