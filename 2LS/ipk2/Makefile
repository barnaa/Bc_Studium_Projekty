# Makefile pre projekt 2 z predmetu IPK
# Autor: Andrej Barna (xbarna01)
# Ak. rok: 2015/2016


CPPC = clang++
CPPPARS = -std=c++1y -O2

.PHONY: clean archive

all: client server

client: client.cc
	$(CPPC) $(CPPPARS) $< -o$@

server: server.cc
	$(CPPC) $(CPPPARS) $< -o$@

clean:
	rm client server
	rm -r ./clientFolder

archive:
	tar -cvzf xbarna01.tar.gz Makefile protokol.pdf README client.cc server.cc test.sh textfile.txt loremipsum.txt mapa.png foto.jpg trombone.wav badum_tss.wav
