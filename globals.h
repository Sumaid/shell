#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

extern	char *shell_pwd;
extern	char shell_home[1024];
extern	char mon[4];
extern	char *shell_prompt;
extern	char *input;
extern	int main_pid;
extern	int pid_stack[1024];
extern	int pid_top;
typedef struct command_struct
	{
		char *command;
		char *flags[1024];
		char *arguments[1024];
		int flags_index;
		int arguments_index;
	} command_struct;

extern	int current_command;
extern	command_struct parsed[1024];
extern	int commands_index; 

void swap(char *x, char *y);
char* reverse(char *buffer, int i, int j);
char* itoa(int value, char* buffer, int base);
int is_bk();
void print_command();
int is_small(char c);
int is_large(char c);
int is_space(char c);
int is_tab(char c);
int is_hyphen(char c);
int is_quotation(char c);
int digits(int num);
int isNOTDIR(const char* name);
int isFile(const char* file);
int isHiddenFile(char* file);
void Mon(int num);

#endif