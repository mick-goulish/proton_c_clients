CFLAGS=-Wall -Wextra -pedantic -std=c99
# CFLAGS+=-O0 -ggdb
CFLAGS+=-O3
CFLAGS+=$(shell pkg-config --cflags libqpid-proton)

LDFLAGS=$(shell pkg-config --libs libqpid-proton)

precv: precv.c
	$(CC) -o precv $(CFLAGS) $(LDFLAGS) precv.c

clean:
	rm -f *.o
	rm -f precv
