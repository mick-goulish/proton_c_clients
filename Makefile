CFLAGS=-Wall -Wextra -pedantic -std=c99
# CFLAGS+=-O0 -ggdb
CFLAGS+=-O3
CFLAGS+=$(shell pkg-config --cflags libqpid-proton)

LDFLAGS=$(shell pkg-config --libs libqpid-proton)

default: precv psend

%.o: %.c %.h
	$(CC) -c -o $@ $(CFLAGS) $<

precv: precv.o common.o
	$(CC) -o precv $(LDFLAGS) precv.o common.o

psend: psend.o common.o
	$(CC) -o psend $(LDFLAGS) psend.o common.o

clean:
	rm -f *.o
	rm -f precv psend

all: default
