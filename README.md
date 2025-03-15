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

You can either clone the whole repo to `~/.config/polybar/scripts/` and run `make`, or if you want more control, use `make install DEST=~/installation/path/executable-name` to put the executable in a path of your choosing.

More detail on both methods:

### Using `make`

1. Put the repo in `~/.config/polybar/scripts/`
2. Go to `~/.config/polybar/scripts/windowlist/`
3. Run `make`

Add module in `~/.config/polybar/config.ini`:

```ini
[module/windowlist]
type = custom/script
exec = ~/.config/polybar/scripts/windowlist/main 2> /dev/null
tail = true
```

Add module `windowlist` in any of `modules-left`, `modules-center` or `modules-right`

### Using `make install` with custom executable path

Let's say you've cloned this repo in `~/repos/windowlist`, example usage:

1. Go to `~/repos/windowlist/`
2. Run `make install DEST=~/.config/polybar/scripts/windowlist`
    - (Note: here `windowlist` is the name of the executable, not the directory)

Now the module would look like this in Polybar's `config.ini`:

```ini
[module/windowlist]
type = custom/script
exec = ~/.config/polybar/scripts/windowlist 2> /dev/null
tail = true
```

Alternatively, install to `~/.local/bin`:

- `make install DEST=~/.local/bin/windowlist`
- Now the `exec` would look like: `exec = windowlist 2> /dev/null`

## Configuration

Windowlist can be configured in `config.toml` in the root of the project.

If `config.toml` doesn't already exist, it will be created as a copy of `config-default.toml` when running `make`. It will not be overwritten if it exists.

Keys can be removed or commented out from `config.toml`, they will fall back to the default values in that case.

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
            <td>Criteria to sort the list of windows.</td>
            <td>
                <ul>
                    <li><code>"position"</code>: sort based on horizontal position on the screen</li>
                    <li><code>"application"</code>: sort alphabetically based on the application class</li>
                    <li><code>"none"</code>: no sort, WM client list order (default)</li>
                <ul>
            </td>
        </tr>
        <tr>
            <td><code>max_windows</code></td>
            <td>How many windows can be visible on the list. Number of windows that did not fit will be shown e.g. <code>"(+3)"</code>.</td>
            <td align="center">number (default: 13)</td>
        </tr>
        <tr>
            <td><code>all_desktops</code></td>
            <td>List windows from all desktops, instead of the current desktop only.</td>
            <td align="center">boolean (default: false)</td>
        </tr>
        <tr>
            <td><code>name</code></td>
            <td>Which X window property is considered window name (label for a window).</td>
            <td>
                <ul>
                    <li><code>"class"</code>: WM_CLASS (default)</li>
                    <li><code>"title"</code>: WM_NAME</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>name_case</code></td>
            <td>Text case for window names.</td>
            <td>
                <ul>
                    <li><code>"lowercase"</code>: all lowercase (default)</li>
                    <li><code>"uppercase"</code>: all uppercase</li>
                    <li><code>"none"</code>: don't change capitalization</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>name_max_length</code></td>
            <td>Maximum length for a window name before it's truncated with <code>"‥"</code>.</td>
            <td align="center">number (default: 30)</td>
        </tr>
        <tr>
            <td><code>name_padding</code></td>
            <td>How many spaces to add before and after a window name.</td>
            <td align="center">number (default: 1)</td>
        </tr>
        <tr>
            <td><code>separator_string</code></td>
            <td>String displayed between window names.</td>
            <td align="center">any string (default: <code>"·"</code>)</td>
        </tr>
        <tr>
            <td><code>empty_desktop_string</code></td>
            <td>String to show when no windows are open.</td>
            <td align="center">any string (default: <code>""</code>)</td>
        </tr>
        <tr>
            <td>
                <code>active_window_left_click</code><br>
            </td>
            <td>Click actions for window names can be set as <code>"raise"</code>, <code>"minimize"</code> or <code>"close"</code>, or a custom script/program in the <code>click-actions</code> directory. Window currently in focus (active) and unfocused windows (inactive) are configurable separately.</td>
            <td>
                <ul>
                    <li>script name (default: <code>"minimize"</code>)</li>
                    <li><code>"none"</code>: no action</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td>
                <code>inactive_window_left_click</code><br>
            </td>
            <td>This is the main reason inactive window click actions can have a separate click action: it only makes sense to raise a window when it's not the active window.</td>
            <td>
                <ul>
                    <li>script name (default: <code>"raise"</code>)</li>
                    <li><code>"none"</code>: no action</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td>
                <code>active_window_right_click</code><br>
                <code>inactive_window_right_click</code><br>
            </td>
            <td>Right click is an example that has the same action regardless of active status, by default.</td>
            <td>
                <ul>
                    <li>script name (default: <code>"close"</code>)</li>
                    <li><code>"none"</code>: no action</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td>
                <code>active_window_middle_click</code><br>
                <code>active_window_scroll_up</code><br>
                <code>active_window_scroll_down</code><br>
                <code>active_window_left_double_click</code><br>
                <code>active_window_right_double_click</code><br>
                <code>active_window_middle_double_click</code><br><br>
                <code>inactive_window_middle_click</code><br>
                <code>inactive_window_scroll_up</code><br>
                <code>inactive_window_scroll_down</code><br>
                <code>inactive_window_left_double_click</code><br>
                <code>inactive_window_right_double_click</code><br>
                <code>inactive_window_middle_double_click</code><br>
            </td>
            <td>These are all the rest of the click actions that Polybar allows. They do nothing by default, but if you have an idea, see <a href="https://github.com/tuurep/windowlist?tab=readme-ov-file#scripting-click-actions">section about scripting</a>.<br><br>
            Note: for double-click actions, make sure to set <code>double-click-interval</code> (ms) to your preference in Polybar's <code>config.ini</code>.</td>
            <td>
                <ul>
                    <li>script name</li>
                    <li><code>"none"</code>: no action (default)</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td>
                <code>active_window_fg_color</code>
                <code>empty_desktop_fg_color</code>
                <code>overflow_fg_color</code>
            </td>
            <td>
                Foreground colors for:
                <ul>
                    <li>Currently focused window</li>
                    <li>The <code>empty_desktop_string</code></li>
                    <li>The string shown when <code>max_windows</code> exceeded</li>
                </ul>
            </td>
            <td>
                <ul>
                    <li>hex color string</li>
                    <li><code>"none"</code>: use fg color defined in your <code>config.ini</code> (default)</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td>
                <code>inactive_window_fg_color</code>
                <code>separator_fg_color</code>
            </td>
            <td>
                Foreground colors for:
                <ul>
                    <li>Windows not in focus</li>
                    <li>The <code>separator_string</code></li>
                </ul>
                You may want to set the color as your <code>foreground-alt</code> from your <code>config.ini</code> to best match your colorscheme. Unfortunately I couldn't make it use that directly because of reasons explained <a href="https://github.com/polybar/polybar/wiki/Formatting#format-tags-inside-polybar-config">here</a>.
            </td>
            <td>
                <ul>
                    <li>hex color string (default: <code>"#808080"</code>)</li>
                    <li><code>"none"</code>: use fg color defined in your <code>config.ini</code></li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>*_bg_color</code></td>
            <td>All of the foreground colors have a background color counterpart, e.g. <code>active_window_bg_color</code>.</td>
            <td>
                <ul>
                    <li>hex color string</li>
                    <li><code>"none"</code>: no background color (default)</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>*_ul_color</code></td>
            <td>All colors also have an underline color counterpart, e.g. <code>active_window_ul_color</code>.<br><br>
            Note that <code>line-size</code> must be set to 1 or higher in your Polybar <code>config.ini</code>, otherwise underline isn't visible.</td>
            <td>
                <ul>
                    <li>hex color string</li>
                    <li><code>"none"</code>: no underline (default)</li>
                </ul>
            </td>
        </tr>
        <tr>
            <td><code>*_font</code></td>
            <td> All elements can be set to use an alternative font defined in your Polybar <code>config.ini</code>. For example, to use <code>font-1</code> for the active window name, set <code>active_window_font = 1</code>.<br><br>
            To use a bold variant of a font, set e.g. <code>font-1 = DejaVu Sans:weight=bold</code> in <code>config.ini</code>.<br><br>
            See <a href="https://github.com/polybar/polybar/wiki/Fonts#fonts">here</a> for more details.</td>
            <td align="center">number (default: 0)</td>
        </tr>
        <tr>
            <td><code>ignored_classes</code></td>
            <td>Windows with a WM_CLASS in this array will not be shown on the bar. Strings are matched case insensitively.</td>
            <td align="center">array of strings (default: empty)</td>
        </tr>
        <tr>
            <td><code>window_nicknames</code></td>
            <td>A window name can be substituted with a custom name using key value pairs. The keys are matched case insensitively.
            <br><br>
            The key should be double quoted in case the window class contains special characters like dots (<code>.</code>).</td>
            <td align="center">table of string key-value pairs (default: empty)</td>
        </tr>
    </tbody>
</table>

> [!NOTE]
> Polybar must be reset before changes take effect.

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

* Wikipedia table to see if a window manager is EWMH compliant: [link](https://en.wikipedia.org/wiki/Comparison_of_X_window_managers#Features)
