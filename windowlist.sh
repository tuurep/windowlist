#!/bin/sh

# SETTINGS {{{ ---

base_dir="$HOME/.config/polybar/scripts/windowlist"

active_text_color="#dfdfdf"
active_bg=
active_underline=

inactive_text_color="#6d6d6d"
inactive_bg=
inactive_underline=

separator="·"
separator_color="$inactive_text_color"

forbidden_classes=
empty_desktop_message=

char_limit=30
max_windows=15
char_case="lower" # normal, upper, lower
add_spaces="true"
resize_increment=16
wm_border_width=1 # setting this might be required for accurate resize position

sort="position"

# --- }}}


main() {
	# If no argument passed...
	if [ -z "$2" ]; then
		# ...print new window list every time
		# the active window changes or
		# a window is opened or closed
		xprop -root -spy _NET_CLIENT_LIST _NET_ACTIVE_WINDOW |
			while read -r _; do
				generate_window_list
			done

	# If arguments are passed, run requested on-click function
	else
		"$@"
	fi
}


# ON-CLICK FUNCTIONS {{{ ---

raise_or_minimize() {
	if [ "$(get_active_wid)" = "$1" ]; then
		wmctrl -ir "$1" -b toggle,hidden
	else
		wmctrl -ir "$1" -b remove,hidden; wmctrl -ia "$1"
	fi
}

close() {
	wmctrl -ic "$1"
}

slop_resize() {
	wmctrl -ia "$1"
	wmctrl -ir "$1" -e "$(slop -f 0,%x,%y,%w,%h)"
}

increment_size() {
	while read -r wid ws wx wy ww wh _; do
		test "$wid" != "$1" && continue
		x=$(( wx - wm_border_width * 2 - resize_increment / 2 ))
		y=$(( wy - wm_border_width * 2 - resize_increment / 2 ))
		w=$(( ww + resize_increment ))
		h=$(( wh + resize_increment ))
	done <<-EOF
	$(wmctrl -lG)
	EOF

	wmctrl -ir "$1" -e "0,$x,$y,$w,$h"
}

decrement_size() {
	while read -r wid ws wx wy ww wh _; do
		test "$wid" != "$1" && continue
		x=$(( wx - wm_border_width * 2 + resize_increment / 2 ))
		y=$(( wy - wm_border_width * 2 + resize_increment / 2 ))
		w=$(( ww - resize_increment ))
		h=$(( wh - resize_increment ))
	done <<-EOF
	$(wmctrl -lG)
	EOF

	wmctrl -ir "$1" -e "0,$x,$y,$w,$h"
}

# --- }}}


# WINDOW LIST SETUP {{{ ---

active_left="%{F$active_text_color}"
active_right="%{F-}"
inactive_left="%{F$inactive_text_color}"
inactive_right="%{F-}"
separator="%{F$separator_color}$separator%{F-}"

if [ -n "$active_underline" ]; then
	active_left="${active_left}%{+u}%{u$active_underline}"
	active_right="%{-u}${active_right}"
fi

if [ -n "$active_bg" ]; then
	active_left="${active_left}%{B$active_bg}"
	active_right="%{B-}${active_right}"
fi

if [ -n "$inactive_underline" ]; then
	inactive_left="${inactive_left}%{+u}%{u$inactive_underline}"
	inactive_right="%{-u}${inactive_right}"
fi

if [ -n "$inactive_bg" ]; then
	inactive_left="${inactive_left}%{B$inactive_bg}"
	inactive_right="%{B-}${inactive_right}"
fi

get_active_wid() {
	active_wid=$(xprop -root _NET_ACTIVE_WINDOW)
	active_wid="${active_wid#*\# }"
	active_wid="${active_wid%,*}" # Necessary for XFCE
	while [ ${#active_wid} -lt 10 ]; do
		active_wid="0x0${active_wid#*x}"
	done
	echo "$active_wid"
}

generate_window_list() {
	active_wid=$(get_active_wid)
	window_count=0
	on_click="$0"

	# Format each window name one by one
	while read -r wid x y class; do

		# Don't show the window if its class is forbidden
		case "$forbidden_classes" in
			*$class*) continue ;;
		esac

		# If max number of windows reached, just increment
		# the windows counter
		if [ "$window_count" -ge "$max_windows" ]; then
			window_count=$(( window_count + 1 ))
			continue
		fi
		
		# Use user-selected character case
		case "$char_case" in
			"lower") class=$(
				echo "$class" | tr '[:upper:]' '[:lower:]'
				) ;;
			"upper") class=$(
				echo "$class" | tr '[:lower:]' '[:upper:]'
				) ;;
		esac

		# Truncate displayed name to user-selected limit
		if [ "${#class}" -gt "$char_limit" ]; then
			class="$(echo "$class" | cut -c1-$((char_limit-1)))…"
		fi

		# Apply add-spaces setting
		if [ "$add_spaces" = "true" ]; then
			class=" $class "
		fi

		# Add left and right formatting to displayed name
		if [ "$wid" = "$active_wid" ]; then
			class="${active_left}${class}${active_right}"
		else
			class="${inactive_left}${class}${inactive_right}"
		fi

		# Add separator unless the window is first in list
		if [ "$window_count" != 0 ]; then
			printf "%s" "$separator"
		fi

		# Add on-click action Polybar formatting
                # A1: Left click  -  A2: Middle click  -  A3: Right click
                # A4: Scroll up   -  A5: Scroll down
		printf "%s" "%{A1:$on_click raise_or_minimize $wid:}"
		printf "%s" "%{A3:$on_click close $wid:}"

		# Print the final window name
		printf "%s" "$class"
		printf "%s" "%{A}%{A}"

		window_count=$(( window_count + 1 ))
	done <<-EOF
        $(
                case "$sort" in
                        "position")
                                "$base_dir"/list_windows | sort -nk 2 -nk 3
                                ;;
                        "alphabetic")
                                "$base_dir"/list_windows | sort -k 4
                                ;;
                        *)
                                "$base_dir"/list_windows
                                ;;
                esac
        )
	EOF

	# After printing all the windows,
	# print number of hidden windows
	if [ "$window_count" -gt "$max_windows" ]; then
		printf "%s" "+$(( window_count - max_windows ))"
	fi

	# Print empty desktop message if no windows are open
	if [ "$window_count" = 0 ]; then
		printf "%s" "$empty_desktop_message"
	fi
	
	# Print newline
	echo ""
}

# --- }}}

main "$@"
