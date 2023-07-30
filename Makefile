CFLAGS = -g -O2 -Wall
LDFLAGS = -lX11

all: main windowlist.o

main: main.c windowlist.o windowlist.h
	gcc $(CFLAGS) -o main main.c windowlist.o $(LDFLAGS)

windowlist.o: windowlist.c
	gcc $(CFLAGS) -c windowlist.c

clean:
	rm windowlist.o
	rm main
