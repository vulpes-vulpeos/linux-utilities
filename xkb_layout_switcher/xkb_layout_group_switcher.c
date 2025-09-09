#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/XKB.h>
#include <X11/extensions/XKBstr.h>
#include <stdlib.h>
#include <string.h>

// gcc xkb_layout_group_switcher.c -o xkb_layout_group_switcher -lX11
int main() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) { return 1; };

    int opcode, event, error, major = XkbMajorVersion, minor = XkbMinorVersion, laygr_ind = 1;
    if (!XkbQueryExtension(dpy, &opcode, &event, &error, &major, &minor)) { XCloseDisplay(dpy); return 1; };
    // Get keyboard description
    XkbDescPtr desc = XkbGetKeyboard(dpy, XkbSymbolsNameMask, XkbUseCoreKbd);
    if (!desc || !desc->names) { XCloseDisplay(dpy); return 1; };
    // Get symbols. Ex: pc_us(mac)_ru(mac)_2_inet(evdev)_terminate(ctrl_alt_bksp)
    char *symbols = XGetAtomName(dpy, desc->names->symbols);
    if (!symbols) { XkbFreeKeyboard(desc, XkbSymbolsNameMask, True); XCloseDisplay(dpy); return 1; };
    // Look for target token Ex: ua(macOS)
    XkbStateRec state;
    char *token = strtok(symbols, "+:_");
    while (token) {
        if(strcmp(token, "ua(macOS)") == 0) { laygr_ind = 0; break; };
        token = strtok(NULL, "+:_");
    };
    // Change layouts group
    system(laygr_ind ? "setxkbmap -layout us,ua -variant mac,macOS" : "setxkbmap -layout us,ru -variant mac,mac");
    if (XkbGetState(dpy, XkbUseCoreKbd, &state) == 0) { state.group = laygr_ind; XkbLockGroup(dpy, XkbUseCoreKbd, state.group); };

    XFree(symbols);
    XkbFreeKeyboard(desc, XkbSymbolsNameMask, True);
    XCloseDisplay(dpy);
    return 0;
}

