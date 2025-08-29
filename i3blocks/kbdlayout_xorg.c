#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/XKB.h>
#include <X11/extensions/XKBstr.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum mouse_buttons { DUMMY, BTNL, BTNM, BTNR, SCWLU, SCWLD, SCWLL, SCWLR, BTNF, BTNB };

int valid_token(char *token){
    // check if token is 1 char long and is a number
    if (strlen(token) == 1 && isdigit(token[0])) { return 0; };
    // check if token has forbidden words
    static const char *invalid[] = { "evdev", "inet", "pc", "base", "ctrl", "alt", "shift", "win", "bksp" };
    for (int i = 0; i < 9; i++) {
        if (strstr(token, invalid[i])) { return 0; };
    };

    return 1;
};
// gcc kbdlayout_xorg.c -o kbdlayout_xorg2 -lX11
int main() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) { return 1; };

    int opcode, event, error, major = XkbMajorVersion, minor = XkbMinorVersion;
    if (!XkbQueryExtension(dpy, &opcode, &event, &error, &major, &minor)) { XCloseDisplay(dpy); return 1; };
    // Get current layout index
    XkbStateRec state;
    if (XkbGetState(dpy, XkbUseCoreKbd, &state) != 0) { XCloseDisplay(dpy); return 1;};
    // Do something if clicked
    char* button = getenv("BLOCK_BUTTON");
    if (button != NULL) {
        switch (atoi(button)) {
            case BTNL: // Switch to next layout
                XkbLockGroup(dpy, XkbUseCoreKbd, ++state.group);
                XkbGetState(dpy, XkbUseCoreKbd, &state);
                break;
            default: break;
        };
    };
    // Get keyboard description
    XkbDescPtr desc = XkbGetKeyboard(dpy, XkbSymbolsNameMask, XkbUseCoreKbd);
    if (!desc || !desc->names) { XCloseDisplay(dpy); return 1; };
    // Get symbols. Ex: pc_us(mac)_ru(mac)_2_inet(evdev)_terminate(ctrl_alt_bksp)
    char *symbols = XGetAtomName(dpy, desc->names->symbols);
    if (!symbols) { XkbFreeKeyboard(desc, XkbSymbolsNameMask, True); XCloseDisplay(dpy); return 1; };
    // Get full name. Ex.: English (Macintosh)
    //char *group = XGetAtomName(dpy, desc->names->groups[state.group]);
    //if (!group) { XkbFreeKeyboard(desc, XkbSymbolsNameMask, True); XCloseDisplay(dpy); return 1; };
    // Get current layout name. Valid token index == state.group. Ex: us(mac)
    char *token = strtok(symbols, "+:_"), *layout = NULL;
    int vt_ind = 0;
    while (token) {
        if (valid_token(token)){
            if(vt_ind == state.group) { layout = token; break; };
            ++vt_ind;
        };
        token = strtok(NULL, "+:_");
    };

    if (layout) { printf("󰌌  %c%c\n", toupper(layout[0]), toupper(layout[1]));
    } else { puts("󰌌   N/A"); };

    XFree(symbols);
    XkbFreeKeyboard(desc, XkbSymbolsNameMask, True);
    XCloseDisplay(dpy);
    return 0;
}
