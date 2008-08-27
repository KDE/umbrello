/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
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
#include "kplayerslideraction.h"
#include "docwindow.h"
#include "optionstate.h"
#include "cmdlineexportallviewsevent.h"
#include "cmds.h"
#include "umbrellosettings.h"
// code generation
#include "codegenerator.h"
#include "codegenerationpolicy.h"
#include "codegenerators/codegenfactory.h"
#include "codegenerators/codegenpolicyext.h"
#include "codegenerators/simplecodegenerator.h"
// utils
#include "widget_utils.h"
#include "icon_utils.h"
#include "model_utils.h"
// dialogs
#include "classwizard.h"
#include "codegenwizard/codegenerationwizard.h"
#include "codeviewerdialog.h"
#include "diagramprintpage.h"
#include "importprojectdlg.h"
#include "settingsdlg.h"
#include "codeimport/classimport.h"
#include "refactoring/refactoringassistant.h"
// clipboard
#include "clipboard/umlclipboard.h"
#include "clipboard/umldragdata.h"
// docgenerators
#include "docgenerators/docbookgenerator.h"
#include "docgenerators/xhtmlgenerator.h"
#include "umlscene.h"

// kde includes
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <krecentfilesaction.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#ifdef Q_WS_WIN
#include <QtGui/QFileDialog>
#else
#include <kfiledialog.h>
#endif
#include <klocale.h>
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
#include <kdeprintdialog.h>

// qt includes
#include <QtCore/QTimer>
#include <QtCore/QRegExp>
#include <QtGui/QClipboard>
#include <QtGui/QSlider>
#include <QtGui/QToolButton>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenuItem>
#include <QtGui/QDockWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>

// Static pointer, holding the last created instance.
UMLApp* UMLApp::s_instance;

/// @todo This is an ugly _HACK_ to allow to compile umbrello.
/// All the menu stuff should be ported to KDE4 (using actions)
QMenu* UMLApp::findMenu(const QString& name)
{
    QWidget* widget = factory()->container(name, this);
    if (widget) {
        return dynamic_cast<QMenu*>(widget);
    }
    uDebug() << "factory()->container(" << name << ") returns NULL";
    return NULL;
}

UMLApp::UMLApp(QWidget* parent) : KXmlGuiWindow(parent)
{
    s_instance   = this;
    m_pDocWindow = NULL;
    m_config     = KGlobal::config();
    m_listView   = NULL;
    m_langSelect = NULL;
    m_zoomSelect = NULL;
    m_loading    = false;
    m_clipTimer  = 0;
    m_copyTimer  = 0;
    m_codegen    = 0;
    m_policyext  = 0;
    m_commoncodegenpolicy = 0;
    m_xhtmlGenerator = 0;
    m_activeLanguage = Uml::pl_Reserved;
    m_pUndoStack = new KUndoStack(this);
    m_doc = new UMLDoc();
    m_doc->init();
    m_hasBegunMacro = false;

    readOptionState();
    initActions();
    // call this here because the statusBar is shown/hidden by setupGUI()
    initStatusBar();
    // use the absolute path to your umbrelloui.rc file for testing purpose in setupGUI();
    setupGUI();
    initView();
    initClip();
    readOptions();

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

    m_refactoringAssist   = NULL;
    m_commoncodegenpolicy = new CodeGenerationPolicy();
    m_imageExporterAll    = new UMLViewImageExporterAll();
}

UMLApp::~UMLApp()
{
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

void UMLApp::setProgLangAction(Uml::Programming_Language pl, const QString& name, const QString& action)
{
    m_langAct[pl] = actionCollection()->addAction(action);
    m_langAct[pl]->setText(name);
    m_langAct[pl]->setCheckable(true);
}

void UMLApp::initActions()
{
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
    importClasses->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Import_Class));
    importClasses->setText(i18n("&Import Classes..."));
    connect(importClasses, SIGNAL( triggered( bool ) ), this, SLOT( slotImportClasses() ));

    importProject = actionCollection()->addAction("import_project");
    importProject->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Import_Project));
    importProject->setText(i18n("Import &Project..."));
    connect(importProject, SIGNAL( triggered( bool ) ), this, SLOT( slotImportProject() ));

    genWizard = actionCollection()->addAction("generation_wizard");
    genWizard->setText(i18n("&Code Generation Wizard..."));
    connect(genWizard, SIGNAL( triggered( bool ) ), this, SLOT( generationWizard() ));

    genAll = actionCollection()->addAction("generate_all");
    genAll->setText(i18n("&Generate All Code"));
    connect(genAll, SIGNAL( triggered( bool ) ), this, SLOT( generateAllCode() ));

    setProgLangAction(Uml::pl_ActionScript, "ActionScript",    "setLang_actionscript");
    setProgLangAction(Uml::pl_Ada,          "Ada",             "setLang_ada");
    setProgLangAction(Uml::pl_Cpp,          "C++",             "setLang_cpp");
    setProgLangAction(Uml::pl_CSharp,       "C#",              "setLang_csharp");
    setProgLangAction(Uml::pl_D,            "D",               "setLang_d");
    setProgLangAction(Uml::pl_IDL,          "IDL",             "setLang_idl");
    setProgLangAction(Uml::pl_Java,         "Java",            "setLang_java");
    setProgLangAction(Uml::pl_JavaScript,   "JavaScript",      "setLang_javascript");
    setProgLangAction(Uml::pl_MySQL,        "MySQL (SQL)",     "setLang_mysql");
    setProgLangAction(Uml::pl_Pascal,       "Pascal",          "setLang_pascal");
    setProgLangAction(Uml::pl_Perl,         "Perl",            "setLang_perl");
    setProgLangAction(Uml::pl_PHP,          "PHP",             "setLang_php");
    setProgLangAction(Uml::pl_PHP5,         "PHP5",            "setLang_php5");
    setProgLangAction(Uml::pl_PostgreSQL,   "PostgreSQL(SQL)", "setLang_postgresql");
    setProgLangAction(Uml::pl_Python,       "Python",          "setLang_python");
    setProgLangAction(Uml::pl_Ruby,         "Ruby",            "setLang_ruby");
    setProgLangAction(Uml::pl_SQL,          "SQL",             "setLang_sql");
    setProgLangAction(Uml::pl_Tcl,          "Tcl",             "setLang_tcl");
    setProgLangAction(Uml::pl_XMLSchema,    "XMLSchema",       "setLang_xmlschema");
    setProgLangAction(Uml::pl_Ocl,          "Ocl",             "setLang_ocl");

    connect(m_langAct[Uml::pl_ActionScript], SIGNAL(triggered()), this, SLOT(setLang_actionscript()));
    connect(m_langAct[Uml::pl_Ada],          SIGNAL(triggered()), this, SLOT(setLang_ada()));
    connect(m_langAct[Uml::pl_Cpp],          SIGNAL(triggered()), this, SLOT(setLang_cpp()));
    connect(m_langAct[Uml::pl_CSharp],       SIGNAL(triggered()), this, SLOT(setLang_csharp()));
    connect(m_langAct[Uml::pl_D],            SIGNAL(triggered()), this, SLOT(setLang_d()));
    connect(m_langAct[Uml::pl_IDL],          SIGNAL(triggered()), this, SLOT(setLang_idl()));
    connect(m_langAct[Uml::pl_Java],         SIGNAL(triggered()), this, SLOT(setLang_java()));
    connect(m_langAct[Uml::pl_JavaScript],   SIGNAL(triggered()), this, SLOT(setLang_javascript()));
    connect(m_langAct[Uml::pl_MySQL],        SIGNAL(triggered()), this, SLOT(setLang_mysql()));
    connect(m_langAct[Uml::pl_Pascal],       SIGNAL(triggered()), this, SLOT(setLang_pascal()));
    connect(m_langAct[Uml::pl_Perl],         SIGNAL(triggered()), this, SLOT(setLang_perl()));
    connect(m_langAct[Uml::pl_PHP],          SIGNAL(triggered()), this, SLOT(setLang_php()));
    connect(m_langAct[Uml::pl_PHP5],         SIGNAL(triggered()), this, SLOT(setLang_php5()));
    connect(m_langAct[Uml::pl_PostgreSQL],   SIGNAL(triggered()), this, SLOT(setLang_postgresql()));
    connect(m_langAct[Uml::pl_Python],       SIGNAL(triggered()), this, SLOT(setLang_python()));
    connect(m_langAct[Uml::pl_Ruby],         SIGNAL(triggered()), this, SLOT(setLang_ruby()));
    connect(m_langAct[Uml::pl_SQL],          SIGNAL(triggered()), this, SLOT(setLang_sql()));
    connect(m_langAct[Uml::pl_Tcl],          SIGNAL(triggered()), this, SLOT(setLang_tcl()));
    connect(m_langAct[Uml::pl_XMLSchema],    SIGNAL(triggered()), this, SLOT(setLang_xmlschema()));
    connect(m_langAct[Uml::pl_Ocl],          SIGNAL(triggered()), this, SLOT(setLang_ocl()));

    fileNew->setToolTip(i18n("Creates a new document"));
    fileOpen->setToolTip(i18n("Opens an existing document"));
    fileOpenRecent->setToolTip(i18n("Opens a recently used file"));
    fileSave->setToolTip(i18n("Saves the document"));
    fileSaveAs->setToolTip(i18n("Saves the document as..."));
    fileClose->setToolTip(i18n("Closes the document"));
    filePrint->setToolTip(i18n("Prints out the document"));
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
    connect(deleteSelectedWidget, SIGNAL( triggered( bool ) ), this, SLOT( slotDeleteSelectedWidget() ));

    // The different views
    newDiagram = actionCollection()->add<KActionMenu>( "new_view" );
    newDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_New) );
    newDiagram->setText( "new_view" );

    classDiagram = actionCollection()->addAction( "new_class_diagram" );
    classDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Class) );
    classDiagram->setText( i18n( "&Class Diagram..." ) );
    connect(classDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotClassDiagram() ));
    newDiagram->addAction(classDiagram);

    sequenceDiagram= actionCollection()->addAction( "new_sequence_diagram" );
    sequenceDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Sequence) );
    sequenceDiagram->setText( i18n( "&Sequence Diagram..." ) );
    connect(sequenceDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotSequenceDiagram() ));
    newDiagram->addAction(sequenceDiagram);

    collaborationDiagram = actionCollection()->addAction( "new_collaboration_diagram" );
    collaborationDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Collaboration) );
    collaborationDiagram->setText( i18n( "C&ollaboration Diagram..." ) );
    connect(collaborationDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotCollaborationDiagram() ));
    newDiagram->addAction(collaborationDiagram);

    useCaseDiagram= actionCollection()->addAction( "new_use_case_diagram" );
    useCaseDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Usecase) );
    useCaseDiagram->setText( i18n( "&Use Case Diagram..." ) );
    connect(useCaseDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotUseCaseDiagram() ));
    newDiagram->addAction(useCaseDiagram);

    stateDiagram= actionCollection()->addAction( "new_state_diagram" );
    stateDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_State) );
    stateDiagram->setText( i18n( "S&tate Diagram..." ) );
    connect(stateDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotStateDiagram() ));
    newDiagram->addAction(stateDiagram);

    activityDiagram= actionCollection()->addAction( "new_activity_diagram" );
    activityDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Activity) );
    activityDiagram->setText( i18n( "&Activity Diagram..." ) );
    connect(activityDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotActivityDiagram() ));
    newDiagram->addAction(activityDiagram);

    componentDiagram = actionCollection()->addAction( "new_component_diagram" );
    componentDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Component) );
    componentDiagram->setText( i18n("Co&mponent Diagram...") );
    connect(componentDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotComponentDiagram() ));
    newDiagram->addAction(componentDiagram);

    deploymentDiagram = actionCollection()->addAction( "new_deployment_diagram" );
    deploymentDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_Deployment) );
    deploymentDiagram->setText( i18n("&Deployment Diagram...") );
    connect(deploymentDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotDeploymentDiagram() ));
    newDiagram->addAction(deploymentDiagram);

    entityRelationshipDiagram = actionCollection()->addAction( "new_entityrelationship_diagram" );
    entityRelationshipDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Diagram_EntityRelationship) );
    entityRelationshipDiagram->setText( i18n("&Entity Relationship Diagram...") );
    connect(entityRelationshipDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotEntityRelationshipDiagram() ));
    newDiagram->addAction(entityRelationshipDiagram);

    viewClearDiagram = actionCollection()->addAction( "view_clear_diagram" );
    viewClearDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Clear) );
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
    deleteDiagram->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Delete) );
    deleteDiagram->setText( i18n("&Delete") );
    connect(deleteDiagram, SIGNAL( triggered( bool ) ), this, SLOT( slotDeleteDiagram() ));

    viewExportImage = actionCollection()->addAction( "view_export_image" );
    viewExportImage->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Export_Picture) );
    viewExportImage->setText( i18n("&Export as Picture...") );
    connect(viewExportImage, SIGNAL( triggered( bool ) ), this, SLOT( slotCurrentViewExportImage() ));

    viewExportImageAll = actionCollection()->addAction( "view_export_image_all" );
    viewExportImageAll->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Export_Picture) );
    viewExportImageAll->setText( i18n("Export &All Diagrams as Pictures...") );
    connect(viewExportImageAll, SIGNAL( triggered( bool ) ), this, SLOT( slotAllViewsExportImage() ));

    viewProperties = actionCollection()->addAction( "view_properties" );
    viewProperties->setIcon( Icon_Utils::SmallIcon(Icon_Utils::it_Properties) );
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
    zoomAction->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Zoom_Slider));
    zoomAction->setShortcuts(KShortcut(Qt::Key_F9));
    actionCollection()->addAction("popup_zoom", zoomAction);
    zoom100Action = actionCollection()->addAction("zoom100");
    zoom100Action->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Zoom_100));
    zoom100Action->setText(i18n("Z&oom to 100%"));
    connect(zoom100Action, SIGNAL( triggered( bool ) ), this, SLOT( slotZoom100() ));

    alignRight = actionCollection()->addAction( "align_right" );
    alignRight->setText(i18n("Align Right" ));
    alignRight->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Right) );
    connect(alignRight, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignRight() ));

    alignLeft = actionCollection()->addAction( "align_left" );
    alignLeft->setText(i18n("Align Left" ));
    alignLeft->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Left) );
    connect(alignLeft, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignLeft() ));

    alignTop = actionCollection()->addAction( "align_top" );
    alignTop->setText(i18n("Align Top" ));
    alignTop->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Top) );
    connect(alignTop, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignTop() ));

    alignBottom = actionCollection()->addAction( "align_bottom" );
    alignBottom->setText(i18n("Align Bottom" ));
    alignBottom->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_Bottom) );
    connect(alignBottom, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignBottom() ));

    alignVerticalMiddle = actionCollection()->addAction( "align_vertical_middle" );
    alignVerticalMiddle->setText(i18n("Align Vertical Middle" ));
    alignVerticalMiddle->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_VerticalMiddle) );
    connect(alignVerticalMiddle, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignVerticalMiddle() ));

    alignHorizontalMiddle = actionCollection()->addAction( "align_horizontal_middle" );
    alignHorizontalMiddle->setText(i18n("Align Horizontal Middle" ));
    alignHorizontalMiddle->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_HorizontalMiddle) );
    connect(alignHorizontalMiddle, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignHorizontalMiddle() ));

    alignVerticalDistribute = actionCollection()->addAction( "align_vertical_distribute" );
    alignVerticalDistribute->setText(i18n("Align Vertical Distribute" ));
    alignVerticalDistribute->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_VerticalDistribute) );
    connect(alignVerticalDistribute, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignVerticalDistribute() ));

    alignHorizontalDistribute = actionCollection()->addAction( "align_horizontal_distribute" );
    alignHorizontalDistribute->setText(i18n("Align Horizontal Distribute" ));
    alignHorizontalDistribute->setIcon(Icon_Utils::SmallIcon(Icon_Utils::it_Align_HorizontalDistribute) );
    connect(alignHorizontalDistribute, SIGNAL( triggered( bool ) ), this, SLOT( slotAlignHorizontalDistribute() ));

    QString moveTabLeftString = i18n("&Move Tab Left");
    QString moveTabRightString = i18n("&Move Tab Right");
    moveTabLeft = actionCollection()->addAction("move_tab_left");
    moveTabLeft->setIcon(Icon_Utils::SmallIcon(QApplication::layoutDirection() ? Icon_Utils::it_Go_Next : Icon_Utils::it_Go_Previous));
    moveTabLeft->setText(QApplication::layoutDirection() ? moveTabRightString : moveTabLeftString);
    moveTabLeft->setShortcut(QApplication::layoutDirection() ?
                 QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Right) : QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_Left));
    connect(moveTabLeft, SIGNAL( triggered( bool ) ), this, SLOT( slotMoveTabLeft() ));

    moveTabRight = actionCollection()->addAction("move_tab_right");
    moveTabRight->setIcon(Icon_Utils::SmallIcon(QApplication::layoutDirection() ? Icon_Utils::it_Go_Previous : Icon_Utils::it_Go_Next));
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

void UMLApp::slotZoomSliderMoved(int value)
{
    int zoom = (int)(value*0.01);
    getCurrentView()->setZoom(zoom*zoom);
}

void UMLApp::slotZoom100()
{
    setZoom(100);
}

void UMLApp::setZoom(int zoom)
{
    getCurrentView()->setZoom(zoom);
}

void UMLApp::slotSetZoom(QAction* action)
{
    QVariant var = action->data();
    if (var.canConvert<int>()) {
        getCurrentView()->setZoom(var.toInt());
    }
}

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

void UMLApp::setupZoomMenu()
{
    m_zoomSelect->clear();

    int currentZoom = getCurrentView()->currentZoom();

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

void UMLApp::initStatusBar()
{
    statusBar()->insertPermanentItem( i18nc("init status bar", "Ready"), 1 );
    connect(m_doc, SIGNAL( sigWriteToStatusBar(const QString &) ), this, SLOT( slotStatusMsg(const QString &) ));
}

void UMLApp::initView()
{
    setCaption(m_doc->url().fileName(),false);
    m_view = NULL;
    toolsbar = new WorkToolBar(this);
    toolsbar->setWindowTitle(i18n("Diagram Toolbar"));
    addToolBar(Qt::TopToolBarArea, toolsbar);

//     m_mainDock = new QDockWidget( this );
//     addDockWidget ( Qt::RightDockWidgetArea, m_mainDock );
    m_newSessionButton = NULL;
    m_diagramMenu = NULL;

    // Prepare Stacked Diagram Representation
    m_viewStack = new QStackedWidget(this);

    // Prepare Tabbed Diagram Representation
    m_tabWidget = new KTabWidget(this);
//    m_tabWidget->setAutomaticResizeTabs(true);
    m_tabWidget->setCloseButtonEnabled(true);
    connect(m_tabWidget, SIGNAL(closeRequest(QWidget*)), SLOT(slotDeleteDiagram(QWidget*)));

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
    m_listView = new UMLListView(m_listDock, "LISTVIEW");
    //m_listView->setSorting(-1);
    m_listView->setDocument(m_doc);
    m_listView->init();
    m_listDock->setWidget(m_listView);

    m_documentationDock = new QDockWidget( i18n("Doc&umentation"), this );
    m_documentationDock->setObjectName("DocumentationDock");

    addDockWidget(Qt::LeftDockWidgetArea, m_documentationDock);
    m_pDocWindow = new DocWindow(m_doc, m_documentationDock);
    m_pDocWindow->setObjectName("DOCWINDOW");
    m_documentationDock->setWidget(m_pDocWindow);

    m_doc->setupSignals();//make sure gets signal from list view

    m_cmdHistoryDock = new QDockWidget(i18n("Co&mmand history"), this);
    m_cmdHistoryDock->setObjectName("CmdHistoryDock");
    addDockWidget(Qt::LeftDockWidgetArea, m_cmdHistoryDock);
    // create cmd history view
    m_pQUndoView = new QUndoView(m_cmdHistoryDock);
    m_cmdHistoryDock->setWidget(m_pQUndoView);
    m_pQUndoView->setCleanIcon(Icon_Utils::SmallIcon(Icon_Utils::it_UndoView));
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

void UMLApp::openDocumentFile(const KUrl& url)
{
    slotStatusMsg(i18n("Opening file..."));

    m_doc->openDocument( url);
    fileOpenRecent->addUrl( url );
    resetStatusMsg();
    setCaption(m_doc->url().fileName(), false);
    enablePrint(true);
}

UMLDoc *UMLApp::getDocument() const
{
    return m_doc;
}

UMLListView* UMLApp::getListView()
{
    return m_listView;
}

void UMLApp::saveOptions()
{
    // The Toolbar settings will be handled by the respective classes ( KToolBar )
    KConfigGroup cg( m_config, "toolbar" );
    toolBar("mainToolBar")->saveSettings( cg );
    KConfigGroup workBarConfig(m_config, "workbar" );
    toolsbar->saveSettings(workBarConfig );
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
    }
    else {
        UmbrelloSettings::setLastFile(  m_doc->url().prettyUrl() );
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

    UmbrelloSettings::self()->writeConfig();
}

void UMLApp::readOptions()
{
    // bar status settings
    toolBar("mainToolBar")->applySettings(m_config->group( "toolbar") );
    // do config for work toolbar
    toolsbar->applySettings(m_config->group( "workbar") );
    fileOpenRecent->loadEntries(m_config->group( "Recent Files") );
    setImageMimeType( UmbrelloSettings::imageMimeType() );
    resize( UmbrelloSettings::geometry());
}

void UMLApp::saveProperties(KConfigGroup & cfg)
{
    uDebug() << "******************* commented out - UNUSED?";
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

void UMLApp::readProperties(const KConfigGroup & cfg)     //:TODO: applyMainWindowSettings(const KConfigGroup& config, bool force = false)
{
    uDebug() << "******************* commented out - UNUSED?";
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

bool UMLApp::queryClose()
{
    QByteArray dockConfig = saveState();
    UmbrelloSettings::setDockConfig( dockConfig );
    return m_doc->saveModified();
}

bool UMLApp::queryExit()
{
    saveOptions();
    m_doc->closeDocument();
    return true;
}

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

void UMLApp::slotFileOpen()
{
    slotStatusMsg(i18n("Opening file..."));
    m_loading = true;

    if (!m_doc->saveModified()) {
        // here saving wasn't successful
    }
    else {
#ifdef Q_WS_WIN
        KUrl url=QFileDialog::getOpenFileName(    //krazy:exclude=qclasses
            this,
            i18n("Open File"),
            "",
            i18n("All Supported Files (*.xmi *.mdl);;"
                 "XMI Files (*.xmi);;"
                 "Rose model files (*.mdl)"));
#else
        KUrl url=KFileDialog::getOpenUrl(KUrl(),
            i18n("*.xmi *.xmi.tgz *.xmi.tar.bz2 *.mdl|All Supported Files (*.xmi, *.xmi.tgz, *.xmi.tar.bz2, *.mdl)\n"
                 "*.xmi|Uncompressed XMI Files (*.xmi)\n"
                 "*.xmi.tgz|Gzip Compressed XMI Files (*.xmi.tgz)\n"
                 "*.xmi.tar.bz2|Bzip2 Compressed XMI Files (*.xmi.tar.bz2)\n"
                 "*.mdl|Rose model files"), this, i18n("Open File"));
#endif
        if (!url.isEmpty()) {
            if (m_doc->openDocument(url)) {
                fileOpenRecent->addUrl( url );
            }
            enablePrint(true);
            setCaption(m_doc->url().fileName(), false);
        }
    }
    slotUpdateViews();
    m_loading = false;
    resetStatusMsg();
}

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

bool UMLApp::slotFileSaveAs()
{
    slotStatusMsg(i18n("Saving file with a new filename..."));
    bool cont = true;
    KUrl url;
    QString ext;
    while (cont) {
#ifdef Q_WS_WIN
        url=QFileDialog::getSaveFileName(    //krazy:exclude=qclasses
            this,
            i18n("Save As"),
            "",
            i18n("XMI File (*.xmi);;All Files (*.*"));
#else
        url=KFileDialog::getSaveUrl(KUrl(), i18n("*.xmi|XMI File\n*.xmi.tgz|Gzip Compressed XMI File\n*.xmi.tar.bz2|Bzip2 Compressed XMI File\n*|All Files"), this, i18n("Save As"));
#endif
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
            QDir d = url.path( KUrl::RemoveTrailingSlash );

            if (QFile::exists(d.path())) {
                int want_save = KMessageBox::warningContinueCancel(this, i18n("The file %1 exists.\nDo you wish to overwrite it?", url.path()),
                                                                   i18n("Warning"), KGuiItem(i18n("Overwrite")));
                if (want_save == KMessageBox::Continue) {
                    cont = false;
                }
            }
            else {
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

void UMLApp::slotFileClose()
{
    slotStatusMsg(i18n("Closing file..."));
    slotFileNew();
}

void UMLApp::slotFilePrint()
{
    slotStatusMsg(i18n("Printing..."));

    QPrinter printer;
    printer.setFullPage(true);

    DiagramPrintPage * selectPage = new DiagramPrintPage(0, m_doc);
    QPrintDialog *printDialog =
                  KdePrint::createPrintDialog(&printer, QList<QWidget*>() << selectPage, this);
    printDialog->setWindowTitle(i18n("Print %1", m_doc->url().prettyUrl()));

    if (printDialog->exec()) {
        m_doc->print(&printer, selectPage);
    }
    delete printDialog;
    resetStatusMsg();
}

void UMLApp::slotFileQuit()
{
    slotStatusMsg(i18n("Exiting..."));
    if (m_doc->saveModified()) {
        QByteArray dockConfig = saveState();
        UmbrelloSettings::setDockConfig( dockConfig );
        saveOptions();
        qApp->quit();
    }
    resetStatusMsg();
}

void UMLApp::slotFileExportDocbook()
{
  DocbookGenerator* docbookGenerator = new DocbookGenerator;
  docbookGenerator->generateDocbookForProject();
  connect( docbookGenerator, SIGNAL( finished( bool ) ), docbookGenerator, SLOT( deleteLater() ) );
}

void UMLApp::slotFileExportXhtml()
{
  if (m_xhtmlGenerator != 0) {
    return;
  }
  m_xhtmlGenerator = new XhtmlGenerator();
  m_xhtmlGenerator->generateXhtmlForProject();
  connect(m_xhtmlGenerator, SIGNAL(finished(bool)), this, SLOT(slotXhtmlDocGenerationFinished(bool)));
}

void UMLApp::slotEditUndo()
{
    undo();
    resetStatusMsg();
}

void UMLApp::slotEditRedo()
{
    redo();
    resetStatusMsg();
}

void UMLApp::slotEditCut()
{
    slotStatusMsg(i18n("Cutting selection..."));
    //FIXME bug 59774 this fromview isn't very reliable.
    //when cutting diagrams it is set to true even though it shouldn't be
    bool fromview = (getCurrentView() && getCurrentView()->umlScene()->getSelectCount());
    if ( editCutCopy(fromview) ) {
        emit sigCutSuccessful();
        slotDeleteSelectedWidget();
        m_doc->setModified(true);
    }
    resetStatusMsg();
}

void UMLApp::slotEditCopy()
{
    slotStatusMsg(i18n("Copying selection to clipboard..."));
    bool  fromview = (getCurrentView() && getCurrentView()->umlScene()->getSelectCount());
    editCutCopy( fromview );
    resetStatusMsg();
    m_doc->setModified( true );
}

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

void UMLApp::slotStatusMsg(const QString &text)
{
    // change status message permanently
    statusBar()->changeItem( text, 1 );
}

void UMLApp::resetStatusMsg()
{
    statusBar()->changeItem( i18nc("reset status bar", "Ready."), 1 );
}

void UMLApp::slotClassDiagram()
{
    executeCommand(new Uml::CmdCreateClassDiag(m_doc));
}

void UMLApp::slotSequenceDiagram()
{
    executeCommand(new Uml::CmdCreateSeqDiag(m_doc));
}

void UMLApp::slotCollaborationDiagram()
{
    executeCommand(new Uml::CmdCreateCollaborationDiag(m_doc));
}

void UMLApp::slotUseCaseDiagram()
{
    executeCommand(new Uml::CmdCreateUseCaseDiag(m_doc));
}

void UMLApp::slotStateDiagram()
{
    executeCommand(new Uml::CmdCreateStateDiag(m_doc));
}

void UMLApp::slotActivityDiagram()
{
    executeCommand(new Uml::CmdCreateActivityDiag(m_doc));
}

void UMLApp::slotComponentDiagram()
{
    executeCommand(new Uml::CmdCreateComponentDiag(m_doc));
}

void UMLApp::slotDeploymentDiagram()
{
    executeCommand(new Uml::CmdCreateDeployDiag(m_doc));
}

void UMLApp::slotEntityRelationshipDiagram()
{
    executeCommand(new Uml::CmdCreateEntityRelationDiag(m_doc));
}

void UMLApp::slotAlignLeft()
{
 // [PORT]
    getCurrentView()->umlScene()->alignLeft();
}

void UMLApp::slotAlignRight()
{
 // [PORT]
    getCurrentView()->umlScene()->alignLeft();
}

void UMLApp::slotAlignTop()
{
 // [PORT]
    getCurrentView()->umlScene()->alignTop();
}

void UMLApp::slotAlignBottom()
{
 // [PORT]
    getCurrentView()->umlScene()->alignBottom();
}

void UMLApp::slotAlignVerticalMiddle()
{
 // [PORT]
    getCurrentView()->umlScene()->alignVerticalMiddle();
}

void UMLApp::slotAlignHorizontalMiddle()
{
 // [PORT]
    getCurrentView()->umlScene()->alignHorizontalMiddle();
}

void UMLApp::slotAlignVerticalDistribute()
{
 // [PORT]
    getCurrentView()->umlScene()->alignVerticalDistribute();
}

void UMLApp::slotAlignHorizontalDistribute()
{
 // [PORT]
    getCurrentView()->umlScene()->alignHorizontalDistribute();
}

WorkToolBar* UMLApp::getWorkToolBar()
{
    return toolsbar;
}

void UMLApp::setModified(bool modified)
{
    //fileSave->setEnabled(modified);

    //if anything else needs to be done on a mofication, put it here

    // printing should be possible whenever there is something to print
    if ( m_loading == false && modified == true && getCurrentView() )  {
        enablePrint(true);
    }

    if (m_loading == false)  {
        setCaption(m_doc->url().fileName(), modified); //add disk icon to taskbar if modified
    }
}

void UMLApp::enablePrint(bool enable)
{
    filePrint->setEnabled(enable);
}

void UMLApp::enableUndo(bool enable)
{
    editUndo->setEnabled(enable);
}

void UMLApp::enableRedo(bool enable)
{
    editRedo->setEnabled(enable);
}

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

void UMLApp::slotClipDataChanged()
{
    const QMimeData * data = QApplication::clipboard()->mimeData();

    // Pass the MimeSource to the Doc
    editPaste->setEnabled( data && canDecode(data) );
}

void UMLApp::slotCopyChanged()
{
    if (m_listView->getSelectedCount() || (getCurrentView() && getCurrentView()->umlScene()->getSelectCount())) {
        editCopy->setEnabled(true);
        editCut->setEnabled(true);
    }
    else {
        editCopy->setEnabled(false);
        editCut->setEnabled(false);
    }
}

void UMLApp::slotPrefs()
{
       Settings::OptionState& optionState = Settings::getOptionState();

       m_dlg = new SettingsDlg(this, &optionState);
       connect(m_dlg, SIGNAL( applyClicked() ), this, SLOT( slotApplyPrefs() ) );

       if ( m_dlg->exec() == QDialog::Accepted && m_dlg->getChangesApplied() ) {
           slotApplyPrefs();
       }

       delete m_dlg;
       m_dlg = NULL;
}

void UMLApp::slotApplyPrefs()
{
    if (m_dlg) {
        // we need this to sync both values
        Settings::OptionState& optionState = Settings::getOptionState();
        bool stackBrowsing = (m_layout->indexOf(m_tabWidget) != -1);
        bool tabBrowsing = optionState.generalState.tabdiagrams;
        uDebug() << "stackBrowsing=" << stackBrowsing << " / tabBrowsing=" << tabBrowsing;

        if (stackBrowsing != tabBrowsing) {
            // Diagram Representation Modified
            UMLView* currentView;
            UMLViewList views = m_doc->getViewIterator();

            if (tabBrowsing) {
                currentView = static_cast<UMLView*>(m_viewStack->currentWidget());
                m_layout->removeWidget(m_viewStack);
                m_viewStack->hide();

                foreach (UMLView *view, views) {
                    UMLScene *scene = view->umlScene();
                    m_viewStack->removeWidget(view);
                    m_tabWidget->addTab(view, scene->getName());
                    m_tabWidget->setTabIcon(m_tabWidget->indexOf(view), Icon_Utils::iconSet(scene->getType()));
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
        const QString plStr = m_dlg->getCodeGenerationLanguage();
        Uml::Programming_Language pl = Model_Utils::stringToProgLang(plStr);
        setGenerator(pl);
    }
}

bool UMLApp::getUndoEnabled()
{
    return editUndo->isEnabled();
}

bool UMLApp::getRedoEnabled()
{
    return editRedo->isEnabled();
}

bool UMLApp::getPasteState()
{
    return editPaste->isEnabled();
}

bool UMLApp::getCutCopyState()
{
    return editCopy->isEnabled();
}

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

void UMLApp::readOptionState()
{
    Settings::OptionState& optionState = Settings::getOptionState();

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

void UMLApp::viewCodeDocument(UMLClassifier* classifier)
{
    CodeGenerator * currentGen = getGenerator();
    if (currentGen && classifier) {
        if (!dynamic_cast<SimpleCodeGenerator*>(currentGen)) {
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
                KMessageBox::sorry(0, i18n("Cannot view code until you generate some first."), i18n("Cannot View Code"));
            }
        } else {
            KMessageBox::sorry(0, i18n("Cannot view code from simple code writer."), i18n("Cannot View Code"));
        }
    } else {
        uWarning() << "No CodeGenerator or UMLClassifier given!";
    }
}

void UMLApp::refactor(UMLClassifier* classifier)
{
    if (!m_refactoringAssist) {
        m_refactoringAssist = new RefactoringAssistant( m_doc, 0, 0, "refactoring_assistant" );
    }
    m_refactoringAssist->refactor(classifier);
    m_refactoringAssist->show();
}

CodeGenerationPolicy *UMLApp::getCommonPolicy()
{
    return m_commoncodegenpolicy;
}

void UMLApp::setPolicyExt(CodeGenPolicyExt *policy)
{
    m_policyext = policy;
}

CodeGenPolicyExt *UMLApp::getPolicyExt()
{
    return m_policyext;
}

CodeGenerator *UMLApp::setGenerator(Uml::Programming_Language pl)
{
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
        if (m_codegen->getLanguage() == pl) {
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

CodeGenerator* UMLApp::getGenerator()
{
    return m_codegen;
}

bool UMLApp::isSimpleCodeGeneratorActive()
{
    if (m_codegen && dynamic_cast<SimpleCodeGenerator*>(m_codegen)) {
        return true;
    }
    else {
        return false;
    }
}

void UMLApp::generateAllCode()
{
    if (m_codegen) {
        m_codegen->writeCodeToFile();
    }
}

void UMLApp::generationWizard()
{
    CodeGenerationWizard wizard(0 /*classList*/);
    wizard.exec();
}

void UMLApp::setLang_actionscript()
{
    setActiveLanguage(Uml::pl_ActionScript);
}

void UMLApp::setLang_ada()
{
    setActiveLanguage(Uml::pl_Ada);
}

void UMLApp::setLang_cpp()
{
    setActiveLanguage(Uml::pl_Cpp);
}

void UMLApp::setLang_csharp()
{
    setActiveLanguage(Uml::pl_CSharp);
}

void UMLApp::setLang_d()
{
    setActiveLanguage(Uml::pl_D);
}

void UMLApp::setLang_idl()
{
    setActiveLanguage(Uml::pl_IDL);
}

void UMLApp::setLang_java()
{
    setActiveLanguage(Uml::pl_Java);
}

void UMLApp::setLang_javascript()
{
    setActiveLanguage(Uml::pl_JavaScript);
}

void UMLApp::setLang_mysql()
{
    setActiveLanguage(Uml::pl_MySQL);
}

void UMLApp::setLang_pascal()
{
    setActiveLanguage(Uml::pl_Pascal);
}

void UMLApp::setLang_perl()
{
    setActiveLanguage(Uml::pl_Perl);
}

void UMLApp::setLang_php()
{
    setActiveLanguage(Uml::pl_PHP);
}

void UMLApp::setLang_php5()
{
    setActiveLanguage(Uml::pl_PHP5);
}

void UMLApp::setLang_postgresql()
{
    setActiveLanguage(Uml::pl_PostgreSQL);
}

void UMLApp::setLang_python()
{
    setActiveLanguage(Uml::pl_Python);
}

void UMLApp::setLang_ruby()
{
    setActiveLanguage(Uml::pl_Ruby);
}

void UMLApp::setLang_sql()
{
    setActiveLanguage(Uml::pl_SQL);
}

void UMLApp::setLang_tcl()
{
    setActiveLanguage(Uml::pl_Tcl);
}

void UMLApp::setLang_xmlschema()
{
    setActiveLanguage(Uml::pl_XMLSchema);
}

void UMLApp::setLang_ocl()
{
    setActiveLanguage(Uml::pl_Ocl);
}

void UMLApp::setActiveLanguage(Uml::Programming_Language pl)
{
    updateLangSelectMenu(pl);
    setGenerator(pl);
}

Uml::Programming_Language UMLApp::getActiveLanguage()
{
    return m_activeLanguage;
}

bool UMLApp::activeLanguageIsCaseSensitive()
{
    return (m_activeLanguage != Uml::pl_Pascal &&
            m_activeLanguage != Uml::pl_Ada &&
            m_activeLanguage != Uml::pl_SQL &&
            m_activeLanguage != Uml::pl_MySQL &&
            m_activeLanguage != Uml::pl_PostgreSQL);
}

QString UMLApp::activeLanguageScopeSeparator()
{
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

void UMLApp::slotCurrentViewClearDiagram()
{
 // [PORT]
    getCurrentView()->umlScene()->clearDiagram();
}

void UMLApp::slotCurrentViewToggleSnapToGrid()
{
 // [PORT]
    getCurrentView()->umlScene()->toggleSnapToGrid();
 // [PORT]
    viewSnapToGrid->setChecked( getCurrentView()->umlScene()->getSnapToGrid() );
}

void UMLApp::slotCurrentViewToggleShowGrid()
{
 // [PORT]
    getCurrentView()->umlScene()->toggleShowGrid();
 // [PORT]
    viewShowGrid->setChecked( getCurrentView()->umlScene()->getShowSnapGrid() );
}

void UMLApp::slotCurrentViewExportImage()
{
 // [PORT]
    getCurrentView()->umlScene()->getImageExporter()->exportView();
}

void UMLApp::slotAllViewsExportImage()
{
    m_imageExporterAll->exportAllViews();
}

void UMLApp::slotCurrentViewProperties()
{
 // [PORT]
    getCurrentView()->umlScene()->showPropDialog();
}

void UMLApp::setDiagramMenuItemsState(bool bState)
{
    viewClearDiagram->setEnabled( bState );
    viewSnapToGrid->setEnabled( bState );
    viewShowGrid->setEnabled( bState );
    deleteDiagram->setEnabled(bState);
    viewExportImage->setEnabled( bState );
    viewProperties->setEnabled( bState );
    filePrint->setEnabled( bState );
    if ( getCurrentView() ) {
 // [PORT]
        viewSnapToGrid->setChecked( getCurrentView()->umlScene()->getSnapToGrid() );
 // [PORT]
        viewShowGrid->setChecked( getCurrentView()->umlScene()->getShowSnapGrid() );
    }
}

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

    UMLViewList views = m_doc->getViewIterator();
    foreach (UMLView *view , views ) {
        menu->addAction(view->umlScene()->getName(), view, SLOT(slotShowView()));
        view->umlScene()->fileLoaded();
    }
}

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

void UMLApp::slotImportClasses()
{
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
#ifdef Q_WS_WIN
    QStringList extList = preselectedExtension.split('|');
    QStringList fileList = QFileDialog::getOpenFileNames(this, i18n("Select Code to Import"),    //krazy:exclude=qclasses
                                "", extList[1] );
#else
    QStringList fileList = KFileDialog::getOpenFileNames(KUrl(), preselectedExtension,
                           this, i18n("Select Code to Import") );
#endif
    importFiles(&fileList);
}

void UMLApp::slotImportProject()
{
    QStringList listFile;

    ImportProjectDlg importDlg(&listFile, m_codegen->getLanguage(), this);
    if (importDlg.exec() == KDialog::Accepted) {
        importFiles(&listFile);
    }
}

void UMLApp::slotClassWizard()
{
    ClassWizard dlg( m_doc );
    dlg.exec();
}

void UMLApp::slotAddDefaultDatatypes()
{
    m_doc->addDefaultDatatypes();
}

void UMLApp::slotCurrentViewChanged()
{
    UMLView *view = getCurrentView();
    if (view) {
        connect(view->umlScene(), SIGNAL( sigShowGridToggled(bool) ),
                this, SLOT( slotShowGridToggled(bool) ) );
        connect(view->umlScene(), SIGNAL( sigSnapToGridToggled(bool) ),
                this, SLOT( slotSnapToGridToggled(bool) ) );
    }
}

void UMLApp::slotSnapToGridToggled(bool gridOn)
{
    viewSnapToGrid->setChecked(gridOn);
}

void UMLApp::slotShowGridToggled(bool gridOn)
{
    viewShowGrid->setChecked(gridOn);
}

void UMLApp::slotSelectAll()
{
 // [PORT]
    getCurrentView()->umlScene()->selectAll();
}

void UMLApp::slotDeleteSelectedWidget()
{
    if ( getCurrentView() ) {
 // [PORT]
        getCurrentView()->umlScene()->deleteSelection();
    }
    else {
        uWarning() << " trying to delete widgets when there is no current view (see bug 59774)";
    }
}

void UMLApp::slotDeleteDiagram(QWidget* tab)
{
    if (tab == NULL) {  // called from menu action
 // [PORT]
        m_doc->removeDiagram( getCurrentView()->umlScene()->getID() );
    }
    else {  // clicked on close button
        UMLView* view = (UMLView*)tab;
        if (view != getCurrentView()) {
            setCurrentView(view);
        }
        m_doc->removeDiagram( view->umlScene()->getID() );
    }
}

Uml::Programming_Language UMLApp::getDefaultLanguage()
{
    Settings::OptionState& optionState = Settings::getOptionState();
    return optionState.generalState.defaultLanguage;
}

void UMLApp::initGenerator()
{
    if (m_codegen) {
        delete m_codegen;
        m_codegen = NULL;
    }
    Uml::Programming_Language defaultLanguage = getDefaultLanguage();
    setActiveLanguage(defaultLanguage);
}

void UMLApp::updateLangSelectMenu(Uml::Programming_Language activeLanguage)
{
    //m_langSelect->clear();
    for (int i = 0; i < Uml::pl_Reserved; i++) {
        m_langAct[i]->setChecked(i == activeLanguage);
    }
}

void UMLApp::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Shift:
        //toolsbar->setOldTool();
        e->accept();
        break;

    default:
        e->ignore();
    }
}

void UMLApp::customEvent(QEvent* e)
{
    if (e->type() == CmdLineExportAllViewsEvent::getType()) {
        CmdLineExportAllViewsEvent* exportAllViewsEvent = static_cast<CmdLineExportAllViewsEvent*>(e);
        exportAllViewsEvent->exportAllViews();
    }
}

//TODO Move this to UMLWidgetController?
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

void UMLApp::keyReleaseEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Backspace:
        if (!m_pDocWindow->isTyping()) {
            toolsbar->setOldTool();
        }
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

void UMLApp::newDocument()
{
    m_doc->newDocument();
    slotUpdateViews();
}

QWidget* UMLApp::getMainViewWidget()
{
    Settings::OptionState& optionState = Settings::getOptionState();
    if ( optionState.generalState.tabdiagrams ) {
        return m_tabWidget;
    }
    else {
        return m_viewStack;
    }
}

void UMLApp::setCurrentView(UMLView* view)
{
    m_view = view;
    if (view == NULL) {
        uDebug() << "view is NULL";
        return;
    }

    Settings::OptionState optionState = Settings::getOptionState();
    if (optionState.generalState.tabdiagrams) {
        if ( m_tabWidget->indexOf(view) < 0 ) {
            m_tabWidget->addTab(view, view->umlScene()->getName());
            m_tabWidget->setTabIcon(m_tabWidget->indexOf(view), Icon_Utils::iconSet(view->umlScene()->getType()));
        }
        m_tabWidget->setCurrentIndex(m_tabWidget->indexOf(view));
    }
    else {
        if (m_viewStack->indexOf(view) < 0) {
            m_viewStack->addWidget(view);
        }
        m_viewStack->setCurrentWidget(view);
        view->show();
    }
    qApp->processEvents();
    slotStatusMsg(view->umlScene()->getName());
    UMLListViewItem* lvitem = m_listView->findView(view);
    if (lvitem) {
        m_listView->setCurrentItem(lvitem);
    }
}

UMLView* UMLApp::getCurrentView()
{
    return m_view;
}

void UMLApp::slotTabChanged(QWidget* tab)
{
    UMLView* view = ( UMLView* )tab;
    if (view) {
        m_doc->changeCurrentView( view->umlScene()->getID() );
    }
}

void UMLApp::slotChangeTabLeft()
{
    //uDebug() << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    if (m_tabWidget) {
        m_tabWidget->setCurrentIndex( m_tabWidget->currentIndex() - 1 );
        return;
    }
    UMLViewList views = m_doc->getViewIterator();
    UMLView *currView = m_view;
    int viewIndex = 0;
    if ( (viewIndex = views.indexOf(currView)) < 0) {
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

void UMLApp::slotChangeTabRight()
{
    //uDebug() << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
    if (m_tabWidget) {
        m_tabWidget->setCurrentIndex( m_tabWidget->currentIndex() + 1 );
        return;
    }
    UMLViewList views = m_doc->getViewIterator();
    UMLView *currView = m_view;
    int viewIndex = 0;
    if (( viewIndex = views.indexOf(currView) ) < 0) {
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
    uDebug() << out;
}
*/

void UMLApp::slotMoveTabLeft()
{
    //uDebug() << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
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

void UMLApp::slotMoveTabRight()
{
    //uDebug() << "currentIndex = " << m_tabWidget->currentIndex() << " of " << m_tabWidget->count();
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

void UMLApp::slotXhtmlDocGenerationFinished(bool status)
{
  if ( !status ) {
      m_doc->writeToStatusBar( i18n( "XHTML Generation failed ." ) );
  }

  delete m_xhtmlGenerator;
  m_xhtmlGenerator = 0;
}

KTabWidget* UMLApp::tabWidget()
{
    return m_tabWidget;
}

QString UMLApp::getStatusBarMsg()
{
    return statusBar()->itemText(1);
}

void UMLApp::clearUndoStack()
{
    m_pUndoStack->clear();
}

void UMLApp::undo()
{
    uDebug() << m_pUndoStack->undoText() << " [" << m_pUndoStack->count() << "]";
    m_pUndoStack->undo();

    if (m_pUndoStack->canUndo()) {
        UMLApp::app()->enableUndo(true);
    }
    else {
        UMLApp::app()->enableUndo(false);
    }

    UMLApp::app()->enableRedo(true);
}

void UMLApp::redo()
{
    uDebug() << m_pUndoStack->redoText() << " [" << m_pUndoStack->count() << "]";
    m_pUndoStack->redo();

    if (m_pUndoStack->canRedo()) {
        UMLApp::app()->enableRedo(true);
    }
    else {
        UMLApp::app()->enableRedo(false);
    }

    UMLApp::app()->enableUndo(true);
}

void UMLApp::executeCommand(QUndoCommand* cmd)
{
    if (cmd != NULL) {
        m_pUndoStack->push(cmd);
    }
    uDebug() << cmd->text() << " [" << m_pUndoStack->count() << "]";

    UMLApp::app()->enableUndo(true);
}

void UMLApp::BeginMacro( const QString & text )
{
    if (m_hasBegunMacro) {
        return;
    }
    m_hasBegunMacro = true;

    m_pUndoStack->beginMacro(text);
}

void UMLApp::EndMacro()
{
    if (m_hasBegunMacro) {
        m_pUndoStack->endMacro();
    }
    m_hasBegunMacro = false;
}

#include "uml.moc"
