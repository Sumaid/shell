#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "cd.h"
#include "pinfo.h"
#include "pwd.h"
#include "echo.h"
#include "ls.h"
#include "remindme.h"
#include "clock.h"
#include "globals.h"

int count = 1;
char proc_stack[1024][1024];

int execute_input(){
		if (!strcmp(parsed[current_command].command,"pwd"))
		{
			pwd();
			return 1;
		}
		else if (!strcmp(parsed[current_command].command,"echo"))
		{
			echo();
			return 1;
		}
		else if(!strcmp(parsed[current_command].command,"exit"))
		{
			kill(getpid(), 9);
			return 1;
		}
		else if (!strcmp(parsed[current_command].command,"pinfo"))
		{
			pinfo();
			return 1;
		}
		else if (!strcmp(parsed[current_command].command, "cd"))
		{
			cd();
			return 1;
		}
		else if (!strcmp(parsed[current_command].command, "clock"))
		{
			self_clock();
			return 1;
		}
		int status;
		pid_t pid = fork(), w;
		if (pid>0)
		{
			if (strcmp(parsed[current_command].command, "remindme"))
			{	
				pid_stack[pid_top] = pid;
		   		strcpy(proc_stack[pid_top],parsed[current_command].command);
		   		pid_top++;
		   	}
			if (is_bk())
				printf("[%d] %d\n", count++, pid);
			else if (strcmp(parsed[current_command].command, "remindme"))
			{
				if (waitpid (pid, &status, 0) != pid)
      				status = -1;	
      		}
		}
		else if (pid==0)
		{
			if (is_bk())
			{
				parsed[current_command].arguments[parsed[current_command].arguments_index-1] = NULL;
				parsed[current_command].arguments_index -= 1;
			}
			int size = 2 + parsed[current_command].flags_index + parsed[current_command].arguments_index;
			char *buf[size];
			buf[0] = (char *)malloc(1024);
			strcpy(buf[0], parsed[current_command].command);
			int i, k = 1;
			for (i=0; i<parsed[current_command].flags_index; i++)
			{
				buf[k] = (char *)malloc(1024);
				strcpy(buf[k], "-");
				strcat(buf[k], parsed[current_command].flags[i]);
				k++;
			}
			for (i=0; i<parsed[current_command].arguments_index; i++)
			{
				if (strcmp(parsed[current_command].arguments[i],"&"))
				{
					buf[k] = (char *)malloc(1024);
					strcat(buf[k], parsed[current_command].arguments[i]);
					k++;
				}
			}
			if (!strcmp(parsed[current_command].command,"remindme"))
			{
				remindme();
				return 1;
			}
			else if (!strcmp(parsed[current_command].command,"ls"))
				ls();
			else
			{
				printf("at least here\n");
				printf("buf[0] is %s\n", buf[0]);
				printf("parsed[current_command].i_fd %d\n", parsed[current_command].i_fd);
				printf("parsed[current_command].o_fd %d\n", parsed[current_command].o_fd);

				dup2(parsed[current_command].i_fd, 0);
				dup2(parsed[current_command].o_fd, 1);
				buf[k] = NULL;
					if (execvp(parsed[current_command].command, buf) < 0) {     
		                printf("*** ERROR: exec failed\n");
		                exit(1);				
				}
		    }
		    status = 1;
		    exit(0);
		}	
		else
		{
			status = -2;
			printf("fork error\n");	
		}
		return status;
}