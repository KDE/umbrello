/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "uml.h"

// app includes
#include "umldoc.h"
#include "umllistview.h"
#include "umlviewlist.h"
#include "worktoolbar.h"
#include "umlviewimageexporter.h"
#include "umlviewimageexporterall.h"
#include "docwindow.h"
#include "optionstate.h"
#include "cmdlineexportallviewsevent.h"
#include "cmds.h"
#include "umbrellosettings.h"
#include "statusbartoolbutton.h"
// code generation
#include "codegenerator.h"
#include "codegenerationpolicy.h"
#include "codegenfactory.h"
#include "codegenpolicyext.h"
#include "simplecodegenerator.h"
// utils
#include "debug_utils.h"
#include "widget_utils.h"
#include "icon_utils.h"
// dialogs
#include "classwizard.h"
#include "codegenerationwizard.h"
#include "codeimportingwizard.h"
#include "codeviewerdialog.h"
#include "diagramprintpage.h"
#include "settingsdlg.h"
#include "classimport.h"
#include "refactoringassistant.h"
// clipboard
#include "umlclipboard.h"
#include "umldragdata.h"
// docgenerators
#include "docbookgenerator.h"
#include "xhtmlgenerator.h"
#include "umlscene.h"

// kde includes
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <krecentfilesaction.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktip.h>
#include <ktabwidget.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kxmlguifactory.h>
#include <kapplication.h>
#include <kdeprintdialog.h>
#include <kundostack.h>
#include <kstatusbar.h>

// qt includes
#include <QtCore/QPointer>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
#include <QtGui/QClipboard>
#include <QtGui/QSlider>
#include <QtGui/QToolButton>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QMenuItem>
#include <QtGui/QDockWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrintPreviewDialog>
#include <QtGui/QUndoView>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>

#include <cmath>

/** Static pointer, holding the last created instance. */
UMLApp* UMLApp::s_instance;

/**
 * Searches for a menu with the given name.
 * @todo This is an ugly _HACK_ to allow to compile umbrello.
 *       All the menu stuff should be ported to KDE4 (using actions)
 *
 * @param name  The name of the menu to search for (name, not text)
 */
QMenu* UMLApp::findMenu(const QString& name)
{
    QWidget* widget = factory()->container(name, this);
    if (widget) {
        return dynamic_cast<QMenu*>(widget);
    }
    DEBUG(DBG_SRC) << "factory()->container(" << name << ") returns NULL";
    return NULL;
}

/**
 * Constructor. Calls all init functions to create the application.
 */
UMLApp::UMLApp(QWidget* parent)
  : KXmlGuiWindow(parent),
    m_langSelect(0),
    m_zoomSelect(0),
    m_activeLanguage(Uml::ProgrammingLanguage::Reserved),
    m_codegen(0),
    m_commoncodegenpolicy(new CodeGenerationPolicy()),
    m_policyext(0),
    m_config(KGlobal::config()),
    m_view(0),
    m_doc(new UMLDoc()),
    m_listView(0),
    m_docWindow(0),
    m_refactoringAssist(0),
    m_clipTimer(0),
    m_copyTimer(0),
    m_loading(false),
    m_imageMimeType(QString()),
    m_settingsDlg(0),
    m_imageExporterAll(new UMLViewImageExporterAll()),
    m_xhtmlGenerator(0),
    m_pUndoStack(new KUndoStack(this)),
    m_hasBegunMacro(false),
    m_printSettings(0),
    m_printer(new QPrinter())
{
    s_instance = this;
    m_doc->init();
    m_printer->setFullPage(true);

    readOptionState();
    initActions();
    // call this here because the statusBar is shown/hidden by setupGUI()
    initStatusBar();
    // use the absolute path to your umbrelloui.rc file for testing purpose in setupGUI();
    StandardWindowOptions opt = Default;
    opt &= ~Save;
    setupGUI( opt );
    initView();
    initClip();
    readOptions();

    DEBUG_REGISTER(DBG_SRC);

    //get a reference to the Code->Active Language and to the Diagram->Zoom menu
    m_langSelect = findMenu(QString("active_lang_menu") );
    //in case langSelect hasn't been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if someone takes the entry away from the ui.rc file
    if (m_langSelect == NULL) {
        m_langSelect = new QMenu( QString("active_lang_menu"), this );
    }

    m_zoomSelect = findMenu(QString("zoom_menu") );
    //in case zoomSelect hasn't been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if some one takes the entry away from the ui.rc file
    if (m_zoomSelect == NULL) {
        m_zoomSelect = new QMenu( QString("zoom_menu"), this );
    }

    //connect zoomSelect menu
    connect(m_zoomSelect,SIGNAL(aboutToShow()),this,SLOT(setupZoomMenu()));
    connect(m_zoomSelect,SIGNAL(triggered(QAction*)),this,SLOT(slotSetZoom(QAction*)));

    setAutoSaveSettings();
    m_toolsbar->setToolButtonStyle(Qt::ToolButtonIconOnly);  // too many items for text, really we want a toolbox widget
}

/**
 * Standard deconstructor.
 */
UMLApp::~UMLApp()
{
    delete m_imageExporterAll;
    delete m_clipTimer;
    delete m_copyTimer;
    delete m_refactoringAssist;
    delete m_pUndoStack;
    delete m_printer;
}

/**
 * Get the last created instance of this class.
 */
UMLApp* UMLApp::app()
{
    return s_instance;
}

/**
 * Helper method to setup the programming language action.
 */
void UMLApp::setProgLangAction(Uml::ProgrammingLanguage pl, const QString& name, const QString& action)
{
    m_langAct[pl] = actionCollection()->addAction(action);
    m_langAct[pl]->setText(name);
    m_langAct[pl]->setCheckable(true);
}

/**
 * Initializes the KActions and the status bar of the application 
 * and calls setupGUI().
 * Note: Check also the file umbrelloui.rc and add actions there too.
 */
void UMLApp::initActions()
{
    QAction* fileNew = KStandardAction::openNew(this, SLOT(slotFileNew()), actionCollection());
    QAction* fileOpen = KStandardAction::open(this, SLOT(slotFileOpen()), actionCollection());
    fileOpenRecent = KStandardAction::openRecent(this, SLOT(slotFileOpenRecent(KUrl)), actionCollection());
    QAction* fileSave = KStandardAction::save(this, SLOT(slotFileSave()), actionCollection());
    QAction* fileSaveAs = KStandardAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
    QAction* fileClose = KStandardAction::close(this, SLOT(slotFileClose()), actionCollection());
    filePrint = KStandardAction::print(this, SLOT(slotFilePrint()), actionCollection());
    printPreview = KStandardAction::printPreview(this, SLOT(slotPrintPreview()), actionCollection());
    QAction* fileQuit = KStandardAction::quit(this, SLOT(slotFileQuit()), actionCollection());

    editUndo = m_pUndoStack->createUndoAction(actionCollection());
    editRedo = m_pUndoStack->createRedoAction(actionCollection());

    disconnect( m_pUndoStack, SIGNAL(undoTextChanged(QString)), editUndo, 0 );
    disconnect( m_pUndoStack, SIGNAL(redoTextChanged(QString)), editRedo, 0 );

    editCut = KStandardAction::cut(this, SLOT(slotEditCut()), actionCollection());
    editCopy = KStandardAction::copy(this, SLOT(slotEditCopy()), actionCollection());
    editPaste = KStandardAction::paste(this, SLOT(slotEditPaste()), actionCollection());

    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);

    /* QAction* selectAll = */ KStandardAction::selectAll(this, SLOT(slotSelectAll()), actionCollection());

    QAction* fileExportDocbook = actionCollection()->addAction("file_export_docbook");
    fileExportDocbook->setText(i18n("&Export model to DocBook"));
    connect(fileExportDocbook, SIGNAL(triggered(bool)), this, SLOT(slotFileExportDocbook()));

    QAction* fileExportXhtml = actionCollection()->addAction("file_export_xhtml");
    fileExportXhtml->setText(i18n("&Export model to XHTML"));
    connect(fileExportXhtml, SIGNAL(triggered(bool)), this, SLOT(slotFileExportXhtml()));

    QAction* classWizard = actionCollection()->addAction("class_wizard");
    classWizard->setText(i18n("&New Class Wizard..."));
    connect(classWizard, SIGNAL(triggered(bool)), this, SLOT(slotClassWizard()));

    QAction* addDefDatatypes = actionCollection()->addAction("create_default_datatypes");
    addDefDatatypes->setText(i18n("&Add Default Datatypes for Active Language"));
    connect(addDefDatatypes, SIGNAL(triggered(bool)), this, SLOT(slotAddDefaultDatatypes()));

    QAction* preferences = KStandardAction::preferences(this, SLOT(slotPrefs()), actionCollection());

    QAction* impWizard = actionCollection()->addAction("importing_wizard");
    impWizard->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Import_Files));
    impWizard->setText(i18n("Code &Importing Wizard..."));
    connect(impWizard, SIGNAL(triggered(bool)), this, SLOT(slotImportingWizard()));

    QAction* genWizard = actionCollection()->addAction("generation_wizard");
    genWizard->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Export_Files));
    genWizard->setText(i18n("&Code Generation Wizard..."));
    connect(genWizard, SIGNAL(triggered(bool)), this, SLOT(slotExecGenerationWizard()));

    QAction* genAll = actionCollection()->addAction("generate_all");
    genAll->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Export_Files));
    genAll->setText(i18n("&Generate All Code"));
    connect(genAll, SIGNAL(triggered(bool)), this, SLOT(slotGenerateAllCode()));

    setProgLangAction(Uml::ProgrammingLanguage::ActionScript, "ActionScript",    "setLang_actionscript");
    setProgLangAction(Uml::ProgrammingLanguage::Ada,          "Ada",             "setLang_ada");
    setProgLangAction(Uml::ProgrammingLanguage::Cpp,          "C++",             "setLang_cpp");
    setProgLangAction(Uml::ProgrammingLanguage::CSharp,       "C#",              "setLang_csharp");
    setProgLangAction(Uml::ProgrammingLanguage::D,            "D",               "setLang_d");
    setProgLangAction(Uml::ProgrammingLanguage::IDL,          "IDL",             "setLang_idl");
    setProgLangAction(Uml::ProgrammingLanguage::Java,         "Java",            "setLang_java");
    setProgLangAction(Uml::ProgrammingLanguage::JavaScript,   "JavaScript",      "setLang_javascript");
    setProgLangAction(Uml::ProgrammingLanguage::MySQL,        "MySQL (SQL)",     "setLang_mysql");
    setProgLangAction(Uml::ProgrammingLanguage::Pascal,       "Pascal",          "setLang_pascal");
    setProgLangAction(Uml::ProgrammingLanguage::Perl,         "Perl",            "setLang_perl");
    setProgLangAction(Uml::ProgrammingLanguage::PHP,          "PHP",             "setLang_php");
    setProgLangAction(Uml::ProgrammingLanguage::PHP5,         "PHP5",            "setLang_php5");
    setProgLangAction(Uml::ProgrammingLanguage::PostgreSQL,   "PostgreSQL(SQL)", "setLang_postgresql");
    setProgLangAction(Uml::ProgrammingLanguage::Python,       "Python",          "setLang_python");
    setProgLangAction(Uml::ProgrammingLanguage::Ruby,         "Ruby",            "setLang_ruby");
    setProgLangAction(Uml::ProgrammingLanguage::SQL,          "SQL",             "setLang_sql");
    setProgLangAction(Uml::ProgrammingLanguage::Tcl,          "Tcl",             "setLang_tcl");
    setProgLangAction(Uml::ProgrammingLanguage::Vala,         "Vala",            "setLang_vala");
    setProgLangAction(Uml::ProgrammingLanguage::XMLSchema,    "XMLSchema",       "setLang_xmlschema");

    connect(m_langAct[Uml::ProgrammingLanguage::ActionScript], SIGNAL(triggered()), this, SLOT(setLang_actionscript()));
    connect(m_langAct[Uml::ProgrammingLanguage::Ada],          SIGNAL(triggered()), this, SLOT(setLang_ada()));
    connect(m_langAct[Uml::ProgrammingLanguage::Cpp],          SIGNAL(triggered()), this, SLOT(setLang_cpp()));
    connect(m_langAct[Uml::ProgrammingLanguage::CSharp],       SIGNAL(triggered()), this, SLOT(setLang_csharp()));
    connect(m_langAct[Uml::ProgrammingLanguage::D],            SIGNAL(triggered()), this, SLOT(setLang_d()));
    connect(m_langAct[Uml::ProgrammingLanguage::IDL],          SIGNAL(triggered()), this, SLOT(setLang_idl()));
    connect(m_langAct[Uml::ProgrammingLanguage::Java],         SIGNAL(triggered()), this, SLOT(setLang_java()));
    connect(m_langAct[Uml::ProgrammingLanguage::JavaScript],   SIGNAL(triggered()), this, SLOT(setLang_javascript()));
    connect(m_langAct[Uml::ProgrammingLanguage::MySQL],        SIGNAL(triggered()), this, SLOT(setLang_mysql()));
    connect(m_langAct[Uml::ProgrammingLanguage::Pascal],       SIGNAL(triggered()), this, SLOT(setLang_pascal()));
    connect(m_langAct[Uml::ProgrammingLanguage::Perl],         SIGNAL(triggered()), this, SLOT(setLang_perl()));
    connect(m_langAct[Uml::ProgrammingLanguage::PHP],          SIGNAL(triggered()), this, SLOT(setLang_php()));
    connect(m_langAct[Uml::ProgrammingLanguage::PHP5],         SIGNAL(triggered()), this, SLOT(setLang_php5()));
    connect(m_langAct[Uml::ProgrammingLanguage::PostgreSQL],   SIGNAL(triggered()), this, SLOT(setLang_postgresql()));
    connect(m_langAct[Uml::ProgrammingLanguage::Python],       SIGNAL(triggered()), this, SLOT(setLang_python()));
    connect(m_langAct[Uml::ProgrammingLanguage::Ruby],         SIGNAL(triggered()), this, SLOT(setLang_ruby()));
    connect(m_langAct[Uml::ProgrammingLanguage::SQL],          SIGNAL(triggered()), this, SLOT(setLang_sql()));
    connect(m_langAct[Uml::ProgrammingLanguage::Tcl],          SIGNAL(triggered()), this, SLOT(setLang_tcl()));
    connect(m_langAct[Uml::ProgrammingLanguage::Vala],         SIGNAL(triggered()), this, SLOT(setLang_vala()));
    connect(m_langAct[Uml::ProgrammingLanguage::XMLSchema],    SIGNAL(triggered()), this, SLOT(setLang_xmlschema()));

    fileNew->setToolTip(i18n("Creates a new document"));
    fileOpen->setToolTip(i18n("Opens an existing document"));
    fileOpenRecent->setToolTip(i18n("Opens a recently used file"));
    fileSave->setToolTip(i18n("Saves the document"));
    fileSaveAs->setToolTip(i18n("Saves the document as..."));
    fileClose->setToolTip(i18n("Closes the document"));
    filePrint->setToolTip(i18n("Prints out the document"));
    printPreview->setToolTip(i18n("Print Preview of the document"));
    fileQuit->setToolTip(i18n("Quits the application"));
    fileExportDocbook->setToolTip(i18n("Exports the model to the docbook format"));
    fileExportXhtml->setToolTip(i18n("Exports the model to the XHTML format"));
    editCut->setToolTip(i18n("Cuts the selected section and puts it to the clipboard"));
    editCopy->setToolTip(i18n("Copies the selected section to the clipboard"));
    editPaste->setToolTip(i18n("Pastes the contents of the clipboard"));
    preferences->setToolTip( i18n( "Set the default program preferences") );

    deleteSelectedWidget = actionCollection()->addAction("delete_selected");
    deleteSelectedWidget->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Delete));
    deleteSelectedWidget->setText(i18nc("delete selected widget", "Delete &Selected"));
    deleteSelectedWidget->setShortcut(QKeySequence(Qt::Key_Delete));
    connect(deleteSelectedWidget, SIGNAL(triggered(bool)), this, SLOT(slotDeleteSelectedWidget()));

    // The different views
    newDiagram = actionCollection()->add<KActionMenu>( "new_view" );
    newDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_New) );
    newDiagram->setText( "new_view" );

    QAction* classDiagram = actionCollection()->addAction( "new_class_diagram" );
    classDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Class) );
    classDiagram->setText( i18n( "&Class Diagram..." ) );
    connect(classDiagram, SIGNAL(triggered(bool)), this, SLOT(slotClassDiagram()));
    newDiagram->addAction(classDiagram);

    QAction* sequenceDiagram= actionCollection()->addAction( "new_sequence_diagram" );
    sequenceDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Sequence) );
    sequenceDiagram->setText( i18n( "&Sequence Diagram..." ) );
    connect(sequenceDiagram, SIGNAL(triggered(bool)), this, SLOT(slotSequenceDiagram()));
    newDiagram->addAction(sequenceDiagram);

    QAction* collaborationDiagram = actionCollection()->addAction( "new_collaboration_diagram" );
    collaborationDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Collaboration) );
    collaborationDiagram->setText( i18n( "C&ollaboration Diagram..." ) );
    connect(collaborationDiagram, SIGNAL(triggered(bool)), this, SLOT(slotCollaborationDiagram()));
    newDiagram->addAction(collaborationDiagram);

    QAction* useCaseDiagram = actionCollection()->addAction( "new_use_case_diagram" );
    useCaseDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Usecase) );
    useCaseDiagram->setText( i18n( "&Use Case Diagram..." ) );
    connect(useCaseDiagram, SIGNAL(triggered(bool)), this, SLOT(slotUseCaseDiagram()));
    newDiagram->addAction(useCaseDiagram);

    QAction* stateDiagram = actionCollection()->addAction( "new_state_diagram" );
    stateDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_State) );
    stateDiagram->setText( i18n( "S&tate Diagram..." ) );
    connect(stateDiagram, SIGNAL(triggered(bool)), this, SLOT(slotStateDiagram()));
    newDiagram->addAction(stateDiagram);

    QAction* activityDiagram = actionCollection()->addAction( "new_activity_diagram" );
    activityDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Activity) );
    activityDiagram->setText( i18n( "&Activity Diagram..." ) );
    connect(activityDiagram, SIGNAL(triggered(bool)), this, SLOT(slotActivityDiagram()));
    newDiagram->addAction(activityDiagram);

    QAction* componentDiagram = actionCollection()->addAction( "new_component_diagram" );
    componentDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Component) );
    componentDiagram->setText( i18n("Co&mponent Diagram...") );
    connect(componentDiagram, SIGNAL(triggered(bool)), this, SLOT(slotComponentDiagram()));
    newDiagram->addAction(componentDiagram);

    QAction* deploymentDiagram = actionCollection()->addAction( "new_deployment_diagram" );
    deploymentDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Deployment) );
    deploymentDiagram->setText( i18n("&Deployment Diagram...") );
    connect(deploymentDiagram, SIGNAL(triggered(bool)), this, SLOT(slotDeploymentDiagram()));
    newDiagram->addAction(deploymentDiagram);

    QAction* entityRelationshipDiagram = actionCollection()->addAction( "new_entityrelationship_diagram" );
    entityRelationshipDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_EntityRelationship) );
    entityRelationshipDiagram->setText( i18n("&Entity Relationship Diagram...") );
    connect(entityRelationshipDiagram, SIGNAL(triggered(bool)), this, SLOT(slotEntityRelationshipDiagram()));
    newDiagram->addAction(entityRelationshipDiagram);

    viewClearDiagram = actionCollection()->addAction( "view_clear_diagram" );
    viewClearDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Clear) );
    viewClearDiagram->setText( i18n("&Clear Diagram") );
    connect(viewClearDiagram, SIGNAL(triggered(bool)), this, SLOT(slotCurrentViewClearDiagram()));

    viewSnapToGrid = actionCollection()->add<KToggleAction>("view_snap_to_grid");
    viewSnapToGrid->setText(i18n("&Snap to Grid"));
    connect(viewSnapToGrid, SIGNAL(triggered(bool)), this, SLOT(slotCurrentViewToggleSnapToGrid()));

    viewShowGrid = actionCollection()->add<KToggleAction>("view_show_grid");
    viewShowGrid->setText(i18n("S&how Grid"));
    connect(viewShowGrid, SIGNAL(triggered(bool)), this, SLOT(slotCurrentViewToggleShowGrid()));

    deleteDiagram = actionCollection()->addAction( "view_delete" );
    deleteDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Delete) );
    deleteDiagram->setText( i18n("&Delete") );
    connect(deleteDiagram, SIGNAL(triggered(bool)), this, SLOT(slotDeleteDiagram()));

    viewExportImage = actionCollection()->addAction( "view_export_image" );
    viewExportImage->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Export_Picture) );
    viewExportImage->setText( i18n("&Export as Picture...") );
    connect(viewExportImage, SIGNAL(triggered(bool)), this, SLOT(slotCurrentViewExportImage()));

    QAction* viewExportImageAll = actionCollection()->addAction( "view_export_image_all" );
    viewExportImageAll->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Export_Picture) );
    viewExportImageAll->setText( i18n("Export &All Diagrams as Pictures...") );
    connect(viewExportImageAll, SIGNAL(triggered(bool)), this, SLOT(slotAllViewsExportImage()));

    viewProperties = actionCollection()->addAction( "view_properties" );
    viewProperties->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Properties) );
    viewProperties->setText( i18n("&Properties") );
    connect(viewProperties, SIGNAL(triggered(bool)), this, SLOT(slotCurrentViewProperties()));

    viewSnapToGrid->setChecked(false);
    viewShowGrid->setChecked(false);

    viewClearDiagram->setEnabled(false);
    viewSnapToGrid->setEnabled(false);
    viewShowGrid->setEnabled(false);
    deleteDiagram->setEnabled(false);
    viewExportImage->setEnabled(false);
    viewProperties->setEnabled(false);

    zoom100Action = actionCollection()->addAction("zoom100");
    zoom100Action->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Zoom_100));
    zoom100Action->setText(i18n("Z&oom to 100%"));
    connect(zoom100Action, SIGNAL(triggered(bool)), this, SLOT(slotZoom100()));

    QAction* alignRight = actionCollection()->addAction( "align_right" );
    alignRight->setText(i18n("Align Right" ));
    alignRight->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Right) );
    connect(alignRight, SIGNAL(triggered(bool)), this, SLOT(slotAlignRight()));

    QAction* alignLeft = actionCollection()->addAction( "align_left" );
    alignLeft->setText(i18n("Align Left" ));
    alignLeft->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Left) );
    connect(alignLeft, SIGNAL(triggered(bool)), this, SLOT(slotAlignLeft()));

    QAction* alignTop = actionCollection()->addAction( "align_top" );
    alignTop->setText(i18n("Align Top" ));
    alignTop->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Top) );
    connect(alignTop, SIGNAL(triggered(bool)), this, SLOT(slotAlignTop()));

    QAction* alignBottom = actionCollection()->addAction( "align_bottom" );
    alignBottom->setText(i18n("Align Bottom" ));
    alignBottom->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Bottom) );
    connect(alignBottom, SIGNAL(triggered(bool)), this, SLOT(slotAlignBottom()));

    QAction* alignVerticalMiddle = actionCollection()->addAction( "align_vertical_middle" );
    alignVerticalMiddle->setText(i18n("Align Vertical Middle" ));
    alignVerticalMiddle->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_VerticalMiddle) );
    connect(alignVerticalMiddle, SIGNAL(triggered(bool)), this, SLOT(slotAlignVerticalMiddle()));

    QAction* alignHorizontalMiddle = actionCollection()->addAction( "align_horizontal_middle" );
    alignHorizontalMiddle->setText(i18n("Align Horizontal Middle" ));
    alignHorizontalMiddle->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_HorizontalMiddle) );
    connect(alignHorizontalMiddle, SIGNAL(triggered(bool)), this, SLOT(slotAlignHorizontalMiddle()));

    QAction* alignVerticalDistribute = actionCollection()->addAction( "align_vertical_distribute" );
    alignVerticalDistribute->setText(i18n("Align Vertical Distribute" ));
    alignVerticalDistribute->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_VerticalDistribute) );
    connect(alignVerticalDistribute, SIGNAL(triggered(bool)), this, SLOT(slotAlignVerticalDistribute()));

    QAction* alignHorizontalDistribute = actionCollection()->addAction( "align_horizontal_distribute" );
    alignHorizontalDistribute->setText(i18n("Align Horizontal Distribute" ));
    alignHorizontalDistribute->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_HorizontalDistribute) );
    connect(alignHorizontalDistribute, SIGNAL(triggered(bool)), this, SLOT(slotAlignHorizontalDistribute()));

    QString moveTabLeftString = i18n("&Move Tab Left");
    QString moveTabRightString = i18n("&Move Tab Right");
    KAction* moveTabLeft = actionCollection()->addAction("move_tab_left");
    moveTabLeft->setIcon(Icon_Utils::SmallIcon(QApplication::layoutDirection() ? Icon_Utils::it_Go_Next : Icon_Utils::it_Go_Previous));
    moveTabLeft->setText(QApplication::layoutDirection() ? moveTabRightString : moveTabLeftString);
    moveTabLeft->setShortcut(QApplication::layoutDirection() ?
                 QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Right) : QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Left));
    connect(moveTabLeft, SIGNAL(triggered(bool)), this, SLOT(slotMoveTabLeft()));

    KAction* moveTabRight = actionCollection()->addAction("move_tab_right");
    moveTabRight->setIcon(Icon_Utils::SmallIcon(QApplication::layoutDirection() ? Icon_Utils::it_Go_Previous : Icon_Utils::it_Go_Next));
    moveTabRight->setText(QApplication::layoutDirection() ? moveTabLeftString : moveTabRightString);
    moveTabRight->setShortcut(QApplication::layoutDirection() ?
                  QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Left) : QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Right));
    connect(moveTabRight, SIGNAL(triggered(bool)), this, SLOT(slotMoveTabRight()));

    QString selectTabLeftString = i18n("Select Diagram on Left");
    QString selectTabRightString = i18n("Select Diagram on Right");
    KAction* changeTabLeft = actionCollection()->addAction("previous_tab");
    changeTabLeft->setText(QApplication::layoutDirection() ? selectTabRightString : selectTabLeftString);
    changeTabLeft->setShortcut(QApplication::layoutDirection() ?
                   QKeySequence(Qt::SHIFT+Qt::Key_Right) : QKeySequence(Qt::SHIFT+Qt::Key_Left));
    connect(changeTabLeft, SIGNAL(triggered(bool)), this, SLOT(slotChangeTabLeft()));

    KAction* changeTabRight = actionCollection()->addAction("next_tab");
    changeTabRight->setText(QApplication::layoutDirection() ? selectTabLeftString : selectTabRightString);
    changeTabRight->setShortcut(QApplication::layoutDirection() ?
                    QKeySequence(Qt::SHIFT+Qt::Key_Left) : QKeySequence(Qt::SHIFT+Qt::Key_Right));
    connect(changeTabRight, SIGNAL(triggered(bool)), this, SLOT(slotChangeTabRight()));

// @todo Check if this should be ported
//     QMenu* menu = findMenu(QString("settings") );
//     menu->insertItem(i18n("&Windows"), dockHideShowMenu(), -1, 0);

    // disable actions at startup
    fileSave->setEnabled(true);
    fileSaveAs->setEnabled(true);
    enablePrint(false);
    editCut->setEnabled(false);
    editCopy->setEnabled(false);
    editPaste->setEnabled(false);
    editUndo->setEnabled(false);
    editRedo->setEnabled(false);
}

/**
 * Connected to by the  zoomAction, a value of between 300
 * and 2200 is scaled to zoom to between 9% and 525%.
 * The min and max values of the slider are hard coded in the statusbar slider.
 */
void UMLApp::slotZoomSliderMoved(int value)
{
    setZoom(value);
}

/**
 * Set zoom to fit the view.
 */
void UMLApp::slotZoomFit()
{
    currentView()->setZoom(100);  // bring it first to the original values
    DEBUG(DBG_SRC) << "canvas width=" << currentView()->getCanvasWidth()
                   << ", height=" << currentView()->getCanvasHeight();
    DEBUG(DBG_SRC) << "visible width=" << currentView()->visibleWidth()
                   << ", height=" << currentView()->visibleHeight();
    int scaleW = ceil(100.0 * currentView()->visibleWidth() / currentView()->getCanvasWidth());
    int scaleH = ceil(100.0 * currentView()->visibleHeight() / currentView()->getCanvasHeight());
    DEBUG(DBG_SRC) << "scale width: " << scaleW << ", height: " << scaleH;
    int scale = 100;
    if (scaleW < scaleH) {
        scale = scaleW;
    }
    else {
        scale = scaleH;
    }
    setZoom(scale);
}

/**
 * Set zoom to 100%.
 */
void UMLApp::slotZoom100()
{
    setZoom(100);
}

/**
 * Decrease the zoom factor of the current diagram.
 */
void UMLApp::slotZoomOut()
{
    setZoom(currentView()->zoom()-5);
}

/**
 * Increase the zoom factor of the current diagram.
 */
void UMLApp::slotZoomIn()
{
    setZoom(currentView()->zoom()+5);
}

/**
 * Set the zoom factor of the current diagram.
 *
 * @param zoom  Zoom factor in percentage.
 */
void UMLApp::setZoom(int zoom)
{
    currentView()->setZoom(zoom);
    m_pZoomSlider->setValue(zoom);
    m_zoomValueLbl->setText(QString::number(zoom) + '%');
}

/**
 * Set the zoom factor of the current diagram.
 *
 * @param action  Action which is called.
 */
void UMLApp::slotSetZoom(QAction* action)
{
    QVariant var = action->data();
    if (var.canConvert<int>()) {
        setZoom(var.toInt());
    }
}

/**
 * Helper method to create the zoom actions.
 */
QAction* UMLApp::createZoomAction(int zoom, int currentZoom)
{
    //IMPORTANT: The zoom value is added to the action.
    QAction* action = new QAction(this);
    action->setCheckable(true);
    action->setText(" &" + QString::number(zoom) + '%');
    action->setData(zoom);
    if (zoom == currentZoom) {
        action->setChecked(true);
    }
    return action;
}

/**
 * Prepares the zoom menu for display.
 */
void UMLApp::setupZoomMenu()
{
    m_zoomSelect->clear();

    int currentZoom = currentView()->currentZoom();

    m_zoomSelect->addAction(createZoomAction(33, currentZoom));
    m_zoomSelect->addAction(createZoomAction(50, currentZoom));
    m_zoomSelect->addAction(createZoomAction(75, currentZoom));
    m_zoomSelect->addAction(createZoomAction(100, currentZoom));
    m_zoomSelect->addAction(createZoomAction(150, currentZoom));
    m_zoomSelect->addAction(createZoomAction(200, currentZoom));
    m_zoomSelect->addAction(createZoomAction(300, currentZoom));

    // if currentZoom is not a "standard zoom" (because of zoom in / zoom out step
    // we add it for information
    switch (currentZoom) {
    case 33:
    case 50:
    case 75:
    case 100:
    case 150:
    case 200:
    case 300:
        break;
    default:
        m_zoomSelect->addSeparator();
        m_zoomSelect->addAction(createZoomAction(currentZoom, currentZoom));
    }
}

/**
 * Sets up the statusbar for the main window by
 * initialzing a statuslabel.
 */
void UMLApp::initStatusBar()
{
    connect(m_doc, SIGNAL(sigWriteToStatusBar(QString)), this, SLOT(slotStatusMsg(QString)));

    m_statusBarMessage = new QLabel(i18nc("init status bar", "Ready"));
    statusBar()->addWidget(m_statusBarMessage);

    QWidget* defaultZoomWdg = new QWidget(this);
    QHBoxLayout* zoomLayout = new QHBoxLayout(defaultZoomWdg);
    zoomLayout->setContentsMargins(0,0,0,0);
    zoomLayout->setSpacing(0);
    zoomLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    m_zoomValueLbl = new QLabel("100%");
    m_zoomValueLbl->setContentsMargins(10, 0, 10, 0);
    zoomLayout->addWidget(m_zoomValueLbl);

    m_pZoomFitSBTB = new StatusBarToolButton(this);
    m_pZoomFitSBTB->setText("Fit");
    m_pZoomFitSBTB->setGroupPosition(StatusBarToolButton::GroupLeft);
    zoomLayout->addWidget(m_pZoomFitSBTB);
    m_pZoomFitSBTB->setContentsMargins(0,0,0,0);
    //m_pZoomFitSBTB->setDisabled(true);
    connect(m_pZoomFitSBTB, SIGNAL(clicked()), this, SLOT(slotZoomFit()));

    m_pZoomFullSBTB = new StatusBarToolButton(this);
    m_pZoomFullSBTB->setText("100%");
    m_pZoomFullSBTB->setGroupPosition(StatusBarToolButton::GroupRight);
    m_pZoomFullSBTB->setContentsMargins(0,0,0,0);
    zoomLayout->addWidget(m_pZoomFullSBTB);
    connect(m_pZoomFullSBTB, SIGNAL(clicked()), this, SLOT(slotZoom100()));

    statusBar()->addPermanentWidget(defaultZoomWdg);

    m_pZoomOutPB = new QPushButton(this);
    m_pZoomOutPB->setIcon(KIcon("zoom-out"));
    m_pZoomOutPB->setFlat(true);
    m_pZoomOutPB->setMaximumSize(30,30);
    statusBar()->addPermanentWidget(m_pZoomOutPB);
    connect(m_pZoomOutPB, SIGNAL(clicked()), this, SLOT(slotZoomOut()));

    m_pZoomSlider = new QSlider(Qt::Horizontal, this);
    m_pZoomSlider->setMaximumSize(100,50);
    m_pZoomSlider->setMinimum (20);
    m_pZoomSlider->setMaximum (480);
    //m_pZoomSlider->setPageStep (1000);
    m_pZoomSlider->setValue (100);
    m_pZoomSlider->setContentsMargins(0,0,0,0);
    connect(m_pZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(slotZoomSliderMoved(int)));

    statusBar()->addPermanentWidget(m_pZoomSlider);

    m_pZoomInPB = new QPushButton(this);
    m_pZoomInPB->setIcon(KIcon("zoom-in"));
    m_pZoomInPB->setFlat(true);
    m_pZoomInPB->setMaximumSize(30,30);
    statusBar()->addPermanentWidget(m_pZoomInPB);
    connect(m_pZoomInPB, SIGNAL(clicked()), this, SLOT(slotZoomIn()));
}

/**
 * Creates the centerwidget of the KMainWindow instance and
 * sets it as the view.
 */
void UMLApp::initView()
{
    setCaption(m_doc->url().fileName(),false);
    m_view = 0;
    m_toolsbar = new WorkToolBar(this);
    m_toolsbar->setWindowTitle(i18n("Diagram Toolbar"));
    addToolBar(Qt::TopToolBarArea, m_toolsbar);

//     m_mainDock = new QDockWidget( this );
//     addDockWidget ( Qt::RightDockWidgetArea, m_mainDock );
    m_newSessionButton = NULL;
    m_diagramMenu = NULL;

    // Prepare Stacked Diagram Representation
    m_viewStack = new QStackedWidget(this);

    // Prepare Tabbed Diagram Representation
    m_tabWidget = new KTabWidget(this);
    m_tabWidget->setAutomaticResizeTabs(true);
    m_tabWidget->setTabsClosable(true);
    connect(m_tabWidget, SIGNAL(closeRequest(QWidget*)), SLOT(slotCloseDiagram(QWidget*)));

    m_newSessionButton = new QToolButton(m_tabWidget);
    m_newSessionButton->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Tab_New));
    m_newSessionButton->adjustSize();
    m_newSessionButton->setAutoRaise(true);
    m_newSessionButton->setPopupMode(QToolButton::InstantPopup);
    m_newSessionButton->setMenu(newDiagram->menu());

    connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)), SLOT(slotTabChanged(QWidget*)));
    connect(m_tabWidget, SIGNAL(contextMenu(QWidget*,const QPoint&)), m_doc, SLOT(slotDiagramPopupMenu(QWidget*,const QPoint&)));
    m_tabWidget->setCornerWidget( m_newSessionButton, Qt::TopLeftCorner );
    m_newSessionButton->installEventFilter(this);

    m_layout = new QVBoxLayout;
    m_layout->setMargin(0);
    if (Settings::optionState().generalState.tabdiagrams) {
        // Tabbed Diagram Representation
        m_layout->addWidget(m_tabWidget);
        m_viewStack->hide();
    }
    else {
        // Stacked Diagram Representation
        m_layout->addWidget(m_viewStack);
        m_tabWidget->hide();
    }

    QWidget *widget = new QWidget;
    widget->setLayout(m_layout);
    setCentralWidget(widget);

    // create the tree viewer
    m_listDock = new QDockWidget( i18n("&Tree View"), this );
    m_listDock->setObjectName("TreeViewDock");
    addDockWidget(Qt::LeftDockWidgetArea, m_listDock);
    m_listView = new UMLListView(m_listDock);
    //m_listView->setSorting(-1);
    m_listView->setDocument(m_doc);
    m_listView->init();
    m_listDock->setWidget(m_listView);

    // create the documentation viewer
    m_documentationDock = new QDockWidget( i18n("Doc&umentation"), this );
    m_documentationDock->setObjectName("DocumentationDock");
    addDockWidget(Qt::LeftDockWidgetArea, m_documentationDock);
    m_docWindow = new DocWindow(m_doc, m_documentationDock);
    m_docWindow->setObjectName("DOCWINDOW");
    m_documentationDock->setWidget(m_docWindow);

    m_doc->setupSignals(); // make sure gets signal from list view

    // create the command history viewer
    m_cmdHistoryDock = new QDockWidget(i18n("Co&mmand history"), this);
    m_cmdHistoryDock->setObjectName("CmdHistoryDock");
    addDockWidget(Qt::LeftDockWidgetArea, m_cmdHistoryDock);
    m_pQUndoView = new QUndoView(m_cmdHistoryDock);
    m_pQUndoView->setCleanIcon(Icon_Utils::SmallIcon(Icon_Utils::it_UndoView));
    m_pQUndoView->setStack(m_pUndoStack);
    m_cmdHistoryDock->setWidget(m_pQUndoView);

    // create the property viewer
    //m_propertyDock = new QDockWidget(i18n("&Properties"), this);
    //m_propertyDock->setObjectName("PropertyDock");
    //addDockWidget(Qt::LeftDockWidgetArea, m_propertyDock);  //:TODO:

    tabifyDockWidget(m_documentationDock, m_cmdHistoryDock);
    //tabifyDockWidget(m_cmdHistoryDock, m_propertyDock);  //:TODO:
}

/**
 * Opens a file specified by commandline option.
 */
void UMLApp::openDocumentFile(const KUrl& url)
{
    slotStatusMsg(i18n("Opening file..."));

    m_doc->openDocument( url);
    fileOpenRecent->addUrl( url );
    resetStatusMsg();
    setCaption(m_doc->url().fileName(), false);
    enablePrint(true);
}

/**
 * Returns a pointer to the current document connected to the
 * KMainWindow instance.
 * Used by the View class to access the document object's methods.
 */
UMLDoc *UMLApp::document() const
{
    return m_doc;
}

/**
 * Returns a pointer to the list view.
 *
 * @return  The listview being used.
 */
UMLListView* UMLApp::listView() const
{
    return m_listView;
}

/**
 * Save general Options like all bar positions and status
 * as well as the geometry and the recent file list to
 * the configuration file.
 */
void UMLApp::saveOptions()
{
    // The Toolbar settings will be handled by the respective classes ( KToolBar )
    KConfigGroup cg( m_config, "toolbar" );
    toolBar("mainToolBar")->saveSettings( cg );
    KConfigGroup workBarConfig(m_config, "workbar" );
    m_toolsbar->saveSettings(workBarConfig );
    fileOpenRecent->saveEntries( m_config->group( "Recent Files") );

    UmbrelloSettings::setGeometry( size() );

    Settings::OptionState& optionState = Settings::optionState();

    UmbrelloSettings::setUndo( optionState.generalState.undo );
    UmbrelloSettings::setTabdiagrams( optionState.generalState.tabdiagrams );
    UmbrelloSettings::setNewcodegen( optionState.generalState.newcodegen );
    UmbrelloSettings::setAngularlines( optionState.generalState.angularlines );
    UmbrelloSettings::setFooterPrinting( optionState.generalState.footerPrinting );
    UmbrelloSettings::setAutosave( optionState.generalState.autosave );
    UmbrelloSettings::setTime(  optionState.generalState.time );
    UmbrelloSettings::setAutosavetime( optionState.generalState.autosavetime );
    UmbrelloSettings::setAutosavesuffix(  optionState.generalState.autosavesuffix );
    UmbrelloSettings::setLoadlast( optionState.generalState.loadlast );

    UmbrelloSettings::setDiagram( optionState.generalState.diagram );
    UmbrelloSettings::setDefaultLanguage(  optionState.generalState.defaultLanguage );

    if( m_doc->url().fileName() == i18n( "Untitled" ) ) {
        UmbrelloSettings::setLastFile(  "" );
    }
    else {
        UmbrelloSettings::setLastFile(  m_doc->url().prettyUrl() );
    }

    UmbrelloSettings::setImageMimeType(  imageMimeType() );

    UmbrelloSettings::setUseFillColor(  optionState.uiState.useFillColor );
    UmbrelloSettings::setFillColor(  optionState.uiState.fillColor );
    UmbrelloSettings::setLineColor(  optionState.uiState.lineColor );
    UmbrelloSettings::setLineWidth(  optionState.uiState.lineWidth );
    UmbrelloSettings::setTextColor( optionState.uiState.textColor );
    UmbrelloSettings::setShowDocWindow(  m_documentationDock->isVisible() );
    UmbrelloSettings::setUiFont(  optionState.uiState.font );
    UmbrelloSettings::setBackgroundColor( optionState.uiState.backgroundColor );
    UmbrelloSettings::setGridDotColor( optionState.uiState.gridDotColor );

    UmbrelloSettings::setShowVisibility(  optionState.classState.showVisibility );
    UmbrelloSettings::setShowAtts( optionState.classState.showAtts);
    UmbrelloSettings::setShowOps(  optionState.classState.showOps );
    UmbrelloSettings::setShowStereoType( optionState.classState.showStereoType );
    UmbrelloSettings::setShowAttSig(  optionState.classState.showAttSig );
    UmbrelloSettings::setShowOpSig( optionState.classState.showOpSig );
    UmbrelloSettings::setShowPackage(  optionState.classState.showPackage );
    UmbrelloSettings::setShowAttribAssocs(  optionState.classState.showAttribAssocs );
    UmbrelloSettings::setShowPublicOnly(  optionState.classState.showPublicOnly );
    UmbrelloSettings::setDefaultAttributeScope( optionState.classState.defaultAttributeScope);
    UmbrelloSettings::setDefaultOperationScope( optionState.classState.defaultOperationScope);

    UmbrelloSettings::setHeight(  optionState.codeViewerState.height );
    UmbrelloSettings::setWidth(  optionState.codeViewerState.width);
    UmbrelloSettings::setCodeViewerFont(  optionState.codeViewerState.font);
    UmbrelloSettings::setFontColor(  optionState.codeViewerState.fontColor);
    UmbrelloSettings::setPaperColor( optionState.codeViewerState.paperColor);
    UmbrelloSettings::setSelectedColor(  optionState.codeViewerState.selectedColor);
    UmbrelloSettings::setEditBlockColor( optionState.codeViewerState.editBlockColor);
    UmbrelloSettings::setNonEditBlockColor(  optionState.codeViewerState.nonEditBlockColor);
    UmbrelloSettings::setUmlObjectColor( optionState.codeViewerState.umlObjectColor);
    UmbrelloSettings::setBlocksAreHighlighted(  optionState.codeViewerState.blocksAreHighlighted);
    UmbrelloSettings::setShowHiddenBlocks( optionState.codeViewerState.showHiddenBlocks);
    UmbrelloSettings::setHiddenColor(  optionState.codeViewerState.hiddenColor);

    // write config for CPP code generation options
    UmbrelloSettings::setAutoGenAccessors( optionState.codeGenerationState.cppCodeGenerationState.autoGenAccessors);

    UmbrelloSettings::setInlineAccessors(optionState.codeGenerationState.cppCodeGenerationState.inlineAccessors);
    UmbrelloSettings::setPublicAccessors( optionState.codeGenerationState.cppCodeGenerationState.publicAccessors);
    UmbrelloSettings::setInlineOps(optionState.codeGenerationState.cppCodeGenerationState.inlineOps);
    UmbrelloSettings::setVirtualDestructors( optionState.codeGenerationState.cppCodeGenerationState.virtualDestructors);
    UmbrelloSettings::setPackageIsNamespace(optionState.codeGenerationState.cppCodeGenerationState.packageIsNamespace);

    UmbrelloSettings::setStringClassName(optionState.codeGenerationState.cppCodeGenerationState.stringClassName);
    UmbrelloSettings::setStringClassNameInclude(optionState.codeGenerationState.cppCodeGenerationState.stringClassNameInclude);
    UmbrelloSettings::setStringIncludeIsGlobal(optionState.codeGenerationState.cppCodeGenerationState.stringIncludeIsGlobal);

    UmbrelloSettings::setVectorClassName(optionState.codeGenerationState.cppCodeGenerationState.vectorClassName);
    UmbrelloSettings::setVectorClassNameInclude(optionState.codeGenerationState.cppCodeGenerationState.vectorClassNameInclude);
    UmbrelloSettings::setVectorIncludeIsGlobal(optionState.codeGenerationState.cppCodeGenerationState.vectorIncludeIsGlobal);
    UmbrelloSettings::setDocToolTag(optionState.codeGenerationState.cppCodeGenerationState.docToolTag);

    // write config for Java code generation options
    UmbrelloSettings::setAutoGenerateAttributeAccessorsJava(optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAttributeAccessors);
    UmbrelloSettings::setAutoGenerateAssocAccessorsJava(optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAssocAccessors);

//     CodeGenerator *codegen = getGenerator();
//     JavaCodeGenerator *javacodegen = dynamic_cast<JavaCodeGenerator*>(codegen);
//     if (javacodegen)
//         UmbrelloSettings::setBuildANTDocumentJava( javacodegen->getCreateANTBuildFile());

    // write config for D code generation options
    UmbrelloSettings::setAutoGenerateAttributeAccessorsD( optionState.codeGenerationState.dCodeGenerationState.autoGenerateAttributeAccessors);
    UmbrelloSettings::setAutoGenerateAssocAccessorsD( optionState.codeGenerationState.dCodeGenerationState.autoGenerateAssocAccessors);

    // write config for Ruby code generation options
    UmbrelloSettings::setAutoGenerateAttributeAccessorsRuby( optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAttributeAccessors);
    UmbrelloSettings::setAutoGenerateAssocAccessorsRuby( optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAssocAccessors);

    // now write the basic defaults to config
    m_commoncodegenpolicy->writeConfig();

    UmbrelloSettings::setCreateArtifacts(optionState.codeImportState.createArtifacts);

    UmbrelloSettings::self()->writeConfig();
}

/**
 * Read general Options again and initialize all variables
 * like the recent file list.
 */
void UMLApp::readOptions()
{
    // bar status settings
    toolBar("mainToolBar")->applySettings(m_config->group( "toolbar") );
    // do config for work toolbar
    m_toolsbar->applySettings(m_config->group( "workbar") );
    fileOpenRecent->loadEntries(m_config->group( "Recent Files") );
    setImageMimeType( UmbrelloSettings::imageMimeType() );
    resize( UmbrelloSettings::geometry());
}

/**
 * Saves the window properties for each open window
 * during session end to the session config file,
 * including saving the currently opened file by a
 * temporary filename provided by KApplication.
 * @see KMainWindow#saveProperties
 */
void UMLApp::saveProperties(KConfigGroup & cfg)
{
    DEBUG(DBG_SRC) << "******************* commented out - UNUSED?";
    Q_UNUSED(cfg);
/*
    if (m_doc->url().fileName() != i18n("Untitled") && !m_doc->isModified()) {
        // saving to tempfile not necessary
    } else {
        KUrl url = m_doc->url();
        cfg.writePathEntry("filename", url.url());
        cfg.writeEntry("modified", m_doc->isModified());
        QString tempname = kapp->tempSaveName(url.url());  //:TODO: change this - deprecated
        QString tempurl = KUrl::toPercentEncoding(tempname);

        KUrl _url(tempurl);
        m_doc->saveDocument(_url);
    }
*/
}

/**
 * Reads the session config file and restores the
 * application's state including the last opened files and
 * documents by reading the temporary files saved by
 * saveProperties()
 * @see KMainWindow#readProperties
 */
void UMLApp::readProperties(const KConfigGroup & cfg)     //:TODO: applyMainWindowSettings(const KConfigGroup& config, bool force = false)
{
    DEBUG(DBG_SRC) << "******************* commented out - UNUSED?";
    Q_UNUSED(cfg);
/*
    QString filename = cfg.readPathEntry("filename", QString());
    KUrl url(filename);
    bool modified = cfg.readEntry("modified", false);
    if (modified) {
        bool canRecover;
        QString tempname = kapp->checkRecoverFile(filename, canRecover);
        KUrl _url(tempname);

        if (canRecover) {
            m_doc->openDocument(_url);
            m_doc->setModified();
            enablePrint(true);
            setCaption(_url.fileName(),true);
            QFile::remove(tempname);
        } else {
            enablePrint(false);
        }
    } else {
        if (!filename.isEmpty()) {
            m_doc->openDocument(url);
            enablePrint(true);
            setCaption(url.fileName(),false);

        } else {
            enablePrint(false);
        }
    }
*/
}

/**
 * queryClose is called by KMainWindow on each closeEvent of a
 * window. Counter to the default implementation (which only
 * returns true), this calls saveModified() on the document object
 * to ask if the document shall be saved if Modified; on cancel
 * the closeEvent is rejected.
 * @see KMainWindow#queryClose
 * @see KMainWindow#closeEvent
 *
 * @return  True if window may be closed.
 */
bool UMLApp::queryClose()
{
    return m_doc->saveModified();
}

/**
 * queryExit is called by KMainWindow when the last
 * window of the application is going to be closed during
 * the closeEvent().  In contrast to the default
 * implementation that just returns true, this calls
 * saveOptions() to save the settings of the last
 * window's properties.
 * @see KMainWindow#queryExit
 * @see KMainWindow#closeEvent
 *
 * @return  True if window may be closed.
 */
bool UMLApp::queryExit()
{
    saveOptions();
    m_doc->closeDocument();
    return true;
}

/**
 * Clears the document in the actual view to reuse it as the new
 * document.
 */
void UMLApp::slotFileNew()
{
    slotStatusMsg(i18n("Creating new document..."));
    if (m_doc->saveModified()) {
        setDiagramMenuItemsState(false);
        m_doc->newDocument();
        setCaption(m_doc->url().fileName(), false);
        fileOpenRecent->setCurrentItem( -1 );
        setModified(false);
        enablePrint(false);
    }
    slotUpdateViews();
    resetStatusMsg();
}

/**
 * Open a file and load it into the document.
 */
void UMLApp::slotFileOpen()
{
    slotStatusMsg(i18n("Opening file..."));
    m_loading = true;

    if (!m_doc->saveModified()) {
        // here saving wasn't successful
    } 
    else {
        KUrl url=KFileDialog::getOpenUrl(KUrl(),
            i18n("*.xmi *.xmi.tgz *.xmi.tar.bz2 *.mdl|All Supported Files (*.xmi, *.xmi.tgz, *.xmi.tar.bz2, *.mdl)\n"
                 "*.xmi|Uncompressed XMI Files (*.xmi)\n"
                 "*.xmi.tgz|Gzip Compressed XMI Files (*.xmi.tgz)\n"
                 "*.xmi.tar.bz2|Bzip2 Compressed XMI Files (*.xmi.tar.bz2)\n"
                 "*.mdl|Rose model files"), this, i18n("Open File"));
        if (!url.isEmpty()) {
            m_listView->setSortingEnabled(false);
            if (m_doc->openDocument(url)) {
                fileOpenRecent->addUrl( url );
            }
            enablePrint(true);
            setCaption(m_doc->url().fileName(), false);
            m_listView->setSortingEnabled(true);
        }
    }
    slotUpdateViews();
    m_loading = false;
    resetStatusMsg();
}

/**
 * Opens a file from the recent files menu.
 */
void UMLApp::slotFileOpenRecent(const KUrl& url)
{
    slotStatusMsg(i18n("Opening file..."));
    m_loading = true;

    KUrl oldUrl = m_doc->url();

    if (!m_doc->saveModified()) {
        // here saving wasn't successful
    }
    else {
        if (!m_doc->openDocument(url)) {
            fileOpenRecent->removeUrl(url);
            fileOpenRecent->setCurrentItem( -1 );
        } 
        else {
            fileOpenRecent->addUrl(url);
        }
        enablePrint(true);
        setCaption(m_doc->url().fileName(), false);
    }

    m_loading = false;
    slotUpdateViews();
    resetStatusMsg();
}

/**
 * Save a document.
 */
void UMLApp::slotFileSave()
{
    slotStatusMsg(i18n("Saving file..."));
    if (m_doc->url().fileName() == i18n("Untitled")) {
        slotFileSaveAs();
    }
    else {
        m_doc->saveDocument(m_doc->url());
    }
    m_pUndoStack->setClean();
    resetStatusMsg();
}

/**
 * Save a document by a new filename.
 */
bool UMLApp::slotFileSaveAs()
{
    slotStatusMsg(i18n("Saving file with a new filename..."));
    bool cont = true;
    KUrl url;
    QString ext;
    while (cont) {
        url=KFileDialog::getSaveUrl(KUrl(), i18n("*.xmi|XMI File\n*.xmi.tgz|Gzip Compressed XMI File\n*.xmi.tar.bz2|Bzip2 Compressed XMI File\n*|All Files"), this, i18n("Save As"));
        if (url.isEmpty()) {
            cont = false;
        }
        else {
            // now check that we have a file extension; standard will be plain xmi
            //QString file = url.path( KUrl::RemoveTrailingSlash );
            //QFileInfo info(file);
            //ext = info.completeSuffix();
            //if (ext != "xmi" && ext != "xmi.tgz" && ext != "xmi.tar.bz2")
            //{
            //    url.setFileName(url.fileName() + ".xmi");
            //    ext = "xmi";
            //}
            if (url.isLocalFile()) {
                QString file = url.toLocalFile( KUrl::RemoveTrailingSlash );

                if (QFile::exists(file)) {
                    int want_save = KMessageBox::warningContinueCancel(this, i18n("The file %1 exists.\nDo you wish to overwrite it?", url.toLocalFile()), 
                                                                       i18n("Warning"), KGuiItem(i18n("Overwrite")));
                    if (want_save == KMessageBox::Continue) {
                        cont = false;
                    }
                }
                else {
                    cont = false;
                }
            } else {
                cont = false;
            }
        }
    }
    if (!url.isEmpty()) {
        bool b = m_doc->saveDocument(url);
        if (b) {
            fileOpenRecent->addUrl(url);
            setCaption(url.fileName(),m_doc->isModified());
            resetStatusMsg();
        }
        return b;
    } 
    else {
        resetStatusMsg();
        return false;
    }
}

/**
 * Asks for saving if the file is modified, then closes the current
 * file and window.
 */
void UMLApp::slotFileClose()
{
    slotStatusMsg(i18n("Closing file..."));
    slotFileNew();
}

/**
 * Slot for showing a print settings dialog.
 */
bool UMLApp::slotPrintSettings()
{
    if (m_printSettings) {
        delete m_printSettings;
    }
    m_printSettings = new DiagramPrintPage(0, m_doc);

    QPointer<KDialog> dlg = new KDialog();
    dlg->setMainWidget(m_printSettings);

    bool result = dlg->exec() == QDialog::Accepted;

    // keep settings
    dlg->setMainWidget(0);
    m_printSettings->setParent(0);

    delete dlg;
    return result;
}

/**
 * Print preview
 */
void UMLApp::slotPrintPreview()
{
    slotStatusMsg(i18n("Print Preview..."));

    if (!slotPrintSettings())
        return;

    QPointer<QPrintPreviewDialog> preview = new QPrintPreviewDialog(m_printer,this);
    connect(preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrintPreviewPaintRequested(QPrinter*)));
    preview->exec();
    delete preview;
    delete m_printSettings;
    m_printSettings = 0;
    resetStatusMsg();
}

/**
 * Print preview painting slot
 */
void UMLApp::slotPrintPreviewPaintRequested(QPrinter *printer)
{
    m_doc->print(printer, m_printSettings);
}

/**
 * Print the current file.
 */
void UMLApp::slotFilePrint()
{
    slotStatusMsg(i18n("Printing..."));

    if (!slotPrintSettings())
        return;

    QPrintDialog *printDialog =
                  KdePrint::createPrintDialog(m_printer, QList<QWidget*>() << m_printSettings, this);
    printDialog->setWindowTitle(i18n("Print %1", m_doc->url().prettyUrl()));

    if (printDialog->exec()) {
        m_doc->print(m_printer, m_printSettings);
    }
    delete m_printSettings;
    m_printSettings = 0;
    delete printDialog;
    resetStatusMsg();
}

/**
 * Closes all open windows by calling close() on each
 * memberList item until the list is empty, then quits the
 * application.  If queryClose() returns false because the
 * user canceled the saveModified() dialog, the closing
 * aborts.
 */
void UMLApp::slotFileQuit()
{
    slotStatusMsg(i18n("Exiting..."));
    if (m_doc->saveModified()) {
        saveOptions();
        qApp->quit();
    }
    resetStatusMsg();
}

/**
 * Exports the current model to docbook in a subdir of the
 * current model directory named from the model name.
 * @todo Let the user chose the destination directory and
 * name, using network transparency.
 */
void UMLApp::slotFileExportDocbook()
{
  DocbookGenerator* docbookGenerator = new DocbookGenerator;
  docbookGenerator->generateDocbookForProject();
  connect( docbookGenerator, SIGNAL(finished(bool)), docbookGenerator, SLOT(deleteLater()));
}

/**
 * Exports the current model to XHTML in a subdir of the
 * current model directory named from the model name.
 * @todo Let the user chose the destination directory and
 * name, using network transparency.
 */
void UMLApp::slotFileExportXhtml()
{
  if (m_xhtmlGenerator != 0) {
    return;
  }
  m_xhtmlGenerator = new XhtmlGenerator();
  m_xhtmlGenerator->generateXhtmlForProject();
  connect(m_xhtmlGenerator, SIGNAL(finished(bool)), this, SLOT(slotXhtmlDocGenerationFinished(bool)));
}

/**
 * Reverts the document back to the state it was prior to the
 * last action performed by the user.
 */
void UMLApp::slotEditUndo()
{
    undo();
    resetStatusMsg();
}

/**
 * Reverts the document back to the state it was prior to the
 * last undo.
 */
void UMLApp::slotEditRedo()
{
    redo();
    resetStatusMsg();
}

/**
 * Put the marked text/object into the clipboard and remove
 * it from the document.
 */
void UMLApp::slotEditCut()
{
    slotStatusMsg(i18n("Cutting selection..."));
    //FIXME bug 59774 this fromview isn't very reliable.
    //when cutting diagrams it is set to true even though it shouldn't be
    bool fromview = (currentView() && currentView()->umlScene()->getSelectCount());
    if ( editCutCopy(fromview) ) {
        emit sigCutSuccessful();
        slotDeleteSelectedWidget();
        m_doc->setModified(true);
    }
    resetStatusMsg();
}

/**
 * Put the marked text/object into the clipboard.
 */
void UMLApp::slotEditCopy()
{
    slotStatusMsg(i18n("Copying selection to clipboard..."));
    bool  fromview = (currentView() && currentView()->umlScene()->getSelectCount());
    editCutCopy( fromview );
    resetStatusMsg();
    m_doc->setModified( true );
}

/**
 * Paste the clipboard into the document.
 */
void UMLApp::slotEditPaste()
{
    slotStatusMsg(i18n("Inserting clipboard contents..."));
    const QMimeData* data = QApplication::clipboard()->mimeData();
    UMLClipboard clipboard;
    setCursor(Qt::WaitCursor);
    if (!clipboard.paste(data)) {
        KMessageBox::sorry( this, i18n("Umbrello could not paste the clipboard contents.  "
                                       "The objects in the clipboard may be of the wrong "
                                       "type to be pasted here."), i18n("Paste Error") );
    }
    resetStatusMsg();
    setCursor(Qt::ArrowCursor);
    editPaste->setEnabled(false);
    m_doc->setModified( true );
}

/**
 * Changes the statusbar contents for the standard label
 * permanently, used to indicate current actions.
 * @param text   The text that is displayed in the statusbar
 */
void UMLApp::slotStatusMsg(const QString &text)
{
    // change status message permanently
    m_statusBarMessage->setText(text);
}

/**
 * Helper method to reset the status bar message.
 */
void UMLApp::resetStatusMsg()
{
    m_statusBarMessage->setText( i18nc("reset status bar", "Ready."));
}

/**
 * Create this view.
 */
void UMLApp::slotClassDiagram()
{
    executeCommand(new Uml::CmdCreateClassDiag(m_doc));
}

/**
 * Create this view.
 */
void UMLApp::slotSequenceDiagram()
{
    executeCommand(new Uml::CmdCreateSeqDiag(m_doc));
}

/**
 * Create this view.
 */
void UMLApp::slotCollaborationDiagram()
{
    executeCommand(new Uml::CmdCreateCollaborationDiag(m_doc));
}

/**
 * Create this view.
 */
void UMLApp::slotUseCaseDiagram()
{
    executeCommand(new Uml::CmdCreateUseCaseDiag(m_doc));
}

/**
 * Create this view.
 */
void UMLApp::slotStateDiagram()
{
    executeCommand(new Uml::CmdCreateStateDiag(m_doc));
}

/**
 * Create this view.
 */
void UMLApp::slotActivityDiagram()
{
    executeCommand(new Uml::CmdCreateActivityDiag(m_doc));
}

/**
 * Create this view.
 */
void UMLApp::slotComponentDiagram()
{
    executeCommand(new Uml::CmdCreateComponentDiag(m_doc));
}

/**
 * Create this view.
 */
void UMLApp::slotDeploymentDiagram()
{
    executeCommand(new Uml::CmdCreateDeployDiag(m_doc));
}

/**
 * Create this view.
 */
void UMLApp::slotEntityRelationshipDiagram()
{
    executeCommand(new Uml::CmdCreateEntityRelationDiag(m_doc));
}

/**
 * Left Alignment
 */
void UMLApp::slotAlignLeft()
{
    currentView()->umlScene()->alignLeft();
}

/**
 * Right Alignment
 */
void UMLApp::slotAlignRight()
{
    currentView()->umlScene()->alignLeft();
}

/**
 * Top Alignment
 */
void UMLApp::slotAlignTop()
{
    currentView()->umlScene()->alignTop();
}

/**
 * Bottom Alignment
 */
void UMLApp::slotAlignBottom()
{
    currentView()->umlScene()->alignBottom();
}

/**
 * Vertical Middle Alignment
 */
void UMLApp::slotAlignVerticalMiddle()
{
    currentView()->umlScene()->alignVerticalMiddle();
}

/**
 * Horizontal Middle Alignment
 */
void UMLApp::slotAlignHorizontalMiddle()
{
    currentView()->umlScene()->alignHorizontalMiddle();
}

/**
 * Vertical Distribute Alignment
 */
void UMLApp::slotAlignVerticalDistribute()
{
    currentView()->umlScene()->alignVerticalDistribute();
}

/**
 * Horizontal Distribute Alignment
 */
void UMLApp::slotAlignHorizontalDistribute()
{
    currentView()->umlScene()->alignHorizontalDistribute();
}

/**
 * Returns the toolbar being used.
 *
 * @return  The toolbar being used.
 */
WorkToolBar* UMLApp::workToolBar() const
{
    return m_toolsbar;
}

/**
 * Returns the doc window used.
 *
 * @return the doc window being used
 */
DocWindow* UMLApp::docWindow() const
{
    return m_docWindow;
}

/**
 * Sets whether the program has been modified.
 * This will change how the program saves/exits.
 *
 * @param modified   true - modified.
 */
void UMLApp::setModified(bool modified)
{
    //fileSave->setEnabled(modified);

    //if anything else needs to be done on a mofication, put it here

    // printing should be possible whenever there is something to print
    if ( m_loading == false && modified == true && currentView() )  {
        enablePrint(true);
    }

    if (m_loading == false)  {
        setCaption(m_doc->url().fileName(), modified); //add disk icon to taskbar if modified
    }
}

/**
 * Set whether to allow printing.
 * It will enable/disable the menu/toolbar options.
 *
 * @param enable    Set whether to allow printing.
 */
void UMLApp::enablePrint(bool enable)
{
    filePrint->setEnabled(enable);
    printPreview->setEnabled(enable);
}

/**
 * Set whether to allow Undo.
 * It will enable/disable the menu/toolbar options.
 *
 * @param enable    Set whether to allow printing.
 */
void UMLApp::enableUndo(bool enable)
{
    editUndo->setEnabled(enable);
}

/**
 * Set whether to allow Redo.
 * It will enable/disable the menu/toolbar options.
 *
 * @param enable    Set whether to allow printing.
 */
void UMLApp::enableRedo(bool enable)
{
    editRedo->setEnabled(enable);
}

/**
 * Initialize Qt's global clipboard support for the application.
 */
void UMLApp::initClip()
{
    QClipboard* clip = QApplication::clipboard();
    connect(clip, SIGNAL(dataChanged()), this, SLOT(slotClipDataChanged()));

    // Don't poll the X11 clipboard every second. This is a little expensive and resulted
    // in very annoying umbrello slowdowns / hangs. Qt will notify us about clipboard
    // changes anyway (see dataChanged() signal above), albeit only when a Qt application
    // changes the clipboard. Work is in progress to make this work with other toolkits
    // as well. (pfeiffer)
    // m_clipTimer = new QTimer(this, "timer");
    // m_clipTimer->start(1000, false);
    // connect(m_clipTimer, SIGNAL(timeout()), this, SLOT(slotClipDataChanged()));

    m_copyTimer = new QTimer(this);
    m_copyTimer->setSingleShot(false);
    m_copyTimer->start(500);
    connect(m_copyTimer, SIGNAL(timeout()), this, SLOT(slotCopyChanged()));
}

/**
 *  Returns whether we can decode the given mimesource
 */
bool UMLApp::canDecode(const QMimeData* mimeData)
{
    QStringList supportedFormats = mimeData->formats();
    foreach( const QString &format, supportedFormats ) {
        QByteArray fba = format.toAscii();
        const char* f = fba.constData();
        if ( !qstrnicmp(f,"application/x-uml-clip", 22) ) {
            //FIXME need to test for clip1, clip2, clip3, clip4 or clip5
            //(the only valid clip types)
            return true;
        }
    }
    return false;
}

/**
 * Notification of changed clipboard data.
 */
void UMLApp::slotClipDataChanged()
{
    const QMimeData * data = QApplication::clipboard()->mimeData();

    // Pass the MimeSource to the Doc
    editPaste->setEnabled( data && canDecode(data) );
}

/**
 * Slot for enabling cut and copy to clipboard.
 */
void UMLApp::slotCopyChanged()
{
    if (m_listView->selectedItemsCount() || (currentView() && currentView()->umlScene()->getSelectCount())) {
        editCopy->setEnabled(true);
        editCut->setEnabled(true);
    }
    else {
        editCopy->setEnabled(false);
        editCut->setEnabled(false);
    }
}

/**
 * Shows the global preferences dialog.
 */
void UMLApp::slotPrefs()
{
       Settings::OptionState& optionState = Settings::optionState();

       m_settingsDlg = new SettingsDlg(this, &optionState);
       connect(m_settingsDlg, SIGNAL(applyClicked()), this, SLOT(slotApplyPrefs()));

       if ( m_settingsDlg->exec() == QDialog::Accepted && m_settingsDlg->getChangesApplied() ) {
           slotApplyPrefs();
       }

       delete m_settingsDlg;
       m_settingsDlg = 0;
}

/**
 * Commits the changes from the global preferences dialog.
 */
void UMLApp::slotApplyPrefs()
{
    if (m_settingsDlg) {
        // we need this to sync both values
        Settings::OptionState& optionState = Settings::optionState();
        bool stackBrowsing = (m_layout->indexOf(m_tabWidget) != -1);
        bool tabBrowsing = optionState.generalState.tabdiagrams;
        DEBUG(DBG_SRC) << "stackBrowsing=" << stackBrowsing << " / tabBrowsing=" << tabBrowsing;

        if (stackBrowsing != tabBrowsing) {
            // Diagram Representation Modified
            UMLView* currentView;
            UMLViewList views = m_doc->viewIterator();

            if (tabBrowsing) {
                currentView = static_cast<UMLView*>(m_viewStack->currentWidget());
                m_layout->removeWidget(m_viewStack);
                m_viewStack->hide();

                foreach (UMLView *view, views) {
                    UMLScene *scene = view->umlScene();
                    m_viewStack->removeWidget(view);
                    int tabIndex = m_tabWidget->addTab(view, scene->name());
                    m_tabWidget->setTabIcon(tabIndex, Icon_Utils::iconSet(scene->type()));
                    m_tabWidget->setTabToolTip(tabIndex, scene->name());
                }
                m_layout->addWidget(m_tabWidget);
                m_tabWidget->show();
            }
            else {  // stackBrowsing
                currentView = static_cast<UMLView*>(m_tabWidget->currentWidget());
                m_layout->removeWidget(m_tabWidget);
                m_tabWidget->hide();

                foreach (UMLView *view, views) {
                    m_tabWidget->removeTab(m_tabWidget->indexOf(view));
                    m_viewStack->addWidget(view);
                }
                m_layout->addWidget(m_viewStack);
                m_viewStack->show();
            }
            setCurrentView(currentView);
        }

        m_doc->settingsChanged( optionState );
        const QString plStr = m_settingsDlg->getCodeGenerationLanguage();
        Uml::ProgrammingLanguage pl = Uml::ProgrammingLanguage::fromString(plStr);
        setGenerator(pl);
    }
}

/**
 * Returns the undo state.
 *
 * @return  True if Undo is enabled.
 */
bool UMLApp::isUndoEnabled() const
{
    return editUndo->isEnabled();
}

/**
 * Returns the redo state.
 *
 * @return  True if Redo is enabled.
 */
bool UMLApp::isRedoEnabled() const
{
    return editRedo->isEnabled();
}

/**
 * Returns the paste state.
 *
 * @return  True if Paste is enabled.
 */
bool UMLApp::isPasteState() const
{
    return editPaste->isEnabled();
}

/**
 * Returns the state on Cut/Copy.
 *
 * @return  True if Cut/Copy is enabled.
 */
bool UMLApp::isCutCopyState() const
{
    return editCopy->isEnabled();
}

/**
 * Carries out the cut/copy command with different action performed
 * depending on if from view or list view.
 * Cut/Copy are the same.  It is up to the caller to delete/cut the selection..
 *
 * If the operation is successful, the signal sigCutSuccessful() is emitted.
 *
 * Callers should connect to this signal to know what to do next.
 */
bool UMLApp::editCutCopy( bool bFromView )
{
    UMLClipboard clipboard;
    QMimeData * clipdata = 0;

    if ((clipdata = clipboard.copy(bFromView)) != 0) {
        QClipboard* clip = QApplication::clipboard();
        clip->setMimeData(clipdata);//the global clipboard takes ownership of the clipdata memory
        connect(clip, SIGNAL(dataChanged()), this, SLOT(slotClipDataChanged()));
        return true;
    }
    return false;
}

/**
 * Reads from the config file the options state.
 * Not in @ref readOptions as it needs to be read earlier than some
 * of the other options, before some items are created.
 */
void UMLApp::readOptionState()
{
    Settings::OptionState& optionState = Settings::optionState();

    UmbrelloSettings::self()->readConfig();
    optionState.generalState.undo = UmbrelloSettings::undo();
    optionState.generalState.tabdiagrams = UmbrelloSettings::tabdiagrams();
    optionState.generalState.newcodegen = UmbrelloSettings::newcodegen();
    optionState.generalState.angularlines = UmbrelloSettings::angularlines();
    optionState.generalState.footerPrinting =  UmbrelloSettings::footerPrinting();
    optionState.generalState.autosave =  UmbrelloSettings::autosave();
    optionState.generalState.time =  UmbrelloSettings::time(); //old autosavetime value kept for compatibility
    optionState.generalState.autosavetime =  UmbrelloSettings::autosavetime();
    //if we don't have a "new" autosavetime value, convert the old one
    if (optionState.generalState.autosavetime == 0) {
        switch (optionState.generalState.time) {
        case 0: optionState.generalState.autosavetime =  5; break;
        case 1: optionState.generalState.autosavetime = 10; break;
        case 2: optionState.generalState.autosavetime = 15; break;
        case 3: optionState.generalState.autosavetime = 20; break;
        case 4: optionState.generalState.autosavetime = 25; break;
        default: optionState.generalState.autosavetime = 5; break;
        }
    }

    optionState.generalState.autosavesuffix =  UmbrelloSettings::autosavesuffix();
    optionState.generalState.loadlast =  UmbrelloSettings::loadlast();
    optionState.generalState.diagram  = UmbrelloSettings::diagram();
    optionState.generalState.defaultLanguage =  UmbrelloSettings::defaultLanguage();

    optionState.uiState.useFillColor =  UmbrelloSettings::useFillColor();
    optionState.uiState.fillColor =  UmbrelloSettings::fillColor();
    optionState.uiState.lineColor =  UmbrelloSettings::lineColor();
    optionState.uiState.lineWidth =  UmbrelloSettings::lineWidth();
    optionState.uiState.textColor = UmbrelloSettings::textColor();
    optionState.uiState.font =  UmbrelloSettings::uiFont();
    optionState.uiState.backgroundColor = UmbrelloSettings::backgroundColor();
    optionState.uiState.gridDotColor = UmbrelloSettings::gridDotColor();

    optionState.classState.showVisibility =  UmbrelloSettings::showVisibility();
    optionState.classState.showAtts =  UmbrelloSettings::showAtts();
    optionState.classState.showOps =  UmbrelloSettings::showOps();
    optionState.classState.showStereoType =  UmbrelloSettings::showStereoType();
    optionState.classState.showAttSig =  UmbrelloSettings::showAttSig();
    optionState.classState.showOpSig =  UmbrelloSettings::showOpSig();
    optionState.classState.showPackage =  UmbrelloSettings::showPackage();
    optionState.classState.showAttribAssocs =  UmbrelloSettings::showAttribAssocs();
    optionState.classState.showPublicOnly =  UmbrelloSettings::showPublicOnly();
    optionState.classState.defaultAttributeScope = UmbrelloSettings::defaultAttributeScope();
    optionState.classState.defaultOperationScope = UmbrelloSettings::defaultOperationScope();

    optionState.codeViewerState.height =  UmbrelloSettings::height();
    optionState.codeViewerState.width =  UmbrelloSettings::width();
    optionState.codeViewerState.font =  UmbrelloSettings::codeViewerFont();
    optionState.codeViewerState.showHiddenBlocks =  UmbrelloSettings::showHiddenBlocks();
    optionState.codeViewerState.blocksAreHighlighted =  UmbrelloSettings::blocksAreHighlighted();
    optionState.codeViewerState.selectedColor =  UmbrelloSettings::selectedColor();
    optionState.codeViewerState.paperColor =  UmbrelloSettings::paperColor();
    optionState.codeViewerState.fontColor =  UmbrelloSettings::fontColor();
    optionState.codeViewerState.editBlockColor =  UmbrelloSettings::editBlockColor();
    optionState.codeViewerState.umlObjectColor =  UmbrelloSettings::umlObjectColor();
    optionState.codeViewerState.nonEditBlockColor =  UmbrelloSettings::nonEditBlockColor();
    optionState.codeViewerState.hiddenColor =  UmbrelloSettings::hiddenColor();

    // CPP code generation options
    optionState.codeGenerationState.cppCodeGenerationState.autoGenAccessors = UmbrelloSettings::autoGenAccessors();

    optionState.codeGenerationState.cppCodeGenerationState.inlineAccessors = UmbrelloSettings::inlineAccessors();
    optionState.codeGenerationState.cppCodeGenerationState.publicAccessors = UmbrelloSettings::publicAccessors();
    optionState.codeGenerationState.cppCodeGenerationState.inlineOps = UmbrelloSettings::inlineOps();
    optionState.codeGenerationState.cppCodeGenerationState.virtualDestructors = UmbrelloSettings::virtualDestructors();
    optionState.codeGenerationState.cppCodeGenerationState.packageIsNamespace = UmbrelloSettings::packageIsNamespace();

    optionState.codeGenerationState.cppCodeGenerationState.stringClassName = UmbrelloSettings::stringClassName();
    optionState.codeGenerationState.cppCodeGenerationState.stringClassNameInclude = UmbrelloSettings::stringClassNameInclude();
    optionState.codeGenerationState.cppCodeGenerationState.stringIncludeIsGlobal = UmbrelloSettings::stringIncludeIsGlobal();

    optionState.codeGenerationState.cppCodeGenerationState.vectorClassName = UmbrelloSettings::vectorClassName();
    optionState.codeGenerationState.cppCodeGenerationState.vectorClassNameInclude = UmbrelloSettings::vectorClassNameInclude();
    optionState.codeGenerationState.cppCodeGenerationState.vectorIncludeIsGlobal = UmbrelloSettings::vectorIncludeIsGlobal();
    optionState.codeGenerationState.cppCodeGenerationState.docToolTag = UmbrelloSettings::docToolTag();

    // Java code generation options
    optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAttributeAccessors = UmbrelloSettings::autoGenerateAttributeAccessorsJava();
    optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAssocAccessors = UmbrelloSettings::autoGenerateAssocAccessorsJava();

    // D code generation options
    optionState.codeGenerationState.dCodeGenerationState.autoGenerateAttributeAccessors = UmbrelloSettings::autoGenerateAttributeAccessorsD();
    optionState.codeGenerationState.dCodeGenerationState.autoGenerateAssocAccessors = UmbrelloSettings::autoGenerateAssocAccessorsD();

    // Ruby code generation options
    optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAttributeAccessors = UmbrelloSettings::autoGenerateAttributeAccessorsRuby();
    optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAssocAccessors = UmbrelloSettings::autoGenerateAssocAccessorsRuby();

    // code importer options
    optionState.codeImportState.createArtifacts = UmbrelloSettings::createArtifacts();

    // general config options will be read when created
}

/**
 * Call the code viewing assistant on a given UMLClassifier.
 *
 * @param classifier  Pointer to the classifier to view.
 */
void UMLApp::viewCodeDocument(UMLClassifier* classifier)
{
    CodeGenerator * currentGen = generator();
    if (currentGen && classifier) {
        if (!dynamic_cast<SimpleCodeGenerator*>(currentGen)) {
            CodeDocument *cdoc = currentGen->findCodeDocumentByClassifier(classifier);

            if (cdoc) {
                Settings::OptionState& optionState = Settings::optionState();
                CodeViewerDialog * dialog = currentGen->getCodeViewerDialog(this,cdoc,optionState.codeViewerState);
                dialog->exec();
                optionState.codeViewerState = dialog->state();
                delete dialog;
                dialog = NULL;
            } else {
                // shouldn't happen..
                KMessageBox::sorry(0, i18n("Cannot view code until you generate some first."), i18n("Cannot View Code"));
            }
        } else {
            KMessageBox::sorry(0, i18n("Cannot view code from simple code writer."), i18n("Cannot View Code"));
        }
    } else {
        uWarning() << "No CodeGenerator or UMLClassifier given!";
    }
}

/**
 * Call the refactoring assistant on a classifier.
 *
 * @param classifier  Pointer to the classifier to refactor.
 */
void UMLApp::refactor(UMLClassifier* classifier)
{
    if (!m_refactoringAssist) {
        m_refactoringAssist = new RefactoringAssistant( m_doc, 0, 0, "refactoring_assistant" );
    }
    m_refactoringAssist->refactor(classifier);
    m_refactoringAssist->show();
}

/**
 * Returns the default code generation policy.
 */
CodeGenerationPolicy *UMLApp::commonPolicy() const
{
    return m_commoncodegenpolicy;
}

/**
 * Sets the CodeGenPolicyExt object.
 */
void UMLApp::setPolicyExt(CodeGenPolicyExt *policy)
{
    m_policyext = policy;
}

/**
 * Returns the CodeGenPolicyExt object.
 */
CodeGenPolicyExt *UMLApp::policyExt() const
{
    return m_policyext;
}

/**
 * Auxiliary function for UMLDoc::loadExtensionsFromXMI():
 * Return the code generator of the given language if it already
 * exists; if it does not yet exist then create it and return
 * the newly created generator. It is the caller's responsibility
 * to load XMI into the newly created generator.
 */
CodeGenerator *UMLApp::setGenerator(Uml::ProgrammingLanguage pl)
{
    if (pl == Uml::ProgrammingLanguage::Reserved) {
        if (m_codegen) {
            delete m_codegen;
            m_codegen = NULL;
        }
        return NULL;
    }
    // does the code generator already exist?
    // then simply return that
    if (m_codegen) {
        if (m_codegen->language() == pl) {
            return m_codegen;
        }
        delete m_codegen;  // ATTENTION! remove all refs to it or its policy first
        m_codegen = NULL;
    }
    m_activeLanguage = pl;
    m_codegen = CodeGenFactory::createObject(pl);
    updateLangSelectMenu(pl);

    slotAddDefaultDatatypes();
    m_codegen->createDefaultStereotypes();

    if (m_policyext) {
        m_policyext->setDefaults(false); // picks up language specific stuff
    }
    return m_codegen;
}

/**
 * Gets the appropriate CodeGenerator.
 *
 * @return  Pointer to the CodeGenerator.
 */
CodeGenerator* UMLApp::generator() const
{
    return m_codegen;
}

/**
 * Determines if SimpleCodeGenerator is active.
 *
 * @return  true if SimpleCodeGenerator is active.
 */
bool UMLApp::isSimpleCodeGeneratorActive()
{
    if (m_codegen && dynamic_cast<SimpleCodeGenerator*>(m_codegen)) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * Generate code for all classes.
 */
void UMLApp::slotGenerateAllCode()
{
    if (m_codegen) {
        m_codegen->writeCodeToFile();
    }
}

/**
 * Runs the code generation wizard.
 */
void UMLApp::slotExecGenerationWizard()
{
    QPointer<CodeGenerationWizard> wizard = new CodeGenerationWizard(0 /*classList*/);
    wizard->exec();
    delete wizard;
}

/**
 * Slots for connection to the QActions of the m_langSelect menu.
 */
void UMLApp::setLang_actionscript()
{
    setActiveLanguage(Uml::ProgrammingLanguage::ActionScript);
}

void UMLApp::setLang_ada()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Ada);
}

void UMLApp::setLang_cpp()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Cpp);
}

void UMLApp::setLang_csharp()
{
    setActiveLanguage(Uml::ProgrammingLanguage::CSharp);
}

void UMLApp::setLang_d()
{
    setActiveLanguage(Uml::ProgrammingLanguage::D);
}

void UMLApp::setLang_idl()
{
    setActiveLanguage(Uml::ProgrammingLanguage::IDL);
}

void UMLApp::setLang_java()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Java);
}

void UMLApp::setLang_javascript()
{
    setActiveLanguage(Uml::ProgrammingLanguage::JavaScript);
}

void UMLApp::setLang_mysql()
{
    setActiveLanguage(Uml::ProgrammingLanguage::MySQL);
}

void UMLApp::setLang_pascal()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Pascal);
}

void UMLApp::setLang_perl()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Perl);
}

void UMLApp::setLang_php()
{
    setActiveLanguage(Uml::ProgrammingLanguage::PHP);
}

void UMLApp::setLang_php5()
{
    setActiveLanguage(Uml::ProgrammingLanguage::PHP5);
}

void UMLApp::setLang_postgresql()
{
    setActiveLanguage(Uml::ProgrammingLanguage::PostgreSQL);
}

void UMLApp::setLang_python()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Python);
}

void UMLApp::setLang_ruby()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Ruby);
}

void UMLApp::setLang_sql()
{
    setActiveLanguage(Uml::ProgrammingLanguage::SQL);
}

void UMLApp::setLang_tcl()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Tcl);
}

void UMLApp::setLang_vala()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Vala);
}

void UMLApp::setLang_xmlschema()
{
    setActiveLanguage(Uml::ProgrammingLanguage::XMLSchema);
}

/**
 * Set the language for which code will be generated.
 *
 * @param pl    The name of the language to set
 */
void UMLApp::setActiveLanguage(Uml::ProgrammingLanguage pl)
{
    updateLangSelectMenu(pl);
    setGenerator(pl);
}

/**
 * Get the language for import and code generation.
 */
Uml::ProgrammingLanguage UMLApp::activeLanguage() const
{
    return m_activeLanguage;
}

/**
 * Return true if the active language is case sensitive.
 */
bool UMLApp::activeLanguageIsCaseSensitive()
{
    return (m_activeLanguage != Uml::ProgrammingLanguage::Pascal &&
            m_activeLanguage != Uml::ProgrammingLanguage::Ada &&
            m_activeLanguage != Uml::ProgrammingLanguage::SQL &&
            m_activeLanguage != Uml::ProgrammingLanguage::MySQL &&
            m_activeLanguage != Uml::ProgrammingLanguage::PostgreSQL);
}

/**
 * Return the target language depedent scope separator.
 */
QString UMLApp::activeLanguageScopeSeparator()
{
    Uml::ProgrammingLanguage pl = activeLanguage();
    if (pl == Uml::ProgrammingLanguage::Ada ||
        pl == Uml::ProgrammingLanguage::CSharp ||
        pl == Uml::ProgrammingLanguage::Pascal ||
        pl == Uml::ProgrammingLanguage::Java ||
        pl == Uml::ProgrammingLanguage::JavaScript ||
        pl == Uml::ProgrammingLanguage::Vala ||
        pl == Uml::ProgrammingLanguage::Python)  // CHECK: more?
        return ".";
    return "::";
}

/**
 * Menu selection for clear current view.
 */
void UMLApp::slotCurrentViewClearDiagram()
{
    currentView()->umlScene()->clearDiagram();
}

/**
 * Menu selection for current view snap to grid property.
 */
void UMLApp::slotCurrentViewToggleSnapToGrid()
{
    currentView()->umlScene()->toggleSnapToGrid();
    viewSnapToGrid->setChecked( currentView()->umlScene()->getSnapToGrid() );
}

/**
 * Menu selection for current view show grid property.
 */
void UMLApp::slotCurrentViewToggleShowGrid()
{
    currentView()->umlScene()->toggleShowGrid();
    viewShowGrid->setChecked( currentView()->umlScene()->getShowSnapGrid() );
}

/**
 * Menu selection for exporting current view as an image.
 */
void UMLApp::slotCurrentViewExportImage()
{
    currentView()->umlScene()->getImageExporter()->exportView();
}

/**
 * Menu selection for exporting all views as images.
 */
void UMLApp::slotAllViewsExportImage()
{
    m_imageExporterAll->exportAllViews();
}

/**
 * Menu selection for current view properties.
 */
void UMLApp::slotCurrentViewProperties()
{
    currentView()->umlScene()->showPropDialog();
}

/**
 * Sets the state of the view properties menu item.
 *
 * @param bState  Boolean, true to enable the view properties item.
 */
void UMLApp::setDiagramMenuItemsState(bool bState)
{
    viewClearDiagram->setEnabled( bState );
    viewSnapToGrid->setEnabled( bState );
    viewShowGrid->setEnabled( bState );
    deleteDiagram->setEnabled(bState);
    viewExportImage->setEnabled( bState );
    viewProperties->setEnabled( bState );
    filePrint->setEnabled( bState );
    if ( currentView() ) {
        viewSnapToGrid->setChecked( currentView()->umlScene()->getSnapToGrid() );
        viewShowGrid->setChecked( currentView()->umlScene()->getShowSnapGrid() );
    }
}

/**
 * Register new views (aka diagram) with the GUI so they show up
 * in the menu.
 */
void UMLApp::slotUpdateViews()
{
    QMenu* menu = findMenu( QString("views") );
    if (!menu) {
        uWarning() << "view menu not found";
        return;
    }

    menu = findMenu( QString("show_view") );
    if (!menu) {
        uWarning() << "show menu not found";
        return;
    }

    menu->clear();

    UMLViewList views = m_doc->viewIterator();
    foreach (UMLView *view , views ) {
        menu->addAction(view->umlScene()->name(), view->umlScene(), SLOT(slotShowView()));
        view->umlScene()->fileLoaded();
    }
}

/**
 * Import the source files that are in fileList.
 */
void UMLApp::importFiles(QStringList* fileList)
{
    if (! fileList->isEmpty()) {
        const QString& firstFile = fileList->first();
        ClassImport *classImporter = ClassImport::createImporterByFileExt(firstFile);
        classImporter->importFiles(*fileList);
        delete classImporter;
        m_doc->setLoading(false);
        // Modification is set after the import is made, because the file was modified when adding the classes.
        // Allowing undo of the whole class importing. I think it eats a lot of memory.
        // Setting the modification, but without allowing undo.
        m_doc->setModified(true);
    }
}

/**
 * Runs the code importing wizard.
 */
void UMLApp::slotImportingWizard()
{
    QPointer<CodeImportingWizard> wizard = new CodeImportingWizard();
    wizard->exec();
    delete wizard;
}

/**
 * Class wizard menu selection.
 */
void UMLApp::slotClassWizard()
{
    QPointer<ClassWizard> dlg  = new ClassWizard( m_doc );
    dlg->exec();
    delete dlg;
}

/**
 * Calls the active code generator to add its default datatypes.
 */
void UMLApp::slotAddDefaultDatatypes()
{
    m_doc->addDefaultDatatypes();
}

/**
 * The displayed diagram has changed.
 */
void UMLApp::slotCurrentViewChanged()
{
    UMLView *view = currentView();
    if (view) {
        connect(view->umlScene(), SIGNAL(sigShowGridToggled(bool)),
                this, SLOT(slotShowGridToggled(bool)));
        connect(view->umlScene(), SIGNAL(sigSnapToGridToggled(bool)),
                this, SLOT(slotSnapToGridToggled(bool)));
    }
}

/**
 * The snap to grid value has been changed.
 */
void UMLApp::slotSnapToGridToggled(bool gridOn)
{
    viewSnapToGrid->setChecked(gridOn);
}

/**
 * The show grid value has been changed.
 */
void UMLApp::slotShowGridToggled(bool gridOn)
{
    viewShowGrid->setChecked(gridOn);
}

/**
 * Select all widgets on the current diagram.
 */
void UMLApp::slotSelectAll()
{
    currentView()->umlScene()->selectAll();
}

/**
 * Deletes the selected widget.
 */
void UMLApp::slotDeleteSelectedWidget()
{
    if ( currentView() ) {
        currentView()->umlScene()->deleteSelection();
    }
    else {
        uWarning() << " trying to delete widgets when there is no current view (see bug 59774)";
    }
}

/**
 * Deletes the current diagram. Called from menu action.
 */
void UMLApp::slotDeleteDiagram()
{
    m_doc->removeDiagram( currentView()->umlScene()->getID() );
}

/**
 * Close the current diagram. Clicked on tab close button.
 * @param tab   Widget's tab to close
 */
void UMLApp::slotCloseDiagram(QWidget* tab)
{
    if (tab) {
        UMLView* view = (UMLView*)tab;
        if (view != currentView()) {
            setCurrentView(view);
        }
        m_tabWidget->removeTab(m_tabWidget->indexOf(view));
        view->umlScene()->setIsOpen(false);
    }
}

/**
 * Return the default code generation language as configured by KConfig.
 * If the activeLanguage is not found in the KConfig then use Uml::ProgrammingLanguage::Cpp
 * as the default.
 */
Uml::ProgrammingLanguage UMLApp::defaultLanguage()
{
    Settings::OptionState& optionState = Settings::optionState();
    return optionState.generalState.defaultLanguage;
}

/**
 * Reads the activeLanguage from the KConfig and calls updateLangSelectMenu()
 */
void UMLApp::initGenerator()
{
    if (m_codegen) {
        delete m_codegen;
        m_codegen = NULL;
    }
    Uml::ProgrammingLanguage defLanguage = defaultLanguage();
    setActiveLanguage(defLanguage);
}

/**
 * Updates the Menu for language selection and sets the
 * active lanugage. If no active lanugage is found or if it is
 * not one of the registered languages it tries to fall back
 * to Cpp
 */
void UMLApp::updateLangSelectMenu(Uml::ProgrammingLanguage activeLanguage)
{
    //m_langSelect->clear();
    for (int i = 0; i < Uml::ProgrammingLanguage::Reserved; ++i) {
        m_langAct[i]->setChecked(i == activeLanguage);
    }
}

/**
 * Event handler to receive key press events.
 */
void UMLApp::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Shift:
        //m_toolsbar->setOldTool();
        e->accept();
        break;

    case Qt::Key_D:
        if (e->modifiers() & Qt::ControlModifier) {  // Ctrl + D
            DEBUG(DBG_SRC) << "Ctrl + D is pressed. Show debug config dialog...";
            DEBUG_SHOW_FILTER();
        }
        break;

    default:
        e->ignore();
    }
}

/**
 * Event handler to receive custom events.
 * It handles events such as exporting all views from command line (in
 * that case, it executes the exportAllViews method in the event).
 */
void UMLApp::customEvent(QEvent* e)
{
    if (e->type() == CmdLineExportAllViewsEvent::eventType()) {
        CmdLineExportAllViewsEvent* exportAllViewsEvent = static_cast<CmdLineExportAllViewsEvent*>(e);
        exportAllViewsEvent->exportAllViews();
    }
}

/**
 * Helper method for handling cursor key release events (refactoring).
 * TODO Move this to UMLWidgetController?
 */
void UMLApp::handleCursorKeyReleaseEvent(QKeyEvent* e)
{
    // in case we have selected something in the diagram, move it by one pixel
    // to the direction pointed by the cursor key
    if (m_view == NULL || !m_view->umlScene()->getSelectCount() || e->modifiers() != Qt::AltModifier) {
        e->ignore();
        return;
    }
    int dx = 0;
    int dy = 0;
    switch (e->key()) {
    case Qt::Key_Left:
        dx = -1;
        break;
    case Qt::Key_Right:
        dx = 1;
        break;
    case Qt::Key_Up:
        dy = -1;
        break;
    case Qt::Key_Down:
        dy = 1;
        break;
    default:
        e->ignore();
        return;
    }
    m_view->umlScene()->moveSelectedBy(dx, dy);

    // notify about modification only at the first key release of possible sequence of auto repeat key releases,
    // this reduces the slow down caused by setModified() and makes the cursor moving of widgets smoother
    if (!e->isAutoRepeat()) {
        m_doc->setModified();
    }
    e->accept();
}

/**
 * Event handler for key release.
 */
void UMLApp::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Backspace:
        if (!m_docWindow->isTyping()) {
            m_toolsbar->setOldTool();
        }
        e->accept();
        break;
    case Qt::Key_Escape:
        m_toolsbar->setDefaultTool();
        e->accept();
        break;
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        handleCursorKeyReleaseEvent(e);
        break;
    default:
        e->ignore();
    }
}

/**
 * Calls the UMLDoc method to create a new Document.
 */
void UMLApp::newDocument()
{
    m_doc->newDocument();
    slotUpdateViews();
}

/**
 * Returns the widget used as the parent for UMLViews.
 * @return  The main view widget.
 */
QWidget* UMLApp::mainViewWidget()
{
    Settings::OptionState& optionState = Settings::optionState();
    if ( optionState.generalState.tabdiagrams ) {
        return m_tabWidget;
    }
    else {
        return m_viewStack;
    }
}

/**
 * Puts this view to the top of the viewStack, i.e. makes it
 * visible to the user.
 *
 * @param view   Pointer to the UMLView to push.
 */
void UMLApp::setCurrentView(UMLView* view)
{
    m_view = view;
    if (view == NULL) {
        DEBUG(DBG_SRC) << "view is NULL";
        docWindow()->newDocumentation();
        return;
    }

    Settings::OptionState optionState = Settings::optionState();
    if (optionState.generalState.tabdiagrams) {
        int tabIndex = m_tabWidget->indexOf(view);
        if ((tabIndex < 0) && (view->umlScene()->isOpen())) {
            tabIndex = m_tabWidget->addTab(view, view->umlScene()->name());
            m_tabWidget->setTabIcon(tabIndex, Icon_Utils::iconSet(view->umlScene()->type()));
            m_tabWidget->setTabToolTip(tabIndex, view->umlScene()->name());
        }
        m_tabWidget->setCurrentIndex(tabIndex);
    }
    else {
        if (m_viewStack->indexOf(view) < 0) {
            m_viewStack->addWidget(view);
        }
        m_viewStack->setCurrentWidget(view);
        view->show();
    }
    qApp->processEvents();
    slotStatusMsg(view->umlScene()->name());
    UMLListViewItem* lvitem = m_listView->findView(view);
    if (lvitem) {
        m_listView->setCurrentItem(lvitem);
    }
    DEBUG(DBG_SRC) << "name=" << view->umlScene()->name()
                   << ", isOpen=" << view->umlScene()->isOpen()
                   << ", id=" << ID2STR(view->umlScene()->getID());
}

/**
 * Get the current view.
 * This may return a null pointer (when no view was previously
 * specified.)
 */
UMLView* UMLApp::currentView() const
{
    return m_view;
}

/**
 * Sets the default mime type for all diagrams that are exported as images.
 * @param mimeType   the mime type
 */
void UMLApp::setImageMimeType(const QString& mimeType)
{
    m_imageMimeType = mimeType;
}

/**
 * Gets the default mime type for all diagrams that are exported as
 * images.
 * @return  The default MIME type for images.
 */
QString UMLApp::imageMimeType() const
{
    return m_imageMimeType;
}

/**
 * Called when the tab has changed.
 * @param tab   The changed tab widget
 */
void UMLApp::slotTabChanged(QWidget* tab)
{
    UMLView* view = ( UMLView* )tab;
    if (view) {
        m_doc->changeCurrentView( view->umlScene()->getID() );
    }
}

/**
 * Make the tab on the left of the current one the active one.
 */
void UMLApp::slotChangeTabLeft()
{
    //DEBUG(DBG_SRC) << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    if (m_tabWidget) {
        m_tabWidget->setCurrentIndex( m_tabWidget->currentIndex() - 1 );
        return;
    }
    UMLViewList views = m_doc->viewIterator();
    UMLView *currView = m_view;
    int viewIndex = 0;
    if ((viewIndex = views.indexOf(currView)) < 0) {
        uError() << "currView not found in viewlist";
        return;
    }
    UMLView* prevView = NULL;
    if ( viewIndex != 0 ) {
        prevView = views.begin()[viewIndex -1 ];
    }

    if ((currView = prevView) != NULL) {
        setCurrentView(currView);
    }
    else {
        setCurrentView(views.last());
    }
}

/**
 * Make the tab on the right of the current one the active one.
 */
void UMLApp::slotChangeTabRight()
{
    //DEBUG(DBG_SRC) << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    if (m_tabWidget) {
        m_tabWidget->setCurrentIndex( m_tabWidget->currentIndex() + 1 );
        return;
    }
    UMLViewList views = m_doc->viewIterator();
    UMLView *currView = m_view;
    int viewIndex = 0;
    if ((viewIndex = views.indexOf(currView)) < 0) {
        uError() << "currView not found in viewlist";
        return;
    }
    UMLView* nextView = NULL;
    if ( viewIndex!= views.count() ) {
        views.begin()[viewIndex + 1];
    }

    if ( (currView = nextView ) != NULL) {
        setCurrentView(currView);
    }
    else {
        setCurrentView(views.first());
    }
}

/* for debugging only
static void showTabTexts(KTabWidget* tabWidget)
{
    QString out = QString("tab texts ");
    for (int i = 0; i < tabWidget->count(); ++i) {
        out += " <" + tabWidget->tabText(i) + '>';
    }
    DEBUG(DBG_SRC) << out;
}
*/

/**
 * Move the current tab left.
 */
void UMLApp::slotMoveTabLeft()
{
    //DEBUG(DBG_SRC) << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    //showTabTexts(m_tabWidget);
    int from = m_tabWidget->currentIndex();
    int to   = -1;
    if (from > 0) {
        to = from - 1;
    }
    else {
        to = m_tabWidget->count() - 1;
    }
    m_tabWidget->moveTab(from, to);
}

/**
 * Move the current tab right.
 */
void UMLApp::slotMoveTabRight()
{
    //DEBUG(DBG_SRC) << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    //showTabTexts(m_tabWidget);
    int from = m_tabWidget->currentIndex();
    int to   = -1;
    if (from < m_tabWidget->count() - 1) {
        to = from + 1;
    }
    else {
        to = 0;
    }
    m_tabWidget->moveTab(from, to);
}

/**
 * This slot deletes the current XHTML documentation generator as soon as
 * this one signals that it has finished.
 * @param status true if successful else false
 */
void UMLApp::slotXhtmlDocGenerationFinished(bool status)
{
  if ( !status ) {
      m_doc->writeToStatusBar( i18n( "XHTML Generation failed ." ) );
  }

  delete m_xhtmlGenerator;
  m_xhtmlGenerator = 0;
}

/**
 * Return the tab widget.
 */
KTabWidget* UMLApp::tabWidget()
{
    return m_tabWidget;
}

/**
 * Returns the current text in the status bar.
 *
 * @return The text in the status bar.
 */
QString UMLApp::statusBarMsg()
{
    return m_statusBarMessage->text();
}

/**
 * Removes all entries from the UndoStack and RedoStack and disables the
 * undo and redo actions.
 */
void UMLApp::clearUndoStack()
{
    m_pUndoStack->clear();
}

/**
 * Undo last command
 */
void UMLApp::undo()
{
    DEBUG(DBG_SRC) << m_pUndoStack->undoText() << " [" << m_pUndoStack->count() << "]";
    m_pUndoStack->undo();

    if (m_pUndoStack->canUndo()) {
        UMLApp::app()->enableUndo(true);
    }
    else {
        UMLApp::app()->enableUndo(false);
    }

    UMLApp::app()->enableRedo(true);
}

/**
 * Redo last 'undoed' command
 */
void UMLApp::redo()
{
    DEBUG(DBG_SRC) << m_pUndoStack->redoText() << " [" << m_pUndoStack->count() << "]";
    m_pUndoStack->redo();

    if (m_pUndoStack->canRedo()) {
        UMLApp::app()->enableRedo(true);
    }
    else {
        UMLApp::app()->enableRedo(false);
    }

    UMLApp::app()->enableUndo(true);
}

/**
 * Execute a command and pushit in the stack.
 */
void UMLApp::executeCommand(QUndoCommand* cmd)
{
    if (cmd != NULL) {
        m_pUndoStack->push(cmd);
        DEBUG(DBG_SRC) << cmd->text() << " [" << m_pUndoStack->count() << "]";
    }

    UMLApp::app()->enableUndo(true);
}

/**
 * Begin a U/R command macro
 */
void UMLApp::beginMacro( const QString & text )
{
    if (m_hasBegunMacro) {
        return;
    }
    m_hasBegunMacro = true;

    m_pUndoStack->beginMacro(text);
}

/**
 * End an U/R command macro
 */
void UMLApp::endMacro()
{
    if (m_hasBegunMacro) {
        m_pUndoStack->endMacro();
    }
    m_hasBegunMacro = false;
}

/**
 * Return the config data.
 */
KConfig* UMLApp::config()
{
    return m_config.data();
}

//#include "uml.moc"
