INCLUDES = -I$(srcdir)/.. -I$(top_srcdir)/umbrello/docgenerators -I../../../umbrello/umbrello $(LIBXML_CFLAGS) $(all_includes)
METASOURCES = AUTO
bin_PROGRAMS = umbodoc

AM_LDFLAGS = $(all_libraries)
umbodoc_COMPILE_FIRST = version.h
umbodoc_SOURCES = main.cpp
umbodoc_LDADD = $(LIB_KDECORE) $(LIBXSLT_LIBS) $(LIBXML_LIBS)
noinst_LTLIBRARIES = libdocgenerators.la
noinst_HEADERS = docbookgenerator.h version.h
libdocgenerators_la_SOURCES = docbookgenerator.h docbookgenerator.cpp xhtmlgenerator.cpp
libdocgenerators_la_LIBADD = $(LIB_KDECORE) $(LIBXSLT_LIBS) $(LIBXML_LIBS)

appdir=$(kde_datadir)/umbrello
app_DATA = xmi2docbook.xsl docbook2xhtml.xsl xmi.css common.ent

version.h: $(top_srcdir)/umbrello/VERSION
	printf "#undef UMBRELLO_VERSION\n#define UMBRELLO_VERSION \"`cat $(top_srcdir)/umbrello/VERSION`\"\n" > version.h

CLEANFILES = version.h

