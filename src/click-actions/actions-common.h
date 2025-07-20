#include <X11/Xlib.h>

int client_msg(Display* d, Window w, char* msg, long desktop);
Window str_to_wid(char* str);
