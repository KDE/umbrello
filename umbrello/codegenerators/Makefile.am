lib_LTLIBRARIES = libcodegenerator.la

INCLUDES = $(all_includes)

libcodegenerator_la_LDFLAGS = $(all_libraries)
libcodegenerator_la_METASOURCES = AUTO
libcodegenerator_la_SOURCES = phpwriter.cpp javawriter.cpp factory.cpp \
                              cppwriter.cpp adawriter.cpp perlwriter.cpp \
                              pythonwriter.cpp idlwriter.cpp

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/umbrello/codegenerators/
	$(INSTALL_DATA) libcodegenerator.la $(kde_datadir)/umbrello/codegenerators/libcodegenerator.la

uninstall-local:
	-rm -f $(kde_datadir)/umbrello/codegenerators/libcodegenerator.la
