#include <X11/Xlib.h>

struct wprops {
    Window id;
    char* class;
    char* title;
    long desktop_id;
    int x, y;
};

struct wprops* generate_window_list(Display* d, int* window_list_size);
Window get_active_window(Display* d);
long get_desktop_id(Display* d, Window w, char* prop_name);
