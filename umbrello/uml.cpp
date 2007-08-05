/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "uml.h"

// qt includes
#include <qclipboard.h>
#include <qtimer.h>
#include <qslider.h>
#include <qregexp.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QFrame>
#include <QMenuItem>
#include <QDockWidget>
#include <QStackedWidget>

// kde includes
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <krecentfilesaction.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kprinter.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <ktip.h>
#include <ktabwidget.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kxmlguifactory.h>
#include <kapplication.h>
// app includes
#include "codeimport/classimport.h"
#include "docwindow.h"
#include "codegenerator.h"
#include "codegenerationpolicy.h"
#include "codegenerators/codegenfactory.h"
#include "codegenerators/codegenpolicyext.h"
#include "optionstate.h"
#include "widget_utils.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlviewlist.h"
#include "worktoolbar.h"
#include "model_utils.h"
#include "clipboard/umlclipboard.h"
#include "dialogs/classwizard.h"
#include "dialogs/codegenerationwizard.h"
#include "dialogs/codeviewerdialog.h"
#include "dialogs/diagramprintpage.h"
#include "dialogs/importprojectdlg.h"

#include "refactoring/refactoringassistant.h"
#include "codegenerators/simplecodegenerator.h"
#include "umlviewimageexporter.h"
#include "umlviewimageexporterall.h"

#include "kplayerslideraction.h"

#include "configurable.h"

#include "cmdlineexportallviewsevent.h"

#include "docgenerators/docbookgenerator.h"
#include "docgenerators/xhtmlgenerator.h"

#include "cmds.h"
#include "umbrellosettings.h"

/// @todo This is an ugly _HACK_ to allow to compile umbrello.
/// All the menu stuff should be ported to KDE4 (using actions)
QMenu* UMLApp::findMenu(KMenuBar* /*menu*/, const QString &name)
{
    QWidget* widget = factory()->container(name, this);
    if (widget)
        return dynamic_cast<QMenu*>(widget);
    kDebug() << "UMLApp::findMenu(KMenuBar*): factory()->container(" << name << ") returns NULL";
    return 0;
}

UMLApp::UMLApp(QWidget* parent) : KXmlGuiWindow(parent) {
    s_instance = this;
    m_pDocWindow = 0;
    m_config = KGlobal::config();
    m_listView = 0;
    m_langSelect = NULL;
    m_zoomSelect = NULL;
    m_loading = false;
    m_clipTimer = 0;
    m_copyTimer = 0;
    m_codegen = 0;
    m_policyext = 0;
    m_commoncodegenpolicy = 0;
    m_xhtmlGenerator = 0;
    m_activeLanguage = Uml::pl_Reserved;
    ///////////////////////////////////////////////////////////////////
    // call inits to invoke all other construction parts
    readOptionState();
    m_pUndoStack = new KUndoStack(this);
    m_doc = new UMLDoc();
    m_doc->init();
    m_hasBegunMacro = false;
    initActions(); //now calls initStatusBar() because it is affected by setupGUI()
    initView();
    initClip();
    readOptions();
    ///////////////////////////////////////////////////////////////////
    // disable actions at startup
    fileSave->setEnabled(true);
    fileSaveAs->setEnabled(true);
    enablePrint(false);
    editCut->setEnabled(false);
    editCopy->setEnabled(false);
    editPaste->setEnabled(false);
    editUndo->setEnabled(false);
    editRedo->setEnabled(false);

    //get a reference to the Code->Active Language and to the Diagram->Zoom menu
    QMenu* menu = findMenu( menuBar(), QString("code") );
    m_langSelect = findMenu( menu, QString("active_lang_menu") );

    //in case langSelect hasn't been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if someone takes the entry away from the ui.rc file
    if (m_langSelect == NULL) {
        m_langSelect = new QMenu( QString("active_lang_menu"), this );
    }

    menu = findMenu( menuBar(), QString("views") );
    m_zoomSelect = findMenu( menu, QString("zoom_menu") );

    //in case zoomSelect hasn't been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if some one takes the entry away from the ui.rc file
    if (m_zoomSelect == NULL) {
        m_zoomSelect = new QMenu( QString("zoom_menu"), this );
    }

    //connect zoomSelect menu
    m_zoomSelect->setCheckable(true);
    connect(m_zoomSelect,SIGNAL(aboutToShow()),this,SLOT(setupZoomMenu()));
    connect(m_zoomSelect,SIGNAL(activated(int)),this,SLOT(setZoom(int)));

    m_refactoringAssist = 0L;

    m_commoncodegenpolicy = new CodeGenerationPolicy();

    m_imageExporterAll = new UMLViewImageExporterAll();
}

UMLApp::~UMLApp() {
    delete m_imageExporterAll;

    delete m_clipTimer;
    delete m_copyTimer;

    delete m_refactoringAssist;

    delete m_pUndoStack;
}

UMLApp* UMLApp::app()
{
    return s_instance;
}

void UMLApp::initActions() {
    fileNew = KStandardAction::openNew(this, SLOT(slotFileNew()), actionCollection());
    fileOpen = KStandardAction::open(this, SLOT(slotFileOpen()), actionCollection());
    fileOpenRecent = KStandardAction::openRecent(this, SLOT(slotFileOpenRecent(const KUrl&)), actionCollection());
    fileSave = KStandardAction::save(this, SLOT(slotFileSave()), actionCollection());
    fileSaveAs = KStandardAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
    fileClose = KStandardAction::close(this, SLOT(slotFileClose()), actionCollection());
    filePrint = KStandardAction::print(this, SLOT(slotFilePrint()), actionCollection());
    fileQuit = KStandardAction::quit(this, SLOT(slotFileQuit()), actionCollection());

    editUndo = m_pUndoStack->createUndoAction(actionCollection());
    editRedo = m_pUndoStack->createRedoAction(actionCollection());

    disconnect( m_pUndoStack, SIGNAL( undoTextChanged(const QString& ) ),editUndo, 0 );
    disconnect( m_pUndoStack, SIGNAL( redoTextChanged(const QString& ) ),editRedo, 0 );

    editCut = KStandardAction::cut(this, SLOT(slotEditCut()), actionCollection());
    editCopy = KStandardAction::copy(this, SLOT(slotEditCopy()), actionCollection());
    editPaste = KStandardAction::paste(this, SLOT(slotEditPaste()), actionCollection());
    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);
    selectAll = KStandardAction::selectAll(this,  SLOT( slotSelectAll() ), actionCollection());

    fileExportDocbook = actionCollection()->addAction("file_export_docbook");
    fileExportDocbook->setText(i18n("&Export model to DocBook"));
    connect(fileExportDocbook, SIGNAL( triggered( bool ) ), this, SLOT( slotFileExportDocbook() ));

    fileExportXhtml = actionCollection()->addAction("file_export_xhtml");
    fileExportXhtml->setText(i18n("&Export model to XHTML"));
    connect(fileExportXhtml, SIGNAL( triggered( bool ) ), this, SLOT( slotFileExportXhtml() ));

    classWizard = actionCollection()->addAction("class_wizard");
    classWizard->setText(i18n("&New Class Wizard..."));
    connect(classWizard, SIGNAL( triggered( bool ) ), this, SLOT( slotClassWizard() ));

    QAction* anAction = actionCollection()->addAction("create_default_datatypes");
    anAction->setText(i18n("&Add Default Datatypes for Active Language"));
    connect(anAction, SIGNAL( triggered( bool ) ), this, SLOT( slotAddDefaultDatatypes() ));

    preferences = KStandardAction::preferences(this,  SLOT( slotPrefs() ), actionCollection());

    importClasses = actionCollection()->addAction("import_class");
    importClasses->setIcon(KIcon("source-cpp"));
    importClasses->setText(i18n("&Import Classes..."));
    connect(importClasses, SIGNAL( triggered( bool ) ), this, SLOT( slotImportClasses() ));

    importProject = actionCollection()->addAction("import_project");
    importProject->setIcon(KIcon("source-cpp"));
    importProject->setText(i18n("Import &Project..."));
    connect(importProject, SIGNAL( triggered( bool ) ), this, SLOT( slotImportProject() ));

    genWizard = actionCollection()->addAction("generation_wizard");
    genWizard->setText(i18n("&Code Generation Wizard..."));
    connect(genWizard, SIGNAL( triggered( bool ) ), this, SLOT( generationWizard() ));

    genAll = actionCollection()->addAction("generate_all");
    genAll->setText(i18n("&Generate All Code"));
    connect(genAll, SIGNAL( triggered( bool ) ), this, SLOT( generateAllCode() ));

#define setProgLangAction(pl, name, action) \
        m_langAct[pl] = actionCollection()->addAction(action);          \
        m_langAct[pl]->setCheckable( true );                            \
        m_langAct[pl]->setText(name);                                   \
        connect(m_langAct[pl], SIGNAL(triggered()), this, "1"action"()")
    setProgLangAction(Uml::pl_ActionScript, "ActionScript", "set_lang_actionscript");
    setProgLangAction(Uml::pl_Ada,          "Ada",          "set_lang_ada");
    setProgLangAction(Uml::pl_Cpp,          "C++",          "set_lang_cpp");
    setProgLangAction(Uml::pl_CSharp,       "C#",           "set_lang_csharp");
    setProgLangAction(Uml::pl_D,            "D",            "set_lang_d");
    setProgLangAction(Uml::pl_IDL,          "IDL",          "set_lang_idl");
    setProgLangAction(Uml::pl_Java,         "Java",         "set_lang_java");
    setProgLangAction(Uml::pl_JavaScript,   "JavaScript",   "set_lang_javascript");
    setProgLangAction(Uml::pl_MySQL,        "MySQL (SQL)",  "set_lang_mysql");
    setProgLangAction(Uml::pl_Pascal,       "Pascal",       "set_lang_pascal");
    setProgLangAction(Uml::pl_Perl,         "Perl",         "set_lang_perl");
    setProgLangAction(Uml::pl_PHP,          "PHP",          "set_lang_php");
    setProgLangAction(Uml::pl_PHP5,         "PHP5",         "set_lang_php5");
    setProgLangAction(Uml::pl_PostgreSQL,   "PostgreSQL(SQL)","set_lang_postgresql");
    setProgLangAction(Uml::pl_Python,       "Python",       "set_lang_python");
    setProgLangAction(Uml::pl_Ruby,         "Ruby",         "set_lang_ruby");
    setProgLangAction(Uml::pl_SQL,          "SQL",          "set_lang_sql");
    setProgLangAction(Uml::pl_Tcl,          "Tcl",          "set_lang_tcl");
    setProgLangAction(Uml::pl_XMLSchema,    "XMLSchema",    "set_lang_xmlschema");
    setProgLangAction(Uml::pl_Ocl,          "Ocl",          "set_lang_ocl");
#undef setProgLangAction

    fileNew->setToolTip(i18n("Creates a new document"));
    fileOpen->setToolTip(i18n("Opens an existing document"));
    fileOpenRecent->setToolTip(i18n("Opens a recently used file"));
    fileSave->setToolTip(i18n("Saves the document"));
    fileSaveAs->setToolTip(i18n("Saves the document as..."));
    fileClose->setToolTip(i18n("Closes the document"));
    filePrint ->setToolTip(i18n("Prints out the document"));
    fileQuit->setToolTip(i18n("Quits the application"));
    fileExportDocbook->setToolTip(i18n("Exports the model to the docbook format"));
    fileExportXhtml->setToolTip(i18n("Exports the model to the XHTML format"));
    editCut->setToolTip(i18n("Cuts the selected section and puts it to the clipboard"));
    editCopy->setToolTip(i18n("Copies the selected section to the clipboard"));
    editPaste->setToolTip(i18n("Pastes the contents of the clipboard"));
    preferences->setToolTip( i18n( "Set the default program preferences") );

    deleteSelectedWidget = actionCollection()->addAction("delete_selected");
    deleteSelectedWidget->setIcon(KIcon("edit-delete"));
    deleteSelectedWidget->setText(i18n("Delete &Selected"));
    deleteSelectedWidget->setShortcut(QKeySequence(Qt::Key_Delete));
    connect(deleteSelectedWidget, SIGNAL( triggered( bool ) ), this, SLOT( slotDeleteSelectedWidget() ));

    // The different views
    newDiagram = actionCollection()->add<KActionMenu>( "new_view" );
    newDiagram->setIcon( KIcon("document-new") );
    newDiagram->setText( "new_view" );

    classDiagram = actionCollection()->addAction( "new_class_diagram" );
    classDiagram->setIcon( KIcon("umbrello_diagram_class") );
    classDiagram->setText( i18n( "&Class Diagram..." ) );
    connect(classDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotClassDiagram() ));
    newDiagram->addAction(classDiagram);

    sequenceDiagram= actionCollection()->addAction( "new_sequence_diagram" );
    sequenceDiagram->setIcon( KIcon("umbrello_diagram_sequence") );
    sequenceDiagram->setText( i18n( "&Sequence Diagram..." ) );
    connect(sequenceDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotSequenceDiagram() ));
    newDiagram->addAction(sequenceDiagram);

    collaborationDiagram = actionCollection()->addAction( "new_collaboration_diagram" );
    collaborationDiagram->setIcon( KIcon("umbrello_diagram_collaboration") );
    collaborationDiagram->setText( i18n( "C&ollaboration Diagram..." ) );
    connect(collaborationDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotCollaborationDiagram() ));
    newDiagram->addAction(collaborationDiagram);

    useCaseDiagram= actionCollection()->addAction( "new_use_case_diagram" );
    useCaseDiagram->setIcon( KIcon("umbrello_diagram_usecase") );
    useCaseDiagram->setText( i18n( "&Use Case Diagram..." ) );
    connect(useCaseDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotUseCaseDiagram() ));
    newDiagram->addAction(useCaseDiagram);

    stateDiagram= actionCollection()->addAction( "new_state_diagram" );
    stateDiagram->setIcon( KIcon("umbrello_diagram_state") );
    stateDiagram->setText( i18n( "S&tate Diagram..." ) );
    connect(stateDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotStateDiagram() ));
    newDiagram->addAction(stateDiagram);

    activityDiagram= actionCollection()->addAction( "new_activity_diagram" );
    activityDiagram->setIcon( KIcon("umbrello_diagram_activity") );
    activityDiagram->setText( i18n( "&Activity Diagram..." ) );
    connect(activityDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotActivityDiagram() ));
    newDiagram->addAction(activityDiagram);

    componentDiagram = actionCollection()->addAction( "new_component_diagram" );
    componentDiagram->setIcon( KIcon("umbrello_diagram_component") );
    componentDiagram->setText( i18n("Co&mponent Diagram...") );
    connect(componentDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotComponentDiagram() ));
    newDiagram->addAction(componentDiagram);

    deploymentDiagram = actionCollection()->addAction( "new_deployment_diagram" );
    deploymentDiagram->setIcon( KIcon("umbrello_diagram_deployment") );
    deploymentDiagram->setText( i18n("&Deployment Diagram...") );
    connect(deploymentDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotDeploymentDiagram() ));
    newDiagram->addAction(deploymentDiagram);

    entityRelationshipDiagram = actionCollection()->addAction( "new_entityrelationship_diagram" );
    entityRelationshipDiagram->setIcon( KIcon("umbrello_diagram_entityrelationship") );
    entityRelationshipDiagram->setText( i18n("&Entity Relationship Diagram...") );
    connect(entityRelationshipDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotEntityRelationshipDiagram() ));
    newDiagram->addAction(entityRelationshipDiagram);

    viewClearDiagram = actionCollection()->addAction( "view_clear_diagram" );
    viewClearDiagram->setIcon( KIcon("edit-clear") );
    viewClearDiagram->setText( i18n("&Clear Diagram") );
    connect(viewClearDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotCurrentViewClearDiagram() ));

    viewSnapToGrid = actionCollection()->add<KToggleAction>("view_snap_to_grid");
    viewSnapToGrid->setText(i18n("&Snap to Grid"));
    connect(viewSnapToGrid, SIGNAL( triggered( bool ) ), this, SLOT( slotCurrentViewToggleSnapToGrid() ));

    viewShowGrid = actionCollection()->add<KToggleAction>("view_show_grid");
    viewShowGrid->setText(i18n("S&how Grid"));
    connect(viewShowGrid, SIGNAL( triggered( bool ) ), this, SLOT( slotCurrentViewToggleShowGrid() ));
    viewShowGrid->setCheckedState(KGuiItem(i18n("&Hide Grid")));
    deleteDiagram = actionCollection()->addAction( "view_delete" );
    deleteDiagram->setIcon( KIcon("edit-delete") );
    deleteDiagram->setText( i18n("&Delete") );
    connect(deleteDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotDeleteDiagram() ));

    viewExportImage = actionCollection()->addAction( "view_export_image" );
    viewExportImage->setIcon( KIcon("image") );
    viewExportImage->setText( i18n("&Export as Picture...") );
    connect(viewExportImage, SIGNAL( triggered( bool ) ), this, SLOT( slotCurrentViewExportImage() ));

    viewExportImageAll = actionCollection()->addAction( "view_export_image_all" );
    viewExportImageAll->setIcon( KIcon("image") );
    viewExportImageAll->setText( i18n("Export &All Diagrams as Pictures...") );
    connect(viewExportImageAll, SIGNAL( triggered( bool ) ), this, SLOT( slotAllViewsExportImage() ));

    viewProperties = actionCollection()->addAction( "view_properties" );
    viewProperties->setIcon( KIcon("document-properties") );
    viewProperties->setText( i18n("&Properties") );
    connect(viewProperties, SIGNAL( triggered( bool ) ), this, SLOT( slotCurrentViewProperties() ));

    viewSnapToGrid->setChecked(false);
    viewShowGrid->setChecked(false);

    viewClearDiagram->setEnabled(false);
    viewSnapToGrid->setEnabled(false);
    viewShowGrid->setEnabled(false);
    deleteDiagram->setEnabled(false);
    viewExportImage->setEnabled(false);
    viewProperties->setEnabled(false);

    zoomAction = new KPlayerPopupSliderAction(this, SLOT(slotZoomSliderMoved(int)), this);
    zoomAction->setText(i18n("&Zoom Slider"));
    zoomAction->setIcon(KIcon("zoom-original"));
    zoomAction->setShortcuts(KShortcut(Qt::Key_F9));
    actionCollection()->addAction("popup_zoom", zoomAction);
    zoom100Action = actionCollection()->addAction("zoom100");
    zoom100Action->setIcon(KIcon("viewmag1"));
    zoom100Action->setText(i18n("Z&oom to 100%"));
    connect(zoom100Action, SIGNAL( triggered( bool ) ), this, SLOT( slotZoom100() ));

    alignRight = actionCollection()->addAction( "align_right" );
    alignRight->setText(i18n("Align Right" ));
    alignRight->setIcon(KIcon("align_right" ) );
    connect(alignRight, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignRight() ));

    alignLeft = actionCollection()->addAction( "align_left" );
    alignLeft->setText(i18n("Align Left" ));
    alignLeft->setIcon(KIcon("align_left" ) );
    connect(alignLeft, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignLeft() ));

    alignTop = actionCollection()->addAction( "align_top" );
    alignTop->setText(i18n("Align Top" ));
    alignTop->setIcon(KIcon("align_top" ) );
    connect(alignTop, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignTop() ));

    alignBottom = actionCollection()->addAction( "align_bottom" );
    alignBottom->setText(i18n("Align Bottom" ));
    alignBottom->setIcon(KIcon("align_bottom" ) );
    connect(alignBottom, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignBottom() ));

    alignVerticalMiddle = actionCollection()->addAction( "align_vertical_middle" );
    alignVerticalMiddle->setText(i18n("Align Vertical Middle" ));
    alignVerticalMiddle->setIcon(KIcon("align_vert_middle" ) );
    connect(alignVerticalMiddle, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignVerticalMiddle() ));

    alignHorizontalMiddle = actionCollection()->addAction( "align_horizontal_middle" );
    alignHorizontalMiddle->setText(i18n("Align Horizontal Middle" ));
    alignHorizontalMiddle->setIcon(KIcon("align_hori_middle" ) );
    connect(alignHorizontalMiddle, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignHorizontalMiddle() ));

    alignVerticalDistribute = actionCollection()->addAction( "align_vertical_distribute" );
    alignVerticalDistribute->setText(i18n("Align Vertical Distribute" ));
    alignVerticalDistribute->setIcon(KIcon("align_vert_distribute" ) );
    connect(alignVerticalDistribute, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignVerticalDistribute() ));

    alignHorizontalDistribute = actionCollection()->addAction( "align_horizontal_distribute" );
    alignHorizontalDistribute->setText(i18n("Align Horizontal Distribute" ));
    alignHorizontalDistribute->setIcon(KIcon("align_hori_distribute" ) );
    connect(alignHorizontalDistribute, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignHorizontalDistribute() ));


    QString moveTabLeftString = i18n("&Move Tab Left");
    QString moveTabRightString = i18n("&Move Tab Right");
    moveTabLeft = actionCollection()->addAction("move_tab_left");
    moveTabLeft->setIcon(KIcon(QApplication::layoutDirection() ? "go-next" : "go-previous"));
    moveTabLeft->setText(QApplication::layoutDirection() ? moveTabRightString : moveTabLeftString);
    moveTabLeft->setShortcut(QApplication::layoutDirection() ?
                 QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Right) : QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Left));
    connect(moveTabLeft, SIGNAL( triggered( bool ) ), this, SLOT( slotMoveTabLeft() ));

    moveTabRight = actionCollection()->addAction("move_tab_right");
    moveTabRight->setIcon(KIcon(QApplication::layoutDirection() ? "go-previous" : "go-next"));
    moveTabRight->setText(QApplication::layoutDirection() ? moveTabLeftString : moveTabRightString);
    moveTabRight->setShortcut(QApplication::layoutDirection() ?
                  QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Left) : QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Right));
    connect(moveTabRight, SIGNAL( triggered( bool ) ), this, SLOT( slotMoveTabRight() ));

    QString selectTabLeftString = i18n("Select Diagram on Left");
    QString selectTabRightString = i18n("Select Diagram on Right");
    changeTabLeft = actionCollection()->addAction("previous_tab");
    changeTabLeft->setText(QApplication::layoutDirection() ? selectTabRightString : selectTabLeftString);

    changeTabLeft->setShortcut(QApplication::layoutDirection() ?
                   QKeySequence(Qt::SHIFT+Qt::Key_Right) : QKeySequence(Qt::SHIFT+Qt::Key_Left));
    connect(changeTabLeft, SIGNAL( triggered( bool ) ), this, SLOT( slotChangeTabLeft() ));

    changeTabRight = actionCollection()->addAction("next_tab");
    changeTabRight->setText(QApplication::layoutDirection() ? selectTabLeftString : selectTabRightString);

    changeTabRight->setShortcut(QApplication::layoutDirection() ?
                    QKeySequence(Qt::SHIFT+Qt::Key_Left) : QKeySequence(Qt::SHIFT+Qt::Key_Right));
    connect(changeTabRight, SIGNAL( triggered( bool ) ), this, SLOT( slotChangeTabRight() ));


    initStatusBar(); //call this here because the statusBar is shown/hidden by setupGUI()

    // use the absolute path to your umbrelloui.rc file for testing purpose in setupGUI();
    setupGUI();

// @todo Check if this should be ported
//     QMenu* menu = findMenu( menuBar(), QString("settings") );
//     menu->insertItem(i18n("&Windows"), dockHideShowMenu(), -1, 0);
}

void UMLApp::slotZoomSliderMoved(int value) {
    int zoom = (int)(value*0.01);
    getCurrentView()->setZoom(zoom*zoom);
}

void UMLApp::slotZoom100()  {
    setZoom(100);
}

void UMLApp::setZoom(int zoom) {
    getCurrentView()->setZoom(zoom);
}

void UMLApp::setupZoomMenu() {
    m_zoomSelect->clear();

    //IMPORTANT: The ID's must match the zoom value (text)
    m_zoomSelect->insertItem(i18n(" &33%"),33);
    m_zoomSelect->insertItem(i18n(" &50%"),50);
    m_zoomSelect->insertItem(i18n(" &75%"),75);
    m_zoomSelect->insertItem(i18n("&100%"),100);
    m_zoomSelect->insertItem(i18n("1&50%"),150);
    m_zoomSelect->insertItem(i18n("&200%"),200);
    m_zoomSelect->insertItem(i18n("3&00%"),300);


    int zoom = getCurrentView()->currentZoom();
    //if current zoom is not a "standard zoom" (because of zoom in / zoom out step
    //we add it for information
    switch(zoom){
    case 33:
    case 50:
    case 75:
    case 100:
    case 150:
    case 200:
    case 300:
        break;
    default:
        m_zoomSelect->insertSeparator();
        m_zoomSelect->insertItem(QString::number(zoom)+" %",zoom);
    }
    m_zoomSelect->setItemChecked(zoom, true);
}

void UMLApp::initStatusBar() {
    statusBar()->insertPermanentItem( i18n( "Ready" ), 1 );
    connect(m_doc, SIGNAL( sigWriteToStatusBar(const QString &) ), this, SLOT( slotStatusMsg(const QString &) ));
}

void UMLApp::initView() {
    setCaption(m_doc->url().fileName(),false);
    m_view = NULL;
    toolsbar = new WorkToolBar(this);
    toolsbar->setLabel(i18n("Diagram Toolbar"));
    addToolBar(Qt::TopToolBarArea, toolsbar);

    //setupGUI();
//     m_mainDock = new QDockWidget( this );
//     addDockWidget ( Qt::RightDockWidgetArea, m_mainDock );
    m_newSessionButton = NULL;
    m_diagramMenu = NULL;
    m_closeDiagramButton = NULL;

    // Prepare Stacked Diagram Representation
    m_viewStack = new QStackedWidget(this);

    // Prepare Tabbed Diagram Representation
    m_tabWidget = new KTabWidget(this);
    m_tabWidget->setAutomaticResizeTabs( true );

    m_newSessionButton = new QToolButton(m_tabWidget);
    m_newSessionButton->setIconSet(SmallIcon("tab-new"));
    m_newSessionButton->adjustSize();
    m_newSessionButton->setAutoRaise(true);
    m_newSessionButton->setPopupMode(QToolButton::InstantPopup);
    m_newSessionButton->setMenu(newDiagram->menu());

    m_closeDiagramButton = new QToolButton(m_tabWidget);
    m_closeDiagramButton->setIconSet( SmallIcon("tab-remove"));
    m_closeDiagramButton->adjustSize();

    connect(m_closeDiagramButton, SIGNAL(clicked()), SLOT(slotDeleteDiagram()));
    connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)), SLOT(slotTabChanged(QWidget*)));
    connect(m_tabWidget, SIGNAL(contextMenu(QWidget*,const QPoint&)), m_doc, SLOT(slotDiagramPopupMenu(QWidget*,const QPoint&)));
    m_tabWidget->setCornerWidget( m_newSessionButton, Qt::TopLeftCorner );
    m_tabWidget->setCornerWidget( m_closeDiagramButton, Qt::TopRightCorner );
    m_newSessionButton->installEventFilter(this);

    // Prepare Stacked Diagram Representation
    m_viewStack = new QStackedWidget(this);

    m_layout = new QVBoxLayout;
    m_layout->setMargin(0);
    if (Settings::getOptionState().generalState.tabdiagrams) {
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

    m_listDock = new QDockWidget( i18n("&Tree View"), this );
    m_listDock->setObjectName("TreeViewDock");
    addDockWidget(Qt::LeftDockWidgetArea, m_listDock);
    m_listView = new UMLListView(m_listDock ,"LISTVIEW");
    //m_listView->setSorting(-1);
    m_listView->setDocument(m_doc);
    m_listView->init();
    m_listDock->setWidget(m_listView);

    m_documentationDock = new QDockWidget( i18n("Doc&umentation"), this );
    m_documentationDock->setObjectName("DocumentationDock");

    addDockWidget(Qt::LeftDockWidgetArea, m_documentationDock);
    m_pDocWindow = new DocWindow(m_doc, m_documentationDock, "DOCWINDOW");
    m_documentationDock->setWidget(m_pDocWindow);

    m_doc->setupSignals();//make sure gets signal from list view

    m_cmdHistoryDock = new QDockWidget(i18n("Co&mmand history"), this);
    m_cmdHistoryDock->setObjectName("CmdHistoryDock");
    addDockWidget(Qt::LeftDockWidgetArea, m_cmdHistoryDock);
    // create cmd history view
    m_pQUndoView = new QUndoView(m_cmdHistoryDock);
    m_cmdHistoryDock->setWidget(m_pQUndoView);
    m_pQUndoView->setCleanIcon(KIcon("filesave"));
    m_pQUndoView->setStack(m_pUndoStack);

    // Create the property viewer
    //m_propertyDock = new QDockWidget(i18n("&Properties"), this);
    //addDockWidget(Qt::LeftDockWidgetArea, m_propertyDock);

    tabifyDockWidget(m_documentationDock, m_cmdHistoryDock);
    //tabifyDockWidget(m_cmdHistoryDock, m_propertyDock);

    QByteArray dockConfig;
    dockConfig += UmbrelloSettings::dockConfig().toAscii();
    restoreState(dockConfig); //reposition all the DockWindows to their saved positions
}

void UMLApp::openDocumentFile(const KUrl& url) {
    slotStatusMsg(i18n("Opening file..."));

    m_doc->openDocument( url);
    fileOpenRecent->addUrl( url );
    slotStatusMsg(i18n("Ready."));
    setCaption(m_doc->url().fileName(), false);
    enablePrint(true);
}

UMLDoc *UMLApp::getDocument() const {
    return m_doc;
}

UMLListView* UMLApp::getListView() {
    return m_listView;
}


void UMLApp::saveOptions() {
    // The Toolbar settings will be handled by the respective classes ( KToolBar )
    KConfigGroup cg( m_config, "toolbar" );
    toolBar("mainToolBar")->saveSettings( cg );
    cg.changeGroup( "workbar" );
    toolsbar->saveSettings(cg );
    fileOpenRecent->saveEntries( m_config->group( "Recent Files") );

    UmbrelloSettings::setGeometry( size() );

    Settings::OptionState& optionState = Settings::getOptionState();

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
    } else {
        UmbrelloSettings::setLastFile(  m_doc -> url().prettyUrl() );
    }

    UmbrelloSettings::setImageMimeType(  getImageMimeType() );

    UmbrelloSettings::setUseFillColor(  optionState.uiState.useFillColor );
    UmbrelloSettings::setFillColor(  optionState.uiState.fillColor );
    UmbrelloSettings::setLineColor(  optionState.uiState.lineColor );
    UmbrelloSettings::setLineWidth(  optionState.uiState.lineWidth );
    UmbrelloSettings::setShowDocWindow(  m_documentationDock->isVisible() );
    UmbrelloSettings::setUiFont(  optionState.uiState.font );

    UmbrelloSettings::setShowVisibility(  optionState.classState.showVisibility );
    UmbrelloSettings::setShowAtts( optionState.classState.showAtts);
    UmbrelloSettings::setShowOps(  optionState.classState.showOps );
    UmbrelloSettings::setShowStereoType( optionState.classState.showStereoType );
    UmbrelloSettings::setShowAttSig(  optionState.classState.showAttSig );
    UmbrelloSettings::setShowOpSig( optionState.classState.showOpSig );
    UmbrelloSettings::setShowPackage(  optionState.classState.showPackage );
    UmbrelloSettings::setShowAttribAssocs(  optionState.classState.showAttribAssocs );
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

    // write config for Java code generation options
    UmbrelloSettings::setAutoGenerateAttributeAccessorsJava(optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAttributeAccessors);
    UmbrelloSettings::setAutoGenerateAssocAccessorsJava(optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAssocAccessors);

//     CodeGenerator *codegen = UMLApp::app()->getGenerator();
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

    UmbrelloSettings::self()->writeConfig();
}

void UMLApp::readOptions() {
    // bar status settings
    toolBar("mainToolBar")->applySettings(m_config->group( "toolbar") );
    // do config for work toolbar
    toolsbar->applySettings(m_config->group( "workbar") );
    fileOpenRecent->loadEntries(m_config->group( "Recent Files") );
    setImageMimeType( UmbrelloSettings::imageMimeType() );
    resize( UmbrelloSettings::geometry());
}

void UMLApp::saveProperties(KConfigGroup &_config) {
    if(m_doc->url().fileName()!=i18n("Untitled") && !m_doc->isModified()) {
        // saving to tempfile not necessary

    } else {
        KUrl url=m_doc->url();
        _config.writePathEntry("filename", url.url());
        _config.writeEntry("modified", m_doc->isModified());
        QString tempname = kapp->tempSaveName(url.url());
        QString tempurl= QUrl::toPercentEncoding(tempname);

        KUrl _url(tempurl);
        m_doc->saveDocument(_url);
    }
}

void UMLApp::readProperties(const KConfigGroup& _config) {
    QString filename = _config.readPathEntry("filename");
    KUrl url(filename);
    bool modified = _config.readEntry("modified", false);
    if(modified) {
        bool canRecover;
        QString tempname = kapp->checkRecoverFile(filename, canRecover);
        KUrl _url(tempname);


        if(canRecover) {
            m_doc->openDocument(_url);
            m_doc->setModified();
            enablePrint(true);
            setCaption(_url.fileName(),true);
            QFile::remove
                (tempname);
        } else {
            enablePrint(false);
        }
    } else {
        if(!filename.isEmpty()) {
            m_doc->openDocument(url);
            enablePrint(true);
            setCaption(url.fileName(),false);

        } else {
            enablePrint(false);
        }
    }
}

bool UMLApp::queryClose() {
    QByteArray dockConfig = saveState();
    UmbrelloSettings::setDockConfig( dockConfig );
    return m_doc->saveModified();
}

bool UMLApp::queryExit() {
    saveOptions();
    m_doc -> closeDocument();
    return true;
}

void UMLApp::slotFileNew() {
    slotStatusMsg(i18n("Creating new document..."));
    if(m_doc->saveModified()) {
        setDiagramMenuItemsState(false);
        m_doc->newDocument();
        setCaption(m_doc->url().fileName(), false);
        fileOpenRecent->setCurrentItem( -1 );
        setModified(false);
        enablePrint(false);
    }
    slotUpdateViews();
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotFileOpen() {
    slotStatusMsg(i18n("Opening file..."));
    m_loading = true;

    if(!m_doc->saveModified()) {

        // here saving wasn't successful

    } else {
        KUrl url=KFileDialog::getOpenUrl(KUrl(),
            i18n("*.xmi *.xmi.tgz *.xmi.tar.bz2 *.mdl|All Supported Files (*.xmi, *.xmi.tgz, *.xmi.tar.bz2, *.mdl)\n"
                 "*.xmi|Uncompressed XMI Files (*.xmi)\n"
                 "*.xmi.tgz|Gzip Compressed XMI Files (*.xmi.tgz)\n"
                 "*.xmi.tar.bz2|Bzip2 Compressed XMI Files (*.xmi.tar.bz2)\n"
                 "*.mdl|Rose model files"), this, i18n("Open File"));
        if(!url.isEmpty()) {
            if(m_doc->openDocument(url))
                fileOpenRecent->addUrl( url );
            enablePrint(true);
            setCaption(m_doc->url().fileName(), false);
        }

    }
    slotUpdateViews();
    m_loading = false;
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotFileOpenRecent(const KUrl& url) {
    slotStatusMsg(i18n("Opening file..."));
    m_loading = true;

    KUrl oldUrl = m_doc->url();

    if(!m_doc->saveModified()) {
        // here saving wasn't successful
    } else {
        if(!m_doc->openDocument(url)) {
            fileOpenRecent->removeUrl(url);
            fileOpenRecent->setCurrentItem( -1 );
        } else {
            fileOpenRecent->addUrl(url);
        }
        enablePrint(true);
        setCaption(m_doc->url().fileName(), false);
    }

    m_loading = false;
    slotUpdateViews();
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotFileSave() {
    slotStatusMsg(i18n("Saving file..."));
    if(m_doc->url().fileName() == i18n("Untitled"))
        slotFileSaveAs();
    else
        m_doc->saveDocument(m_doc -> url());

    m_pUndoStack->setClean();
    slotStatusMsg(i18n("Ready."));
}

bool UMLApp::slotFileSaveAs()
{
    slotStatusMsg(i18n("Saving file with a new filename..."));
    bool cont = true;
    KUrl url;
    QString ext;
    while(cont) {
        url=KFileDialog::getSaveUrl(KUrl(), i18n("*.xmi|XMI File\n*.xmi.tgz|Gzip Compressed XMI File\n*.xmi.tar.bz2|Bzip2 Compressed XMI File\n*|All Files"), this, i18n("Save As"));

        if(url.isEmpty())
            cont = false;
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
            QDir d = url.path( KUrl::RemoveTrailingSlash );

            if(QFile::exists(d.path())) {
                int want_save = KMessageBox::warningContinueCancel(this, i18n("The file %1 exists.\nDo you wish to overwrite it?", url.path()), i18n("Warning"), KGuiItem(i18n("Overwrite")));
                if(want_save == KMessageBox::Continue)
                    cont = false;
            } else
                cont = false;

        }
    }
    if(!url.isEmpty()) {
        bool b = m_doc->saveDocument(url);
        if (b) {
            fileOpenRecent->addUrl(url);
            setCaption(url.fileName(),m_doc->isModified());
            slotStatusMsg(i18n("Ready."));
        }
        return b;

    } else {
        slotStatusMsg(i18n("Ready."));
        return false;
    }
}

void UMLApp::slotFileClose() {
    slotStatusMsg(i18n("Closing file..."));

    slotFileNew();

}

void UMLApp::slotFilePrint()
{
    slotStatusMsg(i18n("Printing..."));

    KPrinter printer;
    printer.setFullPage(true);
    DiagramPrintPage * selectPage = new DiagramPrintPage(0, m_doc);
    printer.addDialogPage(selectPage);
    QString msg;
    if (printer.setup(this, i18n("Print %1", m_doc->url().prettyUrl()))) {

        m_doc -> print(&printer);
    }
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotFileQuit() {
    slotStatusMsg(i18n("Exiting..."));
    if(m_doc->saveModified()) {
        QByteArray dockConfig = saveState();
        UmbrelloSettings::setDockConfig( dockConfig );
        saveOptions();
        qApp->quit();
    }
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotFileExportDocbook()
{
  DocbookGenerator().generateDocbookForProject();
}

void UMLApp::slotFileExportXhtml()
{
  if (m_xhtmlGenerator != 0)
  {
    return;
  }
  m_xhtmlGenerator = new XhtmlGenerator();
  m_xhtmlGenerator->generateXhtmlForProject();
  connect(m_xhtmlGenerator,SIGNAL(finished()),this,SLOT(slotXhtmlDocGenerationFinished()));
}

void UMLApp::slotEditUndo() {
    undo();
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotEditRedo() {
    redo();
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotEditCut() {
    slotStatusMsg(i18n("Cutting selection..."));
    //FIXME bug 59774 this fromview isn't very reliable.
    //when cutting diagrams it is set to true even though it shouldn't be
    bool fromview = (getCurrentView() && getCurrentView()->getSelectCount());
    if ( editCutCopy(fromview) ) {
        emit sigCutSuccessful();
        slotDeleteSelectedWidget();
        m_doc->setModified(true);
    }
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotEditCopy() {
    slotStatusMsg(i18n("Copying selection to clipboard..."));
    bool  fromview = (getCurrentView() && getCurrentView()->getSelectCount());
    editCutCopy( fromview );
    slotStatusMsg(i18n("Ready."));
    m_doc -> setModified( true );
}

void UMLApp::slotEditPaste() {
    slotStatusMsg(i18n("Inserting clipboard contents..."));
    QMimeSource* data = QApplication::clipboard()->data();
    UMLClipboard clipboard;
    setCursor(Qt::WaitCursor);
    if(!clipboard.paste(data)) {
        KMessageBox::sorry( this, i18n("Umbrello could not paste the clipboard contents.  "
                                       "The objects in the clipboard may be of the wrong "
                                       "type to be pasted here."), i18n("Paste Error") );
    }
    slotStatusMsg(i18n("Ready."));
    setCursor(Qt::ArrowCursor);
    editPaste->setEnabled(false);
    m_doc -> setModified( true );
}


void UMLApp::slotStatusMsg(const QString &text) {
    ///////////////////////////////////////////////////////////////////
    // change status message permanently
    statusBar()->changeItem( text, 1 );
}

void UMLApp::slotClassDiagram() {
    executeCommand(new Uml::cmdCreateClassDiag(m_doc));
}


void UMLApp::slotSequenceDiagram() {
    executeCommand(new Uml::cmdCreateSeqDiag(m_doc));
}

void UMLApp::slotCollaborationDiagram() {
    executeCommand(new Uml::cmdCreateCollaborationDiag(m_doc));
}

void UMLApp::slotUseCaseDiagram() {
    executeCommand(new Uml::cmdCreateUseCaseDiag(m_doc));
}

void UMLApp::slotStateDiagram() {
    executeCommand(new Uml::cmdCreateStateDiag(m_doc));
}

void UMLApp::slotActivityDiagram() {
    executeCommand(new Uml::cmdCreateActivityDiag(m_doc));
}

void UMLApp::slotComponentDiagram() {
    executeCommand(new Uml::cmdCreateComponentDiag(m_doc));
}

void UMLApp::slotDeploymentDiagram() {
    executeCommand(new Uml::cmdCreateDeployDiag(m_doc));
}

void UMLApp::slotEntityRelationshipDiagram() {
    executeCommand(new Uml::cmdCreateEntityRelationDiag(m_doc));
}


void UMLApp::slotAlignLeft() {
    getCurrentView()->alignLeft();
}

void UMLApp::slotAlignRight() {
    getCurrentView()->alignLeft();
}

void UMLApp::slotAlignTop() {
    getCurrentView()->alignTop();
}

void UMLApp::slotAlignBottom() {
    getCurrentView()->alignBottom();
}

void UMLApp::slotAlignVerticalMiddle() {
    getCurrentView()->alignVerticalMiddle();
}

void UMLApp::slotAlignHorizontalMiddle() {
    getCurrentView()->alignHorizontalMiddle();
}

void UMLApp::slotAlignVerticalDistribute() {
    getCurrentView()->alignVerticalDistribute();
}

void UMLApp::slotAlignHorizontalDistribute() {
    getCurrentView()->alignHorizontalDistribute();
}

WorkToolBar* UMLApp::getWorkToolBar() {
    return toolsbar;
}

void UMLApp::setModified(bool modified) {
    //fileSave -> setEnabled(modified);

    //if anything else needs to be done on a mofication, put it here

    // printing should be possible whenever there is something to print
    if ( m_loading == false && modified == true && getCurrentView() )  {
        enablePrint(true);
    }

    if (m_loading == false)  {
        setCaption(m_doc->url().fileName(), modified); //add disk icon to taskbar if modified
    }
}

void UMLApp::enablePrint(bool enable) {
    filePrint->setEnabled(enable);
}

void UMLApp::enableUndo(bool enable) {
    editUndo->setEnabled(enable);
}

void UMLApp::enableRedo(bool enable) {
    editRedo->setEnabled(enable);
}

/** initialize the QT's global clipboard support for the application */
void UMLApp::initClip() {
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

    m_copyTimer = new QTimer(this, "copytimer");
    m_copyTimer->start(500, false);
    connect(m_copyTimer, SIGNAL(timeout()), this, SLOT(slotCopyChanged()));
}

bool UMLApp::canDecode(const QMimeSource* mimeSource) {
    const char* f;
    for (int i=0; (f=mimeSource->format(i)); i++) {
        if ( !qstrnicmp(f,"application/x-uml-clip", 22) ) {
            //FIXME need to test for clip1, clip2, clip3, clip4 or clip5
            //(the only valid clip types)
            return true;
        }
    }
    return false;
}

void UMLApp::slotClipDataChanged() {
    QMimeSource * data = QApplication::clipboard()->data();

    //Pass the MimeSource to the Doc
    editPaste->setEnabled( data && canDecode(data) );
}

void UMLApp::slotCopyChanged() {
    if(m_listView->getSelectedCount() || (getCurrentView() && getCurrentView()->getSelectCount())) {
        editCopy->setEnabled(true);
        editCut->setEnabled(true);
    } else {
        editCopy->setEnabled(false);
        editCut->setEnabled(false);
    }
}

void UMLApp::slotPrefs() {

       Settings::OptionState& optionState = Settings::getOptionState();

       m_dlg = new SettingsDlg(this, &optionState);
       connect(m_dlg, SIGNAL( applyClicked() ), this, SLOT( slotApplyPrefs() ) );


       if ( m_dlg->exec() == QDialog::Accepted && m_dlg->getChangesApplied() ) {
           slotApplyPrefs();
       }

       delete m_dlg;
       m_dlg = NULL;

}

void UMLApp::slotApplyPrefs() {
    if (m_dlg) {
        /* we need this to sync both values */

        Settings::OptionState& optionState = Settings::getOptionState();
        bool currentTabbedBrowsing = (m_layout->findWidget(m_tabWidget) != -1);
        bool newTabbedBrowsing = optionState.generalState.tabdiagrams;

        if (currentTabbedBrowsing != newTabbedBrowsing) {
            // Diagram Representation Modified
            UMLView* currentView;
            UMLViewList views = m_doc->getViewIterator();

            if (newTabbedBrowsing) {
                currentView=static_cast<UMLView*>(m_viewStack->currentWidget());
                m_layout->removeWidget(m_viewStack);
                m_viewStack->hide();

                UMLViewList views = m_doc->getViewIterator();
                for(UMLView *view = views.first(); view; view = views.next()) {
                    m_viewStack->removeWidget(view);
                    m_tabWidget->addTab(view, view->getName());
                    m_tabWidget->setTabIconSet(view, Widget_Utils::iconSet(view->getType()));
                }
                m_layout->addWidget(m_tabWidget);
                m_tabWidget->show();
            }
            else {
                currentView=static_cast<UMLView*>(m_tabWidget->currentPage());
                m_layout->removeWidget (m_tabWidget);
                m_tabWidget->hide();

                for(UMLView *view = views.first(); view; view = views.next()) {
                    m_tabWidget->removePage(view);
                    m_viewStack->addWidget(view);
                }

                m_layout->addWidget(m_viewStack);
                m_viewStack->show();
            }

            setCurrentView(currentView);
        }

        m_doc -> settingsChanged( optionState );
        const QString plStr = m_dlg->getCodeGenerationLanguage();
        Uml::Programming_Language pl = Model_Utils::stringToProgLang(plStr);
        setGenerator(pl);
    }
}

bool UMLApp::getUndoEnabled() {
    return editUndo->isEnabled();
}

bool UMLApp::getRedoEnabled() {
    return editRedo->isEnabled();
}

bool UMLApp::getPasteState() {
    return editPaste -> isEnabled();
}

bool UMLApp::getCutCopyState() {
    return editCopy -> isEnabled();
}

bool UMLApp::editCutCopy( bool bFromView ) {
    UMLClipboard clipboard;
    QMimeSource * clipdata = 0;

    if ((clipdata = clipboard.copy(bFromView)) != 0) {
        QClipboard* clip = QApplication::clipboard();
        clip->setData(clipdata);//the global clipboard takes ownership of the clipdata memory
        connect(clip, SIGNAL(dataChanged()), this, SLOT(slotClipDataChanged()));
        return true;
    }
    return false;
}

void UMLApp::readOptionState() {
    Settings::OptionState& optionState = Settings::getOptionState();

    UmbrelloSettings::self()->readConfig();
    optionState.generalState.undo = UmbrelloSettings::undo();
    optionState.generalState.tabdiagrams = UmbrelloSettings::tabdiagrams();
//#if defined (WORK_ON_BUG_126262)
    optionState.generalState.newcodegen = UmbrelloSettings::newcodegen();
//#else
//    optionState.generalState.newcodegen = false;
//#endif
    optionState.generalState.angularlines = UmbrelloSettings::angularlines();
    optionState.generalState.footerPrinting =  UmbrelloSettings::footerPrinting();
    optionState.generalState.autosave =  UmbrelloSettings::autosave();
    optionState.generalState.time =  UmbrelloSettings::time(); //old autosavetime value kept for compatibility
    optionState.generalState.autosavetime =  UmbrelloSettings::autosavetime();
    //if we don't have a "new" autosavetime value, convert the old one
    if (optionState.generalState.autosavetime == 0) {
        switch (optionState.generalState.time) {
        case 0: optionState.generalState.autosavetime = 5; break;
        case 1: optionState.generalState.autosavetime = 10; break;
        case 2: optionState.generalState.autosavetime = 15; break;
        case 3: optionState.generalState.autosavetime = 20; break;
        case 4: optionState.generalState.autosavetime = 25; break;
        default: optionState.generalState.autosavetime = 5; break;
        }
    }
    // 2004-05-17 Achim Spangler: read new config entry for autosave sufix
    optionState.generalState.autosavesuffix =  UmbrelloSettings::autosavesuffix();

    optionState.generalState.loadlast =  UmbrelloSettings::loadlast();

    optionState.generalState.diagram  = UmbrelloSettings::diagram();

    optionState.generalState.defaultLanguage =  UmbrelloSettings::defaultLanguage();

    optionState.uiState.useFillColor =  UmbrelloSettings::useFillColor();

    optionState.uiState.fillColor =  UmbrelloSettings::fillColor();
    optionState.uiState.lineColor =  UmbrelloSettings::lineColor();
    optionState.uiState.lineWidth =  UmbrelloSettings::lineWidth();

    optionState.uiState.font =  UmbrelloSettings::uiFont();

    optionState.classState.showVisibility =  UmbrelloSettings::showVisibility();
    optionState.classState.showAtts =  UmbrelloSettings::showAtts();
    optionState.classState.showOps =  UmbrelloSettings::showOps();
    optionState.classState.showStereoType =  UmbrelloSettings::showStereoType();
    optionState.classState.showAttSig =  UmbrelloSettings::showAttSig();
    optionState.classState.showOpSig =  UmbrelloSettings::showOpSig();
    optionState.classState.showPackage =  UmbrelloSettings::showPackage();
    optionState.classState.showAttribAssocs =  UmbrelloSettings::showAttribAssocs();
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

    // Java code generation options
    optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAttributeAccessors = UmbrelloSettings::autoGenerateAttributeAccessorsJava();
    optionState.codeGenerationState.javaCodeGenerationState.autoGenerateAssocAccessors = UmbrelloSettings::autoGenerateAssocAccessorsJava();

    // D code generation options
    optionState.codeGenerationState.dCodeGenerationState.autoGenerateAttributeAccessors = UmbrelloSettings::autoGenerateAttributeAccessorsD();
    optionState.codeGenerationState.dCodeGenerationState.autoGenerateAssocAccessors = UmbrelloSettings::autoGenerateAssocAccessorsD();

    // Ruby code generation options
    optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAttributeAccessors = UmbrelloSettings::autoGenerateAttributeAccessorsRuby();
    optionState.codeGenerationState.rubyCodeGenerationState.autoGenerateAssocAccessors = UmbrelloSettings::autoGenerateAssocAccessorsRuby();

    // general config options will be read when created
}


/** Call the code viewing assistant on a code document */
void UMLApp::viewCodeDocument(UMLClassifier* classifier) {

    CodeGenerator * currentGen = getGenerator();
    if(currentGen && classifier) {
        if(!dynamic_cast<SimpleCodeGenerator*>(currentGen))
        {
            CodeDocument *cdoc = currentGen->findCodeDocumentByClassifier(classifier);

            if (cdoc) {
                Settings::OptionState& optionState = Settings::getOptionState();
                CodeViewerDialog * dialog = currentGen->getCodeViewerDialog(this,cdoc,optionState.codeViewerState);
                dialog->exec();
                optionState.codeViewerState = dialog->getState();
                delete dialog;
                dialog = NULL;
            } else {
                // shouldn't happen..
                KMessageBox::sorry(0, i18n("Cannot view code until you generate some first."),i18n("Cannot View Code"));
            }
        } else {
            KMessageBox::sorry(0, i18n("Cannot view code from simple code writer."),i18n("Cannot View Code"));
        }
    }

}

void UMLApp::refactor(UMLClassifier* classifier) {
    if (!m_refactoringAssist) {
        m_refactoringAssist = new RefactoringAssistant( m_doc, 0, 0, "refactoring_assistant" );
    }
    m_refactoringAssist->refactor(classifier);
    m_refactoringAssist->show();
}

CodeGenerationPolicy *UMLApp::getCommonPolicy() {
    return m_commoncodegenpolicy;
}

void UMLApp::setPolicyExt(CodeGenPolicyExt *policy) {
    m_policyext = policy;
}

CodeGenPolicyExt *UMLApp::getPolicyExt() {
    return m_policyext;
}

CodeGenerator *UMLApp::setGenerator(Uml::Programming_Language pl) {
    if (pl == Uml::pl_Reserved) {
        if (m_codegen) {
            delete m_codegen;
            m_codegen = NULL;
        }
        return NULL;
    }
    // does the code generator already exist?
    // then simply return that
    if (m_codegen) {
        if (m_codegen->getLanguage() == pl)
            return m_codegen;
        delete m_codegen;  // ATTENTION! remove all refs to it or its policy first
        m_codegen = NULL;
    }
    m_activeLanguage = pl;
    m_codegen = CodeGenFactory::createObject(pl);
    updateLangSelectMenu(pl);

    slotAddDefaultDatatypes();
    m_codegen->createDefaultStereotypes();

    if (m_policyext)
        m_policyext->setDefaults(false); // picks up language specific stuff
    return m_codegen;
}

CodeGenerator* UMLApp::getGenerator() {
    return m_codegen;
}

void UMLApp::generateAllCode() {
    if (m_codegen) {
        m_codegen->writeCodeToFile();
    }
}

void UMLApp::generationWizard() {
    CodeGenerationWizard wizard(0 /*classList*/);
    wizard.exec();
}

void UMLApp::set_lang_actionscript() {
    setActiveLanguage(Uml::pl_ActionScript);
}

void UMLApp::set_lang_ada() {
    setActiveLanguage(Uml::pl_Ada);
}

void UMLApp::set_lang_cpp() {
    setActiveLanguage(Uml::pl_Cpp);
}

void UMLApp::set_lang_csharp() {
    setActiveLanguage(Uml::pl_CSharp);
}

void UMLApp::set_lang_d() {
    setActiveLanguage(Uml::pl_D);
}

void UMLApp::set_lang_idl() {
    setActiveLanguage(Uml::pl_IDL);
}

void UMLApp::set_lang_java() {
    setActiveLanguage(Uml::pl_Java);
}

void UMLApp::set_lang_javascript() {
    setActiveLanguage(Uml::pl_JavaScript);
}

void UMLApp::set_lang_mysql() {
    setActiveLanguage(Uml::pl_MySQL);
}

void UMLApp::set_lang_pascal() {
    setActiveLanguage(Uml::pl_Pascal);
}

void UMLApp::set_lang_perl() {
    setActiveLanguage(Uml::pl_Perl);
}

void UMLApp::set_lang_php() {
    setActiveLanguage(Uml::pl_PHP);
}

void UMLApp::set_lang_php5() {
    setActiveLanguage(Uml::pl_PHP5);
}

void UMLApp::set_lang_postgresql() {
    setActiveLanguage(Uml::pl_PostgreSQL);
}

void UMLApp::set_lang_python() {
    setActiveLanguage(Uml::pl_Python);
}

void UMLApp::set_lang_ruby() {
    setActiveLanguage(Uml::pl_Ruby);
}

void UMLApp::set_lang_sql() {
    setActiveLanguage(Uml::pl_SQL);
}

void UMLApp::set_lang_tcl() {
    setActiveLanguage(Uml::pl_Tcl);
}

void UMLApp::set_lang_xmlschema() {
    setActiveLanguage(Uml::pl_XMLSchema);
}

void UMLApp::set_lang_ocl() {
    setActiveLanguage(Uml::pl_Ocl);
}

void UMLApp::setActiveLanguage(Uml::Programming_Language pl) {
    updateLangSelectMenu(pl);
    setGenerator(pl);
}

Uml::Programming_Language UMLApp::getActiveLanguage() {
    return m_activeLanguage;
}

bool UMLApp::activeLanguageIsCaseSensitive() {
    return (m_activeLanguage != Uml::pl_Pascal &&
            m_activeLanguage != Uml::pl_Ada &&
            m_activeLanguage != Uml::pl_SQL &&
            m_activeLanguage != Uml::pl_MySQL &&
            m_activeLanguage != Uml::pl_PostgreSQL);
}

QString UMLApp::activeLanguageScopeSeparator() {
    Uml::Programming_Language pl = getActiveLanguage();
    if (pl == Uml::pl_Ada ||
        pl == Uml::pl_CSharp ||
        pl == Uml::pl_Pascal ||
        pl == Uml::pl_Java ||
        pl == Uml::pl_JavaScript ||
        pl == Uml::pl_Python)  // CHECK: more?
        return ".";
    return "::";
}

void UMLApp::slotCurrentViewClearDiagram() {
    getCurrentView()->clearDiagram();
}

void UMLApp::slotCurrentViewToggleSnapToGrid() {
    getCurrentView()->toggleSnapToGrid();
    viewSnapToGrid->setChecked( getCurrentView()->getSnapToGrid() );
}

void UMLApp::slotCurrentViewToggleShowGrid() {
    getCurrentView()->toggleShowGrid();
    viewShowGrid->setChecked( getCurrentView()->getShowSnapGrid() );
}

void UMLApp::slotCurrentViewExportImage() {
    getCurrentView()->getImageExporter()->exportView();
}

void UMLApp::slotAllViewsExportImage() {
    m_imageExporterAll->exportAllViews();
}

void UMLApp::slotCurrentViewProperties() {
    getCurrentView()->showPropDialog();
}

void UMLApp::setDiagramMenuItemsState(bool bState) {
    viewClearDiagram->setEnabled( bState );
    viewSnapToGrid->setEnabled( bState );
    viewShowGrid->setEnabled( bState );
    deleteDiagram->setEnabled(bState);
    viewExportImage->setEnabled( bState );
    viewProperties->setEnabled( bState );
    filePrint->setEnabled( bState );
    if ( getCurrentView() ) {
        viewSnapToGrid->setChecked( getCurrentView()->getSnapToGrid() );
        viewShowGrid->setChecked( getCurrentView()->getShowSnapGrid() );
    }
}

void UMLApp::slotUpdateViews() {
    QMenu* menu = findMenu( menuBar(), QString("views") );
    if (!menu) {
        kWarning() << "view menu not found";
        return;
    }

    menu = findMenu( menu, QString("show_view") );
    if (!menu) {
        kWarning() << "show menu not found";
        return;
    }

    menu->clear();

    UMLViewList views = getDocument()->getViewIterator();
    for(UMLView *view = views.first(); view; view = views.next()) {
        menu->insertItem( view->getName(), view, SLOT( slotShowView() ) );
        view->fileLoaded();
    }
}


void UMLApp::importFiles(QStringList* fileList) {
    if (! fileList->isEmpty()) {
        const QString& firstFile = fileList->first();
        ClassImport *classImporter = ClassImport::createImporterByFileExt(firstFile);
        classImporter->importFiles(*fileList);
        delete classImporter;
        m_doc->setLoading(false);
        //Modification is set after the import is made, because the file was modified when adding the classes
        //Allowing undo of the whole class importing. I think it eats a lot of memory
        //m_doc->setModified(true);
        //Setting the modification, but without allowing undo
        m_doc->setModified(true);
    }
}

void UMLApp::slotImportClasses() {
    m_doc->setLoading(true);
    // File selection is separated from invocation of ClassImport::import()
    // because the user might decide to choose a language different from
    // the active language (by using the "All Files" option).
    QString preselectedExtension;
    const Uml::Programming_Language pl = m_codegen->getLanguage();
    if (pl == Uml::pl_IDL) {
        preselectedExtension = i18n("*.idl|IDL Files (*.idl)");
    } else if (pl == Uml::pl_Python) {
        preselectedExtension = i18n("*.py|Python Files (*.py)");
    } else if (pl == Uml::pl_Java) {
        preselectedExtension = i18n("*.java|Java Files (*.java)");
    } else if (pl == Uml::pl_Pascal) {
        preselectedExtension = i18n("*.pas|Pascal Files (*.pas)");
    } else if (pl == Uml::pl_Ada) {
        preselectedExtension = i18n("*.ads *.ada|Ada Files (*.ads *.ada)");
    } else {
        preselectedExtension = i18n("*.h *.hh *.hpp *.hxx *.H|Header Files (*.h *.hh *.hpp *.hxx *.H)");
    }
    preselectedExtension.append("\n*|" + i18n("All Files"));
    QStringList fileList = KFileDialog::getOpenFileNames(KUrl(), preselectedExtension,
                           this, i18n("Select Code to Import") );
    importFiles(&fileList);
}

void UMLApp::slotImportProject() {
    QStringList listFile;

    QDialog::DialogCode code = ImportProjectDlg::getFilesToImport(&listFile,m_codegen->getLanguage(), this);
    if (code == QDialog::Accepted) {
        importFiles(&listFile);
    }
}

void UMLApp::slotClassWizard() {
    ClassWizard dlg( m_doc );
    dlg.exec();
}

void UMLApp::slotAddDefaultDatatypes() {
    m_doc->addDefaultDatatypes();
}

void UMLApp::slotCurrentViewChanged() {
    UMLView *view = getCurrentView();
    if (view) {
        connect(view, SIGNAL( sigShowGridToggled(bool) ),
                this, SLOT( slotShowGridToggled(bool) ) );
        connect(view, SIGNAL( sigSnapToGridToggled(bool) ),
                this, SLOT( slotSnapToGridToggled(bool) ) );
    }
}
void UMLApp::slotSnapToGridToggled(bool gridOn) {
    viewSnapToGrid->setChecked(gridOn);
}

void UMLApp::slotShowGridToggled(bool gridOn) {
    viewShowGrid->setChecked(gridOn);
}

void UMLApp::slotSelectAll() {
    getCurrentView()->selectAll();
}

void UMLApp::slotDeleteSelectedWidget() {
    if ( getCurrentView() ) {
        getCurrentView()->deleteSelection();
    } else {
        kWarning() << " trying to delete widgets when there is no current view (see bug 59774)";
    }
}

void UMLApp::slotDeleteDiagram() {
    m_doc->removeDiagram( getCurrentView()->getID() );
}

Uml::Programming_Language UMLApp::getDefaultLanguage() {
    Settings::OptionState& optionState = Settings::getOptionState();
    return optionState.generalState.defaultLanguage;
}

void UMLApp::initGenerator() {
    if (m_codegen) {
        delete m_codegen;
        m_codegen = NULL;
    }
    Uml::Programming_Language defaultLanguage = getDefaultLanguage();
    setActiveLanguage(defaultLanguage);
}

void UMLApp::updateLangSelectMenu(Uml::Programming_Language activeLanguage) {
    //m_langSelect->clear();
    for (int i = 0; i < Uml::pl_Reserved; i++) {
        m_langAct[i]->setChecked(i == activeLanguage);
    }
}


void UMLApp::keyPressEvent(QKeyEvent *e) {
    switch(e->key()) {
    case Qt::Key_Shift:
        //toolsbar->setOldTool();
        e->accept();
        break;

    default:
        e->ignore();
    }

}

void UMLApp::customEvent(QCustomEvent* e) {
    if (e->type() == CmdLineExportAllViewsEvent::getType()) {
        CmdLineExportAllViewsEvent* exportAllViewsEvent = static_cast<CmdLineExportAllViewsEvent*>(e);
        exportAllViewsEvent->exportAllViews();
    }
}

//TODO Move this to UMLWidgetController?
void UMLApp::handleCursorKeyReleaseEvent(QKeyEvent* e) {
    // in case we have selected something in the diagram, move it by one pixel
    // to the direction pointed by the cursor key
    if (m_view == NULL || !m_view->getSelectCount() || e->state() != Qt::AltButton) {
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
    m_view->moveSelectedBy(dx, dy);

    // notify about modification only at the first key release of possible sequence of auto repeat key releases,
    // this reduces the slow down caused by setModified() and makes the cursor moving of widgets smoother
    if (!e->isAutoRepeat()) {
        m_doc->setModified();
    }
    e->accept();
}

void UMLApp::keyReleaseEvent(QKeyEvent *e) {
    switch(e->key()) {
    case Qt::Key_Backspace:
        if (!m_pDocWindow->isTyping())
            toolsbar->setOldTool();
        e->accept();
        break;
    case Qt::Key_Escape:
        toolsbar->setDefaultTool();
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

void UMLApp::newDocument() {
    m_doc->newDocument();
    slotUpdateViews();
}

QWidget* UMLApp::getMainViewWidget() {
    Settings::OptionState& optionState = Settings::getOptionState();
    if ( optionState.generalState.tabdiagrams )
        return m_tabWidget;
    else
        return m_viewStack;
}

void UMLApp::setCurrentView(UMLView* view) {
    m_view = view;
    if (view == NULL) {
        kDebug() << "UMLApp::setCurrentView: view is NULL";
        return;
    }

    Settings::OptionState optionState = Settings::getOptionState();
    if (optionState.generalState.tabdiagrams) {
        if ( m_tabWidget->indexOf (view) < 0 ) {
            m_tabWidget->addTab(view, view->getName());
            m_tabWidget->setTabIconSet(view, Widget_Utils::iconSet(view->getType()));
        }

        m_tabWidget->showPage(view);
        m_tabWidget->setCurrentPage(m_tabWidget->currentPageIndex());
    }
    else {
        if (m_viewStack->indexOf(view) < 0) m_viewStack->addWidget(view);
        m_viewStack->setCurrentWidget(view);
        view->show();
    }
    qApp->processEvents();
    slotStatusMsg(view->getName());
    UMLListViewItem* lvitem = m_listView->findView(view);
    if (lvitem)
        m_listView->setCurrentItem(lvitem);
}

UMLView* UMLApp::getCurrentView() {
    return m_view;
}

QMenu* UMLApp::findMenu(QMenu* /*menu*/, const QString &name) {
    QWidget* widget = factory()->container(name, this);
    if (widget)
        return dynamic_cast<QMenu*>(widget);
    kDebug() << "UMLApp::findMenu: factory()->container(" << name << ") returns NULL";
    return 0;
}

void UMLApp::slotTabChanged(QWidget* view) {
    UMLView* umlview = ( UMLView* )view;
    m_doc->changeCurrentView( umlview->getID() );
}

void UMLApp::slotChangeTabLeft() {
    if (m_tabWidget) {
        m_tabWidget->setCurrentPage( m_tabWidget->currentPageIndex() - 1 );
        return;
    }
    UMLViewList views = m_doc->getViewIterator();
    UMLView *currView = m_view;
    if (views.find(currView) < 0) {
        kError() << "UMLApp::slotChangeTabLeft(): currView not found in viewlist" << endl;
        return;
    }
    if ((currView = views.prev()) != NULL)
        setCurrentView(currView);
    else
        setCurrentView(views.last());
}

void UMLApp::slotChangeTabRight() {
    if (m_tabWidget) {
        m_tabWidget->setCurrentPage( m_tabWidget->currentPageIndex() + 1 );
        return;
    }
    UMLViewList views = m_doc->getViewIterator();
    UMLView *currView = m_view;
    if (views.find(currView) < 0) {
        kError() << "UMLApp::slotChangeTabRight(): currView not found in viewlist" << endl;
        return;
    }
    if ((currView = views.next()) != NULL)
        setCurrentView(currView);
    else
        setCurrentView(views.first());
}

void UMLApp::slotMoveTabLeft() {
    //causes problems
    //does strange things when moving right most diagram to the right
    //doesn't save order in file
    //m_tabWidget->moveTab( m_tabWidget->currentPageIndex(), m_tabWidget->currentPageIndex() - 1 );
}

void UMLApp::slotMoveTabRight() {
    //causes problems
    //m_tabWidget->moveTab( m_tabWidget->currentPageIndex(), m_tabWidget->currentPageIndex() + 1 );
}

void UMLApp::slotXhtmlDocGenerationFinished()
{
  delete m_xhtmlGenerator;
  m_xhtmlGenerator = 0;
}

KTabWidget* UMLApp::tabWidget() {
    return m_tabWidget;
}

QString UMLApp::getStatusBarMsg() {
    return statusBar()->itemText(1);
}

void UMLApp::clearUndoStack() {
    m_pUndoStack->clear();
}

void UMLApp::undo()
{
    kDebug() << "UMLApp::undo(" << m_pUndoStack->undoText() << ") [" << m_pUndoStack->count() << "]";
    m_pUndoStack->undo();

    if(m_pUndoStack->canUndo())
        UMLApp::app()->enableUndo(true);
    else
        UMLApp::app()->enableUndo(false);

    UMLApp::app()->enableRedo(true);
}

void UMLApp::redo()
{
    kDebug() << "UMLApp::undo(" << m_pUndoStack->redoText() << ") [" << m_pUndoStack->count() << "]";
    m_pUndoStack->redo();

    if(m_pUndoStack->canRedo())
        UMLApp::app()->enableRedo(true);
    else
        UMLApp::app()->enableRedo(false);

    UMLApp::app()->enableUndo(true);
}

void UMLApp::executeCommand(QUndoCommand* cmd)
{
    if(cmd != NULL)
        m_pUndoStack->push(cmd);

    kDebug() << "UMLApp::executeCommand(" << cmd->text() << ") [" << m_pUndoStack->count() << "]";

    UMLApp::app()->enableUndo(true);
}

void UMLApp::BeginMacro( const QString & text ) {
    if(m_hasBegunMacro)
        return;

    m_hasBegunMacro = true;

    m_pUndoStack->beginMacro(text);
}

void UMLApp::EndMacro() {
    if(m_hasBegunMacro)
        m_pUndoStack->endMacro();

    m_hasBegunMacro = false;
}

//static pointer, holding the unique instance
UMLApp* UMLApp::s_instance;

#include "uml.moc"
