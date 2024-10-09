# PartitionManager TUI
Basic TUI app which allows you to mount, unmount, power-off connected storage devices.  
It displays it's menu inside the current terminal session. Also it prints actions log after exit.  

![screenshoot-2024-10-09_11-42-59](https://github.com/user-attachments/assets/8929a1fd-4c60-4221-9e49-eb48d424a7fd)  

**Dependencies:**
- libudev
- udisks2  
- eject from util-linux  

**Compiling:**
1. Install libudev:  
   ```sudo apt install libudev-dev```  
   ```sudo xbps-install eudev-libudev-devel```  
2. Compile with:  
   ```gcc -O2 "/path/to/pmtui.c" -o "/output/path/pmtui" -ludev```
