#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include "toml-c.h"
#include "windowlist.h"

struct configuration {
    char sort_by[20];
    int max_windows;

    char active_window_fg_color[20];
    char inactive_window_fg_color[20];
    char separator_fg_color[20];
    char overflow_fg_color[20];

    char separator_string[200];
    int spaces;

    char name[20];
    char name_case[20];
    int name_max_length;

    // TODO: window_nicknames
} config;

void copy_config_str(toml_table_t* tbl, char* option, char* config_field) {
    // Helper function to do a repetitive free operation
    char* toml_ptr = toml_table_string(tbl, option).u.s;
    strcpy(config_field, toml_ptr);
    free(toml_ptr);
}

void parse_config(char* filename, char* executable_path) {
    int max_path_length = 200;
    char config_path[max_path_length];

    // create path to config file relative to the executable
    char* dir = dirname(STRDUP(executable_path)); // without dup, argv[0] gets modified by dirname()
    snprintf(config_path, max_path_length, "%s/%s", dir, filename);

    char errbuf[200];

    FILE* fp = fopen(config_path, "r");
    toml_table_t* tbl = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    free(dir);

    copy_config_str(tbl, "sort_by", config.sort_by);
    config.max_windows = toml_table_int(tbl, "max_windows").u.i;

    copy_config_str(tbl, "active_window_fg_color", config.active_window_fg_color);
    copy_config_str(tbl, "inactive_window_fg_color", config.inactive_window_fg_color);
    copy_config_str(tbl, "separator_fg_color", config.separator_fg_color);
    copy_config_str(tbl, "overflow_fg_color", config.overflow_fg_color);

    copy_config_str(tbl, "separator_string", config.separator_string);
    config.spaces = toml_table_int(tbl, "spaces").u.i;

    copy_config_str(tbl, "name", config.name);
    copy_config_str(tbl, "name_case", config.name_case);
    config.name_max_length = toml_table_int(tbl, "name_max_length").u.i;

    toml_free(tbl);
}

void lowercase(char* str) {
    for(int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void uppercase(char* str) {
    for(int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

int compare_window_class(const void* v1, const void* v2) {
    const struct window_props* p1 = v1;
    const struct window_props* p2 = v2;
    lowercase(p1->class);
    lowercase(p2->class);
    return strcmp(p1->class, p2->class);
}

int compare_position(const void* v1, const void* v2) {
    // Sort wlist by horizontal position on screen
    // If tied, vertical position decides (higher first)
    const struct window_props* p1 = v1;
    const struct window_props* p2 = v2;
    if (p1->x < p2->x) return -1;
    if (p1->x > p2->x) return 1;
    if (p1->y < p2->y) return -1;
    if (p1->y > p2->y) return 1;
    return 0;
}

void print_spaces() {
    for (int i = 0; i < config.spaces; i++) {
        printf(" ");
    }
}

void output(struct window_props* wlist, int n, Window active_window, char* executable_path) {

    if (!strcmp(config.sort_by, "application")) {
        qsort(wlist, n, sizeof(struct window_props), compare_window_class);
    }
    if (!strcmp(config.sort_by, "position")) {
        qsort(wlist, n, sizeof(struct window_props), compare_position);
    }

    char* l_click = "A1";
    // char* m_click = "A2";
    char* r_click = "A3";
    // char* scroll_up = "A4";
    // char* scroll_down = "A5";

    int window_count = 0;

    for (int i = 0; i < n; i++) {
        if (window_count > config.max_windows) {
            window_count++;
            continue;
        }

        Window wid = wlist[i].id;

        if (window_count > 0) {
            printf("%%{F%s}%s%%{F-}", config.separator_fg_color, config.separator_string);
        }

        printf("%%{%s:%s %s 0x%lx:}", r_click, executable_path, "--close", wid);

        if (wid != active_window) {
            printf("%%{%s:%s %s 0x%lx:}", l_click, executable_path, "--raise", wid);
            printf("%%{F%s}", config.inactive_window_fg_color);
        } else {
            printf("%%{%s:%s %s 0x%lx:}", l_click, executable_path, "--minimize", wid);
            printf("%%{F%s}", config.active_window_fg_color);
        }

        char* window_name;
        if (!strcmp(config.name, "title")) {
            window_name = wlist[i].title;
        } else {
            window_name = wlist[i].class;
        }

        if (!strcmp(config.name_case, "lowercase")) {
            lowercase(window_name);
        }
        if (!strcmp(config.name_case, "uppercase")) {
            uppercase(window_name);
        }

        if (window_count != 0) {
            print_spaces();
        }

        printf("%.*s", config.name_max_length, window_name);

        if (strlen(window_name) > config.name_max_length) {
            // Name is truncated
            printf("‥");
        }

        print_spaces();

        printf("%%{F-}%%{A}%%{A}");

        window_count++;
        free(wlist[i].class);
        free(wlist[i].title);
    }

    if (window_count > config.max_windows) {
        printf("%%{F%s}", config.overflow_fg_color);
        printf("(+%d)", window_count - config.max_windows);
        printf("%%{F-}");
    }

    printf("\n");
}

void spy_root_window(Display* d, char* executable_path) {
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
            struct window_props* wlist = generate_window_list(d, current_desktop_id, &n);
            output(wlist, n, active_window, executable_path);
            free(wlist);
        }
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
    parse_config("config.toml", argv[0]);
    Display* d = XOpenDisplay(NULL);

    if (argc < 2) {
        // No arguments: listen to XEvents forever
        // and print the window list (output to stdout)
        spy_root_window(d, argv[0]);
    } else {
        // Arguments exist: handle on-click action
        char* action = argv[1];
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
