inst_LIBRARIES = libtamuanova.a
pkginclude_HEADERS = tamu_anova.h
libtamuanova_a_SOURCES = anova_1.c anova_2.c
instdir = /usr/lib/

TESTS = $(check_PROGRAMS)
check_PROGRAMS = test
test_LDADD = libtamuanova.a -lgsl -lgslcblas -lm
test_SOURCES = test.c

CLEANFILES = test.dat

info_TEXINFOS = tamu_anova.texi 
tamu_anova_TEXINFOS = fdl.texi mathinclude.texi

EXTRA_DIST = autogen.sh
