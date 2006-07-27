

########### next target ###############


########### install files ###############
install( FILES xmi2docbook.xsl docbook2xhtml.xsl xmi.css common.ent  DESTINATION  ${DATA_INSTALL_DIR}/umbrello )





#original Makefile.am contents follow:

# INCLUDES = -I$(top_srcdir)/. -I$(top_srcdir)/umbrello \
#     -I$(top_srcdir)/umbrello/docgenerators -I../../../umbrello/umbrello -I/usr/include/libxml2 $(all_includes)
# METASOURCES = AUTO
# bin_PROGRAMS = umbodoc
# 
# umbodoc_SOURCES = main.cpp
# umbodoc_LDADD = $(LIB_KDECORE) -lxslt -lxml2
# lib_LTLIBRARIES = libdocgenerators.la
# noinst_HEADERS = docbookgenerator.h
# libdocgenerators_la_SOURCES = docbookgenerator.h docbookgenerator.cpp \
#     xhtmlgenerator.cpp
# 
# appdir=$(kde_datadir)/umbrello
# app_DATA = xmi2docbook.xsl docbook2xhtml.xsl xmi.css common.ent
# 
