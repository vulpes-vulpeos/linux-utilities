#include <string.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

// gcc xkb_numlock_toggle.c -o xkb_numlock_toggle -lX11
int main(int argc, char* argv[]) {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) { return 1; };
    // Compatibility check
    int opcode, event, error, major = XkbMajorVersion, minor = XkbMinorVersion;
    if (!XkbQueryExtension(dpy, &opcode, &event, &error, &major, &minor)) { goto cleanup; };
    // Get keyboard info
    XkbDescPtr desc = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
    if (!desc || !desc->names) { goto cleanup; };
    // Find NumLock in virtual modifiers + convertion to real modifier mask
    unsigned int mask = 0;
    for (int i = 0; i < XkbNumVirtualMods; ++i) {
        char* vmod = XGetAtomName(desc->dpy, desc->names->vmods[i]);
        if (vmod && strcmp("NumLock", vmod) == 0) { XkbVirtualModsToReal(desc, 1u << i, &mask); XFree(vmod); break; };
    };
    XkbFreeKeyboard(desc, 0, True);
    // Toggle NumLock. LED behavior depends on XKB mapping.
    XkbStateRec state;
    if (XkbGetState(dpy, XkbUseCoreKbd, &state) == 0) { XkbLockModifiers(dpy, XkbUseCoreKbd, mask, (state.locked_mods & mask) ? 0 : mask); };
cleanup:
    XCloseDisplay(dpy);
    return 0;
}
