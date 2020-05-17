#include <stdio.h>

main()
{
	char line[200],sta[4],type[20],dig[20],sync[100];
	int n,ti,to,ch,l,sps;
	float data[10000];
	ch = 0;

	while (scanf("%[^\n]",line) > 0) {
		getchar();
		sscanf(line,"%s %d %d %d %d",sta,&ti,&ch,&l,&sps);
		n = getwdata_(sta,&ti,&ch,&l,&sps,data,&to,type,dig,sync,
		                4,                           20, 20,100);
		printf("%s % 9d % 9d %s %s %s % 4d\n",sta,ti,to,type,dig,sync,n);
		fflush(stdout);
	}
}
			
		
