#
# Makefile for Linux
#

CC = cc
CFLAGS = -O3 -Wall
LIBS = `sdl-config --libs`
PREFIX = ../

all_cflags = $(CFLAGS) `sdl-config --cflags`
objs = bme.o bme_gfx.o bme_io.o bme_win.o bme_kbd.o bme_mou.o		\
bme_joy.o bme_map.o bme_txt.o bme_snd.o bme_mod.o bme_xm.o bme_s3m.o	\
bme_raw.o bme_wav.o bme_smp.o bme_tbl.o
picobjs = $(objs:.o=.pic.o)
so = $(PREFIX)libbme.so.0

all: $(PREFIX)libbme.a $(so) $(PREFIX)bmeconv $(PREFIX)datafile $(PREFIX)dat2inc $(PREFIX)mapedit $(PREFIX)infedit $(PREFIX)bmeplay
.PHONY: all

clean: linkclean
	rm -f *.o
	rm -f mapdata.c
	rm -f mapedit.dat
	rm -f *.spr
.PHONY: clean

linkclean:
	rm -f $(PREFIX)libbme.a
	rm -f $(so)
	rm -f $(PREFIX)bmeconv
	rm -f $(PREFIX)bmeplay
	rm -f $(PREFIX)dat2inc
	rm -f $(PREFIX)datafile
	rm -f $(PREFIX)infedit
	rm -f $(PREFIX)mapedit
.PHONY: linkclean

$(PREFIX)libbme.a: $(objs)
	ar -rs $@ $^

%.o: %.c
	$(CC) -c $(all_cflags) -o $@ $<

$(so): versions $(picobjs)
	$(CC) -shared -o $@ -Wl,--version-script=versions,-soname=$(so) $(picobjs) $(LIBS)

%.pic.o: %.c
	$(CC) -c -fPIC $(all_cflags) -o $@ $<

mapdata.c: mapedit.dat $(PREFIX)dat2inc
	$(PREFIX)dat2inc $< $@

mapedit.dat: mapedit.seq $(PREFIX)datafile fonts.spr editor.spr
	$(PREFIX)datafile $@ $<

%.spr: %.lbm $(PREFIX)bmeconv
	$(PREFIX)bmeconv -s $< $@

$(PREFIX)mapedit: mapedit.o fileio.o cfgfile.o mapdata.o $(PREFIX)libbme.a 
	$(CC) -o $@ $^ $(LIBS)

$(PREFIX)infedit: infedit.o fileio.o cfgfile.o mapdata.o $(PREFIX)libbme.a
	$(CC) -o $@ $^ $(LIBS)

$(PREFIX)bmeplay: bmeplay.o $(PREFIX)libbme.a
	$(CC) -o $@ $^ $(LIBS)

$(PREFIX)datafile: datafile.o fileio.o
	$(CC) -o $@ $^

$(PREFIX)bmeconv: bmeconv.o fileio.o
	$(CC) -o $@ $^

$(PREFIX)dat2inc: dat2inc.o
	$(CC) -o $@ $^
