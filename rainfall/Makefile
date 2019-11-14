CC=gcc
CFLAGS=-O3 -fPIC
DEPS=

all: rainfall_seq rainfall_pt
lib: 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< -g

clean:
	rm -f *~ *.o *.so rainfall_seq rainfall_pt

clobber:
	rm -f *~ *.o
