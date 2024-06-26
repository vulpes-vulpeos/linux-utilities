# SinkManager TUI
Basic TUI app which allows to switch sound output port. UI is heavily inspired by nmtui.

![screenshoot-2024-04-09-12_38_23](https://github.com/vulpes-vulpeos/linux-utilities/assets/40931454/626a8078-36b0-4742-9950-2547a303a5f5)

**Dependencies:**
- PulseAudio

**Compiling:**
1. Install ncurses, pulseaudio and alsa development libraries:
   
   ```sudo apt install libncurses-dev libpulse-dev libasound2-dev``` - Debian
   
   ```sudo zypper install ncurses-devel libpulse-devel alsa-devel``` - OpenSuse

   ```sudo dnf install ncurses-devel pulseaudio-libs-devel alsa-lib-devel``` - Fedora
   
3. Compile with:
   ```gcc -O2 "/path/to/smtui.c" -o "/output/path/smtui" -lncursesw -lpulse -lasound```

**Bugs:**
- It can crash if rapidly switch ports 3+ times.
