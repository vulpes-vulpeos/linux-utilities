# System info notification
Script showing notification with system info. Usefull when running fullscreen apps/games.

You'll need to add keyboard identifier for keyboard layout indicator to work, or replace this part of the script if you use another WM/DE.

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
