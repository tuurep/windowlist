/* 
    In this file is a modified version of wmctrl's source code

    The original can be found at:
        https://github.com/Conservatory/wmctrl

    And a particularly helpful fork with some fixes and additions:
        https://github.com/kfogel/wmctrl

    Licensed under GPLv2

    The idea here is to modify the list_windows function so that it
    works better and more seamlessly with a polybar script that
    lists the wm_class of each window in the current desktop.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <glib.h>

#define MAX_PROPERTY_VALUE_LEN 4096

/* declarations of static functions */
static Window *get_client_list (Display *disp, unsigned long *size);
static int list_windows (Display *disp);
static gchar *get_window_class (Display *disp, Window win);
static gchar *get_property (Display *disp, Window win, 
        Atom xa_prop_type, gchar *prop_name, unsigned long *size);
static void init_charset(void);
static unsigned long *current_desktop (Display *disp);

static struct {
    int force_utf8;
} options;

static gboolean envir_utf8;

int main (int argc, char **argv) {
    int ret = EXIT_SUCCESS;
    Display *disp;

    memset(&options, 0, sizeof(options)); /* just to be sure */

    /* necessary to make g_get_charset() and g_locale_*() work */
    setlocale(LC_ALL, "");

    init_charset();

    if (! (disp = XOpenDisplay(NULL))) {
        fputs("Cannot open display.\n", stderr);
        return EXIT_FAILURE;
    }

    ret = list_windows(disp);

    XCloseDisplay(disp);
    return ret;
}

static void init_charset (void) {
    const gchar *charset; /* unused */
    gchar *lang = getenv("LANG") ? g_ascii_strup(getenv("LANG"), -1) : NULL; 
    gchar *lc_ctype = getenv("LC_CTYPE") ? g_ascii_strup(getenv("LC_CTYPE"), -1) : NULL;

    /* this glib function doesn't work on my system ... */
    envir_utf8 = g_get_charset(&charset);

    /* ... therefore we will examine the environment variables */
    if (lc_ctype && (strstr(lc_ctype, "UTF8") || strstr(lc_ctype, "UTF-8"))) {
        envir_utf8 = TRUE;
    }
    else if (lang && (strstr(lang, "UTF8") || strstr(lang, "UTF-8"))) {
        envir_utf8 = TRUE;
    }

    g_free(lang);
    g_free(lc_ctype);

    if (options.force_utf8) {
        envir_utf8 = TRUE;
    }
}

static unsigned long *current_desktop (Display *disp) {
    unsigned long *cur_desktop = NULL;
    Window root = DefaultRootWindow(disp);
    if (! (cur_desktop = (unsigned long *)get_property(disp, root,
            XA_CARDINAL, "_NET_CURRENT_DESKTOP", NULL))) {
        if (! (cur_desktop = (unsigned long *)get_property(disp, root,
                XA_CARDINAL, "_WIN_WORKSPACE", NULL))) {
            fputs("Cannot get current desktop properties. "
                  "(_NET_CURRENT_DESKTOP or _WIN_WORKSPACE property)"
                  "\n", stderr);
            g_free(cur_desktop);
            return NULL;
        }
    }
    return cur_desktop;
}

static Window *get_client_list (Display *disp, unsigned long *size) {
    Window *client_list = NULL;
    char * msg = NULL;

    msg = "_NET_CLIENT_LIST or _WIN_CLIENT_LIST";
    client_list = (Window *)get_property(disp, DefaultRootWindow(disp), 
            XA_WINDOW, "_NET_CLIENT_LIST", size);
    if (!client_list)
        client_list = (Window *)get_property(disp, DefaultRootWindow(disp), 
                XA_CARDINAL, "_WIN_CLIENT_LIST", size);

    if (!client_list)
        fprintf(stderr, "Cannot get client list properties.\n(%s)\n", msg);

    return client_list;
}

static void calculate_window_middle_x_y(Display *disp, Window win, int *x, int *y) {
    Window junkroot;
    int junkx, junky;
    unsigned int w, h, border_width, depth;

    XGetGeometry(disp, win, &junkroot, &junkx, &junky, &w, &h, &border_width, &depth);
    XTranslateCoordinates(disp, win, junkroot, junkx, junky, x, y, &junkroot);

    *x = *x + w/2;
    *y = *y + h/2;
}

static int list_windows (Display *disp) {
    Window *client_list;
    unsigned long client_list_size;
    int i;

    if ((client_list = get_client_list(disp, &client_list_size)) == NULL) {
        return EXIT_FAILURE; 
    }

    unsigned long *cur_desktop = current_desktop(disp);

    /* print the list */
    for (i = 0; i < client_list_size / sizeof(Window); i++) {
        Window win = client_list[i];
        unsigned long *desktop;

        /* desktop ID */
        if ((desktop = (unsigned long *)get_property(disp, win,
                        XA_CARDINAL, "_NET_WM_DESKTOP", NULL)) == NULL) {
            desktop = (unsigned long *)get_property(disp, win,
                    XA_CARDINAL, "_WIN_WORKSPACE", NULL);
        }

        /* Only list windows from the current desktop
           This also excludes windows in the 'omnipresent' desktop ID -1
           like Polybar and Rofi for example. */
        if (*desktop != *cur_desktop) {
            continue;
        }

        /* window ID */
        printf("0x%.8lx ", win);

        int x, y;
        calculate_window_middle_x_y(disp, win, &x, &y);

        /* Coordinates at the middle point of window */
        /* Used for sorting the window list by window position */
        printf("%d %d ", x, y);

        gchar *class = get_window_class(disp, win); /* UTF8 */

        printf("%-20s", class ? class : "N/A");
        printf("\n");

        g_free(desktop);
        g_free(class);
    }
    g_free(client_list);
    g_free(cur_desktop);

    return EXIT_SUCCESS;
}

static gchar *get_window_class (Display *disp, Window win) {
    gchar *wm_class;
    gchar *class_utf8;
    unsigned long size;

    wm_class = get_property(disp, win, XA_STRING, "WM_CLASS", &size);
    if (wm_class) {
        /* 
           WM_CLASS contains two consecutive null-terminated strings:
           <Instance>\0<Class>\0
           We want the second one, so point after the first null-terminator.

           More explanation on this pretty unintuitive window property:
           https://unix.stackexchange.com/questions/494169/wm-class-vs-wm-instance
        */
        gchar *class = strchr(wm_class, '\0') + 1;
        class_utf8 = g_locale_to_utf8(class, -1, NULL, NULL, NULL);
    }
    else {
        class_utf8 = NULL;
    }

    g_free(wm_class);

    return class_utf8;
}

static gchar *get_property (Display *disp, Window win,
        Atom xa_prop_type, gchar *prop_name, unsigned long *size) {
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    gchar *ret;

    xa_prop_name = XInternAtom(disp, prop_name, False);

    /* MAX_PROPERTY_VALUE_LEN / 4 explanation (XGetWindowProperty manpage):
     *
     * long_length = Specifies the length in 32-bit multiples of the
     *               data to be retrieved.
     *
     * NOTE:  see 
     * http://mail.gnome.org/archives/wm-spec-list/2003-March/msg00067.html
     * In particular:
     *
     * 	When the X window system was ported to 64-bit architectures, a
     *  rather peculiar design decision was made. 32-bit quantities such
     *  as Window IDs, atoms, etc, were kept as longs in the client side
     *  APIs, even when long was changed to 64 bits.
     *
     */
    if (XGetWindowProperty(disp, win, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
                xa_prop_type, &xa_ret_type, &ret_format,     
                &ret_nitems, &ret_bytes_after, &ret_prop) != Success) {
        return NULL;
    }

    if (xa_ret_type != xa_prop_type) {
        XFree(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */

    tmp_size = (ret_format / 8) * ret_nitems;
    /* Correct 64 Architecture implementation of 32 bit data */
    if(ret_format==32) tmp_size *= sizeof(long)/4;
    ret = g_malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

