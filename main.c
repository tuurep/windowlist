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

    char name[20];
    char name_case[20];
    int name_max_length;
    int name_padding;

    char empty_desktop_string[200];
    char separator_string[200];

    char active_window_fg_color[20];
    char inactive_window_fg_color[20];
    char empty_desktop_fg_color[20];
    char separator_fg_color[20];
    char overflow_fg_color[20];
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

    copy_config_str(tbl, "name", config.name);
    copy_config_str(tbl, "name_case", config.name_case);
    config.name_max_length = toml_table_int(tbl, "name_max_length").u.i;
    config.name_padding = toml_table_int(tbl, "name_padding").u.i;

    copy_config_str(tbl, "empty_desktop_string", config.empty_desktop_string);
    copy_config_str(tbl, "separator_string", config.separator_string);

    copy_config_str(tbl, "active_window_fg_color", config.active_window_fg_color);
    copy_config_str(tbl, "inactive_window_fg_color", config.inactive_window_fg_color);
    copy_config_str(tbl, "empty_desktop_fg_color", config.empty_desktop_fg_color);
    copy_config_str(tbl, "separator_fg_color", config.separator_fg_color);
    copy_config_str(tbl, "overflow_fg_color", config.overflow_fg_color);

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

void pad_spaces(char* window_name) {
    int n = config.name_padding;
    window_name = realloc(window_name, (strlen(window_name) + n * 2) * sizeof(char));
    size_t original_length = strlen(window_name);
    memmove(window_name + n, window_name, original_length + 1);
    memset(window_name, ' ', n);
    memset(window_name + n + original_length, ' ', n);
}

int unused(char* option) {
    if (option[0] == '\0' || !strcmp(option, "none")) {
        return 1;
    }
    return 0;
}

void print_polybar_str(char* label, char* fg_color, /* char* bg_color, */ /* char* ul_color, */
                       char* l_click, /* char* m_click, */ char* r_click /* char* scroll_up, */ /* char* scroll_down, */) {

    int actions_count = 0;

    if (!unused(l_click)) {
        printf("%%{A1:%s:}", l_click);
        actions_count++;
    }

    if (!unused(r_click)) {
        printf("%%{A3:%s:}", r_click);
        actions_count++;
    }

    printf("%%{F%s}", fg_color);
    printf(label);
    printf("%%{F-}");

    for (int i = 0; i < actions_count; i++) {
        printf("%%{A}");
    }
}

void output(struct window_props* wlist, int n, Window active_window, char* executable_path) {

    if (!strcmp(config.sort_by, "application")) {
        qsort(wlist, n, sizeof(struct window_props), compare_window_class);
    }
    if (!strcmp(config.sort_by, "position")) {
        qsort(wlist, n, sizeof(struct window_props), compare_position);
    }

    int window_count = 0;

    for (int i = 0; i < n; i++) {
        if (window_count > config.max_windows) {
            window_count++;
            continue;
        }

        if (window_count > 0) {
            print_polybar_str(config.separator_string, config.separator_fg_color, "", "");
        }

        Window wid = wlist[i].id;

        char l_click[200];
        char r_click[200];

        snprintf(r_click, 200, "%s %s 0x%lx", executable_path, "--close", wid);

        char* window_fg_color;

        if (wid != active_window) {
            snprintf(l_click, 200, "%s %s 0x%lx", executable_path, "--raise", wid);
            window_fg_color = config.inactive_window_fg_color;
        } else {
            snprintf(l_click, 200, "%s %s 0x%lx", executable_path, "--minimize", wid);
            window_fg_color = config.active_window_fg_color;
        }

        char* window_name;

        if (!strcmp(config.name, "title")) {
            window_name = wlist[i].title;
        } else {
            window_name = wlist[i].class;
        }

        if (strlen(window_name) > config.name_max_length) {
            // Name is truncated
            strcpy(window_name + config.name_max_length, "‥");
        }

        if (!strcmp(config.name_case, "lowercase")) {
            lowercase(window_name);
        }
        if (!strcmp(config.name_case, "uppercase")) {
            uppercase(window_name);
        }

        pad_spaces(window_name);

        print_polybar_str(window_name, window_fg_color, l_click, r_click);

        window_count++;
        free(wlist[i].class);
        free(wlist[i].title);
    }

    if (window_count == 0) {
        print_polybar_str(config.empty_desktop_string, config.empty_desktop_fg_color, "", "");
    }

    if (window_count > config.max_windows) {
        char label[20];
        snprintf(label, 20, "(+%d)", window_count - config.max_windows);
        print_polybar_str(label, config.overflow_fg_color, "", "");
    }

    printf("\n");
}

void configure_windows_notify(Display* d, struct window_props* prev_wlist, int prev_wlist_len, struct window_props* wlist, int n) {
    for (int i = 0; i < n; i++) {
        bool found = false;
        for (int j = 0; j < prev_wlist_len; j++) {
            if (wlist[i].id == prev_wlist[j].id) {
                found = true;
                break;
            }
        }
        if (!found) {
            XSelectInput(d, wlist[i].id, PropertyChangeMask);
        }
    }
}

void spy_root_window(Display* d, char* executable_path) {
    XEvent e;
    Window root = DefaultRootWindow(d);

    // Asks X server to send ConfigureNotify and PropertyNotify events
    // ConfigureNotify is sent when a window's size or position changes
    // PropertyNotify for changes in client list and active window
    XSelectInput(d, root, SubstructureNotifyMask | PropertyChangeMask);

    struct window_props* prev_wlist = NULL;
    int prev_wlist_len = 0;

    for (;;) {
        fflush(stdout);
        XNextEvent(d, &e);

        long current_desktop_id = get_desktop_id(d, root, "_NET_CURRENT_DESKTOP");
        Window active_window = get_active_window(d);

        if (e.type == ConfigureNotify || e.type == PropertyNotify) {
            int n;
            struct window_props* wlist = generate_window_list(d, current_desktop_id, &n);
            configure_windows_notify(d, prev_wlist, prev_wlist_len, wlist, n);
            output(wlist, n, active_window, executable_path);

            free(prev_wlist);
            prev_wlist = wlist;
            prev_wlist_len = n;
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
