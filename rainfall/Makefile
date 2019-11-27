CC = gcc
CFLAGS = -O3 -fPIC
LIB = -lpthread

all: rainfall_seq rainfall_pt

rainfall_seq: rainfall_seq.c
	$(CC) $(CFLAGS) -pg -o rainfall_seq rainfall_seq.c

rainfall_pt: rainfall_pt.c rainfall_pt.h
	$(CC) $(CFLAGS) -o rainfall_pt rainfall_pt.c $(LIB)

clean:
	rm -f *~ *.o *.so rainfall_seq rainfall_pt

clobber:
	rm -f *~ *.o
