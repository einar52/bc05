#include        <stdio.h>
#include        <sys/types.h>
#include        <time.h>
 
time_t
xmktime(t)
struct tm *t;
{
        time_t  ts;
        int     tmp;
        char    string[4];
 
        time(&ts);
        cftime(string,"%y",&ts);
        sscanf(string,"%d",&tmp);
        ts += (t->tm_year - tmp) * 31536000;
 
        cftime(string,"%m",&ts);
        sscanf(string,"%d",&tmp);
        ts += (t->tm_mon - tmp) * 2419200;
 
        cftime(string,"%d",&ts);
        sscanf(string,"%d",&tmp);
        ts += (t->tm_mday - tmp) * 86400;
 
        cftime(string,"%H",&ts);
        sscanf(string,"%d",&tmp);
        ts += (t->tm_hour - tmp) * 3600;
 
        cftime(string,"%M",&ts);
        sscanf(string,"%d",&tmp);
        ts += (t->tm_min - tmp) * 60;
 
        cftime(string,"%S",&ts);
        sscanf(string,"%d",&tmp);
        ts += (t->tm_sec - tmp);
        return(ts);
}



int main(int ac, char **av)
{
	struct tm ts ;
	time_t  j ,jj ;
	ts.tm_year = atoi(av[1]) ;
	ts.tm_mon = atoi(av[2]) ;
	ts.tm_mday = atoi(av[3]) ;
	ts.tm_hour = atoi(av[4]) ;
	ts.tm_min = atoi(av[5]) ;
	ts.tm_sec = atoi(av[6]) ;
	j = xmktime( &ts ) ;
	jj = mktime( &ts ) ;
	printf("%s\n",ctime(&j)) ;
	printf("%s\n",ctime(&jj)) ;
}
