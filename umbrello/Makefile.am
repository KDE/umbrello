bin_PROGRAMS = umbrello

umbrello_SOURCES = classimport.cpp umllistviewitemdata.cpp umllistviewitem.cpp umllistview.cpp seqlinewidget.cpp codegenerator.cpp activitywidget.cpp activitywidgetdata.cpp worktoolbar.cpp usecasewidgetdata.cpp usecasewidget.cpp usecase.cpp umlwidgetdata.cpp umlwidget.cpp umlviewdata.cpp umlview.cpp umlviewcanvas.cpp umlobject.cpp umlnamespace.cpp umldoc.cpp uml.cpp statewidgetdata.cpp statewidget.cpp operation.cpp objectwidgetdata.cpp objectwidget.cpp notewidgetdata.cpp notewidget.cpp messagewidgetdata.cpp messagewidget.cpp main.cpp listpopupmenu.cpp linepath.cpp kstartuplogo.cpp infowidget.cpp floatingtextdata.cpp floatingtext.cpp docwindow.cpp conceptwidgetdata.cpp conceptwidget.cpp concept.cpp attribute.cpp assocrules.cpp associationwidgetdata.cpp associationwidget.cpp association.cpp actorwidgetdata.cpp actorwidget.cpp actor.cpp template.cpp package.cpp packagewidget.cpp packagewidgetdata.cpp interface.cpp interfacewidget.cpp interfacewidgetdata.cpp component.cpp componentwidget.cpp componentwidgetdata.cpp umlcanvasobject.cpp
umbrello_LDADD   = ./diagram/libdiagram.la ./classparser/libclassparser.la ./clipboard/libclipboard.la ./dialogs/libdialogs.la ./codegenerators/libcodegenerator.la -lfl $(LIB_KDEPRINT) $(LIB_KIO)

SUBDIRS = classparser clipboard dialogs pics codegenerators headings diagram

EXTRA_DIST = actor.h actorwidgetdata.h actorwidget.h associationwidgetdata.h associationwidgetdatalist.h associationwidget.h association.h assocrules.h attribute.h concept.h conceptwidgetdata.h conceptwidget.h docwindow.h floatingtextdata.h floatingtext.h infowidget.h kstartuplogo.h linepath.h listpopupmenu.h messagewidgetdata.h messagewidget.h actor.cpp actorwidget.cpp actorwidgetdata.cpp associationwidget.cpp associationwidgetdata.cpp association.cpp assocrules.cpp attribute.cpp concept.cpp conceptwidget.cpp conceptwidgetdata.cpp docwindow.cpp floatingtext.cpp floatingtextdata.cpp infowidget.cpp kstartuplogo.cpp linepath.cpp listpopupmenu.cpp main.cpp messagewidget.cpp messagewidgetdata.cpp notewidget.cpp notewidgetdata.cpp notewidgetdata.h notewidget.h objectwidget.cpp objectwidgetdata.cpp objectwidgetdata.h objectwidget.h operation.cpp operation.h statewidget.cpp statewidgetdata.cpp statewidgetdata.h statewidget.h uml.cpp umldoc.cpp umldoc.h uml.h umlnamespace.cpp umlnamespace.h umlobject.cpp umlobject.h umlobjectlist.h umlviewcanvas.cpp umlviewcanvas.h umlview.cpp umlviewdata.cpp umlviewdata.h umlview.h umlviewlist.h umlwidget.cpp umlwidgetdata.cpp umlwidgetdata.h umlwidget.h umlwidgetlist.h usecase.cpp usecase.h usecasewidget.cpp usecasewidgetdata.cpp usecasewidgetdata.h usecasewidget.h worktoolbar.cpp worktoolbar.h hi16-app-uml.png hi32-app-uml.png lo16-app-uml.png lo32-app-uml.png tips uml.desktop umlui.rc x-uml.desktop activitywidgetdata.cpp activitywidgetdata.h activitywidget.cpp activitywidget.h codegenerator.cpp codegenerator.h seqlinewidget.cpp seqlinewidget.h umllistview.cpp umllistview.h umllistviewitem.cpp umllistviewitem.h umllistviewitemdata.cpp umllistviewitemdata.h umllistviewitemdatalist.h classimport.cpp classimport.h umllistviewitemlist.h template.cpp template.h package.cpp package.h packagewidget.cpp packagewidget.h packagewidgetdata.cpp packagewidgetdata.h interface.cpp interface.h interfacewidget.cpp interfacewidgetdata.cpp component.cpp component.h componentwidget.cpp componentwidgetdata.cpp umlcanvasobject.cpp umlcanvasobject.h

KDE_ICON=AUTO

appdir=$(kde_datadir)/umbrello
app_DATA = tips umbrelloui.rc

xdg_apps_DATA = umbrello.desktop

mimedir = $(kde_mimedir)/application
mime_DATA = x-umbrello.desktop

INCLUDES= -Idialogs $(all_includes)

METASOURCES = AUTO

umbrello_LDFLAGS = $(all_libraries) $(KDE_RPATH)

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


