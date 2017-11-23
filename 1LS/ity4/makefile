proj4:
	latex $@.tex
	bibtex $@
	latex $@.tex
	latex $@.tex
	dvips $@.dvi
	ps2pdf -sPAPERSIZE=a4 $@.ps

clean:
	rm *.log *.dvi *.ps *.aux *.pdf *.bbl *.blg
