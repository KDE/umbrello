# KDE_OPTIONS = nofinal

SUBDIRS = dialogs

INCLUDES = -I../../dialogs -Idialogs $(all_includes)

kde_module_LTLIBRARIES = libumlwidgets.la
libumlwidgets_la_SOURCES = umlwidgetset.cpp umlwidget.cpp classwidget.cpp classtool.cpp \
                           interfacewidget.cpp interfacetool.cpp packagewidget.cpp packagetool.cpp \
                           umlassociationwidget.cpp umlassociationtool.cpp
libumlwidgets_la_METASOURCES = AUTO
libumlwidgets_la_LIBADD = ./dialogs/libdialogs.la $(LIB_KDEUI)
libumlwidgets_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)
