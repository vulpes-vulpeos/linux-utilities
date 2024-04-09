# SinkManager TUI
Basic TUI app which allows to switch sound output port. UI is heavily inspired by nmtui.

![screenshoot-2024-04-09-12_38_23](https://github.com/vulpes-vulpeos/linux-utilities/assets/40931454/626a8078-36b0-4742-9950-2547a303a5f5)

**TODO:**
- Support for multiple sound cards.

**Dependencies:**
- PulseAudio

**Compiling:**
1. Install libncurses-dev and libpulse-dev:
   ```sudo apt install libncurses-dev libpulse-dev```
2. Compile with:
   ```gcc -O2 "/path/to/smtui.c" -o "/output/path/smtui" -lncursesw -lpulse```
