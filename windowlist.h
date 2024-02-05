#include <X11/Xlib.h>

struct window_props {
    Window id;
    char* class;
    char* title;
    int x, y;
};

struct window_props* generate_window_list(Display* d, long current_desktop_id, int* n_wprops);
Window get_active_window(Display* d);
long get_desktop_id(Display* d, Window w, char* prop_name);
int close_window(Display* d, Window w);
int raise_window(Display* d, Window w);
int minimize_window(Display* d, Window w);
