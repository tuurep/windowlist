#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

Window str_to_wid(char* str) {
    unsigned long wid;
    if (sscanf(str, "0x%lx", &wid) != 1) {
        fputs("Cannot convert argument to number.\n", stderr);
        return EXIT_FAILURE;
    }
    return (Window) wid;
}

int main(int argc, char* argv[]) {
    // Must take a window id as first argument
    Window wid = str_to_wid(argv[1]);

    Display* d = XOpenDisplay(NULL);
    XIconifyWindow(d, wid, DefaultScreen(d));
    XCloseDisplay(d);
}
