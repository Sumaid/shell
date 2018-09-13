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
#include "read_input.h"
#include "parse_input.h"
#include "execute_input.h"


char *state;
int vm_result;
char *shell_pwd;
char shell_home[1024];
char mon[4];
char *shell_prompt;
char *input;
char *cur_input;
int main_pid;
int pid_stack[1024];
int pid_top;
int current_command;
command_struct parsed[1024];
int commands_index; 
int redirect_flag;

void free_command(){
	parsed[current_command].flags_index = 0;
	parsed[current_command].arguments_index = 0;
	parsed[current_command].o_fd  = 1;
	parsed[current_command].i_fd  = 0;
}

int io_decider(){
	parsed[current_command].o_fd  = 1;	
	parsed[current_command].i_fd  = 0;
    if (parsed[current_command].outputfile!=NULL)
    {
    	printf("--- output file is %s ---\n", parsed[current_command].outputfile);
    	if (strlen(parsed[current_command].outputfile))
        {	
        	if (redirect_flag == 1)
        		parsed[current_command].o_fd = open(parsed[current_command].outputfile, O_WRONLY | O_CREAT, 00644);
        	else if (redirect_flag == 2)
        		parsed[current_command].o_fd = open(parsed[current_command].outputfile, O_WRONLY | O_APPEND | O_CREAT, 00644);
        }

    }
    if (parsed[current_command].inputfile!=NULL)
    {
    	printf("--- input file is %s ---\n", parsed[current_command].inputfile);
    	if (strlen(parsed[current_command].inputfile))
        {	
        	parsed[current_command].i_fd = open(parsed[current_command].inputfile, O_RDONLY);
        	if (parsed[current_command].i_fd<0)
        	{
        		printf("%s: No such file or directory\n", parsed[current_command].inputfile);
        		return parsed[current_command].i_fd;
        	}
        }
    }
    return 1;
}


void prompt(){
	shell_prompt = (char *)malloc(1024);
//To get user name	
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);

//To get system name
	struct utsname unameData;
	uname(&unameData);
   	shell_dir();
   	strcpy(shell_prompt, "<");
   	strcat(shell_prompt, hostname);
   	strcat(shell_prompt, "@");
   	strcat(shell_prompt, unameData.sysname);
   	strcat(shell_prompt, ":");
   	strcat(shell_prompt, shell_pwd);
   	strcat(shell_prompt, ">");
}

void command_loop(){
	int checker = 1;
	while(1)
	{
		prompt();
		int i;

		for (i=1; i<pid_top; i++)
		{
			int status;
			if (waitpid(pid_stack[i], &status, WNOHANG) > 0)
			{
				if (WIFEXITED(status) > 0) 
				{
					printf("%s with pid %d exited normally\n", proc_stack[i], pid_stack[i]);
				}
				else if(WIFSIGNALED(status))
				{
					printf("%s with pid %d exited with signal\n", proc_stack[i], pid_stack[i]);
				}
				else
					printf("%s with pid %d exited abnormally\n", proc_stack[i], pid_stack[i]);
			}
		}
		printf("%s", shell_prompt);
		read_input();
		parse_input();		
		if (io_decider()<0)
			continue;
		print_command();
		int j = current_command;
		current_command = 0;
		while (current_command < j + 1)
		{
			//print_command();
			//printf("current_command is %d\n", current_command);
			if (strcmp(parsed[current_command].command, ""))
				execute_input();
			free_command();
			current_command += 1;
		}
		current_command = 0;
		free(shell_prompt);
		//print_command();
	}
}

int main(){

//Initialize Shell Home Directory
	if (getcwd(shell_home, sizeof(shell_home)) != NULL) {
   	} 
   	else {
       perror("getcwd() error");
       return 1;
   	}
   	main_pid = (int)getpid();
   	pid_stack[pid_top] = main_pid;
   	strcpy(proc_stack[pid_top],"shell");
   	pid_top++;
//Running Loop which checks for command entered   	
   	command_loop();
	return 0;
}
