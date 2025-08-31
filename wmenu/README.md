# wmenu-desktop
![screenshoot-2024-12-12_12-00-37](https://github.com/user-attachments/assets/cbecb04f-8c80-453d-9e74-28257a2b65ee)

You can change colors inside source code.

**bash**  
Has a noticeable startup delay. Don't forget to **change TERMINAL variable** if you're using a terminal other than Kitty.  
wmenu_desktop parses this folders:  
- ~/.local/share/applications  
- /usr/share/applications

Usage example: ```wmenu_desktop.sh```  

**C**  
Launches instantly compared to the Bash version. Don't forget to **change TERM define** if you're using a terminal other than Foot.  
Compile with: ```gcc /path/to/wmenu_desktop.c -o /path/to/wmenu_desktop```  
Usage example: ```wmenu_desktop /usr/share/applications ~/.local/share/applications```  
Supports multiple selections with ctrl+return.

# wmenu-mpd

![screenshoot-2025-03-17_11-32-21(1)](https://github.com/user-attachments/assets/afab388e-7c03-47f4-9164-9d494a0edbe0)

You can change colors inside source code.  
wmenu_mpd parses provided mpd.conf for music_directory path. After selection in wmenu it executes:  
`mpc -q stop -> mpc -q clear -> mpc -q add *selection* -> mpc -q play`. Supports multiple selections with ctrl+return.  

**Dependencies:**
- mpc

**Compiling:**   
Compile with: ```gcc /path/to/wmenu_mpd.c -o /path/to/wmenu_mpd```  

**Usage:**  
Usage example: ```wmenu_mpd ~/.config/mpd/mpd.conf```
