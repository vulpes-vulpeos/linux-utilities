# Linux utilities
**dmenu**  
- dmenu_desktop - A simple c utility which get list of apps formed from .desktop files and passes it to dmenu. After selection it executes selected app. Supports multiple selections with ctrl+return.  
- dmenu_mpd - A simple c utility which get list of music directories in mpd music directory and passes it to dmenu. After selection in dmenu it executes:  
`mpc -q stop -> mpc -q clear -> mpc -q add *selection* -> mpc -q play`. Supports multiple selections with ctrl+return.  

**dot_clean**  
Utility to remove macOS junk files from provided path. Read it's readme before use.

**duhist**  
Prints out notifications from [dunst](https://github.com/dunst-project/dunst) history. Do not work on systems without systemd.

**frank**  
Yet another snake game nobody needs.

**i3blocks/dwmblocks**  
CPU, RAM, network, alsa volume, keyboard layout (SwayWM,Xorg), mpd blocks for i3blocks/dwmblocks written in C.  

**PartitionManager TUI**  
Basic TUI app which allows to mount, unmount, power-off connected storage devices. Relies on udisks2.

**SinkManager TUI**  
Basic TUI app which allows to switch sound output port. UI is heavily inspired by nmtui. Requires PulseAudio.

**strblk**  
Prints provided text in several styles.

**system info notification**  
Bash script showing notification with system info. Usefull when running fullscreen apps/games.

**wmenu**  
- wmenu_desktop - A simple bash script and c utility which get list of apps formed from .desktop files and passes it to wmenu. After selection it executes selected app. Supports multiple selections with ctrl+return.  
- wmenu_mpd - A simple c utility which get list of music directories in mpd music directory and passes it to wmenu. After selection in wmenu it executes:  
`mpc -q stop -> mpc -q clear -> mpc -q add *selection* -> mpc -q play`. Supports multiple selections with ctrl+return.  

**XKB Layout Switcher**  
Minimal cli keyboard layout switcher for X11. Also there is layouts group switcher for X11 (us,ru <-> us,ua).  
