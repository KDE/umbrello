/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
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
    UMLApp(QWidget* parent=0);
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

    DocWindow * getDocWindow() { return m_pDocWindow; }

    bool getUndoEnabled();
    bool getRedoEnabled();
    bool getPasteState();
    bool getCutCopyState();

    bool isSimpleCodeGeneratorActive();

    void setGenerator(CodeGenerator* gen, bool giveWarning = true);
    CodeGenerator* setGenerator(Uml::Programming_Language pl);
    CodeGenerator* getGenerator();

    CodeGenerator* createGenerator();

    void refactor(UMLClassifier* classifier);

    void viewCodeDocument(UMLClassifier* classifier);

    void setDiagramMenuItemsState(bool bState);

    QWidget* getMainViewWidget();

    void setCurrentView(UMLView* view);
    UMLView* getCurrentView();

    void setImageMimeType(QString const & mimeType);
    QString getImageMimeType() const;

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
    static UMLApp* s_instance;  ///< The last created instance of this class.

    QMenu* m_langSelect;  ///< For selecting the active language.
    QMenu* m_zoomSelect;  ///< Popup menu for zoom selection.

    QAction* createZoomAction(int zoom, int currentZoom);

    void resetStatusMsg();

    void setProgLangAction(Uml::Programming_Language pl, const QString& name, const QString& action);

    Uml::Programming_Language m_activeLanguage;  ///< Active language.
    CodeGenerator*            m_codegen;         ///< Active code generator.
    CodeGenPolicyExt*         m_policyext;       ///< Active policy extension.
    // Only used for new code generators ({Cpp,Java,Ruby,D}CodeGenerator).

    static bool canDecode(const QMimeData* mimeSource);

    void readOptionState();

    void initClip();
    void initSavedCodeGenerators();

    void importFiles(QStringList* fileList);

    KSharedConfigPtr m_config;  ///< The configuration object of the application.

    /**
     * View is the main widget which represents your working area.
     * The View class should handle all events of the view widget.
     * It is kept empty so you can create your view according to your
     * application's needs by changing the view class.
     */
    UMLView* m_view;

    /**
     * Doc represents your actual document and is created only once.
     * It keeps information such as filename and does the loading and
     * saving of your files.
     */
    UMLDoc* m_doc;

    UMLListView* m_listView;  ///< Listview shows the current open file.

    QDockWidget* m_mainDock;           ///< The widget which shows the diagrams.
    QDockWidget* m_listDock;           ///< Contains the UMLListView tree view.
    QDockWidget* m_documentationDock;  ///< Contains the documentation DocWindow widget.
    QDockWidget* m_cmdHistoryDock;     ///< Contains the undo/redo viewer widget.
    QDockWidget* m_propertyDock;       ///< Contains the property browser widget.
    DocWindow*   m_pDocWindow;         ///< Documentation window.
    QUndoView*   m_pQUndoView;         ///< Undo / Redo Viewer
    RefactoringAssistant* m_refactoringAssist;  ///< Refactoring assistant.

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

    bool m_loading;  ///< True if the application is opening an existing document.

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

    QString m_imageMimeType;  ///< Default mime type to use for image export.

    SettingsDlg* m_dlg;  ///< The global UML settings dialog.

    UMLViewImageExporterAll* m_imageExporterAll;  ///< Used to export all the views.

    /**
     * The running XHTML documentation generator. Null when no generation is running.
     */
    XhtmlGenerator* m_xhtmlGenerator;

    KUndoStack* m_pUndoStack;  ///< UndoStack used to store actions, to provide Undo/Redo feature.

    bool m_hasBegunMacro;  ///< Macro creation flag.

signals:
    void sigCutSuccessful();

};

#endif // UML_H
