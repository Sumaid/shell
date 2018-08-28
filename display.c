#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
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

char *shell_pwd;
char shell_home[1024];
char mon[4];
char *shell_prompt;
char *input;

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

typedef struct command_struct
{
	char *command;
	char *flags[1024];
	char *arguments[1024];
	int flags_index;
	int arguments_index;
} command_struct;

command_struct parsed; 

void shell_dir(){
	char temp[1024];
	if (getcwd(temp, sizeof(temp)) != NULL) {
		if (!strcmp(temp,shell_home))
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
   	strcat(shell_prompt, "<");
   	strcat(shell_prompt, hostname);
   	strcat(shell_prompt, "@");
   	strcat(shell_prompt, unameData.sysname);
   	strcat(shell_prompt, ":");
   	strcat(shell_prompt, shell_pwd);
   	strcat(shell_prompt, ">");
}

void cd(){
	if (parsed.arguments_index>1)
	{
		printf("%s: too many arguments\n", parsed.command);
		return;
	}
	if (parsed.arguments_index==0)
	{
		int erp = chdir(shell_home);
		shell_dir();
		if (!erp)
			return;
	}
	else if (parsed.arguments_index==1)
	{
		int erp = chdir(parsed.arguments[0]);
		shell_dir();
		if (!erp)
			return;
	}
	
	if (errno!=0)
		printf("%s\n", strerror(errno));
}

void pwd(){
	char cur[1024];
	if (getcwd(cur, sizeof(cur)) != NULL) 		
	   printf("%s\n", cur);
   	else 
       perror("getcwd() error");
}

void echo(){
	int i;
	for (i=0; i<parsed.arguments_index-1; i++)
		printf("%s ", parsed.arguments[i]);
	if (parsed.arguments_index>0)
		printf("%s\n", parsed.arguments[parsed.arguments_index-1]);
	else
		printf("\n");
}

void regular_ls(char* path, int a){
	DIR * curdir = opendir(path);
    struct dirent *curfile;
	while((curfile = readdir(curdir)) != NULL)
	{
//		if (((!strcmp(curfile->d_name, "."))||(!strcmp(curfile->d_name, "..")))&&(!a))
		if ((isHiddenFile(curfile->d_name))&&(!a))
			continue;
		printf("%s   ", curfile->d_name);
	}
	printf("\n");
}


void full_ls_file(char* file, int a, int max){
	struct stat fileStat;
	if(stat(file,&fileStat) < 0)    
    	return;
    char permissions[10] = "";
    strcat(permissions, ( (S_ISDIR(fileStat.st_mode)) ? "d" : "-"));
    strcat(permissions, ( (fileStat.st_mode & S_IRUSR) ? "r" : "-"));
    strcat(permissions, ( (fileStat.st_mode & S_IWUSR) ? "w" : "-"));
    strcat(permissions, ( (fileStat.st_mode & S_IXUSR) ? "x" : "-"));
    strcat(permissions, ( (fileStat.st_mode & S_IRGRP) ? "r" : "-"));
    strcat(permissions, ( (fileStat.st_mode & S_IWGRP) ? "w" : "-"));
    strcat(permissions, ( (fileStat.st_mode & S_IXGRP) ? "x" : "-"));
    strcat(permissions, ( (fileStat.st_mode & S_IROTH) ? "r" : "-"));
    strcat(permissions, ( (fileStat.st_mode & S_IWOTH) ? "w" : "-"));
    strcat(permissions, ( (fileStat.st_mode & S_IXOTH) ? "x" : "-"));
    
    struct group *grp;
	struct passwd *pwd;
	grp = getgrgid(fileStat.st_gid);
	pwd = getpwuid(fileStat.st_uid);
	time_t *t;
	t = malloc(1024);
	*t = fileStat.st_mtime;
	struct tm tm = *localtime(t);
	Mon(tm.tm_mon+1);
	printf("%s %2ld %s %s %*ld %s %d %d:%02d %s\n", permissions, fileStat.st_nlink, pwd->pw_name, grp->gr_name, max, fileStat.st_size, mon, tm.tm_mday, tm.tm_hour, tm.tm_min, file);
}

void full_ls(char* path, int a){
	DIR * curdir1 = opendir(path);
    struct dirent *curfile;
    struct dirent *curfile1;
	int max = 0;
	while((curfile1 = readdir(curdir1)) != NULL)
	{
    	struct stat fileStat1;
    	if(stat(curfile1->d_name,&fileStat1) < 0)    
        	return;
        if (digits(fileStat1.st_size)>max)
        	max = digits(fileStat1.st_size);
	}
	DIR * curdir = opendir(path);
	while((curfile = readdir(curdir)) != NULL)
	{
		if ((isHiddenFile(curfile->d_name))&&(!a))
			continue;
		full_ls_file(curfile->d_name, a, max);
	}
}

void ls(){
    struct stat curstat;
    int l = 0;
    int a = 0;
    int i;
    for (i=0; i<parsed.flags_index; i++)
    {
    	if (!strcmp(parsed.flags[i], "l"))
    		l = 1;
    	else if (!strcmp(parsed.flags[i], "a"))
    		a = 1;
    	else if (!strcmp(parsed.flags[i], "al"))
    	{
    		a = 1;
    		l = 1;
    	}
    	else if (!strcmp(parsed.flags[i], "la"))
    	{
    		a = 1;
    		l = 1;
    	}
    }

    if (parsed.arguments_index==0)
    {
    	if (l==1)
    		full_ls(shell_home, a);
    	else
        	regular_ls(shell_home, a);
    }

    else
    {
    	int i = 0;
    	for (i=0; i<parsed.arguments_index; i++)
    	{
    		char *path;
    		path = (char *)malloc(1024);
    		strcat(path, parsed.arguments[i]);	
    		
    		struct stat path_stat;
			stat(path, &path_stat);
			if (S_ISDIR(path_stat.st_mode))                   //if ls is run on directory
			{
				if (isNOTDIR(path))
					continue;
				printf("%s:\n", parsed.arguments[i]);
				if (l==1)
    				full_ls(path, a);
    			else
        			regular_ls(path, a);
			}
			else
    		{
    			if (isFile(parsed.arguments[i]))
    				if (l==1)
    					full_ls_file(parsed.arguments[i], a, 5);
    				else
    					printf("%s\n", parsed.arguments[i]);          //if ls is run on file
				else
					printf("No such file or directory exists\n"); 
			}
		}	
    }
}


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
	parsed.command = (char *)malloc(1024);
	int flag = 0;    //flag 0  means command, flag 1 means flags, flag 2 means argument
	int quote_flag = 0;
	int j = 0, k = 0, l = 0, i=0;
	while (i<strlen(input))
	{
		if ((is_small(input[i])||is_large(input[i])||quote_flag)||(input[i]==46))
		{
			if (flag==0)
				parsed.command[j++] = input[i];
			else if (flag==1)
				parsed.flags[parsed.flags_index-1][k++] = input[i];
			else if (flag==2)
				{
					if (is_quotation(input[i]))
						quote_flag = 0; 
					else
						parsed.arguments[parsed.arguments_index-1][l++] = input[i];
				}
		}		
		else if ((is_space(input[i]))&&(j!=0))
		{
			if (is_hyphen(input[i+1]))
			{
				flag = 1;
				k = 0;
				parsed.flags[parsed.flags_index++] = (char *)malloc(1024);
				i += 1;
			}
			else if (is_quotation(input[i+1]))
			{
				quote_flag = 1;
				flag = 2;
				l = 0;
				parsed.arguments[parsed.arguments_index++] = (char *)malloc(1024);
				i += 1;
			}
			else if (!is_space(input[i+1]))
			{
				flag = 2;
				l = 0;
				parsed.arguments[parsed.arguments_index++] = (char *)malloc(1024);
			}			
		}
		i += 1;
	}
}

void execute_input(){
	if (!strcmp(parsed.command,"cd"))
		cd();
	else if (!strcmp(parsed.command,"pwd"))
		pwd();
	else if (!strcmp(parsed.command,"echo"))
		echo();
	else if (!strcmp(parsed.command,"ls"))
		ls();
	else if(!strcmp(parsed.command,"exit"))
	    exit(0);
	else
		system(input);
}

void free_input(){
	free(parsed.command);
	int i,j;
	parsed.flags_index = 0;
	parsed.arguments_index = 0;
}

void print_command(){
	printf("command is %s\n", parsed.command);
	int i,j;
	for (i=0; i<parsed.flags_index; i++)
		printf("flag is %s\n", parsed.flags[i]);
	for (j=0; j<parsed.arguments_index; j++)
		printf("argument is %s\n", parsed.arguments[j]);
}

void command_loop(){
	while(1)
	{
		prompt();
		printf("%s", shell_prompt);
		read_input();
		parse_input();
		pid_t pid = fork();
		if (pid>0)
		{
			if (!strcmp(arguments[arguments_index-1], "&"))
				continue;
			else 
				waitpid(pid, 0, 0);
		}
		else if (pid==0)

		else
			printf("fork error\n");	
		execute_input();
		free_input();
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

//Running Loop which checks for command entered   	
   	command_loop();
	return 0;
}
