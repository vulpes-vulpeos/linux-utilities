# Gameboy tile editor TUI
Basic TUI gameboy tile editor.

<img width="860" height="526" alt="screenshot-20260121-154830" src="https://github.com/user-attachments/assets/c90d6883-8d7e-4925-81bb-10c41d34b7f4" />

**Keybindings:** 
- j/k/i/l and arrow keys - move cursor.
- J/K/I/L - previous/upper/lower/next tile.
- z/x/c/v - fill "pixel" with 0/1/2/3 color.
- Z/X/C/V - fill tile with 0/1/2/3 color.
- -/+ - remove/add line.
- Y - copy to system clipboard.
- t - toggle visibility of tile indexes.
- Ctrl-v - paste from system clipboard.
- q - exit.

**Config variables:**
- MAX_TILES - set maximum number of tiles in row.

**TODO:**
- (?) Receive color palette as arguments.

**Dependencies:**
- ncurses library
- xclip or wl-copy

**Compiling:**
1. Compile with:  
   ```gcc "/path/to/gbtetui.c" -o "/output/path/gbtetui" -lncurses```
