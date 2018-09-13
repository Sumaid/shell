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


void regular_ls(char* path, int a){
    DIR * curdir = opendir(path);
    struct dirent *curfile;
    while((curfile = readdir(curdir)) != NULL)
    {
//      if (((!strcmp(curfile->d_name, "."))||(!strcmp(curfile->d_name, "..")))&&(!a))
        if ((isHiddenFile(curfile->d_name))&&(!a))
            continue;
        dprintf(parsed[current_command].o_fd, "%s   ", curfile->d_name);
    }
    dprintf(parsed[current_command].o_fd, "\n");
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
    dprintf(parsed[current_command].o_fd, "%s %2ld %s %s %*ld %s %d %d:%02d %s\n", permissions, fileStat.st_nlink, pwd->pw_name, grp->gr_name, max, fileStat.st_size, mon, tm.tm_mday, tm.tm_hour, tm.tm_min, file);
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
                dprintf(parsed[current_command].o_fd, "%s:\n", parsed[current_command].arguments[i]);
                if (l==1)
                    full_ls(path, a);
                else
                    regular_ls(path, a);
            }
            else
            {
                if (isFile(path))
                    if (l==1)
                    {
                        full_ls_file(path, a, 5);
                    }
                    else
                    {
                        dprintf(parsed[current_command].o_fd, "%s\n", path);          //if ls is run on file
                    }
                else
                {
                    perror("");
                    continue;
                }    
            }
        }   
    }
}