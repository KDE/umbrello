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
	rubycodegenerator.cpp rubycodeclassfield.cpp \
	rubyclassifiercodedocument.cpp rubycodecomment.cpp rubycodedocumentation.cpp \
	rubycodeoperation.cpp rubycodeaccessormethod.cpp \
	rubyclassdeclarationblock.cpp rubycodeclassfielddeclarationblock.cpp \
	rubycodegenerationpolicy.cpp rubycodegenerationpolicypage.cpp \
	rubycodegenerationformbase.ui \
	simplecodegenerator.cpp \
	adawriter.cpp aswriter.cpp cppwriter.cpp csharpwriter.cpp dwriter.cpp javawriter.cpp jswriter.cpp \
	idlwriter.cpp pascalwriter.cpp perlwriter.cpp php5writer.cpp phpwriter.cpp \
	pythonwriter.cpp rubywriter.cpp sqlwriter.cpp tclwriter.cpp xmlschemawriter.cpp \
	xmlelementcodeblock.cpp xmlcodecomment.cpp \
	codegen_utils.cpp
libcodegenerator_la_COMPILE_FIRST = ../dialogs/codegenerationpolicybase.h

KDE_OPTIONS = nofinal

