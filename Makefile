CFLAGS ?= -g -ftree-vectorize -Wformat=0
CFLAGS += -pedantic -std=gnu99 -Wall -Wextra -I inc/sai/inc
#LIBS=-lm -lrt -libverbs -lhugetlbfs -lrdmacm -lpthread -lcap
LIBS=-lm -lrt -lsai
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MAN1DIR=$(PREFIX)/share/man/man1

SRCS=smarttor.cpp
OBJS=$(SRCS:.c=.o)
BINARY=sai2tc

#MANS=ibv_rc_pingpong_as_notify.1
#MANS_F=$(MANS:.1=.txt) $(MANS:.1=.pdf)
DOCS=README.md LICENSE changelog
#SPEC=ibv_rc_pingpong_as_notify.spec

PACKAGE=sai2tc
SRC_VER:=$(shell sed -ne 's/\#define VERSION \"\(.*\)\"/\1/p' smarttor.cpp)
VERSION:=$(SRC_VER)
DISTDIR=$(PACKAGE)-$(VERSION)
DISTFILES=$(SRCS) $(MANS) $(DOCS) $(SPEC) Makefile
PACKFILES=$(BINARY) $(MANS) $(MANS_F) $(DOCS)

STRIP=strip
TARGET=$(shell ${CXX} -dumpmachine)

all: checkver $(BINARY) $(BINARY)

version: checkver
	@echo ${VERSION}

clean:
	$(RM) -f $(OBJS) $(BINARY) $(MANS_F) sai2tc.tmp

strip: $(BINARY) $(BINARY)
	$(STRIP) $^

install: $(BINARY) $(MANS)
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 0755 $(BINARY) $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MAN1DIR)
	install -m 644 $(MANS) $(DESTDIR)$(MAN1DIR)

%.o: %.c %.h
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.ps: %.1
	man -t ./$< > $@

%.pdf: %.ps
	ps2pdf $< $@

%.txt: %.1
	MANWIDTH=80 man ./$< | col -b > $@

$(BINARY): $(OBJS)
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

dist: checkver $(DISTFILES)
	tar -cz --transform='s,^,$(DISTDIR)/,S' $^ -f $(DISTDIR).tar.gz

binary-tgz: checkver $(PACKFILES)
	${STRIP} ${BINARY}
	tar -cz --transform='s,^,$(DISTDIR)/,S' -f ${PACKAGE}-${VERSION}-${TARGET}.tgz $^

binary-zip: checkver $(PACKFILES)
	${STRIP} ${BINARY}
	ln -s . $(DISTDIR)
	zip ${PACKAGE}-${VERSION}-${TARGET}.zip $(addprefix $(DISTDIR)/,$^)
	rm $(DISTDIR)

.PHONY: all version checkver clean strip test install dist binary-tgz binary-zip
