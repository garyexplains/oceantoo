CC=gcc
CFLAGS=-O3 -Wall

oceantoo : sha256.o oceantoo.o
	$(CC) $(CFLAGS) -o oceantoo sha256.o oceantoo.o

sha256.o : sha256.c sha256.h
	$(CC) $(CFLAGS) -c sha256.c

oceantoo.o : oceantoo.c oceantoo.h
	$(CC) $(CFLAGS) -c oceantoo.c
clean :
	rm oceantoo oceantoo.o sha256.o