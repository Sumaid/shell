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

void remindme(){
	if (parsed[current_command].arguments_index!=2)
	{
		printf("Incorrect number of commands\nusage: remindme time \"message\"\n");
		exit(1);
	}

	clock_t t;
    t = clock();
    double time_expected = (double)atoi(parsed[current_command].arguments[0]);
    while (1)
    {
    	clock_t t1 = clock() - t;
    	double time_passed = ((double)t1)/CLOCKS_PER_SEC;
    	if (time_passed>time_expected)
    	{
	    	dprintf(parsed[current_command].o_fd, "\nReminder: %s\n", parsed[current_command].arguments[1]);
    		exit(0);
    	}
    }
}