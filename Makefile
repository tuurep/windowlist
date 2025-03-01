CFLAGS = -g -O2 -Wall
LDFLAGS = -lX11

.PHONY: all clean

all: main click-actions/raise click-actions/minimize click-actions/close config

main: main.c windowlist.o windowlist.h toml-c.h
	gcc $(CFLAGS) $(LDFLAGS) -o main main.c windowlist.o

windowlist.o: windowlist.c
	gcc $(CFLAGS) -c -o $@ $^

click-actions/src/common.o: click-actions/src/common.c
	gcc $(CFLAGS) -c -o $@ $^

click-actions/raise: click-actions/src/raise.c click-actions/src/common.o windowlist.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

click-actions/minimize: click-actions/src/minimize.c click-actions/src/common.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

click-actions/close: click-actions/src/close.c click-actions/src/common.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

config:
	@if [ -f "config.toml" ]; then \
		echo "'config.toml' already exists, doing nothing"; \
	else \
		echo "Copying 'config-default.toml' -> 'config.toml'"; \
		cp config-default.toml config.toml; \
	fi

clean:
	rm -f main \
	      windowlist.o \
	      click-actions/src/common.o \
	      click-actions/raise \
	      click-actions/minimize \
	      click-actions/close
