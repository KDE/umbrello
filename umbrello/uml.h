/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UML_H
#define UML_H

#define FILE_VERSION	5

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "codegenerator.h"
#include "dialogs/settingsdlg.h"

#include <kdockwidget.h>
#include <kurl.h>
#include <qprogressbar.h>

// forward declaration of the UML classes
class CodeGenerator;
class DocWindow;
class KActionMenu;
class KRecentFilesAction;
class KStatusBarLabel;
class KToggleAction;
class QClipboard;
class QSplitter;
class UMLDoc;
class UMLListView;
class UMLView;
class WorkToolBar;
class InfoWidget;
class QWidgetStack;
class QMenuData;

/**
 * The base class for UML application windows. It sets up the main
 * window and reads the config file as well as providing a menubar, toolbar
 * and statusbar. An instance of UMLView creates your centre view, which is connected
 * to the window's Doc object.
 * UMLApp reimplements the methods that KMainWindow provides for main window handling and supports
 * full session management as well as using KActions.
 * @see KMainWindow
 * @see KApplication
 * @see KConfig
 *
 * @author Paul Hensgen	<phensgen@techie.com>
 * @version 1.0
 */

class UMLApp : public KDockMainWindow {
	Q_OBJECT

	friend class UMLView;

public:
	/**
	 * 	construtor of UMLApp, calls all init functions to create the application.
	 */
	UMLApp(QWidget* parent=0, const char* name=0);

	/**
	 *	Standard deconstructor.
	 */
	~UMLApp();

	/**
	 *	opens a file specified by commandline option
	 */
	void openDocumentFile(const KURL& url=0);

	/**
	 * calls umldoc to create a new Document
	 */
	void newDocument();

	/**
	 *	returns a pointer to the current document connected to the KTMainWindow instance and is used by
	 * the View class to access the document object's methods
	 */
	UMLDoc *getDocument() const;

	/**
	 *	Returns the toolbar being used.
	 *
	 * @return Returns the toolbar being used.
	 */
	WorkToolBar* getWorkToolBar();

	/**
	 *	Sets whether the program has been modified.
	 *	This will change how the program saves/exits.
	 *
	 *	@param	_m	true - modified.
	 */
	void setModified(bool _m);

	/**
	 *	Set whether to allow printing.  It will enable/disable the menu/toolbar options.
	 *
	 *	@param enable Set whether to allow printing.
	 */
	void enablePrint(bool enable);

	/**
	 *	Set whether to allow printing.  It will enable/disable the menu/toolbar options.
	 *
	 *	@param enable Set whether to allow printing.
	 */
	void enableUndo(bool enable);

	/**
	 *	Set whether to allow printing.  It will enable/disable the menu/toolbar options.
	 *
	 *	@param enable Set whether to allow printing.
	 */
	void enableRedo(bool enable);

	/**
	 *      Returns a pointer to the documentation window.
	 */
	DocWindow * getDocWindow() {
		return m_pDocWindow;
	}

	/**
	 *	Returns the undo state
	 */
	bool getUndoEnabled();

	/**
	 *	Returns the redo state
	 */
	bool getRedoEnabled();

	/**
	 *	Returns the paste state
	 */
	bool getPasteState();

	/**
	 *	Returns the state on Cut/Copy
	 */
	bool getCutCopyState();

	/**
	 *	gets the appropiate CodeGenerator
	 */
	CodeGenerator* generator();

	/**
	 *	Sets the state of the view properties menu item.
	 */
	void setDiagramMenuItemsState(bool bState);

	/**
	 * Returns the widget used as the parent for UMLViews
	 */
	QWidget* getMainViewWidget();

	/**
	 * Puts this view to the top of the viewStack, i.e. makes it
	 * visible to the user.  If no view is specified the blank
	 * infoWidget is shown instead.
	 */
	void setCurrentView(UMLView* view = 0);

protected:
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void keyReleaseEvent(QKeyEvent* e);
	/**
	 *	Carries out the cut/copy command with different action performed
	 *	depending on if from view or list view.
	 *	Cut/Copy are the same.  It is up to the caller to delete/cut the selection..
	 *
	 *	If the operation is successful, the signal sigCutSuccessful() is emitted.
	 *
	 *	Callers should connect to this signal to know what to do next.
	 */
	bool editCutCopy( bool bFromView );

	/**
	 *	save general Options like all bar positions and status
	 *	as well as the geometry and the recent file list to
	 *	the configuration file.
	 */
	void saveOptions();

	/**
	 * read general Options again and initialize all variables
	 * like the recent file list
	 */
	void readOptions();

	/**
	 * initializes the KActions of the application
	 */
	void initActions();

	/**
	 *	sets up the statusbar for the main window by
	 *	initialzing a statuslabel.
	 */
	void initStatusBar();

	/**
	 *	initializes the document object of the main window
	 *	that is connected to the view in initView().
	 *      @see initView();
	 */
	void initDocument();

	/**
	 *	creates the centerwidget of the KTMainWindow instance and sets it as the view
	 */
	void initView();

	/**
	 * queryClose is called by KTMainWindow on each closeEvent of a window. Against the
	 * default implementation (only returns true), this calles saveModified() on the document object to ask if the document shall
	 * be saved if Modified; on cancel the closeEvent is rejected.
	 * @see KTMainWindow#queryClose
	 * @see KTMainWindow#closeEvent
	 */
	virtual bool queryClose();

	/**
	 *	queryExit is called by KTMainWindow when the last
	 *	window of the application is going to be closed during
	 *	the closeEvent().  Against the default
	 *	implementation that just returns true, this calls
	 *	saveOptions() to save the settings of the last
	 *	window's properties.
	 * @see KTMainWindow#queryExit
	 * @see KTMainWindow#closeEvent
	 */
	virtual bool queryExit();

	/**
	 *	saves the window properties for each open window
	 *	during session end to the session config file,
	 *	including saving the currently opened file by a
	 *	temporary filename provided by KApplication.
	 * @see KTMainWindow#saveProperties
	 */
	virtual void saveProperties(KConfig *_cfg);

	/** reads the session config file and restores the
	 * application's state including the last opened files and
	 * documents by reading the temporary files saved by
	 * saveProperties()
	 * @see KTMainWindow#readProperties
	 */
	virtual void readProperties(KConfig *_cfg);
protected slots:

	/** Updates the Menu for language selection and sets the
	 * active lanugage. If no active lanugage is found or if it is
	 * not one of the registered languages it tries to fall back
	 * to Cpp
	 */
	void updateLangSelectMenu();

public slots:

	/** Sets up information for dinamically loaded libraries. It
	 *  scans directories looking for new libraries and registers
	 *  them, and deregister the libraries that are not found
	 */
	void initLibraries();

	/**
	 * Runs the code generation wizard
	 */
	void generationWizard();

	/**
	 * clears the document in the actual view to reuse it as the new document
	 */
	void slotFileNew();

	/**
	 * open a file and load it into the document
	 */
	void slotFileOpen();

	/**
	 * opens a file from the recent files menu
	 */
	void slotFileOpenRecent(const KURL& url);

	/**
	 * save a document
	 */
	void slotFileSave();

	/**
	 * save a document by a new filename
	 */
	bool slotFileSaveAs();

	/**
	 * asks for saving if the file is modified, then closes the actual file and window
	 */
	void slotFileClose();

	/**
	 * print the actual file
	 */
	void slotFilePrint();

	/**
	 * closes all open windows by calling close() on each
	 * memberList item until the list is empty, then quits the
	 * application.  If queryClose() returns false because the
	 * user canceled the saveModified() dialog, the closing
	 * breaks.
	 */
	void slotFileQuit();

	/**
	 * put the marked text/object into the clipboard and remove
	 *	it from the document
	 */
	void slotEditCut();

	/**
	 * put the marked text/object into the clipboard
	 */
	void slotEditCopy();

	/**
	 * paste the clipboard into the document
	 */
	void slotEditPaste();

	/**
	 * toggles the toolbar
	 */
	void slotViewToolBar();

	/**
	 * toggles the statusbar
	 */
	void slotViewStatusBar();

	/**
	 * changes the statusbar contents for the standard label
	 * permanently, used to indicate current actions.
	 * @param text the text that is displayed in the statusbar
	 */
	void slotStatusMsg(const QString &text);

	/**
	* 	Create this view
	*/
	void slotClassDiagram();

	/**
	*	 Create this view
	*/
	void slotSequenceDiagram();

	/**
	*	 Create this view
	*/
	void slotCollaborationDiagram();

	/**
	*	Create this view
	*/
	void slotUseCaseDiagram();

	/**
	*	Create this view
	*/
	void slotStateDiagram();

	/**
	*	Create this view
	*/
	void slotActivityDiagram();

	/**
	 *
	 */
	void slotClipDataChanged();

	/**
	 *
	 */
	void slotCopyChanged();

	/**
	 * 	Shows the global preferences dialog.
	 */
	void slotPrefs();

	/**
	 * 	Commits the changes from the global preferences dialog.
	 */
	void slotApplyPrefs();

	/**
	 *	register new views (aka diagram) with the GUI so they show up in the menu
	*/
	void slotUpdateViews();

	/**
	*	generate code for all classes
	*/
	void generateAllCode();

	/**
	*	set the language for which code will be generated
	*	@param menuID the ID of the langSelect menu item for 
	*			the relevant language
	*/
	void setActiveLanguage(int menuID);

	/**
	*	set the language for which code will be generated
	*	@param  activeLanguage  The name of the language to set
	*/
	void setActiveLanguage(QString activeLanguage);

	/**
	*	shows a dialog to add or remove Generators (languages)
	*/
	void configureLanguages();

	/**
	*	Menu selection for clear current view.
	*/
	void slotCurrentViewClearDiagram();

	/**
	*	Menu selection for current view snap to grid property.
	*/
	void slotCurrentViewToggleSnapToGrid();

	/**
	*	Menu selection for current view show grid property.
	*/
	void slotCurrentViewToggleShowGrid();

	/**
	*	Menu selection for exporting current view as an image.
	*/
	void slotCurrentViewExportImage();

	/**
	*	Menu selection for current view properties.
	*/
	void slotCurrentViewProperties();

	/**
	*	Import classes menu selection.
	*/
	void slotImportClasses();

	/**
	*	Class wizard menu selection.
	*/
	void slotClassWizard();

	/**
	 *  The displayed diagram has changed.
	 */
	void slotCurrentViewChanged();
	
	/**
	 * The snap to grid value has been changed.
	 */
	void slotSnapToGridToggled(bool gridOn);

	/**
	 * The show grid value has been changed.
	 */
	void slotShowGridToggled(bool gridOn);

	/**
	 * Select all widgets on the current diagram
	 */
	void slotSelectAll();

	/**
	 * Deletes the selected widget
	 */
	void slotDeleteSelectedWidget();

	/**
	 * Deletes the current diagram
	 */
	void slotDeleteDiagram();

	/**
	* zooms in the current diagram
	*/
	void slotZoomIn();
	/**
	* zooms out of the current diagram
	*/
	void slotZoomOut();
	
	/**
	* set the zoom factor of the current diagram
	* the parameter is in percentage
	*/
	void setZoom(int z);
	
	/**
	* prepares the zoom menu for display
	*/
	void setupZoomMenu();

	/**
	 * Reverts the document back to the state it was prior to the
	 * last action performed by the user.
	 */
	void slotEditUndo();

	/**
	 * Reverts the document back to the state it was prior to the
	 * last undo.
	 */
	void slotEditRedo();

	/**
	 * Searches for a menu with the given name
	 *
	 * @param menu the QPopupMenu or QMenuBar to search through
	 * @param name the name of the menu to search for (name, not text)
	 */
	QPopupMenu* findMenu(QMenuData* menu, QString name);
private:
	/**
	 * to slect the active language
	 */
	QPopupMenu *langSelect;
	
	/**
	* Popup menu for zoom selection
	*/
	QPopupMenu *zoomSelect;

	/**
	 *	active language
	 */
	QString activeLanguage;

	/**
	 *	dictionary containing all the info needed to load a Code Generator at run
	 *	time: Language (so that the user can choose), library and object name
	 */
	QDict<GeneratorInfo> ldict;

	/**
	 *		Reads from the config file the options state.
	 *		Not in @ref readOptions as it needs to be read earlier than some of
	 *		the other options, before some items are created.
	 */
	void readOptionState();

	/**
	 *		initialize the QT's global clipboard support for the application
	 */
	void initClip();

	/**
	 * the configuration object of the application
	 */
	KConfig *config;

	/**
	 * view is the main widget which represents your working
	 * area. The View class should handle all events of the view
	 * widget.  It is kept empty so you can create your view
	 * according to your application's needs by changing the view
	 * class.
	 */
	UMLView *view;

	/**
	 * doc represents your actual document and is created only once. It keeps
	 * information such as filename and does the serialization of your files.
	 */
	UMLDoc *doc;

	/**
	 *	listview shows the current open file
	 */
	UMLListView * listView;

	/**
	 * 	The widget which shows the diagrams
	 */
	KDockWidget* m_mainDock;

	/**
	 * Contains the UMLListView tree view.
	 */
	KDockWidget* m_listDock;

	/**
	 * Contains the documentation DocWindow widget
	 */
	KDockWidget* m_documentationDock;

	/**
	 * 	Documentation window.
	 */
	DocWindow* m_pDocWindow;

	//KAction pointers to enable/disable actions
	KAction* fileNew;
	KAction* fileOpen;
	KRecentFilesAction* fileOpenRecent;
	KAction* fileSave;

	KAction* fileSaveAs;
	KAction* fileClose;
	KAction* filePrint;
	KAction* fileQuit;
	KAction* editCut;
	KAction* editCopy;
	KAction* editPaste;
	KAction* editUndo;
	KAction* editRedo;
	KAction* selectAll;
	KAction* preferences;
	
	KActionMenu* newDiagram;
	KAction* classDiagram;
	KAction* sequenceDiagram;
	KAction* collaborationDiagram;
	KAction* useCaseDiagram;
	KAction* stateDiagram;
	KAction* activityDiagram;
	KAction* viewClearDiagram;
	
	KToggleAction* viewSnapToGrid;
	KToggleAction* viewShowGrid;
	KAction* viewExportImage;
	KAction* viewProperties;
	
	KAction *zoomInAction;
	KAction *zoomOutAction;

	KAction* genAll;
	KAction* genWizard;
	KAction* confLanguages;
	KAction* importClasses;
	KAction* classWizard;
	KAction* deleteSelectedWidget;
	KAction* deleteDiagram;
	KToggleAction* viewToolBar;
	KToggleAction* viewStatusBar;
	WorkToolBar* toolsbar;
	QTimer* m_clipTimer;
	QTimer* m_copyTimer;

	/**  */
	KStatusBarLabel * m_statusLabel;
	/**  */
	QGuardedPtr<QProgressBar> statProg;
	/**
	 * True if the application is opening an existing document
	 */
	bool loading;
	SettingsDlg::OptionState optionState;

	/**
	 * Blank widget, displayed when there are no diagrams
	 */
	InfoWidget* blankWidget;

	/**
	 * Shows, and is parent of, all the UMLViews (diagrams).
	 */
	QWidgetStack* viewStack;

public:
	/**
	 * the global UML settings dialogue
	 */
	SettingsDlg* dlg;
	SettingsDlg::OptionState getOptionState() {
		return optionState;
	}

signals:

	/**
	 * 	Emitted when a cut operation is successful.
	 */
	void sigCutSuccessful();
};

#endif // UML_H
