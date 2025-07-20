#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include "toml-c.h"
#include "windowlist.h"

#define MAX_STR_LEN 256

struct configuration {
    char* sort_by;
    int max_windows;
    bool all_desktops;

    char* name;
    char* name_case;
    int name_max_length;
    int name_padding;

    char* empty_desktop_string;
    char* separator_string;

    char* active_window_left_click;
    char* active_window_right_click;
    char* active_window_middle_click;
    char* active_window_left_double_click;
    char* active_window_right_double_click;
    char* active_window_middle_double_click;
    char* active_window_scroll_up;
    char* active_window_scroll_down;

    char* inactive_window_left_click;
    char* inactive_window_right_click;
    char* inactive_window_middle_click;
    char* inactive_window_left_double_click;
    char* inactive_window_right_double_click;
    char* inactive_window_middle_double_click;
    char* inactive_window_scroll_up;
    char* inactive_window_scroll_down;

    char* active_window_fg_color;
    char* active_window_bg_color;
    char* active_window_ul_color;

    char* inactive_window_fg_color;
    char* inactive_window_bg_color;
    char* inactive_window_ul_color;

    char* separator_fg_color;
    char* separator_bg_color;
    char* separator_ul_color;

    char* empty_desktop_fg_color;
    char* empty_desktop_bg_color;
    char* empty_desktop_ul_color;

    char* overflow_fg_color;
    char* overflow_bg_color;
    char* overflow_ul_color;

    int active_window_font;
    int inactive_window_font;
    int separator_font;
    int empty_desktop_font;
    int overflow_font;

    toml_array_t* ignored_classes;
    toml_table_t* window_nicknames;
} config;

char* toml_string_or_default(toml_table_t* tbl, char* key, char* fallback) {
    if (!tbl) return fallback;
    toml_value_t val = toml_table_string(tbl, key);
    return val.ok ? val.u.s : fallback;
}

int toml_int_or_default(toml_table_t* tbl, char* key, int fallback) {
    if (!tbl) return fallback;
    toml_value_t val = toml_table_int(tbl, key);
    return val.ok ? val.u.i : fallback;
}

bool toml_bool_or_default(toml_table_t* tbl, char* key, bool fallback) {
    if (!tbl) return fallback;
    toml_value_t val = toml_table_bool(tbl, key);
    return val.ok ? val.u.b : fallback;
}

char* expand_path(char* path) {
    // Expand for example `~` in path

    if (!path) {
        return NULL;
    }

    wordexp_t exp;
    if (wordexp(path, &exp, 0) != 0 || exp.we_wordc == 0) {
        // Expansion failed
        wordfree(&exp);
        return NULL;
    }

    char* expanded_path = STRDUP(exp.we_wordv[0]); // STRDUP is in `toml-c`

    wordfree(&exp);
    return expanded_path;
}

toml_table_t* parse_config() {
    // env-CONFIGPATH from  Polybar's config.ini
    char* env_configpath = getenv("CONFIGPATH");
    char default_configpath[MAX_STR_LEN];

    char* xdg = getenv("XDG_CONFIG_HOME");

    if (xdg && strlen(xdg) > 0) {
        char* suffix = "polybar/windowlist.toml";

        // Join correctly with trailing slash vs. no trailing slash
        if (xdg[strlen(xdg) - 1] == '/') {
            snprintf(default_configpath, MAX_STR_LEN, "%s%s", xdg, suffix);
        } else {
            snprintf(default_configpath, MAX_STR_LEN, "%s/%s", xdg, suffix);
        }
    } else {
        snprintf(default_configpath, MAX_STR_LEN, "%s", "~/.config/polybar/windowlist.toml");
    }

    char* tmp = env_configpath ? env_configpath : default_configpath;
    char* configpath = expand_path(tmp);

    // Show message on the bar if env-CONFIGPATH points to a file that doesn't exist
    if (env_configpath && access(configpath, F_OK) != 0) {
        printf("Not found: %s\n", configpath);
        fflush(stdout);
        free(configpath);
        exit(EXIT_FAILURE);
    }

    toml_table_t* tbl = NULL;

    if (configpath && access(configpath, F_OK) == 0) {
        char errbuf[MAX_STR_LEN];
        FILE* fp = fopen(configpath, "r");
        tbl = toml_parse_file(fp, errbuf, sizeof(errbuf));
        fclose(fp);
    }

    free(configpath);

    config.sort_by = toml_string_or_default(tbl, "sort_by", "none");
    config.max_windows = toml_int_or_default(tbl, "max_windows", 13);
    config.all_desktops = toml_bool_or_default(tbl, "all_desktops", false);

    config.name = toml_string_or_default(tbl, "name", "class");
    config.name_case = toml_string_or_default(tbl, "name_case", "lowercase");
    config.name_max_length = toml_int_or_default(tbl, "name_max_length", 30);
    config.name_padding = toml_int_or_default(tbl, "name_padding", 1);

    config.empty_desktop_string = toml_string_or_default(tbl, "empty_desktop_string", "");
    config.separator_string = toml_string_or_default(tbl, "separator_string", "·");

    config.active_window_left_click = toml_string_or_default(tbl, "active_window_left_click", "minimize");
    config.active_window_right_click = toml_string_or_default(tbl, "active_window_right_click", "close");
    config.active_window_middle_click = toml_string_or_default(tbl, "active_window_middle_click", "none");
    config.active_window_left_double_click = toml_string_or_default(tbl, "active_window_left_double_click", "none");
    config.active_window_right_double_click = toml_string_or_default(tbl, "active_window_right_double_click", "none");
    config.active_window_middle_double_click = toml_string_or_default(tbl, "active_window_middle_double_click", "none");
    config.active_window_scroll_up = toml_string_or_default(tbl, "active_window_scroll_up", "none");
    config.active_window_scroll_down = toml_string_or_default(tbl, "active_window_scroll_down", "none");

    config.inactive_window_left_click = toml_string_or_default(tbl, "inactive_window_left_click", "raise");
    config.inactive_window_right_click = toml_string_or_default(tbl, "inactive_window_right_click", "close");
    config.inactive_window_middle_click = toml_string_or_default(tbl, "inactive_window_middle_click", "none");
    config.inactive_window_left_double_click = toml_string_or_default(tbl, "inactive_window_left_double_click", "none");
    config.inactive_window_right_double_click = toml_string_or_default(tbl, "inactive_window_right_double_click", "none");
    config.inactive_window_middle_double_click = toml_string_or_default(tbl, "inactive_window_middle_double_click", "none");
    config.inactive_window_scroll_up = toml_string_or_default(tbl, "inactive_window_scroll_up", "none");
    config.inactive_window_scroll_down = toml_string_or_default(tbl, "inactive_window_scroll_down", "none");

    config.active_window_fg_color = toml_string_or_default(tbl, "active_window_fg_color", "none");
    config.active_window_bg_color = toml_string_or_default(tbl, "active_window_bg_color", "none");
    config.active_window_ul_color = toml_string_or_default(tbl, "active_window_ul_color", "none");

    config.inactive_window_fg_color = toml_string_or_default(tbl, "inactive_window_fg_color", "#808080");
    config.inactive_window_bg_color = toml_string_or_default(tbl, "inactive_window_bg_color", "none");
    config.inactive_window_ul_color = toml_string_or_default(tbl, "inactive_window_ul_color", "none");

    config.separator_fg_color = toml_string_or_default(tbl, "separator_fg_color", "#808080");
    config.separator_bg_color = toml_string_or_default(tbl, "separator_bg_color", "none");
    config.separator_ul_color = toml_string_or_default(tbl, "separator_ul_color", "none");

    config.empty_desktop_fg_color = toml_string_or_default(tbl, "empty_desktop_fg_color", "none");
    config.empty_desktop_bg_color = toml_string_or_default(tbl, "empty_desktop_bg_color", "none");
    config.empty_desktop_ul_color = toml_string_or_default(tbl, "empty_desktop_ul_color", "none");

    config.overflow_fg_color = toml_string_or_default(tbl, "overflow_fg_color", "none");
    config.overflow_bg_color = toml_string_or_default(tbl, "overflow_bg_color", "none");
    config.overflow_ul_color = toml_string_or_default(tbl, "overflow_ul_color", "none");

    config.active_window_font = toml_int_or_default(tbl, "active_window_font", 0);
    config.inactive_window_font = toml_int_or_default(tbl, "inactive_window_font", 0);
    config.separator_font = toml_int_or_default(tbl, "separator_font", 0);
    config.empty_desktop_font = toml_int_or_default(tbl, "empty_desktop_font", 0);
    config.overflow_font = toml_int_or_default(tbl, "overflow_font", 0);

    config.ignored_classes  = tbl ? toml_table_array(tbl, "ignored_classes") : NULL;
    config.window_nicknames = tbl ? toml_table_table(tbl, "window_nicknames") : NULL;

    return tbl;
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
    const struct wprops* p1 = v1;
    const struct wprops* p2 = v2;
    lowercase(p1->class);
    lowercase(p2->class);
    return strcmp(p1->class, p2->class);
}

int compare_position(const void* v1, const void* v2) {
    // Sort wlist by horizontal position on screen
    // If tied, vertical position decides (higher first)
    const struct wprops* p1 = v1;
    const struct wprops* p2 = v2;
    if (p1->x < p2->x) return -1;
    if (p1->x > p2->x) return 1;
    if (p1->y < p2->y) return -1;
    if (p1->y > p2->y) return 1;
    return 0;
}

char* pad_spaces(char* window_name) {
    int padding = config.name_padding;

    int name_length = strlen(window_name);
    int padded_length = name_length + (2 * padding);

    char* padded_name = malloc(padded_length + 1);

    memset(padded_name, ' ', padded_length);
    memcpy(padded_name + padding, window_name, name_length);
    padded_name[padded_length] = '\0';

    return padded_name;
}

bool is_unused(char* option) {
    if (option[0] == '\0' || !strcmp(option, "none")) {
        return true;
    }
    return false;
}

bool is_ignored(char* class) {
    if (!config.ignored_classes) {
        return false;
    }

    for (int i = 0; i < toml_array_len(config.ignored_classes); i++) {
        char* ignored_class = toml_array_string(config.ignored_classes, i).u.s;
        if (!strcasecmp(class, ignored_class)) {
            return true;
        }
    }
    return false;
}

char* get_window_nickname(char* class, char* title) {
    if (!config.window_nicknames) {
        return NULL;
    }

    for (int i = 0; i < toml_table_len(config.window_nicknames); i++) {
        int keylen;

        const char* key = toml_table_key(config.window_nicknames, i, &keylen);
        char* val;

        if (!strcmp(config.name, "title")) {
            if (!strcasecmp(key, title)) {
                val = toml_table_string(config.window_nicknames, key).u.s;
                return val;
            }
        } else {
            if (!strcasecmp(key, class)) {
                val = toml_table_string(config.window_nicknames, key).u.s;
                return val;
            }
        }
    }
    return NULL;
}

void print_polybar_str(char* label, char* fg_color, char* bg_color, char* ul_color, int font,
                       char* l_click, char* m_click, char* r_click, char* scroll_up, char* scroll_down,
                       char* l_d_click, char* m_d_click, char* r_d_click) {

    int actions_count = 0;

    if (!is_unused(l_click)) {
        printf("%%{A1:%s:}", l_click);
        actions_count++;
    }

    if (!is_unused(m_click)) {
        printf("%%{A2:%s:}", m_click);
        actions_count++;
    }

    if (!is_unused(r_click)) {
        printf("%%{A3:%s:}", r_click);
        actions_count++;
    }

    if (!is_unused(scroll_up)) {
        printf("%%{A4:%s:}", scroll_up);
        actions_count++;
    }

    if (!is_unused(scroll_down)) {
        printf("%%{A5:%s:}", scroll_down);
        actions_count++;
    }

    if (!is_unused(l_d_click)) {
        printf("%%{A6:%s:}", l_d_click);
        actions_count++;
    }

    if (!is_unused(m_d_click)) {
        printf("%%{A7:%s:}", m_d_click);
        actions_count++;
    }

    if (!is_unused(r_d_click)) {
        printf("%%{A8:%s:}", r_d_click);
        actions_count++;
    }

    if (!is_unused(bg_color)) {
        printf("%%{B%s}", bg_color);
    }

    if (!is_unused(ul_color)) {
        printf("%%{u%s}%%{+u}", ul_color);
    }

    if (!is_unused(fg_color)) {
        printf("%%{F%s}", fg_color);
    }

    // No need to print font tag if it would be %{T1} (`font-0`)
    if (font > 0) {
        // Convert to %{T#} tag's 1-based indexing
        printf("%%{T%d}", font + 1);
    }

    printf(label);

    if (font > 0) {
        printf("%%{T-}");
    }

    if (!is_unused(fg_color)) {
        printf("%%{F-}");
    }

    if (!is_unused(ul_color)) {
        printf("%%{-u}");
    }

    if (!is_unused(bg_color)) {
        printf("%%{B-}");
    }

    for (int i = 0; i < actions_count; i++) {
        printf("%%{A}");
    }
}

void set_action_str(char* str, char* script_option, Window wid) {
    if (is_unused(script_option)) {
        strcpy(str, "none");
        return;
    }
    snprintf(str, MAX_STR_LEN, "%s 0x%lx", script_option, wid);
}

void output(struct wprops* wlist, int wlist_len, Window active_window, long current_desktop_id) {

    if (!strcmp(config.sort_by, "application")) {
        qsort(wlist, wlist_len, sizeof(struct wprops), compare_window_class);
    }
    if (!strcmp(config.sort_by, "position")) {
        qsort(wlist, wlist_len, sizeof(struct wprops), compare_position);
    }

    int window_count = 0;

    for (int i = 0; i < wlist_len; i++) {

        // Don't show 'omnipresent' windows, which have desktop ID -1
        // otherwise e.g. 'polybar' shows in the list
        // (Also if a window's desktop ID couldn't be retrieved, get_desktop_id() returns -2)
        if (wlist[i].desktop_id < 0) {
            free(wlist[i].class);
            free(wlist[i].title);
            continue;
        }

        if (!config.all_desktops && wlist[i].desktop_id != current_desktop_id) {
            free(wlist[i].class);
            free(wlist[i].title);
            continue;
        }

        if (is_ignored(wlist[i].class)) {
            free(wlist[i].class);
            free(wlist[i].title);
            continue;
        }

        if (window_count > config.max_windows) {
            free(wlist[i].class);
            free(wlist[i].title);
            window_count++;
            continue;
        }

        if (window_count > 0) {
            print_polybar_str(config.separator_string, config.separator_fg_color, config.separator_bg_color, config.separator_ul_color,
                              config.separator_font, "none", "none", "none", "none", "none", "none", "none", "none");
        }

        char window_left_click          [MAX_STR_LEN];
        char window_right_click         [MAX_STR_LEN];
        char window_middle_click        [MAX_STR_LEN];
        char window_left_double_click   [MAX_STR_LEN];
        char window_right_double_click  [MAX_STR_LEN];
        char window_middle_double_click [MAX_STR_LEN];
        char window_scroll_up           [MAX_STR_LEN];
        char window_scroll_down         [MAX_STR_LEN];
        char* window_fg_color;
        char* window_bg_color;
        char* window_ul_color;
        int window_font;

        if (wlist[i].id != active_window) {
            set_action_str(window_left_click,          config.inactive_window_left_click,          wlist[i].id);
            set_action_str(window_right_click,         config.inactive_window_right_click,         wlist[i].id);
            set_action_str(window_middle_click,        config.inactive_window_middle_click,        wlist[i].id);
            set_action_str(window_left_double_click,   config.inactive_window_left_double_click,   wlist[i].id);
            set_action_str(window_right_double_click,  config.inactive_window_right_double_click,  wlist[i].id);
            set_action_str(window_middle_double_click, config.inactive_window_middle_double_click, wlist[i].id);
            set_action_str(window_scroll_up,           config.inactive_window_scroll_up,           wlist[i].id);
            set_action_str(window_scroll_down,         config.inactive_window_scroll_down,         wlist[i].id);
            window_fg_color = config.inactive_window_fg_color;
            window_bg_color = config.inactive_window_bg_color;
            window_ul_color = config.inactive_window_ul_color;
            window_font = config.inactive_window_font;
        } else {
            set_action_str(window_left_click,          config.active_window_left_click,          wlist[i].id);
            set_action_str(window_right_click,         config.active_window_right_click,         wlist[i].id);
            set_action_str(window_middle_click,        config.active_window_middle_click,        wlist[i].id);
            set_action_str(window_left_double_click,   config.active_window_left_double_click,   wlist[i].id);
            set_action_str(window_right_double_click,  config.active_window_right_double_click,  wlist[i].id);
            set_action_str(window_middle_double_click, config.active_window_middle_double_click, wlist[i].id);
            set_action_str(window_scroll_up,           config.active_window_scroll_up,           wlist[i].id);
            set_action_str(window_scroll_down,         config.active_window_scroll_down,         wlist[i].id);
            window_fg_color = config.active_window_fg_color;
            window_bg_color = config.active_window_bg_color;
            window_ul_color = config.active_window_ul_color;
            window_font = config.active_window_font;
        }

        char* window_nickname = get_window_nickname(wlist[i].class, wlist[i].title);
        char* window_name = window_nickname;

        if (!window_nickname) {
            if (!strcmp(config.name, "title")) {
                window_name = wlist[i].title;
            } else {
                window_name = wlist[i].class;
            }
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

        char* padded_name = pad_spaces(window_name);

        print_polybar_str(padded_name, window_fg_color, window_bg_color, window_ul_color, window_font,
                          window_left_click, window_middle_click, window_right_click, window_scroll_up, window_scroll_down,
                          window_left_double_click, window_middle_double_click, window_right_double_click);

        window_count++;
        free(window_nickname);
        free(padded_name);
        free(wlist[i].class);
        free(wlist[i].title);
    }

    if (window_count == 0) {
        print_polybar_str(config.empty_desktop_string, config.empty_desktop_fg_color, config.empty_desktop_bg_color, config.empty_desktop_ul_color,
                          config.empty_desktop_font, "none", "none", "none", "none", "none", "none", "none", "none");
    }

    if (window_count > config.max_windows) {
        char overflow_string[20];
        snprintf(overflow_string, 20, "(+%d)", window_count - config.max_windows);
        print_polybar_str(overflow_string, config.overflow_fg_color, config.overflow_bg_color, config.overflow_ul_color,
                          config.overflow_font, "none", "none", "none", "none", "none", "none", "none", "none");
    }

    printf("\n");
}

void configure_windows_notify(Display* d, struct wprops* prev_wlist, int prev_wlist_len, struct wprops* wlist, int wlist_len) {
    for (int i = 0; i < wlist_len; i++) {
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

int main(int argc, char* argv[]) {
    Display* d = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(d);

    toml_table_t* tbl = parse_config();

    int wlist_len;
    struct wprops* wlist = generate_window_list(d, &wlist_len);

    if (!strcmp(config.sort_by, "application")) {
        qsort(wlist, wlist_len, sizeof(struct wprops), compare_window_class);
    }
    if (!strcmp(config.sort_by, "position")) {
        qsort(wlist, wlist_len, sizeof(struct wprops), compare_position);
    }

    int prev_wlist_len = 0;
    struct wprops* prev_wlist = NULL;

    // Get events for individual windows' property changes,
    // to know when a window's title (WM_NAME) changes
    configure_windows_notify(d, prev_wlist, prev_wlist_len, wlist, wlist_len);

    Window active_window = get_active_window(d);
    long current_desktop_id = get_desktop_id(d, root, "_NET_CURRENT_DESKTOP");

    // Initialize module on polybar launch
    output(wlist, wlist_len, active_window, current_desktop_id);

    // Receive PropertyNotify for changes in client list and active window change
    long event_mask = PropertyChangeMask;

    if (!strcmp(config.sort_by, "position")) {
        // Receive ConfigureNotify for window geometry changes
        event_mask |= SubstructureNotifyMask;
    }

    // Ask X server to send the events
    XSelectInput(d, root, event_mask);
    XEvent e;

    // Listen to XEvents forever and update the window list (output to stdout)
    for (;;) {
        fflush(stdout);
        XNextEvent(d, &e); // Blocks until next event

        if (e.type == ConfigureNotify || e.type == PropertyNotify) {
            free(prev_wlist);
            prev_wlist = wlist;
            prev_wlist_len = wlist_len;

            wlist = generate_window_list(d, &wlist_len);

            if (!strcmp(config.sort_by, "application")) {
                qsort(wlist, wlist_len, sizeof(struct wprops), compare_window_class);
            }
            if (!strcmp(config.sort_by, "position")) {
                qsort(wlist, wlist_len, sizeof(struct wprops), compare_position);
            }

            configure_windows_notify(d, prev_wlist, prev_wlist_len, wlist, wlist_len);

            current_desktop_id = get_desktop_id(d, root, "_NET_CURRENT_DESKTOP");
            active_window = get_active_window(d);

            output(wlist, wlist_len, active_window, current_desktop_id);
        }
    }
    free(wlist);
    free(prev_wlist);

    toml_free(tbl);
    XCloseDisplay(d);
}
