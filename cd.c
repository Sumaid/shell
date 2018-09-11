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

void shell_dir(){
	char temp[1024];
	if (getcwd(temp, sizeof(temp)) != NULL) {
		if (strlen(temp)<strlen(shell_home))
		{
   			shell_pwd = malloc(strlen(temp) + 1 + 1 );
   			strcpy(shell_pwd, temp);
   		}
		else if (!strcmp(temp,shell_home))
   		{
   			shell_pwd = malloc(3);
   			strcpy(shell_pwd, "~");
			return;
      	}
      	else
   		{
   			int p = 0;
   			int home_size = strlen(shell_home);
   			shell_pwd = malloc(strlen(temp) + 1 + 1 );
   			strcpy(shell_pwd, "~");
   			for (int i=home_size; i<strlen(temp); i++)
   				shell_pwd[++p] = temp[i];
   		}
   	} 
   	else {
       perror("getcwd() error");
   	}
}

void cd(){
	if (parsed[current_command].arguments_index>1)
	{
		printf("%s: too many arguments\n", parsed[current_command].command);
		return;
	}
	if (parsed[current_command].arguments_index==0)
	{
		int erp = chdir(shell_home);
		shell_dir();
		if (!erp)
			return;
	}
	else if (parsed[current_command].arguments_index==1)
	{
		char *path;
   		path = (char *)malloc(1024);
		if (!strcmp(parsed[current_command].arguments[0], "~"))
			strcpy(path, shell_home);
		else
		{
			if (parsed[current_command].arguments[0][0]=='~')
			{
				strcpy(path, shell_home);
    			int i;
    			for (i=1; i<strlen(parsed[current_command].arguments[0]); i++)
    				path[strlen(path)] = parsed[current_command].arguments[0][i];	
			}
			else
				strcpy(path, parsed[current_command].arguments[0]);
		}
		int erp = chdir(path);
		shell_dir();
		if (!erp)
			return;
	}
	
	if (errno!=0)
		printf("%s\n", strerror(errno));
}