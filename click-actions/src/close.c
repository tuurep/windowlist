#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "common.h"

int main(int argc, char* argv[]) {
    // Must take a window id as first argument
    Window wid = str_to_wid(argv[1]);

    Display* d = XOpenDisplay(NULL);
    client_msg(d, wid, "_NET_CLOSE_WINDOW", 0);
    XCloseDisplay(d);
}
