INCLUDES = -I$(top_srcdir)/. -I$(top_srcdir)/umbrello -I$(top_srcdir)/umbrello/docgenerators -I../../../umbrello/umbrello -I/usr/include/libxml2 $(all_includes)
METASOURCES = AUTO
bin_PROGRAMS = umbodoc

umbodoc_COMPILE_FIRST = version.h
umbodoc_SOURCES = main.cpp
umbodoc_LDADD = $(LIB_KDECORE) -lxslt -lxml2
lib_LTLIBRARIES = libdocgenerators.la
noinst_HEADERS = docbookgenerator.h version.h
libdocgenerators_la_SOURCES = docbookgenerator.h docbookgenerator.cpp xhtmlgenerator.cpp

appdir=$(kde_datadir)/umbrello
app_DATA = xmi2docbook.xsl docbook2xhtml.xsl xmi.css common.ent

version.h: $(top_srcdir)/umbrello/VERSION
	printf "#undef UMBRELLO_VERSION\n#define UMBRELLO_VERSION \"`cat $(top_srcdir)/umbrello/VERSION`\"\n" > version.h

CLEANFILES = version.h

