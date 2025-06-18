# dot_clean  
Utility to remove macOS junk files from provided path.  
What this utility considers as junk:  
- Junk folders ".Trashes", ".Spotlight-V100", ".fseventsd",".TemporaryItems", ".DocumentRevisions-V100"  
- Junk files: ".DS_Store" (0x42756431),".localized" (empty file), "._*" files (0x00051607)  

WARNING: Uncomment 163 and 172 lines to really delete files.
