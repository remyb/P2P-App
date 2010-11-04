#include "filehandler.h"

/** Count files in directory **/
short countfiles() {
    char systemcall[100];
    sprintf(systemcall,"ls ./content | wc -l");
    printf("%s\n",systemcall);
    FILE *fp;
    fp = popen(systemcall,"r");
    if (fp == NULL)
    {
        printf("fp failed\n");
        abort();
    }
    int k;
    fscanf(fp, "%d", &k);
    printf("there are %d files\n",k);  // comment out
    pclose(fp);
    return k;
}

/** check to see if file exists **/
int file_exist (char *filename)
{
  struct stat buffer;   
  return (stat (filename, &buffer) == 0);
}

/** print out list of files **/
int getFileList(char *path) {
    DIR *dp;
    struct dirent *ep;
    dp = opendir (path);
    if (dp != NULL)
    {
        while (ep = readdir(dp))
        puts(ep->d_name);  // output, prints to screen here
            (void) closedir(dp);
    }
    else
        perror ("Couldn't open the directory");
    return 0;
}

/** delete item in directory **/
int deleteContent(char *filename){
    if (remove(filename) == -1) {
        printf("file not found for removal\n");
        return 0;
    } 
    return 1;
}

/** testing **/
/*
int main () {
    
    printf("\n\n==Testing if File Exists==\n\n");
    if(file_exist("stamp-h1a")) 
        printf("file exists\n");
    else
        printf("file does NOT exist\n");
    
    printf("\n\n===testing listing contents of directory==\n\n");
    printf("%d\n", getFileList("/tmp"));
    
    printf("\n\n===counting # of files of directory==\n\n");
    printf("Number of Files in Directory: %d\n\n", countfiles());
    
    printf("\n\n===Deleting file===\n\n");
    if (deleteContent("temp.txt")) printf("file was successfully deleted...\n");
    else printf("file count not be deleted....\n");
    printf("done...\n\n");
}
*/

