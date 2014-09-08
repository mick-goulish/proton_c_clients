CFLAGS=-Wall -Wextra -pedantic -std=c99
# CFLAGS+=-O0 -ggdb
CFLAGS+=-O3
CFLAGS+=$(shell pkg-config --cflags libqpid-proton)

LDFLAGS=$(shell pkg-config --libs libqpid-proton)

psend: psend.c
	$(CC) -o psend $(CFLAGS) $(LDFLAGS) psend.c

clean:
	rm -f *.o
	rm -f psend
