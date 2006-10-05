add_subdirectory( codeimport ) 
add_subdirectory( dialogs ) 
add_subdirectory( clipboard ) 
add_subdirectory( pics ) 
add_subdirectory( codegenerators ) 
add_subdirectory( docgenerators ) 
add_subdirectory( headings ) 
add_subdirectory( refactoring ) 

macro(CREATE_VERSION _in_FILE _out_FILE)
    FILE(READ ${_in_FILE} _contents)
    FILE(WRITE ${_out_FILE} "#undef UMBRELLO_VERSION\n")
    STRING(REGEX REPLACE "\n" "" _contents "${_contents}" )
    FILE(APPEND ${_out_FILE} "#define UMBRELLO_VERSION \"${_contents}\"")
endmacro(CREATE_VERSION)

macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/version.h )
create_version(${CMAKE_SOURCE_DIR}/umbrello/VERSION version.h)

include_directories (/usr/include/libxml2 ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/)

set(libcodegenerator_SRCS
	codegenerators/codegenfactory.cpp 
	codegenerators/classifierinfo.cpp 
	codegenerators/javacodegenerator.cpp 
	codegenerators/javaantcodedocument.cpp 
	codegenerators/javacodeclassfield.cpp
	codegenerators/javaclassifiercodedocument.cpp 
	codegenerators/javacodecomment.cpp 
	codegenerators/javacodedocumentation.cpp
	codegenerators/javacodeoperation.cpp 
	codegenerators/javacodeaccessormethod.cpp 
	codegenerators/javaclassdeclarationblock.cpp 
	codegenerators/javacodeclassfielddeclarationblock.cpp
	codegenerators/javacodegenerationpolicy.cpp 
	codegenerators/javacodegenerationpolicypage.cpp
	codegenerators/cppcodegenerator.cpp 
	codegenerators/cppcodedocumentation.cpp 
	codegenerators/cppcodeclassfield.cpp
	codegenerators/cppcodecomment.cpp 
	codegenerators/cppmakecodedocument.cpp
	codegenerators/cppsourcecodedocument.cpp 
	codegenerators/cppsourcecodeoperation.cpp 
	codegenerators/cppsourcecodeaccessormethod.cpp
	codegenerators/cppsourcecodeclassfielddeclarationblock.cpp
	codegenerators/cppheadercodedocument.cpp 
	codegenerators/cppheaderclassdeclarationblock.cpp
	codegenerators/cppheadercodeoperation.cpp 
	codegenerators/cppheadercodeaccessormethod.cpp
	codegenerators/cppheadercodeclassfielddeclarationblock.cpp
	codegenerators/cppcodegenerationpolicy.cpp 
	codegenerators/cppcodegenerationpolicypage.cpp
	codegenerators/cppcodegenerationform.cpp 
	codegenerators/rubycodegenerator.cpp 
	codegenerators/rubycodeclassfield.cpp 
	codegenerators/rubyclassifiercodedocument.cpp 
	codegenerators/rubycodecomment.cpp 
	codegenerators/rubycodedocumentation.cpp 
	codegenerators/rubycodeoperation.cpp 
	codegenerators/rubycodeaccessormethod.cpp
	codegenerators/rubyclassdeclarationblock.cpp 
	codegenerators/rubycodeclassfielddeclarationblock.cpp
	codegenerators/rubycodegenerationpolicy.cpp 
	codegenerators/rubycodegenerationpolicypage.cpp
	codegenerators/simplecodegenerator.cpp 
	codegenerators/adawriter.cpp 
	codegenerators/aswriter.cpp 
	codegenerators/cppwriter.cpp 
	codegenerators/javawriter.cpp 
	codegenerators/jswriter.cpp
	codegenerators/idlwriter.cpp 
	codegenerators/pascalwriter.cpp 
	codegenerators/perlwriter.cpp 
	codegenerators/phpwriter.cpp 
	codegenerators/php5writer.cpp 
	codegenerators/pythonwriter.cpp
	codegenerators/rubywriter.cpp 
	codegenerators/sqlwriter.cpp 
	codegenerators/tclwriter.cpp 
	codegenerators/xmlschemawriter.cpp
	codegenerators/xmlelementcodeblock.cpp 
	codegenerators/xmlcodecomment.cpp
	codegenerators/codegen_utils.cpp)

set(libdocgenerator_SRCS
    docgenerators/docbookgenerator.cpp 
    docgenerators/xhtmlgenerator.cpp)

set(libdialogs_SRCS
	dialogs/activitydialog.cpp 
	dialogs/activitypage.cpp 
	dialogs/assocgenpage.cpp 
	dialogs/assocpage.cpp 
	dialogs/assocpropdlg.cpp 
	dialogs/assocrolepage.cpp 
	dialogs/classgenpage.cpp 
	dialogs/classifierlistpage.cpp
	dialogs/classoptionspage.cpp 
	dialogs/classpropdlg.cpp
	dialogs/classwizard.cpp 
	dialogs/codeeditor.cpp 
	dialogs/codegenerationoptionspage.cpp 
	dialogs/codegenerationpolicypage.cpp 
	dialogs/codegenerationwizard.cpp 
	dialogs/codeviewerdialog.cpp 
	dialogs/codevieweroptionspage.cpp 
	dialogs/defaultcodegenpolicypage.cpp 
	dialogs/diagramprintpage.cpp 
    dialogs/exportallviewsdialog.cpp
	dialogs/notedialog.cpp 
	dialogs/overwritedialogue.cpp 
	dialogs/pkgcontentspage.cpp 
	dialogs/parmpropdlg.cpp 
	dialogs/selectopdlg.cpp 
	dialogs/settingsdlg.cpp 
	dialogs/statedialog.cpp 
	dialogs/umlattributedialog.cpp 
	dialogs/umlentityattributedialog.cpp 
	dialogs/umloperationdialog.cpp 
	dialogs/umltemplatedialog.cpp 
	dialogs/umlroledialog.cpp 
	dialogs/umlroleproperties.cpp 
	dialogs/umlviewdialog.cpp 
	dialogs/umlwidgetcolorpage.cpp )

set(librefactoring_SRCS 
		refactoring/refactoringassistant.cpp )

set(libcodeimport_SRCS 
	codeimport/adaimport.cpp
	codeimport/classimport.cpp
	codeimport/cppimport.cpp
	codeimport/idlimport.cpp
	codeimport/import_utils.cpp
	codeimport/javaimport.cpp
	codeimport/nativeimportbase.cpp
	codeimport/pascalimport.cpp
	codeimport/pythonimport.cpp
)

set(libkdevcppparser_SRCS 
	codeimport/kdevcppparser/ast.cpp 
	codeimport/kdevcppparser/driver.cpp 
	codeimport/kdevcppparser/errors.cpp 
	codeimport/kdevcppparser/lexer.cpp 
	codeimport/kdevcppparser/lookup.cpp 
	codeimport/kdevcppparser/parser.cpp 
	codeimport/kdevcppparser/tree_parser.cpp 
	codeimport/kdevcppparser/urlutil.cpp 
	codeimport/kdevcppparser/ast_utils.cpp 
	codeimport/kdevcppparser/cpptree2uml.cpp
)

set(libclipboard_SRCS 
	clipboard/umldrag.cpp 
	clipboard/umlclipboard.cpp 
	clipboard/idchangelog.cpp
	)
########### next target ###############

set(umbrello_SRCS 
	${libdialogs_SRCS} 
	${librefactoring_SRCS} 
	${libclassparser_SRCS} 
	${libclipboard_SRCS} 
    ${libkdevcppparser_SRCS}
    ${libcodeimport_SRCS}
    ${libcodegenerator_SRCS}
    ${libdocgenerator_SRCS}
	activitywidget.cpp 
   actor.cpp 
   actorwidget.cpp 
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
   cmdlineexportallviewsevent.cpp
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
   floatingtextwidget.cpp 
   floatingtextwidgetcontroller.cpp
   folder.cpp
   forkjoinwidget.cpp 
   hierarchicalcodeblock.cpp 
   import_rose.cpp 
   kplayerslideraction.cpp
   kstartuplogo.cpp 
   linepath.cpp 
   linkwidget.cpp 
   listpopupmenu.cpp 
   main.cpp 
   messagewidget.cpp 
   messagewidgetcontroller.cpp
   model_utils.cpp 
   node.cpp 
   nodewidget.cpp 
   notewidget.cpp 
   notewidgetcontroller.cpp
   objectwidget.cpp 
   objectwidgetcontroller.cpp
   object_factory.cpp 
   operation.cpp 
   optionstate.cpp 
   ownedcodeblock.cpp 
   ownedhierarchicalcodeblock.cpp 
   package.cpp 
   packagewidget.cpp 
   petalnode.cpp 
   petaltree2uml.cpp 
   plugin.cpp 
   pluginloader.cpp 
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
   umlviewimageexporter.cpp
   umlviewimageexporterall.cpp
   umlviewimageexportermodel.cpp
   umlwidget.cpp 
   umlwidgetcontroller.cpp
   uniqueid.cpp
   usecase.cpp 
   usecasewidget.cpp 
   widgetbase.cpp 
   widget_factory.cpp
   widget_utils.cpp 
   worktoolbar.cpp 
   )

kde4_automoc(${umbrello_SRCS})

kde4_add_ui3_files( umbrello_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codegenerationoptionsbase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codegenerationpolicybase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codegenerationwizardbase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codeviewerdialogbase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codevieweroptionsbase.ui 
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/exportallviewsdialogbase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/cppcodegenerationformbase.ui
	${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/rubycodegenerationformbase.ui
)

kde4_add_ui_files( umbrello_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/javacodegenerationformbase.ui
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/umlrolepropertiesbase.ui
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/diagrampropertiespage.ui)

kde4_add_executable(umbrello ${umbrello_SRCS})

target_link_libraries(umbrello  ${KDE4_KIO_LIBS}  ${KDE4_KDEPRINT_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})

install(TARGETS umbrello  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES umbrello.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES tips umbrelloui.rc  DESTINATION  ${DATA_INSTALL_DIR}/umbrello )
install( FILES x-umbrello.desktop  DESTINATION  ${MIME_INSTALL_DIR}/application )

kde4_install_icons( ${ICON_INSTALL_DIR}   )




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
