
void utLCase( char *s ) ; /*convert string to lower case */
int utAhPath( char *buf,  char *station, time_t *t ) ;
		/* make path for ah files */
int date2doy( int month, int day,int leap ) ;
int doy2date( int doy, int leap , int *month );
int utMkPath( char *path, int mode ) ;
	/* recursively create directories in path as needed 
	returns 0 if directory exists or was created, -1 otherwise */
FILE *utPathOpen( char *name ) ;
        /*open file for writing, createdirs as needed*/

void get_vika(struct tm *tp, int *ar, int *vika ) ;
/* get number of veek, and its year, for date in tp, 
not, week is from monday (1) to sunday (7) and week belongs to
the year that the thursday belongs to 
Note:	the tp structure is alterded,on return it points to the thursday
*/

int vika2tm( struct tm *tp, int ar, int vika, int vdagur)  ;
/* reverse of get_vika */
