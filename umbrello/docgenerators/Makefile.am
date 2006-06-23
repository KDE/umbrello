INCLUDES = -I$(top_srcdir)/. -I$(top_srcdir)/umbrello \
	-I$(top_srcdir)/umbrello/docgenerators -I../../../umbrello/umbrello -I/usr/include/libxml2 $(all_includes)
METASOURCES = AUTO
bin_PROGRAMS = umbodoc

umbodoc_SOURCES = main.cpp
umbodoc_LDADD = $(LIB_KDECORE) -lxslt -lxml2
lib_LTLIBRARIES = libdocgenerators.la
noinst_HEADERS = docbookgenerator.h
libdocgenerators_la_SOURCES = docbookgenerator.h docbookgenerator.cpp
