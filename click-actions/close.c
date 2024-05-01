#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

int client_msg(Display* d, Window w, char* msg) {
    XEvent e;
    long mask = SubstructureRedirectMask | SubstructureNotifyMask;

    e.xclient.type = ClientMessage;
    e.xclient.serial = 0;
    e.xclient.send_event = True;
    e.xclient.message_type = XInternAtom(d, msg, False);
    e.xclient.window = w;
    e.xclient.format = 32;
    e.xclient.data.l[0] = 0;
    e.xclient.data.l[1] = 0;
    e.xclient.data.l[2] = 0;
    e.xclient.data.l[3] = 0;
    e.xclient.data.l[4] = 0;

    if (XSendEvent(d, DefaultRootWindow(d), False, mask, &e)) {
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Cannot send %s event.\n", msg);
        return EXIT_FAILURE;
    }
}

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
    client_msg(d, wid, "_NET_CLOSE_WINDOW");
    XCloseDisplay(d);
}
