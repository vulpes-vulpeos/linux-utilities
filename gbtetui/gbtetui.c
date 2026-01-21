#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TILES 100
#define KEY_ESC 27

#define CUR_TILE_INDEX  (cur_pos[2] * 64 * MAX_TILES + cur_pos[3] * 64)
#define CUR_PIXEL_INDEX (cur_pos[2] * 64 * MAX_TILES + cur_pos[3] * 64 + cur_pos[0] * 8 + cur_pos[1])
#define DT_TILE_INDEX (line_index * 64 * MAX_TILES) + (tile_index * 64) + (i*8+(j/2))

int *tile_storage;
int cur_pos[4]; // row, col, line, tile
int ttl_lines = 1;
int sb_cp = 4;  // color pair index for statusbar

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
        for (int bit = 7; bit >= 0; bit--) { tile_storage[cur_pos[2] * MAX_TILES * 64 + cur_pos[3] * 64 + ctr++] = (((hi >> bit) & 1) << 1) | ((lo >> bit) & 1); };
    };
}

void tile_to_hex(char *tile_hex){
    int pos = 0;

    for (int y = 0; y < 8; ++y) {
        unsigned char lo = 0, hi = 0;

        for (int x = 0; x < 8; ++x) {
            unsigned char v = tile_storage[cur_pos[2] * MAX_TILES * 64 + cur_pos[3] * 64 + (y * 8 + x)] & 0x03;
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

void draw_tile (WINDOW *window, int row, int col, int line_index, int tile_index){
    // draw tile content
    int  draw_cursor = 0, is_sel = 0;
    for (int i = 0; i < 8; ++i) { for (int j = 0; j < 16; j+=2) {
        draw_cursor = (cur_pos[2] == line_index && cur_pos[0] == i && cur_pos[1] == (j/2) && tile_index == cur_pos[3]);
        wattron(window, COLOR_PAIR(tile_storage[DT_TILE_INDEX]));
        mvwprintw(window, row+i, col+j,"%s", (draw_cursor) ? "[]" : "  ");
        wattroff(window, COLOR_PAIR(tile_storage[DT_TILE_INDEX]));
    }; };
    // draw bottom separator
    is_sel = (cur_pos[2] == line_index && tile_index == cur_pos[3]);
    wattron(window, COLOR_PAIR((is_sel) ? 8: 7));
    mvwprintw(window, row+8, col,"%d:%03d           ", line_index+1, tile_index);
    wattroff(window, COLOR_PAIR((is_sel) ? 8: 7));
}

void draw_status_bar() {
    char tile_hex[48] = {0};
    tile_to_hex(tile_hex);

    wattron(stdscr, COLOR_PAIR(sb_cp));
    mvwprintw(stdscr, LINES-2, 0,"%-*s[%d,%d] [%d:%0.3d] ", COLS-14, tile_hex, cur_pos[0], cur_pos[1], cur_pos[2]+1, cur_pos[3]);
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
    init_pair(7, COLOR_WHITE, 8);
    init_pair(8, COLOR_BLACK, COLOR_WHITE);

    wbkgd(stdscr, COLOR_PAIR(0));
}

void refresh_screen () {
    draw_color_palette();
    clrtobot();

    int side_tiles_num = (COLS-18)/16/2;
    for (int line_ind = 0; line_ind < ttl_lines; ++line_ind) {
        int start_col = 1, start_row = 2 + (line_ind * 9);
        for (int tile_ind = cur_pos[3] - side_tiles_num; tile_ind < cur_pos[3]+side_tiles_num+1; ++tile_ind) {
            if (tile_ind > 0 && tile_ind < MAX_TILES) { draw_tile(stdscr, start_row, start_col, line_ind, tile_ind);
            } else { draw_tile(stdscr, start_row, start_col, line_ind, 0); };
            start_col += 16;
        };
    };
    draw_status_bar();
    wrefresh(stdscr);
}

static inline int is_hex_char(int ch) {
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

int main (int argc, char *argv[]) {
    ncurses_init();
    if (LINES < 15 || COLS < 50) { puts("\033[?2004l"); endwin(); puts("Terminal window should be at least 15x52."); return 0; };

    tile_storage = calloc(ttl_lines*MAX_TILES*64, sizeof(*tile_storage));
    cur_pos[0] = 0; cur_pos[1] = 0; cur_pos[2] = 0; cur_pos[3] = 1;

    refresh_screen();

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
            case 'I': // switch to upper tile
                if (cur_pos[2]-1 >= 0) { --cur_pos[2]; cur_pos[0] = 7; refresh_screen(); }; break;
            case 'K': // switch to lower tile
                if (cur_pos[2]+1 < ttl_lines) { ++cur_pos[2]; cur_pos[0] = 0; refresh_screen(); }; break;
            case 'J': // switch to prev tile
                if (cur_pos[3]-1 > 0) { --cur_pos[3]; cur_pos[1] = 7; refresh_screen(); }; break;
            case 'L': // switch to next tile
                if (cur_pos[3]+1 < MAX_TILES) { ++cur_pos[3]; cur_pos[1] = 0; refresh_screen(); }; break;
            case 'z': // fill pixel with color 0
                tile_storage[CUR_PIXEL_INDEX] = 0; refresh_screen(); break;
            case 'x': // fill pixel with color 1
                tile_storage[CUR_PIXEL_INDEX] = 1; refresh_screen(); break;
            case 'c': // fill pixel with color 2
                tile_storage[CUR_PIXEL_INDEX] = 2; refresh_screen(); break;
            case 'v': // fill pixel with color 3
                tile_storage[CUR_PIXEL_INDEX] = 3; refresh_screen(); break;
            case 'Z': // fill tile with color 0
                memset(&tile_storage[CUR_TILE_INDEX], 0, 64*sizeof(int)); refresh_screen(); break;
            case 'X': // fill tile with color 1
                memset(&tile_storage[CUR_TILE_INDEX], 1, 64*sizeof(int)); refresh_screen(); break;
            case 'C': // fill tile with color 2
                memset(&tile_storage[CUR_TILE_INDEX], 2, 64*sizeof(int)); refresh_screen(); break;
            case 'V': // fill tile with color 3
                memset(&tile_storage[CUR_TILE_INDEX], 3, 64*sizeof(int)); refresh_screen(); break;
            case '-': { // decrease number of tile lines
                if (ttl_lines-1 == 0) { break; };
                if (cur_pos[2] == ttl_lines-1 && cur_pos[2]-1 >= 0) { --cur_pos[2]; };
                --ttl_lines;
                refresh_screen();
                break; };
            case '+': { // increase number of tile lines
                if (LINES-4-((ttl_lines+1)*8) < 0) { break; };
                ++ttl_lines;
                ++cur_pos[2];
                int *tmp_tile_storage = realloc(tile_storage, ttl_lines*MAX_TILES*64*sizeof(*tile_storage));
                if (tmp_tile_storage == NULL) { break; }; // TODO: print allocation error under status bar
                tile_storage = tmp_tile_storage;
                memset(&tile_storage[(ttl_lines-1) * MAX_TILES * 64], 0, MAX_TILES * 64 * sizeof(*tile_storage));
                refresh_screen();
                break; };
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
