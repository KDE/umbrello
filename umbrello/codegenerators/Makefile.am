####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libcodegenerator.la


INCLUDES = $(all_includes)


LDFLAGS =                     -Iuml/classparser -Iuml/dialogs -Iuml/clipboard


libcodegenerator_la_LDFLAGS = -module

libcodegenerator_la_METASOURCES = AUTO

libcodegenerator_la_SOURCES = phpwriter.cpp javawriter.cpp factory.cpp cppwriter.cpp 


EXTRA_DIST = cppwriter.cpp cppwriter.h factory.cpp javawriter.cpp javawriter.h phpwriter.cpp phpwriter.h 

####### kdevelop will overwrite this part!!! (end)############

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/umbrello/codegenerators/
	$(INSTALL_DATA) $(srcdir)/libcodegenerator.la $(kde_datadir)/umbrello/codegenerators/libcodegenerator.la

uninstall-local:
	-rm -f $(kde_datadir)/umbrello/codegenerators/libcodegenerator.la

