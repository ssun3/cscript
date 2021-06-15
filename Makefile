PREFIX ?= /usr/local
DESTDIR ?=
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man

CFLAGS ?= -O3 -march=native
CFLAGS += -std=gnu99 -Wall -D_GNU_SOURCE

all: cscript

cscript:

install: cscript
	@install -v -d "$(DESTDIR)$(BINDIR)" && install -v -m 0755 cscript "$(DESTDIR)$(BINDIR)/cscript"

clean:
	rm -f cscript

.PHONY: all clean install
