#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum Direction {UP, DOWN, LEFT, RIGHT};
struct APPLE { int timer; int loc; } typedef APPLE;
struct SNAKE { int length; int *body; int grow_val; } typedef SNAKE;

int FCOLS, FLINS, FMAXIND;

int collision(int cell, SNAKE *frank){
    // Check borders
    if (cell < 0){return 1; };
    if (cell > FMAXIND){return 1; };
    int col = cell % FCOLS;
    if (col < 0 || col > FCOLS ){return 1; };
    // Check snake body
    if (frank->length > 4){
        for (int i = 1; i < frank->length; ++i){
            if(cell == frank->body[i]){return 1; };
        };
    };
    // Return false if everything ok
    return 0;
}

void move_snake(SNAKE *frank, int *cur_dir) {
    // Turn tail into head
    if (frank->length > 1){
        memmove(&frank->body[1], &frank->body[0], (frank->length - frank->grow_val) * sizeof(int));
        frank->body[0] = frank->body[1];
    };
    // Resetting growing if needed
    if(frank->grow_val == 0){frank->grow_val = 1; ++frank->length; };
    
    // Move head forward
    int col = frank->body[0] % FCOLS;
    switch(*cur_dir) {
        case UP:    frank->body[0] -= FCOLS; break;
        case DOWN:  frank->body[0] += FCOLS; break;
        case LEFT:  frank->body[0]  = ((col-2) < 0) ? -1 : frank->body[0]-2; break;
        case RIGHT: frank->body[0]  = ((col+2) >= FCOLS) ? -1 : frank->body[0]+2; break;
    };
}

char *get_eyes(int *cur_dir){
    switch (*cur_dir) {
        case UP:    return "⠁⠈"; break;
        case DOWN:  return "⡀⢀"; break;
        case LEFT:  return "⡁ "; break;
        case RIGHT: return " ⢈"; break;
        default:    return "XX"; break;
    };
}

void draw_field (WINDOW *field, SNAKE *frank, int *cur_dir, APPLE *apples){
    // Draw field
    werase(field);
    wattron(field, COLOR_PAIR(6));
    for (int i = 0; i < FLINS; ++i){
        for (int j = 0; j < FCOLS; j+=4){
           mvwaddstr(field, i, j, (i%2) ?"  ˵˶" : "˵˶  ");
        };
    };
    wattroff(field, COLOR_PAIR(6));
    //Draw snake
    if (frank == NULL){wrefresh(field); return; };
    wattron(field, COLOR_PAIR(2));
    // "/ COLS" => LINE, "% COLS" => COL
    for (int i = 0; i < frank->length; ++i){
        mvwprintw(field, frank->body[i] / FCOLS, frank->body[i] % FCOLS, "%s", (i == 0) ? get_eyes(cur_dir) :"▞▞");
    };
    wattroff(field, COLOR_PAIR(2));
    // Draw apples. Regular is always present, golden only if randomizer gives 777 
    wattron(field, COLOR_PAIR(3));
    mvwprintw(field, apples[0].loc / FCOLS, apples[0].loc % FCOLS, "%s", "◖◗");
    wattroff(field, COLOR_PAIR(3));
    if (apples[1].timer > 0){
        wattron(field, COLOR_PAIR(4));
        mvwprintw(field, apples[1].loc / FCOLS, apples[1].loc % FCOLS, "%s", "◖◗");
        wattroff(field, COLOR_PAIR(4));
    };
    wrefresh(field);
}

void create_apple(APPLE *apple, int *golden, SNAKE *frank){
    // Generating apple location
    do {apple->loc = random() % (FMAXIND - FCOLS + 1) + FCOLS;
    } while (collision(apple->loc, frank) || apple->loc % 2);
    if (!golden){return; };
    // Randomization for golden apple
    if ((random() % (1000 - 0 + 1) + 0) == 777){
        apple->timer = FCOLS*2;
    } else { apple->timer = -1; };
}

void free_snake(SNAKE *frank){
    free(frank->body);
    free(frank);
}

SNAKE *create_snake(){
    SNAKE *frank = calloc(1, sizeof(SNAKE));
    frank->length = 2;
    frank->body = calloc(FLINS*(FCOLS/2), sizeof(int));
    int start_row = FLINS/2, start_col = FCOLS/2;
    if(start_col % 2){++start_col; }; // Snake should always start in even column
    frank->body[0] = start_row*(FCOLS)+start_col;  
    frank->body[1] = frank->body[0]-2;
    frank->grow_val = 1;
    return frank;
}

void draw_frame ( WINDOW *dialog, int h, int w){
    wbkgd(dialog, COLOR_PAIR(8));
    wattron(dialog, COLOR_PAIR(9));
    wborder(dialog, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK);
    mvwvline(dialog, 1, 1, ACS_BLOCK, h-2);
    mvwvline(dialog, 1, w-2, ACS_BLOCK, h-2);
    wattroff(dialog, COLOR_PAIR(9));
}

void pause(int *score){
    mvwprintw(stdscr, 0, 2, "Score: %-*d", FCOLS-5, *score); wrefresh(stdscr);
    WINDOW *dialog = newwin(7, 40, (LINES-7)/2, (COLS-40)/2);
    draw_frame(dialog, 7, 40);
    mvwprintw(dialog, 2, 17, "PAUSED");
    mvwprintw(dialog, 4, 8, "Press [space] to unpause.");
    wrefresh(dialog);
    
    int ch = 0;
    nodelay(stdscr, FALSE); // Non-blocking input
    while ((ch = getch()) != ' ') {};
    mvwprintw(stdscr, 0, 2, "Score: %-*dPress [space] to pause", FCOLS-29, *score); wrefresh(stdscr);
    nodelay(stdscr, TRUE);
}

int game_over(int *score){
    mvwprintw(stdscr, 0, 2, "Score: %-*d", FCOLS-5, *score); wrefresh(stdscr);
    WINDOW *dialog = newwin(10, 40, (LINES-10)/2, (COLS-40)/2);
    wbkgd(dialog, COLOR_PAIR(8));
    draw_frame(dialog, 10, 40);
    mvwprintw(dialog, 2, 15, "GAME OVER");
    int col = (40 - 12 - snprintf(NULL, 0, "%d", *score))/2;
    mvwprintw(dialog, 4, col, "Your score: %d", *score);
    mvwprintw(dialog, 6, 5, "Press [space/enter] to restart");
    mvwprintw(dialog, 7, 12, "or [q] to exit.");
    wrefresh(dialog);
    
    int ch = 0;
    nodelay(stdscr, FALSE); // Non-blocking input
    while (1) {
        ch = getch();
        if(ch  == 'q'){return 1;
        } else if(ch == ' ' || ch == '\n'){return 0; };
    };
}

void ncurses_init(){
    setlocale(LC_ALL, "en_US.UTF-8");
    initscr();
    use_default_colors(); // Make default background and foreground available via -1 index
    cbreak();             // Disable buffering // Or raw();
    noecho();             // Disable showing of keys pressed
    keypad(stdscr, TRUE); // Enable keys like F1, F2, arrow keys etc.
    curs_set(FALSE);      // Don't display a cursor
    set_escdelay (500);   // Faster ESC key. 1000 - default

    // Colors section
    start_color();
    init_pair(1, 0, COLOR_GREEN);             // stdscr
    init_pair(2, COLOR_WHITE, 8);             // snake body
    init_pair(3, COLOR_RED, 10);              // red apple
    init_pair(4, COLOR_YELLOW, 10);           // golden apple
    init_pair(5, 10, COLOR_GREEN);            // dark green (fence)
    init_pair(6, COLOR_GREEN, 10);            // dark green (field)
    init_pair(7, 10, 10);                     // light green (field bg)
    init_pair(8, COLOR_BLACK, COLOR_YELLOW);  // dark yellow (dialog bg)
    init_pair(9, 11, 11);                     // light yellow (dialog bg)
}

int main() {
    ncurses_init();
    FLINS = LINES-2, FCOLS = (COLS%2)?COLS-5:COLS-4, FMAXIND = (FLINS*FCOLS)-1;
    
    // stdscr with score and help
    wbkgd(stdscr, COLOR_PAIR(1));
    mvwprintw(stdscr, 0, FCOLS-20,"Press [enter] to start");
    wrefresh(stdscr);

    // Playing field, snake and apples
    WINDOW *field = newwin(FLINS, FCOLS, 1, 2); // h, w, row, col
    wbkgd(field, COLOR_PAIR(7));
    SNAKE  *frank = NULL;
    int     cur_dir = RIGHT, score = 0;
    APPLE   apples[2];
    draw_field(field, frank, &cur_dir, apples);
    
    // Start game loop    
    int ch = 0, exit_loop = 0;
    while ((ch = getch())) {
        flushinp();
        switch (ch) {
            case '\n':
                mvwprintw(stdscr, 0, 2, "Score: %-*dPress [space] to pause", FCOLS-29,score);
                srandom(time(NULL));
                frank = create_snake();
                create_apple(&apples[0], &(int){0}, frank);
                create_apple(&apples[1], &(int){1}, frank);
                draw_field(field, frank, &cur_dir, apples);
                exit_loop = 1;
                nodelay(stdscr, TRUE); // Non-blocking input
                break;
            case 'q':
                delwin(field);
                endwin();
                return 0;
                break;
        };
        if (exit_loop){break; };
    };

    int reseed_ctr = 100, apple_val = 0, delay = 500; // 1000 = 1 sec    
    // Main game loop
    while ((ch = getch()) != 'q') { // 'q' key to quit
        flushinp();
        // Switch direction
        switch (ch) {
            case KEY_UP:    if (cur_dir != DOWN)  cur_dir = UP;    break;
            case KEY_DOWN:  if (cur_dir != UP)    cur_dir = DOWN;  break;
            case KEY_LEFT:  if (cur_dir != RIGHT) cur_dir = LEFT;  break;
            case KEY_RIGHT: if (cur_dir != LEFT)  cur_dir = RIGHT; break;
            case ' ':       pause(&score);                         break;
            default: break;
        };
        // Move snake
        move_snake(frank, &cur_dir);
        // Check if head is touching apple
        if(frank->body[0] == apples[0].loc){apple_val = 1;  delay -= 10; };
        if(delay < 100){delay = 100; }; // preventing delay from going too low
        if(frank->body[0] == apples[1].loc){apple_val = 10; apples[1].timer = 0; };
        if(apple_val != 0){
          score += apple_val;  // Increase score
          mvwprintw(stdscr, 0, 2, "Score: %d", score); // Updating score
          frank->grow_val = 0; // Set growing value (resets on next iteration in move_snake func)
          create_apple(&apples[(apple_val == 1)?0:1], &(int){(apple_val == 1)?0:1}, frank); // Recreating eaten apple
          apple_val = 0; // Reseting apple value variable
        };
        // Try to create golden apple if there is none or decrease timer of existing one
        if (apples[1].timer < 1){create_apple(&apples[1], &(int){0}, frank); 
        } else {--apples[1].timer; };
        // Redraw playing field
        draw_field(field, frank, &cur_dir, apples);
        // Check for collisions
        if (collision(frank->body[0], frank)){
          // Draw game over dialog
          if (!game_over(&score)){
              nodelay(stdscr, TRUE);
              // Restart game
              free_snake(frank);
              frank = create_snake();
              cur_dir = RIGHT;
              score = 0;
              // Redraw screen
              mvwprintw(stdscr, 0, 2, "Score: %-*dPress [space] to pause", FCOLS-29,score);
              wrefresh(stdscr);
              draw_field(field, frank, &cur_dir, apples);
          } else {
              free_snake(frank);
              delwin(field);
              endwin();
              return 0;
          };
        };
        // Resseeding randomizer every 100 moves
        --reseed_ctr;
        if (reseed_ctr == 0){srandom(time(NULL)); reseed_ctr = 100; };
        // Delay to reduce cpu usage
        napms(delay);
    }

    free_snake(frank);
    delwin(field);
    endwin();
    return 0;
}
