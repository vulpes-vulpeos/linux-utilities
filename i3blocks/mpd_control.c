#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum mouse_buttons { DUMMY, BTNL, BTNM, BTNR, SCWLU, SCWLD, SCWLL, SCWLR, BTNB, BTNF };
#define MPD_ADDR "127.0.0.1"
#define MPD_PORT 6600

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

char* get_mpd_str(void) {
    // Create socket
    int mpd_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (mpd_sock < 0) { return strdup("Err #001"); };
    // Setup server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(MPD_PORT);
    if (inet_pton(AF_INET, MPD_ADDR, &serv_addr.sin_addr) <= 0) { close(mpd_sock); return strdup("Err #002"); };
    // Connect to MPD server
    if (connect(mpd_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { close(mpd_sock); return strdup("Err #003"); };
    // Read initial OK MPD banner
    char buffer[1024] = { 0 };
    long resp_len = read(mpd_sock, buffer, sizeof(buffer) - 1);
    if (resp_len <= 0) { close(mpd_sock); return strdup("Err #004"); };
    if (strncmp(buffer, "OK MPD", 6) != 0) { close(mpd_sock); return strdup("Err #005"); };
    // Send status command
    if (write(mpd_sock, "status\n", 7) < 0) { close(mpd_sock); return strdup("Err #006"); };
    // Read response
    int mpd_stat = 0;
    memset(&buffer, 0, resp_len);
    resp_len = read(mpd_sock, buffer, sizeof(buffer) - 1);
    if (resp_len <= 0) { close(mpd_sock); return strdup("Err #007"); };
    // Parse status
    if (strstr(buffer, "state: play")) { mpd_stat = 1; };
    char *mpd_str = NULL, *tit = NULL, *art = NULL;
    if (mpd_stat) {
        // Get current song data
        if (write(mpd_sock, "currentsong\n", 12) < 0) { close(mpd_sock); return strdup("Err #008"); };
        // Read response
        memset(&buffer, 0, resp_len);
        resp_len = read(mpd_sock, buffer, sizeof(buffer) - 1);
        if (resp_len <= 0) {close(mpd_sock);  return strdup("Err #009"); };
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            if (strncmp(line, "Artist: ", 8) == 0) { art = strdup(line + 8); };
            if (strncmp(line, "Title: ", 7) == 0) { tit = strdup(line + 7); };
            if (art && tit) { break; };
            line = strtok(NULL, "\n");
        };
        if (!tit) { tit = strdup("Unknown title"); };
        if (!art) { art = strdup("Unknown artist"); };
        asprintf(&mpd_str, "%s\n%s", tit, art);
        // Cleanup
        free(art);
        free(tit);
    } else {
        mpd_str = NULL;
    };

    close(mpd_sock);
    return mpd_str;
}

int main() {
    char* button = getenv("BLOCK_BUTTON");
    if (button != NULL) {
        switch (atoi(button)) {
            case BTNL: system("mpc -q toggle"); break;
            case BTNB: system("mpc -q prev"); break;
            case BTNF: system("mpc -q next"); break;
            default: break;
        };
    };

    puts("  mpd");

    char cmd_notif[1024] = {0};
    char *mpd_str = get_mpd_str();
    if (!mpd_str) { return 0; };
    snprintf(cmd_notif, sizeof(cmd_notif), "notify-send -t 1000 -h string:x-dunst-stack-tag:cur_vol_notif -a 'cur_vol_notif' \"%s\"", mpd_str);
    system(cmd_notif);
    free(mpd_str);
    return 0;
}
