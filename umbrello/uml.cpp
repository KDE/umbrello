/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classimport.h"
#include "docwindow.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "worktoolbar.h"
#include "clipboard/umlclipboard.h"
#include "clipboard/umldrag.h"
#include "dialogs/classwizard.h"
#include "dialogs/codegenerationwizard.h"
#include "dialogs/configcodegenerators.h"
#include "dialogs/diagramprintpage.h"
#include "dialogs/selectlanguagesdlg.h"
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
#include <qpopmenu.h>
#include <qtimer.h>

UMLApp::UMLApp(QWidget* , const char* name):KMainWindow(0, name) {
	m_pDocWindow = 0;
	config=kapp->config();
	listView = 0;
	ldict.setAutoDelete(true);
	langSelect = new QPopupMenu(this);
	zoomSelect = new QPopupMenu(this);

	loading = false;
	m_clipTimer = 0;
	m_copyTimer = 0;
	///////////////////////////////////////////////////////////////////
	// call inits to invoke all other construction parts
	readOptionState();
	initActions();
	initDocument();
	initView();
	initClip();
	initStatusBar();
	readOptions();
	if( optionState.uiState.showDocWindow )
		m_pDocWindow -> show();
	else
		m_pDocWindow -> hide();
	///////////////////////////////////////////////////////////////////
	// disable actions at startup
	fileSave->setEnabled(true);
	fileSaveAs->setEnabled(true);
	filePrint->setEnabled(false);
	editCut->setEnabled(false);
	editCopy->setEnabled(false);
	editPaste->setEnabled(false);

	//insert Language Selection menu in "Code Generation" in the Menubar
	// --> the name "Code Generation" as seen in the Menu is taken from .ui file..
	// and depends on the language we are using (english, spanish, etc) so we need to
	// look for the object name instead.
	int t = menuBar() -> count();
	int id=-1;
	for( int i =0; i<t;i++) {
		id = menuBar()->idAt(i);
		if(id == -1)
			break;
		if(menuBar()->findItem(id)->popup()->name() == QString("code"))
			break;
	}
	if(id!=-1) {
		QPopupMenu *m = menuBar()->findItem(id)->popup();
		if(m) {
			m->insertItem(i18n("Active Language"),langSelect,-1,m->count()-2);
		}
	}

	//insert zoom menu. see comment above (language selection menu)
 	id=-1;
	for( int i =0; i<t;i++)
	{
		id = menuBar()->idAt(i);
		if(id == -1)
			break;
		if(menuBar()->findItem(id)->popup()->name() == QString("views"))
			break;
	}
	if(id!=-1) {
		QPopupMenu *m = menuBar()->findItem(id)->popup();
		if(m) {
			m->insertItem(i18n("Zoom"),zoomSelect,-1,m->count()-1);
		}
	}
	//setup zoomSelect menu
	zoomSelect->setCheckable(true);
	connect(zoomSelect,SIGNAL(aboutToShow()),this,SLOT(setupZoomMenu()));
	connect(zoomSelect,SIGNAL(activated(int)),this,SLOT(setZoom(int)));
	//IMPORTANT: The ID's must match the zoom value (text)
	zoomSelect->insertItem(i18n(" 33 %"),33);
	zoomSelect->insertItem(i18n(" 50 %"),50);
	zoomSelect->insertItem(i18n(" 75 %"),75);
	zoomSelect->insertItem(i18n("100 %"),100);
	zoomSelect->insertItem(i18n("150 %"),150);
	zoomSelect->insertItem(i18n("200 %"),200);
	zoomSelect->insertItem(i18n("300 %"),300);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLApp::~UMLApp() {
	delete m_clipTimer;
	delete m_copyTimer;

	delete statProg;
	delete m_statusLabel;
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
	editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection());
	editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection());
	editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());
	viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
	viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
	selectAll = KStdAction::selectAll(this,  SLOT( slotSelectAll() ), actionCollection());

	zoomInAction = KStdAction::zoomIn(this,  SLOT( zoomIn() ), actionCollection(), "umbrello_zoom_in");
	zoomOutAction = KStdAction::zoomOut(this,  SLOT( zoomOut() ), actionCollection(), "umbrello_zoom_out");

	classWizard = new KAction(i18n("New Class Wizard..."),0,this,SLOT(slotClassWizard()),
	                          actionCollection(),"class_wizard");

	showDocumentation = new KToggleAction( i18n("Show Documentation") , 0 , this, SLOT( slotShowDocWindow() ),
	                                       actionCollection(), "Show_Documentation" );
	preferences = new KAction(i18n("Configure Umbrello..."), SmallIconSet("configure"), 0,
				  this, SLOT( slotPrefs() ), actionCollection(), "Configure_UML");

	genWizard = new KAction(i18n("Code Generation Wizard..."),0,this,SLOT(generationWizard()),
	                        actionCollection(),"generation_wizard");
	genAll = new KAction(i18n("Generate All Code"),0,this,SLOT(generateAllCode()),
	                     actionCollection(),"generate_all");

	importClasses = new KAction(i18n("Import Classes..."), SmallIconSet("source_cpp"), 0,
				    this,SLOT(slotImportClasses()), actionCollection(),"import_class");

	confLanguages = new KAction(i18n("Add/Remove Generation Languages..."),0,this,
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
	viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
	viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));
	preferences->setStatusText( i18n( "Set the default program preferences") );

	deleteSelectedWidget = new KAction( i18n("Delete Selected"),
					    SmallIconSet("editdelete"),
					    KShortcut(Key_Delete), this,
					    SLOT( slotDeleteSelectedWidget() ), actionCollection(),
					    "delete_selected" );

	// The different views
        newDiagram = new KActionMenu(0,  SmallIconSet("new"), actionCollection(), "new_view");
	classDiagram = new KAction( i18n( "&Class Diagram" ), SmallIconSet("folder_green"), 0,
	                            this, SLOT( slotClassDiagram() ), actionCollection(), "new_class_diagram" );

	sequenceDiagram= new KAction( i18n( "&Sequence Diagram" ), SmallIconSet("folder_green"), 0,
	                              this, SLOT( slotSequenceDiagram() ), actionCollection(), "new_sequence_diagram" );

	collaborationDiagram = new KAction( i18n( "&Collaboration Diagram" ), SmallIconSet("folder_green"), 0,
	                                    this, SLOT( slotCollaborationDiagram() ), actionCollection(), "new_collaboration_diagram" );

	useCaseDiagram= new KAction( i18n( "&Use Case Diagram" ), SmallIconSet("folder_grey"), 0,
	                             this, SLOT( slotUseCaseDiagram() ), actionCollection(), "new_use_case_diagram" );

	stateDiagram= new KAction( i18n( "S&tate Diagram" ), SmallIconSet("folder_green"), 0,
	                           this, SLOT( slotStateDiagram() ), actionCollection(), "new_state_diagram" );

	activityDiagram= new KAction( i18n( "&Activity Diagram" ), SmallIconSet("folder_green"), 0,
	                              this, SLOT( slotActivityDiagram() ), actionCollection(), "new_activity_diagram" );

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

	showDocumentation->setChecked( optionState.uiState.showDocWindow );
	showDocumentation->setStatusText( i18n( "Enables/disables the documentation window" ) );
	// use the absolute path to your umbrelloui.rc file for testing purpose in createGUI();
	createGUI();

}
//////////////////////////////////////////////////////////////////////////////////////////////

void UMLApp::setZoom(int z)
{
	doc->getCurrentView()->setZoom(z);
}

void UMLApp::setupZoomMenu()
{
	int zoom = doc->getCurrentView()->currentZoom();

	//first uncheck all
	for(int index = 0; index < zoomSelect->count(); index++)
		zoomSelect->setItemChecked(zoomSelect->idAt(index),false);
	zoomSelect->setItemChecked(zoom,true);
}


void UMLApp::zoomIn()
{
	doc->getCurrentView()->zoomIn();
}

void UMLApp::zoomOut()
{
	doc->getCurrentView()->zoomOut();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::initStatusBar() {
	///////////////////////////////////////////////////////////////////
	// STATUSBAR
	// TODO: add your own items you need for displaying current application status.
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

	connect(doc, SIGNAL( sigWriteToStatusBar(const QString &) ), this, SLOT( slotStatusMsg(const QString &) ));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::initDocument() {
	doc = new UMLDoc(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::initView() {
	splitter = new QSplitter( this,"SPLITTER" );
	m_pDocSplitter = new QSplitter( splitter ,"DOC SPLITTER" );
	m_pDocSplitter -> setOrientation( Vertical );

	listView = new UMLListView( m_pDocSplitter ,"LISTVIEW" );
	m_pDocSplitter -> setResizeMode( listView, QSplitter::Stretch );
	m_pDocWindow = new DocWindow( doc, m_pDocSplitter, "DOCWINDOW" );
	m_pDocWindow -> setMaximumSize( this -> width(), this -> height() / 2 );
	if( optionState.uiState.showDocWindow )
		m_pDocWindow -> show();
	else
		m_pDocWindow -> hide();
	m_pDocSplitter -> setResizeMode( m_pDocWindow, QSplitter::Stretch );
	splitter -> setResizeMode( m_pDocSplitter, QSplitter::KeepSize );
	toolsbar = new WorkToolBar(this,"WORKTOOLBAR");
	addToolBar(toolsbar,Right,true);
	doc -> setupListView(listView);//make sure has a link to list view and add info widget
	doc -> setupSignals();//make sure gets signal from list view
	listView -> setDocument(doc);
	setCentralWidget( splitter );
	setCaption(doc->URL().fileName(),false);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::openDocumentFile(const KURL& url) {
	slotStatusMsg(i18n("Opening file..."));

	doc->openDocument( url);
	fileOpenRecent->addURL( url );
	slotStatusMsg(i18n("Ready."));
	setCaption(doc->URL().fileName(), false);
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

	QValueList<int> list = splitter->sizes();
	config->writeEntry( "listviewwidth", *list.begin() );
	list = m_pDocSplitter->sizes();
	config->writeEntry( "listviewheight", *list.begin() );

	config->writeEntry( "autosave", optionState.generalState.autosave );
	config->writeEntry( "time", optionState.generalState.time );
	config->writeEntry( "logo", optionState.generalState.logo );
	config->writeEntry( "loadlast", optionState.generalState.loadlast );

	config->writeEntry( "diagram", optionState.generalState.diagram );
	if( doc->URL().fileName() == i18n( "Untitled" ) ) {
		config -> writeEntry( "lastFile", "" );
	} else {
		config -> writeEntry( "lastFile", doc -> URL().prettyURL() );
	}
	config->setGroup( "TipOfDay");
	optionState.generalState.tip = config -> readBoolEntry( "RunOnStart", true );
	config->writeEntry( "RunOnStart", optionState.generalState.tip );

	config->setGroup( "UI Options" );
	config->writeEntry( "useFillColor", optionState.uiState.useFillColor );
	config->writeEntry( "fillColor", optionState.uiState.fillColor );
	config->writeEntry( "lineColor", optionState.uiState.lineColor );
	config->writeEntry( "showDocWindow", optionState.uiState.showDocWindow );
	config->writeEntry( "font", optionState.uiState.font );


	config->setGroup( "Class Options" );
	config->writeEntry( "showVisibility", optionState.classState.showScope );
	config->writeEntry( "showAtts", optionState.classState.showAtts);
	config->writeEntry( "showOps", optionState.classState.showOps );
	config->writeEntry( "showStereoType", optionState.classState.showStereoType );
	config->writeEntry( "showAttSig", optionState.classState.showAttSig );
	config->writeEntry( "ShowOpSig", optionState.classState.showOpSig );
	config->writeEntry( "showPackage", optionState.classState.showPackage );

	config->setGroup("Code Generation");
	config->writeEntry("forceDoc",optionState.codegenState.forceDoc);
	config->writeEntry("forceSections",optionState.codegenState.forceSections);
	config->writeEntry("outputDirectory",optionState.codegenState.outputDir);
	config->writeEntry("headingsDirectory",optionState.codegenState.headingsDir);
	config->writeEntry("includeHeadings",optionState.codegenState.includeHeadings);
	config->writeEntry("overwritePolicy",optionState.codegenState.overwritePolicy);
	config->writeEntry("modnamePolicy",  optionState.codegenState.modname);
	config->writeEntry("activeLanguage",activeLanguage);

	//save library information
	QStringList libsknown;
	QDict<QStringList> llist;
	llist.setAutoDelete(true);
	QDictIterator<GeneratorInfo> it( ldict );
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
	resize( config->readSizeEntry("Geometry", new QSize(630,460)) );

	QValueList<int> list = splitter->sizes();
	list[0] = config->readNumEntry( "listviewwidth", 132);
	splitter->setSizes(list);

	list = m_pDocSplitter->sizes();
	list[0] = config->readNumEntry( "listviewheight", 226);
	m_pDocSplitter->setSizes(list);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void UMLApp::saveProperties(KConfig *_config) {
	if(doc->URL().fileName()!=i18n("Untitled") && !doc->isModified()) {
		// saving to tempfile not necessary

	} else {
		KURL url=doc->URL();
		_config->writeEntry("filename", url.url());
		_config->writeEntry("modified", doc->isModified());
		QString tempname = kapp->tempSaveName(url.url());
		QString tempurl= KURL::encode_string(tempname);

		KURL _url(tempurl);
		doc->saveDocument(_url);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::readProperties(KConfig* _config) {
	QString filename = _config->readEntry("filename", "");
	KURL url(filename);
	bool modified = _config->readBoolEntry("modified", false);
	if(modified) {
		bool canRecover;
		QString tempname = kapp->checkRecoverFile(filename, canRecover);
		KURL _url(tempname);


		if(canRecover) {
			doc->openDocument(_url);
			doc->setModified();
			setCaption(_url.fileName(),true);
			QFile::remove
				(tempname);
		}
	} else {
		if(!filename.isEmpty()) {
			doc->openDocument(url);
			setCaption(url.fileName(),false);

		}
	}

}
////////////////////////////////////////////////////////////////////////////////////////////////////

bool UMLApp::queryClose() {
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
		KURL url=KFileDialog::getOpenURL(":open-umbrello-file", i18n("*.xmi|XMI Files\n*.uml|Old UML Files\n*.*|All Files"),
				this, i18n("Open File"));
		if(!url.isEmpty()) {
			if(doc->openDocument(url))
				fileOpenRecent->addURL( url );
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
		url=KFileDialog::getSaveURL(":save-umbrello-file", i18n("*.xmi|XMI Files\n*.*|All Files"), this, i18n("Save As"));

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
	if (printer.setup(this)) {

		doc -> print(&printer);
	}
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotFileQuit() {
	slotStatusMsg(i18n("Exiting..."));
	if(!doc->saveModified()) {
	}
	else {
		saveOptions();
		kapp -> quit();
	}
	slotStatusMsg(i18n("Ready."));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::slotEditCut() {
	slotStatusMsg(i18n("Cutting selection..."));
	bool  fromview = (doc->getCurrentView() && doc->getCurrentView()->getSelectCount());
	if (!fromview) {
		//so it knows to delete the selection
		listView->setStartedCut(true);
	}
	if( editCutCopy(fromview) ) {
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
void UMLApp::slotSequenceDiagram() {
	getDocument() -> createDiagram( Uml::dt_Sequence );
}
void UMLApp::slotCollaborationDiagram() {
	getDocument() -> createDiagram( Uml::dt_Collaboration );
}
void UMLApp::slotUseCaseDiagram() {
	getDocument() -> createDiagram( Uml::dt_UseCase );
}

void UMLApp::slotStateDiagram()
{
	getDocument() -> createDiagram( Uml::dt_State );
}

void UMLApp::slotActivityDiagram() {
	getDocument() -> createDiagram( Uml::dt_Activity );
}


WorkToolBar* UMLApp::getWorkToolBar() {
	return toolsbar;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::setModified(bool _m) {
	//fileSave -> setEnabled(_m);

	//if anything else needs to be done on a mofication, put it here


	//if a file is modified, allow printing
	//we don't stop printing just because a file is modified, if a file has content
	//then we allow printing
	if(_m)
		enablePrint(_m);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLApp::enablePrint(bool _p) {
	filePrint -> setEnabled(_p);
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

	dlg = new SettingsDlg(this, optionState, ldict, activeLanguage);
	connect(dlg, SIGNAL( applyClicked() ), this, SLOT( slotApplyPrefs() ) );

	if( dlg->exec() == QDialog::Accepted && dlg->getChangesApplied() ) {
		slotApplyPrefs();
	}

	disconnect(dlg, SIGNAL( applyClicked() ), this, SLOT( slotShowDocWindow() ) );
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

		if( optionState.uiState.showDocWindow != showDocumentation->isChecked() ) {
			slotShowDocWindow();
		}
		doc -> settingsChanged( optionState );
		setActiveLanguage( dlg->getCodeGenerationLanguage() );
	}
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

void UMLApp::slotShowDocWindow() {
	if(! m_pDocWindow -> isVisible() ) {
		m_pDocWindow -> show();
	} else {
		m_pDocWindow -> hide();
	}
	showDocumentation -> setChecked( m_pDocWindow -> isVisible() );
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
	optionState.uiState.showDocWindow = config -> readBoolEntry( "showDocWindow", true );
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

	config -> setGroup("Code Generation");
	optionState.codegenState.forceDoc = config -> readBoolEntry("forceDoc",true);
	optionState.codegenState.forceSections = config -> readBoolEntry("forceSections",false);
	QString temp = config -> readEntry("outputDirectory","");
	if(temp.isEmpty())
		temp = QDir::homeDirPath() + "/uml-generated-code/";
	optionState.codegenState.outputDir = temp;
	optionState.codegenState.includeHeadings = config->readBoolEntry("includeHeadings",true);
	temp = "";
	temp = config -> readEntry("headingsDirectory","");
	if(temp.isEmpty()) {
		KStandardDirs stddirs;
		temp =  stddirs.findDirs("data","umbrello/headings").first();
	}
	optionState.codegenState.headingsDir = temp;
	optionState.codegenState.overwritePolicy = (CodeGenerator::OverwritePolicy)config -> readNumEntry("overwritePolicy",CodeGenerator::Ask);
	optionState.codegenState.modname = (CodeGenerator::ModifyNamePolicy)config -> readNumEntry("modnamePolicy",CodeGenerator::Capitalise);
}

CodeGenerator* UMLApp::generator() {
	GeneratorInfo *info;
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

	QObject *o=fact->create(0,0,info->object.latin1());
	if(!o) {
		kdDebug()<<"could not create object"<<endl;
		return 0;
	}

	CodeGenerator *g = (CodeGenerator*)o;
	g->setDocument(doc);
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
	CodeGenerator* gen = generator();
	if (gen) {
		gen->setForceDoc(optionState.codegenState.forceDoc);
		gen->setForceSections(optionState.codegenState.forceSections);
		gen->setIncludeHeadings(optionState.codegenState.includeHeadings);
		gen->setHeadingFileDir(optionState.codegenState.headingsDir);
		gen->setModifyNamePolicy(optionState.codegenState.modname);
		gen->setOutputDirectory(optionState.codegenState.outputDir);
		gen->setOverwritePolicy(optionState.codegenState.overwritePolicy);
		gen->generateAllClasses();
		delete gen;
	}
}

void UMLApp::generationWizard() {
	CodeGenerationWizard wizard(doc, 0, optionState.codegenState, ldict, activeLanguage, this);
	wizard.exec();
}

void UMLApp::setActiveLanguage(int id) {
	for(unsigned int i=0; i < langSelect->count(); i++) {
		langSelect->setItemChecked(langSelect->idAt(i),false);  //uncheck everything
	}
	langSelect->setItemChecked(id,true);
	activeLanguage = langSelect->text(id);
}

void UMLApp::setActiveLanguage(QString activeLanguage) {
	for(unsigned int i=0; i < langSelect->count(); i++) {
		if (langSelect->text(langSelect->idAt(i)) == activeLanguage) {
			langSelect->setItemChecked(langSelect->idAt(i),true);
		} else {
			langSelect->setItemChecked(langSelect->idAt(i),false);  //uncheck everything
		}
	}
	this->activeLanguage = activeLanguage;
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
	if ( doc->getCurrentView() ) {
		viewSnapToGrid->setChecked( doc->getCurrentView()->getSnapToGrid() );
		viewShowGrid->setChecked( doc->getCurrentView()->getShowSnapGrid() );
	}
}

void UMLApp::slotUpdateViews() {
	int count = menuBar() -> count();
	int id=-1;
	for( int i =0; i<count;i++) {
		id = menuBar()->idAt(i);
		if(id == -1)
			return;
		if(menuBar()->findItem(id)->popup()->name() == QString("views"))
			break;
	}
	if(id == -1)
		return;
	QPopupMenu *menu = menuBar()->findItem(id)->popup();
	if( !menu )
		return;
	count = menu -> count();
	id = -1;
	for( int i =0; i<count;i++) {
		id = menu -> idAt( i );
		QString text = menu -> findItem( id ) -> text();
		if( text == i18n("Show") )
			break;
	}
	if( id == -1 )
		return;
	menu = menu -> findItem( id ) -> popup();
	if( !menu )
		return;
	//	count = menu -> count();
	//	for( int i =0; i<count;i++)

	//		menu -> removeItemAt( i );
	menu -> clear();
	for(QListIterator<UMLView> views = getDocument()->getViewIterator(); views.current(); ++views) {
		menu->insertItem( views.current()->getName(), views.current(), SLOT( slotShowView() ) );
		views.current()->fileLoaded();
	}
}

void UMLApp::slotImportClasses() {
	QStringList fileList = KFileDialog::getOpenFileNames(":import-classes",
	                       i18n("*.h *.hpp *.hxx|Header Files (*.h *.hpp *.hxx)\n*.*|All Files"), this, i18n("Select Classes to Import") );
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
	doc->getCurrentView()->deleteSelection();
}

void UMLApp::slotDeleteDiagram() {
	doc->removeDiagram( doc->getCurrentView()->getID() );
}

void UMLApp::initLibraries() {
	KStandardDirs stdDirs;
	QStringList libsKnown, libsFound, libsMissing,strlist;
	QString str;
	config -> setGroup("Code Generation");
	activeLanguage = config->readEntry("activeLanguage","");
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

	//////// Load informatin for already registered libraries.
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
	slotUpdateViews();
}

#include "uml.moc"
