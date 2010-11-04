#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

short countfiles();
int file_exist (char *filename);
int getFileList(char *path);
int deleteContent(char *filename);
char *readWholeFile(char *,int *);
int create_content(char *filename,char *content,int content_len);
