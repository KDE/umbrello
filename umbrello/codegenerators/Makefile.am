noinst_LTLIBRARIES = libcodegenerator.la

INCLUDES = -I../dialogs -I$(top_srcdir)/umbrello/umbrello/dialogs $(all_includes)

libcodegenerator_la_LDFLAGS = $(all_libraries)
libcodegenerator_la_METASOURCES = AUTO
libcodegenerator_la_SOURCES = codegenfactory.cpp classifierinfo.cpp \
				javacodegenerator.cpp javaantcodedocument.cpp javacodeclassfield.cpp \
				javaclassifiercodedocument.cpp javacodecomment.cpp javacodedocumentation.cpp \
				javacodeoperation.cpp javacodeaccessormethod.cpp \
				javaclassdeclarationblock.cpp javacodeclassfielddeclarationblock.cpp \
				javacodegenerationpolicy.cpp javacodegenerationpolicypage.cpp \
				javacodegenerationformbase.ui \
				cppcodegenerator.cpp cppcodedocumentation.cpp cppcodeclassfield.cpp \
				cppcodecomment.cpp cppmakecodedocument.cpp \
				cppsourcecodedocument.cpp cppsourcecodeoperation.cpp cppsourcecodeaccessormethod.cpp \
				cppsourcecodeclassfielddeclarationblock.cpp \
				cppheadercodedocument.cpp cppheaderclassdeclarationblock.cpp \
				cppheadercodeoperation.cpp cppheadercodeaccessormethod.cpp \
				cppheadercodeclassfielddeclarationblock.cpp \
				cppcodegenerationpolicy.cpp cppcodegenerationpolicypage.cpp \
				cppcodegenerationformbase.ui cppcodegenerationform.cpp \
				simplecodegenerator.cpp \
				adawriter.cpp aswriter.cpp jswriter.cpp idlwriter.cpp perlwriter.cpp \
				phpwriter.cpp pythonwriter.cpp sqlwriter.cpp xmlschemawriter.cpp \
				xmlelementcodeblock.cpp xmlcodecomment.cpp
libcodegenerator_la_COMPILE_FIRST = ../dialogs/codegenerationpolicybase.h
