CFLAGS		= -g -W -Wall -Wstrict-prototypes -Wno-unused-parameter -Wwrite-strings
CPPFLAGS	= -I/usr/X11/include/cairo
LDFLAGS		= -L/usr/X11/lib
LDADD		= -lcairo
OBJS		= kplot.o \
		  kdata.o \
		  reallocarray.o \
		  label.o \
		  border.o \
		  margin.o \
		  draw.o \
		  tic.o \
		  hist.o \
		  array.o \
		  vector.o
PREFIX		= /usr/local
MANS		= man/kdata_array_alloc.3 \
		  man/kdata_array_fill.3 \
		  man/kdata_array_realloc.3 \
		  man/kdata_copy.3 \
		  man/kdata_destroy.3 \
		  man/kdata_hist_alloc.3 \
		  man/kdata_hist_increment.3 \
		  man/kdata_vector_add.3 \
		  man/kdata_vector_alloc.3 \
		  man/kplot.3 \
		  man/kplot_alloc.3 \
		  man/kplot_draw.3 \
		  man/kplot_free.3

all: libkplot.a example1 example2 example3

install: all
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/share/man/man3
	install -m 0444 libkplot.a $(PREFIX)/lib
	install -m 0444 kplot.h $(PREFIX)/include
	install -m 0444 $(MANS) $(PREFIX)/share/man/man3

example1 example2: libkplot.a

example1: example1.c libkplot.a
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDADD) libkplot.a -o $@ $<

example2: example2.c libkplot.a
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDADD) libkplot.a -o $@ $<

example3: example3.c libkplot.a
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
	rm -f libkplot.a compat.h test-reallocarray 
	rm -f example1 example2 example3
	rm -rf *.dSYM
	rm -f $(OBJS)
