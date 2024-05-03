/*
    In this file is a stripped down version of wmctrl's source code

    The original can be found at:
        https://github.com/Conservatory/wmctrl

    And a particularly helpful fork with some fixes and additions:
        https://github.com/kfogel/wmctrl

    Licensed under GPLv2

    The function `list_windows` has been rewritten as `generate_window_list`
    to get the properties in such a way that the list can be sorted based
    on any criteria, and to be easy to format for the Polybar module.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include "windowlist.h"

#define MAX_PROPERTY_VALUE_LEN 4096

char* get_property(Display* d, Window w, Atom xa_prop_type, char* prop_name,
                   unsigned long* size) {
    unsigned long ret_nitems, ret_bytes_after, tmp_size;
    unsigned char* ret_prop;
    int ret_format;
    char* ret;

    Atom xa_prop_name = XInternAtom(d, prop_name, False);
    Atom xa_ret_type;

    /*
        MAX_PROPERTY_VALUE_LEN / 4 explanation (XGetWindowProperty manpage):

        long_length = Specifies the length in 32-bit multiples of the
                      data to be retrieved.

        NOTE:  see
        http://mail.gnome.org/archives/wm-spec-list/2003-March/msg00067.html
        In particular:

        When the X window system was ported to 64-bit architectures, a
        rather peculiar design decision was made. 32-bit quantities such
        as Window IDs, atoms, etc, were kept as longs in the client side
        APIs, even when long was changed to 64 bits.
    */
    if (XGetWindowProperty(d, w, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
                           xa_prop_type, &xa_ret_type, &ret_format, &ret_nitems,
                           &ret_bytes_after, &ret_prop) != Success) {
        return NULL;
    }

    if (xa_ret_type != xa_prop_type) {
        XFree(ret_prop);
        return NULL;
    }

    // null terminate the result to make string handling easier

    tmp_size = (ret_format / 8) * ret_nitems;
    // Correct 64 Architecture implementation of 32 bit data
    if(ret_format==32) tmp_size *= sizeof(long)/4;
    ret = malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

Window* get_client_list(Display* d, unsigned long* size) {
    Window* client_list = NULL;
    char* msg = NULL;

    msg = "_NET_CLIENT_LIST or _WIN_CLIENT_LIST";
    client_list = (Window*) get_property(d, DefaultRootWindow(d), XA_WINDOW,
                                         "_NET_CLIENT_LIST", size);
    if (!client_list)
        client_list = (Window*) get_property(d, DefaultRootWindow(d), XA_CARDINAL,
                                             "_WIN_CLIENT_LIST", size);
    if (!client_list)
        fprintf(stderr, "Cannot get client list properties.\n(%s)\n", msg);

    return client_list;
}

Window get_active_window(Display* d) {
    char* prop;
    unsigned long size;
    Window ret = (Window) 0;

    prop = get_property(d, DefaultRootWindow(d), XA_WINDOW,
                        "_NET_ACTIVE_WINDOW", &size);
    if (prop) {
        ret = *((Window*) prop);
        free(prop);
    }

    return(ret);
}

long get_desktop_id(Display* d, Window w, char* prop_name) {
    /*
        Get current desktop ID:
            w: DefaultRootWindow(d)
            prop_name: _NET_CURRENT_DESKTOP

        Get desktop ID for given window:
            w: WID
            prop_name: _NET_WM_DESKTOP
    */
    unsigned long* desktop = NULL;
    long desktop_id;
    if (! (desktop = (unsigned long*) get_property(d, w, XA_CARDINAL,
                                                     prop_name, NULL))) {
        if (! (desktop = (unsigned long*) get_property(d, w, XA_CARDINAL,
                                                         "_WIN_WORKSPACE", NULL))) {
            fprintf(stderr, "Getting desktop property failed (%s or _WIN_WORKSPACE)\n", prop_name);
            free(desktop);
            return -2; // Desktop -2 doesn't exist
        }
    }
    desktop_id = (long) *desktop;
    free(desktop);
    return desktop_id;
}

void calculate_window_middle_x_y(Display* d, Window w, int* x, int* y) {
    Window junkroot;
    int junkx, junky;
    unsigned int width, height, border_width, depth;

    XGetGeometry(d, w, &junkroot, &junkx, &junky, &width, &height, &border_width, &depth);
    XTranslateCoordinates(d, w, junkroot, junkx, junky, x, y, &junkroot);

    *x = *x + width/2;
    *y = *y + height/2;
}

char* get_window_class(Display* d, Window w) {
    char* empty_wname = "N/A";
    char* empty = malloc(strlen(empty_wname) + 1);
    strcpy(empty, empty_wname);

    unsigned long size;
    char* wm_class = get_property(d, w, XA_STRING, "WM_CLASS", &size);

    if (!wm_class) {
        return empty;
    }

    char* class = calloc(size, sizeof(char));

    /*
       WM_CLASS contains two consecutive null-terminated strings:
       <Instance>\0<Class>\0
       We want the second one, so point after the first null-terminator.

       More explanation on this pretty unintuitive window property:
       https://unix.stackexchange.com/questions/494169/wm-class-vs-wm-instance
    */
    char* pointer_to_class = strchr(wm_class, '\0') + 1;
    strcpy(class, pointer_to_class);

    if (strlen(class) == 0) {
        free(wm_class);
        free(class);
        return empty;
    }

    free(empty);
    free(wm_class);
    return class;
}

char* get_window_title(Display *d, Window w) {
    char* empty_wname = "N/A";
    char* empty = malloc(strlen(empty_wname) + 1);
    strcpy(empty, empty_wname);

    char* title_utf8;

    char* wm_name = get_property(d, w, XA_STRING, "WM_NAME", NULL);
    char* net_wm_name = get_property(d, w,
            XInternAtom(d, "UTF8_STRING", False), "_NET_WM_NAME", NULL);

    if (net_wm_name) {
        title_utf8 = calloc(strlen(net_wm_name) + 1, sizeof(char));
        strcpy(title_utf8, net_wm_name);
    }
    else {
        if (wm_name) {
            title_utf8 = calloc(strlen(wm_name) + 1, sizeof(char));
            strcpy(title_utf8, wm_name);
        }
        else {
            free(wm_name);
            free(net_wm_name);
            return empty;
        }
    }

    free(empty);
    free(wm_name);
    free(net_wm_name);

    return title_utf8;
}

int error_catcher(Display* d, XErrorEvent* e) {
    /*
       Ignore BadWindow error instead of halting program

       Because XLib is async, a window may already be destroyed when I try to do
       something with its ID.

       See these:

       https://stackoverflow.com/questions/44025639/how-can-i-check-in-xlib-if-window-exists
       https://stackoverflow.com/questions/51908828/xlib-and-badwindow
    */

    if (e->error_code == BadWindow || e->error_code == BadDrawable) {
        // get_desktop_id() causes BadDrawable by extension of ignoring BadWindow:
        // gets an undefined window as argument
        fprintf(stderr, "Expected XError type %d\n", e->error_code);
        return EXIT_SUCCESS;
    }
    fprintf(stderr, "Unexpected XError type: %d\n", e->error_code);
    return EXIT_FAILURE;
}

struct window_props* generate_window_list(Display* d, long current_desktop_id, int* window_list_size) {
    Window* client_list;
    unsigned long client_list_size;

    if (! (client_list = get_client_list(d, &client_list_size))) {
        return NULL;
    }

    int n_clients = client_list_size / sizeof(Window);
    struct window_props* window_list = malloc(n_clients * sizeof(struct window_props));

    // Number of windows on current desktop
    int w_count = 0;

    XSetErrorHandler(error_catcher);

    // Populate the list
    for (int i = 0; i < n_clients; i++) {
        Window w = client_list[i];

        long desktop_id = get_desktop_id(d, w, "_NET_WM_DESKTOP");
        if (desktop_id != current_desktop_id) {
            continue;
        }

        struct window_props wp;
        wp.id = w;
        wp.class = get_window_class(d, w);
        wp.title = get_window_title(d, w);
        calculate_window_middle_x_y(d, w, &wp.x, &wp.y);

        window_list[w_count] = wp;
        w_count++;
    }

    XSetErrorHandler(NULL);
    free(client_list);

    *window_list_size = w_count;

    // Remove uninitialized part from array end
    window_list = realloc(window_list, *window_list_size * sizeof(struct window_props));

    return window_list;
}
