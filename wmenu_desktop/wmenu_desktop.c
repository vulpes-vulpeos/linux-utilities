#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <strings.h>
#include <unistd.h>

#define MAX_APPS 1024
#define BUFFER_SIZE 4096 // This is max path length

struct App { char *dpath, *app_name, *exec_path; int terminal; } typedef App;

App apps[MAX_APPS]; // TODO do not use global variables
size_t app_ctr = 0;

void exec_parse(App *selected) {
    // II parse to get execution path and check if it's terminal app
    FILE *file = fopen(selected->dpath, "r");
    if (!file){return; }; // exec_path will stay NULL if something goes wrong

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        if (!selected->exec_path && strncmp(line, "Exec=", 5) == 0) {
            selected->exec_path = strndup(line + 5, strlen(line +5)-1);
            selected->exec_path[strcspn(selected->exec_path, "%")] = '\0';
        } else if (selected->terminal == 0 && strcmp(line, "Terminal=true\n") == 0) { selected->terminal = 1; };

        if (selected->exec_path && selected->terminal) { break; };
    };

    fclose(file);
}

void base_parse(const char *filepath) {
    // Getting only filepath, app name and skip hidden apps. Do not waste time on filling other fields for all apps.
    FILE *file = fopen(filepath, "r");
    if (!file){return; };

    char line[BUFFER_SIZE], *app_name = NULL;
    int hidden = 0;
    while (fgets(line, sizeof(line), file)) {
        if (!app_name && strncmp(line, "Name=", 5) == 0) { app_name = strndup(line + 5, strlen(line+5)-1);
        } else if (!hidden && (strcmp(line, "Hidden=true\n") == 0 || strcmp(line, "NoDisplay=true\n") == 0)) {++hidden; break; };
    };

    fclose(file);

    if (app_name && !hidden) {
        apps[app_ctr] = (struct App){strdup(filepath), app_name, NULL, 0};
        ++app_ctr;
    };
}

void parse_directory(const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if (!dir){ return; };

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
            char filepath[BUFFER_SIZE];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);
            if (strstr(entry->d_name, ".desktop")) { base_parse(filepath); };
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
    // TODO add duplicates removal
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
        strcat(app_names, apps[i].app_name);
        if (i < app_ctr - 1) { strcat(app_names, "\n"); }
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
            if (strcmp(selected_app, apps[i].app_name) == 0) { exec_parse(&apps[i]); launch_app(apps[i].exec_path, apps[i].terminal); break; };
        };
    };

    pclose(pipe);

    // Free dynamically allocated memory in apps array
    for (size_t i = 0; i < app_ctr; i++) { free(apps[i].app_name); free(apps[i].exec_path); };

    return 0;
   }
