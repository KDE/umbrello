####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = umbrello

## INCLUDES were found outside kdevelop specific part

umbrello_SOURCES = classimport.cpp umllistviewitemdata.cpp umllistviewitem.cpp umllistview.cpp seqlinewidget.cpp codegenerator.cpp activitywidget.cpp activitywidgetdata.cpp worktoolbar.cpp usecasewidgetdata.cpp usecasewidget.cpp usecase.cpp umlwidgetdata.cpp umlwidget.cpp umlviewdata.cpp umlview.cpp umlviewcanvas.cpp umlobject.cpp umlnamespace.cpp umldoc.cpp uml.cpp statewidgetdata.cpp statewidget.cpp operation.cpp objectwidgetdata.cpp objectwidget.cpp notewidgetdata.cpp notewidget.cpp messagewidgetdata.cpp messagewidget.cpp main.cpp listpopupmenu.cpp linepath.cpp kstartuplogo.cpp infowidget.cpp floatingtextdata.cpp floatingtext.cpp docwindow.cpp conceptwidgetdata.cpp conceptwidget.cpp concept.cpp attribute.cpp assocrules.cpp associationwidgetdata.cpp associationwidget.cpp actorwidgetdata.cpp actorwidget.cpp actor.cpp 
umbrello_LDADD   = ./classparser/libclassparser.a ./clipboard/libclipboard.a ./dialogs/libdialogs.a ./codegenerators/libcodegenerator.la -lfl $(LIB_KDEPRINT) $(LIB_KIO) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = classparser clipboard dialogs pics codegenerators headings 

EXTRA_DIST = actor.h actorwidgetdata.h actorwidget.h associationwidgetdata.h associationwidgetdatalist.h associationwidget.h assocrules.h attribute.h concept.h conceptwidgetdata.h conceptwidget.h docwindow.h floatingtextdata.h floatingtext.h infowidget.h kstartuplogo.h linepath.h listpopupmenu.h messagewidgetdata.h messagewidget.h actor.cpp actorwidget.cpp actorwidgetdata.cpp associationwidget.cpp associationwidgetdata.cpp assocrules.cpp attribute.cpp concept.cpp conceptwidget.cpp conceptwidgetdata.cpp docwindow.cpp floatingtext.cpp floatingtextdata.cpp infowidget.cpp kstartuplogo.cpp linepath.cpp listpopupmenu.cpp main.cpp messagewidget.cpp messagewidgetdata.cpp notewidget.cpp notewidgetdata.cpp notewidgetdata.h notewidget.h objectwidget.cpp objectwidgetdata.cpp objectwidgetdata.h objectwidget.h operation.cpp operation.h statewidget.cpp statewidgetdata.cpp statewidgetdata.h statewidget.h uml.cpp umldoc.cpp umldoc.h uml.h umlnamespace.cpp umlnamespace.h umlobject.cpp umlobject.h umlobjectlist.h umlviewcanvas.cpp umlviewcanvas.h umlview.cpp umlviewdata.cpp umlviewdata.h umlview.h umlviewlist.h umlwidget.cpp umlwidgetdata.cpp umlwidgetdata.h umlwidget.h umlwidgetlist.h usecase.cpp usecase.h usecasewidget.cpp usecasewidgetdata.cpp usecasewidgetdata.h usecasewidget.h worktoolbar.cpp worktoolbar.h hi16-app-uml.png hi32-app-uml.png lo16-app-uml.png lo32-app-uml.png tips uml.desktop umlui.rc x-uml.desktop activitywidgetdata.cpp activitywidgetdata.h activitywidget.cpp activitywidget.h codegenerator.cpp codegenerator.h seqlinewidget.cpp seqlinewidget.h umllistview.cpp umllistview.h umllistviewitem.cpp umllistviewitem.h umllistviewitemdata.cpp umllistviewitemdata.h umllistviewitemdatalist.h classimport.cpp classimport.h umllistviewitemlist.h 

install-data-local:
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/hi16-app-uml.png $(kde_icondir)/hicolor/16x16/apps/umbrello.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/hi32-app-uml.png $(kde_icondir)/hicolor/32x32/apps/umbrello.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-uml.png $(kde_icondir)/locolor/16x16/apps/umbrello.png
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-uml.png $(kde_icondir)/locolor/32x32/apps/umbrello.png
	$(mkinstalldirs) $(kde_datadir)/umbrello/
	$(INSTALL_DATA) $(srcdir)/tips $(kde_datadir)/umbrello/tips
	$(mkinstalldirs) $(kde_appsdir)/Development/
	$(INSTALL_DATA) $(srcdir)/uml.desktop $(kde_appsdir)/Development/umbrello.desktop
	$(mkinstalldirs) $(kde_datadir)/umbrello/
	$(INSTALL_DATA) $(srcdir)/umlui.rc $(kde_datadir)/umbrello/umbrelloui.rc
	$(mkinstalldirs) $(kde_mimedir)/application/
	$(INSTALL_DATA) $(srcdir)/x-uml.desktop $(kde_mimedir)/application/x-uml.desktop

uninstall-local:
	-rm -f $(kde_icondir)/hicolor/16x16/apps/umbrello.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/umbrello.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/umbrello.png
	-rm -f $(kde_icondir)/locolor/32x32/apps/umbrello.png
	-rm -f $(kde_datadir)/umbrello/tips
	-rm -f $(kde_appsdir)/Development/umbrello.desktop
	-rm -f $(kde_datadir)/umbrello/umbrelloui.rc
	-rm -f $(kde_mimedir)/application/x-uml.desktop

####### kdevelop will overwrite this part!!! (end)############
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I./dialogs

METASOURCES = AUTO

# the library search path. 
umbrello_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/umbrello
rc_DATA = umlui.rc

messages: rc.cpp
	$(PREPARETIPS) > tips.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/umbrello.pot; \
	fi
	rm -f tips.cpp
