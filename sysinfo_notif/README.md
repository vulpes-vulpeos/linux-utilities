# System info notification
Script showing notification with system info. Usefull when running fullscreen apps/games.

## C version

<img width="307" height="111" alt="screenshot-2025-08-31-17_04_58" src="https://github.com/user-attachments/assets/e25eac37-0d13-4902-9691-51faab0050d8" />

**Dependencies:**
- dunst [notify-send] (target notification width and height = 300)
- alsa (for current volume)
- xlibre/xorg (for keyboard layout)
- mpd (for currently playing)

**Compile with:** ``` gcc sys_notif_str.c -o sys_notif_str -lasound -lX11```   
  
Add this into dunst config:  
```
# Settings for system notification
[sys_notif]
appname = sys_notif
format="%b"
history_ignore = yes
```

## Bash version  
You'll need to add keyboard identifier for keyboard layout indicator to work, or replace this part of the script if you so not use SwayWM.

![sysinfo_notif](https://github.com/vulpes-vulpeos/linux-utilities/assets/40931454/7bf524c5-9f31-4a75-bded-940a76099358)

**Dependencies:**
- mako [notify-send] (target notification width and height = 300).
- amixer (for current volume)
- sway (for keyboard layout)
- playerctl (for currently playing)
  
Add this into mako config:  
```
# Settings for system notification
[app-name=sysinfo_notif]  
history=0  
format=%b
``` 


