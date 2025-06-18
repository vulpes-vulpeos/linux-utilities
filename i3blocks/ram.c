#include <stdint.h>
#include <stdio.h>

int main(){
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) { puts("N/A\n"); return 0; };

    uint64_t mem_total = 0, mem_available;
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if (!mem_total && sscanf(line, "MemTotal: %ld kB", &mem_total) == 1) { continue; };
        if (sscanf(line, "MemAvailable: %ld kB", &mem_available) == 1) { break; };
    };
    fclose(fp);

    if (mem_total == 0 || mem_available == 0)  {
        // something went wrong
        puts("N/A\n");
        return 0;
    };

    // free -h formula
    printf("  %.1lfGiB\n", (double)(mem_total-mem_available) / (1024.0 * 1024.0));

    return 0;
}
