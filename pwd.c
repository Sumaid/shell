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


void pwd(){
	char cur[1024];
	if (getcwd(cur, sizeof(cur)) != NULL) 		
	   printf("%s\n", cur);
   	else 
       perror("getcwd() error");
}