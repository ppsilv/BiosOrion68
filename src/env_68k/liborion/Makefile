#
#	Makefile for generating the standard library (libpopI68.a)
#
#	When making 'release', the sources go to /usr/popeye/src/libpopI68,
#	and the library goes to /usr/p68/lib.
#
INCLUDE = -I../../include
CFLAGS = -O $(INCLUDE)
# 68000 compiler flags for Omnibytes
OB = -target sun3 
#-msoft-float

.SUFFIXES: .o .c .s
.c.o: ; gcc $(OB) $(CFLAGS) -c $*.c
.s.o: ; gcc $(OB) $(CFLAGS) -c $*.s

SOURCES = aldiv.s almul.s alrem.s atof.c atoi.c atox.c blt.s bm.s \
boolarg.c bs.s chrarg.c clear.s crypt.c doublearg.c fgetc.c fgets.c \
fprintf.c fputc.c fputcn.c fputi.c fputs.c fputsi.c fputsx.c fputx.c \
getbool.c getdouble.c gethex.c getint.c getline.c getstab.c getstr.c \
gnulib.c hexarg.c index.c intarg.c ldiv.s lmul.s lrem.s lshift.s \
nxtarg.c path.c prstab.c qprintf.c qsort.c quit.c regex.c rindex.c \
rline.c skipto.c sputi.c sputs.c sputsi.c sputsx.c sputx.c stabarg.c stablk.c \
stlmatch.c strarg.c strcat.c strcmp.c strcpy.c strlen.c strncat.c \
strncmp.c strncpy.c strrev.c swab.c system.s sys_vars.s term.c trap.c uldiv.s \
tas.s ulmul.s ulrem.s ws.s skipwords.c
#float.s

OBJECTS = aldiv.o almul.o alrem.o atof.o atoi.o atox.o blt.o bm.o \
boolarg.o bs.o chrarg.o clear.o crypt.o doublearg.o fgetc.o fgets.o \
fprintf.o fputc.o fputcn.o fputi.o fputs.o fputsi.o fputsx.o fputx.o \
getbool.o getdouble.o gethex.o getint.o getline.o getstab.o getstr.o \
gnulib.o hexarg.o index.o intarg.o ldiv.o lmul.o lrem.o lshift.o \
nxtarg.o path.o prstab.o qprintf.o qsort.o quit.o regex.o rindex.o \
rline.o skipto.o sputi.o sputs.o sputsi.o sputsx.o sputx.o stabarg.o stablk.o \
stlmatch.o strarg.o strcat.o strcmp.o strcpy.o strlen.o strncat.o \
strncmp.o strncpy.o strrev.o swab.o system.o sys_vars.o term.o trap.o uldiv.o \
tas.o ulmul.o ulrem.o ws.o skipwords.o
#float.o

USRC = atof.c atoi.c atox.c fputcn.c fputi.c fputsi.c fputsx.c fputx.c \
sputs.c sputi.c sputsi.c sputsx.c sputx.c skipto.c  \
strcat.c strcmp.c strcpy.c strlen.c strncat.c \
strncmp.c strncpy.c strrev.c swab.c crypt.c

UOBJ = atof.o atoi.o atox.o fputcn.o fputi.o fputsi.o fputsx.o fputx.o \
sputs.o sputi.o sputsi.o sputsx.o sputx.o skipto.o  \
strcat.o strcmp.o strcpy.o strlen.o strncat.o \
strncmp.o strncpy.o strrev.o swab.o crypt.o

# default make is to build and install the libraries
install:	libburte.a userstart.o
	cp libburte.a ../../lib/libburte.a
	ranlib -t ../../lib/libburte.a
	cp userstart.o ../../lib

all:	libburte.a userstart.o libuburte.a

libburte.a:	$(OBJECTS)
		rm -f $@
		ar cr $@ $(OBJECTS)
		ranlib $@

libuburte.a:
	cd ub; \
	for fl in $(USRC); \
	do \
	  echo $${fl}; \
	  cp ../$${fl} .; \
	  gcc -g -c $${fl}; \
	done; \
	rm -f $@; \
	ar cr $@ $(UOBJ); \
	ranlib $@;
	mv ub/$@ .
	ranlib -t $@

uinstall:	libuburte.a
	cp libuburte.a ../../lib/libuburte.a
	ranlib -t ../../lib/libuburte.a

depend:
	cp Makefile Makefile.bak
	makedepend -s "# DO NOT DELETE" $(CFLAGS) $(SOURCES)

clean:
	rm -f *.o *~

backup:
	cp *.c bak
	cp Makefile bak
	cp *.s bak

# you cannot optimize c files with imbedded assembly.
trap.o:	trap.c
	gcc $(OB) $(INCLUDE) -c trap.c

# specific dependencies on files
# DO NOT DELETE THIS LINE IT IS USED BY MAKEDEPEND

atof.o: /usr/include/ctype.h
boolarg.o: ../../include/stdio.h ../../include/burte.h
boolarg.o: ../../include/burteio.h ../../include/romcalls.h
chrarg.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
chrarg.o: ../../include/romcalls.h
doublearg.o: ../../include/stdio.h ../../include/burte.h
doublearg.o: ../../include/burteio.h ../../include/romcalls.h
fgetc.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
fgetc.o: ../../include/romcalls.h
fgets.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
fgets.o: ../../include/romcalls.h
fprintf.o: ../../include/stdio.h ../../include/burte.h
fprintf.o: ../../include/burteio.h ../../include/romcalls.h
fputc.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
fputc.o: ../../include/romcalls.h
fputi.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
fputi.o: ../../include/romcalls.h
fputs.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
fputs.o: ../../include/romcalls.h
fputsi.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
fputsi.o: ../../include/romcalls.h
fputsx.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
fputsx.o: ../../include/romcalls.h
fputx.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
fputx.o: ../../include/romcalls.h
getbool.o: ../../include/stdio.h ../../include/burte.h
getbool.o: ../../include/burteio.h ../../include/romcalls.h
getdouble.o: ../../include/stdio.h ../../include/burte.h
getdouble.o: ../../include/burteio.h ../../include/romcalls.h
gethex.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
gethex.o: ../../include/romcalls.h
getint.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
getint.o: ../../include/romcalls.h
getline.o: ../../include/stdio.h ../../include/burte.h
getline.o: ../../include/burteio.h ../../include/romcalls.h
getstab.o: ../../include/stdio.h ../../include/burte.h
getstab.o: ../../include/burteio.h ../../include/romcalls.h
hexarg.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
hexarg.o: ../../include/romcalls.h
intarg.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
intarg.o: ../../include/romcalls.h
qprintf.o: ../../include/stdio.h ../../include/burte.h
qprintf.o: ../../include/burteio.h ../../include/romcalls.h
quit.o: ../../include/burteio.h
rline.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
rline.o: ../../include/romcalls.h ../../include/rline.h ../../include/term.h
skipto.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
skipto.o: ../../include/romcalls.h
sputi.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
sputi.o: ../../include/romcalls.h
sputsi.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
sputsi.o: ../../include/romcalls.h
sputsx.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
sputsx.o: ../../include/romcalls.h
sputx.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
sputx.o: ../../include/romcalls.h
stablk.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
stablk.o: ../../include/romcalls.h
stlmatch.o: ../../include/stdio.h ../../include/burte.h
stlmatch.o: ../../include/burteio.h ../../include/romcalls.h
term.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
term.o: ../../include/romcalls.h ../../include/term.h
