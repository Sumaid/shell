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
#include <linux/rtc.h>
#include <sys/ioctl.h>

char *shell_pwd;
char shell_home[1024];
char mon[4];
char *shell_prompt;
char *input;
int main_pid;
int pid_stack[1024];
int pid_top;
typedef struct command_struct
{
	char *command;
	char *flags[1024];
	char *arguments[1024];
	int flags_index;
	int arguments_index;
} command_struct;

int current_command;
command_struct parsed[1024];
int commands_index; 

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
	if (parsed[current_command].arguments_index==0)
		return 0;
	else if (!strcmp(parsed[current_command].arguments[parsed[current_command].arguments_index-1], "&"))
		return 1;
	else
		return 0;
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



void shell_dir(){
	char temp[1024];
	if (getcwd(temp, sizeof(temp)) != NULL) {
		if (strlen(temp)<strlen(shell_home))
		{
   			shell_pwd = malloc(strlen(temp) + 1 + 1 );
   			strcpy(shell_pwd, temp);
   		}
		else if (!strcmp(temp,shell_home))
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
	if (parsed[current_command].arguments_index>1)
	{
		printf("%s: too many arguments\n", parsed[current_command].command);
		return;
	}
	if (parsed[current_command].arguments_index==0)
	{
		int erp = chdir(shell_home);
		shell_dir();
		if (!erp)
			return;
	}
	else if (parsed[current_command].arguments_index==1)
	{
		char *path;
   		path = (char *)malloc(1024);
		if (!strcmp(parsed[current_command].arguments[0], "~"))
			strcpy(path, shell_home);
		else
		{
			if (parsed[current_command].arguments[0][0]=='~')
			{
				strcpy(path, shell_home);
    			int i;
    			for (i=1; i<strlen(parsed[current_command].arguments[0]); i++)
    				path[strlen(path)] = parsed[current_command].arguments[0][i];	
			}
			else
				strcpy(path, parsed[current_command].arguments[0]);
		}
		int erp = chdir(path);
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
	for (i=0; i<parsed[current_command].arguments_index-1; i++)
		printf("%s ", parsed[current_command].arguments[i]);
	if (parsed[current_command].arguments_index>0)
		printf("%s\n", parsed[current_command].arguments[parsed[current_command].arguments_index-1]);
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
    for (i=0; i<parsed[current_command].flags_index; i++)
    {
    	if (!strcmp(parsed[current_command].flags[i], "l"))
    		l = 1;
    	else if (!strcmp(parsed[current_command].flags[i], "a"))
    		a = 1;
    	else if (!strcmp(parsed[current_command].flags[i], "al"))
    	{
    		a = 1;
    		l = 1;
    	}
    	else if (!strcmp(parsed[current_command].flags[i], "la"))
    	{
    		a = 1;
    		l = 1;
    	}
    }

    if (parsed[current_command].arguments_index==0)
    {
    	char cur[1024];
		if (getcwd(cur, sizeof(cur)) == NULL) 		
       		perror("getcwd() error");

    	if (l==1)
    		full_ls(cur, a);
    	else
        	regular_ls(cur, a);
    }
    else if (!strcmp(parsed[current_command].arguments[0], "~"))
    {
    	if (l==1)
    		full_ls(shell_home, a);
    	else
        	regular_ls(shell_home, a);
    }
    else
    {
    	int i = 0;
    	for (i=0; i<parsed[current_command].arguments_index; i++)
    	{

	
    		char *path;
    		path = (char *)malloc(1024);
    		char cur[1024];
			if (getcwd(cur, sizeof(cur)) == NULL) 		
       			perror("getcwd() error");
	
			if (!strcmp(parsed[current_command].arguments[i], "~"))
				strcpy(path, shell_home);
			else
			{
				if (parsed[current_command].arguments[i][0]=='~')
				{
					strcpy(path, shell_home);
	    			int j;
	    			for (j=1; j<strlen(parsed[current_command].arguments[i]); j++)
	    				path[strlen(path)] = parsed[current_command].arguments[i][j];	
				}
				else
				{
					strcpy(path, cur);
					strcpy(path, parsed[current_command].arguments[i]);
				}	
			}
    		struct stat path_stat;
			stat(path, &path_stat);
			if (S_ISDIR(path_stat.st_mode))                   //if ls is run on directory
			{
				if (isNOTDIR(path))
					continue;
				printf("%s:\n", parsed[current_command].arguments[i]);
				if (l==1)
    				full_ls(path, a);
    			else
        			regular_ls(path, a);
			}
			else
    		{
    			if (isFile(path))
    				if (l==1)
    					full_ls_file(path, a, 5);
    				else
    					printf("%s\n", path);          //if ls is run on file
				else
					printf("No such file or directory exists\n"); 
			}
		}	
    }
}

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


void remindme(){
	if (parsed[current_command].arguments_index!=2)
	{
		printf("Incorrect number of commands\nusage: remindme time \"message\"\n");
		exit(1);
	}

	clock_t t;
    t = clock();
    double time_expected = (double)atoi(parsed[current_command].arguments[0]);
    while (1)
    {
    	clock_t t1 = clock() - t;
    	double time_passed = ((double)t1)/CLOCKS_PER_SEC;
    	if (time_passed>time_expected)
    	{
	    	printf("\nReminder: %s\n", parsed[current_command].arguments[1]);
    		exit(0);
    	}
    }
}

void self_clock(){
	if (parsed[current_command].arguments_index!=2)
	{
		printf("Incorrect Number of arguments\n");
		printf("usage: clock -t [Delay] -n [Duration]\n");
	}
	int gap = atoi(parsed[current_command].arguments[0]);
	int duration = atoi(parsed[current_command].arguments[1]);
	int d_gap = 0;
	int time_duration = 0;
	while (1)
	{
		if (time_duration==duration)
			break;
		time_t *t;
		t = malloc(1024);
		*t = time(0);
		struct tm tm = *localtime(t);
		Mon(1+tm.tm_mon);
		if (d_gap==0)
			printf("%d %s %d, %02d:%02d:%02d\n", tm.tm_mday, mon, 1900+tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);	
		d_gap = (d_gap + 1)%gap;
		time_duration++;
		sleep(1);
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
	parsed[current_command].command = (char *)malloc(1024);
	int flag = 0;    //flag 0  means command, flag 1 means flags, flag 2 means argument
	int quote_flag = 0;
	int j = 0, k = 0, l = 0, i=0;
	while (i<strlen(input))
	{
		if (input[i]==';')
		{
		//	printf("Detected!!\n");
			flag = 0;
			j = 0, k = 0, l = 0, i++;
			quote_flag = 0;
			current_command++;
			parsed[current_command].command = (char *)malloc(1024);
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
		}		
		else 
		{
			if (is_hyphen(input[i+1]))
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
			else if ((!is_space(input[i+1]))&&(input[i+1]!=0))
			{
				flag = 2;
				l = 0;
				parsed[current_command].arguments[parsed[current_command].arguments_index++] = (char *)malloc(1024);
			}			
		}
		i += 1;
	}
}


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
//			printf("Parent Process\n");
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
//			printf("Child Process\n");
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
			{
				//printf("asdasd\n");
				ls();
				//if (!is_bk())
				//	return 1;
			}
			else
			{
				buf[k] = NULL;
					if (execvp(parsed[current_command].command, buf) < 0) {     
		                printf("*** ERROR: exec failed\n");
		                exit(1);
		        }
		    }
		    status = 1;
		    //bk_end();
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
}

void command_loop(){
	while(1)
	{
		prompt();
		int i;
		for (i=1; i<pid_top; i++)
		{
			int status;
			if (waitpid(pid_stack[i], &status, WNOHANG) > 0) 
			{
				printf("%s with pid %d exited normally\n", proc_stack[i], pid_stack[i]);
			}
		}
		printf("%s", shell_prompt);
		read_input();
		parse_input();
		int j = current_command;
		current_command = 0;
		while (current_command < j + 1)
		{
			//print_command();
			if (strcmp(parsed[current_command].command, ""))
			{
				execute_input();
				free_input();
			}
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
