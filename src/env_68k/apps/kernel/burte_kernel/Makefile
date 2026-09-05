# makefile for real-time executive aplications that
# work with the popeye kernel
#
#  This makefile creates 2 different RAM-based shells:
#    shell.dl - standard 332 configuration
#    shellexp.dl - expanded memory configuration
# 
# 
# HISTORY
#
#  Andrew H. Fagg     	02/19/95	Added expanded memory version B
#					  (shellexpB.dl)
#


all: shell.dl shellexp.dl shellexpB.dl

LIBS = -L../../lib -lburte
INCLUDE = -I../../include

CFLAGS= $(INCLUDE) -O
LNK = -N -Ttext d000 ../../lib/userstart.o #../../lib/crt068.o
LNK2 = -N -Ttext d000 ../../lib/userstart.o #../../lib/crt068.o
CC = gcc

.SUFFIXES: .o .c .s
.c.o: ; $(CC) -target sun3 -msoft-float $(CFLAGS) -c $*.c
.s.o: ; $(CC) -target sun3 -msoft-float $(CFLAGS) -c $*.s

startexp.o: start.c
	$(CC) -target sun3 -msoft-float $(CFLAGS) -c -DEXPAND start.c \
		-o startexp.o

startexpB.o: start.c
	$(CC) -target sun3 -msoft-float $(CFLAGS) -c -DEXPAND_B start.c \
		-o startexpB.o
	

BSRC = exec.c a_exec.s ihandle.s \
serial.c baud.c timer.c file.c alloc.c download.c \
romcalls.c shell.c debug.c break.c putcharacter.s tpu.c disasm.c

BOBJ =  exec.o a_exec.o ihandle.o \
serial.o baud.o timer.o file.o alloc.o download.o \
romcalls.o shell.o debug.o break.o putcharacter.o tpu.o disasm.o

# Expanded memory version
ESRC = cs.c startexp.c
EOBJ = cs.o startexp.o

# Expanded memory version
ESRC_B = csB.c startexpB.c
EOBJ_B = csB.o startexpB.o

# Not Expanded memory version
NESRC = start.c
NEOBJ = start.o

SOURCES = hi.c $(BSRC) $(ESRC) $(NESRC)

all:	shell.dl hi.dl

shell.dl:	$(BOBJ) $(NEOBJ) crt068.o
	gld -target sun3 -N -Ttext 5040 -o shell crt068.o $(BOBJ) $(NEOBJ) $(LIBS)
	dl68 -o $@ shell

shellexp.dl:	$(BOBJ) crt068.o $(EOBJ)
	gld -target sun3 -N -Ttext 5040 -o shellexp crt068.o $(BOBJ) $(EOBJ) \
		$(LIBS) 
	dl68 -o $@ shellexp

shellexpB.dl:	$(BOBJ) crt068.o $(EOBJ_B)
	gld -target sun3 -N -Ttext 5040 -o shellexpB crt068.o $(BOBJ) $(EOBJ_B) \
		$(LIBS) 
	dl68 -o $@ shellexpB

rom.bin: $(BOBJ) $(EOBJ_B) romstart.o
	gld -target sun3 -N -Ttext f0000 -Tbss 5040 -o rom romstart.o $(BOBJ) \
		$(EOBJ_B) $(LIBS)
	mkbin -o rom.bin rom


bytes:	rom.dl
	sdl rom.dl

hi.dl:	hi.o
	gld -target sun3 -o hi $(LNK2) hi.o $(LIBS)
	dl68 -o $@ hi

sleeptest.dl:	sleeptest.o
	gld -target sun3 -o sleeptest $(LNK) sleeptest.o $(LIBS)
	dl68 -o $@ sleeptest

printtest.dl:	printtest.o
	gld -target sun3 -o printtest $(LNK) printtest.o $(LIBS)
	dl68 -o $@ printtest

sleeptest2.dl:	sleeptest2.o
	gld -target sun3 -o sleeptest2 $(LNK) sleeptest2.o $(LIBS)
	dl68 -o $@ sleeptest2

timetest.dl:	timetest.o
	gld -target sun3 -o timetest $(LNK) timetest.o $(LIBS)
	dl68 -o $@ timetest

hello.dl:	hello.o
	gld -target sun3 -o hello $(LNK2) hello.o $(LIBS)
	dl68 -o $@ hello

lint:
	lint $(INCLUDE) $(BSRC) shell.c break.c

depend:
	cp Makefile Makefile.bak
	makedepend -s "# DO NOT DELETE" $(CFLAGS) $(SOURCES) 

clean:
	rm -f *.o a.out core pophi hi *~

backup:
	cp *.c bak
	cp *.h bak
	cp *.s bak
	cp Makefile bak

install:
	cp crt068.o ../../lib
	cp shell.dl ../../bin
	cp shellexp.dl ../../bin
	cp shellexpB.dl ../../bin

# DO NOT DELETE THIS LINE IT IS USED BY MAKEDEPEND

hi.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
hi.o: ../../include/romcalls.h
exec.o: ../../include/burte.h ../../include/burteio.h system.h
exec.o: ../../include/trap-service.h
serial.o: ../../include/burte.h ../../include/burteio.h
serial.o: ../../include/service.h ../../include/Oserial.h
serial.o: ../../include/68332.h ../../include/68332new.h
baud.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
baud.o: ../../include/romcalls.h ../../include/Oserial.h
baud.o: ../../include/68332.h ../../include/68332new.h
timer.o: ../../include/burte.h ../../include/service.h ../../include/Otimer.h
timer.o: ../../include/68332.h ../../include/68332new.h
file.o: ../../include/burte.h ../../include/burteio.h
download.o: ../../include/stdio.h ../../include/burte.h
download.o: ../../include/burteio.h ../../include/romcalls.h
romcalls.o: ../../include/burte.h ../../include/burteio.h system.h
romcalls.o: ../../include/romcalls.h
shell.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
shell.o: ../../include/romcalls.h ../../include/rline.h system.h
debug.o: ../../include/burte.h ../../include/burteio.h system.h
debug.o: ../../include/Oserial.h ../../include/68332.h
debug.o: ../../include/68332new.h
break.o: ../../include/burte.h ../../include/burteio.h
tpu.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
tpu.o: ../../include/romcalls.h system.h ../../include/68332.h
tpu.o: ../../include/68332new.h
disasm.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
disasm.o: ../../include/romcalls.h
cs.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
cs.o: ../../include/romcalls.h system.h ../../include/btpulib.h
cs.o: ../../include/68332.h ../../include/68332new.h
startexp.o: ../../include/stdio.h ../../include/burte.h
startexp.o: ../../include/burteio.h ../../include/romcalls.h system.h
startexp.o: ../../include/service.h ../../include/trap-service.h
startexp.o: ../../include/Oserial.h ../../include/68332.h
startexp.o: ../../include/68332new.h
start.o: ../../include/stdio.h ../../include/burte.h ../../include/burteio.h
start.o: ../../include/romcalls.h system.h ../../include/service.h
start.o: ../../include/trap-service.h ../../include/Oserial.h
start.o: ../../include/68332.h ../../include/68332new.h
