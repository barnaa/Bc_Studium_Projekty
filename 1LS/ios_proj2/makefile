# Makefile k IOS - Projekt 2
# Autor: Andrej Barna (xbarna01)
# Ak. rok: 2014/2015

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

.PHONY: all clean zip

all: h2o

h2o: h2o.o
	$(CC) $(CFLAGS) -pthread -o $@ $<

h2o.o: h2o.c
	$(CC) $(CFLAGS) -c -o $@ $<

###############################################################################

clean:
	rm *.o h2o

zip:
	zip xbarna01.zip -u *.c *.h makefile
