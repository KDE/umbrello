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

#ifndef UML_H
#define UML_H

#include <config.h>

#include "umlnamespace.h"

#include <qmap.h>
#include <q3dict.h>
#include <q3progressbar.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QMenu>
#include <QUndoView>

#include <kmainwindow.h>
#include <kdeversion.h>
#include <kurl.h>
#include <kconfig.h>
#include <kundostack.h>

// forward declaration of the UML classes
class AlignToolBar;
class CodeDocument;
class CodeGenerator;
class CodeGenerationPolicy;
class CodeGenPolicyExt;
class DocWindow;
class UMLClassifier;
class UMLDoc;
class UMLListView;
class UMLView;
class WorkToolBar;
class SettingsDlg;
class UMLViewImageExporterAll;
class RefactoringAssistant;
class KPlayerPopupSliderAction;
class XhtmlGenerator;

// KDE forward declarations
class KActionMenu;
class KRecentFilesAction;
class KStatusBarLabel;
class KToggleAction;
class KTabWidget;
class KMenu;
class KMenuBar;

// Qt forward declarations
class QStackedWidget;
class QMenuData;
class QClipboard;
class QToolButton;
class QCustomEvent;
class QDockWidget;

/**
 * The base class for UML application windows. It sets up the main
 * window and reads the config file as well as providing a menubar, toolbar
 * and statusbar. An instance of UMLView creates your center view, which is connected
 * to the window's Doc object.
 * UMLApp reimplements the methods that KMainWindow provides for main window handling and supports
 * full session management as well as using KActions.
 * @see KMainWindow
 * @see KApplication
 * @see KConfig
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLApp : public KMainWindow {
    Q_OBJECT
public:
    /**
     * Constructor. Calls all init functions to create the application.
     */
    UMLApp(QWidget* parent=0);

    /**
     * Standard deconstructor.
     */
    ~UMLApp();

    static UMLApp* app();

    /**
     * Opens a file specified by commandline option.
     */
    void openDocumentFile(const KUrl& url=KUrl());

    /**
     * Calls the UMLDoc method to create a new Document.
     */
    void newDocument();

    /**
     * Returns a pointer to the current document connected to the
     * KMainWindow instance.
     * Used by the View class to access the document object's methods.
     */
    UMLDoc *getDocument() const;

    /**
     * Returns a pointer to the list view.
     *
     * @return  The listview being used.
     */
    UMLListView* getListView();

    /**
     * Returns the toolbar being used.
     *
     * @return  The toolbar being used.
     */
    WorkToolBar* getWorkToolBar();

    /**
     * Sets whether the program has been modified.
     * This will change how the program saves/exits.
     *
     * @param _m        true - modified.
     */
    void setModified(bool _m);

    /**
     * Set whether to allow printing.
     * It will enable/disable the menu/toolbar options.
     *
     * @param enable    Set whether to allow printing.
     */
    void enablePrint(bool enable);

    /**
     * Set whether to allow printing.
     * It will enable/disable the menu/toolbar options.
     *
     * @param enable    Set whether to allow printing.
     */
    void enableUndo(bool enable);

    /**
     * Set whether to allow printing.
     * It will enable/disable the menu/toolbar options.
     *
     * @param enable    Set whether to allow printing.
     */
    void enableRedo(bool enable);

    /**
     * Returns a pointer to the documentation window.
     *
     * @return  Pointer to the DocWindow.
     */
    DocWindow * getDocWindow() {
        return m_pDocWindow;
    }

    /**
     * Returns the undo state.
     *
     * @return  True if Undo is enabled.
     */
    bool getUndoEnabled();

    /**
     * Returns the redo state.
     *
     * @return  True if Redo is enabled.
     */
    bool getRedoEnabled();

    /**
     * Returns the paste state.
     *
     * @return  True if Paste is enabled.
     */
    bool getPasteState();

    /**
     * Returns the state on Cut/Copy.
     *
     * @return  True if Cut/Copy is enabled.
     */
    bool getCutCopyState();

    /**
     * Gets the appropriate CodeGenerator.
     *
     * @return  Pointer to the CodeGenerator.
     */
    CodeGenerator* getGenerator();

    /**
     * Set the current generator for this app.
     * If giveWarning is true, then a popup box warning that the
     * code generation library is out-of-date will show if you
     * attempt to set the generator to NULL.
     *
     * @param gen               Pointer to the CodeGenerator to set.
     * @param giveWarning       True to enable out-of-date warning.
     */
    void setGenerator(CodeGenerator* gen, bool giveWarning = true);

    /**
     * Creates a new code generator for the given active language.
     *
     * @return  Pointer to the CodeGenerator created.
     */
    CodeGenerator* createGenerator();

    /**
     * Auxiliary function for UMLDoc::loadExtensionsFromXMI():
     * Return the code generator of the given language if it already
     * exists; if it does not yet exist then create it and return
     * the newly created generator. It is the caller's responsibility
     * to load XMI into the newly created generator.
     */
    CodeGenerator *setGenerator(Uml::Programming_Language pl);

    /**
     * Call the refactoring assistant on a classifier.
     *
     * @param   Pointer to the classifier to refactor.
     */
    void refactor(UMLClassifier* classifier);

    /**
     * Call the code viewing assistant on a given UMLClassifier.
     *
     * @param   Pointer to the classifier to view.
     */
    void viewCodeDocument(UMLClassifier* classifier);

    /**
     * Sets the state of the view properties menu item.
     *
     * @param   Boolean, true to enable the view properties item.
     */
    void setDiagramMenuItemsState(bool bState);

    /**
     * Returns the widget used as the parent for UMLViews.
     * @return  The main view widget.
     */
    QWidget* getMainViewWidget();

    /**
     * Puts this view to the top of the viewStack, i.e. makes it
     * visible to the user.
     *
     * @param view              Pointer to the UMLView to push.
     */
    void setCurrentView(UMLView* view);

    /**
     * Get the current view.
     * This may return a null pointer (when no view was previously
     * specified.)
     *
     */
    UMLView* getCurrentView();

    /**
     * Sets the default mime type for all diagrams that are exported as
     * images.
     *
     * @param mimeType  The MIME type to set as the default.
     */
    void setImageMimeType(QString const & mimeType){m_imageMimeType=mimeType;};

    /**
     * Gets the default mime type for all diagrams that are exported as
     * images.
     *
     * @return  The default MIME type for images.
     */
    QString const & getImageMimeType()const{return m_imageMimeType;};

    /**
     * Carries out the cut/copy command with different action performed
     * depending on if from view or list view.
     * Cut/Copy are the same.  It is up to the caller to delete/cut the selection..
     *
     * If the operation is successful, the signal sigCutSuccessful() is emitted.
     *
     * Callers should connect to this signal to know what to do next.
     */
    bool editCutCopy( bool bFromView );

    /**
     * Return the tab widget.
     */
    KTabWidget *tabWidget();

    /**
     * Returns the current text in the status bar.
     *
     * @return The text in the status bar.
     */
    QString getStatusBarMsg();

    /**
     * Returns the default code generation policy.
     */
    CodeGenerationPolicy *getCommonPolicy();

    /**
     * Sets the CodeGenPolicyExt object.
     */
    void setPolicyExt(CodeGenPolicyExt *policy);

    /**
     * Returns the CodeGenPolicyExt object.
     */
    CodeGenPolicyExt *getPolicyExt();

    /**
     * Removes all entries from the UndoStack and RedoStack and disables the
     * undo and redo actions.
     */
    void clearUndoStack();

    /**
     * Undo last command
    */
    void undo();

    /**
     * Redo last 'undoed' command
    */
    void redo();

    /**
     * Execute a command and pushit in the stack.
    */
    void executeCommand(QUndoCommand* cmd);

    /**
     * Begin a U/R command macro
    */
    void BeginMacro( const QString & text );

    /**
     * End an U/R command macro
    */
    void EndMacro();

protected:
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void keyReleaseEvent(QKeyEvent* e);

    /**
     * Event handler to receive custom events.
     * It handles events such as exporting all views from command line (in
     * that case, it executes the exportAllViews method in the event).
     */
    virtual void customEvent(QCustomEvent* e);

    /**
     * Helper method for handling cursor key release events (refactoring).
     */
    void handleCursorKeyReleaseEvent(QKeyEvent* e);

    /**
     * Save general Options like all bar positions and status
     * as well as the geometry and the recent file list to
     * the configuration file.
     */
    void saveOptions();

    /**
     * Read general Options again and initialize all variables
     * like the recent file list.
     */
    void readOptions();

    /**
     * Initializes the KActions of the application.
     */
    void initActions();

    /**
     * Sets up the statusbar for the main window by
     * initialzing a statuslabel.
     */
    void initStatusBar();

    /**
     * Creates the centerwidget of the KMainWindow instance and
     * sets it as the view.
     */
    void initView();

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
    virtual bool queryClose();

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
    virtual bool queryExit();

    /**
     * Saves the window properties for each open window
     * during session end to the session config file,
     * including saving the currently opened file by a
     * temporary filename provided by KApplication.
     * @see KMainWindow#saveProperties
     */
    virtual void saveProperties(KConfig *_cfg);

    /**
     * Reads the session config file and restores the
     * application's state including the last opened files and
     * documents by reading the temporary files saved by
     * saveProperties()
     * @see KMainWindow#readProperties
     */
    virtual void readProperties(KConfig *_cfg);

    CodeGenerationPolicy * m_commoncodegenpolicy;

    /**
     * Updates the Menu for language selection and sets the
     * active lanugage. If no active lanugage is found or if it is
     * not one of the registered languages it tries to fall back
     * to Cpp
     */
    void updateLangSelectMenu(Uml::Programming_Language activeLanguage);

    void setProgLangMenu(Uml::Programming_Language pl);

protected slots:

    /**
     * Show "Tip of the Day" dialog
     */
    void tipOfTheDay();

public slots:

    /**
     * Reads the activeLanguage from the KConfig and calls updateLangSelectMenu()
     */
    void initGenerator();

    /**
     * Runs the code generation wizard.
     */
    void generationWizard();

    /**
     * Clears the document in the actual view to reuse it as the new
     * document.
     */
    void slotFileNew();

    /**
     * Open a file and load it into the document.
     */
    void slotFileOpen();

    /**
     * Opens a file from the recent files menu.
     */
    void slotFileOpenRecent(const KUrl& url);

    /**
     * Save a document.
     */
    void slotFileSave();

    /**
     * Save a document by a new filename.
     */
    bool slotFileSaveAs();

    /**
     * Asks for saving if the file is modified, then closes the current
     * file and window.
     */
    void slotFileClose();

    /**
     * Print the current file.
     */
    void slotFilePrint();

    /**
     * Closes all open windows by calling close() on each
     * memberList item until the list is empty, then quits the
     * application.  If queryClose() returns false because the
     * user canceled the saveModified() dialog, the closing
     * aborts.
     */
    void slotFileQuit();

     /**
     * Exports the current model to docbook in a subdir of the
     * current model directory named from the model name.
     * @todo Let the user chose the destination directory and
     * name, using network transparency.
     */
    void slotFileExportDocbook();

    /**
     * Exports the current model to XHTML in a subdir of the
     * current model directory named from the model name.
     * @todo Let the user chose the destination directory and
     * name, using network transparency.
     */
    void slotFileExportXhtml();

    /**
     * Put the marked text/object into the clipboard and remove
     * it from the document.
     */
    void slotEditCut();

    /**
     * Put the marked text/object into the clipboard.
     */
    void slotEditCopy();

    /**
     * Paste the clipboard into the document.
     */
    void slotEditPaste();

    // #if !KDE_IS_VERSION(3,1,90)
    /**
     * Toggles the toolbar.
     * Deprecated. For compatibility with KDE 3.1, remove if we stop supporting KDE 3.1
     */
    void slotViewToolBar();

    /**
     * Toggles the statusbar.
     * Deprecated. For compatibility with KDE 3.1, remove if we stop supporting KDE 3.1
     */
    void slotViewStatusBar();
    // #endif

    /**
     * Changes the statusbar contents for the standard label
     * permanently, used to indicate current actions.
     * @param text      The text that is displayed in the statusbar
     */
    void slotStatusMsg(const QString &text);

    /**
     * Create this view.
     */
    void slotClassDiagram();

    /**
     * Create this view.
     */
    void slotSequenceDiagram();

    /**
     * Create this view.
     */
    void slotCollaborationDiagram();

    /**
     * Create this view.
     */
    void slotUseCaseDiagram();

    /**
     * Create this view.
     */
    void slotStateDiagram();

    /**
     * Create this view.
     */
    void slotActivityDiagram();

    /**
     * Create this view.
     */
    void slotComponentDiagram();

    /**
     * Create this view.
     */
    void slotDeploymentDiagram();
    /**
     * Create this view.
     */
    void slotEntityRelationshipDiagram();

    /**
     * Notification of changed clipboard data.
     */
    void slotClipDataChanged();

    /**
     *
     */
    void slotCopyChanged();

    /**
     * Shows the global preferences dialog.
     */
    void slotPrefs();

    /**
     * Commits the changes from the global preferences dialog.
     */
    void slotApplyPrefs();

    /**
     * Register new views (aka diagram) with the GUI so they show up
     * in the menu.
     */
    void slotUpdateViews();

    /**
     * Generate code for all classes.
     */
    void generateAllCode();

    /**
     * Slots for connection to the QActions of the m_langSelect menu.
     */
    void set_lang_actionscript();
    void set_lang_ada();
    void set_lang_cpp();
    // void set_lang_csharp();
    void set_lang_idl();
    void set_lang_java();
    void set_lang_javascript();
    void set_lang_pascal();
    void set_lang_perl();
    void set_lang_php();
    void set_lang_php5();
    void set_lang_python();
    void set_lang_ruby();
    void set_lang_sql();
    void set_lang_tcl();
    void set_lang_xmlschema();
    void set_lang_ocl();

    /**
     * Set the language for which code will be generated.
     *
     * @param pl    The name of the language to set
     */
    void setActiveLanguage(Uml::Programming_Language pl);

    /**
     * Get the language for import and code generation.
     */
    Uml::Programming_Language getActiveLanguage();

    /**
     * Return true if the active language is case sensitive.
     */
    bool activeLanguageIsCaseSensitive();

    /**
     * Return the target language depedent scope separator.
     */
    QString activeLanguageScopeSeparator();

    /**
     * Return the default code generation language as configured by KConfig.
     * If the activeLanguage is not found in the KConfig then use Uml::pl_Cpp
     * as the default.
     */
    Uml::Programming_Language getDefaultLanguage();

    /**
     * Menu selection for clear current view.
     */
    void slotCurrentViewClearDiagram();

    /**
     * Menu selection for current view snap to grid property.
     */
    void slotCurrentViewToggleSnapToGrid();

    /**
     * Menu selection for current view show grid property.
     */
    void slotCurrentViewToggleShowGrid();

    /**
     * Menu selection for exporting current view as an image.
     */
    void slotCurrentViewExportImage();

    /**
     * Menu selection for exporting all views as images.
     */
    void slotAllViewsExportImage();

    /**
     * Menu selection for current view properties.
     */
    void slotCurrentViewProperties();

    /**
     * Import classes menu selection.
     */
    void slotImportClasses();

    /**
     * Class wizard menu selection.
     */
    void slotClassWizard();

    /**
     * Calls the active code generator to add its default datatypes
     */
    void slotAddDefaultDatatypes();

    /**
     * The displayed diagram has changed.
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
     * Select all widgets on the current diagram.
     */
    void slotSelectAll();

    /**
     * Deletes the selected widget.
     */
    void slotDeleteSelectedWidget();

    /**
     * Deletes the current diagram.
     */
    void slotDeleteDiagram();

    /**
     * Set the zoom factor of the current diagram.
     *
     * @param zoom              Zoom factor in percentage.
     */
    void setZoom(int zoom);

    /**
     * Connected to by the KPlayerSliderAction zoomAction, a value of between 300
     * and 2200 is scaled to zoom to between 9% and 525%.
     * The min and max values of the slider are hard coded in KPlayerSliderAction for now.
     */
    void slotZoomSliderMoved();

    /**
     * Set zoom to 100%
     */
    void slotZoom100();

    /**
     * Prepares the zoom menu for display.
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
     * @param menu      The QPopupMenu or QMenuBar to search through.
     * @param name      The name of the menu to search for (name, not text)
     */
    QMenu* findMenu(QMenu* menu, const QString &name);

    /// @todo This is an ugly _HACK_ to allow to compile umbrello.
    /// All the menu stuff should be ported to KDE4 (using actions)
    QMenu* findMenu(KMenuBar* menu, const QString &name);

    /**
     * called when the tab has changed
     */
    void slotTabChanged(QWidget* view);

    /**
     * make the tab on the left of the current one the active one
     */
    void slotChangeTabLeft();

    /**
     * make the tab on the right of the current one the active one
     */
    void slotChangeTabRight();

    /**
     * Move the current tab left, not implemented
     */
    void slotMoveTabLeft();

    /**
     * Move the current tab right, not implemented
     */
    void slotMoveTabRight();

    KConfig* getConfig() { return m_config.data(); }

    /**
     * This slot deletes the current XHTML documentation generator as soon as
     * this one signals that it has finished.
     */
    void slotXhtmlDocGenerationFinished();

private:
    static UMLApp* s_instance;

    /**
     * For selecting the active language.
     */
    QMenu *m_langSelect;

    /**
    * Popup menu for zoom selection.
    */
    QMenu *m_zoomSelect;

    /**
     * Active language.
     */
    Uml::Programming_Language m_activeLanguage;

    /**
     * Active code generator.
     */
    CodeGenerator *m_codegen;

    /**
     * Active policy extension.
     * Only used for new code generators ({Cpp,Java,Ruby}CodeGenerator)
     */
    CodeGenPolicyExt *m_policyext;

    /**
     *  Returns whether we can decode the given mimesource
     */
    static bool canDecode(const QMimeSource* mimeSource);

    /**
     * Reads from the config file the options state.
     * Not in @ref readOptions as it needs to be read earlier than some
     * of the other options, before some items are created.
     */
    void readOptionState();

    /**
     * Initialize Qt's global clipboard support for the application.
     */
    void initClip();

    /**
     * Initialize code generators at startup.
     * Why is this important? Because IF we don't do this, then changes
     * to the UML may not be synced with the saved code generation params
     * for those languages which arent currently active.
     */
    void initSavedCodeGenerators();

    /**
     * The configuration object of the application.
     */
    KSharedConfigPtr m_config;

    /**
     * View is the main widget which represents your working area.
     * The View class should handle all events of the view widget.
     * It is kept empty so you can create your view according to your
     * application's needs by changing the view class.
     */
    UMLView* m_view;

    /**
     * doc represents your actual document and is created only once.
     * It keeps information such as filename and does the loading and
     * saving of your files.
     */
    UMLDoc* m_doc;

    /**
     * Listview shows the current open file.
     */
    UMLListView* m_listView;

    /**
     * The widget which shows the diagrams.
     */
    QDockWidget* m_mainDock;

    /**
     * Contains the UMLListView tree view.
     */
    QDockWidget* m_listDock;

    /**
     * Contains the documentation DocWindow widget.
     */
    QDockWidget* m_documentationDock;

    /**
     * Contains the undo/redo viewer widget.
     */
    QDockWidget* m_cmdHistoryDock;

    /**
     * Contains the property browser widget
    */
    QDockWidget* m_propertyDock;
    
    /**
     * Documentation window.
     */
    DocWindow* m_pDocWindow;

    /**
     * Undo / Redo Viewer
    */
    QUndoView* m_pQUndoView;

    /** Refactoring assistant. */
    RefactoringAssistant* m_refactoringAssist;

    //KAction pointers to enable/disable actions
    QAction* fileNew;
    QAction* fileOpen;
    KRecentFilesAction* fileOpenRecent;
    QAction* fileSave;
    QAction* fileSaveAs;
    QAction* fileClose;
    QAction* filePrint;
    QAction* fileQuit;
    QAction* fileExportDocbook;
    QAction* fileExportXhtml;

    QAction* editCut;
    QAction* editCopy;
    QAction* editPaste;
    QAction* editUndo;
    QAction* editRedo;
    QAction* selectAll;
    QAction* preferences;

    KActionMenu* newDiagram;
    QAction* classDiagram;
    QAction* sequenceDiagram;
    QAction* collaborationDiagram;
    QAction* useCaseDiagram;
    QAction* stateDiagram;
    QAction* activityDiagram;
    QAction* componentDiagram;
    QAction* deploymentDiagram;
    QAction* entityRelationshipDiagram;
    QAction* viewClearDiagram;

    KToggleAction* viewSnapToGrid;
    KToggleAction* viewShowGrid;
    QAction* viewExportImage;
    QAction* viewExportImageAll;
    QAction* viewProperties;

    QAction* zoom100Action;
    KPlayerPopupSliderAction* zoomAction;

    QAction* genAll;
    QAction* genWizard;
    QAction* importClasses;
    QAction* classWizard;
    QAction* m_langAct[Uml::pl_Reserved];
    QAction* deleteSelectedWidget;
    QAction* deleteDiagram;
#ifdef HAVE_DOT
    QAction* autolayout;
#endif

    QAction* changeTabLeft;
    QAction* changeTabRight;
    QAction* moveTabLeft;
    QAction* moveTabRight;
    QToolButton* m_newSessionButton;
    KMenu* m_diagramMenu;
    QToolButton* m_closeDiagramButton;
    KToggleAction* viewToolBar;
    KToggleAction* viewStatusBar;
    WorkToolBar* toolsbar;
    QTimer* m_clipTimer;
    QTimer* m_copyTimer;
    AlignToolBar* m_alignToolBar;

    KStatusBarLabel* m_statusLabel;

    /**
     * True if the application is opening an existing document
     */
    bool m_loading;

    /**
     * Shows, and is parent of, all the UMLViews (diagrams)
     * if tabbed diagrams are not enabled.
     */
    QStackedWidget* m_viewStack;

    /**
     * Shows, and is parent of, all the UMLViews (diagrams)
     * if tabbed diagrams are enabled.
     */
    KTabWidget* m_tabWidget;

    /**
     * Default mime type to use for image export.
     */
    QString m_imageMimeType;

    /**
     * the global UML settings dialog
     */
    SettingsDlg* m_dlg;

    /**
     * The UMLViewImageExporterAll used to export all the views.
     */
    UMLViewImageExporterAll* m_imageExporterAll;

    /**
     * The running XHTML documentation generator. null when no generation is
     * running
     */
    XhtmlGenerator* m_xhtmlGenerator;

    /**
     * UndoStack
     * used to store actions, to provide Undo/Redo feature.
    */
    KUndoStack*	m_pUndoStack;

    /**
     * Macro creation flag
    */
    bool m_hasBegunMacro;

signals:

    /**
     * Emitted when a cut operation is successful.
     */
    void sigCutSuccessful();
};

#endif // UML_H
