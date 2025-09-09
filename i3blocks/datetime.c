#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum mouse_buttons { DUMMY, BTNL, BTNM, BTNR, SCWLU, SCWLD, SCWLL, SCWLR, BTNF, BTNB };

static const char *dt_fmt = "%a, %d.%m.%Y";
static const char *tm_fmt = "%H:%M";

int main(){
    char* button = getenv("BLOCK_BUTTON");
    if (button != NULL) {
        switch (atoi(button)) {
            case BTNL: {
                char cmd_notif[128] = {0};
                snprintf(cmd_notif, sizeof(cmd_notif), "notify-send -t 5000 -h string:x-dunst-stack-tag:hist_ign -a 'hist_ign' \"%s\"", "$(cal)");
                system(cmd_notif);
                break; };
            default: break;
        };
    };

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char dt_str[64], tm_str[64];
    strftime(dt_str, sizeof(dt_str), dt_fmt, tm_info);
    strftime(tm_str, sizeof(tm_str), tm_fmt, tm_info);

    printf("󰃰  %s %s \n", dt_str, tm_str);

    return 0;
}
