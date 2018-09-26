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
#include "user_defined.h"


int count = 1;
char proc_stack[1024][1024];

int execute_input(){
		temp_pid = 0;
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
		else if(!strcmp(parsed[current_command].command,"quit"))
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
		else if (!strcmp(parsed[current_command].command, "setenv"))
		{
			env_setter();
			return 1;
		}
		else if (!strcmp(parsed[current_command].command, "unsetenv"))
		{
			env_unsetter();
			return 1;
		}
		else if (!strcmp(parsed[current_command].command, "jobs"))
		{
			jobs();
			return 1;
		}
		else if (!strcmp(parsed[current_command].command, "overkill"))
		{
			overkill();
			return 1;
		}
		else if (!strcmp(parsed[current_command].command, "kjob"))
		{
			kjob();
			return 1;
		}
		else if (!strcmp(parsed[current_command].command, "fg"))
		{
			fg();
			return 1;
		}
		else if (!strcmp(parsed[current_command].command, "bg"))
		{
			bg();
			return 1;
		}


		int status;
		pid_t pid = fork(), w;
		if (pid>0)
		{
			if (strcmp(parsed[current_command].command, "remindme"))
			{	
		/*		pid_stack[pid_top] = pid;
		   		strcpy(proc_stack[pid_top], parsed[current_command].command);
		   		strcpy(proc_status[pid_top],"Running");
		   		pid_top++;
		*/
		   		temp_pid = pid;
		   		temp_command = current_command;
		   	}
			if (is_bk())
			{
				printf("[%d] %d\n", pid_top, temp_pid);
				pid_stack[pid_top] = temp_pid;
		   		strcpy(proc_stack[pid_top], parsed[temp_command].command);
		   		strcpy(proc_status[pid_top],"Running");
		   		pid_top++;			
			}
			else if (strcmp(parsed[current_command].command, "remindme"))
			{
				if (waitpid (pid, &status, WUNTRACED) != pid)
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

int pipes[1024][2];

int pipeexecute_input(){
  temp_pid = 0;
  int i, j;
  int stdin = dup(STDIN_FILENO);
  int stdout = dup(STDOUT_FILENO);
  int pid[pipe_number+1];
  int pipe_fd[2];
  int previous_read = parsed[0].i_fd;

  for (i=0; i<pipe_number+1; i++)
  {
  		//printf("Executed Inside Pipe\n");
	  	//print_command(i);
  		if (pipe(pipe_fd)<0)
  		{
  			printf("Pipe Error\n");
  			return -1;
  		}
	  	pid[i] = fork();
	    if (pid[i] == 0)	
	    {
	    	dup2(previous_read, 0);
	    	parsed[i].i_fd = previous_read;
	    	if (i!=pipe_number)
	    	{
	    		dup2(pipe_fd[1], 1);
	    		parsed[i].o_fd = pipe_fd[1];
	    	}
	    	else
	    		dup2(parsed[i].o_fd, 1);

	    	close(pipe_fd[0]);
			
			if (is_bk())
			{
				parsed[i].arguments[parsed[i].arguments_index-1] = NULL;
				parsed[i].arguments_index -= 1;
			}

			int size = 2 + parsed[i].flags_index + parsed[i].arguments_index;
			char *buf[size];
			buf[0] = (char *)malloc(1024);
			strcpy(buf[0], parsed[i].command);
			int j, k = 1;
			for (j=0; j<parsed[i].flags_index; j++)
			{
				buf[k] = (char *)malloc(1024);
				strcpy(buf[k], "-");
				strcat(buf[k], parsed[i].flags[j]);
				k++;
			}
			for (j=0; j<parsed[i].arguments_index; j++)
			{
				if (strcmp(parsed[i].arguments[j],"&"))
				{
					buf[k] = (char *)malloc(1024);
					strcat(buf[k], parsed[i].arguments[j]);
					k++;
				}
			}
				buf[k] = NULL;
			if (!strcmp(parsed[i].command,"pwd"))
			{
				pwd();
				return 1;
			}
			else if (!strcmp(parsed[i].command,"echo"))
			{
				echo();
				return 1;
			}
			else if(!strcmp(parsed[i].command,"quit"))
			{
				kill(getpid(), 9);
				return 1;
			}
			else if (!strcmp(parsed[i].command,"pinfo"))
			{
				pinfo();
				return 1;
			}
			else if (!strcmp(parsed[i].command, "cd"))
			{
				cd();
				return 1;
			}
			else if (!strcmp(parsed[i].command, "clock"))
			{
				self_clock();
				return 1;
			}
			else if (!strcmp(parsed[i].command, "setenv"))
			{
				env_setter();
				return 1;
			}
			else if (!strcmp(parsed[i].command, "unsetenv"))
			{
				env_unsetter();
				return 1;
			}
			else if (!strcmp(parsed[i].command, "jobs"))
			{
				jobs();
				return 1;
			}
			else if (!strcmp(parsed[i].command, "overkill"))
			{
				overkill();
				return 1;
			}
			else if (!strcmp(parsed[i].command, "kjob"))
			{
				kjob();
				return 1;
			}
			else if (!strcmp(parsed[i].command, "fg"))
			{
				fg();
				return 1;
			}
			else if (!strcmp(parsed[i].command, "bg"))
			{
				bg();
				return 1;
			}
			else if (!strcmp(parsed[i].command,"remindme"))
			{
				remindme();
				return 1;
			}
			else if (!strcmp(parsed[i].command,"ls"))
				ls();
			else
			{
					if (execvp(parsed[i].command, buf) < 0) {     
		                printf("*** ERROR: exec failed\n");
		                exit(1);				
		     }
		    }
	    	_exit(0);
	    }
	    else
	    {
	    	//in parent just waiting and closing pipes(unused)
			if (!is_bk())
			{
				wait(NULL);
	    	}
	    	close(pipe_fd[1]);
	    	previous_read = pipe_fd[0];
	    	//continue;
	    }
  }
}


int pipeexecute_input2(){
  int i, j;
  int stdin = dup(STDIN_FILENO);
  int stdout = dup(STDOUT_FILENO);
  int pid[pipe_number+1];
  int pipe_fd[2];
  int previous_read = parsed[0].i_fd;

  for (i=0; i<pipe_number+1; i++)
  {
  		printf("Executed Inside Pipe\n");
	  	print_command(i);
  		if (pipe(pipe_fd)<0)
  		{
  			printf("Pipe Error\n");
  			return -1;
  		}
	  	pid[i] = fork();
	    if (pid[i] == 0)	
	    {
	    	dup2(previous_read, 0);
	    	if (i!=pipe_number)
	    		dup2(pipe_fd[1], 1);
	    	else
	    		dup2(parsed[i].o_fd, 1);

	    	close(pipe_fd[0]);
			
			if (is_bk())
			{
				parsed[i].arguments[parsed[i].arguments_index-1] = NULL;
				parsed[i].arguments_index -= 1;
			}

			int size = 2 + parsed[i].flags_index + parsed[i].arguments_index;
			char *buf[size];
			buf[0] = (char *)malloc(1024);
			strcpy(buf[0], parsed[i].command);
			int j, k = 1;
			for (j=0; j<parsed[i].flags_index; j++)
			{
				buf[k] = (char *)malloc(1024);
				strcpy(buf[k], "-");
				strcat(buf[k], parsed[i].flags[j]);
				k++;
			}
			for (j=0; j<parsed[i].arguments_index; j++)
			{
				if (strcmp(parsed[i].arguments[j],"&"))
				{
					buf[k] = (char *)malloc(1024);
					strcat(buf[k], parsed[i].arguments[j]);
					k++;
				}
			}
				buf[k] = NULL;
		//		printf("buf is %s\n", buf[0]);
				char c;
				//scanf("%c", &c);
					if (execvp(parsed[i].command, buf) < 0) {     
		                printf("*** ERROR: exec failed\n");
		                exit(1);				
		     }
	    	_exit(0);
	    }
	    else
	    {
	    	//in parent just waiting and closing pipes(unused)
			if (!is_bk())
			{
				wait(NULL);
	    	}
	    	close(pipe_fd[1]);
	    	previous_read = pipe_fd[0];
	    	//continue;
	    }
  }
}


int execute_input2(){
  int i, j;
  int pid[pipe_number+1];
  int stdin = dup(STDIN_FILENO);
  int stdout = dup(STDOUT_FILENO);

  for (i=0; i<pipe_number+1; i++)
  {
	  	print_command(i);
	  	if (i!=pipe_number)
	  	{
	  		if (pipe(pipes[i])<0)
	  		{
	  			printf("Pipe Error\n");
	  			return -1;
	  		}
	  	}
	  	pid[i] = fork();
	    if (pid[i] == 0)	
	    {
			if (i != 0)
			{
				//changing input fd to previous pipe's input port for very iteration except first one(that'll be standard) 
				dup2(pipes[i-1][0], STDIN_FILENO);
				close(pipes[i-1][1]);
				close(pipes[i-1][0]);
			}
			if (i != pipe_number)
			{
				//changing output fd to pipe's output port for very iteration except last one(that'll be standard) 
				dup2(pipes[i][1], STDOUT_FILENO);
				close(pipes[i][0]);
				close(pipes[i][1]);
			}

			int j;
			printf("i is %d\n", i);
			char d;
		//	scanf("%c", &d);
			
			if (is_bk())
			{
				parsed[i].arguments[parsed[i].arguments_index-1] = NULL;
				parsed[i].arguments_index -= 1;
			}
			int size = 2 + parsed[i].flags_index + parsed[i].arguments_index;
			char *buf[size];
			buf[0] = (char *)malloc(1024);
			strcpy(buf[0], parsed[i].command);
			int k = 1;
			for (j=0; j<parsed[i].flags_index; j++)
			{
				buf[k] = (char *)malloc(1024);
				strcpy(buf[k], "-");
				strcat(buf[k], parsed[i].flags[j]);
				k++;
			}
			for (j=0; j<parsed[i].arguments_index; j++)
			{
				if (strcmp(parsed[i].arguments[j],"&"))
				{
					buf[k] = (char *)malloc(1024);
					strcat(buf[k], parsed[i].arguments[j]);
					k++;
				}
			}
				buf[k] = NULL;
		//		printf("buf is %s\n", buf[0]);
				char c;
				//scanf("%c", &c);
					if (execvp(parsed[i].command, buf) < 0) {     
		                printf("*** ERROR: exec failed\n");
		                exit(1);				
		     }
	    	_exit(0);
	    }
	    else
	    {
	    	//in parent just waiting and closing pipes(unused)
			wait(NULL);
	    	int g;
	    	for (g=0; g<=i; g++)
	    	{
	    		close(pipes[g][0]);
	    		close(pipes[g][1]);
	    	}
	    }
  }
}