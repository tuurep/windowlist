#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "actions-common.h"
#include "../xlib-utils.h" // get_desktop_id()

int main(int argc, char* argv[]) {
    Window wid = require_window_id(argc, argv);
    Display* d = XOpenDisplay(NULL);
    long desktop = get_desktop_id(d, wid, "_NET_WM_DESKTOP");
    client_msg(d, DefaultRootWindow(d), "_NET_CURRENT_DESKTOP", desktop);
    client_msg(d, wid, "_NET_ACTIVE_WINDOW", 0);
    XMapRaised(d, wid);
    XCloseDisplay(d);
}
