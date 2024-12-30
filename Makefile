CFLAGS = -g -O2 -Wall
LDFLAGS = -lX11

.PHONY: all clean

all: main click-actions/raise click-actions/minimize click-actions/close

main: main.c windowlist.o windowlist.h toml-c.h
	gcc $(CFLAGS) $(LDFLAGS) -o main main.c windowlist.o

windowlist.o: windowlist.c
	gcc $(CFLAGS) -c windowlist.c

click-actions/src/common.o: click-actions/src/common.c
	gcc $(CFLAGS) -c -o click-actions/src/common.o click-actions/src/common.c

# raise, minimize, close executables
click-actions/%: click-actions/src/%.c click-actions/src/common.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f main \
	      windowlist.o \
	      click-actions/src/common.o \
	      click-actions/raise \
	      click-actions/minimize \
	      click-actions/close
