#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <dirent.h>
//#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
//#include <sys/types.h>
#include <pwd.h>
//#define MAX_FILENAME_LENGTH 1000
//#define MAX_COMMIT_MESSAGE_LENGTH 2000
//#define MAX_LINE_LENGTH 1000
//#define MAX_MESSAGE_LENGTH 1000
//#define debug(x) printf("%s", x);
char cwd[1024];
char tnt_path[1024];
char time_now[100];
//void print_command(int argc, char * const argv[]);
int run_init(int argc, char * const argv[]);
int create_configs();
int run_config(int argc, char * const argv[]);
int run_add(int argc, char * const argv[]);
int file_or_directory(char * path);
int add_to_staging(char * path);
void copyFile(const char *sourcePath, const char *destinationPath);
int check_wildcard(char * wildcard, char  * word);
//int run_reset(int argc, char * const argv[]);
//int remove_from_staging(char *filepath);
//int run_commit(int argc, char * const argv[]);
//int inc_last_commit_ID();
//bool check_file_directory_exists(char *filepath);
//int commit_staged_file(int commit_ID, char *filepath);
//int track_file(char *filepath);
//bool is_tracked(char *filepath);
//int create_commit_file(int commit_ID, char *message);
//int find_file_last_commit(char* filepath);
//int run_checkout(int argc, char *const argv[]);
//int find_file_last_change_before_commit(char *filepath, int commit_ID);
//int checkout_file(char *filepath, int commit_ID);
int find_tnt(){
    char tmp_cwd[1024];
    struct dirent *entry;
    do {
        // find .tnt
        DIR *dir = opendir(".");
        if (dir == NULL) {
            perror("Error opening current directory");
            return 1;
        }
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".tnt") == 0){
                getcwd(tnt_path, sizeof(tnt_path));
                strcat(tnt_path,"/.tnt");
            }
        }
        closedir(dir);
        if (getcwd(tmp_cwd, sizeof(tmp_cwd)) == NULL) return 1;     // update current working directory
        if (strcmp(tmp_cwd, "/") != 0) {        // change cwd to parent
            if (chdir("..") != 0) return 1;
        }
    } while (strcmp(tmp_cwd, "/") != 0);
    if (chdir(cwd) != 0) return 1;  // return to the initial cwd
    return 0;
}
int run_init(int argc, char * const argv[]) {
    if(find_tnt()) return 1;
    if (tnt_path[0] == 0) {
        if (mkdir(".tnt", 0755) != 0) return 1;
        if (chdir(".tnt") != 0) return 1;
        getcwd(tnt_path, sizeof(tnt_path));
        return create_configs();
    } else perror("tnt repository has already initialized");
    return 0;
}
int create_configs(){
    char space[40];
    strcpy(space,"                                   ");
    FILE *file;
    struct passwd *pw = getpwuid(getuid());
    if (chdir(pw->pw_dir) != 0) return 1;   //go to user folder
    if(access(".tnt_global_configs", F_OK) == -1){   //make global configs' place
        file = fopen(".tnt_global_configs", "w");
        fprintf(file,"user name:%s\ndate and time:%s\nuser email:%s\ndate and time:%s\nalias:\n",space,space,space,space);
        fclose(file);
    }
    if (chdir(tnt_path) != 0) return 1;
    file = fopen("tracks", "w");
    fclose(file);
    file = fopen("config", "w");
    fprintf(file,"user name:%s\ndate and time:%s\nuser email:%s\ndate and time:%s\n",space,space,space,space);
    fprintf(file,"last commit ID:%s\ncurrent commit ID:%s\nbranch:master%s\nalias:\n",space,space,space);
    fclose(file);
    file = fopen("branches", "w");
    fprintf(file,"master-0-");
    fclose(file);
    if (mkdir("COMMIT", 0755) != 0) return 1;
    if (mkdir("STAGE", 0755) != 0) return 1;
    if (chdir("STAGE") != 0) return 1;
    file = fopen("history", "w");
    fclose(file);
    if (chdir("..") != 0) return 1;
    if (chdir("COMMIT") != 0) return 1;
    file = fopen("commits", "w");
    fclose(file);
    return  0;
}
int run_config(int argc, char * const argv[]){
    char space[40];
    int m=0;
    strcpy(space,"                                   ");
    FILE *file;
    char buffer[512];
    char data[256];
    if(argc < 3 ){
        fprintf(stdout, "please enter a valid command\n");
        return 1;
    } if(strcmp(argv[2], "-global") == 0) {
        m = 1;
        struct passwd *pw = getpwuid(getuid());
        if (chdir(pw->pw_dir) != 0) return 1;
        if (access(".tnt_global_configs", F_OK) == -1) {   //make global configs' place
            file = fopen(".tnt_global_configs", "w");
            fprintf(file, "user name:%s\ndate and time:%s\nuser email:%s\ndate and time:%s\nalias:\n", space, space,
                    space, space);
            fclose(file);
        }
    }else {
        if (find_tnt() == 1 || tnt_path[0] == 0) {
            fprintf(stdout, "There is not any repository\n");
            return 1;
        }
        if (chdir(tnt_path) != 0) return 1;
    }
    if(argc < 3+m ){
        fprintf(stdout, "please enter a valid command\n");
        return 1;
    }
    if(strcmp(argv[2+m], "user.name") == 0){
        if(m) file = fopen(".tnt_global_configs","r+");
        else if((file = fopen("config","r+")) == NULL) return 1;
        if(argc < 4+m ){
            fprintf(stdout, "please enter a valid command\n");
            return 1;
        }if(strlen(argv[3+m])>30 || argc > 4+m){
            fprintf(stdout, "Your username can has a maximum of 30 characters without space\n");
            return 1;
        }
        sprintf(data, "user name:%s ", argv[3+m]);
        fputs( data, file);
        if (fgets(buffer, sizeof(buffer), file) == NULL) return 1;
        sprintf(data, "date and time:%s ", time_now);
        fputs( data, file);
        fclose(file);
    }else if(strcmp(argv[2+m], "user.email") == 0) {
        if(m) file = fopen(".tnt_global_configs","r+");
        else if((file = fopen("config","r+")) == NULL) return 1;
        if (argc < 4+m) {
            fprintf(stdout, "please enter a valid command\n");
            return 1;
        }if (strlen(argv[3+m]) > 30 || argc > 4+m) {
            fprintf(stdout, "Your email can has a maximum of 30 characters without space\n");
            return 1;
        }
        if (fgets(buffer, sizeof(buffer), file) == NULL) return 1;
        if (fgets(buffer, sizeof(buffer), file) == NULL) return 1;
        sprintf(data, "user email:%s ", argv[3+m]);
        fputs(data, file);
        if (fgets(buffer, sizeof(buffer), file) == NULL) return 1;
        sprintf(data, "date and time:%s ", time_now);
        fputs(data, file);
        fclose(file);
    }else{
        fprintf(stdout, "please enter a valid command\n");
        return 1;
    }
    return  0;
}
int file_or_directory(char * path){
    struct stat file_info;
    if (lstat(path, &file_info) == -1) {
        printf("Error while getting %s information\n",path);
        return 0;
    }
    if (S_ISREG(file_info.st_mode)) {   //file
        return 1;
    } else if (S_ISDIR(file_info.st_mode)) {  //directory
        return 2;
    }else {
        printf("Input is neither a regular file nor a directory.\n");
        return 0;
    }
}
int run_add(int argc, char * const argv[]){
    char path[1024];
    if(argc < 3 ) {
        fprintf(stdout, "please enter a valid command\n");
        return 1;
    }if(strcmp(argv[2], "-redo") == 0){
        FILE *history;
        char add_data[2048];
        sprintf(add_data,"%s/STAGE/history", tnt_path);
        history= fopen(add_data,"r+");
        char line[512];
        while(fgets(line, sizeof(line), history) != NULL){
            if(strstr(line, "STAGE=0") != NULL) fputs("STAGE=1\n", history);
        }
        return 0;
    }else if(strcmp(argv[2], "-f")==0 || argv[2][0] != '-'){
        int m=3;
        if(argv[2][0] != '-') m=2;
        for(int i=m ; i<argc; i++){
            int check = file_or_directory(argv[i]);
            if(!check){
                if(i==argc-1) return 1;
            }
            if(check == 1) return add_to_staging(argv[i]);
            else if(check == 2){
                DIR *dir = opendir(argv[i]);
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_type != DT_DIR){
                        sprintf(path,"%s/%s",argv[i],entry->d_name);
                        add_to_staging(path);
                    }
                }
            }
        }
    }
    return 0;
}
int add_to_staging(char *path){
    int ID=0;
    FILE *history;
    char add_data[2048];
    sprintf(add_data,"%s/STAGE/history", tnt_path);
    history= fopen(add_data,"r+");
    char line[512];
    while(fgets(line, sizeof(line), history) != NULL){
        if(strstr(line, path) != NULL) fputs("STAGE=2\n", history);
        if(strstr(line, "ID=") != NULL) sscanf(line ,"ID=%d", &ID);
    }
    fclose(history);
    char sourcePath[512];
    strcpy(sourcePath,path);
    char destinationPath[2048];
    sprintf(destinationPath,"%s/STAGE/%d", tnt_path, ID+1);
    copyFile(sourcePath, destinationPath);
    history= fopen(add_data,"a");
    char new_data[1024];
    sprintf(new_data,"PATH=%s\nSTAGE=1\nID=%d\n", path, ID+1);
    fputs(new_data, history);
    fclose(history);
    return 0;
}
void copyFile(const char *sourcePath, const char *destinationPath) {
    FILE *sourceFile = fopen(sourcePath, "rb");
    if (sourceFile == NULL) {
        perror("Error opening source file");
        return;
    }
    FILE *destinationFile = fopen(destinationPath, "wb");
    if (destinationFile == NULL) {
        perror("Error opening destination file");
        fclose(sourceFile);
        return;
    }
    const size_t bufferSize = 10000;
    char buffer[bufferSize];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, bufferSize, sourceFile)) > 0) {
        fwrite(buffer, 1, bytesRead, destinationFile);
    }
    fclose(sourceFile);
    fclose(destinationFile);
}
int check_wildcard(char * wildcard, char  * word){
    char start[512] ,end[512];
    strcpy(start, wildcard);
    *(strstr(start, "*"))='\0';
    strcpy(end, strstr(wildcard, "*"));
    return 1;
}

int main(int argc, char *argv[]) {
    time_t raw_time;
    static struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(time_now, sizeof(time_now), "%Y-%m-%d %H:%M:%S", time_info);

    if (argc < 2) {
        fprintf(stdout, "please enter a valid command");
        return 1;
    }
    if (getcwd(cwd, sizeof(cwd)) == NULL) return 1;
    else if (strcmp(argv[1], "init") == 0) {
        return run_init(argc, argv);
    } else if (strcmp(argv[1], "config") == 0){
        return run_config(argc, argv);
    } else if(find_tnt()==1 || tnt_path[0]==0) {
        fprintf(stdout, "There is not any repository");
        return 1;
    }else if (strcmp(argv[1], "add") == 0){
        return run_add(argc, argv);
    } /*else if (strcmp(argv[1], "reset") == 0) {
        return run_reset(argc, argv);
    } else if (strcmp(argv[1], "commit") == 0) {
        return run_commit(argc, argv);
    } else if (strcmp(argv[1], "checkout") == 0) {
        return run_checkout(argc, argv);
    }*/

    return 0;
}