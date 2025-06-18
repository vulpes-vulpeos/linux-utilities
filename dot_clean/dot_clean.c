#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// What this utility considers as junk:
// Junk folders ".Trashes", ".Spotlight-V100", ".fseventsd",".TemporaryItems", ".DocumentRevisions-V100"
// Junk files: ".DS_Store" (0x42756431),".localized" (empty file), "._*" files (0x00051607)

void rrmdir(const char *dir_path) {
	DIR *dptr = opendir(dir_path);
	if(dptr == NULL) {
        fprintf(stderr, "ERROR: %s: ", dir_path); perror("");
        return;
    };

	struct dirent *entry;
    struct stat pstat;
    char *full_path;

	while ((entry = readdir(dptr)) != NULL) {
        // ignore itself and parent
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) { continue; };
        // construct full path for stat()
         if (asprintf(&full_path, "%s/%s", dir_path, entry->d_name) == -1) {
            fprintf(stderr, "ERROR: %s: ", full_path); perror("");
            closedir(dptr);
            return;
        };
        // return error if coudn't get file properties
		if(lstat(full_path, &pstat) == -1) {
            fprintf(stderr, "ERROR: %s: ", full_path); perror("");
            free(full_path);
            closedir(dptr);
            return;
        };
        // go into recurtion if path is a folder
		if (S_ISDIR(pstat.st_mode)) {
			rrmdir(full_path);
        // remove files inside folder
		} else {
			if (remove(full_path)) {
                fprintf(stderr, "ERROR: %s: ", full_path); perror("");
                free(full_path);
                closedir(dptr);
                return;
            };
		};

		free(full_path);
	};

	closedir(dptr);
    // folder should be empty, so delete it
	if (remove(dir_path)) {
        fprintf(stderr, "ERROR: %s: ", dir_path); perror("");
    };
}

int is_djunk (const char *const dir_name) {
           if (strcmp(dir_name, ".Trashes"               ) == 0) { return 1;
    } else if (strcmp(dir_name, ".Spotlight-V100"        ) == 0) { return 1;
    } else if (strcmp(dir_name, ".fseventsd"             ) == 0) { return 1;
    } else if (strcmp(dir_name, ".TemporaryItems"        ) == 0) { return 1;
    } else if (strcmp(dir_name, ".DocumentRevisions-V100") == 0) { return 1;
    };

    return 0;
};

int fempty(const char *const file_path) {
    struct stat st;
    if (lstat(file_path, &st) != 0) {
        fprintf(stderr, "ERROR: %s: ", file_path); perror("");
        return 0;
    };

    return st.st_size == 0;
}

//                             ._* file,   .DS_Store
uint32_t g_magic_numbers[2] = {0x00051607, 0x42756431};
uint32_t g_magic_offsets[2] = {0,          4};
int magchk(const char *const file_path, const uint32_t index) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        fprintf(stderr, "ERROR: %s: ", file_path); perror("");
        return 0;
    };

    if (g_magic_offsets[index] > 0 && fseek(file, g_magic_offsets[index], SEEK_CUR) != 0) {
        fprintf(stderr, "ERROR: %s: ", file_path); perror("");
        fclose(file);
        return 0;
    };

    uint32_t fmagic;
    if (fread(&fmagic, sizeof(fmagic), 1, file) != 1) {
        fprintf(stderr, "ERROR: %s: ", file_path); perror("");
        fclose(file);
        return 0;
    };

    fclose(file);

    // vise-versa endianness conversion
    // ._* files and .DS_Store are big-endian when X86, ARM64 are little-endian
    fmagic = ((fmagic >> 24) & 0xFF    ) | ((fmagic >> 8 ) & 0xFF00    ) |
             ((fmagic << 8 ) & 0xFF0000) | ((fmagic << 24) & 0xFF000000);

    return fmagic == g_magic_numbers[index];
}

int is_fjunk (const char *const full_path, const char *const file_name){
    // ._* file check
    if (file_name[1] == '_' && magchk(full_path, 0)){
        return 1;
    // .DS_Store check
    } else if (strcmp(file_name, ".DS_Store") == 0 && magchk(full_path, 1))  {
        return 1;
    // .localized check
    } else if (strcmp(file_name, ".localized") == 0 && fempty(full_path)) {
        return 1;
    };

    return 0;
};

void parse_folder(const char *const dir_path){
    DIR *dptr = opendir(dir_path);
    if(dptr == NULL) {
        fprintf(stderr, "ERROR: %s: ", dir_path); perror("");
        return;
    };

    struct dirent *entry;
    struct stat sb;
    char *full_path;

    while ((entry = readdir(dptr)) != NULL){
        // ignore itself and parent
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) { continue; };
        // construct full path for stat()
        if (asprintf(&full_path, "%s/%s", dir_path, entry->d_name) == -1) {
            fprintf(stderr, "ERROR: %s: ", full_path); perror("");
            break;
        };
        // get path status
        if (lstat(full_path, &sb) == -1) {
            fprintf(stderr, "ERROR: %s: ", full_path); perror("");
            free(full_path);
            break;
        };
        // check if path is a folder
        if (S_ISDIR(sb.st_mode)){
            // check if folder is junk
            if (entry->d_name[0] == '.' && is_djunk(entry->d_name)){
                printf("Detected junk folder: %s\n", entry->d_name);
                // recursively delete junk folder
                //rrmdir(full_path);
            // go into recursion if not junk
            } else {
                parse_folder(full_path);
            };
        // check if path is a junk file
        } else if (S_ISREG(sb.st_mode) && entry->d_name[0] == '.' && is_fjunk(full_path, entry->d_name)) {
            printf("Detected junk file: %s\n", entry->d_name);
            // delete junk file
            //remove(full_path);
        };

        free(full_path);
    };

	closedir(dptr);	
};

int main (int argc, char **argv){
    // checks
    if (argc < 2) {
        printf("Usage: dot_clean <DIR>...\n");
        return 0;
    };
    struct stat pstat;
    for (int ctr = 1; ctr < argc; ++ctr){
        if (lstat(argv[ctr], &pstat) != 0 && !S_ISDIR(pstat.st_mode)) {
            fprintf(stderr, "ERROR: %s: ", argv[ctr]); perror("");
            return 0;
        };
    };

    // parse recursively each provided path
    for (int ctr = 1; ctr < argc; ++ctr) {
        parse_folder(argv[ctr]);
    };

    return 0;
}
