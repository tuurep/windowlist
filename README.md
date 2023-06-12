# windowlist

Fork of my favorite Polybar script `polywins` by `tam-carre` (now deleted account)

Here is a mirror of the script: [link](https://github.com/uniquepointer/polywins)

Polybar: [link](https://github.com/polybar/polybar)

## Improvements:

* Fixed a bug where names would not be correct if WM_CLASS contains spaces or dots
* Option to sort the window list:
    * Alphabetically
    * By horizontal position on the screen

## Planned:

* Update list when window positions change
    * Currently it only updates when active window changes, or window gets added/removed
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
exec = ~/.config/polybar/scripts/windowlist/windowlist.sh 2>/dev/null
format = <label>
label = %output%
label-padding = 1
tail = true
```

Add module `windowlist` in any of `modules-left`, `modules-center` or `modules-right`

## Dependencies:

* `wmctrl`
* `xprop`
    * Arch: [`xorg-xprop`](https://archlinux.org/packages/extra/x86_64/xorg-xprop/)
    * Debian: [`x11-utils`](https://packages.debian.org/bullseye/x11-utils)
