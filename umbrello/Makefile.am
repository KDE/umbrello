bin_PROGRAMS = umbrello

umbrello_SOURCES = activitywidget.cpp actor.cpp actorwidget.cpp \
	artifact.cpp artifactwidget.cpp \
	association.cpp associationwidget.cpp assocrules.cpp \
	attribute.cpp boxwidget.cpp class.cpp classifier.cpp \
	classifiercodedocument.cpp classifierlistitem.cpp classimport.cpp \
	classwidget.cpp codeaccessormethod.cpp codeblock.cpp \
	codeblockwithcomments.cpp codeclassfield.cpp \
	codeclassfielddeclarationblock.cpp codecomment.cpp \
	codedocument.cpp codegenerationpolicy.cpp codegenerator.cpp \
	codegenobjectwithtextblocks.cpp codemethodblock.cpp \
	codeoperation.cpp codeparameter.cpp component.cpp \
	componentwidget.cpp configurable.cpp \
	docwindow.cpp floatingtext.cpp hierarchicalcodeblock.cpp \
	infowidget.cpp interface.cpp interfacewidget.cpp \
	kstartuplogo.cpp linepath.cpp listpopupmenu.cpp \
	main.cpp messagewidget.cpp \
	node.cpp nodewidget.cpp notewidget.cpp \
	objectwidget.cpp operation.cpp ownedcodeblock.cpp \
	ownedhierarchicalcodeblock.cpp \
	package.cpp packagewidget.cpp plugin.cpp pluginloader.cpp \
	seqlinewidget.cpp statewidget.cpp stereotype.cpp \
	template.cpp textblock.cpp \
	uml.cpp umlcanvasobject.cpp umldoc.cpp umllistview.cpp \
	umllistviewitem.cpp umlnamespace.cpp umlobject.cpp \
	umlrole.cpp umlview.cpp umlviewcanvas.cpp umlwidget.cpp \
	usecase.cpp usecasewidget.cpp worktoolbar.cpp

umbrello_LDADD   = ./refactoring/librefactoring.la ./diagram/libdiagram.la ./classparser/libclassparser.la ./clipboard/libclipboard.la ./dialogs/libdialogs.la ./codegenerators/libcodegenerator.la -lfl $(LIB_KDEPRINT) $(LIB_KIO)

SUBDIRS = classparser clipboard dialogs pics codegenerators headings diagram refactoring plugins

EXTRA_DIST = actor.h actorwidget.h associationwidget.h association.h assocrules.h attribute.h classifier.h classifiercodedocument.h class.h codeclassfield.h codeclassfielddeclarationblock.h codeparameter.h codeaccessormethod.h codemethodblock.h classwidget.h docwindow.h floatingtext.h infowidget.h kstartuplogo.h linepath.h listpopupmenu.h messagewidgetlist.h messagewidget.h actor.cpp actorwidget.cpp associationwidget.cpp association.cpp assocrules.cpp attribute.cpp classifier.cpp classifiercodedocument.cpp class.cpp codeclassfield.cpp codeclassfielddeclarationblock.cpp codeaccessormethod.cpp codemethodblock.cpp codeparameter.cpp classwidget.cpp docwindow.cpp floatingtext.cpp infowidget.cpp kstartuplogo.cpp linepath.cpp listpopupmenu.cpp main.cpp messagewidget.cpp notewidget.cpp notewidget.h objectwidget.cpp objectwidget.h operation.cpp operation.h statewidget.cpp statewidget.h uml.cpp umldoc.cpp umldoc.h uml.h umlnamespace.cpp umlnamespace.h umlobject.cpp umlobject.h umlobjectlist.h umlviewcanvas.cpp umlviewcanvas.h umlview.cpp umlview.h umlviewlist.h umlwidget.cpp umlwidget.h umlwidgetlist.h usecase.cpp usecase.h usecasewidget.cpp usecasewidget.h worktoolbar.cpp worktoolbar.h hi16-app-umbrello.png hi32-app-umbrello.png lo16-app-umbrello.png lo32-app-umbrello.png tips uml.desktop umlui.rc x-uml.desktop activitywidget.cpp activitywidget.h codegenerator.cpp codegenerator.h seqlinewidget.cpp seqlinewidget.h umllistview.cpp umllistview.h umllistviewitem.cpp umllistviewitem.h classimport.cpp classimport.h umllistviewitemlist.h template.cpp template.h package.cpp package.h packagewidget.cpp packagewidget.h codecomment.cpp codecomment.h codeoperation.cpp codeoperation.h codeblock.cpp codeblock.h codeblockwithcomments.cpp codeblockwithcomments.h hierarchicalcodeblock.cpp hierarchicalcodeblock.h ownedhierarchicalcodeblock.cpp ownedhierarchicalcodeblock.h codedocument.cpp codedocument.h codegenerationpolicy.cpp codegenerationpolicy.h textblock.cpp textblock.h interface.cpp interface.h interfacewidget.cpp component.cpp component.h componentwidget.cpp umlcanvasobject.cpp umlcanvasobject.h artifact.cpp artifact.h artifactwidget.cpp artifactwidget.h boxwidget.cpp boxwidget.h node.cpp node.h nodewidget.cpp nodewidget.h umlrole.cpp umlrole.h ownedcodeblock.cpp ownedcodeblock.h codegenobjectwithtextblocks.cpp codegenobjectwithtextblocks.h inputdialog.h

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


