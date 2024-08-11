/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2023 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "uml.h"

// app includes
#include "birdview.h"
#include "umlappprivate.h"
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
#include "findresults.h"
#include "folder.h"
#include "models/diagramsmodel.h"

// code generation
#include "advancedcodegenerator.h"
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
#include "diagramselectiondialog.h"
#include "settingsdialog.h"
#include "finddialog.h"
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
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <krecentfilesaction.h>
#include <kconfig.h>
#include <kcursor.h>
#include <KLocalizedString>
#include <KMessageBox>
#include <ktip.h>
#include <kactionmenu.h>
#include <kxmlguifactory.h>

// qt includes
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPushButton>
#include <QScrollBar>
#include <QSlider>
#include <QStatusBar>
#include <QStackedWidget>
#include <QTemporaryFile>
#include <QTimer>
#include <QToolButton>
#include <QUndoStack>
#include <QUndoView>
#include <QListWidget>

#include <cmath>

DEBUG_REGISTER(UMLApp)

/** Static pointer, holding the last created instance. */
UMLApp* UMLApp::s_instance;

/**
 * Static flag returned by static function shuttingDown().
 * It is intentionally an extra static member because it is queried
 * after calling the UMLApp destructor.
 * Member accesses shall be avoided during destruction.
 */
bool UMLApp::s_shuttingDown = false;

/**
 * Searches for a menu with the given name.
 * @todo This is an ugly _HACK_ to allow to compile umbrello.
 *       All the menu stuff should be ported to KDE4 (using actions)
 *
 * @param name  The name of the menu to search for (name :  not text)
 */
QMenu* UMLApp::findMenu(const QString& name)
{
    QWidget* widget = factory()->container(name, this);
    if (widget) {
        return dynamic_cast<QMenu*>(widget);
    }
    logDebug1("UMLApp::findMenu factory()->container(%1) returns NULL", name);
    return nullptr;
}

/**
 * Constructor. Calls all init functions to create the application.
 */
UMLApp::UMLApp(QWidget* parent)
  : KXmlGuiWindow(parent),
    m_d(nullptr),                // setup()
    m_langSelect(nullptr),
    m_zoomSelect(nullptr),
    m_activeLanguage(Uml::ProgrammingLanguage::Reserved),
    m_codegen(nullptr),
    m_commoncodegenpolicy(new CodeGenerationPolicy()),
    m_policyext(nullptr),
    m_config(KSharedConfig::openConfig()),
    m_view(nullptr),
    m_doc(nullptr),              // setup()
    m_listView(nullptr),
    m_mainDock(nullptr),
    m_listDock(nullptr),
    m_debugDock(nullptr),
    m_documentationDock(nullptr),
    m_cmdHistoryDock(nullptr),
    m_propertyDock(nullptr),
    m_logDock(nullptr),
    m_birdViewDock(nullptr),
    m_docWindow(nullptr),
    m_birdView(nullptr),
    m_pQUndoView(nullptr),
    m_refactoringAssist(nullptr),
    fileOpenRecent(nullptr),
    printPreview(nullptr),
    filePrint(nullptr),
    editCut(nullptr),
    editCopy(nullptr),
    editPaste(nullptr),
    editUndo(nullptr),
    editRedo(nullptr),
    viewShowTree(nullptr),
    viewShowDebug(nullptr),
    viewShowDoc(nullptr),
    viewShowLog(nullptr),
    viewShowCmdHistory(nullptr),
    viewShowBirdView(nullptr),
    newDiagram(nullptr),
    viewClearDiagram(nullptr),
    viewSnapToGrid(nullptr),
    viewShowGrid(nullptr),
    viewExportImage(nullptr),
    viewProperties(nullptr),
    zoom100Action(nullptr),
    deleteSelectedWidget(nullptr),
    deleteDiagram(nullptr),
    m_newSessionButton(nullptr),
    m_toolsbar(nullptr),
    m_clipTimer(nullptr),
    m_copyTimer(nullptr),
    m_loading(false),
    m_viewStack(nullptr),
    m_tabWidget(nullptr),
    m_layout(nullptr),
    m_imageMimeType(QString()),
    m_settingsDialog(nullptr),
    m_imageExporterAll(nullptr),  // setup()
    m_zoomValueLbl(nullptr),
    m_defaultZoomWdg(nullptr),
    m_pZoomOutPB(nullptr),
    m_pZoomInPB(nullptr),
    m_pZoomFitSBTB(nullptr),
    m_pZoomFullSBTB(nullptr),
    m_pZoomSlider(nullptr),
    m_statusBarMessage(nullptr),
    m_xhtmlGenerator(nullptr),
    m_pUndoStack(nullptr),        // setup()
    m_undoEnabled(true),
    m_hasBegunMacro(false),
    m_printSettings(nullptr),
    m_printer(nullptr)            // setup()
{
    for (int i = 0; i <= (int)Uml::ProgrammingLanguage::Reserved; i++)
        m_langAct[i] = nullptr;
}

/**
 * Set up the UMLApp.
 * To be called after the constructor, before anything else.
 * Heavy weight initializations are factored from the constructor to here
 * to avoid passing an UMLApp `this` pointer to other classes where the
 * `this` pointer has not been fully constructed. In other words, it is
 * safe for other classes to invoke UMLApp functions using the `this`
 * pointer passed to them.
 */
void UMLApp::setup()
{
    s_instance = this;
    m_d = new UMLAppPrivate(this);
    m_doc = new UMLDoc();
    m_imageExporterAll = new UMLViewImageExporterAll();
    m_pUndoStack = new QUndoStack(this);
    m_printer = new QPrinter();
    m_doc->init();
    m_printer->setFullPage(true);
    layout()->setSizeConstraint(QLayout::SetNoConstraint);

    readOptionState();
    initActions();
    // call this here because the statusBar is shown/hidden by setupGUI()
    initStatusBar();
    // use the absolute path to your umbrelloui.rc file for testing purpose in setupGUI();
    StandardWindowOptions opt = Default;
    opt &= ~Save;
    QString xmlFile = QStringLiteral(UMBRELLOUI_RC);
    QFileInfo fi(xmlFile);
    setupGUI(opt, fi.exists() ? xmlFile : QString());

    statusBar()->addWidget(m_statusBarMessage);
    statusBar()->addPermanentWidget(m_defaultZoomWdg);
    statusBar()->addPermanentWidget(m_pZoomOutPB);
    statusBar()->addPermanentWidget(m_pZoomSlider);
    statusBar()->addPermanentWidget(m_pZoomInPB);

    initView();
    initClip();
    readOptions();

    //get a reference to the Code->Active Language and to the Diagram->Zoom menu
    m_langSelect = findMenu(QStringLiteral("active_lang_menu"));
    //in case langSelect hasn't been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if someone takes the entry away from the ui.rc file
    if (m_langSelect == nullptr) {
        m_langSelect = new QMenu(QStringLiteral("active_lang_menu"), this);
    }

    m_zoomSelect = findMenu(QStringLiteral("zoom_menu"));
    //in case zoomSelect hasn't been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if some one takes the entry away from the ui.rc file
    if (m_zoomSelect == nullptr) {
        m_zoomSelect = new QMenu(QStringLiteral("zoom_menu"), this);
    }

    //connect zoomSelect menu
    connect(m_zoomSelect, SIGNAL(aboutToShow()), this, SLOT(setupZoomMenu()));
    connect(m_zoomSelect, SIGNAL(triggered(QAction*)), this, SLOT(slotSetZoom(QAction*)));

    setAutoSaveSettings();
    m_toolsbar->setToolButtonStyle(Qt::ToolButtonIconOnly);  // too many items for text, really we want a toolbox widget
}

/**
 * Standard deconstructor.
 */
UMLApp::~UMLApp()
{
    s_shuttingDown = true;
    disconnect(m_pZoomInPB, SIGNAL(clicked()), this, SLOT(slotZoomIn()));
    disconnect(m_pZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(slotZoomSliderMoved(int)));
    disconnect(m_tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(slotCloseDiagram(int)));
    disconnect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));
    disconnect(m_tabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotDiagramPopupMenu(QPoint)));

    delete m_birdView;
    delete m_clipTimer;
    delete m_copyTimer;
    delete m_commoncodegenpolicy;
    delete m_imageExporterAll;
    delete m_printer;
    delete m_policyext;
    delete m_pUndoStack;
    m_pUndoStack = nullptr;
    delete m_refactoringAssist;
    delete m_xhtmlGenerator;
    delete m_listView;
    delete m_doc;
    delete m_d;
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
void UMLApp::setProgLangAction(Uml::ProgrammingLanguage::Enum pl, const char* name, const char* action)
{
    m_langAct[pl] = actionCollection()->addAction(QString::fromLatin1(action));
    m_langAct[pl]->setText(QString::fromLatin1(name));
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
    fileOpenRecent = KStandardAction::openRecent(this, SLOT(slotFileOpenRecent(QUrl)), actionCollection());
    QAction* fileSave = KStandardAction::save(this, SLOT(slotFileSave()), actionCollection());
    QAction* fileSaveAs = KStandardAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
    QAction* fileClose = KStandardAction::close(this, SLOT(slotFileClose()), actionCollection());
    filePrint = KStandardAction::print(this, SLOT(slotFilePrint()), actionCollection());
    KStandardAction::find(this, SLOT(slotFind()), actionCollection());
    KStandardAction::findNext(this, SLOT(slotFindNext()), actionCollection());
    KStandardAction::findPrev(this, SLOT(slotFindPrevious()), actionCollection());
    printPreview = KStandardAction::printPreview(this, SLOT(slotPrintPreview()), actionCollection());
    filePrint->setPriority(QAction::LowPriority);  // icon only
    printPreview->setPriority(QAction::LowPriority);  // icon only
    QAction* fileQuit = KStandardAction::quit(this, SLOT(slotFileQuit()), actionCollection());

    editUndo = KStandardAction::undo(this, SLOT(slotEditUndo()), actionCollection());
    editRedo = KStandardAction::redo(this, SLOT(slotEditRedo()), actionCollection());

    editUndo->setPriority(QAction::LowPriority);   // icon only
    editRedo->setPriority(QAction::LowPriority);   // icon only

    disconnect(m_pUndoStack, SIGNAL(undoTextChanged(QString)), editUndo, nullptr);
    disconnect(m_pUndoStack, SIGNAL(redoTextChanged(QString)), editRedo, nullptr);

    editCut = KStandardAction::cut(this, SLOT(slotEditCut()), actionCollection());
    editCopy = KStandardAction::copy(this, SLOT(slotEditCopy()), actionCollection());
    editPaste = KStandardAction::paste(this, SLOT(slotEditPaste()), actionCollection());

    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);

    /* QAction* selectAll = */ KStandardAction::selectAll(this, SLOT(slotSelectAll()), actionCollection());

    QAction* fileExportDocbook = actionCollection()->addAction(QStringLiteral("file_export_docbook"));
    fileExportDocbook->setText(i18n("&Export model to DocBook"));
    connect(fileExportDocbook, SIGNAL(triggered(bool)), this, SLOT(slotFileExportDocbook()));

    QAction* fileExportXhtml = actionCollection()->addAction(QStringLiteral("file_export_xhtml"));
    fileExportXhtml->setText(i18n("&Export model to XHTML"));
    connect(fileExportXhtml, SIGNAL(triggered(bool)), this, SLOT(slotFileExportXhtml()));

    QAction* classWizard = actionCollection()->addAction(QStringLiteral("class_wizard"));
    classWizard->setText(i18n("&New Class Wizard..."));
    connect(classWizard, SIGNAL(triggered(bool)), this, SLOT(slotClassWizard()));

    QAction* addDefDatatypes = actionCollection()->addAction(QStringLiteral("create_default_datatypes"));
    addDefDatatypes->setText(i18n("&Add Default Datatypes for Active Language"));
    connect(addDefDatatypes, SIGNAL(triggered(bool)), this, SLOT(slotAddDefaultDatatypes()));

    QAction* preferences = KStandardAction::preferences(this, SLOT(slotPrefs()), actionCollection());

    QAction* impWizard = actionCollection()->addAction(QStringLiteral("importing_wizard"));
    impWizard->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Import_Files));
    impWizard->setText(i18n("Code &Importing Wizard..."));
    connect(impWizard, SIGNAL(triggered(bool)), this, SLOT(slotImportingWizard()));

    QAction* importProject = actionCollection()->addAction(QStringLiteral("import_project"));
    importProject->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Import_Project));
    importProject->setText(i18n("Import from Directory..."));
    connect(importProject, SIGNAL(triggered(bool)), this, SLOT(slotImportProject()));

    QAction* genWizard = actionCollection()->addAction(QStringLiteral("generation_wizard"));
    genWizard->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Export_Files));
    genWizard->setText(i18n("&Code Generation Wizard..."));
    connect(genWizard, SIGNAL(triggered(bool)), this, SLOT(slotExecGenerationWizard()));

    QAction* genAll = actionCollection()->addAction(QStringLiteral("generate_all"));
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
    setProgLangAction(Uml::ProgrammingLanguage::Reserved,     "none",            "setLang_none");

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
    connect(m_langAct[Uml::ProgrammingLanguage::Reserved],     SIGNAL(triggered()), this, SLOT(setLang_none()));

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
    editUndo->setToolTip(i18n("Undo last action"));
    editRedo->setToolTip(i18n("Redo last undone action"));

    preferences->setToolTip(i18n("Set the default program preferences"));

    deleteSelectedWidget = actionCollection()->addAction(QStringLiteral("delete_selected"));
    deleteSelectedWidget->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Delete));
    deleteSelectedWidget->setText(i18nc("delete selected widget", "Delete &Selected"));
    deleteSelectedWidget->setShortcut(QKeySequence(Qt::Key_Delete));
    connect(deleteSelectedWidget, SIGNAL(triggered(bool)), this, SLOT(slotDeleteSelected()));

    // The different views
    newDiagram = actionCollection()->add<KActionMenu>(QStringLiteral("new_view"));
    newDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_New));
    newDiagram->setText(QStringLiteral("new_view"));

    QAction* classDiagram = actionCollection()->addAction(QStringLiteral("new_class_diagram"));
    classDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Class));
    classDiagram->setText(i18n("&Class Diagram..."));
    connect(classDiagram, SIGNAL(triggered(bool)), this, SLOT(slotClassDiagram()));
    newDiagram->addAction(classDiagram);

#ifdef ENABLE_OBJECT_DIAGRAM
    QAction* objectDiagram = actionCollection()->addAction(QStringLiteral("new_object_diagram"));
    objectDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Object));
    objectDiagram->setText(i18n("&Object Diagram..."));
    connect(objectDiagram, SIGNAL(triggered()), this, SLOT(slotObjectDiagram()));
    newDiagram->addAction(objectDiagram);
#endif

    QAction* sequenceDiagram= actionCollection()->addAction(QStringLiteral("new_sequence_diagram"));
    sequenceDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Sequence));
    sequenceDiagram->setText(i18n("&Sequence Diagram..."));
    connect(sequenceDiagram, SIGNAL(triggered(bool)), this, SLOT(slotSequenceDiagram()));
    newDiagram->addAction(sequenceDiagram);

    QAction* collaborationDiagram = actionCollection()->addAction(QStringLiteral("new_collaboration_diagram"));
    collaborationDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Collaboration));
    collaborationDiagram->setText(i18n("C&ommunication Diagram..."));
    connect(collaborationDiagram, SIGNAL(triggered(bool)), this, SLOT(slotCollaborationDiagram()));
    newDiagram->addAction(collaborationDiagram);

    QAction* useCaseDiagram = actionCollection()->addAction(QStringLiteral("new_use_case_diagram"));
    useCaseDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Usecase));
    useCaseDiagram->setText(i18n("&Use Case Diagram..."));
    connect(useCaseDiagram, SIGNAL(triggered(bool)), this, SLOT(slotUseCaseDiagram()));
    newDiagram->addAction(useCaseDiagram);

    QAction* stateDiagram = actionCollection()->addAction(QStringLiteral("new_state_diagram"));
    stateDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_State));
    stateDiagram->setText(i18n("S&tate Diagram..."));
    connect(stateDiagram, SIGNAL(triggered(bool)), this, SLOT(slotStateDiagram()));
    newDiagram->addAction(stateDiagram);

    QAction* activityDiagram = actionCollection()->addAction(QStringLiteral("new_activity_diagram"));
    activityDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Activity));
    activityDiagram->setText(i18n("&Activity Diagram..."));
    connect(activityDiagram, SIGNAL(triggered(bool)), this, SLOT(slotActivityDiagram()));
    newDiagram->addAction(activityDiagram);

    QAction* componentDiagram = actionCollection()->addAction(QStringLiteral("new_component_diagram"));
    componentDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Component));
    componentDiagram->setText(i18n("Co&mponent Diagram..."));
    connect(componentDiagram, SIGNAL(triggered(bool)), this, SLOT(slotComponentDiagram()));
    newDiagram->addAction(componentDiagram);

    QAction* deploymentDiagram = actionCollection()->addAction(QStringLiteral("new_deployment_diagram"));
    deploymentDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Deployment));
    deploymentDiagram->setText(i18n("&Deployment Diagram..."));
    connect(deploymentDiagram, SIGNAL(triggered(bool)), this, SLOT(slotDeploymentDiagram()));
    newDiagram->addAction(deploymentDiagram);

    QAction* entityRelationshipDiagram = actionCollection()->addAction(QStringLiteral("new_entityrelationship_diagram"));
    entityRelationshipDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_EntityRelationship));
    entityRelationshipDiagram->setText(i18n("&Entity Relationship Diagram..."));
    connect(entityRelationshipDiagram, SIGNAL(triggered(bool)), this, SLOT(slotEntityRelationshipDiagram()));
    newDiagram->addAction(entityRelationshipDiagram);

    viewShowTree = actionCollection()->add<KToggleAction>(QStringLiteral("view_show_tree"));
    viewShowTree->setText(i18n("&Tree View"));
    connect(viewShowTree, SIGNAL(triggered(bool)), this, SLOT(slotShowTreeView(bool)));

    viewShowDebug = actionCollection()->add<KToggleAction>(QStringLiteral("view_show_debug"));
    viewShowDebug->setText(i18n("&Debugging"));
    connect(viewShowDebug, SIGNAL(triggered(bool)), this, SLOT(slotShowDebugView(bool)));

    viewShowDoc = actionCollection()->add<KToggleAction>(QStringLiteral("view_show_doc"));
    viewShowDoc->setText(i18n("&Documentation"));
    connect(viewShowDoc, SIGNAL(triggered(bool)), this, SLOT(slotShowDocumentationView(bool)));

    viewShowLog = actionCollection()->add<KToggleAction>(QStringLiteral("view_show_log"));
    viewShowLog->setText(i18n("&Logging"));
    connect(viewShowLog, SIGNAL(triggered(bool)), this, SLOT(slotShowLogView(bool)));

    viewShowCmdHistory = actionCollection()->add<KToggleAction>(QStringLiteral("view_show_undo"));
    viewShowCmdHistory->setText(i18n("&Command history"));
    connect(viewShowCmdHistory, SIGNAL(triggered(bool)), this, SLOT(slotShowCmdHistoryView(bool)));

    viewShowBirdView = actionCollection()->add<KToggleAction>(QStringLiteral("view_show_bird"));
    viewShowBirdView->setText(i18n("&Bird's eye view"));
    connect(viewShowBirdView, SIGNAL(triggered(bool)), this, SLOT(slotShowBirdView(bool)));

    viewClearDiagram = actionCollection()->addAction(QStringLiteral("view_clear_diagram"));
    viewClearDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Clear));
    viewClearDiagram->setText(i18n("&Clear Diagram"));
    connect(viewClearDiagram, SIGNAL(triggered(bool)), this, SLOT(slotCurrentViewClearDiagram()));

    viewSnapToGrid = actionCollection()->add<KToggleAction>(QStringLiteral("view_snap_to_grid"));
    viewSnapToGrid->setText(i18n("&Snap to Grid"));
    connect(viewSnapToGrid, SIGNAL(triggered(bool)), this, SLOT(slotCurrentViewToggleSnapToGrid()));

    viewShowGrid = actionCollection()->add<KToggleAction>(QStringLiteral("view_show_grid"));
    viewShowGrid->setText(i18n("S&how Grid"));
    connect(viewShowGrid, SIGNAL(triggered(bool)), this, SLOT(slotCurrentViewToggleShowGrid()));

    deleteDiagram = actionCollection()->addAction(QStringLiteral("view_delete"));
    deleteDiagram->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Delete));
    deleteDiagram->setText(i18n("&Delete Diagram"));
    connect(deleteDiagram, SIGNAL(triggered(bool)), this, SLOT(slotDeleteDiagram()));

    viewExportImage = actionCollection()->addAction(QStringLiteral("view_export_image"));
    viewExportImage->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Export_Picture));
    viewExportImage->setText(i18n("&Export as Picture..."));
    connect(viewExportImage, SIGNAL(triggered(bool)), this, SLOT(slotCurrentViewExportImage()));

    QAction* viewExportImageAll = actionCollection()->addAction(QStringLiteral("view_export_images"));
    viewExportImageAll->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Export_Picture));
    viewExportImageAll->setText(i18n("Export &Diagrams as Pictures..."));
    connect(viewExportImageAll, SIGNAL(triggered(bool)), this, SLOT(slotViewsExportImages()));

    viewProperties = actionCollection()->addAction(QStringLiteral("view_properties"));
    viewProperties->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Properties));
    viewProperties->setText(i18n("&Properties"));
    connect(viewProperties, SIGNAL(triggered(bool)), this, SLOT(slotCurrentProperties()));

    viewSnapToGrid->setChecked(false);
    viewShowGrid->setChecked(false);

    viewClearDiagram->setEnabled(false);
    viewSnapToGrid->setEnabled(false);
    viewShowGrid->setEnabled(false);
    deleteDiagram->setEnabled(false);
    viewExportImage->setEnabled(false);
    viewProperties->setEnabled(false);

    zoom100Action = actionCollection()->addAction(QStringLiteral("zoom100"));
    zoom100Action->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Zoom_100));
    zoom100Action->setText(i18n("Z&oom to 100%"));
    connect(zoom100Action, SIGNAL(triggered(bool)), this, SLOT(slotZoom100()));

    QAction* alignRight = actionCollection()->addAction(QStringLiteral("align_right"));
    alignRight->setText(i18n("Align Right"));
    alignRight->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Right));
    connect(alignRight, SIGNAL(triggered(bool)), this, SLOT(slotAlignRight()));

    QAction* alignLeft = actionCollection()->addAction(QStringLiteral("align_left"));
    alignLeft->setText(i18n("Align Left"));
    alignLeft->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Left));
    connect(alignLeft, SIGNAL(triggered(bool)), this, SLOT(slotAlignLeft()));

    QAction* alignTop = actionCollection()->addAction(QStringLiteral("align_top"));
    alignTop->setText(i18n("Align Top"));
    alignTop->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Top));
    connect(alignTop, SIGNAL(triggered(bool)), this, SLOT(slotAlignTop()));

    QAction* alignBottom = actionCollection()->addAction(QStringLiteral("align_bottom"));
    alignBottom->setText(i18n("Align Bottom"));
    alignBottom->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Bottom));
    connect(alignBottom, SIGNAL(triggered(bool)), this, SLOT(slotAlignBottom()));

    QAction* alignVerticalMiddle = actionCollection()->addAction(QStringLiteral("align_vertical_middle"));
    alignVerticalMiddle->setText(i18n("Align Vertical Middle"));
    alignVerticalMiddle->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_VerticalMiddle));
    connect(alignVerticalMiddle, SIGNAL(triggered(bool)), this, SLOT(slotAlignVerticalMiddle()));

    QAction* alignHorizontalMiddle = actionCollection()->addAction(QStringLiteral("align_horizontal_middle"));
    alignHorizontalMiddle->setText(i18n("Align Horizontal Middle"));
    alignHorizontalMiddle->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_HorizontalMiddle));
    connect(alignHorizontalMiddle, SIGNAL(triggered(bool)), this, SLOT(slotAlignHorizontalMiddle()));

    QAction* alignVerticalDistribute = actionCollection()->addAction(QStringLiteral("align_vertical_distribute"));
    alignVerticalDistribute->setText(i18n("Align Vertical Distribute"));
    alignVerticalDistribute->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_VerticalDistribute));
    connect(alignVerticalDistribute, SIGNAL(triggered(bool)), this, SLOT(slotAlignVerticalDistribute()));

    QAction* alignHorizontalDistribute = actionCollection()->addAction(QStringLiteral("align_horizontal_distribute"));
    alignHorizontalDistribute->setText(i18n("Align Horizontal Distribute"));
    alignHorizontalDistribute->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_HorizontalDistribute));
    connect(alignHorizontalDistribute, SIGNAL(triggered(bool)), this, SLOT(slotAlignHorizontalDistribute()));

    QString moveTabLeftString = i18n("&Move Tab Left");
    QString moveTabRightString = i18n("&Move Tab Right");
    QAction* moveTabLeft = actionCollection()->addAction(QStringLiteral("move_tab_left"));
    moveTabLeft->setIcon(Icon_Utils::SmallIcon(QApplication::layoutDirection() ? Icon_Utils::it_Go_Next : Icon_Utils::it_Go_Previous));
    moveTabLeft->setText(QApplication::layoutDirection() ? moveTabRightString : moveTabLeftString);
    
    moveTabLeft->setShortcut(QApplication::layoutDirection() ?
                 QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Right) : QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Left));
    connect(moveTabLeft, SIGNAL(triggered(bool)), this, SLOT(slotMoveTabLeft()));

    QAction* moveTabRight = actionCollection()->addAction(QStringLiteral("move_tab_right"));
    moveTabRight->setIcon(Icon_Utils::SmallIcon(QApplication::layoutDirection() ? Icon_Utils::it_Go_Previous : Icon_Utils::it_Go_Next));
    moveTabRight->setText(QApplication::layoutDirection() ? moveTabLeftString : moveTabRightString);
    moveTabRight->setShortcut(QApplication::layoutDirection() ?
                  QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Left) : QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Right));
    connect(moveTabRight, SIGNAL(triggered(bool)), this, SLOT(slotMoveTabRight()));

    QString selectTabLeftString = i18n("Select Diagram on Left");
    QString selectTabRightString = i18n("Select Diagram on Right");
    QAction* changeTabLeft = actionCollection()->addAction(QStringLiteral("previous_tab"));
    changeTabLeft->setText(QApplication::layoutDirection() ? selectTabRightString : selectTabLeftString);
    changeTabLeft->setShortcut(QApplication::layoutDirection() ?
                   QKeySequence(Qt::SHIFT | Qt::Key_Right) : QKeySequence(Qt::SHIFT | Qt::Key_Left));
    connect(changeTabLeft, SIGNAL(triggered(bool)), this, SLOT(slotChangeTabLeft()));

    QAction* changeTabRight = actionCollection()->addAction(QStringLiteral("next_tab"));
    changeTabRight->setText(QApplication::layoutDirection() ? selectTabLeftString : selectTabRightString);
    changeTabRight->setShortcut(QApplication::layoutDirection() ?
                    QKeySequence(Qt::SHIFT | Qt::Key_Left) : QKeySequence(Qt::SHIFT | Qt::Key_Right));
    connect(changeTabRight, SIGNAL(triggered(bool)), this, SLOT(slotChangeTabRight()));

// @todo Check if this should be ported
//     QMenu* menu = findMenu(QStringLiteral("settings"));
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
    QRectF items = currentView()->umlScene()->itemsBoundingRect();
    if (items.isNull()) {
        setZoom(100);
        return;
    }
    // TODO: QGraphicsView seems not to be informed about the scene rect update
    currentView()->setSceneRect(items);

    int scaleW = ceil(100.0 * currentView()->viewport()->width() / currentView()->umlScene()->width());
    int scaleH = ceil(100.0 * currentView()->viewport()->height() / currentView()->umlScene()->height());
    int scale = 100;
    if (scaleW < scaleH) {
        scale = scaleW;
    }
    else {
        scale = scaleH;
    }
    if (scale < 0)
        scale = 100;
    else if (scale > 500)
        scale = 500;
    else
        scale -= 2;
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
 * @param withView also setup the currently displayed diagram
 */
void UMLApp::setZoom(int zoom, bool withView)
{
    if (withView)
        currentView()->setZoom(zoom);
    bool oldState = m_pZoomSlider->blockSignals(true);
    m_pZoomSlider->setValue(zoom);
    m_pZoomSlider->blockSignals(oldState);
    m_zoomValueLbl->setText(QString::number(zoom) + QLatin1Char('%'));
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
    action->setText(i18nc("%1 percent value from 20 to 500", " &%1%", zoom));
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

    int currentZoom = currentView()->zoom();

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
 * initializing a statuslabel.
 */
void UMLApp::initStatusBar()
{
    connect(m_doc, SIGNAL(sigWriteToStatusBar(QString)), this, SLOT(slotStatusMsg(QString)));

    m_statusBarMessage = new QLabel(i18nc("init status bar", "Ready"));

    m_defaultZoomWdg = new QWidget(this);
    QHBoxLayout* zoomLayout = new QHBoxLayout(m_defaultZoomWdg);
    zoomLayout->setContentsMargins(0, 0, 0, 0);
    zoomLayout->setSpacing(0);
    zoomLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    m_zoomValueLbl = new QLabel(i18n("100%"));
    m_zoomValueLbl->setContentsMargins(10, 0, 10, 0);
    zoomLayout->addWidget(m_zoomValueLbl);

    m_pZoomFitSBTB = new StatusBarToolButton(this);
    m_pZoomFitSBTB->setText(i18n("Fit"));
    m_pZoomFitSBTB->setGroupPosition(StatusBarToolButton::GroupLeft);
    zoomLayout->addWidget(m_pZoomFitSBTB);
    m_pZoomFitSBTB->setContentsMargins(0, 0, 0, 0);
    //m_pZoomFitSBTB->setDisabled(true);
    connect(m_pZoomFitSBTB, SIGNAL(clicked()), this, SLOT(slotZoomFit()));

    m_pZoomFullSBTB = new StatusBarToolButton(this);
    m_pZoomFullSBTB->setText(i18n("100%"));
    m_pZoomFullSBTB->setGroupPosition(StatusBarToolButton::GroupRight);
    m_pZoomFullSBTB->setContentsMargins(0, 0, 0, 0);
    zoomLayout->addWidget(m_pZoomFullSBTB);
    connect(m_pZoomFullSBTB, SIGNAL(clicked()), this, SLOT(slotZoom100()));

    m_pZoomOutPB = new QPushButton(this);
    /* TODO: On the call to m_pZoomOutPB->setIcon Valgrind reports
       "Conditional jump or move depends on uninitialised value(s)".
     */
    m_pZoomOutPB->setIcon(QIcon(QStringLiteral("zoom-out")));
    m_pZoomOutPB->setFlat(true);
    m_pZoomOutPB->setMaximumSize(30, 30);
    connect(m_pZoomOutPB, SIGNAL(clicked()), this, SLOT(slotZoomOut()));

    m_pZoomSlider = new QSlider(Qt::Horizontal, this);
    m_pZoomSlider->setMaximumSize(100, 50);
    m_pZoomSlider->setMinimum (20);
    m_pZoomSlider->setMaximum (480);
    //m_pZoomSlider->setPageStep (1000);
    m_pZoomSlider->setValue (100);
    m_pZoomSlider->setContentsMargins(0, 0, 0, 0);
    connect(m_pZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(slotZoomSliderMoved(int)));

    m_pZoomInPB = new QPushButton(this);
    /* TODO: On the call to m_pZoomInPB->setIcon Valgrind reports
       "Conditional jump or move depends on uninitialised value(s)".
     */
    m_pZoomInPB->setIcon(QIcon(QStringLiteral("zoom-in")));
    m_pZoomInPB->setFlat(true);
    m_pZoomInPB->setMaximumSize(30, 30);
    connect(m_pZoomInPB, SIGNAL(clicked()), this, SLOT(slotZoomIn()));
}

/**
 * Creates the centerwidget of the KMainWindow instance and
 * sets it as the view.
 */
void UMLApp::initView()
{
    setCaption(m_doc->url().fileName(), false);
    m_view = nullptr;
    m_toolsbar = new WorkToolBar(this);
    m_toolsbar->setWindowTitle(i18n("Diagram Toolbar"));
    addToolBar(Qt::TopToolBarArea, m_toolsbar);

//     m_mainDock = new QDockWidget(this);
//     addDockWidget (Qt::RightDockWidgetArea, m_mainDock);
    m_newSessionButton = nullptr;

    // Prepare Stacked Diagram Representation
    m_viewStack = new QStackedWidget(this);

    // Prepare Tabbed Diagram Representation
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setMovable(true);
    connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(slotCloseDiagram(int)));
    connect(m_tabWidget, SIGNAL(currentChanged(int)), SLOT(slotTabChanged(int)));
    connect(m_tabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotDiagramPopupMenu(QPoint)));
    m_tabWidget->setTabsClosable(true);

    m_newSessionButton = new QToolButton(m_tabWidget);
    m_newSessionButton->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Tab_New));
    m_newSessionButton->adjustSize();
    m_newSessionButton->setAutoRaise(true);
    m_newSessionButton->setPopupMode(QToolButton::InstantPopup);
    m_newSessionButton->setMenu(newDiagram->menu());

    m_tabWidget->setCornerWidget(m_newSessionButton, Qt::TopLeftCorner);
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
    m_listDock = new QDockWidget(i18n("&Tree View"), this);
    m_listDock->setObjectName(QStringLiteral("TreeViewDock"));
    addDockWidget(Qt::LeftDockWidgetArea, m_listDock);
    m_listView = new UMLListView(m_listDock);
    //m_listView->setSorting(-1);
    m_listView->setDocument(m_doc);
    m_listView->init();
    m_listDock->setWidget(m_listView);
    connect(m_listDock, SIGNAL(visibilityChanged(bool)), viewShowTree, SLOT(setChecked(bool)));

    // create the documentation viewer
    m_documentationDock = new QDockWidget(i18n("Doc&umentation"), this);
    m_documentationDock->setObjectName(QStringLiteral("DocumentationDock"));
    addDockWidget(Qt::LeftDockWidgetArea, m_documentationDock);
    m_docWindow = new DocWindow(m_doc, m_documentationDock);
    m_docWindow->setObjectName(QStringLiteral("DOCWINDOW"));
    m_documentationDock->setWidget(m_docWindow);
    connect(m_documentationDock, SIGNAL(visibilityChanged(bool)), viewShowDoc, SLOT(setChecked(bool)));

    m_doc->setupSignals(); // make sure gets signal from list view

    // create the command history viewer
    m_cmdHistoryDock = new QDockWidget(i18n("Co&mmand history"), this);
    m_cmdHistoryDock->setObjectName(QStringLiteral("CmdHistoryDock"));
    addDockWidget(Qt::LeftDockWidgetArea, m_cmdHistoryDock);
    m_pQUndoView = new QUndoView(m_cmdHistoryDock);
    m_pQUndoView->setCleanIcon(Icon_Utils::SmallIcon(Icon_Utils::it_UndoView));
    m_pQUndoView->setStack(m_pUndoStack);
    m_cmdHistoryDock->setWidget(m_pQUndoView);
    connect(m_cmdHistoryDock, SIGNAL(visibilityChanged(bool)), viewShowCmdHistory, SLOT(setChecked(bool)));

    m_d->createDiagramsWindow();
#ifdef ENABLE_UML_OBJECTS_WINDOW
    m_d->createObjectsWindow();
#endif
    m_d->createStereotypesWindow();
    m_d->createWelcomeWindow();

    m_debugDock = new QDockWidget(i18n("&Debug"), this);
    m_debugDock->setObjectName(QStringLiteral("DebugDock"));
    addDockWidget(Qt::LeftDockWidgetArea, m_debugDock);
    m_debugDock->setWidget(Tracer::instance());
    connect(m_debugDock, SIGNAL(visibilityChanged(bool)), viewShowLog, SLOT(setChecked(bool)));

    // create the log viewer
    m_logDock = new QDockWidget(i18n("&Log"), this);
    m_logDock->setObjectName(QStringLiteral("LogDock"));
    addDockWidget(Qt::LeftDockWidgetArea, m_logDock);
    m_logDock->setWidget(m_d->logWindow);
    connect(m_logDock, SIGNAL(visibilityChanged(bool)), viewShowLog, SLOT(setChecked(bool)));

    // create the property viewer
    //m_propertyDock = new QDockWidget(i18n("&Properties"), this);
    //m_propertyDock->setObjectName(QStringLiteral("PropertyDock"));
    //addDockWidget(Qt::LeftDockWidgetArea, m_propertyDock);  //:TODO:

    // create the bird's eye view
    m_birdViewDock = new BirdViewDockWidget(i18n("&Bird's eye view"), this);
    m_birdViewDock->setObjectName(QStringLiteral("BirdViewDock"));
    addDockWidget(Qt::RightDockWidgetArea, m_birdViewDock);
    connect(m_birdViewDock, SIGNAL(visibilityChanged(bool)), viewShowBirdView, SLOT(setChecked(bool)));

    tabifyDockWidget(m_documentationDock, m_cmdHistoryDock);
    tabifyDockWidget(m_cmdHistoryDock, m_logDock);
    //tabifyDockWidget(m_cmdHistoryDock, m_propertyDock);  //:TODO:
    tabifyDockWidget(m_logDock, m_debugDock);
    tabifyDockWidget(m_listDock, m_d->stereotypesWindow);
    tabifyDockWidget(m_d->stereotypesWindow, m_d->diagramsWindow);
#ifdef ENABLE_UML_OBJECTS_WINDOW
    tabifyDockWidget(m_d->diagramsWindow, m_d->objectsWindow);
#endif
    if (m_d->welcomeWindow) {
        tabifyDockWidget(m_d->welcomeWindow, m_birdViewDock);
        m_d->welcomeWindow->raise();
    }
    m_listDock->raise();
}

/**
 * Opens a file specified by commandline option.
 */
void UMLApp::openDocumentFile(const QUrl& url)
{
    slotStatusMsg(i18n("Opening file..."));

    m_doc->openDocument(url);
    fileOpenRecent->addUrl(url);
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
    // The Toolbar settings will be handled by the respective classes (KToolBar)
    KConfigGroup cg(m_config, QStringLiteral("toolbar"));
    toolBar(QStringLiteral("mainToolBar"))->saveSettings(cg);
    KConfigGroup workBarConfig(m_config, QStringLiteral("workbar"));
    m_toolsbar->saveSettings(workBarConfig);
    fileOpenRecent->saveEntries(m_config->group(QStringLiteral("Recent Files")));

    KConfigGroup shortcutConfig(m_config, QStringLiteral("Shortcuts"));
    actionCollection()->writeSettings(&shortcutConfig, false);

    UmbrelloSettings::setGeometry(size());

    Settings::OptionState& optionState = Settings::optionState();
    optionState.save();

    if(m_doc->url().fileName() == i18n("Untitled")) {
        UmbrelloSettings::setLastFile(QString());
    }
    else {
        UmbrelloSettings::setLastFile(m_doc->url().toDisplayString());
    }

    UmbrelloSettings::setImageMimeType(imageMimeType());

    UmbrelloSettings::setShowDocWindow(m_documentationDock->isVisible());
//     CodeGenerator *codegen = getGenerator();
//     JavaCodeGenerator *javacodegen = dynamic_cast<JavaCodeGenerator*>(codegen);
//     if (javacodegen)
//         UmbrelloSettings::setBuildANTDocumentJava(javacodegen->getCreateANTBuildFile());

    // now write the basic defaults to config
    m_commoncodegenpolicy->writeConfig();

    UmbrelloSettings::self()->save();
}

/**
 * Read general Options again and initialize all variables
 * like the recent file list.
 */
void UMLApp::readOptions()
{
    // bar status settings
    KToolBar *mainToolBar = toolBar(QStringLiteral("mainToolBar"));
    mainToolBar->applySettings(m_config->group(QStringLiteral("toolbar")));

    // Add the Undo/Redo actions:
    // In KDE4 this was somehow done automatically but in KF5,
    // it seems we need to code it explicitly.

    mainToolBar->addSeparator();

    const QString undoIconName = Icon_Utils::toString(Icon_Utils::it_Undo);
    QIcon undoIcon = QIcon::fromTheme(undoIconName);
    editUndo->setIcon(undoIcon);
    mainToolBar->addAction(editUndo);

    const QString redoIconName = Icon_Utils::toString(Icon_Utils::it_Redo);
    QIcon redoIcon = QIcon::fromTheme(redoIconName);
    editRedo->setIcon(redoIcon);
    mainToolBar->addAction(editRedo);

    // do config for work toolbar
    m_toolsbar->applySettings(m_config->group(QStringLiteral("workbar")));
    fileOpenRecent->loadEntries(m_config->group(QStringLiteral("Recent Files")));
    setImageMimeType(UmbrelloSettings::imageMimeType());
    QSize size = UmbrelloSettings::geometry();
    if (size.width() == -1 && size.height() == -1)
        size = QApplication::desktop()->screenGeometry().size();
    resize(size);
    enableUndo(Settings::optionState().generalState.undo);

    KConfigGroup shortCutConfig(m_config, QStringLiteral("Shortcuts"));
    actionCollection()->readSettings(&shortCutConfig);
    m_toolsbar->setupActions();
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
    if (m_doc->url().fileName() == i18n("Untitled") || m_doc->isModified()) {
        QUrl url = m_doc->url();
        cfg.writePathEntry("filename", url.toString());
        cfg.writeEntry("modified", m_doc->isModified());
        DEBUG() << "Save properties - filenam: " << url << " | modified: " << m_doc->isModified();

        // saving to tempfile necessary
        QTemporaryFile tmpfile(url.toString());
        if (tmpfile.open()) {
            QUrl dest(QUrl::fromLocalFile(tmpfile.fileName()));
            m_doc->saveDocument(dest);
        }
    }
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
    QString filename = cfg.readPathEntry("filename", QString());
    QUrl url(filename);
    bool modified = cfg.readEntry("modified", false);
    DEBUG() << "Read properties - filename: " << url << " | modified: " << modified;
    if (modified) {
        QTemporaryFile tmpfile(filename);
        if (tmpfile.open()) {
            QUrl _url(QUrl::fromLocalFile(tmpfile.fileName()));
            m_doc->openDocument(_url);
            m_doc->setModified();
            enablePrint(true);
            setCaption(_url.fileName() + QStringLiteral(" [*]"), true);
        } else {
            enablePrint(false);
        }
    } else {
        if (!filename.isEmpty()) {
            m_doc->openDocument(url);
            enablePrint(true);
            setCaption(url.fileName(), false);
        } else {
            enablePrint(false);
        }
    }
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
    if (m_doc->saveModified()) {
        saveOptions();
        m_doc->closeDocument();
        return true;
    }
    return false;
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
        fileOpenRecent->setCurrentItem(-1);
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
        QUrl url = QFileDialog::getOpenFileUrl(this, i18n("Open File"), QUrl(),
            i18n("*.xmi *.xmi.tgz *.xmi.tar.bz2 *.uml *.mdl *.zargo|All Supported Files (*.xmi, *.xmi.tgz, *.xmi.tar.bz2, *.uml, *.mdl, *.zargo)\n"
                 "*.xmi|Uncompressed XMI Files (*.xmi)\n"
                 "*.xmi.tgz|Gzip Compressed XMI Files (*.xmi.tgz)\n"
                 "*.xmi.tar.bz2|Bzip2 Compressed XMI Files (*.xmi.tar.bz2)\n"
                 "*.uml|Eclipse PapyrusUML files (*.uml)\n"
                 "*.mdl|Rose model files (*.mdl)\n"
                 "*.zargo|Compressed argo Files(*.zargo)\n"
                 )
           .replace(QStringLiteral(","), QStringLiteral(""))
            );
        if (!url.isEmpty()) {
            m_listView->setSortingEnabled(false);
            if (m_doc->openDocument(url)) {
                fileOpenRecent->addUrl(url);
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
void UMLApp::slotFileOpenRecent(const QUrl &url)
{
    slotStatusMsg(i18n("Opening file..."));
    m_loading = true;

    QUrl oldUrl = m_doc->url();

    if (!m_doc->saveModified()) {
        // here saving wasn't successful
    }
    else {
        if (!m_doc->openDocument(url)) {
            fileOpenRecent->removeUrl(url);
            fileOpenRecent->setCurrentItem(-1);
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
    if (m_pUndoStack)
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
    QUrl url;
    QString ext;
    while (cont) {
        url = QFileDialog::getSaveFileUrl(this, i18n("Save As"), QUrl(),
                                          i18n("*.xmi | XMI File (*.xmi);;"
                                               "*.xmi.tgz | Gzip Compressed XMI File (*.xmi.tgz);;"
                                               "*.xmi.tar.bz2 | Bzip2 Compressed XMI File (*.xmi.tar.bz2);;"
                                               "* | All Files (*)"));
        if (url.isEmpty()) {
            break;
        }
        if (!url.isLocalFile()) {
            break;
        }
        QString file = url.toLocalFile();
        if (!file.contains(QStringLiteral("."))) {
            file.append(QStringLiteral(".xmi"));
            url = QUrl::fromLocalFile(file);
        }
        if (!QFile::exists(file)) {
            break;
        }
        int want_save = KMessageBox::warningContinueCancel(this, i18n("The file %1 exists.\nDo you wish to overwrite it?", file),
                                                           i18n("Warning"), KGuiItem(i18n("Overwrite")));
        if (want_save == KMessageBox::Continue) {
            cont = false;
        }
    }
    if (!url.isEmpty()) {
        bool b = m_doc->saveDocument(url);
        if (b) {
            fileOpenRecent->addUrl(url);
            setCaption(url.fileName(), m_doc->isModified());
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
    m_doc->diagramsModel()->removeAllDiagrams();
    slotFileNew();
}

/**
 * find text
 */
void UMLApp::slotFind()
{
    if (!m_d->findDialog.exec()) {
        UMLApp::app()->document()->writeToStatusBar(i18n("No search term entered"));
        return;
    }

    int count = m_d->findResults.collect(m_d->findDialog.filter(), m_d->findDialog.category(), m_d->findDialog.text());

    UMLApp::app()->document()->writeToStatusBar(i18n("'%1': %2 found", m_d->findDialog.text(), count));
    slotFindNext();
}

/**
 * Slot for showing next find result
 */
void UMLApp::slotFindNext()
{
    if (!m_d->findResults.displayNext())
        m_d->findResults.displayNext();
}

/**
 * Slot for showing previous find result
 */
void UMLApp::slotFindPrevious()
{
    if (!m_d->findResults.displayPrevious())
        m_d->findResults.displayPrevious();
}

/**
 * Slot for showing a print settings dialog.
 */
bool UMLApp::slotPrintSettings()
{
    if (m_printSettings) {
        delete m_printSettings;
    }
    m_printSettings = new DiagramPrintPage(nullptr, m_doc);
    QPointer<QDialog> dlg = new QDialog();
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_printSettings);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                       QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));
    layout->addWidget(buttonBox);

    dlg->setLayout(layout);
    bool result = dlg->exec() == QDialog::Accepted;
    // keep settings
    layout->removeWidget(m_printSettings);
    m_printSettings->setParent(nullptr);

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

    QPointer<QPrintPreviewDialog> preview = new QPrintPreviewDialog(m_printer, this);
    connect(preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPrintPreviewPaintRequested(QPrinter*)));
    preview->exec();
    delete preview;
    delete m_printSettings;
    m_printSettings = nullptr;
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

    QPointer<QPrintDialog> printDialog = new QPrintDialog(m_printer, this);
    printDialog->setWindowTitle(i18n("Print %1", m_doc->url().toDisplayString()));
    if (printDialog->exec()) {
        m_doc->print(m_printer, m_printSettings);
    }
    delete m_printSettings;
    m_printSettings = nullptr;
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
    QString path = QFileDialog::getExistingDirectory();
    if (path.isEmpty()) {
        return;
    }
    DocbookGenerator* docbookGenerator = new DocbookGenerator;
    docbookGenerator->generateDocbookForProjectInto(QUrl::fromLocalFile(path));
    connect(docbookGenerator, SIGNAL(finished(bool)), docbookGenerator, SLOT(deleteLater()));
}

/**
 * Exports the current model to XHTML in a subdir of the
 * current model directory named from the model name.
 * @todo Let the user chose the destination directory and
 * name, using network transparency.
 */
void UMLApp::slotFileExportXhtml()
{
    QString path = QFileDialog::getExistingDirectory();
    if (path.isEmpty()) {
        return;
    }

    if (!m_xhtmlGenerator) {
        m_xhtmlGenerator = new XhtmlGenerator;
    }
    m_xhtmlGenerator->generateXhtmlForProjectInto(QUrl::fromLocalFile(path));
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
    bool fromview = (currentView() && currentView()->umlScene()->selectedCount());

    // If not cutting diagram widgets, we did cut on a listview item
    if (!fromview) {
        m_listView->setStartedCut(true);
    }

    if (editCutCopy(fromview)) {
        Q_EMIT sigCutSuccessful();
        slotDeleteSelected();
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
    bool  fromview = (currentView() && currentView()->umlScene()->selectedCount());
    editCutCopy(fromview);
    resetStatusMsg();
    m_doc->setModified(true);
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
        KMessageBox::information(this, i18n("Umbrello could not paste the clipboard contents.  "
                                       "The objects in the clipboard may be of the wrong "
                                       "type to be pasted here."), i18n("Paste Error"));
    }
    resetStatusMsg();
    setCursor(Qt::ArrowCursor);
    editPaste->setEnabled(false);
    m_doc->setModified(true);
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
    m_statusBarMessage->setText(i18nc("reset status bar", "Ready."));
}

/**
 * Helper function to create diagram name and the diagram itself.
 * @param type   the type of diagram
 */
void UMLApp::createDiagram(Uml::DiagramType::Enum type)
{
    QString diagramName = m_doc->createDiagramName(type);
    if (!diagramName.isEmpty())
        executeCommand(new Uml::CmdCreateDiagram(m_doc, type, diagramName));
}

/**
 * Create this view.
 */
void UMLApp::slotClassDiagram()
{
    createDiagram(Uml::DiagramType::Class);
}

/**
 * Create this view.
 */
void UMLApp::slotObjectDiagram()
{
    createDiagram(Uml::DiagramType::Object);
}

/**
 * Create this view.
 */
void UMLApp::slotSequenceDiagram()
{
    createDiagram(Uml::DiagramType::Sequence);
}

/**
 * Create this view.
 */
void UMLApp::slotCollaborationDiagram()
{
    createDiagram(Uml::DiagramType::Collaboration);
}

/**
 * Create this view.
 */
void UMLApp::slotUseCaseDiagram()
{
    createDiagram(Uml::DiagramType::UseCase);
}

/**
 * Create this view.
 */
void UMLApp::slotStateDiagram()
{
    createDiagram(Uml::DiagramType::State);
}

/**
 * Create this view.
 */
void UMLApp::slotActivityDiagram()
{
    createDiagram(Uml::DiagramType::Activity);
}

/**
 * Create this view.
 */
void UMLApp::slotComponentDiagram()
{
    createDiagram(Uml::DiagramType::Component);
}

/**
 * Create this view.
 */
void UMLApp::slotDeploymentDiagram()
{
    createDiagram(Uml::DiagramType::Deployment);
}

/**
 * Create this view.
 */
void UMLApp::slotEntityRelationshipDiagram()
{
    createDiagram(Uml::DiagramType::EntityRelationship);
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
    currentView()->umlScene()->alignRight();
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
 * Returns the log window used.
 *
 * @return the log window being used
 */
QListWidget* UMLApp::logWindow() const
{
    return m_d->logWindow;
}

/**
 * Returns true if the environment variable UMBRELLO_LOG_TO_CONSOLE is
 * set to 1 or if the log dock is not visible.
 * The default is to print info/warnings/error messages to the log dock.
 *
 * @return True if warnings/errors shall be logged to the console.
 */
bool UMLApp::logToConsole() const
{
    return (Tracer::instance()->logToConsole() || !m_logDock || !m_logDock->isVisible());
}

/**
 * Adds a line to the log window.
 */
void UMLApp::log(const QString& s)
{
    logWindow()->addItem(s);
}

/**
 * Logs a debug message, either to the log window or to the console.
 * @todo This is not yet hooked up.
 *       Hooking it up entails vast changes because currently Umbrello uses
 *       the uDebug() stream and the stream usages (<<) need to be changed
 *       to normal function call syntax.
 */
void UMLApp::logDebug(const QString& s)
{
    QString fmt = QStringLiteral("[D] ") + s;
    if (logToConsole())
        uDebug() << fmt;
    else
        log(fmt);
}

/**
 * Logs an info message, either to the log window or to the console.
 * @todo This is not yet hooked up but only because Umbrello does not have
 *       a uInfo() stream analogous to uDebug / uWarning / uError, i.e.
 *       hooking up does not imply a change avalanche in the existing code
 *       and can be done as needed.
 */
void UMLApp::logInfo(const QString& s)
{
    QString fmt = QStringLiteral("[I] ") + s;
    if (logToConsole())
        uDebug() << fmt;   // @todo add Umbrello uInfo(), see uWarning etc
    else
        log(fmt);
}

/**
 * Logs a warning message, either to the log window or to the console.
 */
void UMLApp::logWarn(const QString& s)
{
    QString fmt = QStringLiteral("[W] ") + s;
    if (logToConsole())
        uWarning() << fmt;
    else
        log(fmt);
}

/**
 * Logs an error message, either to the log window or to the console.
 */
void UMLApp::logError(const QString& s)
{
    QString fmt = QStringLiteral("[E] ") + s;
    if (logToConsole())
        uError() << fmt;
    else
        log(fmt);
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

    //if anything else needs to be done on a modification, put it here

    // printing should be possible whenever there is something to print
    if (m_loading == false && modified == true && currentView())  {
        enablePrint(true);
    }

    if (m_loading == false)  {
        if (m_doc) {
            DEBUG() << "Modified file=" << m_doc->url().fileName();
            setCaption(m_doc->url().fileName(), modified); //add disk icon to taskbar if modified
        }
        else {
            DEBUG() << "m_doc is NULL!";
        }
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
    for(const QString &format: supportedFormats) {
        QByteArray fba = format.toLatin1();
        const char* f = fba.constData();
        if (!qstrnicmp(f,"application/x-uml-clip", 22)) {
            //FIXME need to test for clip1, clip2, clip3, clip4 or clip5
            //(the only valid clip types)
            return true;
        }
        else if (!qstrnicmp(f,"text/plain", 10)) {
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
    editPaste->setEnabled(data && canDecode(data));
}

/**
 * Slot for enabling cut and copy to clipboard.
 */
void UMLApp::slotCopyChanged()
{
    if (m_listView->selectedItemsCount() || (currentView() && currentView()->umlScene()->selectedCount())) {
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
void UMLApp::slotPrefs(MultiPageDialogBase::PageType page)
{
       Settings::OptionState& optionState = Settings::optionState();

       m_settingsDialog = new SettingsDialog(this, &optionState);
       m_settingsDialog->setCurrentPage(page);
       connect(m_settingsDialog, SIGNAL(applyClicked()), this, SLOT(slotApplyPrefs()));

       if (m_settingsDialog->exec() == QDialog::Accepted && m_settingsDialog->getChangesApplied()) {
           slotApplyPrefs();
       }

       delete m_settingsDialog;
       m_settingsDialog = nullptr;
}

/**
 * Commits the changes from the global preferences dialog.
 */
void UMLApp::slotApplyPrefs()
{
    if (m_settingsDialog) {
        // we need this to sync both values
        Settings::OptionState& optionState = Settings::optionState();
        enableUndo(optionState.generalState.undo);

        bool stackBrowsing = (m_layout->indexOf(m_tabWidget) != -1);
        bool tabBrowsing = optionState.generalState.tabdiagrams;
        DEBUG() << "stackBrowsing=" << stackBrowsing << " / tabBrowsing=" << tabBrowsing;

        if (stackBrowsing != tabBrowsing) {
            // Diagram Representation Modified
            UMLView* currentView;
            UMLViewList views = m_doc->viewIterator();

            if (tabBrowsing) {
                currentView = static_cast<UMLView*>(m_viewStack->currentWidget());
                m_layout->removeWidget(m_viewStack);
                m_viewStack->hide();

                for(UMLView  *view : views) {
                    UMLScene *scene = view->umlScene();
                    m_viewStack->removeWidget(view);
                    int tabIndex = m_tabWidget->addTab(view, scene->name());
                    m_tabWidget->setTabIcon(tabIndex, QIcon(Icon_Utils::iconSet(scene->type())));
                    m_tabWidget->setTabToolTip(tabIndex, scene->name());
                }
                m_layout->addWidget(m_tabWidget);
                m_tabWidget->show();
            }
            else {  // stackBrowsing
                currentView = static_cast<UMLView*>(m_tabWidget->currentWidget());
                m_layout->removeWidget(m_tabWidget);
                m_tabWidget->hide();

                for(UMLView  *view : views) {
                    m_tabWidget->removeTab(m_tabWidget->indexOf(view));
                    m_viewStack->addWidget(view);
                }
                m_layout->addWidget(m_viewStack);
                m_viewStack->show();
            }
            setCurrentView(currentView);
        }

        m_doc->settingsChanged(optionState);
        const QString plStr = m_settingsDialog->getCodeGenerationLanguage();
        Uml::ProgrammingLanguage::Enum pl = Uml::ProgrammingLanguage::fromString(plStr);
        setGenerator(pl);
    }
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
 * Returns the state of undo support.
 *
 * @return  True if undo is enabled.
 */
bool UMLApp::isUndoEnabled() const
{
    return m_undoEnabled;

}

/**
 * Set the state of undo support.
 *
 */
void UMLApp::enableUndo(bool enable)
{
    m_undoEnabled = enable;
    editRedo->setVisible(enable);
    editUndo->setVisible(enable);
    viewShowCmdHistory->setVisible(enable);
    clearUndoStack();
    slotShowCmdHistoryView(enable);
}

/**
 * Returns the undo state. Is used for popupmenu of a view.
 *
 * @return  True if Undo is enabled.
 */
bool UMLApp::isUndoActionEnabled() const
{
    return editUndo->isEnabled();
}

/**
 * Set whether to allow Undo.
 * It will enable/disable the menu/toolbar options.
 *
 * @param enable    Set whether to allow printing.
 */
void UMLApp::enableUndoAction(bool enable)
{
    editUndo->setEnabled(enable);
}

/**
 * Returns the redo state.
 *
 * @return  True if Redo is enabled. Is used for popupmenu of a view.
 */
bool UMLApp::isRedoActionEnabled() const
{
    return editRedo->isEnabled();
}

/**
 * Set whether to allow Redo.
 * It will enable/disable the menu/toolbar options.
 *
 * @param enable    Set whether to allow printing.
 */
void UMLApp::enableRedoAction(bool enable)
{
    editRedo->setEnabled(enable);
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
bool UMLApp::editCutCopy(bool bFromView)
{
    UMLClipboard clipboard;
    QMimeData  *clipdata = nullptr;

    // If not from-view, list items are copied. This flag is
    // used in UMLDragData to determine whether to assign new IDs
    if (!bFromView) {
        listView()->setStartedCopy(true);
    }

    if ((clipdata = clipboard.copy(bFromView)) != nullptr) {
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
void UMLApp::readOptionState() const
{
    Settings::OptionState& optionState = Settings::optionState();

    UmbrelloSettings::self()->load();
    optionState.load();
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
        AdvancedCodeGenerator *generator = dynamic_cast<AdvancedCodeGenerator*>(currentGen);
        if (generator) {
            CodeDocument *cdoc = generator->findCodeDocumentByClassifier(classifier);

            if (cdoc) {
                Settings::OptionState& optionState = Settings::optionState();
                CodeViewerDialog * dialog = generator->getCodeViewerDialog(this, cdoc, optionState.codeViewerState);
                dialog->exec();
                optionState.codeViewerState = dialog->state();
                delete dialog;
                dialog = nullptr;
            } else {
                // shouldn't happen..
                KMessageBox::information(nullptr, i18n("Cannot view code until you generate some first."), i18n("Cannot View Code"));
            }
        } else {
            KMessageBox::information(nullptr, i18n("Cannot view code from simple code writer."), i18n("Cannot View Code"));
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
        m_refactoringAssist = new RefactoringAssistant(m_doc, nullptr, nullptr,
                                                       QStringLiteral("refactoring_assistant"));
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
CodeGenerator *UMLApp::setGenerator(Uml::ProgrammingLanguage::Enum pl)
{
    if (m_codegen) {
        UMLFolder *dtFolder = m_doc->datatypeFolder();
        UMLObjectList dataTypes = dtFolder->containedObjects();
        UMLObjectList::Iterator end(dataTypes.end());
        if (m_codegen->language() == pl) {
            const QStringList predefTypes = m_codegen->defaultDatatypes();
            const Qt::CaseSensitivity cs =
                (activeLanguageIsCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
            for (UMLObjectList::Iterator it = dataTypes.begin(); it != end; ++it) {
                if (!predefTypes.contains((*it)->name(), cs)) {
                    m_doc->removeUMLObject(*it);
                }
            }
            return m_codegen;
        }
        for (UMLObjectList::Iterator it = dataTypes.begin(); it != end; ++it) {
            m_doc->removeUMLObject(*it);
        }
        delete m_codegen;  // ATTENTION! remove all refs to it or its policy first
        m_codegen = nullptr;
    }
    m_activeLanguage = pl;
    if (pl != Uml::ProgrammingLanguage::Reserved) {
        m_codegen = CodeGenFactory::createObject(pl);
    }
    updateLangSelectMenu(pl);

    slotAddDefaultDatatypes();
    if (pl != Uml::ProgrammingLanguage::Reserved) {
        m_codegen->createDefaultStereotypes();
        if (m_policyext) {
            m_policyext->setDefaults(false); // picks up language specific stuff
        }
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
bool UMLApp::isSimpleCodeGeneratorActive() const
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
    QPointer<CodeGenerationWizard> wizard = new CodeGenerationWizard(nullptr /*classList*/);
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

void UMLApp::setLang_none()
{
    setActiveLanguage(Uml::ProgrammingLanguage::Reserved);
}

/**
 * Called when right clicking on tab widget.
 * @param point  the point where the right mouse button was clicked
 */
void UMLApp::slotDiagramPopupMenu(const QPoint& point)
{
    QTabBar* tabBar = m_tabWidget->tabBar();
    int index = tabBar->tabAt(point);
    UMLView* view = (UMLView*)m_tabWidget->widget(index);
    QPoint globalPoint = m_tabWidget->mapToGlobal(point);
    m_doc->slotDiagramPopupMenu(view, globalPoint);
}

/**
 * Set the language for which code will be generated.
 *
 * @param pl    The name of the language to set
 */
void UMLApp::setActiveLanguage(Uml::ProgrammingLanguage::Enum pl)
{
    //updateLangSelectMenu(pl);  //:TODO:checkit - is already called in setGenerator
    setGenerator(pl);
}

/**
 * Get the language for import and code generation.
 */
Uml::ProgrammingLanguage::Enum UMLApp::activeLanguage() const
{
    return m_activeLanguage;
}

/**
 * Return true if the active language is case sensitive.
 */
bool UMLApp::activeLanguageIsCaseSensitive() const
{
    Uml::ProgrammingLanguage::Enum pl = activeLanguage();
    return Uml::ProgrammingLanguage::isCaseSensitive(pl);
}

/**
 * Return the target language depedent scope separator.
 */
QString UMLApp::activeLanguageScopeSeparator() const
{
    Uml::ProgrammingLanguage::Enum pl = activeLanguage();
    return Uml::ProgrammingLanguage::scopeSeparator(pl);
}

void UMLApp::slotShowTreeView(bool state)
{
    m_listDock->setVisible(state);
    viewShowTree->setChecked(state);
}

void UMLApp::slotShowDebugView(bool state)
{
    m_debugDock->setVisible(state);
    viewShowDebug->setChecked(state);
}

void UMLApp::slotShowDocumentationView(bool state)
{
    m_documentationDock->setVisible(state);
    viewShowDoc->setChecked(state);
}

void UMLApp::slotShowCmdHistoryView(bool state)
{
    m_cmdHistoryDock->setVisible(state);
    viewShowCmdHistory->setChecked(state);
}

void UMLApp::slotShowLogView(bool state)
{
    m_logDock->setVisible(state);
    viewShowLog->setChecked(state);
}

void UMLApp::slotShowBirdView(bool state)
{
    m_birdViewDock->setVisible(state);
    viewShowBirdView->setChecked(state);
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
    viewSnapToGrid->setChecked(currentView()->umlScene()->snapToGrid());
}

/**
 * Menu selection for current view show grid property.
 */
void UMLApp::slotCurrentViewToggleShowGrid()
{
    currentView()->umlScene()->toggleShowGrid();
    viewShowGrid->setChecked(currentView()->umlScene()->isSnapGridVisible());
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
void UMLApp::slotViewsExportImages()
{
    //delete m_printSettings;
    m_printSettings = new DiagramPrintPage(nullptr, m_doc);

    DiagramSelectionDialog dlg(m_printSettings);
    if (dlg.exec() == QDialog::Accepted)
        m_imageExporterAll->exportViews(m_printSettings);
}

/**
 * Menu selection for current view and contained widgets properties.
 */
void UMLApp::slotCurrentProperties()
{
    UMLWidgetList items = currentView()->umlScene()->selectedWidgets();
    if (items.count() == 0)
        currentView()->showPropertiesDialog();
    else if (items.count() == 1)
        items.at(0)->showPropertiesDialog();
}

/**
 * Sets the state of the view properties menu item.
 *
 * @param bState  Boolean, true to enable the view properties item.
 */
void UMLApp::setDiagramMenuItemsState(bool bState)
{
    viewClearDiagram->setEnabled(bState);
    viewSnapToGrid->setEnabled(bState);
    viewShowGrid->setEnabled(bState);
    deleteDiagram->setEnabled(bState);
    viewExportImage->setEnabled(bState);
    viewProperties->setEnabled(bState);
    filePrint->setEnabled(bState);
    if (currentView()) {
        viewSnapToGrid->setChecked(currentView()->umlScene()->snapToGrid());
        viewShowGrid->setChecked(currentView()->umlScene()->isSnapGridVisible());
    }
}

/**
 * Register new views (aka diagram) with the GUI so they show up
 * in the menu.
 */
void UMLApp::slotUpdateViews()
{
    QMenu* menu = findMenu(QStringLiteral("views"));
    if (!menu) {
        uWarning() << "view menu not found";
        return;
    }

    menu = findMenu(QStringLiteral("show_view"));
    if (!menu) {
        uWarning() << "show menu not found";
        return;
    }

    menu->clear();

    UMLViewList views = m_doc->viewIterator();
    for(UMLView  *view : views) {
        menu->addAction(view->umlScene()->name(), view->umlScene(), SLOT(slotShowView()));
        view->umlScene()->fileLoaded();
    }
}

/**
 * Import the source files that are in fileList.
 */
void UMLApp::importFiles(QStringList &fileList, const QString &rootPath)
{
    if (!fileList.isEmpty()) {
        bool really_visible = !listView()->parentWidget()->visibleRegion().isEmpty();
        bool saveState = listView()->parentWidget()->isVisible();
        listView()->parentWidget()->setVisible(false);
        logWindow()->parentWidget()->setVisible(true);
        logWindow()->clear();

        const QString& firstFile = fileList.first();
        ClassImport *classImporter = ClassImport::createImporterByFileExt(firstFile);
        classImporter->setRootPath(rootPath);
        classImporter->importFiles(fileList);
        delete classImporter;
        m_doc->setLoading(false);
        // Modification is set after the import is made, because the file was modified when adding the classes.
        // Allowing undo of the whole class importing. I think it eats a lot of memory.
        // Setting the modification, but without allowing undo.
        m_doc->setModified(true);
        listView()->setUpdatesEnabled(true);
        logWindow()->setUpdatesEnabled(true);
        listView()->parentWidget()->setVisible(saveState);
        if (really_visible)
            m_listDock->raise();
    }
}

/**
 * Import class menu selection.
 */
void UMLApp::slotImportClass()
{
    QStringList filters = Uml::ProgrammingLanguage::toExtensions(UMLApp::app()->activeLanguage());
    QString f = filters.join(QStringLiteral(" ")) + QStringLiteral("|") +
                             Uml::ProgrammingLanguage::toExtensionsDescription(UMLApp::app()->activeLanguage());

    QStringList files = QFileDialog::getOpenFileNames(this, i18n("Select file(s) to import:"), QString(), f);
    if (!files.isEmpty()) {
        importFiles(files);
    }
}

/**
 * @brief getFiles
 * @param files
 * @param path
 * @param filters
 */
void getFiles(QStringList& files, const QString& path, QStringList& filters)
{
    QDir searchDir(path);
    if (searchDir.exists()) {
        for (const QString &file: searchDir.entryList(QDir::Files))
            files.append(searchDir.absoluteFilePath(file));
        for (const QString &subDir: searchDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks))
            getFiles(files, searchDir.absoluteFilePath(subDir), filters);
    }
}

/**
 * Import project menu selection.
 */
void UMLApp::slotImportProject()
{
    QStringList listFile;
    QString dir = QFileDialog::getExistingDirectory(this, i18n("Select directory to import:"));
    if (!dir.isEmpty()) {
        QStringList filter = Uml::ProgrammingLanguage::toExtensions(UMLApp::app()->activeLanguage());
        getFiles(listFile, dir, filter);
        importFiles(listFile, dir);
    }
}

/**
 * Runs the code importing wizard.
 */
void UMLApp::slotImportingWizard()
{
    QPointer<CodeImportingWizard> wizard = new CodeImportingWizard();
    wizard->setupPages();
    wizard->exec();
    delete wizard;
}

/**
 * Class wizard menu selection.
 */
void UMLApp::slotClassWizard()
{
    QPointer<ClassWizard> dlg  = new ClassWizard(m_doc);
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
 * Deletes selected widgets or list view items.
 */
void UMLApp::slotDeleteSelected()
{
    // deleteSelectedWidget grabs DEL key as shortcut,
    // which prevents routing DEL key through the regular
    // key press event handler
    QWidget *f = focusWidget();
    if (f == m_listView) {
        QWidgetAction *o = static_cast<QWidgetAction *>(sender());
        if (o && o->objectName() == QStringLiteral("delete_selected")) {
            m_listView->slotDeleteSelectedItems();
        }
        return;
    }
    if (currentView()) {
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
    m_doc->removeDiagram(currentView()->umlScene()->ID());
}

/**
 * Close the current diagram. Clicked on tab close button.
 * @param index   widget's index to close
 */
void UMLApp::slotCloseDiagram(int index)
{
    UMLView* view = (UMLView*)m_tabWidget->widget(index);
    if (view) {
        if (view != currentView()) {
            setCurrentView(view);
        }
        m_tabWidget->removeTab(index);
        view->umlScene()->setIsOpen(false);
    }
}

/**
 * Return the default code generation language as configured by KConfig.
 * If the activeLanguage is not found in the KConfig then use Uml::ProgrammingLanguage::Cpp
 * as the default.
 */
Uml::ProgrammingLanguage::Enum UMLApp::defaultLanguage() const
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
        m_codegen = nullptr;
    }
    Uml::ProgrammingLanguage::Enum defLanguage = defaultLanguage();
    setActiveLanguage(defLanguage);
}

/**
 * Updates the Menu for language selection and sets the
 * active language. If no active language is found or if it is
 * not one of the registered languages it tries to fall back
 * to Cpp
 */
void UMLApp::updateLangSelectMenu(Uml::ProgrammingLanguage::Enum activeLanguage)
{
    //m_langSelect->clear();
    for (int i = 0; i <= Uml::ProgrammingLanguage::Reserved; ++i) {
        m_langAct[i]->setChecked(i == activeLanguage);
    }
}

/**
 * Return true during shutdown, i.e. during ~UMLApp().
 */
bool UMLApp::shuttingDown()
{
    return s_shuttingDown;
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
    if (m_view == nullptr || !m_view->umlScene()->selectedCount() || e->modifiers() != Qt::AltModifier) {
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
QWidget* UMLApp::mainViewWidget() const
{
    Settings::OptionState& optionState = Settings::optionState();
    if (optionState.generalState.tabdiagrams) {
        return m_tabWidget;
    }
    else {
        return m_viewStack;
    }
}

/**
 * Create bird's view window in a dock widget.
 */
void UMLApp::createBirdView(UMLView *view)
{
    if (m_birdView) {
        delete m_birdView;
    }
    m_birdView = new BirdView(m_birdViewDock, view);
    connect(m_birdView, SIGNAL(viewPositionChanged(QPointF)), this, SLOT(slotBirdViewChanged(QPointF)));
    connect(m_birdViewDock, SIGNAL(sizeChanged(QSize)), m_birdView, SLOT(slotDockSizeChanged(QSize)));
}

/**
 * Slot for changes of the bird view's rectangle by moving.
 * @param delta   change value for a move
 */
void UMLApp::slotBirdViewChanged(const QPointF& delta)
{
    m_birdView->setSlotsEnabled(false);
    UMLView* view = currentView();
    QPointF oldCenter = view->mapToScene(view->viewport()->rect().center());
    QPointF newCenter = oldCenter + delta;
    view->centerOn(newCenter);
    // DEBUG() << "view moved with: " << delta;
    m_birdView->setSlotsEnabled(true);
}

/**
 * Puts this view to the top of the viewStack, i.e. makes it
 * visible to the user.
 *
 * @param view   Pointer to the UMLView to push.
 * @param updateTreeView   A false value disables updating of the tree view
 */
void UMLApp::setCurrentView(UMLView* view, bool updateTreeView)
{
    m_view = view;
    if (view == nullptr) {
        DEBUG() << "view is NULL";
        docWindow()->reset();
        return;
    }

    Settings::OptionState optionState = Settings::optionState();
    if (optionState.generalState.tabdiagrams) {
        int tabIndex = m_tabWidget->indexOf(view);
        if ((tabIndex < 0) && (view->umlScene()->isOpen())) {
            tabIndex = m_tabWidget->addTab(view, view->umlScene()->name());
            m_tabWidget->setTabIcon(tabIndex, QIcon(Icon_Utils::iconSet(view->umlScene()->type())));
            m_tabWidget->setTabToolTip(tabIndex, view->umlScene()->name());
        }
        if (!updateTreeView)
            disconnect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));
        m_tabWidget->setCurrentIndex(tabIndex);
        if (!updateTreeView)
            connect(m_tabWidget, SIGNAL(currentChanged(int)), SLOT(slotTabChanged(int)));
    }
    else {
        if (m_viewStack->indexOf(view) < 0) {
            m_viewStack->addWidget(view);
        }
        m_viewStack->setCurrentWidget(view);
        view->show();
    }
    setZoom(view->zoom());
    slotStatusMsg(view->umlScene()->name());
    if (updateTreeView) {
        UMLListViewItem* lvitem = m_listView->findView(view);
        if (lvitem) {
            m_listView->setCurrentItem(lvitem);
        }
    }
    DEBUG() << "Changed view to" << view->umlScene();

    createBirdView(view);
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
 * @param index   the index of the changed tab widget
 */
void UMLApp::slotTabChanged(int index)
{
    UMLView* view = (UMLView*)m_tabWidget->widget(index);
    if (view && !m_doc->closing() && !s_shuttingDown) {
        m_doc->changeCurrentView(view->umlScene()->ID());
    }
}

/**
 * Make the tab on the left of the current one the active one.
 */
void UMLApp::slotChangeTabLeft()
{
    //DEBUG() << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    if (Settings::optionState().generalState.tabdiagrams && m_tabWidget) {
        m_tabWidget->setCurrentIndex(m_tabWidget->currentIndex() - 1);
        return;
    }
    UMLViewList views = m_doc->viewIterator();
    UMLView *currView = m_view;
    int viewIndex = 0;
    if ((viewIndex = views.indexOf(currView)) < 0) {
        uError() << "currView not found in viewlist";
        return;
    }
    UMLView *prevView = nullptr;
    if (viewIndex != 0) {
        prevView = views.begin()[viewIndex -1 ];
    }

    if ((currView = prevView) != nullptr) {
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
    //DEBUG() << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    if (Settings::optionState().generalState.tabdiagrams && m_tabWidget) {
        m_tabWidget->setCurrentIndex(m_tabWidget->currentIndex() + 1);
        return;
    }
    UMLViewList views = m_doc->viewIterator();
    UMLView *currView = m_view;
    int viewIndex = 0;
    if ((viewIndex = views.indexOf(currView)) < 0) {
        uError() << "currView not found in viewlist";
        return;
    }
    UMLView *nextView = nullptr;
    if (viewIndex < views.count()-1) {
        nextView = views.begin()[viewIndex + 1];
        setCurrentView(nextView);
    }
    else
        setCurrentView(views.first());
}

/* for debugging only
static void showTabTexts(KTabWidget* tabWidget)
{
    QString out = QStringLiteral("tab texts ");
    for (int i = 0; i < tabWidget->count(); ++i) {
        out += " <" + tabWidget->tabText(i) + '>';
    }
    DEBUG() << out;
}
*/

/**
 * Move the current tab left.
 */
void UMLApp::slotMoveTabLeft()
{
    //DEBUG() << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    //showTabTexts(m_tabWidget);
    int from = m_tabWidget->currentIndex();
    int to   = -1;
    if (from > 0) {
        to = from - 1;
    }
    else {
        to = m_tabWidget->count() - 1;
    }
    m_tabWidget->tabBar()->moveTab(from, to);
}

/**
 * Move the current tab right.
 */
void UMLApp::slotMoveTabRight()
{
    //DEBUG() << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    //showTabTexts(m_tabWidget);
    int from = m_tabWidget->currentIndex();
    int to   = -1;
    if (from < m_tabWidget->count() - 1) {
        to = from + 1;
    }
    else {
        to = 0;
    }
    m_tabWidget->tabBar()->moveTab(from, to);
}

/**
 * This slot deletes the current XHTML documentation generator as soon as
 * this one signals that it has finished.
 * @param status true if successful else false
 */
void UMLApp::slotXhtmlDocGenerationFinished(bool status)
{
  if (!status) {
      m_doc->writeToStatusBar(i18n("XHTML Generation failed ."));
  }

  delete m_xhtmlGenerator;
  m_xhtmlGenerator = nullptr;
}

/**
 * open file in internal editor
 * @param file path to the file to open
 * @param startCursor cursor position for selection start
 * @param endCursor cursor position for selection end
 * @return true file could be loaded
 * @return false file could not be loaded
 */
bool UMLApp::slotOpenFileInEditor(const QUrl &file, int startCursor, int endCursor)
{
    return m_d->openFileInEditor(file, startCursor, endCursor);
}

/**
 * Return the tab widget.
 */
QTabWidget* UMLApp::tabWidget()
{
    return m_tabWidget;
}

/**
 * Returns the current text in the status bar.
 *
 * @return The text in the status bar.
 */
QString UMLApp::statusBarMsg() const
{
    return m_statusBarMessage->text();
}

/**
 * Removes all entries from the UndoStack and RedoStack and disables the
 * undo and redo actions.
 */
void UMLApp::clearUndoStack()
{
    if (m_pUndoStack)
        m_pUndoStack->clear();
}

/**
 * Undo last command. Is called from popupmenu of a view.
 */
void UMLApp::undo()
{
    if (!m_pUndoStack)
        return;

    if (!isUndoEnabled())
        return;

    DEBUG() << m_pUndoStack->undoText() << " [" << m_pUndoStack->count() << "]";
    m_pUndoStack->undo();

    if (m_pUndoStack->canUndo()) {
        UMLApp::app()->enableUndoAction(true);
    }
    else {
        UMLApp::app()->enableUndoAction(false);
    }

    UMLApp::app()->enableRedoAction(true);
}

/**
 * Redo last 'undone' command. Is called from popupmenu of a view.
 */
void UMLApp::redo()
{
    if (!m_pUndoStack)
        return;

    if (!isUndoEnabled())
        return;

    DEBUG() << m_pUndoStack->redoText() << " [" << m_pUndoStack->count() << "]";
    m_pUndoStack->redo();

    if (m_pUndoStack->canRedo()) {
        UMLApp::app()->enableRedoAction(true);
    }
    else {
        UMLApp::app()->enableRedoAction(false);
    }

    UMLApp::app()->enableUndoAction(true);
}

/**
 * Execute a command and push it on the undo stack.
 */
void UMLApp::executeCommand(QUndoCommand* cmd)
{
    if (!m_pUndoStack)
        return;

    if (cmd == nullptr)
        return;
    if (isUndoEnabled()) {
        m_pUndoStack->push(cmd);
        DEBUG() << cmd->text() << " [" << m_pUndoStack->count() << "]";
        UMLApp::app()->enableUndoAction(true);
    } else {
        cmd->redo();
        delete cmd;
    }

    m_doc->setModified(true);
}

/**
 * Begin a U/R command macro
 */
void UMLApp::beginMacro(const QString & text)
{
    if (!m_pUndoStack)
        return;

    if (!isUndoEnabled()) {
        return;
    }
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
    if (!m_pUndoStack)
        return;

    if (!isUndoEnabled()) {
        return;
    }
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

