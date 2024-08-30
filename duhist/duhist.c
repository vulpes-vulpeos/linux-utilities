#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct ENTRY {
    char     *body;
    char     *summary;
    char     *appname;
    long      timestamp;
    char      timestamp_str[20];
} typedef ENTRY;

void cleanup_data(char *string){
    char *data_beg;
    // New line
    while ((data_beg = strstr(string, "\\n"))!= NULL){
        *data_beg = '\n';
        memmove(data_beg+1,data_beg+2, strlen(data_beg+2));
    };
    // Horizontal tab
    while ((data_beg = strstr(string, "\\t"))!= NULL){
        *data_beg = '\t';
        memmove(data_beg+1,data_beg+2, strlen(data_beg+2));
    };
}

char *get_data(char *buffer){
    int data_size = 0;
    char *data_beg = strstr(buffer, "\"data\" : ") + 9;
    if (*data_beg == '\"'){++data_beg; --data_size;};
    data_size += strlen(data_beg);
    char *data = calloc(data_size, sizeof(char));
    strncpy(data, data_beg, data_size-1);
    return data;
};

long get_sys_uptime(){
    double uptime_sec;
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp == NULL) {printf("ERROR: failed to get system uptime.\n"); return -1; };
    fscanf(fp, "%lf", &uptime_sec);
    fclose(fp);

    return (long) uptime_sec;
}

void get_dunst_history(ENTRY *entries) {
    char buffer[1024];
    int entry_index = 0;
    long uptime_sec = get_sys_uptime(),
         cur_time_sec = time(NULL);

    FILE *fp = popen("dunstctl history 2>&1", "r");
    if (fp == NULL) {printf("ERROR: dunstctl history failed.\n"); exit(1); };
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "body")){
            // Skipping data type line
            fgets(buffer, sizeof(buffer), fp);
            // Getting data line
            fgets(buffer, sizeof(buffer), fp);
            entries[entry_index].body = get_data(buffer);
            cleanup_data(entries[entry_index].body);
        } else if (strstr(buffer, "summary")){
            // Skipping data type line
            fgets(buffer, sizeof(buffer), fp);
            // Getting data line
            fgets(buffer, sizeof(buffer), fp);
            entries[entry_index].summary = get_data(buffer);
        } else if (strstr(buffer, "appname")){
            // Skipping data type line
            fgets(buffer, sizeof(buffer), fp);
            // Getting data line
            fgets(buffer, sizeof(buffer), fp);
            entries[entry_index].appname = get_data(buffer);
        } else if (strstr(buffer, "timestamp")){
            // Skipping data type line
            fgets(buffer, sizeof(buffer), fp);
            // Getting data line
            fgets(buffer, sizeof(buffer), fp);
            char *end_ptr;
            long dunst_timestamp_sec= strtol(get_data(buffer), &end_ptr, 10)/1000/1000, // ns to sec
                 unix_timestamp_sec = cur_time_sec - uptime_sec + dunst_timestamp_sec;
            time_t timestamp = (time_t)unix_timestamp_sec;
            struct tm *time_info = localtime(&timestamp);
            strftime(entries[entry_index].timestamp_str, sizeof(entries[entry_index].timestamp_str), "%Y-%m-%d %H:%M", time_info);
            ++entry_index;
        };
    };

    pclose(fp);
}

int get_history_size() {
    char buffer[128];
    int count = 0;

    FILE *fp = popen("dunstctl count history 2>&1", "r");
    if (fp == NULL) {printf("ERROR: couldn't get size of history.\n"); return -1; };
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {count = atoi(buffer); };
    
    pclose(fp);
    return count;
}

int main() {
    int hist_size = get_history_size();

    if (hist_size > 0){
        ENTRY entries[hist_size];
        get_dunst_history(entries);

        for (int i = hist_size-1; i >= 0; --i){
            printf("\033[1m%s\033[m - \033[1;34m%s\033[m - %s\n%s\n%s\n", entries[i].timestamp_str, entries[i].appname, entries[i].summary, entries[i].body,(i == 0) ? "" : "-----");
        };
    };

    return 0;
}
