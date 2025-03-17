#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

char *exec_comm(char *wmenu_comm, int *longest_name){
    // Open wmenu, read and execute selected app
    FILE *pipe = popen(wmenu_comm, "r");
    if (!pipe) { perror("Failed to open wmenu pipe"); exit(1); };

    char *sel_dir = calloc(*longest_name + 1, sizeof(char)); 
    if (fgets(sel_dir, *longest_name+1, pipe)) {
        sel_dir[strcspn(sel_dir, "\n")] = '\0'; // remove new line char
    };

    pclose(pipe);
    
    return sel_dir;
}

char* constr_comm(char **dir_list, int *const dir_num, int *longest_name, int *const path_skip){
    // calculate total length of dir names + new line chars
    int dir_list_len = 0, cur_len = 0;
    for (int ctr = 0; ctr < *dir_num; ++ctr){
        cur_len = strlen(dir_list[ctr] + *path_skip) + 1; dir_list_len += cur_len;
        if (cur_len > *longest_name){ *longest_name = cur_len; }; 
    };

    // combine names into string separated by new lines
    char *dir_list_str = calloc(dir_list_len, sizeof(char));
    for (size_t ctr = 0; ctr < *dir_num; ++ctr) { strcat(dir_list_str, dir_list[ctr] + *path_skip); if (ctr < *dir_num - 1) { strcat(dir_list_str, "\n"); }; };

    // calculate length of the wmenu command
    char *bg = "#403f3a", *fg = "#dbdbdb", *sbg = "#9c4f30", *sfg = "#dbdbdb", *pt = "Search:" , *pbg = "#9c4f30", *pfg = "#dbdbdb";
    int wmenu_len = strlen(pt) + 69 + 42 + 1; // 42 - total len of color strings, 69 - total length of predefined command text
                                              //
    // combine everything
    char *command = calloc(dir_list_len + wmenu_len, sizeof(char*));
    sprintf(command, "echo \"%s\" | wmenu -bi -N \"%s\" -n \"%s\" -S \"%s\" -s \"%s\" -p \"%s\" -M \"%s\" -m \"%s\"", dir_list_str, bg, fg, sbg, sfg, pt, pbg, pfg);
    free(dir_list_str);

    return command;
}

// TODO add macOS junk recognition?
int dir_empty (char *path){
    DIR *dptr = opendir(path);
    if(dptr == NULL) { exit(1); };
    struct dirent *entry;
    while ((entry = readdir(dptr)) != NULL){
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) { continue; }; // ignore itself and parent folders
        closedir(dptr);
        return 0;                                                                       // return false if there is anything
    };

	closedir(dptr);
    return 1;
}

void dir_parse (char *mus_dir, int *const dir_num, char **dir_list, int *ctr){
    DIR *dptr = opendir(mus_dir);
    if(dptr == NULL) { exit(1); };
    struct dirent *entry;
    while ((entry = readdir(dptr)) != NULL){
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) { continue; }; // ignore itself and parent folders
        if (entry->d_type == DT_DIR){                                                   // check if path is a folder
            char full_path[strlen(mus_dir)+strlen(entry->d_name)+2];
            snprintf(full_path, sizeof(full_path), "%s/%s", mus_dir, entry->d_name);    // create full path
            if (!dir_empty(full_path)){                                                 // process folder only if it's not empty
                dir_list[*ctr] = strdup(full_path); ++(*ctr);
                dir_parse(full_path, dir_num, dir_list, ctr);                           // look for subfolders
            };
        };
    };

	closedir(dptr);	
}


int dir_count (char *mus_dir){
    DIR *dptr = opendir(mus_dir);
    if(dptr == NULL) { return 0; };
    struct dirent *entry;
    int dir_num = 0;
    while ((entry = readdir(dptr)) != NULL){
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) { continue; };  // ignore itself and parent folders
        if(entry->d_type == DT_DIR){                                                     // process folder only if it's not empty
            char full_path[strlen(mus_dir)+strlen(entry->d_name)+2];
            snprintf(full_path, sizeof(full_path), "%s/%s", mus_dir, entry->d_name);     // create full path
            if (!dir_empty(full_path)){ ++dir_num; dir_num += dir_count(full_path); };   // +1 folder counter and look for subfolders
        };
    };

	closedir(dptr);	
    return dir_num;
}

char* get_musdir(char *conf_dir){
    FILE* fptr = fopen(conf_dir, "r");
    if (fptr == NULL) { return NULL; };
    char buffer[1024];
    char *mus_dir = NULL;
    while(fgets(buffer, sizeof(buffer), fptr)) {
        if (strstr(buffer, "music_directory") != NULL){                                   // check if string contains "music_directory"
            long beg = strchr(buffer, '"') - buffer;                                      // find first "
            long end = strrchr(buffer, '"') - buffer;                                     // find last "
            mus_dir = strndup(buffer+beg+1, end-beg-1);                                   // copy path into mus_dir variable
            break;
        };
    };

    fclose(fptr);
    return mus_dir;
};

int qsort_comp (const void *elem1, const void *elem2) {
    return strcasecmp(*(char**)elem1, *(char**)elem2);
}

void usage(){
    puts("[wmenu_mpd] USAGE: wmenu_mpd /path/to/mdp/config\n");
};

int main(int argc, char **argv){
    // checks    
    if (argc < 2 || argc > 2) { usage(); return 1;};
    struct stat sb;
    if(stat(argv[1], &sb) != 0 || S_ISDIR(sb.st_mode)) { printf("ERROR: Path %s doesn't exist or is a folder.\n", argv[1]); return 1; };

    // get music directory path from mpd config
    char *mus_dir = get_musdir(argv[1]);
    if (mus_dir == NULL) { printf("Wasn't able to get music directory from provided mpd config path.\n"); return 1; };

    // count folders and subfolders in music directory
    int dir_num = dir_count(mus_dir) ; 
    if (!dir_num) {printf("No folders to parse\n"); free(mus_dir); return 1; };
    
    // copy folders names into array 
    char **dir_list = malloc(dir_num * sizeof(char*));
    int ctr = 0;
    dir_parse(mus_dir, &dir_num, dir_list, &ctr);

    // sort alphabeticaly case insensetive
    qsort(dir_list, dir_num, sizeof(char*), qsort_comp);
    //int cut_start = strlen(mus_dir) +1; for (int ctr = 0; ctr < dir_num; ++ctr){printf("%s\n", (dir_list[ctr]) ? dir_list[ctr]+cut_start : "NULL"); };
    
    // construct wmenu command
    int longest_name = 0, path_skip = strlen(mus_dir)+1;
    char *wmenu_comm = constr_comm(dir_list, &dir_num, &longest_name, &path_skip);

    // open wmenu, read response
    char *response = exec_comm(wmenu_comm, &longest_name);

    free(wmenu_comm); 
    free(mus_dir); 
    for (int ctr = 0; ctr < dir_num; ++ctr){free(dir_list[ctr]); };
    free(dir_list);

    // run mpc commands
    if (strlen(response) > 0) {
        if (fork() == 0) { execlp("mpc", "mpc", "-q", "stop", (char *) NULL); _exit(1); };
        wait(NULL); 
        if (fork() == 0) { execlp("mpc", "mpc", "-q", "clear", (char *) NULL); _exit(1); };
        wait(NULL);
        if (fork() == 0) { execlp("mpc", "mpc", "-q", "--wait", "add", response, (char *) NULL); _exit(1); };
        wait(NULL); 
        if (fork() == 0) { execlp("mpc", "mpc", "-q", "play", (char *) NULL); _exit(1); };
        wait(NULL);
    };
    
    free(response);
    
    return 0;
}
