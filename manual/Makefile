all: en pdf-it pdf-fr
en: web pdf
it: web-it pdf-it
fr: web-fr pdf-fr

build_folder = ./build

# we can't name the target "html", since there is already a folder with that name
# and so make thinks this target is already up to date
web:
	mkdir --parents build/web-en
	docbook2html --dsl qtiplot.dsl docbook-en/index.docbook  --error no-valid --output build/web-en
	cp html/qtiplot.css $(build_folder)/web-en/
	cp -r html/common $(build_folder)/web-en/
	cp -r html/icons $(build_folder)/web-en/
	cp -r html/equations $(build_folder)/web-en/
	cp -r html/pics $(build_folder)/web-en/
	cp -r html/tutorial $(build_folder)/web-en/
web-it:
	mkdir --parents build/web-it
	docbook2html --dsl qtiplot.dsl docbook-it/index.docbook --error no-valid --output build/web-it
	cp html/qtiplot.css $(build_folder)/web-it/
	cp -r html/common $(build_folder)/web-it/
	cp -r html/icons $(build_folder)/web-it/
	cp -r html/equations $(build_folder)/web-it/
	cp -r html/pics-it $(build_folder)/web-it/
	cp -r html/tutorial $(build_folder)/web-it/
	cp -r html/pics-it $(build_folder)/web-it/
web-fr:
#	export SP_ENCODING=XML
#	export SP_CHARSET_FIXED=YES
	mkdir --parents $(build_folder)/web-fr
	docbook2html --dsl qtiplot.dsl docbook-fr/index.docbook  --error no-valid --output $(build_folder)/web-fr
	cp html/qtiplot.css $(build_folder)/web-fr/
	cp -r html/common $(build_folder)/web-fr/
	cp -r html/icons $(build_folder)/web-fr/
	cp -r html/equations $(build_folder)/web-fr/
	cp -r html/pics $(build_folder)/web-fr/
	cp -r html/tutorial $(build_folder)/web-fr/

pdf:
	dblatex --output=$(build_folder)/qtiplot-manual-en.pdf --config=dblatex.conf docbook-en/index.docbook
pdf-it:
	dblatex --output=$(build_folder)/qtiplot-manual-it.pdf --config=dblatex.conf docbook-it/index.docbook
pdf-fr:
	dblatex --output=$(build_folder)/qtiplot-manual-fr.pdf --config=dblatex.conf docbook-fr/index.docbook

verify:
	xmllint -noout --valid docbook-en/index.docbook

clean:
	rm -r $(build_folder)