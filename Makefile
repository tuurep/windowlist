CFLAGS = -g -O2 -Wall

GLIB_CFLAGS = -I/usr/include/glib-2.0 \
	      -I/usr/lib/glib-2.0/include \
	      -pthread

LDADD = -lX11 -lglib-2.0

all: list_windows.o list_windows eventlistener

list_windows.o: list_windows.c
	gcc $(CFLAGS) $(GLIB_CFLAGS) -c list_windows.c

list_windows: list_windows.o
	gcc $(CFLAGS) list_windows.o -o list_windows $(LDADD)

eventlistener: eventlistener.c
	gcc $(CFLAGS) eventlistener.c -o eventlistener -lX11

clean:
	rm list_windows.o
	rm list_windows
	rm eventlistener
