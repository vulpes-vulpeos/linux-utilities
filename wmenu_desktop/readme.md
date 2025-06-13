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
