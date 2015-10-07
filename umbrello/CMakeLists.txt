add_subdirectory( pics )
add_subdirectory( headings )

########### set variables #############

# Some definitions for experimental code or debugging
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
#add_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
#add_definitions(-DWANT_LVTOOLTIP)                # umllistiew
#add_definitions(-DXMI_FLAT_PACKAGES)             # umlobject
#add_definitions(-DDEBUG_LVITEM_INSERTION_ORDER)  # umllistviewitem
#add_definitions(-DTRY_BUGFIX_120682)             # model_utils
#add_definitions(-DQ_CC_MSVC)                     # preprocesslexer
#add_definitions(-DDEBUG_CPPTREE2UML)             # cpptree2uml
#add_definitions(-DDEBUG_PRINTING)                # umlview
#add_definitions(-DVERBOSE_DEBUGGING)             # listpopupmenu, association, petaltree2uml,
                                                  # umlobject, umldoc, classifierlistpage

if(UNIX)
    add_definitions(-D_GLIBCXX_PERMIT_BACKWARD_HASH)
endif()

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
  ${CMAKE_CURRENT_SOURCE_DIR}/debug/
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/pages/
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/widgets/
  ${CMAKE_CURRENT_SOURCE_DIR}/docgenerators/
  ${CMAKE_CURRENT_SOURCE_DIR}/finder/
  ${CMAKE_CURRENT_SOURCE_DIR}/refactoring/
  ${CMAKE_CURRENT_SOURCE_DIR}/widgets/
  ${CMAKE_SOURCE_DIR}/lib/cppparser/
  ${CMAKE_SOURCE_DIR}/lib/interfaces/
)

set(libdebug_SRCS
    debug/debug_utils.cpp
)

set(libcodegenerator_SRCS
    codegenerators/advancedcodegenerator.cpp
    codegenerators/classifiercodedocument.cpp
    codegenerators/codeaccessormethod.cpp
    codegenerators/codeblock.cpp
    codegenerators/codeblockwithcomments.cpp
    codegenerators/codeclassfield.cpp
    codegenerators/codeclassfielddeclarationblock.cpp
    codegenerators/codecomment.cpp
    codegenerators/codedocument.cpp
    codegenerators/codegenerationpolicy.cpp
    codegenerators/codegenerator.cpp
    codegenerators/codegenfactory.cpp
    codegenerators/codegenobjectwithtextblocks.cpp
    codegenerators/codegenpolicyext.cpp
    codegenerators/codegen_utils.cpp
    codegenerators/codemethodblock.cpp
    codegenerators/codeoperation.cpp
    codegenerators/codeparameter.cpp
    codegenerators/ada/adawriter.cpp
    codegenerators/as/aswriter.cpp
    codegenerators/cpp/cppcodeclassfield.cpp
    codegenerators/cpp/cppcodecomment.cpp
    codegenerators/cpp/cppcodedocumentation.cpp
    codegenerators/cpp/cppcodegenerationform.cpp
    codegenerators/cpp/cppcodegenerationpolicy.cpp
    codegenerators/cpp/cppcodegenerationpolicypage.cpp
    codegenerators/cpp/cppcodegenerator.cpp
    codegenerators/cpp/cppheaderclassdeclarationblock.cpp
    codegenerators/cpp/cppheadercodeaccessormethod.cpp
    codegenerators/cpp/cppheadercodeclassfielddeclarationblock.cpp
    codegenerators/cpp/cppheadercodedocument.cpp
    codegenerators/cpp/cppheadercodeoperation.cpp
    codegenerators/cpp/cppmakecodedocument.cpp
    codegenerators/cpp/cppsourcecodeaccessormethod.cpp
    codegenerators/cpp/cppsourcecodeclassfielddeclarationblock.cpp
    codegenerators/cpp/cppsourcecodedocument.cpp
    codegenerators/cpp/cppsourcecodeoperation.cpp
    codegenerators/cpp/cppwriter.cpp
    codegenerators/csharp/csharpwriter.cpp
    codegenerators/d/dclassdeclarationblock.cpp
    codegenerators/d/dclassifiercodedocument.cpp
    codegenerators/d/dcodeaccessormethod.cpp
    codegenerators/d/dcodeclassfield.cpp
    codegenerators/d/dcodeclassfielddeclarationblock.cpp
    codegenerators/d/dcodecomment.cpp
    codegenerators/d/dcodedocumentation.cpp
    codegenerators/d/dcodegenerationpolicy.cpp
    codegenerators/d/dcodegenerationpolicypage.cpp
    codegenerators/d/dcodegenerator.cpp
    codegenerators/d/dcodeoperation.cpp
    codegenerators/d/dwriter.cpp
    codegenerators/hierarchicalcodeblock.cpp
    codegenerators/idl/idlwriter.cpp
    codegenerators/java/javaantcodedocument.cpp
    codegenerators/java/javaclassdeclarationblock.cpp
    codegenerators/java/javaclassifiercodedocument.cpp
    codegenerators/java/javacodeaccessormethod.cpp
    codegenerators/java/javacodeclassfield.cpp
    codegenerators/java/javacodeclassfielddeclarationblock.cpp
    codegenerators/java/javacodecomment.cpp
    codegenerators/java/javacodedocumentation.cpp
    codegenerators/java/javacodegenerationpolicy.cpp
    codegenerators/java/javacodegenerationpolicypage.cpp
    codegenerators/java/javacodegenerator.cpp
    codegenerators/java/javacodeoperation.cpp
    codegenerators/java/javawriter.cpp
    codegenerators/js/jswriter.cpp
    codegenerators/ownedcodeblock.cpp
    codegenerators/ownedhierarchicalcodeblock.cpp
    codegenerators/pascal/pascalwriter.cpp
    codegenerators/perl/perlwriter.cpp
    codegenerators/php/php5writer.cpp
    codegenerators/php/phpwriter.cpp
    codegenerators/python/pythonwriter.cpp
    codegenerators/ruby/rubyclassdeclarationblock.cpp
    codegenerators/ruby/rubyclassifiercodedocument.cpp
    codegenerators/ruby/rubycodeaccessormethod.cpp
    codegenerators/ruby/rubycodeclassfield.cpp
    codegenerators/ruby/rubycodeclassfielddeclarationblock.cpp
    codegenerators/ruby/rubycodecomment.cpp
    codegenerators/ruby/rubycodedocumentation.cpp
    codegenerators/ruby/rubycodegenerationpolicy.cpp
    codegenerators/ruby/rubycodegenerationpolicypage.cpp
    codegenerators/ruby/rubycodegenerator.cpp
    codegenerators/ruby/rubycodeoperation.cpp
    codegenerators/ruby/rubywriter.cpp
    codegenerators/simplecodegenerator.cpp
    codegenerators/sql/mysqlwriter.cpp
    codegenerators/sql/postgresqlwriter.cpp
    codegenerators/sql/sqlwriter.cpp
    codegenerators/tcl/tclwriter.cpp
    codegenerators/textblock.cpp
    codegenerators/vala/valawriter.cpp
    codegenerators/xml/xmlcodecomment.cpp
    codegenerators/xml/xmlelementcodeblock.cpp
    codegenerators/xml/xmlschemawriter.cpp
)

ki18n_wrap_ui(  libcodegenerator_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/cpp/cppcodegenerationformbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/d/dcodegenerationformbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/java/javacodegenerationformbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenerators/ruby/rubycodegenerationformbase.ui
)

set(libdocgenerator_SRCS
    docgenerators/docbook2xhtmlgeneratorjob.cpp
    docgenerators/docbookgenerator.cpp
    docgenerators/docbookgeneratorjob.cpp
    docgenerators/xhtmlgenerator.cpp
)

set(libdialogs_SRCS
    dialogs/activitydialog.cpp
    dialogs/associationpropertiesdialog.cpp
    dialogs/classpropertiesdialog.cpp
    dialogs/classwizard.cpp
    dialogs/codeeditor.cpp
    dialogs/codetextedit.cpp
    dialogs/codetexthighlighter.cpp
    dialogs/codeviewerdialog.cpp
    dialogs/diagramselectiondialog.cpp
    dialogs/dialog_utils.cpp
    dialogs/exportallviewsdialog.cpp
    dialogs/finddialog.cpp
    dialogs/multipagedialogbase.cpp
    dialogs/notedialog.cpp
    dialogs/objectnodedialog.cpp
    dialogs/overwritedialog.cpp
    dialogs/parameterpropertiesdialog.cpp
    dialogs/selectoperationdialog.cpp
    dialogs/settingsdialog.cpp
    dialogs/singlepagedialogbase.cpp
    dialogs/statedialog.cpp
    dialogs/umlattributedialog.cpp
    dialogs/umlcheckconstraintdialog.cpp
    dialogs/umlentityattributedialog.cpp
    dialogs/umlfiledialog.cpp
    dialogs/umlforeignkeyconstraintdialog.cpp
    dialogs/umloperationdialog.cpp
    dialogs/umlroledialog.cpp
    dialogs/umltemplatedialog.cpp
    dialogs/umluniqueconstraintdialog.cpp
    dialogs/umlviewdialog.cpp
    dialogs/pages/activitypage.cpp
    dialogs/pages/associationgeneralpage.cpp
    dialogs/pages/associationrolepage.cpp
    dialogs/pages/autolayoutoptionpage.cpp
    dialogs/pages/classassociationspage.cpp
    dialogs/pages/classgeneralpage.cpp
    dialogs/pages/classifierlistpage.cpp
    dialogs/pages/classoptionspage.cpp
    dialogs/pages/codegenerationpolicypage.cpp
    dialogs/pages/codeimportoptionspage.cpp
    dialogs/pages/codevieweroptionspage.cpp
    dialogs/pages/constraintlistpage.cpp
    dialogs/pages/defaultcodegenpolicypage.cpp
    dialogs/pages/diagramprintpage.cpp
    dialogs/pages/diagrampropertiespage.cpp
    dialogs/pages/dialogpagebase.cpp
    dialogs/pages/generaloptionpage.cpp
    dialogs/pages/packagecontentspage.cpp
    dialogs/pages/umlroleproperties.cpp
    dialogs/pages/umlwidgetstylepage.cpp
    dialogs/widgets/documentationwidget.cpp
    dialogs/widgets/umlartifacttypewidget.cpp
    dialogs/widgets/umlobjectnamewidget.cpp
    dialogs/widgets/umlpackagewidget.cpp
    dialogs/widgets/umlstereotypewidget.cpp
    dialogs/widgets/visibilityenumwidget.cpp
)

ki18n_wrap_ui(  libdialogs_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codeviewerdialogbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/codevieweroptionsbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/exportallviewsdialogbase.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/finddialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/pages/autolayoutoptionpage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/pages/codeimportoptionspage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/pages/diagrampropertiespage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/pages/umlrolepropertiesbase.ui
)

set(librefactoring_SRCS
    refactoring/refactoringassistant.cpp
)

set(libcodegenwizard_SRCS
    codegenwizard/codegenerationwizard.cpp
    codegenwizard/codegenoptionspage.cpp
    codegenwizard/codegenselectpage.cpp
    codegenwizard/codegenstatuspage.cpp
)

ki18n_wrap_ui(  libcodegenwizard_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenwizard/codegenoptionspage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenwizard/codegenselectpage.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/codegenwizard/codegenstatuspage.ui
)

set(libcodeimpwizard_SRCS
    codeimpwizard/codeimportingwizard.cpp
    codeimpwizard/codeimpselectpage.cpp
    codeimpwizard/codeimpstatuspage.cpp
    codeimpwizard/codeimpthread.cpp
)

ki18n_wrap_ui(  libcodeimpwizard_SRCS
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
    codeimport/sqlimport.cpp
    codeimport/csharp/csharpimport.cpp
)

list(APPEND libcodeimport_SRCS
    codeimport/cppimport.cpp
)

set(libkdevcppparser_SRCS
    ${CMAKE_SOURCE_DIR}/lib/cppparser/ast.cpp
    ${CMAKE_SOURCE_DIR}/lib/cppparser/ast_utils.cpp
    ${CMAKE_SOURCE_DIR}/lib/cppparser/cachemanager.cpp
    ${CMAKE_SOURCE_DIR}/lib/cppparser/driver.cpp
    ${CMAKE_SOURCE_DIR}/lib/cppparser/errors.cpp
    ${CMAKE_SOURCE_DIR}/lib/cppparser/lexer.cpp
    ${CMAKE_SOURCE_DIR}/lib/cppparser/lexercache.cpp
    ${CMAKE_SOURCE_DIR}/lib/cppparser/lookup.cpp
    ${CMAKE_SOURCE_DIR}/lib/cppparser/parser.cpp
    ${CMAKE_SOURCE_DIR}/lib/cppparser/tree_parser.cpp
    ${CMAKE_SOURCE_DIR}/lib/interfaces/hashedstring.cpp
    codeimport/kdevcppparser/cpptree2uml.cpp
)

set(libclipboard_SRCS
    clipboard/idchangelog.cpp
    clipboard/umlclipboard.cpp
    clipboard/umldragdata.cpp
)

set(libcmds_SRCS
    cmds/cmd_baseObjectCommand.cpp
    cmds/cmd_createDiagram.cpp
    cmds/cmd_handleRename.cpp
    cmds/cmd_removeDiagram.cpp
    cmds/cmd_setStereotype.cpp
    cmds/cmd_setVisibility.cpp
    cmds/generic/cmd_createUMLObject.cpp
    cmds/generic/cmd_removeUMLObject.cpp
    cmds/generic/cmd_renameUMLObject.cpp
    cmds/widget/cmd_baseWidgetCommand.cpp
    cmds/widget/cmd_changeFont.cpp
    cmds/widget/cmd_changeTextColor.cpp
    cmds/widget/cmd_changeLineColor.cpp
    cmds/widget/cmd_changeLineWidth.cpp
    cmds/widget/cmd_changeFillColor.cpp
    cmds/widget/cmd_changeUseFillColor.cpp
    cmds/widget/cmd_changeMultiplicity.cpp
    cmds/widget/cmd_changeVisualProperty.cpp
    cmds/widget/cmd_createWidget.cpp
    cmds/widget/cmd_moveWidget.cpp
    cmds/widget/cmd_resizeWidget.cpp
    cmds/widget/cmd_removeWidget.cpp
    cmds/widget/cmd_setName.cpp
    cmds/widget/cmd_setTxt.cpp
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
    widgets/layoutgrid.cpp
    widgets/linkwidget.cpp
    widgets/messagewidget.cpp
    widgets/nodewidget.cpp
    widgets/notewidget.cpp
    widgets/objectnodewidget.cpp
    widgets/objectwidget.cpp
    widgets/packagewidget.cpp
    widgets/pinportbase.cpp
    widgets/pinwidget.cpp
    widgets/portwidget.cpp
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
    widgets/widgetlist_utils.cpp
    widgets/statusbartoolbutton.cpp
)

set(libfinder_SRCS
    finder/findresults.cpp
    finder/umldocfinder.cpp
    finder/umlfinder.cpp
    finder/umllistviewfinder.cpp
    finder/umlscenefinder.cpp
)

########### next target ###############

set(umbrellobase_SRCS
    actor.cpp
    artifact.cpp
    association.cpp
    assocrules.cpp
    attribute.cpp
    basictypes.cpp
    birdview.cpp
    category.cpp
    checkconstraint.cpp
    classifier.cpp
    classifierlistitem.cpp
    cmdlineexportallviewsevent.cpp
    component.cpp
    docwindow.cpp
    dotgenerator.cpp
    entity.cpp
    entityattribute.cpp
    entityconstraint.cpp
    enum.cpp
    enumliteral.cpp
    folder.cpp
    foreignkeyconstraint.cpp
    icon_utils.cpp
    import_argo.cpp
    import_rose.cpp
    layoutgenerator.cpp
    listpopupmenu.cpp
    model_utils.cpp
    node.cpp
    object_factory.cpp
    operation.cpp
    optionstate.cpp
    package.cpp
    petalnode.cpp
    petaltree2uml.cpp
    port.cpp
    stereotype.cpp
    template.cpp
    stereotypeswindow.cpp
    stereotypesmodel.cpp
    toolbarstatearrow.cpp
    toolbarstateassociation.cpp
    toolbarstate.cpp
    toolbarstatefactory.cpp
    toolbarstatemessages.cpp
    toolbarstateother.cpp
    toolbarstatepool.cpp
    umlappprivate.cpp
    umlattributelist.cpp
    umlcanvasobject.cpp
    umlclassifierlistitemlist.cpp
    uml.cpp
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
    umlviewimageexporterall.cpp
    umlviewimageexporter.cpp
    umlviewimageexportermodel.cpp
    uniqueconstraint.cpp
    uniqueid.cpp
    usecase.cpp
    worktoolbar.cpp
)

kconfig_add_kcfg_files(umbrellobase_SRCS umbrellosettings.kcfgc)

set(libumbrello_SRCS
    ${libdebug_SRCS}
    ${libdialogs_SRCS}
    ${libfinder_SRCS}
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
    ${umbrellobase_SRCS}
)

set(umbrello_SRCS
    main.cpp
)

set(LIB_BUILD_MODE STATIC)

if(NOT BUILD_KF5)
    kde4_add_library(libumbrello ${LIB_BUILD_MODE} ${libumbrello_SRCS})
    set_target_properties(libumbrello PROPERTIES OUTPUT_NAME umbrello)
    target_link_libraries(libumbrello
        Qt4::QtCore
        Qt4::QtGui
        Qt4::QtXml
        ${LIBXML2_LIBRARIES}
        ${LIBXSLT_LIBRARIES}
        ${KDE4_KTEXTEDITOR_LIBS}
        ${KDE4_KFILE_LIBS}
    )
    kde4_add_app_icon(umbrello_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/global/hi*-apps-umbrello.png")
    kde4_add_executable(umbrello ${umbrello_SRCS})
    target_link_libraries(umbrello ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KFILE_LIBS} libumbrello)
else()
    add_library(libumbrello ${LIB_BUILD_MODE} ${libumbrello_SRCS})
    set_target_properties(libumbrello PROPERTIES OUTPUT_NAME umbrello)
    target_link_libraries(libumbrello
        Qt5::Widgets
        Qt5::Xml
        Qt5::DBus
        Qt5::PrintSupport
        Qt5::Svg
        KF5::Archive
        KF5::Auth
        KF5::Completion
        KF5::CoreAddons
        KF5::I18n
        KF5::IconThemes
        KF5::GuiAddons
        KF5::KIOCore
        KF5::TextEditor
        KF5::WidgetsAddons
        KF5::WindowSystem
        KF5::XmlGui
        ${LIBXSLT_LIBRARIES}
        ${LIBXML2_LIBRARIES}
    )
    ecm_add_app_icon(umbrello_SRCS
        ICONS
        ${CMAKE_CURRENT_SOURCE_DIR}/pics/global/hi16-apps-umbrello.png
        ${CMAKE_CURRENT_SOURCE_DIR}/pics/global/hi22-apps-umbrello.png
        ${CMAKE_CURRENT_SOURCE_DIR}/pics/global/hi32-apps-umbrello.png
        ${CMAKE_CURRENT_SOURCE_DIR}/pics/global/hi48-apps-umbrello.png
        ${CMAKE_CURRENT_SOURCE_DIR}/pics/global/hi64-apps-umbrello.png
        ${CMAKE_CURRENT_SOURCE_DIR}/pics/global/hi128-apps-umbrello.png
    )
    add_executable(umbrello ${umbrello_SRCS})
    target_link_libraries(umbrello libumbrello)
endif()
if(LIB_BUILD_MODE EQUAL SHARED)
    install(TARGETS libumbrello ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif()
install(TARGETS umbrello ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files #############

if(NOT BUILD_KF5)
    install( PROGRAMS umbrello.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
    install( FILES
        umbrelloui.rc
        DESTINATION ${DATA_INSTALL_DIR}/umbrello )
else()
    file(READ umbrello.desktop UMBRELLO_DESKTOP)
    string(REPLACE "-caption" "-qwindowtitle" UMBRELLO_DESKTOP_OUT "${UMBRELLO_DESKTOP}")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/umbrello.desktop "${UMBRELLO_DESKTOP_OUT}")
    install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/umbrello.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} RENAME org.kde.umbrello.desktop)
    install( FILES org.kde.umbrello.appdata.xml  DESTINATION  ${CMAKE_INSTALL_METAINFODIR})
    install( FILES
        umbrelloui.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/umbrello )
endif()

install( FILES
    docgenerators/xmi2docbook.xsl
    docgenerators/docbook2xhtml.xsl
    docgenerators/xmi.css
    docgenerators/common.ent
    DESTINATION ${DATA_INSTALL_DIR}/umbrello )

install( DIRECTORY
    layouts
    DESTINATION ${DATA_INSTALL_DIR}/umbrello
    PATTERN .svn EXCLUDE )
