kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#noinst_LTLIBRARIES = librefactoring.la
#
#INCLUDES = -I$(top_srcdir) -I$(top_builddir)/umbrello/umbrello/dialogs $(all_includes)
#
#librefactoring_la_METASOURCES = AUTO
#
#librefactoring_la_SOURCES = refactoringassistant.cpp
#
