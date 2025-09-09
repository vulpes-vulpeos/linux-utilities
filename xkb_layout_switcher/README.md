# XKB Layout Switcher
A very simple and basic keyboard layout switcher for linux.


**Usage:**  
- Switch to next layout: ```xkb_layout_swticher```  
- Switch to first layout: ```xkb_layout_swticher 0```  
- Switch to third layout: ```xkb_layout_swticher 2```  

**Usecases:**
- Switch cyrillic layout to latin before locking session.
- Be able to switch keyboard layouts group by Super+Shift+Space and switch layouts by Super+Space.

**Compiling:**
Compile with: ```gcc -O2 "/path/to/xkb_layout_swticher.c" -o "/output/path/xkb_layout_swticher" -lX11```

# XKB Layouts Group Switcher

**Usecases:**
- You need English, Russian and Ukrainian layouts, but do not want to switch through 3 layouts.
- Be able to switch keyboard layouts group by Super+Shift+Space and switch layouts by Super+Space.

**Compiling:**
Compile with: ```gcc xkb_layout_group_switcher.c -o xkb_layout_group_switcher -lX11```
