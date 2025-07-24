#include <stdio.h>
#include <string.h>

const char *g_style1[][27] ={
{"▄▀█", "██▄", "█▀▀", "█▀▄", "█▀▀", "█▀▀", "█▀▀", "█ █", "█", "  █", "█▄▀", "█  ", "███", "█▀▄", "█▀█", "█▀█", "█▀█", "█▀█", "█▀▀", "▀█▀", "█ █", "█ █", "█ █", "▀▄▀", "█ █", "▀▀█"},
{"█▀█", "█▄█", "█▄▄", "█▄▀", "██▄", "█▀▀", "█▄█", "█▀█", "█", "█▄█", "█▀▄", "█▄▄", "█ █", "█ █", "█▄█", "█▀▀", "▀▀█", "█▀▄", "▄▄█", " █ ", "▀▄█", "▀▄▀", "███", "▄▀▄", " █ ", "█▄▄"},

{" ", "▄▀█", "▄▀█", "▀▀▄", "▀██", "█ █", "█▀▀", "█▀▀", "▀▀█", "███", "███", " ", " ", "█", "█ █",  "█", "▀▀█", "▄▀", "▀▄", "▄▄▄", "   ", "▄█▄", "▀▀▀", "▀", "▀"},
{" ", "█▄▀", "  █", "██▄", "▄▄█", "▀▀█", "▄██", "███", "▄▀ ", "█▄█", "▄▄█", "▄", "█", " ", "   ",  "▄", " ▄ ", "▀▄", "▄▀", "   ", "▄▄▄", " ▀ ", "▀▀▀", "█", "▀"},

{" ▄▀", "▀▄ ", "█▀", "▀█", " ▄", "▄ ", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
{"▄▀ ", " ▀▄", "█▄", "▄█", "▀▄", "▄▀", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""}
};

const char *g_style2[][27] ={
{"⣾⠛⣿", "⣿⣻⡆", "⣾⠛⠛", "⣿⠛⣷", "⣾⣛⣛", "⣾⠛⠛", "⣾⠛⠛", "⣿ ⣿", "⣿", "  ⣿", "⣿⣴⠟", "⣿  ", "⣿⡶⣿", "⣿⣄⣿", "⣾⠛⣷", "⣿⠛⣷", "⣾⠛⣿", "⣿⠛⣷", "⣾⣛⡛", "⠛⣿⠛", "⣿ ⣿", "⣿ ⣿", "⣿ ⣿", "⠻⣶⠟", "⣿ ⣿", "⠛⣻⡿"},
{"⣿⠛⣿", "⣿⣭⡿", "⢿⣤⣤", "⣿⣤⡿", "⢿⣭⣭", "⣿⠛⠛", "⢿⣤⣿", "⣿⠛⣿", "⣿", "⢿⣤⡿", "⣿⠻⣦", "⢿⣤⣤", "⣿ ⣿", "⣿⠙⣿", "⢿⣤⡿", "⣿⠛⠋", "⠙⠛⣿", "⣿⠻⣯", "⣬⣭⡿", " ⣿ ", "⢿⣤⡿", "⠻⣤⠟", "⠻⣿⠟", "⣴⠿⣦", "⠈⣿⠁", "⣾⣯⣤"},

{" ", "⣾⢛⣷", "⣴⠟⣿", "⠛⢛⡷", "⣛⣛⣷", "⣿ ⣿", "⣿⣛⡛", "⣾⣛⡛", "⠛⢛⡿", "⢾⣛⡷", "⣾⣛⣷", " ", " ", "⣿", "⣿ ⣿", "⣿", "⠾⢛⡷", "⣴⠟", "⠻⣦", "⣀⣀⣀", "   ", "⣀⣶⣀", "⠶⠶⠶", "⠶", "⠶"},
{" ", "⢿⣥⡿", "  ⣿", "⣴⣯⣤", "⣭⣭⡿", "⠙⠛⣿", "⣬⣭⡿", "⢿⣭⡿", "⡴⠋ ", "⢾⣭⡷", "⣬⣭⡿", "⣤", "⣿", " ", "   ", "⣤", " ⣭ ", "⠻⣦", "⣴⠟", "⠉⠉⠉", "⣤⣤⣤", "⠉⠿⠉", "⠶⠶⠶", "⣶", "⠶"},

{" ⣠⡾ ", "⢷⣄ ", "⣿⠛", "⠛⣿", "⣠⡾", "⢷⣄", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
{"⡾⠋  ", " ⠙⢷", "⣿⣤", "⣤⣿", "⠙⢷", "⡾⠋", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""}
};


const char *g_style3[][27] ={
{"𜷡𜴗█", "█𜷚𜵈", "𜷡𜴗▀", "█🬂𜷤", "𜷥𜶺𜶺", "𜷥🬂🬂", "𜷥🬂🬂", "█ █", "█", "  █", "█𜷡𜴗", "█  ", "█𜵭█", "█𜶻█", "𜷥🬂𜷤", "█🬂𜷤", "𜷥🬂█", "█🬂𜷤", "𜷥𜶺𜵄", "🬂█🬂", "█ █", "█ █", "█ █", "𜴦🬹𜴗", "█ █", "▀𜷚𜵰"},
{"█▀█", "█𜷝𜵰", "𜴦𜷞▄", "█🬭𜵰", "𜶫𜷝𜷝", "█🬂🬂", "𜶫🬭█", "█🬂█", "█", "𜶫🬭𜵰", "█𜴦𜷞", "𜶫🬭🬭", "█ █", "█🬁█", "𜶫🬭𜵰", "█🬂🬀", "🬁🬂█", "█𜴦𜷠", "𜷜𜷝𜵰", " █ ", "𜶫🬭𜵰", "𜴦▄𜴗", "𜴦█𜴗", "𜷡🬎𜷞", "🬁█🬀", "𜷥𜷠▄"},

{" ", "𜷥𜵿𜷤", "𜷡𜴗█", "▀𜵿𜵮", "𜶺𜶺𜷤", "█ █", "█𜶺𜵄", "𜷥𜶺𜵄", "▀𜵿𜵰", "𜶪𜶺𜵮", "𜷥𜶺𜷤", " ", " ", "█", "█ █", "█", "⠾𜵿𜵮", "𜷡𜴗", "𜴦𜷞", "🬭🬭🬭", "   ", "🬭🬹🬭", "𜴳𜴳𜴳", "𜴳", "𜴳"},
{" ", "𜶫𜷛𜵰", "  █", "𜷡𜷠▄", "𜷝𜷝𜵰", "🬁🬂█", "𜷜𜷝𜵰", "𜶫𜷝𜵰", "𜵩𜴂 ", "𜶪𜷝𜵮", "𜷜𜷝𜵰", "🬭", "█", " ", "   ", "🬭", " 𜷝 ", "𜴦𜷞", "𜷡𜴗", "🬂🬂🬂", "🬭🬭🬭", "🬂🬎🬂", "𜴳𜴳𜴳", "🬹", "𜴳"},

{" 𜷋𜵯 ", "𜶩𜶻 ", "█🬂", "🬂█", "𜷋𜴖", "𜴤𜶻", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
{"𜵯𜴂  ", " 𜴅𜶩", "█🬭", "🬭█", "𜴅𜶠", "𜵩𜴂", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""}
};

const char *g_style4[][27] ={
{"𜹦𜹻", "𜹧𜹤", "𜹦𜹓", "𜹧𜹹", "𜹧𜹓", "𜹧𜹓", "𜹦𜹓", "𜹥𜹺", "𜹺", " 𜹺", "𜹥𜹪", "𜹥 ", "𜹭𜺀𜹾", "𜺅𜹺", "𜹦𜹹", "𜹧𜹹", "𜹦𜹻", "𜹧𜹹", "𜹶𜹓", "𜹓𜹧", "𜹥𜹺", "𜹥𜹺", "𜹥 𜹺", "𜹥𜹺", "𜹥𜹺", "𜹓𜹫"},
{"𜹧𜹻", "𜺇𜺉", "𜹵𜺀", "𜺅𜹪", "𜺇𜺃", "𜹧𜹓", "𜹵𜺋", "𜹧𜹻", "𜹺", "𜹵𜺊", "𜹧𜹹", "𜺅𜺀", "𜹥 𜹺", "𜹥𜹻", "𜹵𜹪", "𜹧𜹑", "𜹒𜹻", "𜹧𜹵", "𜺀𜹩", " 𜹥", "𜹵𜹪", "𜹙𜺊", "𜹭𜹓𜹾", "𜹦𜹹", "𜹒𜹥", "𜺆𜺀"},

{" ", "𜹦𜺉", "𜹾", "𜹖𜹩", "𜹖𜺉", "𜹨𜹻", "𜺇𜹣", "𜹦𜹙", "𜹓𜹫", "𜹶𜹩", "𜹶𜺉", " ", " ", "𜹚", "𜹟", "𜹺", "𜹖𜹩", "𜹦", "𜹹", "  ", "  ", "𜹰 ", "𜺀𜺀", "𜹰", "𜹰"},
{" ", "𜹷𜹪", "𜹺", "𜺆𜺀", "𜹴𜹫", "𜹓𜹻", "𜹴𜹪", "𜹷𜹩", "𜹦 ", "𜹵𜹪", "𜹴𜹪", "𜹰", "𜹴", " ", " ", "𜹲", "𜹲 ", "𜹵", "𜹪", "𜹓𜹓", "𜺀𜺀", "𜹛𜹑", "𜹜𜹜", "𜹸", "𜹘"},

{" 𜹪", "𜹵 ", "𜹧", "𜹻", "𜹨𜹑", "𜹒𜹴", "𜺉𜺍𜹡", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
{"𜹦 ", " 𜹹", "𜺅", "𜺊", "𜹙𜹠", "𜹰𜹖", "𜹖𜹗𜹔", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""}
};

const char *g_style5[][27] ={
{"𜰬𜰣𜰯", "𜰯𜰯𜰬", "𜰮𜰣𜰣", "𜰯𜰣𜰬", "𜰯𜰣𜰣", "𜰯𜰣𜰣", "𜰮𜰣𜰣", "𜰯 𜰯", "𜰯", "  𜰯", "𜰯𜰬𜰣", "𜰯  ", "𜰯𜰯𜰯", "𜰯𜰣𜰬", "𜰯𜰣𜰯", "𜰯𜰣𜰯", "𜰯𜰣𜰯", "𜰯𜰣𜰭", "𜰮𜰣𜰣", "𜰣𜰯𜰣", "𜰯 𜰯", "𜰯 𜰯", "𜰯 𜰯", "𜰫𜰬𜰧", "𜰯 𜰯", "𜰣𜰣𜰯"},
{"𜰯𜰣𜰯", "𜰯𜰬𜰯", "𜰫𜰬𜰬", "𜰯𜰬𜰣", "𜰯𜰯𜰬", "𜰯𜰣𜰣", "𜰫𜰬𜰯", "𜰯𜰣𜰯", "𜰯", "𜰯𜰬𜰯", "𜰯𜰣𜰬", "𜰯𜰬𜰬", "𜰯 𜰯", "𜰯 𜰯", "𜰯𜰬𜰯", "𜰯𜰣𜰣", "𜰣𜰣𜰯", "𜰯𜰣𜰬", "𜰬𜰬𜰧", " 𜰯 ", "𜰫𜰬𜰯", "𜰢𜰯𜰡", "𜰫𜰯𜰧", "𜰮𜰣𜰭", " 𜰯 ", "𜰯𜰬𜰬"},

{" ", "𜰬𜰣𜰭", "𜰬𜰣𜰯", "𜰣𜰫𜰭", "𜰣𜰯𜰯", "𜰯 𜰯", "𜰯𜰣𜰣", "𜰮𜰣𜰣", "𜰣𜰣𜰯", "𜰮𜰯𜰭", "𜰮𜰯𜰭", " ", " ", "𜰯", "𜰯 𜰯",  "𜰯", "𜰦𜰣𜰭", "𜰮𜰣", "𜰣𜰭", "𜰬𜰬𜰬", "   ", "𜰬𜰯𜰬", "𜰣𜰣𜰣", "𜰣", "𜰣"},
{" ", "𜰫𜰬𜰣", "  𜰯", "𜰮𜰯𜰬", "𜰬𜰬𜰧", "𜰢𜰣𜰯", "𜰬𜰯𜰧", "𜰫𜰯𜰧", "𜰬𜰣 ", "𜰫𜰬𜰧", "𜰬𜰬𜰧", "𜰬", "𜰯", " ", "   ",  "𜰬", " 𜰬 ", "𜰫𜰬", "𜰬𜰧", "   ", "𜰬𜰬𜰬", " 𜰣 ", "𜰣𜰣𜰣", "𜰯", "𜰣"},

{" 𜰨𜰧", "𜰫𜰤 ", "𜰯𜰣", "𜰣𜰯", " 𜰬", "𜰬 ", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
{"𜰨𜰧 ", " 𜰫𜰤", "𜰯𜰬", "𜰬𜰯", "𜰣𜰬", "𜰬𜰣", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""}
};

void get_index(char in_char, int *row, int *col){
    switch (in_char) {
        case 'a': case 'A': *row = 0; *col = 0;  break;
        case 'b': case 'B': *row = 0; *col = 1;  break;
        case 'c': case 'C': *row = 0; *col = 2;  break;
        case 'd': case 'D': *row = 0; *col = 3;  break;
        case 'e': case 'E': *row = 0; *col = 4;  break;
        case 'f': case 'F': *row = 0; *col = 5;  break;
        case 'g': case 'G': *row = 0; *col = 6;  break;
        case 'h': case 'H': *row = 0; *col = 7;  break;
        case 'i': case 'I': *row = 0; *col = 8;  break;
        case 'j': case 'J': *row = 0; *col = 9;  break;
        case 'k': case 'K': *row = 0; *col = 10; break;
        case 'l': case 'L': *row = 0; *col = 11; break;
        case 'm': case 'M': *row = 0; *col = 12; break;
        case 'n': case 'N': *row = 0; *col = 13; break;
        case 'o': case 'O': *row = 0; *col = 14; break;
        case 'p': case 'P': *row = 0; *col = 15; break;
        case 'q': case 'Q': *row = 0; *col = 16; break;
        case 'r': case 'R': *row = 0; *col = 17; break;
        case 's': case 'S': *row = 0; *col = 18; break;
        case 't': case 'T': *row = 0; *col = 19; break;
        case 'u': case 'U': *row = 0; *col = 20; break;
        case 'v': case 'V': *row = 0; *col = 21; break;
        case 'w': case 'W': *row = 0; *col = 22; break;
        case 'x': case 'X': *row = 0; *col = 23; break;
        case 'y': case 'Y': *row = 0; *col = 24; break;
        case 'z': case 'Z': *row = 0; *col = 25; break;

        case ' ':  *row = 2; *col = 0;  break;
        case '0':  *row = 2; *col = 1;  break;
        case '1':  *row = 2; *col = 2;  break;
        case '2':  *row = 2; *col = 3;  break;
        case '3':  *row = 2; *col = 4;  break;
        case '4':  *row = 2; *col = 5;  break;
        case '5':  *row = 2; *col = 6;  break;
        case '6':  *row = 2; *col = 7;  break;
        case '7':  *row = 2; *col = 8;  break;
        case '8':  *row = 2; *col = 9;  break;
        case '9':  *row = 2; *col = 10; break;
        case '.':  *row = 2; *col = 11; break;
        case ',':  *row = 2; *col = 12; break;
        case '\'': *row = 2; *col = 13; break;
        case '"':  *row = 2; *col = 14; break;
        case '!':  *row = 2; *col = 15; break;
        case '?':  *row = 2; *col = 16; break;
        case '(':  *row = 2; *col = 17; break;
        case ')':  *row = 2; *col = 18; break;
        case '-':  *row = 2; *col = 19; break;
        case '_':  *row = 2; *col = 20; break;
        case '+':  *row = 2; *col = 21; break;
        case '=':  *row = 2; *col = 22; break;
        case ';':  *row = 2; *col = 23; break;
        case ':':  *row = 2; *col = 24; break;

        case '/':  *row = 4; *col = 0;  break;
        case '\\': *row = 4; *col = 1;  break;
        case '[':  *row = 4; *col = 2;  break;
        case ']':  *row = 4; *col = 3;  break;
        case '<':  *row = 4; *col = 4;  break;
        case '>':  *row = 4; *col = 5;  break;
        //case '*':  *row = 4; *col = 6;  break;

        default: *row = -1; *col = -1; break;
    };
}

void print_pretty_str(const char *in_str, int style){
    int row = 0, col = 0, text_len = strlen(in_str);
    // print top part
    for (int ctr = 0; ctr < text_len; ++ctr){
        get_index(in_str[ctr], &row, &col);
        if (row == -1 || col == -1) { continue; };

        switch(style){
            case 2:  printf("%s ", g_style2[row][col]); break;
            case 3:  printf("%s ", g_style3[row][col]); break;
            case 4:  printf("%s ", g_style4[row][col]); break;
            case 1: default: printf("%s ", g_style1[row][col]); break;
        };
    };
    putchar('\n');
    // print bottom part
    for (int ctr = 0; ctr < text_len; ++ctr){
        get_index(in_str[ctr], &row, &col);
        if (row == -1 || col == -1) { continue; };

        switch(style){
            case 2:  printf("%s ", g_style2[row+1][col]); break;
            case 3:  printf("%s ", g_style3[row+1][col]); break;
            case 4:  printf("%s ", g_style4[row+1][col]); break;
            case 1: default: printf("%s ", g_style1[row+1][col]); break;
        };
    };
    putchar('\n');
}

void usage(){
    puts("gentext2x3 [2] \"string(s)\"");
    puts("Style 1 (3x2):");
    print_pretty_str("hello, world!", 0);
    puts("\nStyle 2 (3x2):");
    print_pretty_str("hello, world!", 2);
    puts("\nStyle 3 (3x2):");
    print_pretty_str("hello, world!", 3);
    puts("\nStyle 4 (2x2):");
    print_pretty_str("hello, world!", 4);
};

int main (int argc, const char** argv){
    //char test_text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890";
    //char test_text[] = ".,'\"!?()-_+=;:/\\[]<>";

    if (argc < 2) {
        usage();
        return 0;
    };

    int style = 0, str_ind = 1;
    if (argc > 2){
        // TODO convert with atoi() and check if not 0?
               if (strcmp(argv[str_ind], "1") == 0) { style = 1; ++str_ind;
        } else if (strcmp(argv[str_ind], "2") == 0) { style = 2; ++str_ind;
        } else if (strcmp(argv[str_ind], "3") == 0) { style = 3; ++str_ind;
        } else if (strcmp(argv[str_ind], "4") == 0) { style = 4; ++str_ind; };
    };

    for (; str_ind < argc; ++str_ind){
        print_pretty_str(argv[str_ind], style);
    };

    return 0;
}
