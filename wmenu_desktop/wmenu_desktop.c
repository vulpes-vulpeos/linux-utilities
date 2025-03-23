#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WM_BG  "#393939" // normal background color
#define WM_FG  "#DADADA" // normal text color
#define WM_SBG "#834B3A" // selection background
#define WM_SFG "#FBFBFB" // selection text color
#define WM_PT  "Search:" // prompt to the left of the input field
#define WM_PBG "#834B3A" // prompt background color
#define WM_PFG "#FBFBFB" // prompt text color

#define MAX_NAME_LEN 128

typedef struct DFlist { // struct with element de_ctrer and dynamic array of struct pointers
    int dentr_ctr;
    struct Dentry {
        char *df_path, *app_name, *exec_path; 
        int   hid, term;
    } **dentries;
} DFlist;

void launch_app (DFlist *df_list, char *response){
    // find response index
    int app_index = -1;
    for (int ctr = 0; ctr < df_list->dentr_ctr-1; ++ctr){
        if (strstr(df_list->dentries[ctr]->app_name, response) && !df_list->dentries[ctr]->hid){app_index = ctr; break; }; 
    };
    if (app_index == -1) { return; }; // Do nothing if response wasn't found

    if (fork() == 0) {
        if (df_list->dentries[app_index]->term) {
            execlp("kitty", "kitty", "-e", "sh", "-c", df_list->dentries[app_index]->exec_path, NULL);
        } else {
            execlp("sh", "sh", "-c", df_list->dentries[app_index]->exec_path, NULL);
        };
        printf("[wmenu_desktop] Failed to launch application. Exec path: %s\n", df_list->dentries[app_index]->exec_path);
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

char* constr_comm (DFlist *df_list){
    // counting length of all visible app names and total number of visible entries
    int names_str_len = 0, vis_ctr = 0;
    for (int ctr = 0; ctr < df_list->dentr_ctr; ++ctr){
        if (!df_list->dentries[ctr]->hid) {
            names_str_len += strlen(df_list->dentries[ctr]->app_name) + 1;
            ++vis_ctr;
        };
    };
    // combine names into one string using new line as separator
    char *names_str = calloc(names_str_len, sizeof(char));
    if (!names_str){
        perror("[wmenu_desktop] ERROR: calloc: ");
        return NULL;
    };
    char *pos = names_str;
    for (int ctr = 0, add_ctr = 0; ctr < df_list->dentr_ctr; ++ctr) {
        if (!df_list->dentries[ctr]->hid) {
            int len = strlen(df_list->dentries[ctr]->app_name);
            memcpy(pos, df_list->dentries[ctr]->app_name, len);
            pos += len;
            if (add_ctr++ < vis_ctr - 1) {
                *pos++ = '\n';
            };
        };
    };
    *pos = '\0';
     // construct full command
    char *command;
    asprintf(&command, "echo \"%s\" | wmenu -bi -N \"%s\" -n \"%s\" -S \"%s\" -s \"%s\" -p \"%s\" -M \"%s\" -m \"%s\"",
                       names_str, WM_BG, WM_FG, WM_SBG, WM_SFG, WM_PT, WM_PBG, WM_PFG);
    free(names_str);

    return command;
};

void free_df_list(DFlist *df_list){
    for(int ctr = 0; ctr < df_list->dentr_ctr; ++ctr) {
        free(df_list->dentries[ctr]->df_path);
        free(df_list->dentries[ctr]->app_name);
        free(df_list->dentries[ctr]->exec_path);
        free(df_list->dentries[ctr]);
    };
    free(df_list->dentries);
}

void process_pair (struct Dentry *a, struct Dentry *b){
    // do nothing if names are different
    if (strcmp(a->app_name, b->app_name) != 0) { return; };
    // get size of dir path
    long dp_end = strchr(a->df_path, '/') - a->df_path;
    if (strncmp(a->df_path, b->df_path, dp_end) == 0) {
        // do nothing if same path and one is hidden
        if (a->hid || b->hid) { return;
        // set b entry to hidden if none hidden
        } else { b->hid = 1; };
    }; 
    int fl_ha = strncmp(a->df_path, "/home", 5);
    int fl_hb = strncmp(b->df_path, "/home", 5);
    // .desktop entries from user home directory have priority
    if (fl_ha == 0 && a->hid && !b->hid) { ++b->hid; };
    if (fl_hb == 0 && b->hid && !a->hid) { ++a->hid; };      
}

int qsort_comp(const void *a, const void *b) {
    struct Dentry *d1 = *(struct Dentry **)a;
    struct Dentry *d2 = *(struct Dentry **)b;
    
    int cmp = strcasecmp(d1->app_name, d2->app_name);
    if (cmp != 0) { return cmp; };
    
    return d2->hid - d1->hid; // prioritize hid = 1 over hid = 0
}

int parse_dfile (struct Dentry *dentry){
    FILE *file = fopen(dentry->df_path, "r");
    if (!file){
        fprintf(stderr, "[wmenu_desktop] ERROR: %s: ", dentry->df_path); perror("");
        return 1;
    };

    char line[1024]; 
    int fl_an = 0, fl_ep = 0, len = 0;
    while (fgets(line, sizeof(line), file)) {
        // remove new line at the end of the line
        len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        };
        // look for needed data
        if (!fl_an && !strncmp(line, "Name=", 5)) { 
            dentry->app_name = strdup(line + 5);
            ++fl_an;
        } else if (!fl_ep && !strncmp(line, "Exec=", 5)) {
            // remove argument placeholder
            char *percent = strrchr(line + 5, '%');
            if (percent) { *(percent - 1) = '\0'; };
            dentry->exec_path = strdup(line + 5);
            ++fl_ep;
        } else if (!dentry->term && !strcmp(line, "Terminal=true")) {
            dentry->term = 1;
        } else if (!dentry->hid && (!strcmp(line, "Hidden=true") || !strcmp(line, "NoDisplay=true"))) {
            dentry->hid = 1;
        };
        
        // In case of hidden entry we care only about app name
        if (fl_an && dentry->hid) {
            break;
        };
    };

    // Hide entries without a name
    if (!fl_an) {
        dentry->app_name = strdup("N/A");
        dentry->hid = 1;
    };
    // or execution path
    if (!fl_ep) {
        dentry->exec_path = strdup("N/A");
        dentry->hid = 1;
    }; 

    fclose(file);

    return 0;
}

// TODO make this simpler
// look for last . and cmp from there?
int filter(const struct dirent *entry) {
    const char *ext = ".desktop";
    size_t len = strlen(entry->d_name);
    size_t ext_len = strlen(ext);

    return len > ext_len && strcmp(entry->d_name + len - ext_len, ext) == 0;
}

int parse_paths(char **paths, size_t num_paths, DFlist *df_list) {
    for (size_t i = 0; i < num_paths; ++i) {
        // get dirent list of .desktop files
        struct dirent **de_list;
        int de_ctr = scandir(paths[i], &de_list, filter, NULL);
        if (de_ctr < 0) {
            fprintf(stderr, "[wmenu_desktop] ERROR: %s: ", paths[i]); perror("");
            return 1;
        } else if (de_ctr == 0) {
            continue;
        };
        // resize dentries array to fit new entries
        df_list->dentries= realloc(df_list->dentries, (df_list->dentr_ctr + de_ctr) * sizeof(struct Dentry*));
        if (!df_list->dentries) { 
            perror("[wmenu_desktop] ERROR: realloc: ");
            return 1;
        }
        // fill .desktop entries data
        for (int j = 0; j < de_ctr; ++j) {
            df_list->dentries[df_list->dentr_ctr] = calloc(1, sizeof(struct Dentry));
            if (!df_list->dentries[df_list->dentr_ctr]) {
                perror("[wmenu_desktop] ERROR: calloc: ");
                return 1;
            }
            // fill full path to .desktop entry
            asprintf(&df_list->dentries[df_list->dentr_ctr]->df_path, "%s/%s", paths[i], de_list[j]->d_name);
            // fil app name, execution path, hidden and terminal flags
            if (parse_dfile(df_list->dentries[df_list->dentr_ctr])) {
                return 1;
            };

            ++df_list->dentr_ctr;
            free(de_list[j]);
        };
        free(de_list);
    };

    return 0;
}

void usage (){
    puts("Usage: wmenu_dektop <DIR>...\n");
    puts("Arguments:");
    puts("<DIR>...    One or more directories containing .desktop files.");
    puts("            Subfolders are ignored.");
    puts("            ex.: /usr/share/applications ~/.local/share/applications");
}

int main (int argc, char **argv) {
    // checks
    if (argc < 2) { usage(); return 0; };
    // parse provided path(s)
    DFlist df_list = {0};
    if (parse_paths(&argv[1], argc-1, &df_list)){ 
        return 0;
    };
    // sort .desktop entries
    qsort(df_list.dentries, df_list.dentr_ctr, sizeof(struct Dentry*), qsort_comp);
    //for (int i = 0; i < df_list.dentr_ctr; i++) { printf("%s | %s | %d\n", df_list.dentries[i]->app_name, df_list.dentries[i]->exec_path, df_list.dentries[i]->hid); };

    // process .desktop entries to hide duplicates 
    for (int ctr = 0; ctr < df_list.dentr_ctr-1; ++ctr){
        process_pair(df_list.dentries[ctr], df_list.dentries[ctr+1]);
    };
         
    // form wmenu command
    char *wmenu_comm = constr_comm(&df_list);
    if (!wmenu_comm) {
        free_df_list(&df_list);
        return 0;
    };
    
    // execute wmenu command
    char *response = exec_comm(wmenu_comm);
    free(wmenu_comm);

    // launch app
    if (strlen(response) > 0){
        launch_app(&df_list, response);
    };

    free(response);
    free_df_list(&df_list);

    return 0;
}
