#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "common.h"

int main(int argc, char* argv[]) {
    // Must take a window id as first argument
    Window wid = str_to_wid(argv[1]);

    Display* d = XOpenDisplay(NULL);
    XIconifyWindow(d, wid, DefaultScreen(d));
    XCloseDisplay(d);
}
