#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    // CPU usage
    float cpuu = -1.0f;
    int cput = -1;
    uint64_t user1, nice1, system1, idle1, iowait1, irq1, softirq1, steal1,
             user2, nice2, system2, idle2, iowait2, irq2, softirq2, steal2,
             total_diff, idle_diff;

    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) { goto cpu_temp; };
    if (fscanf(fp, "cpu  %lu %lu %lu %lu %lu %lu %lu %lu", &user1, &nice1, &system1, &idle1, &iowait1, &irq1, &softirq1, &steal1) != 8) {
        fclose(fp); goto cpu_temp;};
    fclose(fp);

    usleep(300000); // 300 milliseconds

    fp = fopen("/proc/stat", "r");
    if (!fp) { goto cpu_temp; };
    if (fscanf(fp, "cpu  %lu %lu %lu %lu %lu %lu %lu %lu", &user2, &nice2, &system2, &idle2, &iowait2, &irq2, &softirq2, &steal2) != 8) {
        fclose(fp); goto cpu_temp;};
    fclose(fp);

    idle_diff = (idle2 + iowait2) - (idle1 + iowait1);
    total_diff = (user2 + nice2 + system2 + idle2 + iowait2 + irq2 + softirq2 + steal2) -
                 (user1 + nice1 + system1 + idle1 + iowait1 + irq1 + softirq1 + steal1);

    if (total_diff != 0) { cpuu = 100.0 * (total_diff - idle_diff) / total_diff; };
    // CPU temp
cpu_temp:
    cput = -1;
    for (int i = 0; ; ++i) {
        char buff[64];
        sprintf(buff, "/sys/class/thermal/thermal_zone%d/type", i);
        if (!(fp = fopen(buff, "r"))) { break; }; // No more thermal zones
        if (!fgets(buff, sizeof(buff), fp)) { fclose(fp); continue; }; // couldn't get type
        fclose(fp);
        if (strcmp(buff, "x86_pkg_temp\n") != 0) { continue; }; //wrong type
        snprintf(buff, sizeof(buff), "/sys/class/thermal/thermal_zone%d/temp", i);
        if (!(fp = fopen(buff, "r"))) {cput= -1; break; }; // couldn't get temp
        int cputm;
        if (fscanf(fp, "%d", &cputm) == 1) { cput = cputm / 1000; };
        fclose(fp);
        break; // got temp, break loop
    };

    printf("  %02.0f%%(%d°C)\n", cpuu, cput);

    return 0;
}
