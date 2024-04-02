#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct Snapshot{
    char name[256];
    int size;
    char location[300];
    ino_t inode;
    time_t last_modified;
}file[1000];

void parse(DIR *dirp, char* path,int index){
    
    struct stat *st = (struct stat*)malloc(sizeof(struct stat));
    struct dirent *r;
    r = readdir(dirp);
    if(r == NULL){
        return;
    }
    else{
        int result = stat(path , st); //not the path of the file
        printf("%s\n" , path);
        file[index].inode = st->st_ino;
        strcpy(file[index].name , r->d_name);
        strcpy(file[index].location , path);
        strcat(file[index].location , "/");
        strcat(file[index].location , file[index].name);
        file[index].last_modified = st->st_mtime;
        // printf("%d\n"  ,st->st_size);
        file[index].size = st->st_size;

        if(S_ISDIR(st->st_mode) && strcmp(r->d_name,".") && strcmp(r->d_name,"..") ){
            char *new_path = malloc((strlen(path)+strlen(r->d_name)+2)*sizeof(char));
            sprintf(new_path,"%s%s/",path,r->d_name);
            DIR *new_dir = opendir(new_path);
            if(new_dir != NULL){
                parse(new_dir,path,index+1);
                closedir(new_dir);
            }
        }
        parse(dirp,path,index+1);
    }
}

int main(int argc , char **argv){
    if(argc != 2){
        return 0;
    }
    // char *path = strdup("/home/mihai/Desktop/");
    DIR *dirp = opendir(argv[1]);

    if(dirp == NULL){
        perror("nu merge");
        exit(1);
    }

    parse(dirp,argv[1],0);
    closedir(dirp);

    for(int i=0;i<5;i++){
        printf("%s  %d\n" , file[i].name , file[i].size);
    }

    return 0;
}
