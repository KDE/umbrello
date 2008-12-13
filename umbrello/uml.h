/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UML_H
#define UML_H

#include "umlnamespace.h"

#include <kxmlguiwindow.h>
#include <kurl.h>
#include <ksharedconfig.h>

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
class KToggleAction;
class KTabWidget;
class KMenu;
class KMenuBar;
class KUndoStack;
class KAction;

// Qt forward declarations
class QStackedWidget;
class QToolButton;
class QCustomEvent;
class QDockWidget;
class QVBoxLayout;
class QKeyEvent;
class QMenu;
class QMimeData;
class QUndoCommand;
class QUndoView;

/**
 * The base class for UML application windows. It sets up the main
 * window and reads the config file as well as providing a menubar, toolbar
 * and statusbar. A list of UMLView instances creates the center views, which are connected
 * to the window's Doc object. The handling of views is realized with two different widgets:
 *   - stack widget
 *   - tab widget
 * The current view handling is set as an option.
 * UMLApp reimplements the methods that KMainWindow provides for main window handling and supports
 * full session management as well as using KActions.
 *
 * @see KMainWindow
 * @see KApplication
 * @see KConfig
 *
 * @author Paul Hensgen <phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLApp : public KXmlGuiWindow
{
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

    void openDocumentFile(const KUrl& url=KUrl());

    void newDocument();

    UMLDoc *getDocument() const;

    UMLListView* getListView();

    WorkToolBar* getWorkToolBar();

    void setModified(bool _m);

    void enablePrint(bool enable);

    void enableUndo(bool enable);

    void enableRedo(bool enable);

    DocWindow * getDocWindow() {
        return m_pDocWindow;
    }

    bool getUndoEnabled();

    bool getRedoEnabled();

    bool getPasteState();

    bool getCutCopyState();

    CodeGenerator* getGenerator();

    bool isSimpleCodeGeneratorActive();

    void setGenerator(CodeGenerator* gen, bool giveWarning = true);

    CodeGenerator* createGenerator();

    CodeGenerator *setGenerator(Uml::Programming_Language pl);

    void refactor(UMLClassifier* classifier);

    void viewCodeDocument(UMLClassifier* classifier);

    void setDiagramMenuItemsState(bool bState);

    QWidget* getMainViewWidget();

    void setCurrentView(UMLView* view);

    UMLView* getCurrentView();

    void setImageMimeType(QString const & mimeType){m_imageMimeType=mimeType;}

    /**
     * Gets the default mime type for all diagrams that are exported as
     * images.
     *
     * @return  The default MIME type for images.
     */
    QString const & getImageMimeType()const{return m_imageMimeType;}

    bool editCutCopy( bool bFromView );

    KTabWidget *tabWidget();

    QString getStatusBarMsg();

    CodeGenerationPolicy *getCommonPolicy();

    void setPolicyExt(CodeGenPolicyExt *policy);

    CodeGenPolicyExt *getPolicyExt();

    void clearUndoStack();

    void undo();

    void redo();

    void executeCommand(QUndoCommand* cmd);

    void BeginMacro( const QString & text );

    void EndMacro();

protected:
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void keyReleaseEvent(QKeyEvent* e);

    virtual void customEvent(QEvent* e);

    void handleCursorKeyReleaseEvent(QKeyEvent* e);

    void saveOptions();

    void readOptions();

    void initActions();

    void initStatusBar();

    void initView();

    virtual bool queryClose();

    virtual bool queryExit();

    virtual void saveProperties(KConfigGroup & cfg);

    virtual void readProperties(const KConfigGroup & cfg);

    CodeGenerationPolicy * m_commoncodegenpolicy;

    void updateLangSelectMenu(Uml::Programming_Language activeLanguage);

public slots:

    void initGenerator();

    void generationWizard();

    void slotFileNew();

    void slotFileOpen();

    void slotFileOpenRecent(const KUrl& url);

    void slotFileSave();

    bool slotFileSaveAs();

    void slotFileClose();

    void slotFilePrint();

    void slotFileQuit();

    void slotFileExportDocbook();

    void slotFileExportXhtml();

    void slotEditCut();

    void slotEditCopy();

    void slotEditPaste();

    void slotStatusMsg(const QString &text);

    void slotClassDiagram();

    void slotSequenceDiagram();

    void slotCollaborationDiagram();

    void slotUseCaseDiagram();

    void slotStateDiagram();

    void slotActivityDiagram();

    void slotComponentDiagram();

    void slotDeploymentDiagram();

    void slotEntityRelationshipDiagram();

    void slotAlignLeft();

    void slotAlignRight();

    void slotAlignTop();

    void slotAlignBottom();

    void slotAlignVerticalMiddle();

    void slotAlignHorizontalMiddle();

    void slotAlignVerticalDistribute();

    void slotAlignHorizontalDistribute();

    void slotClipDataChanged();

    void slotCopyChanged();

    void slotPrefs();

    void slotApplyPrefs();

    void slotUpdateViews();

    void generateAllCode();

    void setActiveLanguage(Uml::Programming_Language pl);

    Uml::Programming_Language getActiveLanguage();

    bool activeLanguageIsCaseSensitive();

    QString activeLanguageScopeSeparator();

    Uml::Programming_Language getDefaultLanguage();

    void slotCurrentViewClearDiagram();

    void slotCurrentViewToggleSnapToGrid();

    void slotCurrentViewToggleShowGrid();

    void slotCurrentViewExportImage();

    void slotAllViewsExportImage();

    void slotCurrentViewProperties();

    void slotImportClasses();

    void slotImportProject();

    void slotClassWizard();

    void slotAddDefaultDatatypes();

    void slotCurrentViewChanged();

    void slotSnapToGridToggled(bool gridOn);

    void slotShowGridToggled(bool gridOn);

    void slotSelectAll();

    void slotDeleteSelectedWidget();

    void slotDeleteDiagram(QWidget* tab = NULL);

    void setZoom(int zoom);

    void slotSetZoom(QAction* action);

    void slotZoomSliderMoved(int value);

    void slotZoom100();

    void setupZoomMenu();

    void slotEditUndo();

    void slotEditRedo();

    QMenu* findMenu(const QString &name);

    void slotTabChanged(QWidget* tab);

    void slotChangeTabLeft();

    void slotChangeTabRight();

    void slotMoveTabLeft();

    void slotMoveTabRight();

    KConfig* getConfig() { return m_config.data(); }

    void slotXhtmlDocGenerationFinished(bool status);

private slots:

    void setLang_actionscript();
    void setLang_ada();
    void setLang_cpp();
    void setLang_csharp();
    void setLang_d();
    void setLang_idl();
    void setLang_java();
    void setLang_javascript();
    void setLang_mysql();
    void setLang_pascal();
    void setLang_perl();
    void setLang_php();
    void setLang_php5();
    void setLang_postgresql();
    void setLang_python();
    void setLang_ruby();
    void setLang_sql();
    void setLang_tcl();
    void setLang_xmlschema();

private:

    /**
     * Variable for holding the last created instance of this class.
     */
    static UMLApp* s_instance;

    /**
     * For selecting the active language.
     */
    QMenu *m_langSelect;

    /**
     * Popup menu for zoom selection.
     */
    QMenu *m_zoomSelect;

    QAction* createZoomAction(int zoom, int currentZoom);

    void resetStatusMsg();

    void setProgLangAction(Uml::Programming_Language pl, const QString& name, const QString& action);

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
     * Only used for new code generators ({Cpp,Java,Ruby,D}CodeGenerator)
     */
    CodeGenPolicyExt *m_policyext;

    static bool canDecode(const QMimeData* mimeSource);

    void readOptionState();

    void initClip();

    void initSavedCodeGenerators();

    void importFiles(QStringList* fileList);

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

    // KAction pointers to enable/disable actions
    KRecentFilesAction* fileOpenRecent;
    QAction* filePrint;
    QAction* editCut;
    QAction* editCopy;
    QAction* editPaste;
    QAction* editUndo;
    QAction* editRedo;

    KActionMenu* newDiagram;
    QAction* viewClearDiagram;

    KToggleAction* viewSnapToGrid;
    KToggleAction* viewShowGrid;
    QAction* viewExportImage;
    QAction* viewProperties;

    QAction* zoom100Action;
    KPlayerPopupSliderAction* zoomAction;

    QAction* m_langAct[Uml::pl_Reserved];
    KAction* deleteSelectedWidget;
    KAction* deleteDiagram;

    QToolButton* m_newSessionButton;
    KMenu* m_diagramMenu;
    KToggleAction* viewToolBar;
    KToggleAction* viewStatusBar;
    WorkToolBar* toolsbar;
    QTimer* m_clipTimer;
    QTimer* m_copyTimer;
    AlignToolBar* m_alignToolBar;

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
     * Layout supports the dynamic management of the diagram representation (tabbed/stacked)
     * if tabbed diagrams is enabled it contains m_tabWidget
     * if tabbed diagrams is disabled it contains m_viewStack
     */
    QVBoxLayout* m_layout;

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
    KUndoStack* m_pUndoStack;

    /**
     * Macro creation flag
     */
    bool m_hasBegunMacro;

signals:

    void sigCutSuccessful();
};

#endif // UML_H
