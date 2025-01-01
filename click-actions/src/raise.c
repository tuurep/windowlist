#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "common.h"
#include "../../windowlist.h" // get_desktop_id()

int main(int argc, char* argv[]) {
    // Must take a window id as first argument
    Window wid = str_to_wid(argv[1]);

    Display* d = XOpenDisplay(NULL);
    long desktop = get_desktop_id(d, wid, "_NET_WM_DESKTOP");
    client_msg(d, DefaultRootWindow(d), "_NET_CURRENT_DESKTOP", desktop);
    client_msg(d, wid, "_NET_ACTIVE_WINDOW", 0);
    XMapRaised(d, wid);
    XCloseDisplay(d);
}
