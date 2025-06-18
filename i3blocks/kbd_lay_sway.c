#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

//#define KBD_ID "1452:592:Apple_Inc._Apple_Keyboard"
#define KBD_ID "7847:24674:skyloong_GK61_Pro_DS_1.0.0\""
#define SWAY_MAGIC "i3-ipc"
#define IPC_GET_INPUTS 100


char *recv_ipc_reply(int sock, uint32_t *reply_type, uint32_t *reply_len) {
    char magic[6];
    if (read(sock, magic, 6) != 6 || strncmp(magic, SWAY_MAGIC, 6) != 0) { return NULL; };
    if (read(sock, reply_len, 4) != 4) { return NULL; };
    if (read(sock, reply_type, 4) != 4) { return NULL; };

    char *buf = malloc(*reply_len + 1);
    if (!buf) return NULL;
    if (read(sock, buf, *reply_len) != (ssize_t)(*reply_len)) { free(buf); return NULL; };
    buf[*reply_len] = '\0';
    return buf;
}

int send_ipc_request(int sock, uint32_t type, const void *payload, uint32_t length) {
    if (write(sock, SWAY_MAGIC, 6) != 6) { return -1; };
    if (write(sock, &length, 4) != 4) { return -1; };
    if (write(sock, &type, 4) != 4) { return -1; };
    if (length > 0 && write(sock, payload, length) != (ssize_t)length) { return -1; };
    return 0;
}

int main() {
    // get path to sway socket
    const char *sock_path = getenv("SWAYSOCK");
    if (!sock_path) { puts("N/A"); return 1; };
    // create socket
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) { puts("N/A"); return 1; };
    // set socket addr
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);
    // connect to socket and request data
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) { close(sock); puts("N/A"); return 1; };
    if (send_ipc_request(sock, IPC_GET_INPUTS, NULL, 0) != 0) { close(sock); puts("N/A"); return 1; };
    // get reply
    uint32_t reply_type, reply_len;
    char *reply = recv_ipc_reply(sock, &reply_type, &reply_len);
    close(sock);
    if (!reply) { puts("N/A"); return 1; };
    // find keyboard id
    char *kbd_ptr = strstr(reply, KBD_ID);
    // find "xkb_active_layout_name" location
    char *act_lay_ptr = strstr(kbd_ptr, "xkb_active_layout_name");
    if (act_lay_ptr == NULL) { puts("N/A"); return 1; };
    act_lay_ptr += 26;
    int sec_chr_ind = (act_lay_ptr - reply) +1;
    reply[sec_chr_ind] = toupper(reply[sec_chr_ind]);

    printf("󰘳  %.2s\n", act_lay_ptr);

    return 0;
}
