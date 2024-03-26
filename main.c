#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

void parse(DIR *dirp){
    struct dirent *r;
    struct stat *st;
    r = readdir(dirp);
    if(r == NULL){
        return;
    }
    else{
        printf("%s" , r->d_name);
        // stat(r->d_ino,st);
    }
}

int main(int argc , char **argv){
    if(argc != 2){
        return 0;
    }

    DIR *dirp = opendir(argv[1]);

    if(dirp == NULL){
        perror("un merge");
        exit(1);
    }
    parse(dirp);
    return 0;
}