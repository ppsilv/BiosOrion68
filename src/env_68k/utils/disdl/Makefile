CFLAGS = -O

TARGET = disdl
GROUPBIN = /usr/dac/68K/bin

OBJECTS = disdl.o dis68.o rd_dlfile.o rd_dlbytes.o

$(TARGET): $(OBJECTS)
	cc -o disdl $(OBJECTS)

install:	disdl
	cp $(TARGET) $(GROUPBIN)

clean:
	rm $(TARGET) $(OBJECTS)
