################################################################################
#                       # Projekt: DU1 - IJC             #                     #
#                       # Autor: Andrej Barna (xbarna01) #                     #
#                       # Datum: 26.03.2015              #                     #
################################################################################
CC = gcc
CFLAGS = -std=c99 -Wall -pedantic -O2
################################################################################
all: prvocisla prvocisla-inline steg-decode
################################################################################
error.o: error.c error.h
	$(CC) $(CFLAGS) -c $< -o $@ 
eratosthenes.o: eratosthenes.c bit-array.h error.h eratosthenes.h
	$(CC) $(CFLAGS) -c $< -o $@ -lm
	$(CC) $(CFLAGS) -c $< -DUSE_INLINE -o eratosthenes-inline.o -lm
ppm.o: ppm.c ppm.h
	$(CC) $(CFLAGS) -c $< -o $@
prvocisla.o: prvocisla.c bit-array.h error.h eratosthenes.h
	$(CC) $(CFLAGS) -c $< -o $@ 
	$(CC) $(CFLAGS) -c $< -DUSE_INLINE -o prvocisla-inline.o 
steg-decode.o: steg-decode.c ppm.h error.h bit-array.h eratosthenes.h
	$(CC) $(CFLAGS) -c $< -o $@
################################################################################
prvocisla: prvocisla.o eratosthenes.o error.o
	$(CC) $(CFLAGS) -o $@ $< eratosthenes.o error.o -lm

prvocisla-inline: prvocisla-inline.o eratosthenes-inline.o error.o
	$(CC) $(CFLAGS) -o $@ $< eratosthenes-inline.o error.o -lm

steg-decode: steg-decode.o ppm.o eratosthenes.o error.o
	$(CC) $(CFLAGS) -o $@ $< ppm.o eratosthenes.o error.o -lm
################################################################################
clean:
	rm *.o prvocisla prvocisla-inline steg-decode
################################################################################
zip:
	zip xbarna01.zip -u *.h *.c Makefile
################################################################################
################################################################################
