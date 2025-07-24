# strblk

Prints provided text in several styles:
```
Style 1 (3x2):
█ █ █▀▀ █   █   █▀█     █ █ █▀█ █▀█ █   █▀▄ █ 
█▀█ ██▄ █▄▄ █▄▄ █▄█ █   ███ █▄█ █▀▄ █▄▄ █▄▀ ▄ 

Style 2 (3x2):
⣿ ⣿ ⣾⣛⣛ ⣿   ⣿  ⣾⠛⣷     ⣿ ⣿ ⣾⠛⣷ ⣿⠛⣷ ⣿   ⣿⠛⣷ ⣿ 
⣿⠛⣿ ⢿⣭⣭ ⢿⣤⣤ ⢿⣤⣤ ⢿⣤⡿ ⣿   ⠻⣿⠟ ⢿⣤⡿ ⣿⠻⣯ ⢿⣤⣤ ⣿⣤⡿ ⣤ 

Style 3 (3x2):
█ █ 𜷥𜶺𜶺 █   █   𜷥🬂𜷤     █ █ 𜷥🬂𜷤 █🬂𜷤 █  █🬂𜷤 █ 
█🬂█ 𜶫𜷝𜷝 𜶫🬭🬭 𜶫🬭🬭  𜶫🬭𜵰 █   𜴦█𜴗 𜶫🬭𜵰 █𜴦𜷠 𜶫🬭🬭 █🬭𜵰 🬭 

Style 4 (2x2):
𜹥𜹺 𜹧𜹓 𜹥  𜹥  𜹦𜹹     𜹥 𜹺 𜹦𜹹 𜹧𜹹 𜹥  𜹧𜹹 𜹺 
𜹧𜹻 𜺇𜺃 𜺅𜺀 𜺅𜺀 𜹵𜹪 𜹴   𜹭𜹓𜹾 𜹵𜹪 𜹧𜹵 𜺅𜺀 𜺅𜹪 𜹲 

Style 5 (3x2):
𜰯 𜰯 𜰯𜰣𜰣 𜰯   𜰯   𜰯𜰣𜰯     𜰯 𜰯 𜰯𜰣𜰯 𜰯𜰣𜰭 𜰯   𜰯𜰣𜰬 𜰯 
𜰯𜰣𜰯 𜰯𜰯𜰬 𜰯𜰬𜰬 𜰯𜰬𜰬  𜰯𜰬𜰯 𜰯   𜰫𜰯𜰧 𜰯𜰬𜰯 𜰯𜰣𜰬 𜰯𜰬𜰬 𜰯𜰬𜰣 𜰬 
```
Useful for config header. Supported characters: ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890 .,'"!?()-_+=;:/\[]<>

**Compiling:**   
Compile with: ```gcc /path/to/strblk.c -o /path/to/strblk```  

**Usage:**  
Usage example: ```strblk 2 "Hello, world!"```
