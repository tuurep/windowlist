CFLAGS = -g -O2 -Wall
LDFLAGS = -lX11

.PHONY: all install clean

all: main click-actions/raise click-actions/minimize click-actions/close config.toml

main: main.c windowlist.o windowlist.h toml-c.h
	gcc -DCOMPILE_DIR=\"$(shell pwd)\" $(CFLAGS) -o main main.c windowlist.o $(LDFLAGS)

windowlist.o: windowlist.c
	gcc $(CFLAGS) -c -o $@ $^

click-actions/src/common.o: click-actions/src/common.c
	gcc $(CFLAGS) -c -o $@ $^

click-actions/raise: click-actions/src/raise.c click-actions/src/common.o windowlist.o
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

click-actions/minimize: click-actions/src/minimize.c click-actions/src/common.o
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

click-actions/close: click-actions/src/close.c click-actions/src/common.o
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

config.toml: config-default.toml
	@if [ ! -f config.toml ]; then \
		echo "Copying 'config-default.toml' -> 'config.toml'"; \
		cp config-default.toml config.toml; \
	fi

install: all
	@if [ -z "$(DEST)" ]; then \
		echo    "Usage: make install DEST=/path/to/executable"; \
		echo -e "       (Warning: overwrites target)\n"; \
		exit 1; \
	fi
	@DEST_EXPANDED=$(shell eval echo $(DEST)); \
	if [ -d "$$DEST_EXPANDED" ]; then \
		echo    "'$$DEST_EXPANDED' is a directory!"; \
		echo -e "Provide path to executable, including filename.\n"; \
		exit 1; \
	fi; \
	echo "Installing 'main' as '$$DEST_EXPANDED'"; \
	mkdir -p "$$(dirname $$DEST_EXPANDED)"; \
	install -m 755 main "$$DEST_EXPANDED"

clean:
	rm -f main \
	      windowlist.o \
	      click-actions/src/common.o \
	      click-actions/raise \
	      click-actions/minimize \
	      click-actions/close
