CFLAGS += -std=c99 -Wall -Wextra -pedantic -Wold-style-declaration
CFLAGS += -Wmissing-prototypes -Wno-unused-parameter
PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
CC     ?= gcc

all: config.h wm

config.h:
	cp config.def.h config.h

wm: wm.o
	$(CC) $(LDFLAGS) -O3 -o wm wm.c -lX11

install: all
	install -Dm755 wm $(DESTDIR)$(BINDIR)/wm

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/wm

clean:
	rm -f wm *.o
