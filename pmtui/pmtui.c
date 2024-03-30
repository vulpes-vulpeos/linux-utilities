#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <ncursesw/ncurses.h>
#include <libudev.h>

// Buttons defines
#define BTNS_NUM  6
#define BTN_MNT   1
#define BTN_UMNT  2
#define BTN_POFF  3
#define BTN_EJCT  4
#define BTN_REFR  5
#define BTN_EXIT  6

// Errors defines
#define ERR_BUSY  1
#define ERR_MNTD  2
#define ERR_USE   3
#define ERR_NMNTB 4
#define ERR_UKNWN 5

struct device {
    const char *devnode;     // dev path (/dev/sb*)
          char *size;        // size in number of 512 bytes sectors
    const char *id_fs_type;  // filesystem (ex. vfat)
    const char *id_fs_lbl;   // partition name
          int   mounted;     // mounted flag
} typedef device;

struct button {
    char name[14];
    int  disabled;
    int  row;
} typedef button;

void draw_bwin(WINDOW **bwin, int *bwin_foc, button (*btns_arr)[], int *bwin_sel){
    werase(*bwin);
    wbkgd(*bwin, COLOR_PAIR(2));
    // Printing buttons
    for (int i = 0, row_mod = 0; i < BTNS_NUM; ++i) {
        // Fixing position of button(s) after disabled button
        if ((*btns_arr)[i].disabled){
            ++row_mod; 
            continue;
        };
        // Resseting row mod for reset and exit buttons
        if (i > BTN_POFF){
            row_mod = 0;
        };
        // Highlighting selected button
        if (i+1 == *bwin_sel && *bwin_foc){
            wattron(*bwin, COLOR_PAIR(6));
        };
        mvwprintw(*bwin, (*btns_arr)[i].row - row_mod, 1, "%s", (*btns_arr)[i].name);
        wattroff(*bwin, COLOR_PAIR(6));
    };
    wrefresh(*bwin);
};

// Enable buttons based on device selected
void proc_lwin_sel(device ***list_arr, button (*btns_arr)[],int *lwin_sel, int *bwin_sel){
    (*btns_arr)[0].disabled = TRUE; // Mount
    (*btns_arr)[1].disabled = TRUE; // Unmount
    (*btns_arr)[2].disabled = TRUE; // Power off
    (*btns_arr)[3].disabled = TRUE; // Eject
    // Moving selection to refresh button if no storage device was found
    if ((*list_arr)[(*lwin_sel)-1]->devnode[0] != '/'){
        *bwin_sel = BTN_REFR;
        return;
    };
    // Enabling needed buttons
    if ((*list_arr)[(*lwin_sel)-1]->mounted){
        (*btns_arr)[1].disabled = FALSE;
        *bwin_sel = BTN_UMNT;
    } else {
        (*btns_arr)[0].disabled = FALSE;
        // Choosing between Poweroff and Eject buttons based on device devnode
        ((*list_arr)[(*lwin_sel)-1]->devnode[6] == 'r') ?
        ((*btns_arr)[3].disabled = FALSE) : ((*btns_arr)[2].disabled = FALSE);
        *bwin_sel = BTN_MNT;
    };
};

void draw_lwin(WINDOW **lwin, int *lwin_foc, device ***list_arr,
               int *list_arr_size, int *lwin_sel, int *lwin_max_sel){
    werase(*lwin);
    wbkgd(*lwin, COLOR_PAIR(2));
    // Highlighting window border
    if (*lwin_foc){
        wattron(*lwin, COLOR_PAIR(7));
    };
    box(*lwin, 0, 0);
    wattroff(*lwin, COLOR_PAIR(7));
    // List
    int lwin_lst_h = LINES-7-2;
    int start_ind = 0, col_pair = 6;
    // Scrolling math
    if (*list_arr_size > lwin_lst_h){
        // Forward scrolling
        if (*lwin_sel > lwin_lst_h){
            start_ind = *lwin_sel - lwin_lst_h;
        };
        // Backward scrolling
        if (*lwin_max_sel > *lwin_sel && (*lwin_max_sel - *lwin_sel) == lwin_lst_h){
            --*lwin_max_sel;
        };
        if (*lwin_max_sel > *lwin_sel && (*lwin_max_sel - *lwin_sel) < lwin_lst_h &&
            (*lwin_max_sel - lwin_lst_h) >= 0 ){
            start_ind = *lwin_max_sel - lwin_lst_h;
        };
    };
    
    for (int i = start_ind; i < ((*list_arr_size > lwin_lst_h) ? lwin_lst_h+start_ind : *list_arr_size); ++i){
        // Highlighting selected storage device
        if (i+1 == *lwin_sel){ 
            if (!(*lwin_foc)){
                // Unfocused selection highlight color
                col_pair = 7;
            };
            wattron(*lwin, COLOR_PAIR(col_pair));
            mvwhline(*lwin,i+1-start_ind, 2, ' ', COLS-24);
        };
        if ((*list_arr)[i]->devnode[0] != '/'){
            mvwprintw(*lwin, i+1-start_ind, 2, "%s", "No USB storage device found.");
        } else {
            mvwprintw(*lwin, i+1-start_ind, 2, "%s [%s] %s %s %s",(*list_arr)[i]->devnode,
                     (*list_arr)[i]->id_fs_lbl,(*list_arr)[i]->id_fs_type,
                     (*list_arr)[i]->size,(*list_arr)[i]->mounted ? "| Mounted": "");
        };
        wattroff(*lwin, COLOR_PAIR(col_pair));
    };
    // Drawing scrollbar if number of devices is bigger then list height
    if (*list_arr_size > lwin_lst_h){
        int sbar_h = 1, sbar_ind = 0;
        if(*lwin_sel == 1){ sbar_ind = 0;
        } else if (*lwin_sel == (*list_arr_size)){ sbar_ind = lwin_lst_h-sbar_h;
        } else { sbar_ind = ((*lwin_sel)-1)/2; };

        mvwvline(*lwin, 1, COLS-22, ACS_BOARD, lwin_lst_h);
        wattron(*lwin, COLOR_PAIR(3));
        mvwvline(*lwin, 1+sbar_ind, COLS-22, ' ', sbar_h);
        wattroff(*lwin, COLOR_PAIR(3));
    };
    wrefresh(*lwin);
};

// Draw error window. Any input removes it.
void draw_dwin(WINDOW **dwin, char **message){
    wbkgd(*dwin, COLOR_PAIR(8));
    box(*dwin, 0, 0);
    int msg_y = ((COLS-17)-strlen(*message))/2;
    mvwprintw(*dwin, 2, msg_y, "%s", *message);
    wrefresh(*dwin);
    getch();
};

void draw_mwin(WINDOW **mwin, char **app_name){
    int app_name_x = (COLS-4 - strlen(*app_name))/2;
    // Shadow
    werase(stdscr);
    wattron(stdscr, COLOR_PAIR(3));
    mvwvline(stdscr, 2, COLS-2, ACS_BLOCK, LINES-3);
    mvwhline(stdscr, LINES-2, 3, ACS_BLOCK, COLS-4);
    wattroff(stdscr, COLOR_PAIR(3));
    wrefresh(stdscr);
    // Window body
    werase(*mwin);
    wbkgd(*mwin, COLOR_PAIR(2));
    box(*mwin, 0, 0);
    // App name
    mvwprintw(*mwin, 0, app_name_x-1, "┤");
    wattron(*mwin, COLOR_PAIR(4));
    mvwprintw(*mwin, 0, app_name_x, "%s", *app_name);
    wattroff(*mwin, COLOR_PAIR(4));
    mvwprintw(*mwin, 0, app_name_x+strlen(*app_name), "├");
    
    wrefresh(*mwin);
};

void to_human_size(char **size_str){
    const char *suffixes[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
    int suffixIndex = 0;
    double size = (double)(atoll(*size_str) * 512);

    while (size >= 1024 && suffixIndex < 8) {size /= 1024; suffixIndex++;};
    sprintf(*size_str, "%.2f%s", size, suffixes[suffixIndex]);
}

int manage_device(const char ** devnode, int * bwin_sel){
    char *command = calloc(256,sizeof(char));
    switch (*bwin_sel) {
        case BTN_MNT: {
            sprintf(command, "udisksctl mount --no-user-interaction -b %s 2>&1", *devnode);
            break; };
        case BTN_UMNT: {
            sprintf(command, "udisksctl unmount --no-user-interaction -b %s 2>&1", *devnode);
            break; };
        case BTN_POFF: {
            sprintf(command, "udisksctl power-off --no-user-interaction -b %s 2>&1", *devnode);
            break; };
        case BTN_EJCT: {
            sprintf(command, "eject %s && sleep 0.2", *devnode);
            break; };
    };
    
    FILE *fp;
    char buffer[1024];
    int error = 0;
    fp = popen(command, "r");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "not a mount") != NULL) {
            error = ERR_NMNTB;
        };
        if (strstr(buffer, "Error") == NULL) {
            continue;
        };
        if (strstr(buffer, "DeviceBusy") != NULL){
            error = ERR_BUSY;
        } else if (strstr(buffer, "AlreadyMounted") != NULL){
            error = ERR_MNTD;
        } else if (strstr(buffer, "drive is in use") != NULL){
            error = ERR_USE;
        } else {
            error = ERR_UKNWN;
        };
    };
    pclose(fp);

    return error;
}

int prsnt_in_file(const char *path, const char **str){
    FILE *file_ptr = fopen(path, "r");
    char line[1024];
    while (fgets(line, sizeof(line), file_ptr) != NULL) {
        if (strstr(line, *str)){
            fclose(file_ptr);
            return TRUE;
        };
    };
    fclose(file_ptr);
    return FALSE;
}

int dev_valid(struct udev_device **dev){
    const char *devnode = udev_device_get_devnode(*dev);
    if (!udev_device_get_property_value(*dev, "ID_BUS")){
        return FALSE;
    };
    // Search only for USB drives (HDD wouldn't show up)
    //if (strcmp(udev_device_get_property_value(*dev, "ID_BUS"), "usb") != 0){
    //    return FALSE;
    //};
    if (!isdigit(devnode[strlen(devnode)-1])){return FALSE;};
    // Search if HDD present in fstab
    const char *id_fs_uuid = udev_device_get_property_value(*dev, "ID_FS_UUID");
    if (prsnt_in_file("/etc/fstab", &id_fs_uuid)){
        return FALSE;
    };
    if (udev_device_get_property_value(*dev, "ID_FS_TYPE") == NULL){
        return FALSE;
    };

    return TRUE;
}

void fill_list_arr(device ***list_arr, int *list_arr_size){
    struct udev *udev = udev_new();
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    struct udev_list_entry *devices, *dev_list_entry;

    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    // Geting list of devices
    devices = udev_enumerate_get_list_entry(enumerate);
    // Counting usb storage devices
    udev_list_entry_foreach(dev_list_entry, devices) {
        struct udev_device *dev = udev_device_new_from_syspath(udev, 
                                  udev_list_entry_get_name(dev_list_entry));
        if (dev_valid(&dev)) {++(*list_arr_size);};
        udev_device_unref(dev);
    };

    // Filling array with devices
    if(*list_arr_size != 0){
        *list_arr = malloc((*list_arr_size) * sizeof(device *));
        int index = 0;
        udev_list_entry_foreach(dev_list_entry, devices) {
            struct udev_device *dev = udev_device_new_from_syspath(udev, 
                                      udev_list_entry_get_name(dev_list_entry));
            if (dev_valid(&dev)) {
                (*list_arr)[index] = malloc(sizeof(device));
                // /dev/sd*
                (*list_arr)[index]->devnode = strdup(udev_device_get_devnode(dev));
                // volume label
                const char *vol_lbl = udev_device_get_property_value(dev, "ID_FS_LABEL");
                if (vol_lbl) {
                    (*list_arr)[index]->id_fs_lbl = strdup(vol_lbl);
                } else {
                    (*list_arr)[index]->id_fs_lbl = strdup("NO_NAME");
                };
                // filesystem (ex. vfat)
                (*list_arr)[index]->id_fs_type = strdup(udev_device_get_property_value(dev, "ID_FS_TYPE"));
                // size
                (*list_arr)[index]->size = strdup(udev_device_get_sysattr_value(dev, "size"));
                to_human_size(&(*list_arr)[index]->size);
                // is mounted?
                (*list_arr)[index]->mounted = prsnt_in_file("/proc/mounts", &(*list_arr)[index]->devnode);
                ++index;
            };
            udev_device_unref(dev);
        };
    } else {
        // Adding dummy device if nothing is connected
        *list_arr = malloc(sizeof(device));
        if (!list_arr){exit(0);};
        (*list_arr)[0] = (device*)calloc(1, sizeof(device));
        (*list_arr)[0]->devnode = strdup("dummy");
        ++(*list_arr_size);
    };

    // Cleanup
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

void ncurses_init(){
    setlocale(LC_ALL, "en_US.UTF-8");
    initscr();
    cbreak(); // Disable buffering // Or raw();
    noecho(); // Disable showing of keys pressed
    keypad(stdscr, TRUE); // Enable keys like F1, F2, arrow keys etc.
    curs_set(FALSE); //Don't display a cursor

    // Colors section
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_BLUE);  // Background
    init_pair(2, COLOR_BLACK, COLOR_WHITE); // Window
    init_pair(3, COLOR_BLACK, COLOR_BLACK); // Shadow
    init_pair(4, COLOR_RED,   COLOR_WHITE); // Window title
    init_pair(5, 15,          COLOR_WHITE); // Hidden text
    init_pair(6, COLOR_WHITE, COLOR_RED);   // Active selection
    init_pair(7, COLOR_WHITE, COLOR_BLUE);  // Nonactive selection
    init_pair(8, COLOR_BLACK, COLOR_RED);   // dialog background
}

int main() {
    // Getting devices
    int list_arr_size = 0;
    device **list_arr;
    fill_list_arr(&list_arr, &list_arr_size);
    // Initial drawing
    ncurses_init();
    // Exit if terminal window is too small
    if (COLS < 64 || LINES < 13){endwin();
                                printf("Terminal window should have at least 13 lines and 64 columns.\n");
                                return 0;};
    wbkgd(stdscr, COLOR_PAIR(1));
    wrefresh(stdscr);
    // Main window
    char *app_name = " PartitionManager TUI ";
    WINDOW *mwin = newwin(LINES-3, COLS-4, 1, 2); // h, w, x, y
    draw_mwin(&mwin, &app_name);
    // Variables for list and buttons windows
    button btns_arr[] = {{"[Mount]"    , FALSE, 1},
                         {"[Unmount]"  , FALSE, 2}, 
                         {"[Power off]", FALSE, 3},
                         {"[Eject]"    , FALSE, 4},
                         {"[Refresh]"  , FALSE, LINES-10},
                         {"[Exit]"     , FALSE, LINES-9}};

    char *err_msg[6] = {"OK",
                        "Device is busy.",
                        "Device is already mounted",
                        "Device is in use",
                        "Object is not a mountable filesystem.",
                        "Something went wrong."};
                        
    int bwin_foc = 0,
        bwin_sel = -1;
    int lwin_foc = TRUE,
        lwin_sel = 1,
        lwin_max_sel = lwin_sel;
    // Prepearing buttons for selected device
    proc_lwin_sel(&list_arr, &btns_arr, &lwin_sel, &bwin_sel);
    // List window
    WINDOW *lwin = newwin(LINES-7, COLS-20, 3, 4); // h, w, x, y
    draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
    // Buttons window
    WINDOW *bwin = newwin(LINES-7, 12, 3, COLS-16); // h, w, x, y
    draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);

    // Parsing keys input
    int ch = 0;
    while ((ch = getch()) != 'q') { // 'q' key to quit
        switch (ch) {
            case KEY_UP:
                if (lwin_foc) {
                    --lwin_sel;
                    if (lwin_sel < 1) {
                        lwin_sel = 1;
                    };
                    proc_lwin_sel(&list_arr, &btns_arr, &lwin_sel, &bwin_sel);
                    draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                    draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                };
                if (bwin_foc) {
                    --bwin_sel;
                    if (bwin_sel < 1) {
                        bwin_sel = BTNS_NUM;
                    };
                    while (btns_arr[bwin_sel-1].disabled){
                        --bwin_sel;
                        if (bwin_sel < 1) {
                            bwin_sel = BTNS_NUM;
                        };
                    };
                    draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                };
               
                break;
            case KEY_DOWN:
                if (lwin_foc) {
                    ++lwin_sel;
                    if (lwin_sel > list_arr_size) {
                        lwin_sel = list_arr_size;
                    };
                    if (lwin_max_sel < lwin_sel){
                        lwin_max_sel = lwin_sel;
                    };
                    proc_lwin_sel(&list_arr, &btns_arr, &lwin_sel, &bwin_sel);
                    draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                    draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                };
                if (bwin_foc) {
                    ++bwin_sel;
                    if (bwin_sel > BTNS_NUM) {
                        bwin_sel = 1;
                    };
                    while (btns_arr[bwin_sel-1].disabled){++bwin_sel;};
                    draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                };
                break;
            case KEY_LEFT:
                if (bwin_foc) {
                    bwin_foc = FALSE;
                    lwin_foc = TRUE;
                    draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                    draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                };
                break;
            case KEY_RIGHT:
                if (lwin_foc) {
                    lwin_foc = FALSE;
                    bwin_foc = TRUE;
                    draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                    draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                };
                break;

            case KEY_ENTER:
            case '\n':
                if (bwin_foc){
                    if (bwin_sel == BTN_MNT){
                        int result = manage_device(&list_arr[lwin_sel-1]->devnode, &bwin_sel);
                        if (result == 0){
                            list_arr[lwin_sel-1]->mounted = TRUE;
                            proc_lwin_sel(&list_arr, &btns_arr, &lwin_sel, &bwin_sel);
                        } else {
                            WINDOW *dwin = newwin(5, COLS-14, (LINES/2)-4, 7);
                            draw_dwin(&dwin, &err_msg[result]);
                            bwin_sel = BTN_REFR;
                        };
                        draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                        draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                    } else if (bwin_sel == BTN_UMNT){
                        int result = manage_device(&list_arr[lwin_sel-1]->devnode, &bwin_sel);
                        if (result == 0){
                            list_arr[lwin_sel-1]->mounted = FALSE;
                            proc_lwin_sel(&list_arr, &btns_arr, &lwin_sel, &bwin_sel);
                        } else {
                            WINDOW *dwin = newwin(5, COLS-14, (LINES/2)-4, 7);
                            draw_dwin(&dwin, &err_msg[result]);
                            bwin_sel = BTN_REFR;
                        };
                        draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                        draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                    };
                    
                    if (bwin_sel == BTN_POFF){
                        int result = manage_device(&list_arr[lwin_sel-1]->devnode, &bwin_sel);
                        if (result == 0){
                            bwin_sel = BTN_REFR;
                            lwin_sel = 1;
                        } else {
                            WINDOW *dwin = newwin(5, COLS-14, (LINES/2)-4, 7); // h, w, x, y
                            draw_dwin(&dwin, &err_msg[result]);
                            bwin_sel = BTN_REFR;
                        };
                        draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                        draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                    };
                    if (bwin_sel == BTN_EJCT){
                        int result = manage_device(&list_arr[lwin_sel-1]->devnode, &bwin_sel);
                        if (result == 0){
                            bwin_sel = BTN_REFR;
                            lwin_sel = 1;
                        } else {
                            WINDOW *dwin = newwin(5, COLS-14, (LINES/2)-4, 7); // h, w, x, y
                            draw_dwin(&dwin, &err_msg[result]);
                        };
                        draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                        draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                    };
                    if (bwin_sel == BTN_REFR){
                        list_arr_size = 0;
                        lwin_foc = TRUE;
                        bwin_foc = FALSE;
                        fill_list_arr(&list_arr, &list_arr_size);
                        proc_lwin_sel(&list_arr, &btns_arr, &lwin_sel, &bwin_sel);
                        draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                        draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                    };
                    if (bwin_sel == BTN_EXIT){
                        endwin(); // End the ncurses mode
                        return 0;
                    };
                } else if (lwin_foc) {
                    proc_lwin_sel(&list_arr, &btns_arr, &lwin_sel, &bwin_sel);
                    // Changing focus variables
                    lwin_foc = FALSE;
                    bwin_foc = TRUE;
                    draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                    draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                };
                break;
            case KEY_RESIZE:
                // recalculate parameters
                btns_arr[4].row = LINES-10;
                btns_arr[5].row = LINES-9;
                // resize windows
                wresize(mwin, LINES-3, COLS-4);
                wresize(lwin, LINES-7, COLS-20);
                wresize(bwin, LINES-7, 12);
                // Move button window to new place
                mvwin(bwin, 3, COLS-16);
                // redraw main window
                draw_mwin(&mwin, &app_name);
                draw_lwin(&lwin, &lwin_foc, &list_arr, &list_arr_size, &lwin_sel, &lwin_max_sel);
                draw_bwin(&bwin, &bwin_foc, &btns_arr, &bwin_sel);
                break;
        };
    };

    endwin(); // End the ncurses mode
    return 0;
}
