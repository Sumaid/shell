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


void read_input(){
	input = (char *)malloc(1024);
	while(1)
	{
		char temp;
		scanf("%c", &temp);
		if (temp=='\n')
			break;
    		strcat(input, &temp);
	}
	printed = 0;
}