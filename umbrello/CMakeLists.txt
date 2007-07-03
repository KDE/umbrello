find_package(LibXml2)
find_package(LibXslt)

add_subdirectory( pics ) 
add_subdirectory( headings ) 
add_subdirectory( cmds )

macro(CREATE_VERSION _in_FILE _out_FILE)
    FILE(READ ${_in_FILE} _contents)
    FILE(WRITE ${_out_FILE} "#undef UMBRELLO_VERSION\n")
    STRING(REGEX REPLACE "\n" "" _contents "${_contents}" )
    FILE(APPEND ${_out_FILE} "#define UMBRELLO_VERSION \"${_contents}\"\n")
endmacro(CREATE_VERSION)

#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/version.h )
create_version(${CMAKE_SOURCE_DIR}/umbrello/VERSION ${CMAKE_CURRENT_BINARY_DIR}/version.h)



include_directories (${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/)

set(libcodegenerator_SRCS
	codegenerators/codegenfactory.cpp 
	codegenerators/codegenpolicyext.cpp 
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
	codegenerators/dcodegenerator.cpp 
	codegenerators/dcodedocumentation.cpp 
	codegenerators/dcodeclassfield.cpp
	codegenerators/dcodecomment.cpp 
	codegenerators/dclassdeclarationblock.cpp
	codegenerators/dclassifiercodedocument.cpp 
	codegenerators/dcodeoperation.cpp 
	codegenerators/dcodeaccessormethod.cpp
	codegenerators/dcodeclassfielddeclarationblock.cpp
	codegenerators/dcodegenerationpolicy.cpp 
	codegenerators/dcodegenerationpolicypage.cpp
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
	codegenerators/csharpwriter.cpp 
	codegenerators/dwriter.cpp 
	codegenerators/javawriter.cpp 
	codegenerators/jswriter.cpp
	codegenerators/idlwriter.cpp 
        codegenerators/mysqlwriter.cpp
	codegenerators/pascalwriter.cpp 
	codegenerators/perlwriter.cpp 
	codegenerators/phpwriter.cpp 
	codegenerators/php5writer.cpp 
        codegenerators/postgresqlwriter.cpp
	codegenerators/pythonwriter.cpp
	codegenerators/rubywriter.cpp 
	codegenerators/sqlwriter.cpp 
	codegenerators/tclwriter.cpp 
	codegenerators/xmlschemawriter.cpp
	codegenerators/xmlelementcodeblock.cpp 
	codegenerators/xmlcodecomment.cpp
	codegenerators/codegen_utils.cpp
)

kde4_add_ui3_files( libcodegenerator_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/cppcodegenerationformbase.ui
	${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/rubycodegenerationformbase.ui
)

kde4_add_ui_files(  libcodegenerator_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/dcodegenerationformbase.ui
	${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/javacodegenerationformbase.ui
)

kde4_automoc(${libcodegenerator_SRCS})

set(libdocgenerator_SRCS
    docgenerators/docbookgenerator.cpp 
    docgenerators/xhtmlgenerator.cpp)

kde4_automoc(${libdocgenerator_SRCS})

set(libdialogs_SRCS
	dialogs/activitydialog.cpp
	dialogs/activitypage.cpp 
	dialogs/assocgenpage.cpp 
	dialogs/assocpage.cpp 
	dialogs/assocpropdlg.cpp 
	dialogs/assocrolepage.cpp 
	dialogs/classgenpage.cpp 
	dialogs/classifierlistpage.cpp
        dialogs/constraintlistpage.cpp
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
        dialogs/objectnodedialog.cpp
	dialogs/importprojectdlg.cpp
	dialogs/overwritedialogue.cpp 
	dialogs/pkgcontentspage.cpp 
	dialogs/parmpropdlg.cpp 
	dialogs/selectopdlg.cpp 
	dialogs/settingsdlg.cpp 
	dialogs/statedialog.cpp 
	dialogs/umlattributedialog.cpp 
        dialogs/umlcheckconstraintdialog.cpp
	dialogs/umlentityattributedialog.cpp
        dialogs/umlforeignkeyconstraintdialog.cpp 
	dialogs/umloperationdialog.cpp 
	dialogs/umltemplatedialog.cpp 
	dialogs/umlroledialog.cpp 
	dialogs/umlroleproperties.cpp 
	dialogs/umluniqueconstraintdialog.cpp
        dialogs/umlviewdialog.cpp 
	dialogs/umlwidgetcolorpage.cpp )

kde4_add_ui3_files( libdialogs_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codegenerationoptionsbase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codegenerationpolicybase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codegenerationwizardbase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codeviewerdialogbase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codevieweroptionsbase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/exportallviewsdialogbase.ui 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/importprojectdlgbase.ui 
)

kde4_add_ui_files(  libdialogs_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/umlrolepropertiesbase.ui
	${CMAKE_CURRENT_SOURCE_DIR}/dialogs/diagrampropertiespage.ui
)

kde4_automoc(${libdialogs_SRCS})

set(librefactoring_SRCS 
		refactoring/refactoringassistant.cpp )

kde4_automoc(${librefactoring_SRCS})

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

kde4_automoc(${libkdevcppparser_SRCS})

set(libclipboard_SRCS 
	clipboard/umldrag.cpp 
	clipboard/umlclipboard.cpp 
	clipboard/idchangelog.cpp
)
kde4_automoc(${libclipboard_SRCS})

set(libcmds_SRCS
	cmds/cmd_create_activitydiag.cpp
	cmds/cmd_create_classdiag.cpp
	cmds/cmd_create_collaborationdiag.cpp
	cmds/cmd_create_componentdiag.cpp
	cmds/cmd_create_deploydiag.cpp
	cmds/cmd_create_entityrelationdiag.cpp
	cmds/cmd_create_seqdiag.cpp
	cmds/cmd_create_statediag.cpp
	cmds/cmd_create_usecasediag.cpp

	cmds/classdiag/cmd_create_class.cpp

	cmds/generic/cmd_create_umlobject.cpp
	cmds/generic/cmd_rename_umlobject.cpp
	
	cmds/widget/cmd_create_widget.cpp
	
	cmds/cmd_moveWidget.cpp
	cmds/cmd_resizeWidget.cpp
	cmds/cmd_set_visibility.cpp
	cmds/cmd_set_stereotype.cpp
	cmds/widget/cmd_set_name.cpp
	cmds/widget/cmd_set_txt.cpp
	cmds/cmd_handle_rename.cpp
	cmds/widget/cmd_changeFontSelection.cpp
	cmds/widget/cmd_changeLineColor.cpp
	cmds/widget/cmd_changeFillColor.cpp
	cmds/widget/cmd_changeMulti.cpp

)
kde4_automoc(${libclipboard_SRCS})
########### next target ###############

set(libumbrello_SRCS 
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
   checkconstraint.cpp
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
   combinedfragmentwidget.cpp 
   component.cpp 
   componentwidget.cpp 
   configurable.cpp 
   datatypewidget.cpp 
   dialog_utils.cpp 
   docwindow.cpp 
   entity.cpp 
   entityattribute.cpp 
   entityconstraint.cpp
   entitywidget.cpp 
   regionwidget.cpp 
   enum.cpp 
   enumliteral.cpp 
   enumwidget.cpp 
   floatingdashlinewidget.cpp
   floatingtextwidget.cpp 
   floatingtextwidgetcontroller.cpp
   folder.cpp
   foreignkeyconstraint.cpp
   forkjoinwidget.cpp 
   hierarchicalcodeblock.cpp 
   import_rose.cpp 
   kplayerslideraction.cpp
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
   objectnodewidget.cpp
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
   pinwidget.cpp   
   plugin.cpp 
   pluginloader.cpp 
   preconditionwidget.cpp
   seqlinewidget.cpp 
   statewidget.cpp 
   signalwidget.cpp 
   stereotype.cpp 
   template.cpp 
   textblock.cpp 
   toolbarstate.cpp 
   toolbarstatearrow.cpp 
   toolbarstateassociation.cpp 
   toolbarstatefactory.cpp 
   toolbarstatemessages.cpp 
   toolbarstateonewidget.cpp 
   toolbarstateother.cpp 
   toolbarstatepool.cpp 
   uml.cpp 
   umlattributelist.cpp 
   umlentityattributelist.cpp 
   umlentityconstraintlist.cpp
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
   uniqueconstraint.cpp
   usecase.cpp 
   usecasewidget.cpp 
   widgetbase.cpp 
   widget_factory.cpp
   widget_utils.cpp 
   worktoolbar.cpp 
)

kde4_automoc(${libumbrello_SRCS})

set (umbrello_SRCS
	${libdialogs_SRCS} 
	${librefactoring_SRCS} 
	${libclassparser_SRCS} 
	${libclipboard_SRCS} 
	${libkdevcppparser_SRCS}
	${libcodeimport_SRCS}
	${libcodegenerator_SRCS}
	${libdocgenerator_SRCS}
	${libcmds_SRCS}
	${libumbrello_SRCS}
)

kde4_add_kcfg_files(umbrello_SRCS umbrellosettings.kcfgc)

kde4_add_executable(umbrello ${umbrello_SRCS})
target_link_libraries(umbrello ${KDE4_KIO_LIBS} ${KDE4_KDEPRINT_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${LIBUUID})
install(TARGETS umbrello DESTINATION ${BIN_INSTALL_DIR} )

########### install files ###############

install( FILES umbrello.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES
  umbrelloui.rc
  docgenerators/xmi2docbook.xsl
  docgenerators/docbook2xhtml.xsl
  docgenerators/xmi.css
  docgenerators/common.ent
  DESTINATION ${DATA_INSTALL_DIR}/umbrello )

kde4_install_icons( ${ICON_INSTALL_DIR}   )

