# windowlist

![screenshot](screenshot.png)

Began as a fork of my favorite Polybar script [polywins](https://github.com/uniquepointer/polywins)

Windowlist has been fully rewritten in C using the relevant parts of the source code from [wmctrl](https://github.com/Conservatory/wmctrl) and [xprop](https://gitlab.freedesktop.org/xorg/app/xprop).

## Improvements over polywins

* Fixed a bug where names would not be correct if WM_CLASS contains spaces or dots
* Option to sort the window list:
    * By horizontal position on the screen
    * By the application name
* Ability to set nicknames for windows if a window has a bad default name
* More flexible styling
* Configurable click actions

## Installation

Project directory should be in `~/.config/polybar/scripts/`

In `~/.config/polybar/scripts/windowlist/` run `make`

Add module in `~/.config/polybar/config.ini`:

```ini
[module/windowlist]
type = custom/script
exec = ~/.config/polybar/scripts/windowlist/main 2> /dev/null
tail = true
```

Add module `windowlist` in any of `modules-left`, `modules-center` or `modules-right`

## Configuration

Windowlist can be configured in `config.toml` in the root of the project.

All options are detailed below:

<table>
    <tbody>
        <tr>
            <th>Option</th>
            <th>Description</th>
            <th>Possible values</th>
        </tr>
        <tr>
            <td><code>sort_by</code></td>
            <td>Criteria to sort the list of windows</td>
            <td>
                <ul>
                    <li><code>"none"</code>: unordered (WM client list order)</li>
                    <li><code>"position"</code>: sort based on horizontal position on the screen</li>
                    <li><code>"application"</code>: sort alphabetically based on the application class</li>
                <ul>
            </td>
        </tr>
        <tr>
            <td><code>max_windows</code></td>
            <td>How many windows can be visible on the list. Number of windows that did not fit will be shown e.g. <code>(+3)</code></td>
            <td align="center">number (int)</td>
        </tr>
        <tr>
            <td><code>name</code></td>
            <td>Which X window property is considered window name (label for a window)</td>
            <td>
                <ul>
                    <li><code>"class"</code>: WM_CLASS</li>
                    <li><code>"title"</code>: WM_NAME</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>name_case</code></td>
            <td>Text case for window names</td>
            <td>
                <ul>
                    <li><code>"none"</code>: don't change capitalization</li>
                    <li><code>"lowercase"</code>: all lowercase</li>
                    <li><code>"uppercase"</code>: all uppercase</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>name_max_length</code></td>
            <td>Maximum length for a window name before it's truncated with <code>‥</code></td>
            <td align="center">number (int)</td>
        </tr>
        <tr>
            <td><code>name_padding</code></td>
            <td>How many spaces to add before and after a window name</td>
            <td align="center">number (int)</td>
        </tr>
        <tr>
            <td><code>separator_string</code></td>
            <td>String displayed between window names</td>
            <td align="center">any string</td>
        </tr>
        <tr>
            <td><code>empty_desktop_string</code></td>
            <td>String to show when no windows are open</td>
            <td align="center">any string</td>
        </tr>
        <tr>
            <td>
                <code>active_window_left_click</code><br>
                <code>active_window_middle_click</code><br>
                <code>active_window_right_click</code><br>
                <code>active_window_scroll_up</code><br>
                <code>active_window_scroll_down</code><br>
                <code>inactive_window_left_click</code><br>
                <code>inactive_window_middle_click</code><br>
                <code>inactive_window_right_click</code><br>
                <code>inactive_window_scroll_up</code><br>
                <code>inactive_window_scroll_down</code><br>
            </td>
            <td>Click actions for window names can be set as <code>"raise"</code>, <code>"minimize"</code> or <code>"close"</code>, or a custom script/program in the <code>click-actions</code> directory. Window currently in focus (active) and unfocused windows (inactive) are configurable separately.</td>
            <td>
                <ul>
                    <li><code>"none"</code>: no action</li>
                    <li>name of script (string)</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td>
                <code>active_window_fg_color</code>
                <code>inactive_window_fg_color</code>
                <code>separator_fg_color</code>
                <code>empty_desktop_fg_color</code>
                <code>overflow_fg_color</code>
            </td>
            <td>
                Foreground colors for:
                <ul>
                    <li>Currently focused window</li>
                    <li>Windows not in focus</li>
                    <li>The <code>separator_string</code></li>
                    <li>The <code>empty_desktop_string</code></li>
                    <li>The string shown when <code>max_windows</code> exceeded</li>
                </ul>
            </td>
            <td>
                <ul>
                    <li><code>"none"</code>: default polybar fg</code></li>
                    <li>hex color (string)</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>*_bg_color</code></td>
            <td>All of the foreground colors have a background color counterpart, e.g. <code>active_window_bg_color</code></td>
            <td>
                <ul>
                    <li><code>"none"</code>: no background color</li>
                    <li>hex color (string)</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>*_ul_color</code></td>
            <td>All colors also have an underline color counterpart, e.g. <code>active_window_ul_color</code><br><br>
            Note that <code>line-size</code> must be set to 1 or higher in your polybar <code>config.ini</code>, otherwise underline isn't visible.</td>
            <td>
                <ul>
                    <li><code>"none"</code>: no underline</li>
                    <li>hex color (string)</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>ignored_classes</code></td>
            <td>Windows with a WM_CLASS in this array will not be shown on the bar. Strings are matched case insensitively.</td>
            <td align="center">array of strings</td>
        </tr>
        <tr>
            <td><code>window_nicknames</code></td>
            <td>A window name can be substituted with a custom name using key value pairs. The keys are matched case insensitively.</td>
            <td align="center">table of string key-value pairs</td>
        </tr>
    </tbody>
</table>

Note: polybar must be reset before changes take effect.

### Defaults

Check the `config.toml` in this repo, the options set there are the default values.

You can also remove any key and it will fall back to the default value.

### Scripting click actions

The most convenient way is to write a shell script in the `click-actions` directory. Any language could be used, though. There are three "default" actions as small C programs: `raise`, `minimize` and `close`.

You can write a new action as a script such as:

`click-actions/foo.sh`

```bash
#!/bin/sh

window_id="$1"

# Do something with the window id of the window that has been clicked/scrolled on
```

Set the script as executable: `chmod +x click-actions/foo.sh`

Then in `config.toml`:

```toml
active_window_middle_click = "foo.sh"
```

Window id is always given as arg `$1`. Tools I know that could be used to make something happen with a window id:

* [wmctrl](https://github.com/Conservatory/wmctrl)
* [wmutils](https://github.com/wmutils/core)
* [xdo](https://github.com/baskerville/xdo)
* [xdotool](https://github.com/jordansissel/xdotool)

## Dependencies

Requires an [EWMH](https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html) compliant window manager

* Wikipedia list of EWMH compliant window managers: [link](https://en.wikipedia.org/wiki/Extended_Window_Manager_Hints#List_of_window_managers_that_support_Extended_Window_Manager_Hints)
