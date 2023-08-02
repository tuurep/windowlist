#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include "windowlist.h"

void lowercase(char *str) {
    for(int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void uppercase(char *str) {
    for(int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

int compare_alphabetic(const void *v1, const void *v2) {
    const struct window_props *p1 = v1;
    const struct window_props *p2 = v2;
    lowercase(p1->wname);
    lowercase(p2->wname);
    return strcmp(p1->wname, p2->wname);
}

int compare_position(const void *v1, const void *v2) {
    // Sort wlist by horizontal position on screen
    // If tied, vertical position decides (higher first)
    const struct window_props *p1 = v1;
    const struct window_props *p2 = v2;
    if (p1->x < p2->x) return -1;
    if (p1->x > p2->x) return 1;
    if (p1->y < p2->y) return -1;
    if (p1->y > p2->y) return 1;
    return 0;
}

void output(struct window_props *wlist, int n, long current_desktop_id,
            Window active_window, char *progname) {
    qsort(wlist, n, sizeof(struct window_props), compare_position);

    char *separator = "·";
    char *inactive_fg = "#787878";
    char *active_fg = "#e0e0e0";

    char *l_click = "A1";
    // char *m_click = "A2";
    char *r_click = "A3";
    // char *scroll_up = "A4";
    // char *scroll_down = "A5";

    int window_count = 0;

    for (int i = 0; i < n; i++) {
        if (wlist[i].desktop_id == current_desktop_id) {
            Window wid = wlist[i].window;

            if (window_count > 0)
                printf("%%{F%s}%s%%{F-}", inactive_fg, separator);

            printf("%%{%s:%s %s 0x%lx:}", r_click, progname, "--close", wid);

            if (wid != active_window) {
                printf("%%{%s:%s %s 0x%lx:}", l_click, progname, "--raise", wid);
                printf("%%{F%s}", inactive_fg);
            } else {
                printf("%%{%s:%s %s 0x%lx:}", l_click, progname, "--minimize", wid);
                printf("%%{F%s}", active_fg);
            }

            lowercase(wlist[i].wname);
            printf(" %s ", wlist[i].wname);

            printf("%%{F-}%%{A}%%{A}");

            window_count++;
        }
        free(wlist[i].wname);
    }
    printf("\n");
}

void spy_root_window(Display *d, char *progname) {
    XEvent e;
    Window root = DefaultRootWindow(d);

    // Asks X server to send ConfigureNotify and PropertyNotify events
    // ConfigureNotify is sent when a window's size or position changes
    // PropertyNotify for changes in client list and active window
    XSelectInput(d, root, SubstructureNotifyMask | PropertyChangeMask);

    for (;;) {
        fflush(stdout);
        XNextEvent(d, &e);

        long current_desktop_id = get_desktop_id(d, root, "_NET_CURRENT_DESKTOP");
        Window active_window = get_active_window(d);

        if (e.type == ConfigureNotify || e.type == PropertyNotify) {
            int n;
            struct window_props *wlist = generate_window_list(d, &n);
            output(wlist, n, current_desktop_id, active_window, progname);
            free(wlist);
        }
    }
}

Window str_to_wid(char *str) {
    unsigned long wid;
    if (sscanf(str, "0x%lx", &wid) != 1) {
            fputs("Cannot convert argument to number.\n", stderr);
            return EXIT_FAILURE;
    }
    return (Window) wid;
}

int main(int argc, char *argv[]) {
    Display *d = XOpenDisplay(NULL);

    if (argc < 2) {
        // No arguments: listen to XEvents forever
        // and print the window list (output to stdout)
        spy_root_window(d, argv[0]);
    } else {
        // Arguments exist: handle on-click action
        char *action = argv[1];
        Window wid = str_to_wid(argv[2]);

        if (!strcmp(action, "--close")) {
            close_window(d, wid);
        }
        if (!strcmp(action, "--raise")) {
            raise_window(d, wid);
        }
        if (!strcmp(action, "--minimize")) {
            minimize_window(d, wid);
        }
    }
    XCloseDisplay(d);
}
