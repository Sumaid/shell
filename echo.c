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


void echo(){
	int i;
	for (i=0; i<parsed[current_command].arguments_index-1; i++)
		printf("%s ", parsed[current_command].arguments[i]);
	if (parsed[current_command].arguments_index>0)
		printf("%s\n", parsed[current_command].arguments[parsed[current_command].arguments_index-1]);
	else
		printf("\n");
}
