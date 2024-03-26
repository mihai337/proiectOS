#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>


void parse(DIR *dirp, char* path ,int nr_tabs){
    
    struct stat *st = (struct stat*)malloc(sizeof(struct stat));
    struct dirent *r;
    r = readdir(dirp);
    if(r == NULL){
        return;
    }
    else{
        char *init_path = malloc((strlen(path)+3) * sizeof(char));
        strcat(path,"/");
        strcpy(init_path,path);
        path = realloc(path , (strlen(path) + strlen(r->d_name) + 1)*sizeof(char));
        strcat(path,r->d_name);

        int result = stat(path , st);
        for(int i=0;i<=nr_tabs;i++)
            printf("  ");
        printf("%s\n" ,r->d_name);
        if(S_ISDIR(st->st_mode) && strcmp(r->d_name,".") && strcmp(r->d_name,"..") ){
            DIR *new_dir = opendir(path);
            if(new_dir != NULL){
                parse(new_dir,path,nr_tabs+1);
                closedir(new_dir);
            }
        }
        parse(dirp,init_path,nr_tabs);
    }
}

int main(int argc , char **argv){
    // if(argc != 2){
    //     return 0;
    // }
    char *path = strdup("/home/mihai/Desktop/");
    DIR *dirp = opendir(path);

    if(dirp == NULL){
        perror("un merge");
        exit(1);
    }
    parse(dirp,path,0);
    closedir(dirp);
    return 0;
}