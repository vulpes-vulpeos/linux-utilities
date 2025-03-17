# wmenu-desktop
![screenshoot-2024-12-12_12-00-37](https://github.com/user-attachments/assets/cbecb04f-8c80-453d-9e74-28257a2b65ee)

In sway config set ```$menu``` to /path/to/wmenu_desktop/executable  
You can change colors inside source code.
dmenu_desktop parses this folders:  
- ~/.local/share/applications
- /usr/share/applications

**bash**  
Has a noticeable startup delay.

**C**  
Launches instantly, compared to bash version. Do not forget to **change username** to yours in ```/home/vulpeos/.local/share/applications``` path.  
Compile with: ```gcc /path/to/wmenu_desktop.c -o /path/to/wmenu_desktop```  

