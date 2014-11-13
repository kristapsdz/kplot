CFLAGS = -g -W -Wall -Wstrict-prototypes -Wno-unused-parameter -Wwrite-strings
CPPFLAGS = -I/usr/X11/include/cairo
LDFLAGS = -L/usr/X11/lib
LDADD = -lcairo
OBJS = kplot.o reallocarray.o label.o border.o margin.o draw.o tic.o
PREFIX = /usr/local
MANS = man/kplot.3 man/kplot_draw.3

all: libkplot.a example1

install: all
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/share/man/man3
	install -m 0444 libkplot.a $(PREFIX)/lib
	install -m 0444 kplot.h $(PREFIX)/include
	install -m 0444 $(MANS) $(PREFIX)/share/man/man3

example1: libkplot.a

example1: example1.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDADD) libkplot.a -o $@ $<

libkplot.a: $(OBJS)
	$(AR) rs $@ $(OBJS)

$(OBJS): kplot.h compat.h extern.h

compat.h: compat.pre.h compat.post.h
	( cat compat.pre.h ; \
	  CC="$(CC)" CFLAGS="$(CFLAGS)" sh compat.sh reallocarray ; \
	  cat compat.post.h ; ) >$@

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f libkplot.a compat.h test-reallocarray example1
	rm -f $(OBJS)
