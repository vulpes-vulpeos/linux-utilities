# wmenu-mpd

![screenshoot-2025-03-17_11-32-21(1)](https://github.com/user-attachments/assets/afab388e-7c03-47f4-9164-9d494a0edbe0)

You can change colors inside source code.  
wmenu_mpd parses provided mpd.conf for music_directory path. After selection in wmenu it executes:  
`mpc -q stop -> mpc -q clear -> mpc -q add *selection* -> mpc -q play`

**Dependencies:**
- mpc

**Compiling:**   
Compile with: ```gcc /path/to/wmenu_mpd.c -o /path/to/wmenu_mpd```  

**Usage:**  
Usage example: ```wmenu_mpd ~/.config/mpd/mpd.conf```
