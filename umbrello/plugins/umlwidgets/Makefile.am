
SUBDIRS = dialogs

INCLUDES = -I. $(all_includes)
LDFLAGS = $(all_libraries)
LDADD = $(LIB_KDECORE)

kde_module_LTLIBRARIES = libumlwidgets.la
libumlwidgets_la_SOURCES = umlwidgetset.cpp umlwidget.cpp classwidget.cpp classtool.cpp \
                           interfacewidget.cpp interfacetool.cpp packagewidget.cpp packagetool.cpp \
                           umlassociationwidget.cpp umlassociationtool.cpp
libumlwidgets_la_METASOURCES = AUTO
libumlwidgets_la_LIBADD = ./dialogs/libdialogs.la $(LIB_KDEUI) $(LIB_QT)
libumlwidgets_la_LDFLAGS = -module
