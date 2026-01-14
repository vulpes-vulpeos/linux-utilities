# Gameboy tile editor TUI
Basic TUI gameboy tile editor.

<img width="857" height="525" alt="screenshot-20260114-110046" src="https://github.com/user-attachments/assets/84da47cd-ae76-428f-99a6-b0a36f05a1b7" />

**Keybindings:** 
- j/k/i/l and arrow keys - move cursor.
- J/L - previous/next tile.
- z/x/c/v - fill "pixel" with 0/1/2/3 color.
- Z/X/C/V - fill tile with 0/1/2/3 color.
- Y - copy to system clipboard.
- Ctrl-v - paste from system clipboard.
- q - exit.

**Config variables:**
- MAX_TILES - set maximum number of tiles in row.

**TODO:**
- Add more rows.
- Receive color palette as arguments.

**Dependencies:**
- ncurses library
- xclip or wl-copy

**Compiling:**
1. Compile with:  
   ```gcc "/path/to/gbtetui.c" -o "/output/path/gbtetui" -lncurses```
