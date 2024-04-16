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

struct Snapshot{
    char name[256];
    int size;
    char location[300];
    ino_t inode;
    time_t last_modified;
}file[1000];


void parse(DIR *dirp,char *path,int level){
    struct dirent *dp;
    struct stat buf;
    char *new_path = (char *)malloc(5000);
    while((dp = readdir(dirp)) != NULL){
        if(strcmp(dp->d_name,".") == 0 || strcmp(dp->d_name,"..") == 0){
            continue;
        }
        strcpy(new_path,path);
        strcat(new_path,dp->d_name);
        stat(new_path,&buf);
        strcpy(file[level].name,dp->d_name);
        file[level].size = buf.st_size;
        strcpy(file[level].location,new_path);
        file[level].inode = buf.st_ino;
        file[level].last_modified = buf.st_mtime;
        level++;
        if(S_ISDIR(buf.st_mode)){
            strcat(new_path,"/");
            DIR *new_dir = opendir(new_path);
            parse(new_dir,new_path,level);
            closedir(new_dir);
        }
    }
}

// void parse(DIR *dirp, char *dirname, int file_count) {
//     struct dirent *entry;
//     struct stat statbuf;
//     char path[5000];

//     while ((entry = readdir(dirp)) != NULL) {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
//             continue;
        
//         snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

//         if (lstat(path, &statbuf) == -1) {
//             perror("lstat");
//             continue;
//         }

//         // Populate file_info struct with information about the file
//         strncpy(file[file_count].name, entry->d_name, sizeof(file[file_count].name));
//         file[file_count].size = statbuf.st_size;
//         strcpy(file[file_count].location, path);
//         file[file_count].inode = statbuf.st_ino;
//         file[file_count].last_modified = statbuf.st_mtime;

//         // Increment file count
//         file_count++;

//         // Process file or directory
//         if (S_ISDIR(statbuf.st_mode)) {
//             DIR *subdir = opendir(path);
//             if (subdir != NULL) {
//                 parse(subdir, path, file_count);
//                 closedir(subdir);
//             }
//         }
//     }
// }

void create_snapshot(){
    //copy child process
}

int main(int argc , char **argv){
    int i;
    if(argc < 2){
        return 0;
    }

    for(i=0;i<1000;i++){
        file[i].size = -1;
    }
    
    int child_index = 0;
    for(i=1;i<argc;i++){
        child_index++;
        pid_t pid = fork();
        if(pid == -1){
            perror("fork error");
            exit(1);
        }
        else if(pid == 0){
            //child process
            DIR *dirp = opendir(argv[i]);

            if(dirp == NULL){
                perror("opendir error");
                exit(1);
            }

            parse(dirp,argv[i],0);
            closedir(dirp);
            printf("%s\n",argv[i]);
            //save snapshot
            char* snap_location = (char *)malloc(5000);
            strcpy(snap_location,argv[i]);
            strcat(snap_location,"snapshot.txt");
            FILE *f = fopen(snap_location,"w");

            if(f == NULL){
                perror("fopen error");
                exit(1);
            }

            for(int j=0;j<1000;j++){
                // if(file[j].size == -1){
                //     break;
                // }
                fprintf(f,"%s %d %s %ld %ld\n",file[j].name,file[j].size,file[j].location,file[j].inode,file[j].last_modified);
            }
            fclose(f);
            free(snap_location);
            exit(0);
            // wait(NULL);
            }
    }

    for(i=1;i<argc;i++){
        wait(NULL);
    }
    

    return 0;
}
