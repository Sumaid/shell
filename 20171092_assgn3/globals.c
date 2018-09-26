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

void swap(char *x, char *y) {
	char t = *x; *x = *y; *y = t;
}

char* reverse(char *buffer, int i, int j)
{
	while (i < j)
		swap(&buffer[i++], &buffer[j--]);

	return buffer;
}

char* itoa(int value, char* buffer, int base)
{
	if (base < 2 || base > 32)
		return buffer;
	int n = abs(value);

	int i = 0;
	while (n)
	{
		int r = n % base;

		if (r >= 10) 
			buffer[i++] = 65 + (r - 10);
		else
			buffer[i++] = 48 + r;

		n = n / base;
	}

	if (i == 0)
		buffer[i++] = '0';
	if (value < 0 && base == 10)
		buffer[i++] = '-';

	buffer[i] = '\0';
	return reverse(buffer, 0, i - 1);
}

int is_bk(){
	int index = current_command + pipe_number;
	if (parsed[index].arguments_index==0)
		return 0;
	else if (!strcmp(parsed[index].arguments[parsed[index].arguments_index-1], "&"))
		return 1;
	else
		return 0;
}

void print_command(int index){
	printf("Command Index: %d\n", index);
	printf("--- command is %s ---\n", parsed[index].command);
	printf("--- output fd is %d --- \n", parsed[index].o_fd);
	printf("--- input fd is %d --- \n", parsed[index].i_fd);
	printf("--- input file is %s\n", parsed[index].inputfile);
	printf("--- output file is %s\n", parsed[index].outputfile);


	int i,j;
	for (i=0; i<parsed[index].flags_index; i++)
		printf("--- flag is %s --- \n", parsed[index].flags[i]);
	for (j=0; j<parsed[index].arguments_index; j++)
		printf("--- argument is %s --- \n", parsed[index].arguments[j]);
}

int is_small(char c)
{
	return ((c<123)&&(96<c));
}

int is_large(char c)
{
	return ((c<91)&&(64<c));
}

int is_space(char c)
{
	return (c==32);
}

int is_tab(char c)
{
	return (c==9);
}

int is_hyphen(char c)
{
	return (c==45);
}

int is_quotation(char c)
{
	return ((c==34)||(c==39));
}

int digits(int num)
{
	int count = 0;
	while(num != 0)
    {
        count++;
        num /= 10;
    }
    return count;
}

int isNOTDIR(const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL)
    {
     closedir(directory);
     return 0;
    }

    if(errno == ENOTDIR)
    {
     return 1;
    }

    return -1;
}

int isFile(const char* file){
    struct stat fil;
    return (stat(file, &fil) == 0);
}

int isHiddenFile(char* file){
	return (file[0]==46);
}

void Mon(int num){
	switch (num)
	{
		case 1: strcpy(mon, "Jan");
				break;
		case 2: strcpy(mon, "Feb");
				break;
		case 3: strcpy(mon, "Mar");
				break;
		case 4: strcpy(mon, "Apr");
				break;
		case 5: strcpy(mon, "May");
				break;
		case 6: strcpy(mon, "Jun");
				break;
		case 7: strcpy(mon, "Jul");
				break;
		case 8: strcpy(mon, "Aug");
				break;
		case 9: strcpy(mon, "Sep");
				break;
		case 10: strcpy(mon, "Oct");
				break;
		case 11: strcpy(mon, "Nov");
				break;
		case 12: strcpy(mon, "Dec");
				break;
	}
}
