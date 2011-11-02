all: en pdf-it
en: web pdf
it: web-it pdf-it

# we can't name the target "html", since there is already a folder with that name
# and so make thinks this target is already up to date
web:
	docbook2html -d qtiplot.dsl docbook-en/index.docbook -e no-valid
web-it:
	docbook2html -d qtiplot.dsl docbook-it/index.docbook -e no-valid

pdf:
	dblatex -o qtiplot-manual-en.pdf -c dblatex.conf docbook-en/index.docbook
pdf-it:
	dblatex -o qtiplot-manual-it.pdf -c dblatex.conf docbook-it/index.docbook
