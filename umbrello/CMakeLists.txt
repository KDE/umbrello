add_subdirectory( pics )
add_subdirectory( headings )

add_definitions(-DQT3_SUPPORT_WARNINGS)

#### get subversion revision number ###

MESSAGE(STATUS "\n")
UNSET(SVN_FILE CACHE)
find_file(SVN_FILE .svn ${CMAKE_CURRENT_SOURCE_DIR} NO_DEFAULT_PATH)
if(SVN_FILE)
    MESSAGE(STATUS "SVN used: ${SVN_FILE}.")
    find_package(Subversion)
    if(Subversion_FOUND)
        # extract working copy information for SOURCE_DIR into MY_XXX variables
        Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} MY)
        set(SUBVERSION_REVISION ${MY_WC_REVISION})
        MESSAGE(STATUS "SVN Revision: ${MY_WC_REVISION}")
    else(Subversion_FOUND)
        set(SUBVERSION_REVISION 0)
        MESSAGE(STATUS "SVN Revision: 0 - no package Subversion available!")
    endif(Subversion_FOUND)
else(SVN_FILE)
    MESSAGE(STATUS "SVN not used.")
    set(SUBVERSION_REVISION 0)
    MESSAGE(STATUS "SVN Revision: 0 - not yet implemented!")
endif(SVN_FILE)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/revision.h.in ${CMAKE_CURRENT_BINARY_DIR}/revision.temp)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/revision.temp ${CMAKE_CURRENT_BINARY_DIR}/revision.h
                COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/revision.temp)

########### set variables #############

# Some definitions for experimental code or debugging
#add_definitions(-DWANT_LVTOOLTIP)                # umllistiew
#add_definitions(-DXMI_FLAT_PACKAGES)             # umlobject
#add_definitions(-DDEBUG_LVITEM_INSERTION_ORDER)  # umllistviewitem
#add_definitions(-DTRY_BUGFIX_120682)             # model_utils
#add_definitions(-DQ_CC_MSVC)                     # preprocesslexer
#add_definitions(-DDEBUG_CPPTREE2UML)             # cpptree2uml
#add_definitions(-DDEBUG_PRINTING)                # umlview
#add_definitions(-DVERBOSE_DEBUGGING)             # listpopupmenu, association, petaltree2uml,
                                                  # umlobject, umldoc, classifierlistpage

# Some Boost require exceptions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

include_directories(
  ${LIBXML2_INCLUDE_DIR}
  ${LIBXSLT_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/
  ${CMAKE_CURRENT_SOURCE_DIR}/clipboard/
  ${CMAKE_CURRENT_SOURCE_DIR}/cmds
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/ada/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/as/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/cpp/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/csharp/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/d/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/idl/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/java/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/js/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/pascal/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/perl/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/php/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/python/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/ruby/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/sql/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/tcl/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/vala/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/xml/
  ${CMAKE_CURRENT_SOURCE_DIR}/codegenwizard/
  ${CMAKE_CURRENT_SOURCE_DIR}/codeimport/
  ${CMAKE_CURRENT_SOURCE_DIR}/codeimport/csharp/
  ${CMAKE_CURRENT_SOURCE_DIR}/codeimpwizard/
  ${CMAKE_CURRENT_SOURCE_DIR}/controller/
  ${CMAKE_CURRENT_SOURCE_DIR}/debug/
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/
  ${CMAKE_CURRENT_SOURCE_DIR}/docgenerators/
  ${CMAKE_CURRENT_SOURCE_DIR}/refactoring/
  ${CMAKE_CURRENT_SOURCE_DIR}/widgets/
  ${Boost_INCLUDE_DIR}
)

set(libdebug_SRCS
    debug/debug_utils.cpp
)

set(libcodegenerator_SRCS
    codegenerators/codegenfactory.cpp
    codegenerators/codegenpolicyext.cpp
    codegenerators/codeblock.cpp
    codegenerators/codeblockwithcomments.cpp
    codegenerators/codeclassfield.cpp
    codegenerators/codeclassfielddeclarationblock.cpp
    codegenerators/codecomment.cpp
    codegenerators/codedocument.cpp
    codegenerators/codegenerationpolicy.cpp
    codegenerators/codegenerator.cpp
    codegenerators/codegenobjectwithtextblocks.cpp
    codegenerators/codemethodblock.cpp
    codegenerators/codeoperation.cpp
    codegenerators/codeparameter.cpp
    codegenerators/codegen_utils.cpp
    codegenerators/hierarchicalcodeblock.cpp
    codegenerators/ownedcodeblock.cpp
    codegenerators/ownedhierarchicalcodeblock.cpp
    codegenerators/textblock.cpp
    codegenerators/java/javacodegenerator.cpp
    codegenerators/java/javaantcodedocument.cpp
    codegenerators/java/javacodeclassfield.cpp
    codegenerators/java/javaclassifiercodedocument.cpp
    codegenerators/java/javacodecomment.cpp
    codegenerators/java/javacodedocumentation.cpp
    codegenerators/java/javacodeoperation.cpp
    codegenerators/java/javacodeaccessormethod.cpp
    codegenerators/java/javaclassdeclarationblock.cpp
    codegenerators/java/javacodeclassfielddeclarationblock.cpp
    codegenerators/java/javacodegenerationpolicy.cpp
    codegenerators/java/javacodegenerationpolicypage.cpp
    codegenerators/cpp/cppcodegenerator.cpp
    codegenerators/cpp/cppcodedocumentation.cpp
    codegenerators/cpp/cppcodeclassfield.cpp
    codegenerators/cpp/cppcodecomment.cpp
    codegenerators/cpp/cppmakecodedocument.cpp
    codegenerators/cpp/cppsourcecodedocument.cpp
    codegenerators/cpp/cppsourcecodeoperation.cpp
    codegenerators/cpp/cppsourcecodeaccessormethod.cpp
    codegenerators/cpp/cppsourcecodeclassfielddeclarationblock.cpp
    codegenerators/cpp/cppheadercodedocument.cpp
    codegenerators/cpp/cppheaderclassdeclarationblock.cpp
    codegenerators/cpp/cppheadercodeoperation.cpp
    codegenerators/cpp/cppheadercodeaccessormethod.cpp
    codegenerators/cpp/cppheadercodeclassfielddeclarationblock.cpp
    codegenerators/cpp/cppcodegenerationpolicy.cpp
    codegenerators/cpp/cppcodegenerationpolicypage.cpp
    codegenerators/cpp/cppcodegenerationform.cpp
    codegenerators/d/dcodegenerator.cpp
    codegenerators/d/dcodedocumentation.cpp
    codegenerators/d/dcodeclassfield.cpp
    codegenerators/d/dcodecomment.cpp
    codegenerators/d/dclassdeclarationblock.cpp
    codegenerators/d/dclassifiercodedocument.cpp
    codegenerators/d/dcodeoperation.cpp
    codegenerators/d/dcodeaccessormethod.cpp
    codegenerators/d/dcodeclassfielddeclarationblock.cpp
    codegenerators/d/dcodegenerationpolicy.cpp
    codegenerators/d/dcodegenerationpolicypage.cpp
    codegenerators/ruby/rubycodegenerator.cpp
    codegenerators/ruby/rubycodeclassfield.cpp
    codegenerators/ruby/rubyclassifiercodedocument.cpp
    codegenerators/ruby/rubycodecomment.cpp
    codegenerators/ruby/rubycodedocumentation.cpp
    codegenerators/ruby/rubycodeoperation.cpp
    codegenerators/ruby/rubycodeaccessormethod.cpp
    codegenerators/ruby/rubyclassdeclarationblock.cpp
    codegenerators/ruby/rubycodeclassfielddeclarationblock.cpp
    codegenerators/ruby/rubycodegenerationpolicy.cpp
    codegenerators/ruby/rubycodegenerationpolicypage.cpp
    codegenerators/simplecodegenerator.cpp
    codegenerators/ada/adawriter.cpp
    codegenerators/as/aswriter.cpp
    codegenerators/cpp/cppwriter.cpp
    codegenerators/csharp/csharpwriter.cpp
    codegenerators/d/dwriter.cpp
    codegenerators/java/javawriter.cpp
    codegenerators/js/jswriter.cpp
    codegenerators/idl/idlwriter.cpp
    codegenerators/sql/mysqlwriter.cpp
    codegenerators/pascal/pascalwriter.cpp
    codegenerators/perl/perlwriter.cpp
    codegenerators/php/phpwriter.cpp
    codegenerators/php/php5writer.cpp
    codegenerators/sql/postgresqlwriter.cpp
    codegenerators/python/pythonwriter.cpp
    codegenerators/ruby/rubywriter.cpp
    codegenerators/sql/sqlwriter.cpp
    codegenerators/tcl/tclwriter.cpp
    codegenerators/vala/valawriter.cpp
    codegenerators/xml/xmlschemawriter.cpp
    codegenerators/xml/xmlelementcodeblock.cpp
    codegenerators/xml/xmlcodecomment.cpp
)

kde4_add_ui_files(  libcodegenerator_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/cpp/cppcodegenerationformbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/ruby/rubycodegenerationformbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/d/dcodegenerationformbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/java/javacodegenerationformbase.ui
)

set(libdocgenerator_SRCS
    docgenerators/docbookgenerator.cpp
    docgenerators/docbookgeneratorjob.cpp
    docgenerators/xhtmlgenerator.cpp
    docgenerators/docbook2xhtmlgeneratorjob.cpp
)

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
    dialogs/codegenerationpolicypage.cpp
    dialogs/codeimportoptionspage.cpp
    dialogs/codeviewerdialog.cpp
    dialogs/codevieweroptionspage.cpp
    dialogs/codetextedit.cpp
    dialogs/codetexthighlighter.cpp
    dialogs/defaultcodegenpolicypage.cpp
    dialogs/diagramprintpage.cpp
    dialogs/dialog_utils.cpp
    dialogs/dialogbase.cpp
    dialogs/exportallviewsdialog.cpp
    dialogs/notedialog.cpp
    dialogs/objectnodedialog.cpp
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
    dialogs/umlwidgetstylepage.cpp
)

kde4_add_ui_files(  libdialogs_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codeviewerdialogbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codevieweroptionsbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/exportallviewsdialogbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/umlrolepropertiesbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/diagrampropertiespage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codeimportoptionspage.ui
)

set(librefactoring_SRCS
    refactoring/refactoringassistant.cpp
)

set(libcodegenwizard_SRCS
    codegenwizard/codegenselectpage.cpp
    codegenwizard/codegenoptionspage.cpp
    codegenwizard/codegenstatuspage.cpp
    codegenwizard/codegenerationwizard.cpp
)

kde4_add_ui_files(  libcodegenwizard_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenwizard/codegenselectpage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenwizard/codegenoptionspage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenwizard/codegenstatuspage.ui
)

set(libcodeimpwizard_SRCS
    codeimpwizard/codeimpselectpage.cpp
    codeimpwizard/codeimpstatuspage.cpp
    codeimpwizard/codeimpthread.cpp
    codeimpwizard/codeimportingwizard.cpp
)

kde4_add_ui_files(  libcodeimpwizard_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/codeimpwizard/codeimpselectpage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codeimpwizard/codeimpstatuspage.ui
)

set(libcodeimport_SRCS
    codeimport/adaimport.cpp
    codeimport/classimport.cpp
    codeimport/idlimport.cpp
    codeimport/import_utils.cpp
    codeimport/javaimport.cpp
    codeimport/nativeimportbase.cpp
    codeimport/pascalimport.cpp
    codeimport/pythonimport.cpp
    codeimport/csharp/csharpimport.cpp
)

if (WIN32 AND NOT MSVC)
    # SunPRO CC have a problem with position.h; we try to put a
    # QChar into a spirit iterator, which, after a bunch of futzing, ends up in
    # the STL string_ref templates, which use
    #
    #     union {
    #        size_type _C_size;    // Number of actual data values stored
    #        _CharT    _C_dummy;   // force the alignment of the first char
    #     } _C_size;               // named to work around an HP aCC 3.30 bug
    #
    # and you can't have a class which requires an initializer in a union.
    #
    add_definitions(-DDISABLE_CPP_IMPORT)
    set(libkdevcppparser_SRCS)
else (WIN32 AND NOT MSVC)
    list(APPEND libcodeimport_SRCS
        codeimport/cppimport.cpp
    )
    set(libkdevcppparser_SRCS
        codeimport/kdevcppparser/ast.cpp
        codeimport/kdevcppparser/driver.cpp
        codeimport/kdevcppparser/errors.cpp
        codeimport/kdevcppparser/lexer.cpp
        codeimport/kdevcppparser/lookup.cpp
        codeimport/kdevcppparser/parser.cpp
        codeimport/kdevcppparser/preprocesslexer.cpp
        codeimport/kdevcppparser/tree_parser.cpp
        codeimport/kdevcppparser/ast_utils.cpp
        codeimport/kdevcppparser/cpptree2uml.cpp
    )
endif(WIN32 AND NOT MSVC)

set(libclipboard_SRCS
    clipboard/umldragdata.cpp
    clipboard/umlclipboard.cpp
    clipboard/idchangelog.cpp
)

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
    cmds/widget/cmd_changeTextColor.cpp
    cmds/widget/cmd_changeLineColor.cpp
    cmds/widget/cmd_changeFillColor.cpp
    cmds/widget/cmd_changeMulti.cpp
)

set(libwidgets_SRCS
    widgets/activitywidget.cpp
    widgets/actorwidget.cpp
    widgets/artifactwidget.cpp
    widgets/associationline.cpp
    widgets/associationwidget.cpp
    widgets/boxwidget.cpp
    widgets/categorywidget.cpp
    widgets/classifierwidget.cpp
    widgets/combinedfragmentwidget.cpp
    widgets/componentwidget.cpp
    widgets/datatypewidget.cpp
    widgets/entitywidget.cpp
    widgets/enumwidget.cpp
    widgets/floatingdashlinewidget.cpp
    widgets/floatingtextwidget.cpp
    widgets/forkjoinwidget.cpp
    widgets/linkwidget.cpp
    widgets/messagewidget.cpp
    widgets/nodewidget.cpp
    widgets/notewidget.cpp
    widgets/objectnodewidget.cpp
    widgets/objectwidget.cpp
    widgets/packagewidget.cpp
    widgets/pinwidget.cpp
    widgets/preconditionwidget.cpp
    widgets/regionwidget.cpp
    widgets/seqlinewidget.cpp
    widgets/signalwidget.cpp
    widgets/statewidget.cpp
    widgets/toolbarstateonewidget.cpp
    widgets/umlwidget.cpp
    widgets/usecasewidget.cpp
    widgets/widget_factory.cpp
    widgets/widget_utils.cpp
    widgets/widgetbase.cpp
    widgets/statusbartoolbutton.cpp
)

set (libcontroller_SRCS
    controller/floatingtextwidgetcontroller.cpp
    controller/messagewidgetcontroller.cpp
    controller/notewidgetcontroller.cpp
    controller/objectwidgetcontroller.cpp
    controller/umlwidgetcontroller.cpp
)

########### next target ###############

set(libumbrello_SRCS
    actor.cpp
    artifact.cpp
    association.cpp
    assocrules.cpp
    attribute.cpp
    basictypes.cpp
    category.cpp
    checkconstraint.cpp
    classifier.cpp
    classifiercodedocument.cpp
    classifierlistitem.cpp
    cmdlineexportallviewsevent.cpp
    codeaccessormethod.cpp
    component.cpp
    docwindow.cpp
    entity.cpp
    entityattribute.cpp
    entityconstraint.cpp
    enum.cpp
    enumliteral.cpp
    folder.cpp
    foreignkeyconstraint.cpp
    icon_utils.cpp
    import_rose.cpp
    listpopupmenu.cpp
    main.cpp
    model_utils.cpp
    node.cpp
    object_factory.cpp
    operation.cpp
    optionstate.cpp
    package.cpp
    petalnode.cpp
    petaltree2uml.cpp
    stereotype.cpp
    template.cpp
    toolbarstate.cpp
    toolbarstatearrow.cpp
    toolbarstateassociation.cpp
    toolbarstatefactory.cpp
    toolbarstatemessages.cpp
    toolbarstateother.cpp
    toolbarstatepool.cpp
    uml.cpp
    umlattributelist.cpp
    umlcanvasobject.cpp
    umlclassifierlistitemlist.cpp
    umldoc.cpp
    umlentityattributelist.cpp
    umlentityconstraintlist.cpp
    umllistview.cpp
    umllistviewitem.cpp
    umlobject.cpp
    umlobjectlist.cpp
    umlrole.cpp
    umlscene.cpp
    umlview.cpp
    umlviewcanvas.cpp
    umlviewimageexporter.cpp
    umlviewimageexporterall.cpp
    umlviewimageexportermodel.cpp
    uniqueconstraint.cpp
    uniqueid.cpp
    usecase.cpp
    worktoolbar.cpp
)

kde4_add_kcfg_files(umbrello_SRCS umbrellosettings.kcfgc)

set(umbrello_SRCS
    ${umbrello_SRCS}
    ${libcontroller_SRCS}
    ${libdebug_SRCS}
    ${libdialogs_SRCS}
    ${librefactoring_SRCS}
    ${libcodegenwizard_SRCS}
    ${libcodeimpwizard_SRCS}
    ${libclassparser_SRCS}
    ${libclipboard_SRCS}
    ${libkdevcppparser_SRCS}
    ${libcodeimport_SRCS}
    ${libcodegenerator_SRCS}
    ${libdocgenerator_SRCS}
    ${libcmds_SRCS}
    ${libwidgets_SRCS}
    ${libumbrello_SRCS}
)

kde4_add_app_icon(umbrello_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/global/hi*-app-umbrello.png")
kde4_add_executable(umbrello ${umbrello_SRCS})
target_link_libraries(umbrello  ${KDE4_KFILE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${LIBUUID} )
install(TARGETS umbrello ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files #############

install( PROGRAMS umbrello.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES
    umbrelloui.rc
    docgenerators/xmi2docbook.xsl
    docgenerators/docbook2xhtml.xsl
    docgenerators/xmi.css
    docgenerators/common.ent
    DESTINATION ${DATA_INSTALL_DIR}/umbrello )

kde4_install_icons( ${ICON_INSTALL_DIR} )

