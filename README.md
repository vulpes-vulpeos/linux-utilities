# XKB Layout Switcher
A very simple and basic keyboard layout switcher for linux.

**Usage**
Switch to next layout: ```xkb_layout_swticher```
Switch to first layout: ```xkb_layout_swticher 0```
Switch to third layout: ```xkb_layout_swticher 2```

**Usecases**
- Switch cyrillic layout to latin before locking session.
- Be able to switch keyboard layouts group by Super+Shift+Space and switch layouts by Super+Space.

**Compiling:**
1. Install libx11-dev (sudo apt install libx11-dev).
2. Compile with: ```gcc -O2 "/path/to/xkb_layout_swticher.c" -o "/output/path/xkb_layout_swticher" -lX11```
