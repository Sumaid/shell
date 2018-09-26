#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
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
#include "user_defined.h"

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
char proc_status[1024][1024];
int pid_top;
int current_command;
command_struct parsed[1024];
int commands_index; 
int redirect_flag;
int temp_pid;
int temp_command;

void free_command(){
	parsed[current_command].flags_index = 0;
	parsed[current_command].arguments_index = 0;
}

int io_decider(int current_command){
//	printf("io decider is applied on %d\n", current_command);
    if (parsed[current_command].outputfile!=NULL)
    {
//    	printf("--- output file is %s ---\n", parsed[current_command].outputfile);
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
//    	printf("--- input file is %s ---\n", parsed[current_command].inputfile);
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

int printed = 0;
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

 void sig_handler(int signo)
  {
    if(signo==SIGINT)
    {
        prompt();
        fprintf(stderr,"\n%s", shell_prompt);
        printed = 1;
    }
    else if (signo==SIGTSTP)
    {
        prompt();
/*		if ((pid_top>1)&&(strcmp(proc_status[pid_top-1], "Stopped"))&&(strcmp(proc_status[pid_top-1], "Deleted")))
		{
			strcpy(proc_status[pid_top-1],"Stopped");        
        	fprintf(stderr,"\n[%d]      Stopped      %s\n", pid_top-1, proc_stack[pid_top-1]);
    	}
*/
        if (temp_pid!=0)
        {
        	pid_stack[pid_top] = temp_pid;
		   	strcpy(proc_stack[pid_top], parsed[temp_command].command);
		   	strcpy(proc_status[pid_top],"Stopped");
        	fprintf(stderr,"\n[%d]      Stopped      %s\n", pid_top, proc_stack[pid_top]);
		   	pid_top++;
        }
    	else
    	{
        fprintf(stderr,"\n%s", shell_prompt);
       	}	
    }
}

void command_loop(){
	int checker = 1;
	signal (SIGINT, sig_handler);
	signal (SIGTSTP, sig_handler);
	printed = 0;
	while(1)
	{	
		temp_pid = 0;
		prompt();
		if (!printed)
		{
			printf("%s", shell_prompt);
			printed = 0;
		}
		else
		{
			printed = 0;
		}

		int i;
		pipe_number = 0;
		for (i=1; i<pid_top; i++)
		{	
			int status;
			int joker = waitpid(pid_stack[i], &status, WNOHANG | WUNTRACED); 
			if (joker == -1)
				strcpy(proc_status[i],"Deleted");

		//	printf("Waitpid returned %d\n", joker);
			if ( joker > 0)
			{
				if (WIFSTOPPED(status))
				{
					strcpy(proc_status[i],"Stopped");
				    printf("[%d] Stopped %s[%d]\n", i, proc_stack[i], pid_stack[i]);
				    continue;
				}
				if (WIFEXITED(status) > 0) 
				{
					printf("%s with pid %d exited normally\n", proc_stack[i], pid_stack[i]);
					strcpy(proc_status[i],"Deleted");
				}
				else if(WIFSIGNALED(status))
				{
					printf("%s with pid %d exited with signal\n", proc_stack[i], pid_stack[i]);
					strcpy(proc_status[i],"Deleted");
				}
				else
					strcpy(proc_status[i],"Deleted");
			}
		}
		read_input();
		parse_input();		
		for (int p=0; p < current_command + 1; p++)
			io_decider(p);

		int l = current_command;
		current_command = 0;
		while (current_command < l + 1)
		{
			if (strcmp(parsed[current_command].command, ""))
			{
				temp_pid = 0;
				if (pipe_number > 0)
				{
					pipeexecute_input();
					current_command += pipe_number;
				}
				else
					execute_input();
			}
			free_command();
			current_command += 1;
		}
		current_command = 0;
		free(shell_prompt);
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
