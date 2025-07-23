CFLAGS = -g -O2 -Wall
LDFLAGS = -lX11

# `make install` destination
BINDIR ?= ~/.local/bin

BINS = build/windowlist \
       build/windowlist-close \
       build/windowlist-minimize \
       build/windowlist-raise

.PHONY: all install uninstall clean

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

install: all
	@BINDIR_EXPANDED=$$(eval echo $(BINDIR)); \
	mkdir -p "$$BINDIR_EXPANDED"; \
	for bin in $(BINS); do \
		TARGET=$$(basename $$bin); \
		echo "Installing $$bin to $$BINDIR_EXPANDED/$$TARGET"; \
		install -m 755 $$bin "$$BINDIR_EXPANDED/$$TARGET"; \
	done		

uninstall:
	@BINDIR_EXPANDED=$$(eval echo $(BINDIR)); \
	for bin in $(BINS); do \
		TARGET=$$(basename $$bin); \
		FILE="$$BINDIR_EXPANDED/$$TARGET"; \
		if [ -f "$$FILE" ]; then \
			echo "Removing $$FILE"; \
			rm -f "$$FILE"; \
		else \
			echo "Nothing to remove at $$FILE"; \
		fi; \
	done

clean:
	rm -rf build
