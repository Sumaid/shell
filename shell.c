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
#include "cd.h"
#include "pinfo.h"
#include "pwd.h"
#include "echo.h"
#include "ls.h"
#include "remindme.h"
#include "clock.h"
#include "globals.h"

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
int count = 1;
char proc_stack[1024][1024];
int current_command;
command_struct parsed[1024];
int commands_index; 
int redirect_flag;

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
}

void parse_input(){
	current_command = 0;
	parsed[current_command].command = (char *)malloc(1024);
	parsed[current_command].outputfile = (char *)malloc(1024);
	int flag = 0;    //flag 0  means command, flag 1 means flags, flag 2 means argument
	int quote_flag = 0;
	int output_redirect_flag = 0;
	int output_file_index = 0;
	int j = 0, k = 0, l = 0, i=0;
	while (i<strlen(input))
	{
		if (input[i]==';')
		{
			flag = 0;
			j = 0, k = 0, l = 0, i++;
			quote_flag = 0;
			output_file_index = 0;
			output_redirect_flag = 0;
			current_command++;
			parsed[current_command].command = (char *)malloc(1024);
			parsed[current_command].outputfile = (char *)malloc(1024);
			continue;
		}
		if ((input[i]=='>')&&(input[i+1]!='>'))
		{
			redirect_flag = 1;
			flag = 3;
			parsed[current_command].outputfile = (char *)malloc(1024);
			output_file_index = 0;
			i += 1;
			continue;
		}
		if ((input[i]=='>')&&(input[i+1]=='>'))
		{
			redirect_flag = 2;
			flag = 3;
			parsed[current_command].outputfile = (char *)malloc(1024);
			output_file_index = 0;
			i += 2;
			continue;
		}
		if (is_quotation(input[i])&&flag==3)
		{
			i += 1;
			quote_flag = 1;
			continue;
		}
		if (((is_space(input[i]))||(is_tab(input[i])))&&((output_file_index==0)&(flag==3)))
		{
			i += 1;
			continue;
		}
		if (((is_space(input[i]))||(is_tab(input[i])))&&(j==0))
		{
			i += 1;
			continue;
		}
		if ((!is_space(input[i]))||((is_space(input[i]))&&quote_flag))
		{
			if (flag==0)
				parsed[current_command].command[j++] = input[i];
			else if (flag==1)
				parsed[current_command].flags[parsed[current_command].flags_index-1][k++] = input[i];
			else if (flag==2)
			{
				if (is_quotation(input[i]))
					quote_flag = 0; 
				else
					parsed[current_command].arguments[parsed[current_command].arguments_index-1][l++] = input[i];
			}
			else if (flag==3)
			{
				if (is_quotation(input[i]))
					quote_flag = 0; 
				else
					parsed[current_command].outputfile[output_file_index++] = input[i];
			}
		}		
		else 
		{
			if ((input[i+1]=='>')&&(input[i+2]!='>'))
			{
				redirect_flag = 1;
				flag = 3;
				parsed[current_command].outputfile = (char *)malloc(1024);
				output_file_index = 0;
				i += 1;
			}
			else if ((input[i+1]=='>')&&(input[i+2]=='>'))
			{
				redirect_flag = 2;
				flag = 3;
				parsed[current_command].outputfile = (char *)malloc(1024);
				output_file_index = 0;
				i += 2;
			}
			else if (is_hyphen(input[i+1]))
			{
				flag = 1;
				k = 0;
				parsed[current_command].flags[parsed[current_command].flags_index++] = (char *)malloc(1024);
				i += 1;
			}
			else if (is_quotation(input[i+1]))
			{
				quote_flag = 1;
				flag = 2;
				l = 0;
				parsed[current_command].arguments[parsed[current_command].arguments_index++] = (char *)malloc(1024);
				i += 1;
			}
			else if ((!is_space(input[i+1]))&&(input[i+1]!=0)&&(flag!=3))
			{
				flag = 2;
				l = 0;
				parsed[current_command].arguments[parsed[current_command].arguments_index++] = (char *)malloc(1024);
			}			
		}
		i += 1;
	}
}

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
				dup2(parsed[current_command].o_fd, 1);
				buf[k] = NULL;
					if (execvp(parsed[current_command].command, buf) < 0) {     
		                printf("*** ERROR: exec failed\n");
		                exit(1);
				}
				close(1);
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

void free_input(){
	parsed[current_command].flags_index = 0;
	parsed[current_command].arguments_index = 0;
	if (parsed[current_command].o_fd!=1)
		close(parsed[current_command].o_fd);
	parsed[current_command].o_fd  = 1;
	parsed[current_command].i_fd  = 0;
}

void io_decider(){
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
    	free(parsed[current_command].outputfile);
    }

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
		io_decider();
		print_command();
		int j = current_command;
		current_command = 0;
		while (current_command < j + 1)
		{
			//print_command();
			//printf("current_command is %d\n", current_command);
			if (strcmp(parsed[current_command].command, ""))
			{
				execute_input();
				free_input();
			}
			current_command += 1;
		}
		current_command = 0;
		free(shell_prompt);
		checker += 1;
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