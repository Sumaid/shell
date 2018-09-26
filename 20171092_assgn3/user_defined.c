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
#include "user_defined.h"


void env_setter(){
	if ((parsed[current_command].arguments_index==0)||(parsed[current_command].arguments_index>2))
	{
		printf("usage: setenv var[value]\n");
		return;
	}
	setenv(parsed[current_command].arguments[0], parsed[current_command].arguments[1], 1);
}

void env_unsetter(){
	if (parsed[current_command].arguments_index!=1)
	{
		printf("usage: unsetenv var\n");
		return;
	}
	unsetenv(parsed[current_command].arguments[0]);
}

void jobs(){
	for (int i=1; i<pid_top; i++)
	{
		if (strcmp(proc_status[i], "Deleted"))
	    	printf("[%d]      %s    %s[%d]\n", i, proc_status[i], proc_stack[i], pid_stack[i]);
	}
}

void fg(){
	if (parsed[current_command].arguments_index!=1)
	{
		printf("usage: fg <jobnumber>\n");
		return;
	}
	kill(pid_stack[atoi(parsed[current_command].arguments[0])], SIGCONT);
    fprintf(stderr,"\n[%d]   %s\n", atoi(parsed[current_command].arguments[0]), proc_stack[atoi(parsed[current_command].arguments[0])]);
	waitpid(pid_stack[atoi(parsed[current_command].arguments[0])], NULL, 0);
}

void bg(){
	if (parsed[current_command].arguments_index!=1)
	{
		printf("usage: bg <jobnumber>\n");
		return;
	}
	kill(pid_stack[atoi(parsed[current_command].arguments[0])], SIGCONT);
    fprintf(stderr,"\n[%d]   %s &\n", atoi(parsed[current_command].arguments[0]), proc_stack[atoi(parsed[current_command].arguments[0])]);
    strcpy(proc_status[atoi(parsed[current_command].arguments[0])],"Running");
}

void kjob(){
	if (parsed[current_command].arguments_index!=2)
	{
		printf("usage: kjob <jobnumber> <signalnumber>\n");
		return;
	}
	kill(pid_stack[atoi(parsed[current_command].arguments[0])], atoi(parsed[current_command].arguments[1]));
}

void overkill(){
	for (int i = 1; i<pid_top; i++)
		kill(pid_stack[i], 9);
}