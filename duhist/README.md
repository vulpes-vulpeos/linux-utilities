# duhist
Prints out notifications from [dunst](https://github.com/dunst-project/dunst) history. Uses basic naive approach to extract data from dunst .json output.

In terminal it looks like this:  
**YYYY-MM-DD HH:MM** - **App name** - Summary  
Body  
\-\-\-\-\-  
**YYYY-MM-DD HH:MM** - **App name** - Summary  
Body  
\-\-\-\-\-  
**YYYY-MM-DD HH:MM** - **App name** - Summary  
Body  
\-\-\-\-\-  
[...]

**Dependencies:**
- [dunst](https://github.com/dunst-project/dunst)

**Compiling:**
- ```gcc "/path/to/duhist.c" -o "/output/path/duhist"```
