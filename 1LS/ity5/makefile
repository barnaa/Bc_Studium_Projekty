proj5:
	latex $@.tex
	latex $@.tex
	dvips -t a4 $@.dvi
	ps2pdf -sPAPERSIZE=a4 $@.ps

clean:
	rm *.log *.dvi *.ps *.aux *.pdf *.out

zip:
	zip xbarna01-fit.zip -u *.eps *.tex makefile

