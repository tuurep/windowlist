#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "actions-common.h"

int main(int argc, char* argv[]) {
    Window wid = require_window_id(argc, argv);
    Display* d = XOpenDisplay(NULL);
    client_msg(d, wid, "_NET_CLOSE_WINDOW", 0);
    XCloseDisplay(d);
}
