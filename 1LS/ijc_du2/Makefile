################################################################################
#                       # Projekt: DU2 - IJC             #                     #
#                       # Autor: Andrej Barna (xbarna01) #                     #
#                       # Datum: 26.04.2015              #                     #
################################################################################

CC = gcc
CPPC = g++
CFLAGS = -std=c99 -pedantic -Wall -O2
CFLAGSPIC = -std=c99 -pedantic
CPPFLAGS = -std=c++11 -pedantic -Wall -O2
LIBFILES = hash_function.o htab_clear.o htab_foreach.o htab_free.o htab_init.o htab_lookup.o htab_remove.o htab_statistics.o htable.h

.PHONY: all clean zip

all: tail tail2 libhtable.a libhtable.so wordcount wordcount-dynamic

################################################################################
# Binaries
################################################################################

tail: tail.c
	$(CC) $(CFLAGS) -o $@ $<

tail2: tail2.cc
	$(CPPC) $(CPPFLAGS) -o $@ $<

wordcount: wordcount.o libhtable.a io.o
	$(CC) -static $(CFLAGS) -o $@ io.o $< -L. -lhtable

wordcount-dynamic: wordcount.o libhtable.so io.o
	$(CC) $(CFLAGS) -o $@ io.o $< -L. -lhtable

################################################################################
# Libraries
################################################################################

libhtable.a: $(LIBFILES)
	ar cr $@ $^
	ranlib $@

libhtable.so: $(LIBFILES)
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $^

################################################################################
# Objects
################################################################################

hash_function.o: hash_function.c htable.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
htab_clear.o: htab_clear.c htable.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
htab_foreach.o: htab_foreach.c htable.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
htab_free.o: htab_free.c htable.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
htab_init.o: htab_init.c htable.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
htab_lookup.o: htab_lookup.c htable.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
htab_remove.o: htab_remove.c htable.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
htab_statistics.o: htab_statistics.c htable.h
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

io.o: io.c io.h
	$(CC) $(CFLAGS) -c $< -o $@

wordcount.o: wordcount.c htable.h io.h
	$(CC) $(CFLAGS) -c $< -o $@

################################################################################

clean:
	rm *.o libhtable.a libhtable.so tail tail2 wordcount wordcount-dynamic
  
zip:
	zip xbarna01.zip -u *.h *.c *.cc Makefile
