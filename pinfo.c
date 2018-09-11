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


char *state;
int vm_result;

void proc_info(){ 
	char *filelink;
	filelink = (char *)malloc(1024);
	strcat(filelink, "/proc/");
	char pid_string[1024];
	//printf("%s\n", parsed[current_command].arguments[0]);
	if (parsed[current_command].arguments_index!=0)
		strcat(filelink, parsed[current_command].arguments[0]);
	else
	{
		char pid_string[1024];
		itoa(main_pid, pid_string, 10);
		strcat(filelink, pid_string);
	}

	strcat(filelink, "/status");
    FILE* file = fopen(filelink, "r");
    state = (char *)malloc(20);
    state[0] = '\0';
    vm_result = 0;
    char line[1024];
    while (fgets(line, 1024, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            int i = strlen(line);
		    char* p = line;
		    while (*p <'0' || *p > '9') p++;
		    line[i-3] = '\0';
		    i = atoi(p);
            vm_result = i;
        }
        else if (strncmp(line, "State:", 6) == 0){
        	char* q = line;
        	q = q + 6;
        	int g = 0;
        	while (!is_large(*q)) 
        		q++;
        	while (*q != '\n')
        	{	
        		state[g++] = *q;
        		q++;
        	}
        	state[g] = '\0';
        }
        
    }
    fclose(file);
    return;
}

void pinfo(){
	if (parsed[current_command].arguments_index==0)
		printf("pid -- %d\n", main_pid);
	else
	{
		if (parsed[current_command].arguments_index>1)
		{
			printf("too many arguments\n");
			return;
		}
		char *filelink;
		filelink = (char *)malloc(1024);
		strcat(filelink, "/proc/");
		strcat(filelink, parsed[current_command].arguments[0]);
		struct stat sts;
		if (stat(filelink, &sts) == -1 && errno == ENOENT) {
			printf("There's no process with given pid\n");
			return;
		}
		printf("pid -- %s\n", parsed[current_command].arguments[0]);
	}
	proc_info();
	printf("Process Status -- %s memory\n", state);
	printf("- %d{Virtual Memory}\n", vm_result);
	printf("Executable Path --");
	char *link;
	link = (char *)malloc(1024);
	//link[0] = '\0';
	strcpy(link, "/proc/");
	//printf("%s\n", parsed[current_command].arguments[0]);
	if (parsed[current_command].arguments_index!=0)
	{
		strcat(link, parsed[current_command].arguments[0]);
	}
	else
	{
		char pid_string[1024];
		itoa(main_pid, pid_string, 10);
		strcat(link, pid_string);
	}
	strcat(link, "/exe");
	int i;
	char *buf;
	buf = (char *)malloc(1024);
	int bufsize = 1024;
	for (i=0; i<bufsize; i++) buf[i] = '\0';
	readlink(link, buf, bufsize);
	printf("%s", buf);
	//printf("%s\n", buf);
    printf("\n");
}
