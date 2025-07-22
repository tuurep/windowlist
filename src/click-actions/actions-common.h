#include <X11/Xlib.h>

int client_msg(Display* d, Window w, char* msg, long desktop);
Window require_window_id(int argc, char* argv[]);
