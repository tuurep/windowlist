# windowlist

Originally began as a fork of my favorite Polybar script `polywins` by `tam-carre` (now deleted account)

Here is a mirror of the original script: [link](https://github.com/uniquepointer/polywins)

Windowlist has been fully rewritten in C using the relevant parts of the source code from `wmctrl` and `xprop`.

*TODO: Implement user settings*

## Improvements:

* Fixed a bug where names would not be correct if WM_CLASS contains spaces or dots
* Option to sort the window list:
    * By horizontal position on the screen
    * Alphabetically

## Planned:

* More styling options:
    * Style minimized window differently
    * Style visible and non-visible windows differently
* Window naming configurability:
    * Window nicknames (Example: `telegram-desktop` -> `telegram`)

## Installation:

Project folder should be in `~/.config/polybar/scripts/`

In `~/.config/polybar/scripts/windowlist/` run `make`

Add module in `~/.config/polybar/config.ini`:

```dosini
[module/windowlist]
type = custom/script
exec = ~/.config/polybar/scripts/windowlist/main 2>/dev/null
label-padding = 1
tail = true
```

Add module `windowlist` in any of `modules-left`, `modules-center` or `modules-right`
