bin_PROGRAMS = umbrello

umbrello_SOURCES = classimport.cpp umllistviewitemdata.cpp umllistviewitem.cpp umllistview.cpp seqlinewidget.cpp codegenerator.cpp activitywidget.cpp activitywidgetdata.cpp worktoolbar.cpp usecasewidgetdata.cpp usecasewidget.cpp usecase.cpp umlwidgetdata.cpp umlwidget.cpp umlviewdata.cpp umlview.cpp umlviewcanvas.cpp umlobject.cpp umlnamespace.cpp umldoc.cpp uml.cpp statewidgetdata.cpp statewidget.cpp operation.cpp objectwidgetdata.cpp objectwidget.cpp notewidgetdata.cpp notewidget.cpp messagewidgetdata.cpp messagewidget.cpp main.cpp listpopupmenu.cpp linepath.cpp kstartuplogo.cpp infowidget.cpp floatingtextdata.cpp floatingtext.cpp docwindow.cpp class.cpp classwidgetdata.cpp classwidget.cpp classifier.cpp attribute.cpp assocrules.cpp associationwidgetdata.cpp associationwidget.cpp association.cpp actorwidgetdata.cpp actorwidget.cpp actor.cpp template.cpp package.cpp packagewidget.cpp packagewidgetdata.cpp interface.cpp interfacewidget.cpp interfacewidgetdata.cpp component.cpp componentwidget.cpp componentwidgetdata.cpp umlcanvasobject.cpp artifact.cpp artifactwidget.cpp artifactwidgetdata.cpp boxwidget.cpp boxwidgetdata.cpp node.cpp nodewidget.cpp nodewidgetdata.cpp configurable.cpp pluginloader.cpp plugin.cpp 

umbrello_LDADD   = ./refactoring/librefactoring.la ./diagram/libdiagram.la ./classparser/libclassparser.la ./clipboard/libclipboard.la ./dialogs/libdialogs.la ./codegenerators/libcodegenerator.la -lfl $(LIB_KDEPRINT) $(LIB_KIO)

SUBDIRS = classparser clipboard dialogs pics codegenerators headings diagram refactoring plugins

KDE_ICON=AUTO

appdir=$(kde_datadir)/umbrello
app_DATA = tips umbrelloui.rc

xdg_apps_DATA = umbrello.desktop

mimedir = $(kde_mimedir)/application
mime_DATA = x-umbrello.desktop

INCLUDES= -Idialogs -Irefactoring $(all_includes)

METASOURCES = AUTO

umbrello_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

messages: rc.cpp
	$(PREPARETIPS) > tips.cpp
	$(EXTRACTRC) *.rc dialogs/*.ui > ./rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/umbrello.pot; \
	fi
	rm -f tips.cpp

version.h: $(top_srcdir)/umbrello/VERSION
	printf "#undef VERSION\n#define VERSION \"`cat $(top_srcdir)/umbrello/VERSION`\"\n" > version.h

noinst_HEADERS = version.h

main.o: version.h


