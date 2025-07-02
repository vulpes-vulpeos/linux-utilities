// https://mpd.readthedocs.io/en/stable/protocol.html#command-reference
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MPD_ADDR "127.0.0.1"
#define MPD_PORT 6600

void mpd_cur(int mpd_sock, char **tit, char **art){
    if (write(mpd_sock, "currentsong\n", 12) < 0) {
        return;
    };
    // Read response
    char buffer[1024] = {0};
    long resp_len = read(mpd_sock, buffer, sizeof(buffer) - 1);
    if (resp_len <= 0) {
        return;
    };

    char *line = strtok(buffer, "\n");
    while (line != NULL) {
        if (strncmp(line, "Artist: ", 8) == 0) {
            *art = strdup(line + 8);
        };
        if (strncmp(line, "Title: ", 7) == 0) {
            *tit = strdup(line + 7);
        };
        if (*art && *tit) {
            break;
        };

        line = strtok(NULL, "\n");
    };
}

int mpd_status(int mpd_sock){
    // Send status command
    if (write(mpd_sock, "status\n", 7) < 0) {
        return 0;
    };
    // Read response
    char buffer[1024] = {0};
    long resp_len = read(mpd_sock, buffer, sizeof(buffer) - 1);
    if (resp_len <= 0) {
        return 0;
    };
    if (strstr(buffer, "state: play")) {
        return 1;
    };

    return 0;
}

int mpd_connect(void) {
    // Create socket
    int mpd_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (mpd_sock < 0) {
        return -1;
    };
    // Setup server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(MPD_PORT);
    if (inet_pton(AF_INET, MPD_ADDR, &serv_addr.sin_addr) <= 0) {
        close(mpd_sock);
        return -1;
    };
    // Connect to MPD server
    if (connect(mpd_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(mpd_sock);
        return -1;
    };
    // Read initial OK MPD banner
    char buffer[1024];
    long resp_len = read(mpd_sock, buffer, sizeof(buffer) - 1);
    if (resp_len <= 0) {
        close(mpd_sock);
        return -1;
    };
    buffer[resp_len] = '\0';
    if (strncmp(buffer, "OK MPD", 6) != 0) {
        close(mpd_sock);
        return -1;
    };

    return mpd_sock;
}

int main(void) {
    // Connect to mpd
    int mpd_sock = mpd_connect();
    if (mpd_sock == -1){
        printf("  N/A\n");
        return 0;
    };

    int mpd_stat = mpd_status(mpd_sock);
    char *tit = NULL, *art = NULL;
    // Get playing status
    if (mpd_stat) {
        // Getting current track title - artist
        mpd_cur(mpd_sock, &tit, &art);
        if (!tit) {
            tit = strdup("Unknown title");
        };
        if (!art) {
            art = strdup("Unknown artist");
        };
        printf("  %s - %s\n", tit, art);

        free(art);
        free(tit);
    };

    close(mpd_sock);
    return 0;
}
