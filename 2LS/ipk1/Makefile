CPPC = clang++
CPPPARS = -std=c++1y -O2
#-Wall -pedantic -O2

.PHONY: clean zip

webclient: webclient.cc
	$(CPPC) $(CPPPARS) $< -o$@

clean:
	rm webclient

zip:
	tar -cvzf xbarna01.tar.gz webclient.cc Makefile
