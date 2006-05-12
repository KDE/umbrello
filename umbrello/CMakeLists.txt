
add_subdirectory( classparser ) 
add_subdirectory( dialogs ) 
add_subdirectory( clipboard ) 
add_subdirectory( pics ) 
add_subdirectory( codegenerators ) 
add_subdirectory( headings ) 
add_subdirectory( refactoring ) 




########### next target ###############

set(umbrello_SRCS 
   activitywidget.cpp 
   actor.cpp 
   actorwidget.cpp 
   adaimport.cpp 
   aligntoolbar.cpp 
   artifact.cpp 
   artifactwidget.cpp 
   association.cpp 
   associationwidget.cpp 
   assocrules.cpp 
   attribute.cpp 
   boxwidget.cpp 
   classifier.cpp 
   classifiercodedocument.cpp 
   classifierlistitem.cpp 
   classifierwidget.cpp 
   classimport.cpp 
   codeaccessormethod.cpp 
   codeblock.cpp 
   codeblockwithcomments.cpp 
   codeclassfield.cpp 
   codeclassfielddeclarationblock.cpp 
   codecomment.cpp 
   codedocument.cpp 
   codegenerationpolicy.cpp 
   codegenerator.cpp 
   codegenobjectwithtextblocks.cpp 
   codemethodblock.cpp 
   codeoperation.cpp 
   codeparameter.cpp 
   component.cpp 
   componentwidget.cpp 
   configurable.cpp 
   cppimport.cpp 
   datatype.cpp 
   datatypewidget.cpp 
   dialog_utils.cpp 
   docwindow.cpp 
   entity.cpp 
   entityattribute.cpp 
   entitywidget.cpp 
   enum.cpp 
   enumliteral.cpp 
   enumwidget.cpp 
   exportviewaction.cpp 
   floatingtext.cpp 
   forkjoinwidget.cpp 
   kplayerslideraction.cpp 
   hierarchicalcodeblock.cpp 
   idlimport.cpp 
   import_rose.cpp 
   import_utils.cpp 
   infowidget.cpp 
   javaimport.cpp 
   kstartuplogo.cpp 
   linepath.cpp 
   linkwidget.cpp 
   listpopupmenu.cpp 
   main.cpp 
   messagewidget.cpp 
   model_utils.cpp 
   nativeimportbase.cpp 
   node.cpp 
   nodewidget.cpp 
   notewidget.cpp 
   objectwidget.cpp 
   object_factory.cpp 
   operation.cpp 
   ownedcodeblock.cpp 
   ownedhierarchicalcodeblock.cpp 
   package.cpp 
   packagewidget.cpp 
   petalnode.cpp 
   petaltree2uml.cpp 
   plugin.cpp 
   pluginloader.cpp 
   pythonimport.cpp 
   seqlinewidget.cpp 
   statewidget.cpp 
   stereotype.cpp 
   template.cpp 
   textblock.cpp 
   toolbarstate.cpp 
   toolbarstatearrow.cpp 
   toolbarstateassociation.cpp 
   toolbarstatefactory.cpp 
   toolbarstatemessages.cpp 
   toolbarstateother.cpp 
   toolbarstatepool.cpp 
   uml.cpp 
   umlattributelist.cpp 
   umlentityattributelist.cpp 
   umlcanvasobject.cpp 
   umlclassifierlistitemlist.cpp 
   umldoc.cpp 
   umllistview.cpp 
   umllistviewitem.cpp 
   umlnamespace.cpp 
   umlobject.cpp 
   umlobjectlist.cpp 
   umlrole.cpp 
   umlview.cpp 
   umlviewcanvas.cpp 
   umlwidget.cpp 
   usecase.cpp 
   usecasewidget.cpp 
   widgetbase.cpp 
   widget_utils.cpp 
   worktoolbar.cpp )

kde4_automoc(${umbrello_SRCS})

kde4_add_executable(umbrello ${umbrello_SRCS})

target_link_libraries(umbrello  ${KDE4_KDECORE_LIBS} refactoring classparser clipboard dialogs codegenerator kdeprint kio )

install_targets(/bin umbrello )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES umbrello.desktop )
install_files( ${DATA_INSTALL_DIR}/umbrello FILES tips umbrelloui.rc )
install_files( ${MIME_INSTALL_DIR}/application FILES x-umbrello.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

#bin_PROGRAMS = umbrello
#
#umbrello_COMPILE_FIRST = version.h
#umbrello_SOURCES = activitywidget.cpp \
#actor.cpp \
#actorwidget.cpp \
#adaimport.cpp \
#aligntoolbar.cpp \
#artifact.cpp \
#artifactwidget.cpp \
#association.cpp \
#associationwidget.cpp \
#assocrules.cpp \
#attribute.cpp \
#boxwidget.cpp \
#classifier.cpp \
#classifiercodedocument.cpp \
#classifierlistitem.cpp \
#classifierwidget.cpp \
#classimport.cpp \
#codeaccessormethod.cpp \
#codeblock.cpp \
#codeblockwithcomments.cpp \
#codeclassfield.cpp \
#codeclassfielddeclarationblock.cpp \
#codecomment.cpp \
#codedocument.cpp \
#codegenerationpolicy.cpp \
#codegenerator.cpp \
#codegenobjectwithtextblocks.cpp \
#codemethodblock.cpp \
#codeoperation.cpp \
#codeparameter.cpp \
#component.cpp \
#componentwidget.cpp \
#configurable.cpp \
#cppimport.cpp \
#datatype.cpp \
#datatypewidget.cpp \
#dialog_utils.cpp \
#docwindow.cpp \
#entity.cpp \
#entityattribute.cpp \
#entitywidget.cpp \
#enum.cpp \
#enumliteral.cpp \
#enumwidget.cpp \
#exportviewaction.cpp \
#floatingtext.cpp \
#forkjoinwidget.cpp \
#kplayerslideraction.cpp \
#hierarchicalcodeblock.cpp \
#idlimport.cpp \
#import_rose.cpp \
#import_utils.cpp \
#infowidget.cpp \
#javaimport.cpp \
#kstartuplogo.cpp \
#linepath.cpp \
#linkwidget.cpp \
#listpopupmenu.cpp \
#main.cpp \
#messagewidget.cpp \
#model_utils.cpp \
#nativeimportbase.cpp \
#node.cpp \
#nodewidget.cpp \
#notewidget.cpp \
#objectwidget.cpp \
#object_factory.cpp \
#operation.cpp \
#ownedcodeblock.cpp \
#ownedhierarchicalcodeblock.cpp \
#package.cpp \
#packagewidget.cpp \
#petalnode.cpp \
#petaltree2uml.cpp \
#plugin.cpp \
#pluginloader.cpp \
#pythonimport.cpp \
#seqlinewidget.cpp \
#statewidget.cpp \
#stereotype.cpp \
#template.cpp \
#textblock.cpp \
#toolbarstate.cpp \
#toolbarstatearrow.cpp \
#toolbarstateassociation.cpp \
#toolbarstatefactory.cpp \
#toolbarstatemessages.cpp \
#toolbarstateother.cpp \
#toolbarstatepool.cpp \
#uml.cpp \
#umlattributelist.cpp \
#umlentityattributelist.cpp \
#umlcanvasobject.cpp \
#umlclassifierlistitemlist.cpp \
#umldoc.cpp \
#umllistview.cpp \
#umllistviewitem.cpp \
#umlnamespace.cpp \
#umlobject.cpp \
#umlobjectlist.cpp \
#umlrole.cpp \
#umlview.cpp \
#umlviewcanvas.cpp \
#umlwidget.cpp \
#usecase.cpp \
#usecasewidget.cpp \
#widgetbase.cpp \
#widget_utils.cpp \
#worktoolbar.cpp
#
##umbrello_LDADD   = ./refactoring/librefactoring.la ./classparser/libclassparser.la ./clipboard/libclipboard.la ./dialogs/libdialogs.la ./codegenerators/libcodegenerator.la $(LIB_KDEPRINT) $(LIB_KIO)
#umbrello_LDADD   = ./refactoring/librefactoring.la ./classparser/libclassparser.la ./clipboard/libclipboard.la ./dialogs/libdialogs.la ./codegenerators/libcodegenerator.la $(LIB_KDEPRINT) $(LIB_KIO)
#
#SUBDIRS = classparser dialogs clipboard pics codegenerators headings refactoring
#
#KDE_ICON=AUTO
#
#appdir=$(kde_datadir)/umbrello
#app_DATA = tips umbrelloui.rc
#
#xdg_apps_DATA = umbrello.desktop
#
#mimedir = $(kde_mimedir)/application
#mime_DATA = x-umbrello.desktop
#
##INCLUDES= -Idialogs -Irefactoring $(all_includes) -I/usr/include/graphviz/
#INCLUDES= -Idialogs -Irefactoring $(all_includes)
#
#METASOURCES = AUTO
#
#umbrello_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic
#
#messages: rc.cpp
#	$(PREPARETIPS) > tips.cpp
#	$(EXTRACTRC) *.rc codegenerators/*.ui dialogs/*.ui > ./rc.cpp
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#	 $(XGETTEXT) $$LIST -o $(podir)/umbrello.pot; \
#	fi
#	rm -f tips.cpp
#
#version.h: $(top_srcdir)/umbrello/VERSION
#	printf "#undef UMBRELLO_VERSION\n#define UMBRELLO_VERSION \"`cat $(top_srcdir)/umbrello/VERSION`\"\n" > version.h
#
#CLEANFILES = version.h
#
#noinst_HEADERS = version.h
#
