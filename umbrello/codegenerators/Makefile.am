lib_LTLIBRARIES = libcodegenerator.la

INCLUDES = $(all_includes)

libcodegenerator_la_LDFLAGS = $(all_libraries)
libcodegenerator_la_METASOURCES = AUTO
libcodegenerator_la_SOURCES = phpwriter.cpp javawriter.cpp factory.cpp classifierinfo.cpp \
                              cppwriter.cpp adawriter.cpp perlwriter.cpp \
                              pythonwriter.cpp idlwriter.cpp sqlwriter.cpp \
                              aswriter.cpp jswriter.cpp xmlschemawriter.cpp

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/umbrello/codegenerators/
	$(INSTALL_DATA) libcodegenerator.la $(DESTDIR)$(kde_datadir)/umbrello/codegenerators/libcodegenerator.la

uninstall-local:
	-rm -f $(DESTDIR)$(kde_datadir)/umbrello/codegenerators/libcodegenerator.la
