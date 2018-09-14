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

int pipe_number = 0;
int pipes[1024];
void parse_input(){
	current_command = 0;
	parsed[current_command].command = (char *)malloc(1024);
	parsed[current_command].outputfile = (char *)malloc(1024);
	parsed[current_command].inputfile = (char *)malloc(1024);
	int flag = 0;    //flag 0  means command, flag 1 means flags, flag 2 means argument
	int quote_flag = 0;
	int output_redirect_flag = 0;
	int output_file_index = 0;
	int input_file_index = 0;
	int j = 0, k = 0, l = 0, i=0;
	while (i<strlen(input))
	{
		if (i==0)
      	{
      		parsed[current_command].i_fd = 0;
      		parsed[current_command].o_fd = 1;
      	}	

		if (i==strlen(input)-1)
      	{
      		//parsed[current_command].i_fd = 0;
      		parsed[current_command].o_fd = 1;
      	}

		if (input[i]=='|')
		{
			flag = 0;
			j = 0, k = 0, l = 0, i++;
			quote_flag = 0;
			output_file_index = 0;
			input_file_index = 0;
			output_redirect_flag = 0;
			int fildes[2];
			if(pipe(fildes) != 0)
      			perror("pipe failed");
      		pipes[pipe_number*2] = fildes[0];
      		pipes[pipe_number*2+1] = fildes[1];
      		pipe_number++;			
      		parsed[current_command].o_fd = fildes[1];
			current_command++;
      		parsed[current_command].i_fd = fildes[0];
			parsed[current_command].command = (char *)malloc(1024);
			parsed[current_command].outputfile = (char *)malloc(1024);
			parsed[current_command].inputfile = (char *)malloc(1024);
		}
		if (input[i]==';')
		{
			pipe_number=0;
			flag = 0;
			j = 0, k = 0, l = 0, i++;
			quote_flag = 0;
			output_file_index = 0;
			input_file_index = 0;
			output_redirect_flag = 0;
			current_command++;
			parsed[current_command].command = (char *)malloc(1024);
			parsed[current_command].outputfile = (char *)malloc(1024);
			parsed[current_command].inputfile = (char *)malloc(1024);
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
		if (input[i] == '<')
		{
			flag = 4;
			parsed[current_command].inputfile = (char *)malloc(1024);
			input_file_index = 0;
			i += 1;
			continue;
		}
		if (is_quotation(input[i])&&flag>2)
		{
			i += 1;
			quote_flag = 1;
			continue;
		}
		if (((is_space(input[i]))||(is_tab(input[i])))&&(((output_file_index==0)||(input_file_index==0))&&(flag>2)))
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
			else if (flag==4)
			{
				if (is_quotation(input[i]))
					quote_flag = 0; 
				else
					parsed[current_command].inputfile[input_file_index++] = input[i];
			}

		}		
		else 
		{
			if (input[i+1]=='|')
			{
				flag = 0;
				j = 0, k = 0, l = 0, i++;
				quote_flag = 0;
				output_file_index = 0;
				input_file_index = 0;
				output_redirect_flag = 0;
				int fildes[2];
				if(pipe(fildes) != 0)
	      			perror("pipe failed");
	      		pipes[pipe_number*2] = fildes[0];
	      		pipes[pipe_number*2+1] = fildes[1];
    	  		pipe_number++;				
	      		parsed[current_command].o_fd = fildes[1];
				current_command++;
	      		parsed[current_command].i_fd = fildes[0];
				parsed[current_command].command = (char *)malloc(1024);
				parsed[current_command].outputfile = (char *)malloc(1024);
				parsed[current_command].inputfile = (char *)malloc(1024);
			}
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
			else if (input[i+1]=='<')
			{
				flag = 4;
				parsed[current_command].inputfile = (char *)malloc(1024);
				input_file_index = 0;
				i += 1;
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
			else if ((!is_space(input[i+1]))&&(input[i+1]!=0)&&(flag!=3)&&(flag!=4))
			{
				flag = 2;
				l = 0;
				parsed[current_command].arguments[parsed[current_command].arguments_index++] = (char *)malloc(1024);
			}			
		}
		i += 1;
	}
}