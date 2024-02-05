#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <X11/XKBlib.h>

int main(int argc, char const *argv[]) {
    // Get display
    Display *display = XOpenDisplay(NULL);
    if (!display) {return 1;};

    XkbStateRec state;
    // Set value to next layout if no arg provided
    if (argc == 1){
        XkbGetState(display, XkbUseCoreKbd, &state);
        state.group+=1;
    } else {
        state.group = atoi(argv[1]);
    };

    // Switch to next layout
    XkbLockGroup(display, XkbUseCoreKbd, state.group);

    // Close the display
    XCloseDisplay(display);

    return 0;
}
