# PartitionManager TUI
Basic TUI app which allows to mount, unmount, power-off connected storage devices. UI is heavily inspired by nmtui.

**Screenshot:**
![screenshoot-2024-03-30-18_25_28](https://github.com/vulpes-vulpeos/linux-utilities/assets/40931454/36a68eb6-3596-417a-b86b-180b5d1c8399)

**Dependencies:**
- udisks2
- eject from util-linux

**Compiling:**
1. Install libncurses-dev and libudev-dev:
   ```sudo apt install libncurses-dev libudev-dev```
2. Compile with:
   ```gcc -O2 "/path/to/pmtui.c" -o "/output/path/pmtui" -lncursesw -ludev```
