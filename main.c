#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NEW 0
#define DELETED 1
#define MODIFIED 2
#define UNCHANGED 3

int safe_flag;
char* safe_path;

struct Snapshot{
    char name[256];
    int size;
    char location[300];
    ino_t inode;
    time_t last_modified;
    int tag;

}file[1000];


int check_permissions(char* path){
    struct stat sb;
    if(stat(path,&sb) == -1){
        perror("cannot check permissions");
        exit(1);
    }

    if((sb.st_mode & S_IRUSR) || (sb.st_mode & S_IWUSR) || (sb.st_mode & S_IXUSR) || (sb.st_mode & S_IRGRP) || (sb.st_mode & S_IWGRP) || (sb.st_mode & S_IXGRP) || (sb.st_mode & S_IROTH) || (sb.st_mode & S_IWOTH) || (sb.st_mode & S_IXOTH)){
        return 1;
    }
    return 0;
}


void traverse_directory(const char *path , int* count) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    
    if ((dir = opendir(path)) == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
    
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        
        if (stat(full_path, &statbuf) == -1) {
            perror("Error getting file status");
            continue;
        }

        if(safe_flag){
            if(check_permissions(full_path) == 0){

                pid_t pid = fork();
                if(pid == -1){
                    perror("fork error");
                    exit(1);
                }
                else{
                    if(pid == 0){
                        int result = execl("./verify_for_malicious.sh","./verify_for_malicious.sh",full_path , safe_path,NULL);
                        if(result != 0)
                            exit(1);
                        exit(0);
                    }
                    else{
                        int status;
                        wait(&status);
                        if(WIFEXITED(status)){
                            if(WEXITSTATUS(status) == 1){
                                continue;
                            }
                        }
                    }         
                }
            }   
        }

        strcpy(file[*count].name,entry->d_name);
        file[*count].size = statbuf.st_size;
        strcpy(file[*count].location,full_path);
        file[*count].inode = statbuf.st_ino;
        file[*count].last_modified = statbuf.st_mtime;
        (*count)++;

        if (S_ISDIR(statbuf.st_mode)){
            traverse_directory(full_path,count);
        }
        
    }

    closedir(dir);
}

int search_snapfile(char* path){
    char* snap_location = (char *)malloc(5000);
    strcpy(snap_location,path);
    strcat(snap_location,"/snapshot.txt");
    FILE *f = fopen(snap_location,"r");
    if(f == NULL){
        return 0;
    }
    fclose(f);
    free(snap_location);
    return 1;

}

void new_snapshot(char* path){
    char* snap_location = (char *)malloc(5000);
    strcpy(snap_location,path);
    strcat(snap_location,"/snapshot.txt");
    FILE *f = fopen(snap_location,"w");

    if(f == NULL){
        perror("fopen error");
        exit(1);
    }

    for(int j=0;j<1000;j++){
        if(file[j].size == -1){
            break;
        }
        fprintf(f,"%s %d %ld %ld %d\n",file[j].location,file[j].size,file[j].inode,file[j].last_modified , file[j].tag);
    }
    fclose(f);
    free(snap_location);
}

int snap_size(){
    int count = 0;
    for(int i=0;i<1000;i++){
        if(file[i].size == -1){
            break;
        }
        count++;
    }
    return count;

}

void compare_snapshot(char* path){
    char* snap_location = (char *)malloc(5000);
    strcpy(snap_location,path);
    strcat(snap_location,"/snapshot.txt");
    FILE *f = fopen(snap_location,"r");

    if(f == NULL){
        perror("fopen error");
        exit(1);
    }

    char location[300];
    int size;
    ino_t inode;
    time_t last_modified;
    int tag;
    int found = 0;

    while(fscanf(f,"%s %d %ld %ld %d",location,&size,&inode,&last_modified,&tag) != EOF){
        found = 0;

        if(tag == DELETED){
            continue;
        }

        for(int i=0;i<1000;i++){
            if(file[i].size == -1){
                break;
            }

            if(inode == file[i].inode){
                found = 1;
                if(file[i].last_modified != last_modified){
                    file[i].tag = MODIFIED;
                    // printf("%s %d %ld %ld %d\n",file[i].location,file[i].size,file[i].inode,file[i].last_modified , file[i].tag);
                }
                else{
                    file[i].tag = UNCHANGED;
                }
                break;
            }

        }

        if(!found){
            // fprintf(f,"%s %d %ld %ld %d\n",location,size,inode,last_modified , DELETED); //doesn't work, file opened as read only
            int s = snap_size();
            // strcpy(file[s].name,location);
            file[s].size = size;
            strcpy(file[s].location,location);
            file[s].inode = inode;
            file[s].last_modified = last_modified;
            file[s].tag = DELETED;
        }
    }
    new_snapshot(path);
    fclose(f);
    free(snap_location);
}

void child_process(char *path){

    int count=0;
    traverse_directory(path,&count);
    //save snapshot
    if(search_snapfile(path) == 0){
        new_snapshot(path);
    }
    else{
        compare_snapshot(path);
    }
    exit(0);
}

int main(int argc , char **argv){
    int i;
    
    if(argc < 2 || argc > 11){
        perror("invalid number of arguments, try between 1 and 10 arguments");
        return 0;
    }

    for(i=1;i<argc;i++){
        if(strcmp(argv[i],"-s") == 0){
            safe_flag = i;
            safe_path = argv[i+1];
            break;
        }
    }

    for(i=0;i<1000;i++){
        file[i].size = -1;
    }
    
    for(i=1;i<argc;i++){
        if(i == safe_flag){
            i++;
            continue;
        }

        pid_t pid = fork();
        if(pid == -1){
            perror("fork error");
            exit(1);
        }
        else{
            if(pid == 0){
                child_process(argv[i]);
            }         
        }
    }

    for(i=1;i<argc;i++){
        wait(NULL);
    }
    
    return 0;
}
