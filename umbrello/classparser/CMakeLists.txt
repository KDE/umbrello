kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I../dialogs $(all_includes)
#
#noinst_LTLIBRARIES = libclassparser.la
#libclassparser_la_SOURCES = ast.cpp driver.cpp errors.cpp lexer.cpp lookup.cpp parser.cpp tree_parser.cpp urlutil.cpp ast_utils.cpp cpptree2uml.cpp
#
