CFLAGS = -g -O2 -Wall
LDFLAGS = -lX11

all: main windowlist.o click-actions/raise click-actions/minimize click-actions/close

main: main.c windowlist.o windowlist.h toml-c.h
	gcc $(CFLAGS) $(LDFLAGS) -o main main.c windowlist.o

windowlist.o: windowlist.c
	gcc $(CFLAGS) -c windowlist.c

click-actions/raise: click-actions/raise.c
	gcc $(CFLAGS) $(LDFLAGS) -o click-actions/raise click-actions/raise.c

click-actions/minimize: click-actions/minimize.c
	gcc $(CFLAGS) $(LDFLAGS) -o click-actions/minimize click-actions/minimize.c

click-actions/close: click-actions/close.c
	gcc $(CFLAGS) $(LDFLAGS) -o click-actions/close click-actions/close.c

clean:
	rm windowlist.o
	rm main
	rm click-actions/raise
	rm click-actions/minimize
	rm click-actions/close
