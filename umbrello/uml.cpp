/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "uml.h"

#include "infowidget.h"
#include "classimport.h"
#include "docwindow.h"
#include "codegenerator.h"
#include "codegenerationpolicy.h"

#include "umldoc.h"
#include "umllistview.h"
#include "umlviewlist.h"
#include "worktoolbar.h"

#include "clipboard/umlclipboard.h"
#include "clipboard/umldrag.h"
#include "dialogs/classwizard.h"
#include "dialogs/codegenerationwizard.h"
#include "dialogs/codeviewerdialog.h"
#include "dialogs/configcodegenerators.h"
#include "dialogs/diagramprintpage.h"
#include "dialogs/selectlanguagesdlg.h"
#include "diagram/diagramview.h"
#include "refactoring/refactoringassistant.h"
#include "codegenerators/simplecodegenerator.h"

#include "kplayerslideraction.h"

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

#include <qclipboard.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qwidgetstack.h>
#include <qslider.h>

#include "configurable.h"

////FIXME - remove when dialog linking problems are solved
#include "interface.h"
#include "package.h"
#include "class.h"
#include "dialogs/umbrellodialog.h"
#include "dialogs/classpropertiespage.h"
#include "dialogs/classattributespage.h"
#include "dialogs/classifieroperationspage.h"
#include "dialogs/umlobjectassociationspage.h"
#include "dialogs/classtemplatespage.h"
#include "dialogs/interfacepropertiespage.h"
#include "dialogs/packagepropertiespage.h"
///////////////////////////////////


using Umbrello::Diagram;
using Umbrello::RefactoringAssistant;


UMLApp::UMLApp(QWidget* , const char* name):KDockMainWindow(0, name) {
	s_instance = this;
	m_pDocWindow = 0;
	config=kapp->config();
	listView = 0;
	ldict.setAutoDelete(true);
	langSelect = 0L;
	zoomSelect = 0L;
	loading = false;
	m_clipTimer = 0;
	m_copyTimer = 0;
	m_defaultcodegenerationpolicy = 0;

	///////////////////////////////////////////////////////////////////
	// call inits to invoke all other construction parts
	readOptionState();
	initActions();
	initDocument();
	initView();
	initClip();
	initStatusBar();
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
	langSelect = findMenu( menu, QString("active_lang_menu") );

	//in case langSelect hasnt been initialized we create the Popup menu.
	//it will be hidden, but at least we wont crash if someone takes the entry away from the ui.rc file
	if (langSelect == 0L) {
		langSelect = new QPopupMenu(this);
	}

	menu = findMenu( menuBar(), QString("views") );
	zoomSelect = findMenu( menu, QString("zoom_menu") );

	//in case zoomSelect hasnt been initialized we create the Popup menu.
	//it will be hidden, but at least we wont crash if some one takes the entry away from the ui.rc file
	if (zoomSelect == 0L) {
		zoomSelect = new QPopupMenu(this);
	}

	//connect zoomSelect menu
	zoomSelect->setCheckable(true);
	connect(zoomSelect,SIGNAL(aboutToShow()),this,SLOT(setupZoomMenu()));
	connect(zoomSelect,SIGNAL(activated(int)),this,SLOT(setZoom(int)));

	m_refactoringAssist = 0L;

	m_defaultcodegenerationpolicy = new CodeGenerationPolicy(this,config);

////FIXME - remove when dialog linking problems are solved
	UMLClass *dummyc = new UMLClass(doc, "dummy", 9999);
	UMLInterface *dummyi = new UMLInterface(doc, "dummy", 9999);
	UMLPackage *dummyp = new UMLPackage(doc, "dummy", 9999);

		delete new UmbrelloDialog(this);
		delete new Umbrello::ClassPropertiesPage(dummyc);
		delete new Umbrello::ClassPropertiesPage(dummyc);
		delete new Umbrello::ClassAttributesPage(dummyc,doc,(QWidget*)0L);
		delete new Umbrello::ClassifierOperationsPage(dummyc,doc,(QWidget*)0L);
		delete new Umbrello::UMLObjectAssociationsPage(dummyc,(QWidget*)0L);
		delete new Umbrello::ClassTemplatesPage(dummyc,doc,(QWidget*)0L);
		delete new Umbrello::InterfacePropertiesPage(dummyi);
		delete new Umbrello::PackagePropertiesPage(dummyp);
	delete dummyc;
	delete dummyi;
	delete dummyp;
////////////////////////////////////////

}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
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
#if KDE_VERSION >= 0x030190
	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);
#else
	viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
	viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
#endif
	selectAll = KStdAction::selectAll(this,  SLOT( slotSelectAll() ), actionCollection());

	classWizard = new KAction(i18n("&New Class Wizard..."),0,this,SLOT(slotClassWizard()),
	                          actionCollection(),"class_wizard");

	preferences = KStdAction::preferences(this,  SLOT( slotPrefs() ), actionCollection(), "umbrello_configure");

	genWizard = new KAction(i18n("&Code Generation Wizard..."),0,this,SLOT(generationWizard()),
	                        actionCollection(),"generation_wizard");
	genAll = new KAction(i18n("&Generate All Code"),0,this,SLOT(generateAllCode()),
	                     actionCollection(),"generate_all");

	importClasses = new KAction(i18n("&Import Classes..."), SmallIconSet("source_cpp"), 0,
				    this,SLOT(slotImportClasses()), actionCollection(),"import_class");

	confLanguages = new KAction(i18n("&Add/Remove Generation Languages..."),0,this,
	                            SLOT(configureLanguages()),actionCollection(),"configure_languages");

	fileNew->setStatusText(i18n("Creates a new document"));
	fileOpen->setStatusText(i18n("Opens an existing document"));
	fileOpenRecent->setStatusText(i18n("Opens a recently used file"));
	fileSave->setStatusText(i18n("Saves the actual document"));
	fileSaveAs->setStatusText(i18n("Saves the actual document as..."));
	fileClose->setStatusText(i18n("Closes the actual document"));
	filePrint ->setStatusText(i18n("Prints out the actual document"));
	fileQuit->setStatusText(i18n("Quits the application"));
	editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
	editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
	editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
#if KDE_VERSION < 0x030190
	viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
	viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));
#endif
	preferences->setStatusText( i18n( "Set the default program preferences") );

	deleteSelectedWidget = new KAction( i18n("&Delete Selected"),
					    SmallIconSet("editdelete"),
					    KShortcut(Key_Delete), this,
					    SLOT( slotDeleteSelectedWidget() ), actionCollection(),
					    "delete_selected" );

	// The different views
        newDiagram = new KActionMenu(0, SmallIconSet("filenew"), actionCollection(), "new_view");
	classDiagram = new KAction( i18n( "&Class Diagram" ), SmallIconSet("folder_green"), 0,
	                            this, SLOT( slotClassDiagram() ), actionCollection(), "new_class_diagram" );
	UclassDiagram = new KAction (i18n("Class Diagram (NewDiagram)"),SmallIconSet("folder_green"), 0,
					this,SLOT(UcreateDiagram()),actionCollection(), "umbrello::new_class_diagram");

	sequenceDiagram= new KAction( i18n( "&Sequence Diagram" ), SmallIconSet("folder_green"), 0,
	                              this, SLOT( slotSequenceDiagram() ), actionCollection(), "new_sequence_diagram" );
	UsequenceDiagram= new KAction( i18n( "&Sequence Diagram(NEW)" ), SmallIconSet("folder_green"), 0,
	                              this, SLOT( UcreateDiagram() ), actionCollection(), "umbrello::new_sequence_diagram" );

	collaborationDiagram = new KAction( i18n( "C&ollaboration Diagram" ), SmallIconSet("folder_green"), 0,
	                                    this, SLOT( slotCollaborationDiagram() ), actionCollection(), "new_collaboration_diagram" );

	UcollaborationDiagram = new KAction( i18n( "Collaboration &Diagram(NEW)" ), SmallIconSet("folder_green"), 0,
	                                    this, SLOT( UcreateDiagram() ), actionCollection(), "umbrello::new_collaboration_diagram" );

	useCaseDiagram= new KAction( i18n( "&Use Case Diagram" ), SmallIconSet("folder_grey"), 0,
	                             this, SLOT( slotUseCaseDiagram() ), actionCollection(), "new_use_case_diagram" );

	UuseCaseDiagram= new KAction( i18n( "&Use Case Diagram(NEW)" ), SmallIconSet("folder_grey"), 0,
	                             this, SLOT( UcreateDiagram() ), actionCollection(), "umbrello::new_use_case_diagram" );

	stateDiagram= new KAction( i18n( "S&tate Diagram" ), SmallIconSet("folder_green"), 0,
	                           this, SLOT( slotStateDiagram() ), actionCollection(), "new_state_diagram" );

	UstateDiagram= new KAction( i18n( "S&tate Diagram(NEW)" ), SmallIconSet("folder_green"), 0,
	                           this, SLOT( UcreateDiagram() ), actionCollection(), "umbrello::new_state_diagram" );
	activityDiagram= new KAction( i18n( "&Activity Diagram" ), SmallIconSet("folder_green"), 0,
	                              this, SLOT( slotActivityDiagram() ), actionCollection(), "new_activity_diagram" );

	componentDiagram = new KAction( i18n("Co&mponent Diagram"), SmallIconSet("folder_red"), 0,
					this, SLOT( slotComponentDiagram() ), actionCollection(),
					"new_component_diagram" );

	deploymentDiagram = new KAction( i18n("&Deployment Diagram"), SmallIconSet("folder_violet"), 0,
					this, SLOT( slotDeploymentDiagram() ), actionCollection(),
					"new_deployment_diagram" );

	viewClearDiagram = new KAction(i18n("&Clear Diagram"), SmallIconSet("editclear"), 0,
	                        this, SLOT( slotCurrentViewClearDiagram() ), actionCollection(), "view_clear_diagram");
	viewSnapToGrid = new KToggleAction(i18n("&Snap to Grid"), 0,
	                        this, SLOT( slotCurrentViewToggleSnapToGrid() ), actionCollection(), "view_snap_to_grid");
	viewShowGrid = new KToggleAction(i18n("S&how Grid"), 0,
	                        this, SLOT( slotCurrentViewToggleShowGrid() ), actionCollection(), "view_show_grid");
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

	zoomAction = new KPlayerPopupSliderAction(i18n("&Volume Popup"), "viewmag", Key_F9,
						  this, SLOT(slotZoomSliderMoved(int)),
						  actionCollection(), "popup_zoom");
	zoom100Action = new KAction(i18n( "&Zoom to 100%" ), "viewmag1", 0,
	                            this, SLOT( slotZoom100() ), actionCollection(),
				    "zoom100");

	// use the absolute path to your umbrelloui.rc file for testing purpose in createGUI();
	createGUI();

	QPopupMenu* menu = findMenu( menuBar(), QString("settings") );
	menu->insertItem(i18n("Windows"), dockHideShowMenu(), -1, 0);


}
//////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotZoomSliderMoved(int value) {
	int zoom = (int)(value*0.01);
	doc->getCurrentView()->setZoom(zoom*zoom);
}

void UMLApp::slotZoom100()  {
	setZoom(100);
}

void UMLApp::setZoom(int zoom) {
	doc->getCurrentView()->setZoom(zoom);
}

void UMLApp::setupZoomMenu() {
	zoomSelect->clear();

	//IMPORTANT: The ID's must match the zoom value (text)
	zoomSelect->insertItem(i18n(" &33 %"),33);
	zoomSelect->insertItem(i18n(" &50 %"),50);
	zoomSelect->insertItem(i18n(" &75 %"),75);
	zoomSelect->insertItem(i18n("&100 %"),100);
	zoomSelect->insertItem(i18n("1&50 %"),150);
	zoomSelect->insertItem(i18n("&200 %"),200);
	zoomSelect->insertItem(i18n("&300 %"),300);


	int zoom = doc->getCurrentView()->currentZoom();
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
			zoomSelect->insertSeparator();
			zoomSelect->insertItem(QString::number(zoom)+" %",zoom);
	}
	zoomSelect->setItemChecked(zoom, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::initStatusBar() {
	statProg = new QProgressBar(statusBar(),"Progressbar");
	statProg->setFixedWidth( 100 );             // arbitrary width
	statProg->setCenterIndicator(true);
	statProg->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	statProg->setMargin( 0 );
	statProg->setLineWidth(0);
	statProg->setBackgroundMode( QWidget::PaletteBackground );
	statProg->setFixedHeight( statProg->sizeHint().height() - 8 );

	m_statusLabel = new KStatusBarLabel( i18n("Ready."), 0, statusBar() );
	m_statusLabel->setFixedHeight( m_statusLabel->sizeHint().height() );

	m_statusLabel->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	m_statusLabel->setMargin( 0 );
	m_statusLabel->setLineWidth(0);

	statusBar()->addWidget( m_statusLabel, 1, false );
	statusBar()->addWidget(statProg, 0,  true);

	m_statusLabel->setAlignment(AlignLeft|AlignVCenter);

	connect(doc,SIGNAL(sigSetStatusbarProgressSteps(int)),statProg,SLOT(setTotalSteps(int)));

	connect(doc,SIGNAL(sigSetStatusbarProgress(int)),statProg,SLOT(setProgress(int)));
	connect(doc,SIGNAL(sigResetStatusbarProgress()),statProg,SLOT(reset()));

	connect(listView,SIGNAL(sigSetStatusbarProgressSteps(int)),statProg,SLOT(setTotalSteps(int)));
	connect(listView,SIGNAL(sigSetStatusbarProgress(int)),statProg,SLOT(setProgress(int)));
	connect(listView,SIGNAL(sigResetStatusbarProgress()),statProg,SLOT(reset()));
	connect(listView,SIGNAL(diagramSelected(int)),this,SLOT(UdiagramSelected(int)));
	//FIXME change name to raiseDiagram

	connect(doc, SIGNAL( sigWriteToStatusBar(const QString &) ), this, SLOT( slotStatusMsg(const QString &) ));
	statusBar()->show(); //needs to be forced to show when on first ever startup for some reason
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::initDocument() {
	doc = new UMLDoc(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::initView() {
	setCaption(doc->URL().fileName(),false);

	toolsbar = new WorkToolBar(this,"WORKTOOLBAR");
	addToolBar(toolsbar,Qt::DockRight,true);

	m_mainDock = createDockWidget("maindock", 0L, 0L, "main dock");
	viewStack = new QWidgetStack(m_mainDock, "viewstack");

	m_mainDock->setWidget(viewStack);
	m_mainDock->setDockSite(KDockWidget::DockCorner);
	m_mainDock->setEnableDocking(KDockWidget::DockNone);
	setView(m_mainDock);
	setMainDockWidget(m_mainDock);

	m_listDock = createDockWidget( "Model", 0L, 0L, i18n("Tree View") );
	listView = new UMLListView(m_listDock ,"LISTVIEW");
	m_listDock->setWidget(listView);
	m_listDock->setDockSite(KDockWidget::DockCorner);
	m_listDock->manualDock(m_mainDock, KDockWidget::DockLeft, 20);

	m_documentationDock = createDockWidget( "Documentation", 0L, 0L, i18n("Documentation") );
	m_pDocWindow = new DocWindow(doc, m_documentationDock, "DOCWINDOW");
	m_documentationDock->setWidget(m_pDocWindow);
	m_documentationDock->setDockSite(KDockWidget::DockCorner);
	m_documentationDock->manualDock(m_listDock, KDockWidget::DockBottom, 80);

	listView->setDocument(doc);
	doc->setupListView(listView);//make sure has a link to list view and add info widget
	doc->setupSignals();//make sure gets signal from list view

	readDockConfig(); //reposition all the DockWindows to their saved positions
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::openDocumentFile(const KURL& url) {
	slotStatusMsg(i18n("Opening file..."));

	doc->openDocument( url);
	fileOpenRecent->addURL( url );
	slotStatusMsg(i18n("Ready."));
	setCaption(doc->URL().fileName(), false);
	enablePrint(true);

	// restore any saved code generators to memory.
	// This is important because we need to insure that
	// even if a code generator isnt worked with, it will
	// remain in sync with the UML diagram(s) should they change.
	initSavedCodeGenerators();

}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLDoc *UMLApp::getDocument() const {
	return doc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void UMLApp::saveOptions() {
	toolBar("mainToolBar")->saveSettings(config, "toolbar");
	toolsbar->saveSettings(config, "workbar");
	fileOpenRecent->saveEntries(config,"Recent Files");
	config->setGroup( "General Options" );
	config->writeEntry( "Geometry", size() );

	config->writeEntry( "autosave", optionState.generalState.autosave );
	config->writeEntry( "time", optionState.generalState.time );
	config->writeEntry( "logo", optionState.generalState.logo );
	config->writeEntry( "loadlast", optionState.generalState.loadlast );

	config->writeEntry( "diagram", optionState.generalState.diagram );
	if( doc->URL().fileName() == i18n( "Untitled" ) ) {
		config -> writeEntry( "lastFile", "" );
	} else {
#if KDE_IS_VERSION(3,1,3)
		config -> writePathEntry( "lastFile", doc -> URL().prettyURL() );
#else
		config -> writeEntry( "lastFile", doc -> URL().prettyURL() );
#endif
	}
	config->writeEntry( "imageMimetype", getImageMimetype() );

	config->setGroup( "TipOfDay");
	optionState.generalState.tip = config -> readBoolEntry( "RunOnStart", true );
	config->writeEntry( "RunOnStart", optionState.generalState.tip );

	config->setGroup( "UI Options" );
	config->writeEntry( "useFillColor", optionState.uiState.useFillColor );
	config->writeEntry( "fillColor", optionState.uiState.fillColor );
	config->writeEntry( "lineColor", optionState.uiState.lineColor );
	config->writeEntry( "showDocWindow", m_documentationDock->isVisible() );
	config->writeEntry( "font", optionState.uiState.font );

	config->setGroup( "Class Options" );
	config->writeEntry( "showVisibility", optionState.classState.showScope );
	config->writeEntry( "showAtts", optionState.classState.showAtts);
	config->writeEntry( "showOps", optionState.classState.showOps );
	config->writeEntry( "showStereoType", optionState.classState.showStereoType );
	config->writeEntry( "showAttSig", optionState.classState.showAttSig );
	config->writeEntry( "ShowOpSig", optionState.classState.showOpSig );
	config->writeEntry( "showPackage", optionState.classState.showPackage );

	config -> setGroup( "Code Viewer Options" );
	config->writeEntry( "height", optionState.codeViewerState.height );
	config->writeEntry( "width", optionState.codeViewerState.width);
	config->writeEntry( "font", optionState.codeViewerState.font);
	config->writeEntry( "fontColor", optionState.codeViewerState.fontColor);
	config->writeEntry( "paperColor", optionState.codeViewerState.paperColor);
	config->writeEntry( "selectedColor", optionState.codeViewerState.selectedColor);
	config->writeEntry( "editBlockColor", optionState.codeViewerState.editBlockColor);
	config->writeEntry( "nonEditBlockColor", optionState.codeViewerState.nonEditBlockColor);
	config->writeEntry( "umlObjectBlockColor", optionState.codeViewerState.umlObjectColor);
	config->writeEntry( "blocksAreHighlighted", optionState.codeViewerState.blocksAreHighlighted);
	config->writeEntry( "showHiddenBlocks", optionState.codeViewerState.showHiddenBlocks);
	config->writeEntry( "hiddenColor", optionState.codeViewerState.hiddenColor);

	// merge current defaults into the default policy, just in case they
	// are out of sync (yes, yes, we could have a callback do this).
	CodeGenerator* gen = getGenerator(false);
	if(gen && gen->getPolicy())
	      m_defaultcodegenerationpolicy->setDefaults(gen->getPolicy());

	// write the config for each language-specific code gen policies
	QDictIterator<GeneratorInfo> it( ldict );
	for(it.toFirst() ; it.current(); ++it )
	{
		CodeGenerator * gen = doc->findCodeGeneratorByLanguage(it.current()->language);
		if (gen)
			gen->getPolicy()->writeConfig(config);
	}

	// now write the basic defaults to the config file
	m_defaultcodegenerationpolicy->writeConfig(config);

	// next, we record the activeLanguage in the Code Generation Group
	config->setGroup("Code Generation");
	config->writeEntry("activeLanguage",activeLanguage);

	//now, we save library information
	QStringList libsknown;
	QDict<QStringList> llist;
	llist.setAutoDelete(true);
//	QDictIterator<GeneratorInfo> it( ldict );
	QStringList templ;
	for(it.toFirst() ; it.current(); ++it ) {
		if(!libsknown.contains(it.current()->library))
			libsknown.append(it.current()->library);
		if(!llist.find(it.current()->library))
			llist.insert(it.current()->library,new QStringList());

		llist[it.current()->library]->append(it.current()->language);
		llist[it.current()->library]->append(it.current()->object);
	}

	config -> writeEntry("libsKnown",libsknown);
	QDictIterator<QStringList> sit(llist);
	for(sit.toFirst();sit.current();++sit)
		config -> writeEntry(sit.currentKey(),**sit);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::readOptions() {
	// bar status settings
	toolBar("mainToolBar")->applySettings(config, "toolbar");
	//do config for work toolbar
	toolsbar->applySettings(config, "workbar");
	fileOpenRecent->loadEntries(config,"Recent Files");
	config->setGroup("General Options");
	setImageMimetype(config->readEntry("imageMimetype","image/png"));
	resize( config->readSizeEntry("Geometry", new QSize(630,460)) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void UMLApp::saveProperties(KConfig *_config) {
	if(doc->URL().fileName()!=i18n("Untitled") && !doc->isModified()) {
		// saving to tempfile not necessary

	} else {
		KURL url=doc->URL();
#if KDE_IS_VERSION(3,1,3)
		_config->writePathEntry("filename", url.url());
#else
		_config->writeEntry("filename", url.url());
#endif
		_config->writeEntry("modified", doc->isModified());
		QString tempname = kapp->tempSaveName(url.url());
		QString tempurl= KURL::encode_string(tempname);

		KURL _url(tempurl);
		doc->saveDocument(_url);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::readProperties(KConfig* _config) {
	QString filename = _config->readPathEntry("filename");
	KURL url(filename);
	bool modified = _config->readBoolEntry("modified", false);
	if(modified) {
		bool canRecover;
		QString tempname = kapp->checkRecoverFile(filename, canRecover);
		KURL _url(tempname);


		if(canRecover) {
			doc->openDocument(_url);
			doc->setModified();
			enablePrint(true);
			setCaption(_url.fileName(),true);
			QFile::remove
				(tempname);
		} else {
			enablePrint(false);
		}
	} else {
		if(!filename.isEmpty()) {
			doc->openDocument(url);
			enablePrint(true);
			setCaption(url.fileName(),false);

		} else {
			enablePrint(false);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////

bool UMLApp::queryClose() {
	writeDockConfig();
	return doc->saveModified();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLApp::queryExit() {
	saveOptions();
	doc -> deleteContents();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotFileNew() {
	slotStatusMsg(i18n("Creating new document..."));
	if(doc->saveModified()) {
		setDiagramMenuItemsState(false);
		doc->newDocument();
		setCaption(doc->URL().fileName(), false);
		fileOpenRecent->setCurrentItem( -1 );
		setModified(false);
		enablePrint(false);
	}
	slotUpdateViews();
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotFileOpen() {
	slotStatusMsg(i18n("Opening file..."));
	loading = true;

	if(!doc->saveModified()) {

		// here saving wasn't successful

	} else {
		KURL url=KFileDialog::getOpenURL(":open-umbrello-file",
						 i18n("*.xmi|Umbrello XMI Files\n*|All Files"),
				this, i18n("Open File"));
		if(!url.isEmpty()) {
			if(doc->openDocument(url))
				fileOpenRecent->addURL( url );
			enablePrint(true);
			setCaption(doc->URL().fileName(), false);
		}

	}
	slotUpdateViews();
	loading = false;
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotFileOpenRecent(const KURL& url) {
	slotStatusMsg(i18n("Opening file..."));
	loading = true;

	KURL oldURL = doc->URL();

	if(!doc->saveModified()) {
		// here saving wasn't successful
	} else {
		if(!doc->openDocument(url)) {
			fileOpenRecent->removeURL(url);
			fileOpenRecent->setCurrentItem( -1 );
		}
		enablePrint(true);
		setCaption(doc->URL().fileName(), false);
	}

	loading = false;
	slotUpdateViews();
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotFileSave() {
	slotStatusMsg(i18n("Saving file..."));
	if(doc->URL().fileName() == i18n("Untitled"))
		slotFileSaveAs();
	else
		doc->saveDocument(doc -> URL());

	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLApp::slotFileSaveAs()
{
	slotStatusMsg(i18n("Saving file with a new filename..."));
	bool cont = true;
	KURL url;
	QString ext;
	while(cont) {
		url=KFileDialog::getSaveURL(":save-umbrello-file", i18n("*.xmi|XMI Files\n*|All Files"), this, i18n("Save As"));

		if(url.isEmpty())
			cont = false;
		else {

			QString file = url.path(-1);
			QFileInfo info(file);
			ext = info.extension(false);
			if(ext != "xmi")
				url.setFileName(url.fileName() + ".xmi");
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
		doc->saveDocument(url);
		fileOpenRecent->addURL(url);
		setCaption(url.fileName(),doc->isModified());
		slotStatusMsg(i18n("Ready."));
		return true;

	} else {
		slotStatusMsg(i18n("Ready."));
		return false;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotFileClose() {
	slotStatusMsg(i18n("Closing file..."));

	slotFileNew();

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotFilePrint()
{
	slotStatusMsg(i18n("Printing..."));

	KPrinter printer;
	printer.setFullPage(true);
	DiagramPrintPage * selectPage = new DiagramPrintPage(0, doc);
	printer.addDialogPage(selectPage);
	QString msg;
	if (printer.setup(this, i18n("Print %1").arg(doc->URL().prettyURL()))) {

		doc -> print(&printer);
	}
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotFileQuit() {
	slotStatusMsg(i18n("Exiting..."));
	if(doc->saveModified()) {
		writeDockConfig();
		saveOptions();
		kapp->quit();
	}
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotEditUndo() {
	doc->loadUndoData();
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotEditRedo() {
	doc->loadRedoData();
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotEditCut() {
	slotStatusMsg(i18n("Cutting selection..."));
	//FIXME bug 59774 this fromview isn't very reliable.
	//when cutting diagrams it is set to true even though it shouldn't be
	bool fromview = (doc->getCurrentView() && doc->getCurrentView()->getSelectCount());
	if (!fromview) {
		//so it knows to delete the selection
		listView->setStartedCut(true);
	}
	if ( editCutCopy(fromview) ) {
		emit sigCutSuccessful();
		slotDeleteSelectedWidget();
		doc->setModified(true);
	}
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotEditCopy() {
	slotStatusMsg(i18n("Copying selection to clipboard..."));
	bool  fromview = (doc->getCurrentView() && doc->getCurrentView()->getSelectCount());
	editCutCopy( fromview );
	slotStatusMsg(i18n("Ready."));
	doc -> setModified( true );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotEditPaste() {
	slotStatusMsg(i18n("Inserting clipboard contents..."));
	QMimeSource* data = QApplication::clipboard()->data();
	UMLClipboard clipboard;
	setCursor(KCursor::waitCursor());
	if(!clipboard.paste(doc, data)) {
		KMessageBox::sorry( this, i18n("Umbrello could not paste the clipboard contents.  "
					       "The objects in the clipboard may be of the wrong "
					       " type to be pasted here."), i18n("Paste Error") );
	}
	slotStatusMsg(i18n("Ready."));
	setCursor(KCursor::arrowCursor());
	editPaste->setEnabled(false);
	doc -> setModified( true );
}

#if KDE_VERSION < 0x030190
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
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
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotStatusMsg(const QString &text) {
	///////////////////////////////////////////////////////////////////
	// change status message permanently
	statusBar()->clear();
	m_statusLabel->setText( text );

	if ( loading )
		m_statusLabel->repaint();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotClassDiagram() {
	getDocument() -> createDiagram( Uml::dt_Class ) ;
}

void UMLApp::UcreateDiagram() {
	Diagram::DiagramType t;
	const QObject *o = sender();
	if(o == UclassDiagram ) {
		t = Diagram::ClassDiagram;
	} else if( o == UuseCaseDiagram) {
		t = Diagram::UseCaseDiagram;
	} else if( o ==  UcollaborationDiagram) {
		t = Diagram::CollaborationDiagram;
	} else if( o == UactivityDiagram ) {
		t = Diagram::ActivityDiagram;
	} else if( o == UsequenceDiagram ) {
		t = Diagram::SequenceDiagram;
	} else {
		kdWarning() << "uninitialised variable t, using default" << endl;
		t = Diagram::ClassDiagram;
	}
	Umbrello::Diagram *d = getDocument()->UcreateDiagram( t );
	if(!d) {
		return;
	}
	Umbrello::DiagramView *v = d->createView(viewStack);
	diagramViews[d->getID()] = v;
	kdDebug()<<"diagram created with id = "<<d->getID()<<endl;
	viewStack->raiseWidget(v);
	v->setFocus();
}

void UMLApp::UdiagramSelected(int id) {
	if(diagramViews.find(id) != diagramViews.end()) {
		viewStack->raiseWidget(diagramViews[id]);
		diagramViews[id]->setFocus();
	}
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

WorkToolBar* UMLApp::getWorkToolBar() {
	return toolsbar;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::setModified(bool _m) {
	//fileSave -> setEnabled(_m);

	//if anything else needs to be done on a mofication, put it here

	// printing should be possible whenever there is something to print
	if ( _m == true && doc->getCurrentView() )  {
		enablePrint(true);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::enablePrint(bool enable) {
	filePrint->setEnabled(enable);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::enableUndo(bool enable) {
	editUndo->setEnabled(enable);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::enableRedo(bool enable) {
	editRedo->setEnabled(enable);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

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
	if(listView->getSelectedCount() || (doc->getCurrentView() && doc->getCurrentView()->getSelectCount())) {
		editCopy->setEnabled(true);
		editCut->setEnabled(true);
	} else {
		editCopy->setEnabled(false);
		editCut->setEnabled(false);
	}
}

void UMLApp::slotPrefs() {
	/* the KTipDialog may have changed the value */
	config -> setGroup( "TipOfDay");
	optionState.generalState.tip = config -> readBoolEntry( "RunOnStart", true );

	dlg = new SettingsDlg(this, optionState, ldict, activeLanguage, getGenerator());
	connect(dlg, SIGNAL( applyClicked() ), this, SLOT( slotApplyPrefs() ) );

	if( dlg->exec() == QDialog::Accepted && dlg->getChangesApplied() ) {
		slotApplyPrefs();
	}

	delete dlg;
	dlg = NULL;
}

void UMLApp::slotApplyPrefs() {
	if (dlg) {
		//changes made so let the document tell the views
		optionState = dlg->getOptionState();

		/* we need this to sync both values */
		config -> setGroup( "TipOfDay");
		config -> writeEntry( "RunOnStart", optionState.generalState.tip );

		doc -> settingsChanged( optionState );
		setActiveLanguage( dlg->getCodeGenerationLanguage() );
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

	if((clipdata = clipboard.copy(doc, bFromView)) != 0) {
		QClipboard* clip = QApplication::clipboard();
		clip->setData(clipdata);//the global clipboard takes ownership of the clipdata memory
		connect(clip, SIGNAL(dataChanged()), this, SLOT(slotClipDataChanged()));
		return true;
	}
	return false;
}

void UMLApp::readOptionState() {
	config -> setGroup( "General Options" );
	optionState.generalState.autosave = config -> readBoolEntry( "autosave", false );
	optionState.generalState.time = config -> readNumEntry( "time", 0 );
	optionState.generalState.logo = config -> readBoolEntry( "logo", true );
	optionState.generalState.loadlast = config -> readBoolEntry( "loadlast", true );

	optionState.generalState.diagram  = ( SettingsDlg::Diagram ) config -> readNumEntry( "diagram", 1 );
	config -> setGroup( "TipOfDay");

	optionState.generalState.tip = config -> readBoolEntry( "RunOnStart", true );

	config -> setGroup( "UI Options" );
	optionState.uiState.useFillColor = config -> readBoolEntry( "useFillColor", true );
	QColor defaultYellow = QColor( 255, 255, 192 );

	optionState.uiState.fillColor = config -> readColorEntry( "fillColor", &defaultYellow );
	optionState.uiState.lineColor = config -> readColorEntry( "lineColor", &red );
	QFont font = ((QWidget *) this)->font() ;
	optionState.uiState.font = config -> readFontEntry("font", &font );

	config -> setGroup( "Class Options" );

	optionState.classState.showScope = config -> readBoolEntry("showVisibility", true);
	optionState.classState.showAtts = config -> readBoolEntry("showAtts", true);
	optionState.classState.showOps = config -> readBoolEntry("showOps", true);
	optionState.classState.showStereoType = config -> readBoolEntry("showStereoType", false);
	optionState.classState.showAttSig = config -> readBoolEntry("showAttSig", true);
	optionState.classState.showOpSig = config -> readBoolEntry("ShowOpSig", true);
	optionState.classState.showPackage = config -> readBoolEntry("showPackage", false);

	config -> setGroup( "Code Viewer Options" );

	QColor defaultWhite = QColor( "white" );
	QColor defaultBlack = QColor( "black" );
	QColor defaultPink = QColor( "pink" );
	QColor defaultGrey = QColor( "grey" );

	optionState.codeViewerState.height = config -> readNumEntry( "height", 40 );
	optionState.codeViewerState.width = config -> readNumEntry( "width", 80 );
	optionState.codeViewerState.font = config -> readFontEntry("font", &font );
	optionState.codeViewerState.showHiddenBlocks = config -> readBoolEntry( "showHiddenBlocks", false);
	optionState.codeViewerState.blocksAreHighlighted = config -> readBoolEntry( "blocksAreHighlighted", false);
	optionState.codeViewerState.selectedColor = config -> readColorEntry( "selectedColor", &defaultYellow );
	optionState.codeViewerState.paperColor = config -> readColorEntry( "paperColor", &defaultWhite);
	optionState.codeViewerState.fontColor = config -> readColorEntry( "fontColor", &defaultBlack);
	optionState.codeViewerState.editBlockColor = config -> readColorEntry( "editBlockColor", &defaultPink);
	optionState.codeViewerState.umlObjectColor = config -> readColorEntry( "umlObjectBlockColor", &defaultPink);
	optionState.codeViewerState.nonEditBlockColor = config -> readColorEntry( "nonEditBlockColor", &defaultGrey);
	optionState.codeViewerState.hiddenColor = config -> readColorEntry( "hiddenColor", &defaultGrey);

}


/** Call the code viewing assistant on a code document */
void UMLApp::viewCodeDocument ( UMLClassifier * c)
{

	CodeGenerator * currentGen = getGenerator();
        if(currentGen && c)
        {
 		if(!dynamic_cast<SimpleCodeGenerator*>(currentGen))
		{
		   CodeDocument *cdoc = currentGen->findCodeDocumentByClassifier(c);

		   if (cdoc) {
			CodeViewerDialog * dialog = currentGen->getCodeViewerDialog(this,cdoc,optionState.codeViewerState);
			dialog->exec();
			optionState.codeViewerState = dialog->getState();
			delete dialog;
			dialog = NULL;
		   } else {
			// shouldnt happen..
			KMessageBox::sorry(0, i18n("Cannot view code until you generate some first!"),i18n("Cannot View Code"));
		   }
		} else {
		    KMessageBox::sorry(0, i18n("Cannot view code from simple code writer!"),i18n("Cannot View Code"));
		}
	}

}

void UMLApp::refactor( UMLClassifier *c ){
 if(! m_refactoringAssist )
 {
	m_refactoringAssist = new RefactoringAssistant( doc, 0, 0, "refactoring_assistant" );
 }
 m_refactoringAssist->setObject( c );
 m_refactoringAssist->show();
}

void UMLApp::setGenerator ( CodeGenerator * gen, bool giveWarning ) {

	if(!gen )
	{
		if(giveWarning)
			KMessageBox::sorry(this,i18n("Could not find a code generator. Is the code generation library out of date?\n Please (re-)install libcodegenerator."),i18n("No Code Generation Library"));

		return;
	}

        // IF there is a current generator, it has a policy that MAY have
        // been changed. IF so, we should merge it back with our 'default'
        // policy. Yes, it would be better if we simply sync'd up all the
        // existing policies to the default policy via callbacks, but it
        // is more work and harder to implment. THis simple solution is ugly
        // but works. -b.t.
        CodeGenerator * current = getDocument()->getCurrentCodeGenerator();
        if (current)
                m_defaultcodegenerationpolicy->setDefaults(current->getPolicy(), false);

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
	GeneratorInfo *info;
	CodeGenerator *g = 0;

	if(activeLanguage.isEmpty()) {
		KMessageBox::sorry(this,i18n("There is no active language defined.\nPlease select\
		                             one of the installed languages to generate the code with."),i18n("No Language Selected"));
		return 0;
	}
	info = ldict.find(activeLanguage);
	if(!info) {
		KMessageBox::sorry(this,i18n("Could not find active language.\nPlease select\
		                             one of the installed languages to generate the code with."),i18n("No Language Selected"));
		return 0;
	}

	// does the code generator for our activeLanguage already exist?
	// then simply return that
	g = getDocument()->findCodeGeneratorByLanguage(activeLanguage);
	if(g) {
		return g;
	}

	KLibLoader *loader = KLibLoader::self();
	if(!loader) {
		kdDebug()<<"error getting KLibLoader!"<<endl;
		return 0;
	}

	KLibFactory *fact = loader->factory(info->library.latin1());
	if(!fact) {
		kdDebug()<<"error getting the Factory"<<endl;
		return 0;
	}

	QObject *o=fact->create(doc,0,info->object.latin1());
	if(!o) {
		kdDebug()<<"could not create object"<<endl;
		return 0;
	}

	//g = static_cast<CodeGenerator*>(o);
	g = dynamic_cast<CodeGenerator*>(o);

        // now set defaults on the new policy from the the configuration
        // file and THEN the default policy, which may have been updated
	// since it was first created. In both cases, DONT emit the modifiedCodeContent
	// signal as we will be doing that later.
	//
	g->getPolicy()->setDefaults(config, false); // picks up language specific stuff
	g->getPolicy()->setDefaults(m_defaultcodegenerationpolicy, false);

	// configure it from XMI
	QDomElement elem = getDocument()->getCodeGeneratorXMIParams( g->getLanguage() );
	g->loadFromXMI(elem);

	// add to our UML document
	getDocument()->addCodeGenerator(g);

	return g;
}

void UMLApp::configureLanguages() {
	ConfigCodeGenerators d(ldict,this);
	if(d.exec()) {
		ldict.clear();
		ldict = d.configDictionary();
		updateLangSelectMenu();
	}
}

void UMLApp::generateAllCode() {
	CodeGenerator* gen = getGenerator();
	if (gen) {
		gen->writeCodeToFile();
	}
}

void UMLApp::generationWizard() {
	CodeGenerationWizard wizard(doc, 0, ldict, activeLanguage, this);
	wizard.exec();
}

void UMLApp::setActiveLanguage(int id) {

	// only change the active language IF different from one we currently have
	if (!langSelect->isItemChecked(id))
	{

		for(unsigned int i=0; i < langSelect->count(); i++) {
			langSelect->setItemChecked(langSelect->idAt(i),false);  //uncheck everything
		}


		langSelect->setItemChecked(id,true);
		activeLanguage = langSelect->text(id);

		// update the generator
		setGenerator(createGenerator());
	}
}

void UMLApp::setActiveLanguage(QString activeLanguage) {

	for(unsigned int j=0; j < langSelect->count(); j++) {
		int id = langSelect->idAt(j);

		if (langSelect->text(id) == activeLanguage &&
		      langSelect->isItemChecked(id))
			return; // already set.. no need to do anything
	}

	for(unsigned int i=0; i < langSelect->count(); i++) {
		bool isActiveLang = (langSelect->text(langSelect->idAt(i)) == activeLanguage);
		//uncheck everything except the active language
		langSelect->setItemChecked(langSelect->idAt(i), isActiveLang);
	}
	this->activeLanguage = activeLanguage;

	setGenerator(createGenerator());
}

void UMLApp::slotCurrentViewClearDiagram() {
	doc->getCurrentView()->clearDiagram();
}

void UMLApp::slotCurrentViewToggleSnapToGrid() {
	doc->getCurrentView()->toggleSnapToGrid();
	viewSnapToGrid->setChecked( doc->getCurrentView()->getSnapToGrid() );
}

void UMLApp::slotCurrentViewToggleShowGrid() {
	doc->getCurrentView()->toggleShowGrid();
	viewShowGrid->setChecked( doc->getCurrentView()->getShowSnapGrid() );
}

void UMLApp::slotCurrentViewExportImage() {
	doc->getCurrentView()->exportImage();
}

void UMLApp::slotCurrentViewProperties() {
	doc->getCurrentView()->showPropDialog();
}

void UMLApp::setDiagramMenuItemsState(bool bState) {
	viewClearDiagram->setEnabled( bState );
	viewSnapToGrid->setEnabled( bState );
	viewShowGrid->setEnabled( bState );
	deleteDiagram->setEnabled(bState);
	viewExportImage->setEnabled( bState );
	viewProperties->setEnabled( bState );
	filePrint->setEnabled( bState );
	if ( doc->getCurrentView() ) {
		viewSnapToGrid->setChecked( doc->getCurrentView()->getSnapToGrid() );
		viewShowGrid->setChecked( doc->getCurrentView()->getShowSnapGrid() );
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

void UMLApp::slotImportClasses() {
	QStringList fileList = KFileDialog::getOpenFileNames(":import-classes",
	                       i18n("*.h *.hpp *.hxx|Header Files (*.h *.hpp *.hxx)\n*|All Files"), this, i18n("Select Classes to Import") );
	((ClassImport*)doc)->importCPP( fileList );
}

void UMLApp::slotClassWizard() {
	ClassWizard dlg( doc );
	dlg.exec();
}

void UMLApp::slotCurrentViewChanged() {
	if ( doc->getCurrentView() ) {
		connect(doc->getCurrentView(), SIGNAL( sigShowGridToggled(bool) ),
			this, SLOT( slotShowGridToggled(bool) ) );
		connect(doc->getCurrentView(), SIGNAL( sigSnapToGridToggled(bool) ),
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
        doc->getCurrentView()->selectAll();
}

void UMLApp::slotDeleteSelectedWidget() {
	if ( doc->getCurrentView() ) {
		doc->getCurrentView()->deleteSelection();
	} else {
		kdWarning() << " trying to delete widgets when there is no current view (see bug 59774)" << endl;
	}
}

void UMLApp::slotDeleteDiagram() {
	doc->removeDiagram( doc->getCurrentView()->getID() );
}

void UMLApp::initLibraries() {
	KStandardDirs stdDirs;
	QStringList libsKnown, libsFound, libsMissing,strlist;
	QString str;
	config -> setGroup("Code Generation");
	activeLanguage = config->readEntry("activeLanguage");
	//first search for libraries in all "codegenerators" directoris, and then
	//see which of them are already known and which are new
	libsFound = stdDirs.findAllResources("data","umbrello/codegenerators/*.la",false,true);

	QFileInfo fi;
	for ( QStringList::Iterator it = libsFound.begin(); it != libsFound.end(); ++it ) {
		fi.setFile(*it);        //leav only filename (no path, no extension)
		*it = fi.fileName().left(fi.fileName().length()-3);
	}


	libsKnown = config->readListEntry("libsKnown",':');

	for ( QStringList::Iterator it = libsKnown.begin(); it != libsKnown.end(); ++it )
		if(libsFound.contains(*it))
			libsFound.remove(*it);
		else
			libsMissing.append(*it); //it was registered and now is gone,
	// add to missing libs.


	for ( QStringList::Iterator it = libsMissing.begin(); it != libsMissing.end(); ++it )
		libsKnown.remove(*it);


	//debug output..
	for ( QStringList::Iterator it = libsKnown.begin(); it != libsKnown.end(); ++it )
		kdDebug()<<"Known lib: "<<*it<<endl;
	for ( QStringList::Iterator it = libsFound.begin(); it != libsFound.end(); ++it )
		kdDebug()<<"New lib: "<<*it<<endl;
	for ( QStringList::Iterator it = libsMissing.begin(); it != libsMissing.end(); ++it )
		kdDebug()<<"Missing lib: "<<*it<<endl;
	///////////////

	//add paths for the library loader.
	QString ld_path = getenv("LD_LIBRARY_PATH");
	if(!ld_path.isEmpty())
		ld_path +=":";

	QStringList dirs = stdDirs.findDirs("data","umbrello/codegenerators/");
	for ( QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it )
		ld_path+=*it+":";

	setenv("LD_LIBRARY_PATH",ld_path.latin1(),1);



	QStringList languages;
	GeneratorInfo *info;

	//////// Load information for already registered libraries.
	for ( QStringList::Iterator it = libsKnown.begin(); it != libsKnown.end(); ++it ) {
		languages = config->readListEntry(*it);
		for ( QStringList::Iterator lit = languages.begin(); lit != languages.end(); ++lit ) {
			info = new GeneratorInfo;
			info -> language = *lit;  //language name
			info->library = *it ;
			++lit;                 //go to next entry list (object)
			info->object = *lit;
			ldict.insert(info->language,info);
		}
	}



	//////// Register new libraries
	if(!libsFound.isEmpty()) {

		KMessageBox::information(this,i18n("Umbrello UML Modeller found new code generation libraries.\nPlease select the languages you want to use."),
		                         i18n("New Libraries Found"));

		SelectLanguagesDlg d(this);
		d.offerLanguages(libsFound);
		if(d.exec()) {
			GeneratorInfoDict tempdict;
			tempdict = d.selectedLanguages();
			tempdict.setAutoDelete(false);
			QDictIterator<GeneratorInfo> dictit( tempdict );
			for( dictit.toFirst(); dictit.current(); ++dictit ) { //check if there is another language with the same name in other lib...
				info = dictit.current();
				int suffix=2;
				if(ldict.find(info->language)) {
					while(ldict.find(info->language+":"+QString::number(suffix)))
						suffix++;
					info->language+=":" + QString::number(suffix);
				}
				ldict.insert(info->language,info);
			}
		}

	} //endif (!libFound.isEmpty())

	////////// Discard deleted libraries
	if(!libsMissing.isEmpty()) {
		for ( QStringList::Iterator it = libsMissing.begin(); it != libsMissing.end(); ++it )
			str += *it + "\n";


		KMessageBox::information(this,i18n("The following libraries are registered in Umbrello UML Modeller\
		                                   for code generation but can not be found. They will be removed from the configuration.\n")+ str);
		//maybe show the path were the libs were looked for?

	}


	///////// Finaly done... just update the language selection menu.
	updateLangSelectMenu();
}


void UMLApp::updateLangSelectMenu() {
	langSelect->clear();
	langSelect->setCheckable(true);
	int id;
	bool foundActive = false;
	QDictIterator<GeneratorInfo> it( ldict );
	for(it.toFirst() ; it.current(); ++it ) {
		id = langSelect->insertItem(it.current()->language,this,SLOT(setActiveLanguage(int)));
		if(activeLanguage == it.current()->language) {
			langSelect->setItemChecked(id,true);
			foundActive = true;
		} else
			langSelect->setItemChecked(id,false);
	}

	//if we could not find the active language, we try to fall back to C++
	if(!foundActive) {
		for(uint index=0; index <langSelect->count(); index++)
			if (langSelect->text(langSelect->idAt(index)) =="Cpp" ) {
				langSelect->setItemChecked(langSelect->idAt(index),true);
				activeLanguage = langSelect->text(langSelect->idAt(index));
				break;
			}
	}
	//last try... if we dont have a activeLanguage and we have no Cpp installed we just
	//take the first language we find as "active"
	if(activeLanguage.isEmpty() && langSelect->count() > 0) {
		langSelect -> setItemChecked(langSelect->idAt(0),true);
		activeLanguage = langSelect -> text(langSelect->idAt(0));
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

void UMLApp::keyReleaseEvent(QKeyEvent *e) {
	switch(e->key()) {
		case Qt::Key_Shift:
			toolsbar->setOldTool();
			e->accept();
			break;
		case Qt::Key_Escape:
			toolsbar->setDefaultTool();
			e->accept();
			break;
		default:
			e->ignore();
	}

}

void UMLApp::newDocument() {

	doc->newDocument();
	setGenerator(createGenerator());
	slotUpdateViews();
}

void UMLApp::initSavedCodeGenerators() {
	QString activeLang = activeLanguage;
        QDictIterator<GeneratorInfo> it( ldict );
        for(it.toFirst() ; it.current(); ++it )
	{
		activeLanguage = it.current()->language;
		if( doc->hasCodeGeneratorXMIParams(activeLanguage))
			createGenerator();
	}

	// now set back to old activeLanguage
	activeLanguage = activeLang;
	setGenerator(createGenerator());
}

QWidget* UMLApp::getMainViewWidget() {
	return viewStack;
}

void UMLApp::setCurrentView(UMLView* view /*=0*/) {
	if (view) {
		viewStack->raiseWidget(view);
		slotStatusMsg(view->getName());
	} else {
		viewStack->raiseWidget(blankWidget);
	}
}

QPopupMenu* UMLApp::findMenu(QMenuData* menu, QString name) {

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

//static pointer, holding the unique instance
UMLApp* UMLApp::s_instance;

#include "uml.moc"
