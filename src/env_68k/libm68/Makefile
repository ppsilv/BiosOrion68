#
#	MAKEFILE FOR THE 68000 MATH LIBRARY
#
# HISTORY
# 10-Dec-83  Rafael Bracho (rxb) at Carnegie-Mellon University
#	gamma function added.
#
# 21-Oct-83  Rafael Bracho (rxb) at Carnegie-Mellon University
#	error function (erfc.c) code added.
#
# 13-May-83  Rafael Bracho (rxb) at Carnegie-Mellon University
#	Created. All these files were on the standard library.
#
#

.SUFFIXES: .b .s .c
.s.b: ; cc68 -c -V $*.s
.c.b: ; cc68 -c $(SCFLAGS) $*.c

SCFLAGS = -O -V -K

SOURCES = Makefile math.h abs.c fabs.c floor.c \
	sin.c tan.c asin.c atan.c sinh.c tanh.c \
	lsqrt.c sqrt.c exp.c log.c pow.c gamma.c \
	hypot.c rand.c j0.c j1.c jn.c erfc.c

OBJECTS = abs.b fabs.b floor.b sin.b tan.b asin.b \
	atan.b sinh.b tanh.b j0.b j1.b jn.b lsqrt.b \
	sqrt.b exp.b log.b pow.b hypot.b rand.b erfc.b \
	gamma.b

# This is what we make

libm68.a:	$(OBJECTS)
		ar cr libm68.a `lorder68 $(OBJECTS) | tsort`

print: .print
.print: $(SOURCES)
	yapp $?
	touch .print
