kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_builddir)/umbrello/umbrello/dialogs $(all_includes)
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libclipboard.la
#libclipboard_la_SOURCES = umldrag.cpp umlclipboard.cpp idchangelog.cpp 
