#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>         // network
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>


#define ONLINE_URL "www.google.com"
#define ONLINE_PORT "80"

int get_loc_ip_lo(const char *iface) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { return -1; };
    // specify interface
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);
    // get IP
    if (ioctl(fd, SIOCGIFADDR, &ifr) == -1) { close(fd); return -1; };
    struct sockaddr_in *sa = (struct sockaddr_in *)&ifr.ifr_addr;
    unsigned char* ip_bytes = (unsigned char*)&sa->sin_addr.s_addr;
    close(fd);
    // return last octet
    return ip_bytes[3];
}

int get_sig_perc(const char *iface) {
    FILE *fp = fopen("/proc/net/wireless", "r");
    if (!fp) { return -100; };

    char line[256];
    int signal = -100;
    fgets(line, sizeof(line), fp); // skip first line
    fgets(line, sizeof(line), fp); // skip second line

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, iface)) {
            float sig;
            sscanf(line, "%*s %*f %f", &sig);
            signal = (int)sig;
            break;
        };
    };

    fclose(fp);

    signal = (signal * 100 + 70 / 2) / 70;  // normalize: 70 is a typical max
    if (signal > 100) { return 100;
    } else if (signal < 0) { return 0; };

    return signal;
}

char* get_ssid(const char *iface) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { return strdup("N/A"); };

    struct iwreq req = {0};
    strncpy(req.ifr_name, iface, IFNAMSIZ - 1);
    char buffer[IW_ESSID_MAX_SIZE + 1] = {0};
    req.u.essid.pointer = buffer;
    req.u.essid.length = IW_ESSID_MAX_SIZE + 1;
    req.u.essid.flags = 0;
    if (ioctl(sock, SIOCGIWESSID, &req) == -1) { close(sock); return strdup("N/A"); };

    char* ssid_str = calloc(IW_ESSID_MAX_SIZE + 1, sizeof(char));
    if(!ssid_str) { close(sock); return strdup("N/A"); };
    strcpy(ssid_str, buffer);
    close(sock);
    return ssid_str;
}

int is_online() {
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(ONLINE_URL, ONLINE_PORT, &hints, &res);
    if (ret == 0) { freeaddrinfo(res); return 1; };
    return 0;
}

int main(){
    FILE *fp = fopen("/proc/net/route", "r");
    if (!fp) { puts("N/A"); return 1; };

    char line[256];
    int  no_route = 1;
    while (fgets(line, sizeof(line), fp)) {
        char iface[IFNAMSIZ]; // replace with 16 to remove net/if.h?
        unsigned long dest, gateway;
        int flags;

        if (sscanf(line, "%s %lx %lx %X", iface, &dest, &gateway, &flags) == 4) {
            if (dest == 0) { // 0.0.0.0 means default route
                if (strcmp(iface, "lo") == 0) { break; };
                no_route = 0;
                int online_fl = is_online();
                char path[256];
                snprintf(path, sizeof(path), "/sys/class/net/%s/wireless", iface);
                // wireless
                if(access(path, F_OK) == 0) {
                    char* ssid = get_ssid(iface);
                    //printf("%s (%d%%) [.%d]", ssid, get_sig_perc(iface), get_loc_ip_lo(iface));
                    printf("%s  %s (%d%%)\n",(online_fl) ? "󰖩" : "󱚵",  ssid, get_sig_perc(iface));
                    free(ssid);
                    break;
                // ethernet
                } else {
                    printf("%s  .%d\n", (online_fl) ? "󰛳" : "󰅛", get_loc_ip_lo(iface));
                    break;
                };
            };
        };
    };

    if (no_route) {
        puts("󰅛  Offline");
    };

    fclose(fp);
    return 0;
}
