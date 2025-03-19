#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_NAME_LEN 128

typedef struct DFlist { // struct with element counter and dynamic array of struct pointers
    int dentr_ctr;
    struct Dentry {
        char *df_path, *app_name, *exec_path; 
        int   hid, term;
    } **dentries;
} DFlist;

void launch_app (DFlist *dflist, char *response){
    // find response index
    int app_index = -1;
    for (int ctr = 0; ctr < dflist->dentr_ctr-1; ++ctr){
        if (strstr(dflist->dentries[ctr]->app_name, response) && !dflist->dentries[ctr]->hid){app_index = ctr; break; }; 
    };
    if (app_index == -1) { return; }; // Do nothing if response wasn't found

    if (fork() == 0) {
        if (dflist->dentries[app_index]->term) {
            execlp("kitty", "kitty", "-e", "sh", "-c", dflist->dentries[app_index]->exec_path, NULL);
        } else {
            execlp("sh", "sh", "-c", dflist->dentries[app_index]->exec_path, NULL);
        };
        printf("[wmenu_desktop] Failed to launch application. Exec path: %s\n", dflist->dentries[app_index]->exec_path);
    };
}

char *exec_comm(char *wmenu_comm){
    // Open wmenu, read and execute selected app
    FILE *pipe = popen(wmenu_comm, "r");
    if (!pipe) { perror("[wmenu_desktop] Failed to open wmenu pipe"); exit(1); };

    char *selection = calloc(MAX_NAME_LEN + 1, sizeof(char)); 
    if (fgets(selection, MAX_NAME_LEN, pipe)) { selection[strcspn(selection, "\n")] = '\0'; };

    pclose(pipe);
    
    return selection;
}

char* constr_comm (DFlist *dflist){
    // counting length of all visible app names and total number of visible entries
    int names_str_len = 0, vis_ctr = 0;
    for (int ctr = 0; ctr < dflist->dentr_ctr; ++ctr){
        if (!dflist->dentries[ctr]->hid) { names_str_len += strlen(dflist->dentries[ctr]->app_name) + 1; ++vis_ctr; };
    };
    // combine names into one string using new line as separator
    char *names_str = calloc(names_str_len, sizeof(char));
    for (int ctr = 0, add_ctr = 0; ctr < dflist->dentr_ctr; ++ctr){
        if (!dflist->dentries[ctr]->hid) {
            strcat(names_str, dflist->dentries[ctr]->app_name);
            if (add_ctr++ < vis_ctr - 1) { strcat(names_str, "\n"); };
        };
    };
    // calculate length of wmenu part of command
    char *bg = "#403f3a", *fg = "#dbdbdb", *sbg = "#9c4f30", *sfg = "#dbdbdb", *pt = "Search:" , *pbg = "#9c4f30", *pfg = "#dbdbdb";
    int wmenu_len = strlen(pt) + 69 + 42 + 1; // 42 - total len of color strings, 69 - total length of predefined command text
    // combine everything
    char *command = calloc(names_str_len + wmenu_len, sizeof(char*));
    sprintf(command, "echo \"%s\" | wmenu -bi -N \"%s\" -n \"%s\" -S \"%s\" -s \"%s\" -p \"%s\" -M \"%s\" -m \"%s\"", names_str, bg, fg, sbg, sfg, pt, pbg, pfg);
    free(names_str);

    return command;

};

void process_pair (struct Dentry *a, struct Dentry *b){
    if (strcmp(a->app_name, b->app_name) != 0) { return; }; // Do nothing if names are different
    long dp_end = strchr(a->df_path, '/') - a->df_path;     // Get size of dir path
    if (strncmp(a->df_path, b->df_path, dp_end) == 0) {
        if (a->hid || b->hid) { return;                     // Do nothing if same path and one is hidden
        } else { b->hid = 1; };                             // Set b entry to hidden if none hidden
    }; 
    int fl_ha = strncmp(a->df_path, "/home", 5);
    int fl_hb = strncmp(b->df_path, "/home", 5);
    if (fl_ha == 0 && a->hid && !b->hid) { ++b->hid; };     // .desktop entries from user home directory
    if (fl_hb == 0 && b->hid && !a->hid) { ++a->hid; };     // have priority 
}

int qsort_comp(const void *a, const void *b) {
    struct Dentry *d1 = *(struct Dentry **)a;
    struct Dentry *d2 = *(struct Dentry **)b;
    
    int cmp = strcasecmp(d1->app_name, d2->app_name);
    if (cmp != 0) { return cmp; };
    
    return d2->hid - d1->hid; // prioritize hid = 1 over hid = 0
}

void parse_dfile (struct Dentry *dentry){
    FILE *file = fopen(dentry->df_path, "r");
    if (!file){                                                          // Create dummy entry if can't open file
        dentry->app_name = strdup("N/A");
        dentry->exec_path = strdup("N/A");
        dentry->hid = 1;
        return;
    };

    char line[1024]; 
    int fl_an = 0, fl_ep = 0, fl_h = 0, fl_t = 0;                        // Flags to check if info is already filled 
    while (fgets(line, sizeof(line), file)) {
        if (!fl_an && strncmp(line, "Name=", 5) == 0) {
            dentry->app_name = strndup(line + 5, strlen(line+5)-1); ++fl_an;
        } else if (!fl_ep && strncmp(line, "Exec=", 5) == 0) {
            dentry->exec_path = strndup(line + 5, strlen(line +5)-1); ++fl_ep;
            dentry->exec_path[strcspn(dentry->exec_path, "%")] = '\0';
        } else if (!fl_t && strcmp(line, "Terminal=true\n") == 0) {
            dentry->term = 1; ++fl_t;
        } else if (!fl_h && (strcmp(line, "Hidden=true\n") == 0 || strcmp(line, "NoDisplay=true\n") == 0)) {
            dentry->hid = 1; ++fl_h;
        };

        if (fl_an && fl_ep && fl_h && fl_t) { break; };
    };
    
    if (!fl_an) { dentry->app_name = strdup("N/A"); dentry->hid = 1; };  // Hide entries without a name
    if (!fl_ep) { dentry->exec_path = strdup("N/A"); dentry->hid = 1; }; // or execution path

    fclose(file);
}

void parse_folder (char *dir_path, DFlist *dflist) {
    DIR *dptr = opendir(dir_path); if(dptr == NULL) { return; };
    struct dirent *entry;
    int path_size = 0;
    while ((entry = readdir(dptr)) != NULL){
        if (strstr(entry->d_name, ".desktop")) {
            dflist->dentries[dflist->dentr_ctr] = calloc(1, sizeof(struct Dentry));
            path_size = strlen(dir_path) + strlen(entry->d_name) + 2;
            dflist->dentries[dflist->dentr_ctr]->df_path = calloc(path_size, sizeof(char));
            snprintf(dflist->dentries[dflist->dentr_ctr]->df_path, path_size, "%s/%s", dir_path, entry->d_name);
            ++dflist->dentr_ctr;
        };
    };

	closedir(dptr);
}

void count_dfiles (char *dir_path, int *dfctr){
    DIR *dptr = opendir(dir_path); if(dptr == NULL) { return; };
    struct dirent *entry;
    while ((entry = readdir(dptr)) != NULL){ if (strstr(entry->d_name, ".desktop")) { ++*dfctr; }; };

	closedir(dptr);
}

int path_usable (char *const dir_path) {
    struct stat sb;
    if(stat(dir_path, &sb) == 0 && S_ISDIR(sb.st_mode)) { return 1; }; // Path exists and is a directory

    return 0;
}

void usage (){
    puts("[wmenu_desktop] Usage: wmenu_dektop /path(s)/to/folder(s)/with/.dekstop/files");
}

int main (int argc, char **argv) {
    // checks
    if (argc < 2) { usage(); return 0; };
    for (int ctr = 1; ctr < argc; ++ctr){
        if (!path_usable(argv[ctr])) { printf("[wmenu_desktop] Path doesn't exist or isn't a folder: %s", argv[ctr]); return 0; };
    };
    
    // count .desktop files
    int df_ctr = 0;
    for (int ctr = 1; ctr < argc; ++ctr){ count_dfiles(argv[ctr], &df_ctr); };

    // parse folders with .desktop files
    DFlist dflist = {0};
    dflist.dentries = malloc(df_ctr * sizeof(struct Dentry*));
    for (int ctr = 1; ctr < argc; ++ctr){ parse_folder(argv[ctr], &dflist); };

    // parse .desktop files 
    for (int ctr = 0; ctr < dflist.dentr_ctr; ++ctr){ parse_dfile(dflist.dentries[ctr]); };

    // sort .desktop entries
    qsort(dflist.dentries, dflist.dentr_ctr, sizeof(struct Dentry*), qsort_comp);

    // process .desktop entries to hide duplicates 
    for (int ctr = 0; ctr < dflist.dentr_ctr-1; ++ctr){ process_pair(dflist.dentries[ctr], dflist.dentries[ctr+1]); };
    
    // form wmenu command
    char *wmenu_comm = constr_comm(&dflist);
    //printf("wmenu command: %s\n", wmenu_comm);
    
    // execute wmenu command
    char *response = exec_comm(wmenu_comm);
    //printf("User response: %s (len: %lu)\n", response, strlen(response));
    free(wmenu_comm);

    // launch app
    if (strlen(response) > 0){ launch_app(&dflist, response); };
    free(response);
    
    for(int ctr = 0; ctr < dflist.dentr_ctr; ++ctr){
        free(dflist.dentries[ctr]->df_path);
        free(dflist.dentries[ctr]->app_name);
        free(dflist.dentries[ctr]->exec_path);
        free(dflist.dentries[ctr]);
    };
    free(dflist.dentries);

 return 0;
}
