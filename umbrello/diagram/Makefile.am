noinst_LTLIBRARIES = libdiagram.la

INCLUDES = -I$(top_srcdir) -I$(top_builddir)/umbrello/umbrello/dialogs $(all_includes)

libdiagram_la_METASOURCES = AUTO

libdiagram_la_SOURCES = diagram.cpp diagramview.cpp diagramelement.cpp umlwidget.cpp \
                        classwidget.cpp actorwidget.cpp associationwidget.cpp \
                        widgetfactory.cpp widgetset.cpp 
libdiagram_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

