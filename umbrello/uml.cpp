/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "uml.h"

// qt includes
#include <qclipboard.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qwidgetstack.h>
#include <qslider.h>
#include <qregexp.h>
#include <qtoolbutton.h>

// kde includes
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kprinter.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <ktip.h>
#if KDE_IS_VERSION(3,1,90)
#include <ktabwidget.h>
#endif
#include <ktoolbarbutton.h>
#include <kpopupmenu.h>

// app includes
#include "aligntoolbar.h"
#include "infowidget.h"
#include "classimport.h"
#include "docwindow.h"
#include "codegenerator.h"
#include "generatorinfo.h"
#include "codegenerationpolicy.h"
#include "codegenerators/codegenfactory.h"
#include "widget_utils.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlviewlist.h"
#include "worktoolbar.h"

#include "clipboard/umlclipboard.h"
#include "clipboard/umldrag.h"
#include "dialogs/classwizard.h"
#include "dialogs/codegenerationwizard.h"
#include "dialogs/codeviewerdialog.h"
#include "dialogs/diagramprintpage.h"

#include "refactoring/refactoringassistant.h"
#include "codegenerators/simplecodegenerator.h"

#include "kplayerslideraction.h"

#include "configurable.h"


UMLApp::UMLApp(QWidget* , const char* name):KDockMainWindow(0, name) {
    s_instance = this;
    m_pDocWindow = 0;
    m_config = kapp->config();
    m_listView = 0;
    m_generatorDict.setAutoDelete(true);
    m_langSelect = NULL;
    m_zoomSelect = NULL;
    m_loading = false;
    m_clipTimer = 0;
    m_copyTimer = 0;
    m_defaultcodegenerationpolicy = 0;
    ///////////////////////////////////////////////////////////////////
    // call inits to invoke all other construction parts
    readOptionState();
    m_doc = new UMLDoc();
    m_classImporter = new ClassImport();
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

    //get a reference to the Code->Active Lanugage and to the Diagram->Zoom menu
    QPopupMenu* menu = findMenu( menuBar(), QString("code") );
    m_langSelect = findMenu( menu, QString("active_lang_menu") );

    //in case langSelect hasnt been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if someone takes the entry away from the ui.rc file
    if (m_langSelect == NULL) {
        m_langSelect = new QPopupMenu(this);
    }

    menu = findMenu( menuBar(), QString("views") );
    m_zoomSelect = findMenu( menu, QString("zoom_menu") );

    //in case zoomSelect hasnt been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if some one takes the entry away from the ui.rc file
    if (m_zoomSelect == NULL) {
        m_zoomSelect = new QPopupMenu(this);
    }

    //connect zoomSelect menu
    m_zoomSelect->setCheckable(true);
    connect(m_zoomSelect,SIGNAL(aboutToShow()),this,SLOT(setupZoomMenu()));
    connect(m_zoomSelect,SIGNAL(activated(int)),this,SLOT(setZoom(int)));

    m_refactoringAssist = 0L;

    m_defaultcodegenerationpolicy = new CodeGenerationPolicy(m_config);

}

UMLApp::~UMLApp() {
    delete m_clipTimer;
    delete m_copyTimer;

    delete statProg;
    delete m_statusLabel;
    delete m_refactoringAssist;
}

UMLApp* UMLApp::app()
{
    return s_instance;
}

void UMLApp::initActions() {
    fileNew = KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
    fileOpen = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
    fileOpenRecent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
    fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
    fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
    fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
    filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());
    fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
    editUndo = KStdAction::undo(this, SLOT(slotEditUndo()), actionCollection());
    editRedo = KStdAction::redo(this, SLOT(slotEditRedo()), actionCollection());
    editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection());
    editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection());
    editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());
#if KDE_IS_VERSION(3,1,90)
    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);
#else
    viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
    viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
#endif
    selectAll = KStdAction::selectAll(this,  SLOT( slotSelectAll() ), actionCollection());

    classWizard = new KAction(i18n("&New Class Wizard..."),0,this,SLOT(slotClassWizard()),
                              actionCollection(),"class_wizard");
    new KAction(i18n("&Add Default Datatypes for Active Language"), 0, this,
                SLOT(slotAddDefaultDatatypes()), actionCollection(), "create_default_datatypes");

    preferences = KStdAction::preferences(this,  SLOT( slotPrefs() ), actionCollection());

    genWizard = new KAction(i18n("&Code Generation Wizard..."),0,this,SLOT(generationWizard()),
                            actionCollection(),"generation_wizard");
    genAll = new KAction(i18n("&Generate All Code"),0,this,SLOT(generateAllCode()),
                         actionCollection(),"generate_all");

    importClasses = new KAction(i18n("&Import Classes..."), SmallIconSet("source_cpp"), 0,
                                this,SLOT(slotImportClasses()), actionCollection(),"import_class");

    fileNew->setToolTip(i18n("Creates a new document"));
    fileOpen->setToolTip(i18n("Opens an existing document"));
    fileOpenRecent->setToolTip(i18n("Opens a recently used file"));
    fileSave->setToolTip(i18n("Saves the document"));
    fileSaveAs->setToolTip(i18n("Saves the document as..."));
    fileClose->setToolTip(i18n("Closes the document"));
    filePrint ->setToolTip(i18n("Prints out the document"));
    fileQuit->setToolTip(i18n("Quits the application"));
    editCut->setToolTip(i18n("Cuts the selected section and puts it to the clipboard"));
    editCopy->setToolTip(i18n("Copies the selected section to the clipboard"));
    editPaste->setToolTip(i18n("Pastes the contents of the clipboard"));
#if !KDE_IS_VERSION(3,1,90)
    viewToolBar->setToolTip(i18n("Enables/disables the toolbar"));
    viewStatusBar->setToolTip(i18n("Enables/disables the statusbar"));
#endif
    preferences->setToolTip( i18n( "Set the default program preferences") );

    deleteSelectedWidget = new KAction( i18n("Delete &Selected"),
                                        SmallIconSet("editdelete"),
                                        KShortcut(Key_Delete), this,
                                        SLOT( slotDeleteSelectedWidget() ), actionCollection(),
                                        "delete_selected" );

    // The different views
    newDiagram = new KActionMenu(0, SmallIconSet("filenew"), actionCollection(), "new_view");
    classDiagram = new KAction( i18n( "&Class Diagram..." ), SmallIconSet("umbrello_diagram_class"), 0,
                                this, SLOT( slotClassDiagram() ), actionCollection(), "new_class_diagram" );

    sequenceDiagram= new KAction( i18n( "&Sequence Diagram..." ), SmallIconSet("umbrello_diagram_sequence"), 0,
                                  this, SLOT( slotSequenceDiagram() ), actionCollection(), "new_sequence_diagram" );

    collaborationDiagram = new KAction( i18n( "C&ollaboration Diagram..." ), SmallIconSet("umbrello_diagram_collaboration"), 0,
                                        this, SLOT( slotCollaborationDiagram() ), actionCollection(), "new_collaboration_diagram" );

    useCaseDiagram= new KAction( i18n( "&Use Case Diagram..." ), SmallIconSet("umbrello_diagram_usecase"), 0,
                                 this, SLOT( slotUseCaseDiagram() ), actionCollection(), "new_use_case_diagram" );

    stateDiagram= new KAction( i18n( "S&tate Diagram..." ), SmallIconSet("umbrello_diagram_state"), 0,
                               this, SLOT( slotStateDiagram() ), actionCollection(), "new_state_diagram" );

    activityDiagram= new KAction( i18n( "&Activity Diagram..." ), SmallIconSet("umbrello_diagram_activity"), 0,
                                  this, SLOT( slotActivityDiagram() ), actionCollection(), "new_activity_diagram" );

    componentDiagram = new KAction( i18n("Co&mponent Diagram..."), SmallIconSet("umbrello_diagram_component"), 0,
                                    this, SLOT( slotComponentDiagram() ), actionCollection(),
                                    "new_component_diagram" );

    deploymentDiagram = new KAction( i18n("&Deployment Diagram..."), SmallIconSet("umbrello_diagram_deployment"), 0,
                                     this, SLOT( slotDeploymentDiagram() ), actionCollection(),
                                     "new_deployment_diagram" );

    entityRelationshipDiagram = new KAction( i18n("&Entity Relationship Diagram..."), SmallIconSet("umbrello_diagram_entityrelationship"), 0,
                                this, SLOT( slotEntityRelationshipDiagram() ), actionCollection(),
                                "new_entityrelationship_diagram" );

    viewClearDiagram = new KAction(i18n("&Clear Diagram"), SmallIconSet("editclear"), 0,
                                   this, SLOT( slotCurrentViewClearDiagram() ), actionCollection(), "view_clear_diagram");
    viewSnapToGrid = new KToggleAction(i18n("&Snap to Grid"), 0,
                                       this, SLOT( slotCurrentViewToggleSnapToGrid() ), actionCollection(), "view_snap_to_grid");
    viewShowGrid = new KToggleAction(i18n("S&how Grid"), 0,
                                     this, SLOT( slotCurrentViewToggleShowGrid() ), actionCollection(), "view_show_grid");
#if (KDE_VERSION_MINOR>=3) && (KDE_VERSION_MAJOR>=3)
    viewShowGrid->setCheckedState(i18n("&Hide Grid"));
#endif
    deleteDiagram = new KAction(i18n("&Delete"), SmallIconSet("editdelete"), 0,
                                this, SLOT( slotDeleteDiagram() ), actionCollection(), "view_delete");
    viewExportImage = new KAction(i18n("&Export as Picture..."), SmallIconSet("image"), 0,
                                  this, SLOT( slotCurrentViewExportImage() ), actionCollection(), "view_export_image");
    viewProperties = new KAction(i18n("&Properties"), SmallIconSet("info"), 0,
                                 this, SLOT( slotCurrentViewProperties() ), actionCollection(), "view_properties");

    viewSnapToGrid->setChecked(false);
    viewShowGrid->setChecked(false);

    viewClearDiagram->setEnabled(false);
    viewSnapToGrid->setEnabled(false);
    viewShowGrid->setEnabled(false);
    deleteDiagram->setEnabled(false);
    viewExportImage->setEnabled(false);
    viewProperties->setEnabled(false);

    zoomAction = new KPlayerPopupSliderAction(i18n("&Zoom Slider"), "viewmag", Key_F9,
                 this, SLOT(slotZoomSliderMoved(int)),
                 actionCollection(), "popup_zoom");
    zoom100Action = new KAction(i18n( "Z&oom to 100%" ), "viewmag1", 0,
                                this, SLOT( slotZoom100() ), actionCollection(),
                                "zoom100");

    KStdAction::tipOfDay( this, SLOT( tipOfTheDay() ), actionCollection() );

    QString moveTabLeftString = i18n("&Move Tab Left");
    QString moveTabRightString = i18n("&Move Tab Right");
    moveTabLeft = new KAction(QApplication::reverseLayout() ? moveTabRightString : moveTabLeftString,
                              QApplication::reverseLayout() ? "forward" : "back",
                              QApplication::reverseLayout() ? Qt::CTRL+Qt::SHIFT+Qt::Key_Right : Qt::CTRL+Qt::SHIFT+Qt::Key_Left,
                              this, SLOT(slotMoveTabLeft()), actionCollection(),
                              "move_tab_left");
    moveTabRight = new KAction(QApplication::reverseLayout() ? moveTabLeftString : moveTabRightString,
                               QApplication::reverseLayout() ? "back" : "forward",
                               QApplication::reverseLayout() ? Qt::CTRL+Qt::SHIFT+Qt::Key_Left : Qt::CTRL+Qt::SHIFT+Qt::Key_Right,
                               this, SLOT(slotMoveTabRight()), actionCollection(),
                               "move_tab_right");

    QString selectTabLeftString = i18n("Select Diagram on Left");
    QString selectTabRightString = i18n("Select Diagram on Right");
    changeTabLeft = new KAction(QApplication::reverseLayout() ? selectTabRightString : selectTabLeftString,
                                QApplication::reverseLayout() ? Qt::SHIFT+Qt::Key_Right : Qt::SHIFT+Qt::Key_Left,
                                this, SLOT(slotChangeTabLeft()), actionCollection(), "previous_tab");
    changeTabRight = new KAction(QApplication::reverseLayout() ? selectTabLeftString : selectTabRightString,
                                 QApplication::reverseLayout() ? Qt::SHIFT+Qt::Key_Left : Qt::SHIFT+Qt::Key_Right,
                                 this, SLOT(slotChangeTabRight()), actionCollection(), "next_tab");


    initStatusBar(); //call this here because the statusBar is shown/hidden by setupGUI()

    // use the absolute path to your umbrelloui.rc file for testing purpose in setupGUI();
#if KDE_IS_VERSION(3,2,90)
    setupGUI();
#else
    createGUI();
#endif
    QPopupMenu* menu = findMenu( menuBar(), QString("settings") );
    menu->insertItem(i18n("&Windows"), dockHideShowMenu(), -1, 0);
}

void UMLApp::slotZoomSliderMoved(int value) {
    int zoom = (int)(value*0.01);
    m_doc->getCurrentView()->setZoom(zoom*zoom);
}

void UMLApp::slotZoom100()  {
    setZoom(100);
}

void UMLApp::setZoom(int zoom) {
    m_doc->getCurrentView()->setZoom(zoom);
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


    int zoom = m_doc->getCurrentView()->currentZoom();
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
    /* Progress bar removed, it didn't reflect the actual load status of a file
    statProg = new QProgressBar(statusBar(),"Progressbar");
    statProg->setFixedWidth( 100 );             // arbitrary width
    statProg->setCenterIndicator(true);
    statProg->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
    statProg->setMargin( 0 );
    statProg->setLineWidth(0);
    statProg->setBackgroundMode( QWidget::PaletteBackground );
    statProg->setFixedHeight( statProg->sizeHint().height() - 8 );
    */

    m_statusLabel = new KStatusBarLabel( i18n("Ready."), 0, statusBar() );
    m_statusLabel->setFixedHeight( m_statusLabel->sizeHint().height() );

    m_statusLabel->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
    m_statusLabel->setMargin( 0 );
    m_statusLabel->setLineWidth(0);

    statusBar()->addWidget( m_statusLabel, 1, false );

    /*
    statusBar()->addWidget(statProg, 0,  true);
    */

    m_statusLabel->setAlignment(AlignLeft|AlignVCenter);

    /*
    connect(m_doc,SIGNAL(sigSetStatusbarProgressSteps(int)),statProg,SLOT(setTotalSteps(int)));
    connect(m_doc,SIGNAL(sigSetStatusbarProgress(int)),statProg,SLOT(setProgress(int)));
    connect(m_doc,SIGNAL(sigResetStatusbarProgress()),statProg,SLOT(reset()));
    */

    connect(m_doc, SIGNAL( sigWriteToStatusBar(const QString &) ), this, SLOT( slotStatusMsg(const QString &) ));
}

void UMLApp::initView() {
    setCaption(m_doc->URL().fileName(),false);
    m_view = NULL;
    toolsbar = new WorkToolBar(this, "");
    toolsbar->setLabel(i18n("Diagram Toolbar"));
    addToolBar(toolsbar, Qt::DockTop, false);

    m_alignToolBar = new AlignToolBar(this, "");
    m_alignToolBar->setLabel(i18n("Alignment Toolbar"));
    addToolBar(m_alignToolBar, Qt::DockTop, false);

    m_mainDock = createDockWidget("maindock", 0L, 0L, "main dock");
#if KDE_IS_VERSION(3,1,90)
    m_newSessionButton = NULL;
    m_diagramMenu = NULL;
    m_closeDiagramButton = NULL;
    if (m_optionState.generalState.tabdiagrams) {
        m_viewStack = NULL;
        m_tabWidget = new KTabWidget(m_mainDock, "tab_widget");

#if KDE_IS_VERSION(3,3,89)
        m_tabWidget->setAutomaticResizeTabs( true );
#endif

        m_newSessionButton = new KToolBarButton("tab_new", 0, m_tabWidget);
        m_newSessionButton->setIconSet( SmallIcon( "tab_new" ) );
        m_newSessionButton->adjustSize();
        m_newSessionButton->setAutoRaise(true);
        m_diagramMenu = new KPopupMenu(m_newSessionButton);

        m_diagramMenu->insertItem(Umbrello::iconSet(Uml::dt_Class), i18n("Class Diagram..."), this, SLOT(slotClassDiagram()) );
        m_diagramMenu->insertItem(Umbrello::iconSet(Uml::dt_Sequence), i18n("Sequence Diagram..."), this, SLOT(slotSequenceDiagram()) );
        m_diagramMenu->insertItem(Umbrello::iconSet(Uml::dt_Collaboration), i18n("Collaboration Diagram..."), this, SLOT(slotCollaborationDiagram()) );
        m_diagramMenu->insertItem(Umbrello::iconSet(Uml::dt_UseCase), i18n("Use Case Diagram..."), this, SLOT(slotUseCaseDiagram()) );
        m_diagramMenu->insertItem(Umbrello::iconSet(Uml::dt_State), i18n("State Diagram..."), this, SLOT(slotStateDiagram()) );
        m_diagramMenu->insertItem(Umbrello::iconSet(Uml::dt_Activity), i18n("Activity Diagram..."), this, SLOT(slotActivityDiagram()) );
        m_diagramMenu->insertItem(Umbrello::iconSet(Uml::dt_Component), i18n("Component Diagram..."), this, SLOT(slotComponentDiagram()) );
        m_diagramMenu->insertItem(Umbrello::iconSet(Uml::dt_Deployment), i18n("Deployment Diagram..."), this, SLOT(slotDeploymentDiagram()) );
        m_diagramMenu->insertItem(Umbrello::iconSet(Uml::dt_EntityRelationship), i18n("Entity Relationship Diagram..."), this, SLOT(slotEntityRelationshipDiagram()) );
        m_newSessionButton->setPopup(m_diagramMenu);
        //FIXME why doesn't this work?
        //m_newSessionButton->setPopup(newDiagram->popupMenu());

        //m_closeDiagramButton = new KToolBarButton("tab_remove", 0, m_tabWidget);
        m_closeDiagramButton = new QToolButton(m_tabWidget);
        m_closeDiagramButton->setIconSet( SmallIcon("tab_remove") );
        m_closeDiagramButton->adjustSize();

        connect(m_closeDiagramButton, SIGNAL(clicked()), SLOT(slotDeleteDiagram()));
        connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)), SLOT(slotTabChanged(QWidget*)));
        connect(m_tabWidget, SIGNAL(contextMenu(QWidget*,const QPoint&)), m_doc, SLOT(slotDiagramPopupMenu(QWidget*,const QPoint&)));
        m_tabWidget->setCornerWidget( m_newSessionButton, TopLeft );
        m_tabWidget->setCornerWidget( m_closeDiagramButton, TopRight );
        m_newSessionButton->installEventFilter(this);

        m_mainDock->setWidget(m_tabWidget);
    }
    else
#endif
    {
        m_tabWidget = NULL;
        m_viewStack = new QWidgetStack(m_mainDock, "viewstack");
        m_mainDock->setWidget(m_viewStack);
    }
    m_mainDock->setDockSite(KDockWidget::DockCorner);
    m_mainDock->setEnableDocking(KDockWidget::DockNone);
    setView(m_mainDock);
    setMainDockWidget(m_mainDock);

    m_listDock = createDockWidget( "Model", 0L, 0L, i18n("&Tree View") );
    m_listView = new UMLListView(m_listDock ,"LISTVIEW");
    //m_listView->setSorting(-1);
    m_listDock->setWidget(m_listView);
    m_listDock->setDockSite(KDockWidget::DockCorner);
    m_listDock->manualDock(m_mainDock, KDockWidget::DockLeft, 20);

    m_documentationDock = createDockWidget( "Documentation", 0L, 0L, i18n("&Documentation") );
    m_pDocWindow = new DocWindow(m_doc, m_documentationDock, "DOCWINDOW");
    m_documentationDock->setWidget(m_pDocWindow);
    m_documentationDock->setDockSite(KDockWidget::DockCorner);
    m_documentationDock->manualDock(m_listDock, KDockWidget::DockBottom, 80);

    m_listView->setDocument(m_doc);
    m_doc->setupSignals();//make sure gets signal from list view

    readDockConfig(); //reposition all the DockWindows to their saved positions
}

void UMLApp::openDocumentFile(const KURL& url) {
    slotStatusMsg(i18n("Opening file..."));

    m_doc->openDocument( url);
    fileOpenRecent->addURL( url );
    slotStatusMsg(i18n("Ready."));
    setCaption(m_doc->URL().fileName(), false);
    enablePrint(true);

    // restore any saved code generators to memory.
    // This is important because we need to insure that
    // even if a code generator isnt worked with, it will
    // remain in sync with the UML diagram(s) should they change.
    initSavedCodeGenerators();

}

UMLDoc *UMLApp::getDocument() const {
    return m_doc;
}

UMLListView* UMLApp::getListView() {
    return m_listView;
}


void UMLApp::saveOptions() {
    toolBar("mainToolBar")->saveSettings(m_config, "toolbar");
    toolsbar->saveSettings(m_config, "workbar");
    m_alignToolBar->saveSettings(m_config, "aligntoolbar");
    fileOpenRecent->saveEntries(m_config,"Recent Files");
    m_config->setGroup( "General Options" );
    m_config->writeEntry( "Geometry", size() );

    m_config->writeEntry( "undo", m_optionState.generalState.undo );
    m_config->writeEntry( "tabdiagrams", m_optionState.generalState.tabdiagrams );
    m_config->writeEntry( "newcodegen", m_optionState.generalState.newcodegen );
    m_config->writeEntry( "autosave", m_optionState.generalState.autosave );
    m_config->writeEntry( "time", m_optionState.generalState.time );
    m_config->writeEntry( "autosavetime", m_optionState.generalState.autosavetime );
    m_config->writeEntry( "autosavesuffix", m_optionState.generalState.autosavesuffix );

    m_config->writeEntry( "logo", m_optionState.generalState.logo );
    m_config->writeEntry( "loadlast", m_optionState.generalState.loadlast );

    m_config->writeEntry( "diagram", m_optionState.generalState.diagram );
    if( m_doc->URL().fileName() == i18n( "Untitled" ) ) {
        m_config -> writeEntry( "lastFile", "" );
    } else {
#if KDE_IS_VERSION(3,1,3)
        m_config -> writePathEntry( "lastFile", m_doc -> URL().prettyURL() );
#else
        m_config -> writeEntry( "lastFile", m_doc -> URL().prettyURL() );
#endif
    }
    m_config->writeEntry( "imageMimetype", getImageMimetype() );

    m_config->setGroup( "TipOfDay");
    m_optionState.generalState.tip = m_config -> readBoolEntry( "RunOnStart", true );
    m_config->writeEntry( "RunOnStart", m_optionState.generalState.tip );

    m_config->setGroup( "UI Options" );
    m_config->writeEntry( "useFillColor", m_optionState.uiState.useFillColor );
    m_config->writeEntry( "fillColor", m_optionState.uiState.fillColor );
    m_config->writeEntry( "lineColor", m_optionState.uiState.lineColor );
    m_config->writeEntry( "lineWidth", m_optionState.uiState.lineWidth );
    m_config->writeEntry( "showDocWindow", m_documentationDock->isVisible() );
    m_config->writeEntry( "font", m_optionState.uiState.font );

    m_config->setGroup( "Class Options" );
    m_config->writeEntry( "showVisibility", m_optionState.classState.showScope );
    m_config->writeEntry( "showAtts", m_optionState.classState.showAtts);
    m_config->writeEntry( "showOps", m_optionState.classState.showOps );
    m_config->writeEntry( "showStereoType", m_optionState.classState.showStereoType );
    m_config->writeEntry( "showAttSig", m_optionState.classState.showAttSig );
    m_config->writeEntry( "ShowOpSig", m_optionState.classState.showOpSig );
    m_config->writeEntry( "showPackage", m_optionState.classState.showPackage );
    m_config->writeEntry( "defaultAttributeScope", m_optionState.classState.defaultAttributeScope);
    m_config->writeEntry( "defaultOperationScope", m_optionState.classState.defaultOperationScope);

    m_config -> setGroup( "Code Viewer Options" );
    m_config->writeEntry( "height", m_optionState.codeViewerState.height );
    m_config->writeEntry( "width", m_optionState.codeViewerState.width);
    m_config->writeEntry( "font", m_optionState.codeViewerState.font);
    m_config->writeEntry( "fontColor", m_optionState.codeViewerState.fontColor);
    m_config->writeEntry( "paperColor", m_optionState.codeViewerState.paperColor);
    m_config->writeEntry( "selectedColor", m_optionState.codeViewerState.selectedColor);
    m_config->writeEntry( "editBlockColor", m_optionState.codeViewerState.editBlockColor);
    m_config->writeEntry( "nonEditBlockColor", m_optionState.codeViewerState.nonEditBlockColor);
    m_config->writeEntry( "umlObjectBlockColor", m_optionState.codeViewerState.umlObjectColor);
    m_config->writeEntry( "blocksAreHighlighted", m_optionState.codeViewerState.blocksAreHighlighted);
    m_config->writeEntry( "showHiddenBlocks", m_optionState.codeViewerState.showHiddenBlocks);
    m_config->writeEntry( "hiddenColor", m_optionState.codeViewerState.hiddenColor);

    // merge current defaults into the default policy, just in case they
    // are out of sync (yes, yes, we could have a callback do this).
    CodeGenerator* gen = getGenerator(false);
    if(gen && gen->getPolicy())
        m_defaultcodegenerationpolicy->setDefaults(gen->getPolicy());

    // write the config for each language-specific code gen policies
    GeneratorDictIt it( m_generatorDict );
    for(it.toFirst() ; it.current(); ++it )
    {
        CodeGenerator * gen = m_doc->findCodeGeneratorByLanguage(it.current()->language);
        if (gen)
            gen->getPolicy()->writeConfig(m_config);
    }

    // now write the basic defaults to the m_config file
    m_defaultcodegenerationpolicy->writeConfig(m_config);

    // next, we record the activeLanguage in the Code Generation Group
    m_config->setGroup("Code Generation");
    m_config->writeEntry("activeLanguage", m_activeLanguage);
}

void UMLApp::readOptions() {
    // bar status settings
    toolBar("mainToolBar")->applySettings(m_config, "toolbar");
    // do config for work toolbar
    toolsbar->applySettings(m_config, "workbar");
    m_alignToolBar->applySettings(m_config, "aligntoolbar");
    fileOpenRecent->loadEntries(m_config,"Recent Files");
    m_config->setGroup("General Options");
    setImageMimetype(m_config->readEntry("imageMimetype","image/png"));
    QSize tmpQSize(630,460);
    resize( m_config->readSizeEntry("Geometry", & tmpQSize) );
}

void UMLApp::saveProperties(KConfig *_config) {
    if(m_doc->URL().fileName()!=i18n("Untitled") && !m_doc->isModified()) {
        // saving to tempfile not necessary

    } else {
        KURL url=m_doc->URL();
#if KDE_IS_VERSION(3,1,3)
        _config->writePathEntry("filename", url.url());
#else
        _config->writeEntry("filename", url.url());
#endif
        _config->writeEntry("modified", m_doc->isModified());
        QString tempname = kapp->tempSaveName(url.url());
        QString tempurl= KURL::encode_string(tempname);

        KURL _url(tempurl);
        m_doc->saveDocument(_url);
    }
}

void UMLApp::readProperties(KConfig* _config) {
    QString filename = _config->readPathEntry("filename");
    KURL url(filename);
    bool modified = _config->readBoolEntry("modified", false);
    if(modified) {
        bool canRecover;
        QString tempname = kapp->checkRecoverFile(filename, canRecover);
        KURL _url(tempname);


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
    writeDockConfig();
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
        setCaption(m_doc->URL().fileName(), false);
        fileOpenRecent->setCurrentItem( -1 );
        setModified(false);
        enablePrint(false);
    }
    setGenerator(createGenerator());
    slotUpdateViews();
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotFileOpen() {
    slotStatusMsg(i18n("Opening file..."));
    m_loading = true;

    if(!m_doc->saveModified()) {

        // here saving wasn't successful

    } else {
        KURL url=KFileDialog::getOpenURL(":open-umbrello-file",
                                         i18n("*.xmi *.xmi.tgz *.xmi.tar.bz2|All Supported Files (*.xmi, *.xmi.tgz, *.xmi.tar.bz2)\n*.xmi|Uncompressed XMI Files (*.xmi)\n*.xmi.tgz|Gzip Compressed XMI Files (*.xmi.tgz)\n*.xmi.tar.bz2|Bzip2 Compressed XMI Files (*.xmi.tar.bz2)"), this, i18n("Open File"));
        if(!url.isEmpty()) {
            if(m_doc->openDocument(url))
                fileOpenRecent->addURL( url );
            enablePrint(true);
            setCaption(m_doc->URL().fileName(), false);
        }

    }
    setGenerator(createGenerator());
    slotUpdateViews();
    m_loading = false;
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotFileOpenRecent(const KURL& url) {
    slotStatusMsg(i18n("Opening file..."));
    m_loading = true;

    KURL oldURL = m_doc->URL();

    if(!m_doc->saveModified()) {
        // here saving wasn't successful
    } else {
        if(!m_doc->openDocument(url)) {
            fileOpenRecent->removeURL(url);
            fileOpenRecent->setCurrentItem( -1 );
        }
        enablePrint(true);
        setCaption(m_doc->URL().fileName(), false);
    }

    m_loading = false;
    setGenerator(createGenerator());
    slotUpdateViews();
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotFileSave() {
    slotStatusMsg(i18n("Saving file..."));
    if(m_doc->URL().fileName() == i18n("Untitled"))
        slotFileSaveAs();
    else
        m_doc->saveDocument(m_doc -> URL());

    slotStatusMsg(i18n("Ready."));
}

bool UMLApp::slotFileSaveAs()
{
    slotStatusMsg(i18n("Saving file with a new filename..."));
    bool cont = true;
    KURL url;
    QString ext;
    while(cont) {
        url=KFileDialog::getSaveURL(":save-umbrello-file", i18n("*.xmi|XMI File\n*.xmi.tgz|Gzip Compressed XMI File\n*.xmi.tar.bz2|Bzip2 Compressed XMI File\n*|All Files"), this, i18n("Save As"));

        if(url.isEmpty())
            cont = false;
        else {
            // now check that we have a file extension; standard will be plain xmi
            QString file = url.path(-1);
            QFileInfo info(file);
            ext = info.extension();
            if (ext != "xmi" && ext != "xmi.tgz" && ext != "xmi.tar.bz2")
            {
                url.setFileName(url.fileName() + ".xmi");
                ext = "xmi";
            }
            QDir d = url.path(-1);

            if(QFile::exists(d.path())) {
                int want_save = KMessageBox::warningYesNo(this, i18n("The file %1 exists.\nDo you wish to overwrite it?").arg(url.path()), i18n("Warning"));
                if(want_save == KMessageBox::Yes)
                    cont = false;
            } else
                cont = false;

        }
    }
    if(!url.isEmpty()) {
        bool b = m_doc->saveDocument(url);
        if (b) {
            fileOpenRecent->addURL(url);
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
    if (printer.setup(this, i18n("Print %1").arg(m_doc->URL().prettyURL()))) {

        m_doc -> print(&printer);
    }
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotFileQuit() {
    slotStatusMsg(i18n("Exiting..."));
    if(m_doc->saveModified()) {
        writeDockConfig();
        saveOptions();
        kapp->quit();
    }
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotEditUndo() {
    m_doc->loadUndoData();
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotEditRedo() {
    m_doc->loadRedoData();
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotEditCut() {
    slotStatusMsg(i18n("Cutting selection..."));
    //FIXME bug 59774 this fromview isn't very reliable.
    //when cutting diagrams it is set to true even though it shouldn't be
    bool fromview = (m_doc->getCurrentView() && m_doc->getCurrentView()->getSelectCount());
    if ( editCutCopy(fromview) ) {
        emit sigCutSuccessful();
        slotDeleteSelectedWidget();
        m_doc->setModified(true);
    }
    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotEditCopy() {
    slotStatusMsg(i18n("Copying selection to clipboard..."));
    bool  fromview = (m_doc->getCurrentView() && m_doc->getCurrentView()->getSelectCount());
    editCutCopy( fromview );
    slotStatusMsg(i18n("Ready."));
    m_doc -> setModified( true );
}

void UMLApp::slotEditPaste() {
    slotStatusMsg(i18n("Inserting clipboard contents..."));
    QMimeSource* data = QApplication::clipboard()->data();
    UMLClipboard clipboard;
    setCursor(KCursor::waitCursor());
    if(!clipboard.paste(data)) {
        KMessageBox::sorry( this, i18n("Umbrello could not paste the clipboard contents.  "
                                       "The objects in the clipboard may be of the wrong "
                                       "type to be pasted here."), i18n("Paste Error") );
    }
    slotStatusMsg(i18n("Ready."));
    setCursor(KCursor::arrowCursor());
    editPaste->setEnabled(false);
    m_doc -> setModified( true );
}

//Remove these once we stop supporting KDE 3.1
// #if !KDE_IS_VERSION(3,1,90)

void UMLApp::slotViewToolBar() {
    slotStatusMsg(i18n("Toggling toolbar..."));

    ///////////////////////////////////////////////////////////////////
    // turn Toolbar on or off

    if(!viewToolBar->isChecked()) {
        toolBar("mainToolBar")->hide();
    } else {
        toolBar("mainToolBar")->show();
    }

    slotStatusMsg(i18n("Ready."));
}

void UMLApp::slotViewStatusBar() {
    slotStatusMsg(i18n("Toggle the statusbar..."));
    ///////////////////////////////////////////////////////////////////
    //turn Statusbar on or off
    if(!viewStatusBar->isChecked()) {
        statusBar()->hide();
    } else {
        statusBar()->show();
    }

    slotStatusMsg(i18n("Ready."));
}
// #endif


void UMLApp::slotStatusMsg(const QString &text) {
    ///////////////////////////////////////////////////////////////////
    // change status message permanently
    statusBar()->clear();
    m_statusLabel->setText( text );

    m_statusLabel->repaint();
}

void UMLApp::slotClassDiagram() {
    getDocument() -> createDiagram( Uml::dt_Class ) ;
}


void UMLApp::slotSequenceDiagram() {
    getDocument() -> createDiagram( Uml::dt_Sequence );
}

void UMLApp::slotCollaborationDiagram() {
    getDocument() -> createDiagram( Uml::dt_Collaboration );
}

void UMLApp::slotUseCaseDiagram() {
    getDocument() -> createDiagram( Uml::dt_UseCase );
}

void UMLApp::slotStateDiagram() {
    getDocument() -> createDiagram( Uml::dt_State );
}

void UMLApp::slotActivityDiagram() {
    getDocument() -> createDiagram( Uml::dt_Activity );
}

void UMLApp::slotComponentDiagram() {
    getDocument()->createDiagram( Uml::dt_Component );
}

void UMLApp::slotDeploymentDiagram() {
    getDocument()->createDiagram(Uml::dt_Deployment);
}

void UMLApp::slotEntityRelationshipDiagram() {
    getDocument()->createDiagram(Uml::dt_EntityRelationship);
}

WorkToolBar* UMLApp::getWorkToolBar() {
    return toolsbar;
}

void UMLApp::setModified(bool modified) {
    //fileSave -> setEnabled(modified);

    //if anything else needs to be done on a mofication, put it here

    // printing should be possible whenever there is something to print
    if ( m_loading == false && modified == true && m_doc->getCurrentView() )  {
        enablePrint(true);
    }

    if (m_loading == false)  {
        setCaption(m_doc->URL().fileName(), modified); //add disk icon to taskbar if modified
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
    //	m_clipTimer = new QTimer(this, "timer");
    //	m_clipTimer->start(1000, FALSE);
    //	connect(m_clipTimer, SIGNAL(timeout()), this, SLOT(slotClipDataChanged()));

    m_copyTimer = new QTimer(this, "copytimer");
    m_copyTimer->start(500, FALSE);
    connect(m_copyTimer, SIGNAL(timeout()), this, SLOT(slotCopyChanged()));
}

void UMLApp::slotClipDataChanged() {
    QMimeSource * data = QApplication::clipboard()->data();

    //Pass the MimeSource to the Doc
    editPaste->setEnabled( data && UMLDrag::canDecode(data) );
}

void UMLApp::slotCopyChanged() {
    if(m_listView->getSelectedCount() || (m_doc->getCurrentView() && m_doc->getCurrentView()->getSelectCount())) {
        editCopy->setEnabled(true);
        editCut->setEnabled(true);
    } else {
        editCopy->setEnabled(false);
        editCut->setEnabled(false);
    }
}

void UMLApp::slotPrefs() {
    /* the KTipDialog may have changed the value */
    m_config->setGroup("TipOfDay");
    m_optionState.generalState.tip = m_config->readBoolEntry( "RunOnStart", true );

    m_dlg = new SettingsDlg(this, &m_optionState, m_activeLanguage, getGenerator());
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
        m_config -> setGroup( "TipOfDay");
        m_config -> writeEntry( "RunOnStart", m_optionState.generalState.tip );

        m_doc -> settingsChanged( m_optionState );
        setActiveLanguage( m_dlg->getCodeGenerationLanguage() );
        m_dlg->setCodeGenerator(getDocument()->getCurrentCodeGenerator()); // do this AFTER setActiveLanguage;
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
    m_config -> setGroup( "General Options" );
    m_optionState.generalState.undo = m_config -> readBoolEntry( "undo", true );
    m_optionState.generalState.tabdiagrams = m_config -> readBoolEntry( "tabdiagrams",
#if KDE_IS_VERSION(3,1,90)
            true
#else
            false
#endif
                                                                      );
    m_optionState.generalState.newcodegen = m_config -> readBoolEntry( "newcodegen", true );
    m_optionState.generalState.autosave = m_config -> readBoolEntry( "autosave", true );
    m_optionState.generalState.time = m_config -> readNumEntry( "time", 0 ); //old autosavetime value kept for compatibility
    m_optionState.generalState.autosavetime = m_config -> readNumEntry( "autosavetime", 0 );
    //if we don't have a "new" autosavetime value, convert the old one
    if (m_optionState.generalState.autosavetime == 0) {
        switch (m_optionState.generalState.time) {
        case 0: m_optionState.generalState.autosavetime = 5; break;
        case 1: m_optionState.generalState.autosavetime = 10; break;
        case 2: m_optionState.generalState.autosavetime = 15; break;
        case 3: m_optionState.generalState.autosavetime = 20; break;
        case 4: m_optionState.generalState.autosavetime = 25; break;
        default: m_optionState.generalState.autosavetime = 5; break;
        }
    }
    // 2004-05-17 Achim Spangler: read new config entry for autosave sufix
    m_optionState.generalState.autosavesuffix = m_config -> readEntry( "autosavesuffix", ".xmi" );

    m_optionState.generalState.logo = m_config -> readBoolEntry( "logo", true );
    m_optionState.generalState.loadlast = m_config -> readBoolEntry( "loadlast", true );

    m_optionState.generalState.diagram  = ( Settings::Diagram ) m_config -> readNumEntry( "diagram", 1 );
    m_config -> setGroup( "TipOfDay");

    m_optionState.generalState.tip = m_config -> readBoolEntry( "RunOnStart", true );

    m_config -> setGroup( "UI Options" );
    m_optionState.uiState.useFillColor = m_config -> readBoolEntry( "useFillColor", true );
    QColor defaultYellow = QColor( 255, 255, 192 );

    m_optionState.uiState.fillColor = m_config -> readColorEntry( "fillColor", &defaultYellow );
    m_optionState.uiState.lineColor = m_config -> readColorEntry( "lineColor", &red );
    m_optionState.uiState.lineWidth = m_config -> readNumEntry( "lineWidth", 0 );
    QFont font = ((QWidget *) this)->font() ;
    m_optionState.uiState.font = m_config -> readFontEntry("font", &font );

    m_config -> setGroup( "Class Options" );

    m_optionState.classState.showScope = m_config -> readBoolEntry("showVisibility", true);
    m_optionState.classState.showAtts = m_config -> readBoolEntry("showAtts", true);
    m_optionState.classState.showOps = m_config -> readBoolEntry("showOps", true);
    m_optionState.classState.showStereoType = m_config -> readBoolEntry("showStereoType", false);
    m_optionState.classState.showAttSig = m_config -> readBoolEntry("showAttSig", true);
    m_optionState.classState.showOpSig = m_config -> readBoolEntry("ShowOpSig", true);
    m_optionState.classState.showPackage = m_config -> readBoolEntry("showPackage", false);
    m_optionState.classState.defaultAttributeScope = (Uml::Scope) m_config -> readNumEntry("defaultAttributeScope", Uml::Private);
    m_optionState.classState.defaultOperationScope = (Uml::Scope) m_config -> readNumEntry("defaultOperationScope", Uml::Public);

    m_config -> setGroup( "Code Viewer Options" );

    QColor defaultWhite = QColor( "white" );
    QColor defaultBlack = QColor( "black" );
    QColor defaultPink = QColor( "pink" );
    QColor defaultGrey = QColor( "grey" );

    m_optionState.codeViewerState.height = m_config -> readNumEntry( "height", 40 );
    m_optionState.codeViewerState.width = m_config -> readNumEntry( "width", 80 );
    m_optionState.codeViewerState.font = m_config -> readFontEntry("font", &font );
    m_optionState.codeViewerState.showHiddenBlocks = m_config -> readBoolEntry( "showHiddenBlocks", false);
    m_optionState.codeViewerState.blocksAreHighlighted = m_config -> readBoolEntry( "blocksAreHighlighted", false);
    m_optionState.codeViewerState.selectedColor = m_config -> readColorEntry( "selectedColor", &defaultYellow );
    m_optionState.codeViewerState.paperColor = m_config -> readColorEntry( "paperColor", &defaultWhite);
    m_optionState.codeViewerState.fontColor = m_config -> readColorEntry( "fontColor", &defaultBlack);
    m_optionState.codeViewerState.editBlockColor = m_config -> readColorEntry( "editBlockColor", &defaultPink);
    m_optionState.codeViewerState.umlObjectColor = m_config -> readColorEntry( "umlObjectBlockColor", &defaultPink);
    m_optionState.codeViewerState.nonEditBlockColor = m_config -> readColorEntry( "nonEditBlockColor", &defaultGrey);
    m_optionState.codeViewerState.hiddenColor = m_config -> readColorEntry( "hiddenColor", &defaultGrey);

}


/** Call the code viewing assistant on a code document */
void UMLApp::viewCodeDocument(UMLClassifier* classifier) {

    CodeGenerator * currentGen = getGenerator();
    if(currentGen && classifier) {
        if(!dynamic_cast<SimpleCodeGenerator*>(currentGen))
        {
            CodeDocument *cdoc = currentGen->findCodeDocumentByClassifier(classifier);

            if (cdoc) {
                CodeViewerDialog * dialog = currentGen->getCodeViewerDialog(this,cdoc,m_optionState.codeViewerState);
                dialog->exec();
                m_optionState.codeViewerState = dialog->getState();
                delete dialog;
                dialog = NULL;
            } else {
                // shouldnt happen..
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

void UMLApp::setGenerator(CodeGenerator* gen, bool giveWarning) {

    if (!gen) {
        if(giveWarning)  {
            KMessageBox::sorry(this, i18n("Could not find a code generator."),
                               i18n("No Code Generator"));
        }
        return;
    }

    // IF there is a current generator, it has a policy that MAY have
    // been changed. IF so, we should merge it back with our 'default'
    // policy. Yes, it would be better if we simply sync'd up all the
    // existing policies to the default policy via callbacks, but it
    // is more work and harder to implment. THis simple solution is ugly
    // but works. -b.t.
    CodeGenerator* current = getDocument()->getCurrentCodeGenerator();
    if (current)  {
        m_defaultcodegenerationpolicy->setDefaults(current->getPolicy(), false);
    }

    // now set defaults on the new generator policy from the default policy
    gen->getPolicy()->setDefaults(m_defaultcodegenerationpolicy, true);

    getDocument()->setCurrentCodeGenerator(gen);
}

CodeGenerator* UMLApp::getGenerator(bool warnMissing ) {

    CodeGenerator * gen = getDocument()->getCurrentCodeGenerator();

    if( !gen)
    {
        gen = createGenerator();
        setGenerator(gen, warnMissing);
    }

    return gen;
}

CodeGenerator* UMLApp::createGenerator() {
    GeneratorInfo* info;
    CodeGenerator* g = 0;

    if(m_activeLanguage.isEmpty()) {
        KMessageBox::sorry(this,i18n("There is no active language defined.\nPlease select "
                                     "one of the installed languages to generate the code with."),
                           i18n("No Language Selected"));
        return 0;
    }
    info = m_generatorDict.find(m_activeLanguage);
    if(!info) {
        KMessageBox::sorry(this,i18n("Could not find active language.\nPlease select "
                                     "one of the installed languages to generate the code with."),
                           i18n("No Language Selected"));
        return 0;
    }

    // does the code generator for our activeLanguage already exist?
    // then simply return that
    g = getDocument()->findCodeGeneratorByLanguage(m_activeLanguage);
    if(g) {
        return g;
    }

    CodeGeneratorFactory codeGeneratorFactory;
    QString generatorName = codeGeneratorFactory.generatorName(m_activeLanguage);
    g = codeGeneratorFactory.createObject(getDocument(), generatorName.latin1());
    if (getDocument()->getCurrentCodeGenerator() == NULL)
        getDocument()->setCurrentCodeGenerator(g);

    // now set defaults on the new policy from the configuration
    // file and THEN the default policy, which may have been updated
    // since it was first created. In both cases, DONT emit the modifiedCodeContent
    // signal as we will be doing that later.
    //
    g->getPolicy()->setDefaults(m_config, false); // picks up language specific stuff
    g->getPolicy()->setDefaults(m_defaultcodegenerationpolicy, false);

    // configure it from XMI
    QDomElement elem = getDocument()->getCodeGeneratorXMIParams( g->getLanguage() );
    g->loadFromXMI(elem);

    // add to our UML document
    getDocument()->addCodeGenerator(g);

    return g;

}

GeneratorDict& UMLApp::generatorDict() {
    return m_generatorDict;
}

void UMLApp::generateAllCode() {
    CodeGenerator* gen = getGenerator();
    if (gen) {
        gen->writeCodeToFile();
    }
}

void UMLApp::generationWizard() {
    CodeGenerationWizard wizard(m_doc, 0, m_activeLanguage, this);
    wizard.exec();
}

void UMLApp::setActiveLanguage(int id) {

    // only change the active language IF different from one we currently have
    if (!m_langSelect->isItemChecked(id))
    {

        for(unsigned int i=0; i < m_langSelect->count(); i++) {
            m_langSelect->setItemChecked(m_langSelect->idAt(i),false);  //uncheck everything
        }


        m_langSelect->setItemChecked(id,true);
        m_activeLanguage = m_langSelect->text(id).remove('&');

        // update the generator
        setGenerator(createGenerator());
    }
}

void UMLApp::setActiveLanguage( const QString &activeLanguage ) {

    for(unsigned int j=0; j < m_langSelect->count(); j++) {
        int id = m_langSelect->idAt(j);

        if (m_langSelect->text(id) == activeLanguage &&
                m_langSelect->isItemChecked(id))
            return; // already set.. no need to do anything
    }

    for(unsigned int i=0; i < m_langSelect->count(); i++) {
        bool isActiveLang = (m_langSelect->text(m_langSelect->idAt(i)) == activeLanguage);
        //uncheck everything except the active language
        m_langSelect->setItemChecked(m_langSelect->idAt(i), isActiveLang);
    }
    this->m_activeLanguage = activeLanguage;

    setGenerator(createGenerator());
}

void UMLApp::slotCurrentViewClearDiagram() {
    m_doc->getCurrentView()->clearDiagram();
}

void UMLApp::slotCurrentViewToggleSnapToGrid() {
    m_doc->getCurrentView()->toggleSnapToGrid();
    viewSnapToGrid->setChecked( m_doc->getCurrentView()->getSnapToGrid() );
}

void UMLApp::slotCurrentViewToggleShowGrid() {
    m_doc->getCurrentView()->toggleShowGrid();
    viewShowGrid->setChecked( m_doc->getCurrentView()->getShowSnapGrid() );
}

void UMLApp::slotCurrentViewExportImage() {
    m_doc->getCurrentView()->exportImage();
}

void UMLApp::slotCurrentViewProperties() {
    m_doc->getCurrentView()->showPropDialog();
}

void UMLApp::setDiagramMenuItemsState(bool bState) {
    viewClearDiagram->setEnabled( bState );
    viewSnapToGrid->setEnabled( bState );
    viewShowGrid->setEnabled( bState );
    deleteDiagram->setEnabled(bState);
    viewExportImage->setEnabled( bState );
    viewProperties->setEnabled( bState );
    filePrint->setEnabled( bState );
    if ( m_doc->getCurrentView() ) {
        viewSnapToGrid->setChecked( m_doc->getCurrentView()->getSnapToGrid() );
        viewShowGrid->setChecked( m_doc->getCurrentView()->getShowSnapGrid() );
    }
}

void UMLApp::slotUpdateViews() {
    QPopupMenu* menu = findMenu( menuBar(), QString("views") );
    if (!menu) {
        kdWarning() << "view menu not found" << endl;
        return;
    }

    menu = findMenu( menu, QString("show_view") );
    if (!menu) {
        kdWarning() << "show menu not found" << endl;
        return;
    }

    menu->clear();

    UMLViewList views = getDocument()->getViewIterator();
    for(UMLView *view = views.first(); view; view = views.next()) {
        menu->insertItem( view->getName(), view, SLOT( slotShowView() ) );
        view->fileLoaded();
    }
}

ClassImport * UMLApp::classImport() {
    return m_classImporter;
}

void UMLApp::slotImportClasses() {
    m_doc->setLoading(true);
    // File selection is separated from invocation of ClassImport::import()
    // because the user might decide to choose a language different from
    // the m_activeLanguage (by using the "All Files" option).
    QString preselectedExtension;
    if (m_activeLanguage == "IDL") {
        preselectedExtension = i18n("*.idl|IDL Files (*.idl)");
    } else if (m_activeLanguage == "Ada") {
        preselectedExtension = i18n("*.ads *.ada|Ada Files (*.ads *.ada)");
    } else {
        preselectedExtension = i18n("*.h *.hh *.hpp *.hxx *.H|Header Files (*.h *.hh *.hpp *.hxx *.H)");
    }
    preselectedExtension.append("\n*|" + i18n("All Files"));
    QStringList fileList = KFileDialog::getOpenFileNames(":import-classes", preselectedExtension,
                           this, i18n("Select Code to Import") );
    const QString& firstFile = fileList.first();
    if (firstFile.endsWith(".idl"))
        m_classImporter->importIDL( fileList );
    else if (firstFile.contains( QRegExp("\\.ad[sba]$") ))
        /* m_classImporter->importAda( fileList ) */;
    else
        m_classImporter->importCPP( fileList );	 // the default.
    m_doc->setLoading(false);
}

void UMLApp::slotClassWizard() {
    ClassWizard dlg( m_doc );
    dlg.exec();
}

void UMLApp::slotAddDefaultDatatypes() {
    m_doc->addDefaultDatatypes();
}

void UMLApp::slotCurrentViewChanged() {
    if ( m_doc->getCurrentView() ) {
        connect(m_doc->getCurrentView(), SIGNAL( sigShowGridToggled(bool) ),
                this, SLOT( slotShowGridToggled(bool) ) );
        connect(m_doc->getCurrentView(), SIGNAL( sigSnapToGridToggled(bool) ),
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
    m_doc->getCurrentView()->selectAll();
}

void UMLApp::slotDeleteSelectedWidget() {
    if ( m_doc->getCurrentView() ) {
        m_doc->getCurrentView()->deleteSelection();
    } else {
        kdWarning() << " trying to delete widgets when there is no current view (see bug 59774)" << endl;
    }
}

void UMLApp::slotDeleteDiagram() {
    m_doc->removeDiagram( m_doc->getCurrentView()->getID() );
}

void UMLApp::initGenerators() {
    m_config->setGroup("Code Generation");
    m_activeLanguage = m_config->readEntry("activeLanguage", "Cpp");

    CodeGeneratorFactory codeGeneratorFactory;
    QStringList languages = codeGeneratorFactory.languagesAvailable();

    QStringList::Iterator end(languages.end());
    for ( QStringList::Iterator langit(languages.begin()); langit != end; ++langit ) {
        GeneratorInfo* info;
        info = new GeneratorInfo;
        info->language = *langit;  //language name
        info->object = codeGeneratorFactory.generatorName(*langit);
        m_generatorDict.insert(info->language,info);
    }

    updateLangSelectMenu();
}

void UMLApp::updateLangSelectMenu() {
    m_langSelect->clear();
    m_langSelect->setCheckable(true);
    int id;
    bool foundActive = false;
    GeneratorDictIt it( m_generatorDict );
    for(it.toFirst() ; it.current(); ++it ) {
        id = m_langSelect->insertItem(it.current()->language,this,SLOT(setActiveLanguage(int)));
        if(m_activeLanguage == it.current()->language) {
            m_langSelect->setItemChecked(id,true);
            foundActive = true;
        } else
            m_langSelect->setItemChecked(id,false);
    }

    //if we could not find the active language, we try to fall back to C++
    if(!foundActive) {
        for(uint index=0; index <m_langSelect->count(); index++)
            if ( m_langSelect->text(m_langSelect->idAt(index)) == "Cpp" ) {
                m_langSelect->setItemChecked(m_langSelect->idAt(index),true);
                m_activeLanguage = m_langSelect->text(m_langSelect->idAt(index));
                break;
            }
    }
    //last try... if we dont have an activeLanguage and we have no Cpp installed we just
    //take the first language we find as "active"
    if(m_activeLanguage.isEmpty() && m_langSelect->count() > 0) {
        m_langSelect -> setItemChecked(m_langSelect->idAt(0),true);
        m_activeLanguage = m_langSelect -> text(m_langSelect->idAt(0));
    }

}

void UMLApp::tipOfTheDay()
{
    KTipDialog::showTip(this ,QString::null, true);
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
    setGenerator(createGenerator());
    slotUpdateViews();
}

void UMLApp::initSavedCodeGenerators() {
    QString activeLang = m_activeLanguage;
    GeneratorDictIt it( m_generatorDict );
    for(it.toFirst() ; it.current(); ++it )
    {
        m_activeLanguage = it.current()->language;
        if( m_doc->hasCodeGeneratorXMIParams(m_activeLanguage))
            createGenerator();
    }

    // now set back to old activeLanguage
    m_activeLanguage = activeLang;
    setGenerator(createGenerator());
}

QWidget* UMLApp::getMainViewWidget() {
#if KDE_IS_VERSION(3,1,90)
    if (m_optionState.generalState.tabdiagrams)
        return m_tabWidget;
#endif
    return m_viewStack;
}

void UMLApp::setCurrentView(UMLView* view /*=0*/) {
    m_view = view;
    if (m_viewStack == NULL)
        return;
    if (view) {
        m_viewStack->raiseWidget(view);
        slotStatusMsg(view->getName());
    } else {
        m_viewStack->raiseWidget(blankWidget);
    }
}

UMLView* UMLApp::getCurrentView() {
    return m_view;
}

QPopupMenu* UMLApp::findMenu(QMenuData* menu, const QString &name) {

    if (menu) {
        int menuCount = menu->count();

        for (int i=0; i<menuCount; i++) {
            int idAt = menu->idAt(i);
            QPopupMenu* popupMenu = menu->findItem(idAt)->popup();
            if (popupMenu) {
                QString menuName = popupMenu->name();
                if( menuName == name) {
                    return popupMenu;
                }
            }
        }
    }
    return 0;
}

void UMLApp::slotTabChanged(QWidget* view) {
    UMLView* umlview = ( UMLView* )view;
    m_doc->changeCurrentView( umlview->getID() );
}

void UMLApp::slotChangeTabLeft() {
#if KDE_IS_VERSION(3,1,90)
    m_tabWidget->setCurrentPage( m_tabWidget->currentPageIndex() - 1 );
#endif
}

void UMLApp::slotChangeTabRight() {
#if KDE_IS_VERSION(3,1,90)
    m_tabWidget->setCurrentPage( m_tabWidget->currentPageIndex() + 1 );
#endif
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

KTabWidget* UMLApp::tabWidget() {
    return m_tabWidget;
}

//static pointer, holding the unique instance
UMLApp* UMLApp::s_instance;

#include "uml.moc"
