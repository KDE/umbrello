umbrellodir = $(kde_datadir)/umbrello/codegenerators
umbrello_LTLIBRARIES = libcodegenerator.la

INCLUDES = $(all_includes)
#LDFLAGS =                     -Iuml/classparser -Iuml/dialogs -Iuml/clipboard

libcodegenerator_la_LDFLAGS = $(KDE_PLUGIN)
libcodegenerator_la_METASOURCES = AUTO
libcodegenerator_la_SOURCES = phpwriter.cpp javawriter.cpp factory.cpp \
                              cppwriter.cpp 
