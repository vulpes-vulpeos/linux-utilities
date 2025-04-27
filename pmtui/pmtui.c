#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <libudev.h>

struct Drive {
    char *devnode;     // dev path (/dev/sb*)
    char *size;        // size in number of 512 bytes sectors
    char *fs;          // filesystem (ex. vfat)
    char *label;       // partition name
    char *mount_point; // path to mount point 
    int   mounted;     // mounted flag
} typedef Drive;

struct Menu_list {
    Drive **drives;
    int     size;
    int     sel;
    int     top_sel;
    int     max_rows;
    int     cols_len[5];
} typedef Menu_list;

struct Term {
    int rows;
    int cols;
    int hrow;
    int hcol;
} typedef Term ;

// █▀▄ █▀█ █ █ █ █▀▀ █▀▀
// █▄▀ █▀▄ ▀▄▀ █ ██▄ ▄▄█
int manage_drive(const char * devnode, const char bwin_sel){
    char *command = calloc(256,sizeof(char));
    if (bwin_sel == 'm'){sprintf(command, "udisksctl mount --no-user-interaction -b %s 2>&1", devnode);
    } else if (bwin_sel == 'u') {sprintf(command, "udisksctl unmount --no-user-interaction -b %s 2>&1", devnode);
    } else if (bwin_sel == 'p') {sprintf(command, "udisksctl power-off --no-user-interaction -b %s 2>&1", devnode);
    } else if (bwin_sel == 'e') {sprintf(command, "eject %s && sleep 0.2", devnode); };

    FILE *fp;
    char buffer[1024];
    int error = 0;
    fp = popen(command, "r");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "not a mount") != NULL) {error = 5; };
        if (strstr(buffer, "Error") == NULL) {continue; };

        if (strstr(buffer, "DeviceBusy") != NULL){error = 1;
        } else if (strstr(buffer, "AlreadyMounted") != NULL){error = 2;
        } else if (strstr(buffer, "drive is in use") != NULL){error = 3;
        } else if (strstr(buffer, "Not authorized") != NULL){error = 4;
        } else {error = 6; };
    };
    pclose(fp);
    free(command);
    return error;
}

char* get_mount_point(const char *dev_node){
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) {return NULL; };

    char line[256];
    char *mount_point = NULL;

    while (fgets(line, sizeof(line), fp)){
        char device[256], mount[256];
        if (sscanf(line, "%255s %255s", device, mount) == 2){
            if (strcmp(device, dev_node) == 0){mount_point = strdup(mount); break; };
        };
    };

    fclose(fp);
    return mount_point;
 }

char* get_human_size(struct udev_device *dev){
    const char *suffixes[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
    int suffixIndex = 0;
    double size = (double)(atoll(udev_device_get_sysattr_value(dev, "size")) * 512);

    while (size >= 1024 && suffixIndex < 8) {size /= 1024; suffixIndex++;};
    char *size_str = malloc(12*sizeof(char));
    sprintf(size_str, "%.2f%s", size, suffixes[suffixIndex]);
    return size_str;
}

int valid_device(struct udev_device **dev){
    const char *devnode = udev_device_get_devnode(*dev);
    if (!udev_device_get_property_value(*dev, "ID_BUS")){return 0; };
    if (!isdigit(devnode[strlen(devnode)-1])){return 0; }; // Ignore if not partition
    if (udev_device_get_property_value(*dev, "ID_FS_TYPE") == NULL){return 0; };
    // Search if HDD UUID present in fstab
    const char *id_fs_uuid = udev_device_get_property_value(*dev, "ID_FS_UUID");
    FILE *file_ptr = fopen("/etc/fstab", "r");
    char line[1024];
    while (fgets(line, sizeof(line), file_ptr) != NULL) {
        if (strstr(line, id_fs_uuid)){fclose(file_ptr); return 0; };
    };
    fclose(file_ptr);

    return 1;
}

void free_drives(Menu_list *menu){
    for (int i = 0; i < menu->size; ++i){
        free(menu->drives[i]->devnode);
        free(menu->drives[i]->size);
        free(menu->drives[i]->fs);
        free(menu->drives[i]->label);
        free(menu->drives[i]->mount_point);
        free(menu->drives[i]);
    };
    free(menu->drives);
}

void get_drives(Menu_list *menu){
    // Do not leak memory on drives list update
    if (menu->drives != NULL){
        free_drives(menu);
    };

    struct udev *udev = udev_new();
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    struct udev_list_entry *devices, *dev_list_entry;

    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    // Geting number of drives
    devices = udev_enumerate_get_list_entry(enumerate);
    menu->size = 0;
    udev_list_entry_foreach(dev_list_entry, devices) {
        struct udev_device *dev = udev_device_new_from_syspath(udev, udev_list_entry_get_name(dev_list_entry));
        if (valid_device(&dev)) {++menu->size; };
        udev_device_unref(dev);
    };
    // Do nothing if no drives connected
    if (menu->size < 1){udev_enumerate_unref(enumerate); udev_unref(udev); return; };
    // Geting list of drives
    int ctr = 0;
    char *str_ptr = NULL;
    menu->drives = malloc(menu->size * sizeof(Drive));
    udev_list_entry_foreach(dev_list_entry, devices) {
        struct udev_device *dev = udev_device_new_from_syspath(udev, udev_list_entry_get_name(dev_list_entry));
        if (valid_device(&dev)) {
            menu->drives[ctr] = malloc(sizeof(Drive));
            str_ptr = (char *)udev_device_get_devnode(dev);
            if (str_ptr != NULL){ menu->drives[ctr]->devnode = strdup(str_ptr);
            } else { menu->drives[ctr]->devnode = strdup("<NO DEVNODE>"); };
            str_ptr = (char *)udev_device_get_property_value(dev, "ID_FS_LABEL");
            if (str_ptr != NULL){ menu->drives[ctr]->label = strdup(str_ptr);
            } else { menu->drives[ctr]->label = strdup("<NO LABEL>"); };
            str_ptr = (char *)udev_device_get_property_value(dev, "ID_FS_TYPE");
            if (str_ptr != NULL){ menu->drives[ctr]->fs = strdup(str_ptr);
            } else { menu->drives[ctr]->fs = strdup("<NO FS TYPE>"); };
            menu->drives[ctr]->size = get_human_size(dev);
            menu->drives[ctr]->mount_point = get_mount_point(udev_device_get_devnode(dev));
            menu->drives[ctr]->mounted = 1;
            if (menu->drives[ctr]->mount_point == NULL){
                   menu->drives[ctr]->mount_point = malloc(sizeof(char) * 12);
                   strcpy(menu->drives[ctr]->mount_point, "Not mounted");
                   --menu->drives[ctr]->mounted;
            };
            ++ctr;
        };
        udev_device_unref(dev);
    };

    // Cleanup
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

// ▀█▀ █ █ █
//  █  ▀▄█ █
void err_mess(const int err_code, const Term *term){
    const char *err_strings[] = {"", "drive is busy", "already mounted", "drive is in use",
                                 "permission denied", "not mountable", "unknown error"};
    int col = (term->cols - strlen(err_strings[err_code]))/2 ;
    printf("\033[%d;%dH", term->hrow, col);
    printf("\033[7;31m * %s * \033[0m", err_strings[err_code]);

    getchar();
}

// Terminal settings to read input without pressing Enter
void term_mode(int flag) {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    // Toggle canonical mode and echo + toggle cursor visibility
    if (flag){term.c_lflag &= ~(ICANON | ECHO); printf("\033[?25l");
    } else {term.c_lflag |= (ICANON | ECHO); printf("\033[?25h"); };
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Function to get terminal size (rows and columns)
void get_term_size(Term *term) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        term->rows = 24;  // Default to 24 lines if unable to get terminal size
        term->cols = 80;  // Default to 80 columns
    } else {
        term->rows = ws.ws_row;
        term->cols = ws.ws_col;
    };

}
// Function to get current cursor position (row and column)
void get_curs_pos(Term *term) {
    char buf[32];
    unsigned int i = 0;
    term->hrow = -1;
    term->hcol = -1;
    printf("\033[6n"); // Request cursor position
    fflush(stdout);    // Ensure the escape code is sent to the terminal
    while (i < sizeof(buf) - 1) { // Read the response: ESC [rows;colsR
        if (read(STDIN_FILENO, buf + i, 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    };
    buf[i] = '\0';
    // Parse the response to extract rows and columns
    if (buf[0] == '\033' && buf[1] == '[') {
        if (sscanf(buf + 2, "%d;%d", &term->hrow, &term->hcol) != 2) {
            perror("Failed to parse cursor position");
        };
    };
}

void calc_menu_param(Menu_list *menu, Term *term){
    // Drives list can occupy 4 rows or less
    (menu->size > 4) ? (menu->max_rows = 4) : (menu->max_rows = menu->size);
    int rows_needed = (menu->max_rows + 2) - (term->rows - (term->hrow - 1));
    if (rows_needed > 0) {
        for (int i = 0; i < menu->size + 1; ++i) {printf("\n"); }
        term->hrow -= rows_needed;
        fflush(stdout); // Ensure new lines are flushed
    };
    // Calculate columns length
    menu->cols_len[0] = menu->cols_len[1] = menu->cols_len[2] = menu->cols_len[3] = menu->cols_len[4] = 0;
    for (int i = 0; i < menu->max_rows; i++) {
        if (menu->cols_len[0] < strlen(menu->drives[i]->devnode)){menu->cols_len[0] = strlen(menu->drives[i]->devnode); };
        if (menu->cols_len[1] < strlen(menu->drives[i]->label)){menu->cols_len[1] = strlen(menu->drives[i]->label); };
        if (menu->cols_len[2] < strlen(menu->drives[i]->fs)){menu->cols_len[2] = strlen(menu->drives[i]->fs); };
        if (menu->cols_len[3] < strlen(menu->drives[i]->size)){menu->cols_len[3] = strlen(menu->drives[i]->size); };
    };
    menu->cols_len[4] = term->cols - menu->cols_len[0] - menu->cols_len[1] - menu->cols_len[2] - menu->cols_len[3] - 15;
    // Fix selection position if needed
    while(menu->sel > menu->size-1){
        --menu->sel; 
        if(menu->sel < 0){menu->sel = 0; break; };
    };
}

void print_menu(Menu_list *menu, const Term *term) {
    int start_ind = 0;

    // Scrolling math (adding +1 in some places to compensate 0 index)
    if (menu->size > menu->max_rows){
        // Forward scrolling 
        if (menu->sel+1 > menu->max_rows){start_ind = menu->sel+1 - menu->max_rows; };
        // Backward scrolling
        if (menu->top_sel > menu->sel && (menu->top_sel - menu->sel) == menu->max_rows){--menu->top_sel; };
        if (menu->top_sel > menu->sel && (menu->top_sel - menu->sel) < menu->max_rows &&
            (menu->top_sel+1 - menu->max_rows) >= 0 ){start_ind = menu->top_sel+1 - menu->max_rows; };
    };

    // Move cursor to the start of the menu area and clear each line
    printf("\033[%d;%dH", term->hrow, 1);
    for (int i = start_ind, row = 0; i < ((menu->size > menu->max_rows) ? menu->max_rows+start_ind : menu->size); ++i, ++row){
         printf("\033[K%s %-*s   %-*s   %-*s   %-*s   %-*.*s\n", (i == menu->sel) ? "->" : "  ",
                                                         menu->cols_len[0], menu->drives[i]->devnode, 
                                                         menu->cols_len[1], menu->drives[i]->label,
                                                         menu->cols_len[2], menu->drives[i]->fs,
                                                         menu->cols_len[3], menu->drives[i]->size,
                                                         menu->cols_len[4], menu->cols_len[4], menu->drives[i]->mount_point);     
         fflush(stdout); // ensure the output is flushed to the terminal
    };
    for(int i = 0; i < term->cols; ++i){putchar('-'); };
    printf("\n[%d/%d] keys: ", menu->sel+1, menu->size);
    //Padding to align keys help to the right
    int padd = term->cols - ((menu->drives[menu->sel]->mounted) ? 0 : 8) - 
               ((menu->drives[menu->sel]->mounted) ? 10 : 0) -
               ((menu->drives[menu->sel]->devnode[6] != 'r') ? 12 : 8) - 
               10 - 6 - snprintf(NULL, 0, "%i", menu->sel) - 1 - snprintf(NULL, 0, "%i", menu->size);
    //printf("%*s", padd-2, "");
    printf("%s%s%s%s%s\033[K", (menu->drives[menu->sel]->mounted) ? "" : "(\033[1mm\033[0m)ount ",
                               (menu->drives[menu->sel]->mounted) ? "(\033[1mu\033[0m)nmount " : "",
                               (menu->drives[menu->sel]->devnode[6] != 'r') ? "(\033[1mp\033[0m)ower-off " : "(\033[1me\033[0m)ject ",
                               "(\033[1mr\033[0m)efresh ", "(\033[1mq\033[0m)uit\033[0J");
}

// █   █▀█ █▀▀ █▀▀
// █▄▄ █▄█ █▄█ ▄▄█
struct Logs {
    int    size;
    int    fill_ind;
    char **events; 
} typedef Logs;

void free_logs_events(Logs *logs){
    for (int i = 0; i < logs->fill_ind; ++i){
        free(logs->events[i]);
    };
    free(logs->events);
}

void print_logs(Logs *logs, const Term *term){
    printf("\033[%d;%dH\033[0J", term->hrow, 1); // Clear the menu
    if(logs->fill_ind < 1){return; }; // Nothing to print
    for (int i = 0; i < logs->fill_ind; ++i){puts(logs->events[i]); };
}

void log_event(Logs *logs, Drive *drive, const int action){
    char *action_text[] = {"mounted at ", "unmounted", "powered off", "ejected"};
    if (logs->fill_ind >= logs->size){}; // TODO: Allocate more memory to array if needed
    int str_len = strlen(drive->devnode) + 2 + strlen(drive->label) + 2 + 13 + ((action == 0) ? strlen(drive->mount_point) : 0);
    logs->events[logs->fill_ind] = calloc(str_len, sizeof(char));
    sprintf(logs->events[logs->fill_ind], "%s [%s] %s%s", drive->devnode, drive->label, action_text[action], (action == 0) ? drive->mount_point : "");
    ++logs->fill_ind;
}

void init_logs(Logs *logs){
    logs->size = 128;
    logs->fill_ind = 0;
    logs->events = malloc(sizeof(char*) * 128);
}

int main() {
    Menu_list *menu = calloc(1, sizeof(Menu_list));
    get_drives(menu);
    if (menu->size < 1){
        free(menu);
        puts("Nothing to see here -_-");
        return 0;
    };
    menu->sel = 0;

    Logs *logs = calloc(1, sizeof(Logs));
    init_logs(logs);
    term_mode(1); // Enable raw mode for input

    Term *term = calloc(1, sizeof(Term));
    get_term_size(term);
    if (term->rows < 7 || term->cols < 75){printf("Terminal window should be at least 7x75\n"); term_mode(0); return 1;};
    get_curs_pos(term);
    if (term->hrow == -1 || term->hcol == -1) {printf("Failed to get cursor position.\n"); term_mode(0); return 1; };

    calc_menu_param(menu, term);
    print_menu(menu, term);

    char ch; int result = 0;
    while (1) {
        ch = getchar();
        // Go up/down
        if (ch == 'k' && menu->sel > 0) {--menu->sel;
        } else if (ch == 'j' && menu->sel < menu->size - 1) {
            ++menu->sel;
            // Update highest selection for backward scrolling
            if (menu->top_sel < menu->sel){menu->top_sel = menu->sel; }; 
        } else if (ch == '\033') { // Parse arrow keys
            getchar(); ch = getchar(); // Skip [ and get actual code (A/B)
            if (ch == 'A' && menu->sel > 0) {--menu->sel;
            } else if (ch == 'B' && menu->sel < menu->size - 1) {
                ++menu->sel;
                // Update highest selection for backward scrolling
                if (menu->top_sel < menu->sel){menu->top_sel = menu->sel; };
            };
        // Mount drive
        } else if (ch == 'm' && !menu->drives[menu->sel]->mounted) {
            result = manage_drive(menu->drives[menu->sel]->devnode, 'm');
            get_drives(menu);
            if (result == 0){log_event(logs, menu->drives[menu->sel], 0); } else {err_mess(result, term); };
        // Unmount drive
        } else if (ch == 'u' && menu->drives[menu->sel]->mounted) {
            result = manage_drive(menu->drives[menu->sel]->devnode, 'u');
            if (result == 0){log_event(logs, menu->drives[menu->sel], 1); } else {err_mess(result, term); };
            get_drives(menu);
        // Power off drive
        } else if (ch == 'p') {
            if (menu->drives[menu->sel]->mounted){
                result = manage_drive(menu->drives[menu->sel]->devnode, 'u'); 
                if (result == 0){log_event(logs, menu->drives[menu->sel], 1); } else {err_mess(result, term); };
            };
            result = manage_drive(menu->drives[menu->sel]->devnode, 'p');
            if (result == 0){log_event(logs, menu->drives[menu->sel], 2); } else {err_mess(result, term); };
            get_drives(menu);
            calc_menu_param(menu, term);
        // Refresh list
        } else if (ch == 'r') {
            get_drives(menu);calc_menu_param(menu, term);
        // Eject cd drive
        } else if (ch == 'e' && menu->drives[menu->sel]->devnode[6] == 'r'){
            if (menu->drives[menu->sel]->mounted){
                result = manage_drive(menu->drives[menu->sel]->devnode, 'u'); 
                if (result == 0){log_event(logs, menu->drives[menu->sel], 1); } else {err_mess(result, term); };
            };
            result = manage_drive(menu->drives[menu->sel]->devnode, 'e');
            if (result == 0){log_event(logs, menu->drives[menu->sel], 3); } else {err_mess(result, term); };
            get_drives(menu);
            calc_menu_param(menu, term);
        // Quit
        } else if (ch == 'q') {break;
        };

        if (menu->size < 1){break;};
        print_menu(menu, term);
    };

    print_logs(logs, term);
    term_mode(0);

    free(term);
    free_drives(menu);
    free(menu);
    free_logs_events(logs);
    free(logs);

    return 0;
}
