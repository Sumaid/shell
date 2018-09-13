#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "globals.h"


void self_clock(){
	if (parsed[current_command].arguments_index!=2)
	{
		printf("Incorrect Number of arguments\n");
		printf("usage: clock -t [Delay] -n [Duration]\n");
	}
	int gap = atoi(parsed[current_command].arguments[0]);
	int duration = atoi(parsed[current_command].arguments[1]);
	int d_gap = 0;
	int time_duration = 0;
	while (1)
	{
		if (time_duration==duration)
			break;
		time_t *t;
		t = malloc(1024);
		*t = time(0);
		struct tm tm = *localtime(t);
		Mon(1+tm.tm_mon);
		if (d_gap==0)
			dprintf(parsed[current_command].o_fd, "%d %s %d, %02d:%02d:%02d\n", tm.tm_mday, mon, 1900+tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);	
		d_gap = (d_gap + 1)%gap;
		time_duration++;
		sleep(1);
	}
}