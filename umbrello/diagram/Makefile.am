noinst_LTLIBRARIES = libdiagram.la

INCLUDES = -I../dialogs -I$(srcdir)/dialogs $(all_includes)

libdiagram_la_METASOURCES = AUTO

libdiagram_la_SOURCES = diagram.cpp diagramview.cpp diagramelement.cpp umlwidget.cpp \
                        classwidget.cpp actorwidget.cpp associationwidget.cpp \
                        widgetfactory.cpp widgetset.cpp 
libdiagram_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

