# Linux utilities
**PartitionManager TUI**  
Basic TUI app which allows to mount, unmount, power-off connected storage devices. Relies on udisks2.

**SinkManager TUI**  
Basic TUI app which allows to switch sound output port. UI is heavily inspired by nmtui. Requires PulseAudio.

**XKB Layout Switcher**  
A very simple and basic cli keyboard layout switcher for X11.

**System info notification**  
Bash script showing notification with system info. Usefull when running fullscreen apps/games.

**duhist**  
Prints out notifications from [dunst](https://github.com/dunst-project/dunst) history. Do not work on systems without systemd.

**Frank**  
Yet another snake game nobody needs.

**wmenu_desktop**  
A simple bash script and c utility which get list of apps formed from .desktop files and passes it to wmenu. After selection it executes selected app.

**wmenu_mpd**  
A simple c utility which get list of music directories in mpd music directory and passes it to wmenu. After selection in wmenu it executes:  
`mpc -q stop -> mpc -q clear -> mpc -q add *selection* -> mpc -q play`

**i3blocks**  
CPU, RAM, network, alsa volume, keyboard layout (SwayWM) blocks for i3blocks written in C.  

**dot_clean**  
Utility to remove macOS junk files from provided path. Read it's readme before use.

**strblk**  
Prints provided text in several styles.
