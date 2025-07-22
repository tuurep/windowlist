#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "actions-common.h"

int main(int argc, char* argv[]) {
    Window wid = require_window_id(argc, argv);
    Display* d = XOpenDisplay(NULL);
    XIconifyWindow(d, wid, DefaultScreen(d));
    XCloseDisplay(d);
}
