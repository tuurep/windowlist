/*
This is a replacement for `xprop -root -spy`
to be able to listen to window geometry and position change events
so that the windowlist keeps being sorted by position without
having to change window focus in between.

This loop will be baked into the main file when I rewrite
the bash script in C
*/

#include <stdio.h>
#include <X11/Xlib.h>

int main() {
    XEvent event;
    Display *disp = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(disp);

    // Asks X server to send ConfigureNotify and PropertyNotify events
    // ConfigureNotify is sent when window's size or position changes
    // PropertyNotify for changes in client list and active window
    XSelectInput(disp, root, SubstructureNotifyMask | PropertyChangeMask);

    for (;;) {
        fflush(stdout);
        XNextEvent(disp, &event);

        if (event.type == ConfigureNotify || event.type == PropertyNotify) {
            printf("Redraw windowlist\n");
        }
    }
}
