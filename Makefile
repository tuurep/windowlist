CFLAGS = -g -O2 -Wall
LDFLAGS = -lX11

# Todo: BINDIR

BINS = build/windowlist \
       build/windowlist-close \
       build/windowlist-minimize \
       build/windowlist-raise

.PHONY: all clean # install uninstall

all: $(BINS)

# Ensure build dir is created before it's needed
build/:
	mkdir -p build

build/windowlist: src/windowlist.c \
		  build/xlib-utils.o \
		  src/xlib-utils.h \
		  src/toml-c.h | build/

	gcc -o build/windowlist src/windowlist.c build/xlib-utils.o $(LDFLAGS)

build/xlib-utils.o: src/xlib-utils.c \
		    src/xlib-utils.h | build/

	gcc $(CFLAGS) -c -o $@ $<

build/actions-common.o: src/click-actions/actions-common.c \
			src/click-actions/actions-common.h | build/

	gcc $(CFLAGS) -c -o $@ $<

build/windowlist-raise: src/click-actions/windowlist-raise.c \
			build/actions-common.o \
			src/click-actions/actions-common.h \
			build/xlib-utils.o \
			src/xlib-utils.h | build/

	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/windowlist-minimize: src/click-actions/windowlist-minimize.c \
			   build/actions-common.o \
			   src/click-actions/actions-common.h | build/

	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/windowlist-close: src/click-actions/windowlist-close.c \
			build/actions-common.o \
			src/click-actions/actions-common.h | build/

	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Todo: install to BINDIR

# Todo: uninstall from BINDIR

clean:
	rm -rf build
