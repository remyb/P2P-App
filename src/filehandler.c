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