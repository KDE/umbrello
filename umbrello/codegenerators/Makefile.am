lib_LTLIBRARIES = libcodegenerator.la

INCLUDES = $(all_includes)

libcodegenerator_la_LDFLAGS = $(all_libraries)
libcodegenerator_la_METASOURCES = AUTO
libcodegenerator_la_SOURCES = codegenfactory.cpp classifierinfo.cpp \
				javacodegenerator.cpp javaantcodedocument.cpp javacodeclassfield.cpp \
				javaclassifiercodedocument.cpp javacodecomment.cpp javacodedocumentation.cpp \
				javacodeoperation.cpp javasetaccessormethod.cpp javagetaccessormethod.cpp \
				javaclassdeclarationblock.cpp javacodeclassfielddeclarationblock.cpp \
				javalistaccessormethod.cpp javaremoveaccessormethod.cpp javaaddaccessormethod.cpp \
				javacodegenerationpolicy.cpp javacodegenerationpolicypage.cpp \
				javacodegenerationformbase.ui \
				cppcodegenerator.cpp cppcodedocumentation.cpp cppcodeclassfield.cpp \
				cppcodecomment.cpp cppmakecodedocument.cpp \
				cppsourcecodedocument.cpp cppsourceclassdeclarationblock.cpp\
				cppsourcecodeoperation.cpp cppsourcecodeaccessormethod.cpp \
				cppsourcecodeclassfielddeclarationblock.cpp \
				cppheadercodedocument.cpp cppheaderclassdeclarationblock.cpp \
				cppheadercodeoperation.cpp cppheadercodeaccessormethod.cpp \
				cppheadercodeclassfielddeclarationblock.cpp \
				cppcodegenerationpolicy.cpp cppcodegenerationpolicypage.cpp \
				cppcodegenerationformbase.ui \
				xmlschemacodegenerator.cpp xmlschemaclassifiercodedocument.cpp \
				xmlschemacodeclassfield.cpp \
				simplecodegenerator.cpp \
				perlwriter.cpp
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/umbrello/codegenerators/
	$(INSTALL_DATA) libcodegenerator.la $(DESTDIR)$(kde_datadir)/umbrello/codegenerators/libcodegenerator.la

uninstall-local:
	-rm -f $(DESTDIR)$(kde_datadir)/umbrello/codegenerators/libcodegenerator.la
