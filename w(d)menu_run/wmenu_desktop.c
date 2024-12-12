#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <strings.h>
#include <unistd.h>

#define MAX_APPS 1024
#define BUFFER_SIZE 4096 // This is max path length

struct App { char *app_name; char *exec_path; int terminal; } typedef App;

App apps[MAX_APPS]; // TODO do not use global variables
size_t app_ctr = 0;

void parse_desktop_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file){return; };

    char line[BUFFER_SIZE];
    char *app_name = NULL, *exec_path = NULL;
    int terminal = 0, hidden = 0;
    int fl_name = 0, fl_exec = 0, fl_term = 0, fl_hidd = 0;
    while (fgets(line, sizeof(line), file)) {
        if (!fl_name && strncmp(line, "Name=", 5) == 0) { app_name = strndup(line + 5, strlen(line+5)-1); ++fl_name;
        } else if (!fl_exec && strncmp(line, "Exec=", 5) == 0) { exec_path = strndup(line + 5, strlen(line +5)-1); exec_path[strcspn(exec_path, "%")] = '\0'; ++fl_exec;
        } else if (!fl_term && strcmp(line, "Terminal=true\n") == 0) { terminal = 1; ++fl_term;
        } else if (!fl_hidd && strcmp(line, "Hidden=true\n") == 0) { hidden = 1; ++fl_hidd;
        } else if (!fl_hidd && strcmp(line, "NoDisplay=true\n") == 0) { hidden = 1; ++fl_hidd; };
    };

    fclose(file);

    if (app_name && exec_path && !hidden) {
        apps[app_ctr].app_name = app_name;
        apps[app_ctr].exec_path = exec_path;
        apps[app_ctr].terminal = terminal;
        ++app_ctr;
    } else { if (app_name){ free(app_name); }; if (exec_path){ free(exec_path); }; };
}

void parse_directory(const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if (!dir){ return; };

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
            char filepath[BUFFER_SIZE];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);
            if (strstr(filepath, ".desktop")) { parse_desktop_file(filepath); };
        };
    };

    closedir(dir);
}

void launch_app(const char *command, int terminal) {
    if (fork() == 0) {
        if (terminal) { execlp("kitty", "kitty", "-e", "sh", "-c", command, NULL);
        } else { execlp("sh", "sh", "-c", command, NULL); };
        perror("[WMENU_DESKTOP] Failed to launch application");
        printf("Exec path: %s\n", command);
        exit(1);
    }
}

int qsort_comp (const void *elem1, const void *elem2) {
    return strcasecmp(((App*)elem1)->app_name, ((App*)elem2)->app_name);
}

int main() {
    parse_directory("/home/vulpeos/.local/share/applications");
    parse_directory("/usr/share/applications");
    if (app_ctr == 0){ printf("[WMENU_DESKTOP] no .desktop files found\n"); return 1; };

    // sort case insensetive
    qsort(apps, app_ctr, sizeof(App), qsort_comp);
    //for (int i = 0; i < app_ctr; ++i){ printf("%s, %s, %d\n", apps[i].app_name, apps[i].exec_path, apps[i].terminal); }; return 0; // Parsing and sorting control

    // Combine app names into a single string with newline separation
    int names_tot_len = 0, longest_name = 0;
    for (int i = 0; i < app_ctr; ++i){ // Calculating total length of app names + new line char
        names_tot_len += strlen(apps[i].app_name)+1;
        if (strlen(apps[i].app_name) > longest_name){ longest_name = strlen(apps[i].app_name); }; 
    }; 
    char *app_names = calloc(names_tot_len, sizeof(char));
    if (!app_names) { perror("Failed to allocate memory for app_names"); return 1; };
    for (size_t i = 0; i < app_ctr; i++) { 
        strncat(app_names, apps[i].app_name, BUFFER_SIZE - strlen(app_names) - 1);
        if (i < app_ctr - 1) { strncat(app_names, "\n", BUFFER_SIZE - strlen(app_names) - 1); };
    };
    // Construct the wmenu command
    char *bg = "#403f3a", *fg = "#dbdbdb", *sbg = "#9c4f30", *sfg = "#dbdbdb", *pt = "Search:" , *pbg = "#9c4f30", *pfg = "#dbdbdb";
    char command[names_tot_len + strlen(pt) + 69 + 42 + 1]; // 42 - total len of color strings, 69 - total length of predefined command text
    snprintf(command, sizeof(command), "echo \"%s\" | wmenu -bi -N \"%s\" -n \"%s\" -S \"%s\" -s \"%s\" -p \"%s\" -M \"%s\" -m \"%s\"",
             app_names, bg, fg, sbg, sfg, pt, pbg, pfg);
    free(app_names);
    
    // Open wmenu, read and execute selected app
    FILE *pipe = popen(command, "r");
    if (!pipe) { perror("Failed to open wmenu pipe"); return 1; };

    char selected_app[longest_name+1]; 
    if (fgets(selected_app, sizeof(selected_app), pipe)) {
        selected_app[strcspn(selected_app, "\n")] = '\0'; // remove new line char
        for (size_t i = 0; i < app_ctr; i++) { // Find and launch the selected app
            if (strcmp(selected_app, apps[i].app_name) == 0) { launch_app(apps[i].exec_path, apps[i].terminal); break; };
        };
    };

    pclose(pipe);

    // Free dynamically allocated memory in apps array
    for (size_t i = 0; i < app_ctr; i++) { free(apps[i].app_name); free(apps[i].exec_path); };

    return 0;
}
