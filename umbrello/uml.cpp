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
#include <ktabwidget.h>
#include <ktoolbarbutton.h>
#include <kpopupmenu.h>

// app includes
#include "aligntoolbar.h"
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
#ifdef HAVE_DOT
# include "autolayout/autolayoutdlg.h" //dimitri
#endif
#include "model_utils.h"
#include "clipboard/umlclipboard.h"
#include "dialogs/classwizard.h"
#include "dialogs/codegenerationwizard.h"
#include "dialogs/codeviewerdialog.h"
#include "dialogs/diagramprintpage.h"

#include "refactoring/refactoringassistant.h"
#include "codegenerators/simplecodegenerator.h"
#include "umlviewimageexporter.h"
#include "umlviewimageexporterall.h"

#include "kplayerslideraction.h"

#include "configurable.h"

#include "cmdlineexportallviewsevent.h"

#include "docgenerators/docbookgenerator.h"
#include "docgenerators/xhtmlgenerator.h"

UMLApp::UMLApp(QWidget* , const char* name):KDockMainWindow(0, name) {
    s_instance = this;
    m_pDocWindow = 0;
    m_config = kapp->config();
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
    m_doc = new UMLDoc();
    m_doc->init();
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
    QPopupMenu* menu = findMenu( menuBar(), QString("code") );
    m_langSelect = findMenu( menu, QString("active_lang_menu") );

    //in case langSelect hasn't been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if someone takes the entry away from the ui.rc file
    if (m_langSelect == NULL) {
        m_langSelect = new QPopupMenu(this);
    }

    menu = findMenu( menuBar(), QString("views") );
    m_zoomSelect = findMenu( menu, QString("zoom_menu") );

    //in case zoomSelect hasn't been initialized we create the Popup menu.
    //it will be hidden, but at least we wont crash if some one takes the entry away from the ui.rc file
    if (m_zoomSelect == NULL) {
        m_zoomSelect = new QPopupMenu(this);
    }

    //connect zoomSelect menu
    m_zoomSelect->setCheckable(true);
    connect(m_zoomSelect,SIGNAL(aboutToShow()),this,SLOT(setupZoomMenu()));
    connect(m_zoomSelect,SIGNAL(activated(int)),this,SLOT(setZoom(int)));

    m_refactoringAssist = 0L;

    m_commoncodegenpolicy = new CodeGenerationPolicy(m_config);

    m_imageExporterAll = new UMLViewImageExporterAll();
}

UMLApp::~UMLApp() {
    delete m_imageExporterAll;

    delete m_clipTimer;
    delete m_copyTimer;

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
    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);
    selectAll = KStdAction::selectAll(this,  SLOT( slotSelectAll() ), actionCollection());
    fileExportDocbook = new KAction(i18n("&Export model to DocBook"), 0,
                                    this, SLOT( slotFileExportDocbook() ),
                                    actionCollection(), "file_export_docbook");
    fileExportXhtml = new KAction(i18n("&Export model to XHTML"), 0,
                                    this, SLOT( slotFileExportXhtml() ),
                                    actionCollection(), "file_export_xhtml");

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
    fileExportDocbook->setToolTip(i18n("Exports the model to the docbook format"));
    fileExportXhtml->setToolTip(i18n("Exports the model to the XHTML format"));
    editCut->setToolTip(i18n("Cuts the selected section and puts it to the clipboard"));
    editCopy->setToolTip(i18n("Copies the selected section to the clipboard"));
    editPaste->setToolTip(i18n("Pastes the contents of the clipboard"));
    preferences->setToolTip( i18n( "Set the default program preferences") );

    deleteSelectedWidget = new KAction( i18n("Delete &Selected"),
                                        SmallIconSet("editdelete"),
                                        KShortcut(Qt::Key_Delete), this,
                                        SLOT( slotDeleteSelectedWidget() ), actionCollection(),
                                        "delete_selected" );

    // The different views
    newDiagram = new KActionMenu(0, SmallIconSet("filenew"), actionCollection(), "new_view");
    classDiagram = new KAction( i18n( "&Class Diagram..." ), SmallIconSet("umbrello_diagram_class"), 0,
                                this, SLOT( slotClassDiagram() ), actionCollection(), "new_class_diagram" );

#if defined (HAVE_DOT)
    autolayout = new KAction(i18n("&Autolayout..."),0,0,this,SLOT(slotAutolayout()),
                             actionCollection(),"autolayout");
#endif
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
    viewExportImageAll = new KAction(i18n("Export &All Diagrams as Pictures..."), SmallIconSet("image"), 0,
                                     this, SLOT( slotAllViewsExportImage() ), actionCollection(), "view_export_image_all");
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
    m_statusLabel = new KStatusBarLabel( i18n("Ready."), 0, statusBar() );
    m_statusLabel->setFixedHeight( m_statusLabel->sizeHint().height() );

    m_statusLabel->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
    m_statusLabel->setMargin( 0 );
    m_statusLabel->setLineWidth(0);

    statusBar()->addWidget( m_statusLabel, 1, false );

    m_statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

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
    m_newSessionButton = NULL;
    m_diagramMenu = NULL;
    m_closeDiagramButton = NULL;
    Settings::OptionState& optionState = Settings::getOptionState();
    if (optionState.generalState.tabdiagrams) {
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

        m_diagramMenu->insertItem(Widget_Utils::iconSet(Uml::dt_Class), i18n("Class Diagram..."), this, SLOT(slotClassDiagram()) );
        m_diagramMenu->insertItem(Widget_Utils::iconSet(Uml::dt_Sequence), i18n("Sequence Diagram..."), this, SLOT(slotSequenceDiagram()) );
        m_diagramMenu->insertItem(Widget_Utils::iconSet(Uml::dt_Collaboration), i18n("Collaboration Diagram..."), this, SLOT(slotCollaborationDiagram()) );
        m_diagramMenu->insertItem(Widget_Utils::iconSet(Uml::dt_UseCase), i18n("Use Case Diagram..."), this, SLOT(slotUseCaseDiagram()) );
        m_diagramMenu->insertItem(Widget_Utils::iconSet(Uml::dt_State), i18n("State Diagram..."), this, SLOT(slotStateDiagram()) );
        m_diagramMenu->insertItem(Widget_Utils::iconSet(Uml::dt_Activity), i18n("Activity Diagram..."), this, SLOT(slotActivityDiagram()) );
        m_diagramMenu->insertItem(Widget_Utils::iconSet(Uml::dt_Component), i18n("Component Diagram..."), this, SLOT(slotComponentDiagram()) );
        m_diagramMenu->insertItem(Widget_Utils::iconSet(Uml::dt_Deployment), i18n("Deployment Diagram..."), this, SLOT(slotDeploymentDiagram()) );
        m_diagramMenu->insertItem(Widget_Utils::iconSet(Uml::dt_EntityRelationship), i18n("Entity Relationship Diagram..."), this, SLOT(slotEntityRelationshipDiagram()) );
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
    m_listView->setDocument(m_doc);
    m_listView->init();
    m_listDock->setWidget(m_listView);
    m_listDock->setDockSite(KDockWidget::DockCorner);
    m_listDock->manualDock(m_mainDock, KDockWidget::DockLeft, 20);

    m_documentationDock = createDockWidget( "Documentation", 0L, 0L, i18n("&Documentation") );
    m_pDocWindow = new DocWindow(m_doc, m_documentationDock, "DOCWINDOW");
    m_documentationDock->setWidget(m_pDocWindow);
    m_documentationDock->setDockSite(KDockWidget::DockCorner);
    m_documentationDock->manualDock(m_listDock, KDockWidget::DockBottom, 80);

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

    Settings::OptionState& optionState = Settings::getOptionState();
    m_config->writeEntry( "undo", optionState.generalState.undo );
    m_config->writeEntry( "tabdiagrams", optionState.generalState.tabdiagrams );
    m_config->writeEntry( "newcodegen", optionState.generalState.newcodegen );
    m_config->writeEntry( "angularlines", optionState.generalState.angularlines );
    m_config->writeEntry( "autosave", optionState.generalState.autosave );
    m_config->writeEntry( "time", optionState.generalState.time );
    m_config->writeEntry( "autosavetime", optionState.generalState.autosavetime );
    m_config->writeEntry( "autosavesuffix", optionState.generalState.autosavesuffix );

    m_config->writeEntry( "logo", optionState.generalState.logo );
    m_config->writeEntry( "loadlast", optionState.generalState.loadlast );

    m_config->writeEntry( "diagram", optionState.generalState.diagram );
    if( m_doc->URL().fileName() == i18n( "Untitled" ) ) {
        m_config -> writeEntry( "lastFile", "" );
    } else {
        m_config -> writePathEntry( "lastFile", m_doc -> URL().prettyURL() );
    }
    m_config->writeEntry( "imageMimeType", getImageMimeType() );

    m_config->setGroup( "TipOfDay");
    optionState.generalState.tip = m_config -> readBoolEntry( "RunOnStart", true );
    m_config->writeEntry( "RunOnStart", optionState.generalState.tip );

    m_config->setGroup( "UI Options" );
    m_config->writeEntry( "useFillColor", optionState.uiState.useFillColor );
    m_config->writeEntry( "fillColor", optionState.uiState.fillColor );
    m_config->writeEntry( "lineColor", optionState.uiState.lineColor );
    m_config->writeEntry( "lineWidth", optionState.uiState.lineWidth );
    m_config->writeEntry( "showDocWindow", m_documentationDock->isVisible() );
    m_config->writeEntry( "font", optionState.uiState.font );

    m_config->setGroup( "Class Options" );
    m_config->writeEntry( "showVisibility", optionState.classState.showVisibility );
    m_config->writeEntry( "showAtts", optionState.classState.showAtts);
    m_config->writeEntry( "showOps", optionState.classState.showOps );
    m_config->writeEntry( "showStereoType", optionState.classState.showStereoType );
    m_config->writeEntry( "showAttSig", optionState.classState.showAttSig );
    m_config->writeEntry( "ShowOpSig", optionState.classState.showOpSig );
    m_config->writeEntry( "showPackage", optionState.classState.showPackage );
    m_config->writeEntry( "defaultAttributeScope", optionState.classState.defaultAttributeScope);
    m_config->writeEntry( "defaultOperationScope", optionState.classState.defaultOperationScope);

    m_config -> setGroup( "Code Viewer Options" );
    m_config->writeEntry( "height", optionState.codeViewerState.height );
    m_config->writeEntry( "width", optionState.codeViewerState.width);
    m_config->writeEntry( "font", optionState.codeViewerState.font);
    m_config->writeEntry( "fontColor", optionState.codeViewerState.fontColor);
    m_config->writeEntry( "paperColor", optionState.codeViewerState.paperColor);
    m_config->writeEntry( "selectedColor", optionState.codeViewerState.selectedColor);
    m_config->writeEntry( "editBlockColor", optionState.codeViewerState.editBlockColor);
    m_config->writeEntry( "nonEditBlockColor", optionState.codeViewerState.nonEditBlockColor);
    m_config->writeEntry( "umlObjectBlockColor", optionState.codeViewerState.umlObjectColor);
    m_config->writeEntry( "blocksAreHighlighted", optionState.codeViewerState.blocksAreHighlighted);
    m_config->writeEntry( "showHiddenBlocks", optionState.codeViewerState.showHiddenBlocks);
    m_config->writeEntry( "hiddenColor", optionState.codeViewerState.hiddenColor);

    // write the config for a language-specific code gen policy
    if (m_policyext)
        m_policyext->writeConfig(m_config);

    // now write the basic defaults to the m_config file
    m_commoncodegenpolicy->writeConfig(m_config);

    // next, we record the activeLanguage in the Code Generation Group
    if (m_codegen) {
        m_config->setGroup("Code Generation");
        m_config->writeEntry("activeLanguage", Model_Utils::progLangToString(m_codegen->getLanguage()));
    }
}

void UMLApp::readOptions() {
    // bar status settings
    toolBar("mainToolBar")->applySettings(m_config, "toolbar");
    // do config for work toolbar
    toolsbar->applySettings(m_config, "workbar");
    m_alignToolBar->applySettings(m_config, "aligntoolbar");
    fileOpenRecent->loadEntries(m_config,"Recent Files");
    m_config->setGroup("General Options");
    setImageMimeType(m_config->readEntry("imageMimeType","image/png"));
    QSize tmpQSize(630,460);
    resize( m_config->readSizeEntry("Geometry", & tmpQSize) );
}

void UMLApp::saveProperties(KConfig *_config) {
    if(m_doc->URL().fileName()!=i18n("Untitled") && !m_doc->isModified()) {
        // saving to tempfile not necessary

    } else {
        KURL url=m_doc->URL();
        _config->writePathEntry("filename", url.url());
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
            i18n("*.xmi *.xmi.tgz *.xmi.tar.bz2 *.mdl|All Supported Files (*.xmi, *.xmi.tgz, *.xmi.tar.bz2, *.mdl)\n"
                 "*.xmi|Uncompressed XMI Files (*.xmi)\n"
                 "*.xmi.tgz|Gzip Compressed XMI Files (*.xmi.tgz)\n"
                 "*.xmi.tar.bz2|Bzip2 Compressed XMI Files (*.xmi.tar.bz2)\n"
                 "*.mdl|Rose model files"), this, i18n("Open File"));
        if(!url.isEmpty()) {
            if(m_doc->openDocument(url))
                fileOpenRecent->addURL( url );
            enablePrint(true);
            setCaption(m_doc->URL().fileName(), false);
        }

    }
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
        } else {
            fileOpenRecent->addURL(url);
        }
        enablePrint(true);
        setCaption(m_doc->URL().fileName(), false);
    }

    m_loading = false;
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
            QDir d = url.path(-1);

            if(QFile::exists(d.path())) {
                int want_save = KMessageBox::warningContinueCancel(this, i18n("The file %1 exists.\nDo you wish to overwrite it?").arg(url.path()), i18n("Warning"), i18n("Overwrite"));
                if(want_save == KMessageBox::Continue)
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
    UMLFolder *root = m_doc->getRootFolder(Uml::mt_Logical);
    getDocument()->createDiagram(root, Uml::dt_Class);
}


void UMLApp::slotSequenceDiagram() {
    UMLFolder *root = m_doc->getRootFolder(Uml::mt_Logical);
    m_doc->createDiagram(root, Uml::dt_Sequence);
}

void UMLApp::slotCollaborationDiagram() {
    UMLFolder *root = m_doc->getRootFolder(Uml::mt_Logical);
    m_doc->createDiagram(root, Uml::dt_Collaboration);
}

void UMLApp::slotUseCaseDiagram() {
    UMLFolder *root = m_doc->getRootFolder(Uml::mt_UseCase);
    m_doc->createDiagram(root, Uml::dt_UseCase);
}

void UMLApp::slotStateDiagram() {
    UMLFolder *root = m_doc->getRootFolder(Uml::mt_Logical);
    m_doc->createDiagram(root, Uml::dt_State);
}

void UMLApp::slotActivityDiagram() {
    UMLFolder *root = m_doc->getRootFolder(Uml::mt_Logical);
    m_doc->createDiagram(root, Uml::dt_Activity);
}

void UMLApp::slotComponentDiagram() {
    UMLFolder *root = m_doc->getRootFolder(Uml::mt_Component);
    m_doc->createDiagram(root, Uml::dt_Component );
}

void UMLApp::slotDeploymentDiagram() {
    UMLFolder *root = m_doc->getRootFolder(Uml::mt_Deployment);
    m_doc->createDiagram(root, Uml::dt_Deployment);
}

void UMLApp::slotEntityRelationshipDiagram() {
    UMLFolder *root = m_doc->getRootFolder(Uml::mt_EntityRelationship);
    m_doc->createDiagram(root, Uml::dt_EntityRelationship);
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
    /* the KTipDialog may have changed the value */
    m_config->setGroup("TipOfDay");
    Settings::OptionState& optionState = Settings::getOptionState();
    optionState.generalState.tip = m_config->readBoolEntry( "RunOnStart", true );

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
        m_config -> setGroup( "TipOfDay");
        Settings::OptionState& optionState = Settings::getOptionState();
        m_config -> writeEntry( "RunOnStart", optionState.generalState.tip );

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
    m_config -> setGroup( "General Options" );
    Settings::OptionState& optionState = Settings::getOptionState();
    optionState.generalState.undo = m_config -> readBoolEntry( "undo", true );
    optionState.generalState.tabdiagrams = m_config -> readBoolEntry("tabdiagrams", false);
#if defined (WORK_ON_BUG_126262)
    optionState.generalState.newcodegen = m_config -> readBoolEntry("newcodegen", false);
#else
    optionState.generalState.newcodegen = false;
#endif
    optionState.generalState.angularlines = m_config->readBoolEntry("angularlines", false);
    optionState.generalState.autosave = m_config -> readBoolEntry( "autosave", true );
    optionState.generalState.time = m_config -> readNumEntry( "time", 0 ); //old autosavetime value kept for compatibility
    optionState.generalState.autosavetime = m_config -> readNumEntry( "autosavetime", 0 );
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
    optionState.generalState.autosavesuffix = m_config -> readEntry( "autosavesuffix", ".xmi" );

    optionState.generalState.logo = m_config -> readBoolEntry( "logo", true );
    optionState.generalState.loadlast = m_config -> readBoolEntry( "loadlast", true );

    optionState.generalState.diagram  = (Uml::Diagram_Type) m_config->readNumEntry("diagram", 1);
    m_config -> setGroup( "TipOfDay");

    optionState.generalState.tip = m_config -> readBoolEntry( "RunOnStart", true );

    m_config -> setGroup( "UI Options" );
    optionState.uiState.useFillColor = m_config -> readBoolEntry( "useFillColor", true );
    QColor defaultYellow = QColor( 255, 255, 192 );
    QColor red ( Qt::red );

    optionState.uiState.fillColor = m_config -> readColorEntry( "fillColor", &defaultYellow );
    optionState.uiState.lineColor = m_config -> readColorEntry( "lineColor", &red );
    optionState.uiState.lineWidth = m_config -> readNumEntry( "lineWidth", 0 );
    QFont font = ((QWidget *) this)->font() ;
    optionState.uiState.font = m_config -> readFontEntry("font", &font );

    m_config -> setGroup( "Class Options" );

    optionState.classState.showVisibility = m_config -> readBoolEntry("showVisibility", true);
    optionState.classState.showAtts = m_config -> readBoolEntry("showAtts", true);
    optionState.classState.showOps = m_config -> readBoolEntry("showOps", true);
    optionState.classState.showStereoType = m_config -> readBoolEntry("showStereoType", false);
    optionState.classState.showAttSig = m_config -> readBoolEntry("showAttSig", true);
    optionState.classState.showOpSig = m_config -> readBoolEntry("ShowOpSig", true);
    optionState.classState.showPackage = m_config -> readBoolEntry("showPackage", false);
    optionState.classState.defaultAttributeScope = (Uml::Visibility::Value) m_config -> readNumEntry("defaultAttributeScope", Uml::Visibility::Private);
    optionState.classState.defaultOperationScope = (Uml::Visibility::Value) m_config -> readNumEntry("defaultOperationScope", Uml::Visibility::Public);

    m_config -> setGroup( "Code Viewer Options" );

    QColor defaultWhite = QColor( "white" );
    QColor defaultBlack = QColor( "black" );
    QColor defaultPink = QColor( "pink" );
    QColor defaultGrey = QColor( "grey" );

    optionState.codeViewerState.height = m_config -> readNumEntry( "height", 40 );
    optionState.codeViewerState.width = m_config -> readNumEntry( "width", 80 );
    optionState.codeViewerState.font = m_config -> readFontEntry("font", &font );
    optionState.codeViewerState.showHiddenBlocks = m_config -> readBoolEntry( "showHiddenBlocks", false);
    optionState.codeViewerState.blocksAreHighlighted = m_config -> readBoolEntry( "blocksAreHighlighted", false);
    optionState.codeViewerState.selectedColor = m_config -> readColorEntry( "selectedColor", &defaultYellow );
    optionState.codeViewerState.paperColor = m_config -> readColorEntry( "paperColor", &defaultWhite);
    optionState.codeViewerState.fontColor = m_config -> readColorEntry( "fontColor", &defaultBlack);
    optionState.codeViewerState.editBlockColor = m_config -> readColorEntry( "editBlockColor", &defaultPink);
    optionState.codeViewerState.umlObjectColor = m_config -> readColorEntry( "umlObjectBlockColor", &defaultPink);
    optionState.codeViewerState.nonEditBlockColor = m_config -> readColorEntry( "nonEditBlockColor", &defaultGrey);
    optionState.codeViewerState.hiddenColor = m_config -> readColorEntry( "hiddenColor", &defaultGrey);

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

    if (m_policyext)
        m_policyext->setDefaults(m_config, false); // picks up language specific stuff
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

void UMLApp::setActiveLanguage(int menuId) {

    // only change the active language IF different from one we currently have
    if (!m_langSelect->isItemChecked(menuId))
    {
        uint index = 0;
        for(unsigned int i=0; i < m_langSelect->count(); i++) {
            int id = m_langSelect->idAt(i);
            m_langSelect->setItemChecked(id, false);  //uncheck everything
            if (id == menuId)
                index = i;
        }

        m_langSelect->setItemChecked(menuId,true);
        m_activeLanguage = (Uml::Programming_Language)index;

        // update the generator
        setGenerator(m_activeLanguage);
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

    setGenerator(Model_Utils::stringToProgLang(activeLanguage));
}

Uml::Programming_Language UMLApp::getActiveLanguage() {
    return m_activeLanguage;
}

bool UMLApp::activeLanguageIsCaseSensitive() {
    return (m_activeLanguage != Uml::pl_Pascal &&
            m_activeLanguage != Uml::pl_Ada &&
            m_activeLanguage != Uml::pl_SQL);
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
    QPopupMenu* menu = findMenu( menuBar(), QString("views") );
    if (!menu) {
        kWarning() << "view menu not found" << endl;
        return;
    }

    menu = findMenu( menu, QString("show_view") );
    if (!menu) {
        kWarning() << "show menu not found" << endl;
        return;
    }

    menu->clear();

    UMLViewList views = getDocument()->getViewIterator();
    for(UMLView *view = views.first(); view; view = views.next()) {
        menu->insertItem( view->getName(), view, SLOT( slotShowView() ) );
        view->fileLoaded();
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
    QStringList fileList = KFileDialog::getOpenFileNames(":import-classes", preselectedExtension,
                           this, i18n("Select Code to Import") );
    const QString& firstFile = fileList.first();
    ClassImport *classImporter = ClassImport::createImporterByFileExt(firstFile);
    classImporter->importFiles(fileList);
    delete classImporter;
    m_doc->setLoading(false);
    //Modification is set after the import is made, because the file was modified when adding the classes
    //Allowing undo of the whole class importing. I think it eats a lot of memory
    //m_doc->setModified(true);
    //Setting the modification, but without allowing undo
    m_doc->setModified(true, false);
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
        kWarning() << " trying to delete widgets when there is no current view (see bug 59774)" << endl;
    }
}

void UMLApp::slotDeleteDiagram() {
    m_doc->removeDiagram( getCurrentView()->getID() );
}

Uml::Programming_Language UMLApp::getDefaultLanguage() {
    m_config->setGroup("Code Generation");
    QString activeLanguage = m_config->readEntry("activeLanguage", "C++");
    return Model_Utils::stringToProgLang(activeLanguage);
}

void UMLApp::initGenerator() {
    if (m_codegen) {
        delete m_codegen;
        m_codegen = NULL;
    }
    Uml::Programming_Language defaultLanguage = getDefaultLanguage();
    setActiveLanguage(Model_Utils::progLangToString(defaultLanguage));
    if (m_codegen == NULL)
        setGenerator(defaultLanguage);
    updateLangSelectMenu(defaultLanguage);
}

void UMLApp::updateLangSelectMenu(Uml::Programming_Language activeLanguage) {
    m_langSelect->clear();
    m_langSelect->setCheckable(true);
    for (int i = 0; i < Uml::pl_Reserved; i++) {
        QString language = Model_Utils::progLangToString((Uml::Programming_Language) i);
        int id = m_langSelect->insertItem(language,this,SLOT(setActiveLanguage(int)));
        const bool isActiveLanguage = (activeLanguage == i);
        m_langSelect->setItemChecked(id, isActiveLanguage);
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
    Uml::Programming_Language defaultLanguage = getDefaultLanguage();
    if (m_codegen) {
        defaultLanguage = m_codegen->getLanguage();
        delete m_codegen;
        m_codegen = NULL;
    }
    setGenerator(defaultLanguage);
    slotUpdateViews();
}

QWidget* UMLApp::getMainViewWidget() {
    Settings::OptionState& optionState = Settings::getOptionState();
    if (optionState.generalState.tabdiagrams)
        return m_tabWidget;
    return m_viewStack;
}

void UMLApp::setCurrentView(UMLView* view) {
    m_view = view;
    if (m_viewStack == NULL) {
        kError() << "UMLApp::setCurrentView: m_viewStack is NULL" << endl;
        return;
    }
    if (view == NULL) {
        kDebug() << "UMLApp::setCurrentView: view is NULL" << endl;
        return;
    }
    if (m_viewStack->id(view) < 0)
        m_viewStack->addWidget(view);
    m_viewStack->raiseWidget(view);
    slotStatusMsg(view->getName());
    UMLListViewItem* lvitem = m_listView->findView(view);
    if (lvitem)
        m_listView->setCurrentItem(lvitem);
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

void UMLApp::slotAutolayout(){
#ifdef HAVE_DOT
/*
    QDialog* d = new AutolayoutDlg(getCurrentView());
    d->show();
 */
#endif
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
    return m_statusLabel->text();
}

//static pointer, holding the unique instance
UMLApp* UMLApp::s_instance;

#include "uml.moc"
