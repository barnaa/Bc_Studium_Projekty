# Makefile pre projekt z predmetu ISA
# Autor: Andrej Barna (xbarna01)
# Ak. rok: 2016/2017, zimny semester

CPPC = gcc
CPPPARS = --std=c++11 -lstdc++ -lncurses -lpcap

.PHONY: clean archive

dhcp-stats: dhcp-stats.cc
	$(CPPC) $(CPPPARS) $< -o $@

clean:
	rm dhcp-stats

archive:
	tar -cvf xbarna01.tar Makefile README dhcp-stats.1 dhcp-stats.cc
