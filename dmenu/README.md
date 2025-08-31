# dmenu-desktop
![screenshoot-2024-12-12_12-00-37](https://github.com/user-attachments/assets/cbecb04f-8c80-453d-9e74-28257a2b65ee)

Don't forget to **change TERM define** if you're using a terminal other than Kitty. Supports multiple selections with ctrl+return.  

**Compiling:**  
Compile with: ```gcc /path/to/wmenu_desktop.c -o /path/to/wmenu_desktop```  

**Usage:**  
Usage example: ```wmenu_desktop /usr/share/applications ~/.local/share/applications```  


# dmenu-mpd
wmenu_mpd parses provided mpd.conf for music_directory path. After selection in wmenu it executes:  
`mpc -q stop -> mpc -q clear -> mpc -q add *selection* -> mpc -q play`. Supports multiple selections with ctrl+return.  

**Dependencies:**  
- mpc

**Compiling:**   
Compile with: ```gcc /path/to/wmenu_mpd.c -o /path/to/wmenu_mpd```  

**Usage:**  
Usage example: ```wmenu_mpd ~/.config/mpd/mpd.conf```
