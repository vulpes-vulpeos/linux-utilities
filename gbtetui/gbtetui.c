#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TILES 100
#define KEY_ESC 27

int tile_storage[MAX_TILES][64];
int cur_pos[2];
int cur_tile_index = 1;
int sb_cp = 4;
WINDOW *cur_tile;

void hextile_to_colors(const char hex_str[33]) {
    int ctr = 0;

    for (int row = 0; row < 8; row++) {
        // char to hex
        uint8_t lo = 0;
        uint8_t hi = 0;

        for (int i = 0; i < 2; i++) {
            char c = hex_str[(row * 4) + i];
            lo = (lo << 4) | (c >= '0' && c <= '9' ? c - '0' :c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : 0);
        };
        for (int i = 0; i < 2; i++) {
            char c = hex_str[(row * 4) + 2 + i];
            hi = (hi << 4) | (c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : 0);
        };

        // Pixels to colors
        for (int bit = 7; bit >= 0; bit--) { tile_storage[cur_tile_index][ctr++] = (((hi >> bit) & 1) << 1) | ((lo >> bit) & 1); };
    };
}

void tile_to_hex(char *tile_hex){
        int pos = 0;

    for (int y = 0; y < 8; ++y) {
        unsigned char lo = 0, hi = 0;

        for (int x = 0; x < 8; ++x) {
            unsigned char v = tile_storage[cur_tile_index][y * 8 + x] & 0x03;
            lo |= (v & 1) << (7 - x);        // bit 0
            hi |= ((v >> 1) & 1) << (7 - x); // bit 1
        };

        pos += sprintf(tile_hex + pos, "%02X %02X", lo, hi);
        if (y != 7) { tile_hex[pos++] = ' '; };
    };
}

static void do_sysclip_yank() {
    FILE *fp;
    const char *cmd;
    char hex[48] = {0};
    tile_to_hex(hex);
    if (getenv("WAYLAND_DISPLAY")) { cmd = "wl-copy"; } else { cmd = "xclip -selection clipboard -in"; };
    fp = popen(cmd, "w");
    if (!fp) { return; };
    fwrite(hex, 1, 47, fp);
    pclose(fp);
}

void draw_tile (WINDOW *window, int row, int col, int tile_index){
    int  draw_cursor = 0;
    for (int i = 0; i < 8; ++i) { for (int j = 0; j < 16; j+=2) {
        draw_cursor = (cur_pos[0] == i && cur_pos[1] == (j/2) && tile_index == cur_tile_index);
        wattron(window, COLOR_PAIR(tile_storage[tile_index][i*8+(j/2)]));
        mvwprintw(window, row+i, col+j,"%s", (draw_cursor) ? "[]" : "  ");
        wattroff(window, COLOR_PAIR(tile_storage[tile_index][i*8+(j/2)]));
    }; };
    if (tile_index != cur_tile_index) { mvwprintw(window, row+8, col,"%03d", tile_index); };
}

void draw_status_bar() {
    char tile_hex[48] = {0};
    tile_to_hex(tile_hex);

    wattron(stdscr, COLOR_PAIR(sb_cp));
    mvwprintw(stdscr, LINES-2, 0,"%-*s[%d,%d] [%0.3d] ", COLS-12, tile_hex, cur_pos[0], cur_pos[1], cur_tile_index);
    wattroff(stdscr, COLOR_PAIR(sb_cp));
}

void draw_color_palette() {
    wattron(stdscr, COLOR_PAIR(0)); mvwprintw(stdscr, 0, 0," 1"); wattroff(stdscr, COLOR_PAIR(0));
    wattron(stdscr, COLOR_PAIR(1)); mvwprintw(stdscr, 0, 3," 2"); wattroff(stdscr, COLOR_PAIR(1));
    wattron(stdscr, COLOR_PAIR(2)); mvwprintw(stdscr, 0, 6," 3"); wattroff(stdscr, COLOR_PAIR(2));
    wattron(stdscr, COLOR_PAIR(3)); mvwprintw(stdscr, 0, 9," 4"); wattroff(stdscr, COLOR_PAIR(3));
}

void ncurses_init() {
    setlocale(LC_ALL, "en_US.UTF-8");
    initscr();
    use_default_colors(); // Make default background and foreground available via -1 index
    cbreak();             // Disable buffering // Or raw();
    noecho();             // Disable showing of keys pressed
    keypad(stdscr, TRUE); // Enable keys like F1, F2, arrow keys etc.
    curs_set(FALSE);      // Don't display a cursor
    set_escdelay (100);   // Faster ESC key. 1000 - default
    puts("\033[?2004h");  // Enable brackated paste
    fflush(stdout);

    start_color(); // fg          bg
    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_BLACK, COLOR_GREEN);
    init_pair(2, COLOR_BLACK, COLOR_BLUE);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);

    init_pair(4, COLOR_BLACK, COLOR_CYAN);
    init_pair(5, COLOR_BLACK, COLOR_YELLOW);
    init_pair(6, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(7, COLOR_BLACK, COLOR_RED);

    wbkgd(stdscr, COLOR_PAIR(0));
}

void refresh_screen () {
    draw_status_bar();
    draw_tile(stdscr, 3, 1, cur_tile_index-1);
    int nx_tiles_ttl = (COLS - 35)/16;
    for (int i = 0; i < nx_tiles_ttl; ++i) {
        if (cur_tile_index+i+1 < MAX_TILES) { draw_tile(stdscr, 3, 35+(16*i), cur_tile_index+i+1);
        } else { draw_tile(stdscr, 3, 35+(16*i), 0); };
    };
    wrefresh(stdscr);
    box(cur_tile, 0 , 0);
    draw_tile(cur_tile, 1, 1, cur_tile_index);
    wrefresh(cur_tile);
}

static inline int is_hex_char(int ch) {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

int main (int argc, char *argv[]) {
    ncurses_init();
    if (LINES < 15 && COLS < 52) { puts("\033[?2004l"); endwin(); puts("Terminal window should be at least 15x52."); return 0; };

    memset(tile_storage, 0, MAX_TILES*64*sizeof(int)); // fill all tiles with 0 color

    draw_color_palette();
                    // h, w, row, col
    cur_tile = newwin(10, 18, 2, 17);
    wbkgd(cur_tile, COLOR_PAIR(0));
    refresh_screen();

    //wrefresh(stdscr);
    //wrefresh(cur_tile);

    int ch = 0, running = 1, pasting = 0, pb_len = 0;
    char paste_buff[42] = {0};
    while (running) {
        ch = getch();

        // switch-case for pasting
        if (pasting) {
            switch (ch) {
                case KEY_ESC: {
                    int a = getch();
                    if (a == '[') {
                        int b = getch(), c = getch(), d = getch();
                        if (b == '2' && c == '0' && d == '1' && getch() == '~') {
                            pasting = 0;
                            hextile_to_colors(paste_buff);
                            refresh_screen();
                            pb_len = 0;
                            break;
                        };
                    };
                    pasting = 0; // unexpected ESC seqence, aborting
                    pb_len = 0;
                    break; };
                default:
                    if (is_hex_char(ch)) { paste_buff[pb_len++] = tolower(ch); }; // get only hex chars
                    if (pb_len > 32) { pasting = 0; pb_len = 0;};
                    break;
            };
            continue;
        };

        switch (ch) {
            case 'i':
            case KEY_UP: // move cursor up
                if (cur_pos[0]-1 >=0) { --cur_pos[0]; refresh_screen(); }; break;
            case 'k':
            case KEY_DOWN: // move cursor down
                if (cur_pos[0]+1 < 8) { ++cur_pos[0]; refresh_screen(); }; break;
            case 'j':
            case KEY_LEFT: // move cursor left
                if (cur_pos[1]-1 >= 0) { --cur_pos[1]; refresh_screen(); }; break;
            case 'l':
            case KEY_RIGHT: // move cursor right
                if (cur_pos[1]+1 < 8) { ++cur_pos[1]; refresh_screen(); }; break;
            case 'J': // switch to prev tile
                if (cur_tile_index-1 > 0) { --cur_tile_index; refresh_screen(); }; break;
            case 'L': // switch to next tile
                if (cur_tile_index+1 < MAX_TILES) { ++cur_tile_index; refresh_screen(); }; break;
            case 'z': // fill pixel with color 0
                tile_storage[cur_tile_index][cur_pos[0]*8+cur_pos[1]] = 0; refresh_screen(); break;
            case 'x': // fill pixel with color 1
                tile_storage[cur_tile_index][cur_pos[0]*8+cur_pos[1]] = 1; refresh_screen(); break;
            case 'c': // fill pixel with color 2
                tile_storage[cur_tile_index][cur_pos[0]*8+cur_pos[1]] = 2; refresh_screen(); break;
            case 'v': // fill pixel with color 3
                tile_storage[cur_tile_index][cur_pos[0]*8+cur_pos[1]] = 3; refresh_screen(); break;
            case 'Z': // fill tile with color 0
                memset(tile_storage[cur_tile_index], 0, 64*sizeof(int)); refresh_screen(); break;
            case 'X': // fill tile with color 1
                memset(tile_storage[cur_tile_index], 1, 64*sizeof(int)); refresh_screen(); break;
            case 'C': // fill tile with color 2
                memset(tile_storage[cur_tile_index], 2, 64*sizeof(int)); refresh_screen(); break;
            case 'V': // fill tile with color 3
                memset(tile_storage[cur_tile_index], 3, 64*sizeof(int)); refresh_screen(); break;
            case 'Y': // copy to system clipboard
                do_sysclip_yank(); break;
            case 'q': // exit running loop
                running = 0; break;
            case KEY_ESC: { // receive ctrl+v paste
                int a = getch();
                if (a == '[') {
                    int b = getch(), c = getch(), d = getch();
                    // TODO: change statusbar color when in pasting mode?
                    if (b == '2' && c == '0' && d == '0' && getch() == '~') { pasting = 1; pb_len = 0; break; };
                }; break; };
            case KEY_RESIZE: // respond to change of sreen size
                refresh_screen(); break;
            default:
                break;
        };
    };

    puts("\033[?2004l"); // Disable brackated paste
    endwin();
    return 0;
}
