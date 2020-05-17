
#include <time.h>
int cftime(char *s, char *format, const time_t *clock)
{
	struct tm *tp ;
	int rr ;
	tp = gmtime( clock ) ;
	rr = strftime(s,80,format,tp) ;
	return(rr) ;
}
