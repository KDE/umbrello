/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UML_H
#define UML_H

#include "basictypes.h"
#include "multipagedialogbase.h"

// kde includes
#include <kxmlguiwindow.h>
#include <ksharedconfig.h>

// qt includes
#include <QPointer>
#include <QUrl>

// forward declaration of the UML classes
class BirdView;
class BirdViewDockWidget;
class CodeDocument;
class CodeGenerator;
class CodeGenerationPolicy;
class CodeGenPolicyExt;
class DocWindow;
class UMLAppPrivate;
class UMLClassifier;
class UMLDoc;
class UMLListView;
class UMLView;
class WorkToolBar;
class SettingsDialog;
class UMLViewImageExporterAll;
class RefactoringAssistant;
class XhtmlGenerator;
class StatusBarToolButton;
class DiagramPrintPage;

// KDE forward declarations
class KActionMenu;
class KRecentFilesAction;
class KToggleAction;
class KTabWidget;
class QMenu;
class KAction;

// Qt forward declarations
class QStackedWidget;
class QToolButton;
class QDockWidget;
class QVBoxLayout;
class QKeyEvent;
class QMenu;
class QMimeData;
class QPrinter;
class QUndoCommand;
class QUndoView;
class QUndoStack;
class QPushButton;
class QLabel;
class QListWidget;
class QSlider;

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLApp : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit UMLApp(QWidget *parent = nullptr);
    ~UMLApp();

    void setup();

    static UMLApp* app();

    void openDocumentFile(const QUrl& url=QUrl());

    void newDocument();

    UMLDoc *document() const;

    UMLListView* listView() const;
    WorkToolBar* workToolBar() const;
    DocWindow * docWindow() const;
    QListWidget *logWindow() const;
    bool logToConsole() const;
    void log(const QString& s);
    void logDebug(const QString& s);
    void logInfo(const QString& s);
    void logWarn(const QString& s);
    void logError(const QString& s);

    QCursor defaultCursor() const;

    void setModified(bool _m);

    void enablePrint(bool enable);

    bool isPasteState() const;
    bool isCutCopyState() const;

    bool isUndoEnabled() const;
    void enableUndo(bool enable);

    bool isUndoActionEnabled() const;
    void enableUndoAction(bool enable);

    bool isRedoActionEnabled() const;
    void enableRedoAction(bool enable);

    bool isSimpleCodeGeneratorActive() const;

    void setGenerator(CodeGenerator* gen, bool giveWarning = true);
    CodeGenerator* setGenerator(Uml::ProgrammingLanguage::Enum pl);
    CodeGenerator* generator() const;

    CodeGenerator* createGenerator();
    void initGenerator();

    void refactor(UMLClassifier* classifier);

    void viewCodeDocument(UMLClassifier* classifier);

    void setDiagramMenuItemsState(bool bState);

    QWidget* mainViewWidget() const;

    void setCurrentView(UMLView* view, bool updateTreeView = true);
    UMLView* currentView() const;

    void setImageMimeType(const QString& mimeType);
    QString imageMimeType() const;

    bool editCutCopy(bool bFromView);

    QTabWidget *tabWidget();

    QString statusBarMsg() const;

    CodeGenerationPolicy *commonPolicy() const;

    void setPolicyExt(CodeGenPolicyExt *policy);
    CodeGenPolicyExt *policyExt() const;

    void clearUndoStack();

    void undo();
    void redo();

    void executeCommand(QUndoCommand* cmd);

    void beginMacro(const QString & text);
    void endMacro();

    void setActiveLanguage(Uml::ProgrammingLanguage::Enum pl);
    Uml::ProgrammingLanguage::Enum activeLanguage() const;
    Uml::ProgrammingLanguage::Enum defaultLanguage() const;

    bool activeLanguageIsCaseSensitive() const;

    QString activeLanguageScopeSeparator() const;

    KConfig* config();
    void importFiles(QStringList& fileList, const QString &rootPath = QString());

    static bool shuttingDown();

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

    virtual void saveProperties(KConfigGroup & cfg);
    virtual void readProperties(const KConfigGroup & cfg);

    void updateLangSelectMenu(Uml::ProgrammingLanguage::Enum activeLanguage);

    Q_SLOT void slotExecGenerationWizard();
    Q_SLOT void slotImportingWizard();

    Q_SLOT void slotFileNew();
    Q_SLOT void slotFileOpen();

    Q_SLOT void slotFileOpenRecent(const QUrl& url);
    Q_SLOT void slotFileSave();
    Q_SLOT bool slotFileSaveAs();
    Q_SLOT void slotFileClose();
    Q_SLOT bool slotPrintSettings();
    Q_SLOT void slotPrintPreview();
    Q_SLOT void slotPrintPreviewPaintRequested(QPrinter *printer);
    Q_SLOT void slotFilePrint();
    Q_SLOT void slotFileQuit();
    Q_SLOT void slotFileExportDocbook();
    Q_SLOT void slotFileExportXhtml();
    Q_SLOT void slotFind();
    Q_SLOT void slotFindNext();
    Q_SLOT void slotFindPrevious();
    Q_SLOT void slotEditCut();
    Q_SLOT void slotEditCopy();
    Q_SLOT void slotEditPaste();
    Q_SLOT void slotStatusMsg(const QString &text);
    Q_SLOT void slotClassDiagram();
    Q_SLOT void slotObjectDiagram();
    Q_SLOT void slotSequenceDiagram();
    Q_SLOT void slotCollaborationDiagram();
    Q_SLOT void slotUseCaseDiagram();
    Q_SLOT void slotStateDiagram();
    Q_SLOT void slotActivityDiagram();
    Q_SLOT void slotComponentDiagram();
    Q_SLOT void slotDeploymentDiagram();
    Q_SLOT void slotEntityRelationshipDiagram();
    Q_SLOT void slotAlignLeft();
    Q_SLOT void slotAlignRight();
    Q_SLOT void slotAlignTop();
    Q_SLOT void slotAlignBottom();
    Q_SLOT void slotAlignVerticalMiddle();
    Q_SLOT void slotAlignHorizontalMiddle();
    Q_SLOT void slotAlignVerticalDistribute();
    Q_SLOT void slotAlignHorizontalDistribute();
    Q_SLOT void slotClipDataChanged();
    Q_SLOT void slotCopyChanged();
    Q_SLOT void slotPrefs(MultiPageDialogBase::PageType page = MultiPageDialogBase::GeneralPage);
    Q_SLOT void slotApplyPrefs();
    Q_SLOT void slotImportClass();
    Q_SLOT void slotImportProject();
    Q_SLOT void slotUpdateViews();
    Q_SLOT void slotShowTreeView(bool state);
    Q_SLOT void slotShowDebugView(bool state);
    Q_SLOT void slotShowDocumentationView(bool state);
    Q_SLOT void slotShowCmdHistoryView(bool state);
    Q_SLOT void slotShowLogView(bool state);
    Q_SLOT void slotShowBirdView(bool state);
    Q_SLOT void slotCurrentViewClearDiagram();
    Q_SLOT void slotCurrentViewToggleSnapToGrid();
    Q_SLOT void slotCurrentViewToggleShowGrid();
    Q_SLOT void slotCurrentViewExportImage();
    Q_SLOT void slotViewsExportImages();
    Q_SLOT void slotCurrentProperties();
    Q_SLOT void slotClassWizard();
    Q_SLOT void slotAddDefaultDatatypes();
    Q_SLOT void slotCurrentViewChanged();
    Q_SLOT void slotSnapToGridToggled(bool gridOn);
    Q_SLOT void slotShowGridToggled(bool gridOn);
    Q_SLOT void slotSelectAll();
    Q_SLOT void slotDeleteSelected();
    Q_SLOT void slotDeleteDiagram();
    Q_SLOT void slotCloseDiagram(int index);
    Q_SLOT void slotGenerateAllCode();

    Q_SLOT void slotSetZoom(QAction* action);
    Q_SLOT void slotZoomSliderMoved(int value);
    Q_SLOT void slotZoomFit();
    Q_SLOT void slotZoom100();
    Q_SLOT void slotZoomOut();
    Q_SLOT void slotZoomIn();

    Q_SLOT void slotBirdViewChanged(const QPointF& newCenter);

    Q_SLOT void setupZoomMenu();

    Q_SLOT void slotEditUndo();
    Q_SLOT void slotEditRedo();

    Q_SLOT void slotTabChanged(int index);
    Q_SLOT void slotChangeTabLeft();
    Q_SLOT void slotChangeTabRight();
    Q_SLOT void slotMoveTabLeft();
    Q_SLOT void slotMoveTabRight();

    Q_SLOT void slotXhtmlDocGenerationFinished(bool status);
    Q_SLOT bool slotOpenFileInEditor(const QUrl &file, int startCursor = 0, int endCursor = 0);

private:
    Q_SLOT void setLang_actionscript();
    Q_SLOT void setLang_ada();
    Q_SLOT void setLang_cpp();
    Q_SLOT void setLang_csharp();
    Q_SLOT void setLang_d();
    Q_SLOT void setLang_idl();
    Q_SLOT void setLang_java();
    Q_SLOT void setLang_javascript();
    Q_SLOT void setLang_mysql();
    Q_SLOT void setLang_pascal();
    Q_SLOT void setLang_perl();
    Q_SLOT void setLang_php();
    Q_SLOT void setLang_php5();
    Q_SLOT void setLang_postgresql();
    Q_SLOT void setLang_python();
    Q_SLOT void setLang_ruby();
    Q_SLOT void setLang_sql();
    Q_SLOT void setLang_tcl();
    Q_SLOT void setLang_vala();
    Q_SLOT void setLang_xmlschema();
    Q_SLOT void setLang_none();
    Q_SLOT void slotDiagramPopupMenu(const QPoint& point);

private:
    static UMLApp* s_instance;  ///< The last created instance of this class.
    UMLAppPrivate *m_d;

    QMenu* findMenu(const QString &name);

    QAction* createZoomAction(int zoom, int currentZoom);
    void setZoom(int zoom, bool withView = true);

    void resetStatusMsg();

    void setProgLangAction(Uml::ProgrammingLanguage::Enum pl, const char* name, const char* action);

    static bool canDecode(const QMimeData* mimeSource);

    void readOptionState() const;

    void initClip();
    void initSavedCodeGenerators();

    void createDiagram(Uml::DiagramType::Enum type);

    void createBirdView(UMLView* view);

    QMenu* m_langSelect;  ///< For selecting the active language.
    QMenu* m_zoomSelect;  ///< Popup menu for zoom selection.

    Uml::ProgrammingLanguage::Enum  m_activeLanguage;  ///< Active language.
    CodeGenerator*            m_codegen;         ///< Active code generator.
    CodeGenerationPolicy*     m_commoncodegenpolicy;
    CodeGenPolicyExt*         m_policyext;       ///< Active policy extension.
    // Only used for new code generators ({Cpp, Java, Ruby, D}CodeGenerator).

    KSharedConfigPtr m_config;  ///< The configuration object of the application.

    /**
     * View is the main widget which represents your working area.
     * The View class should handle all events of the view widget.
     * It is kept empty so you can create your view according to your
     * application's needs by changing the view class.
     */
    QPointer<UMLView> m_view;

    /**
     * Doc represents your actual document and is created only once.
     * It keeps information such as filename and does the loading and
     * saving of your files.
     */
    UMLDoc* m_doc;

    UMLListView* m_listView;  ///< Listview shows the current open file.

    QDockWidget* m_mainDock;           ///< The widget which shows the diagrams.
    QDockWidget* m_listDock;           ///< Contains the UMLListView tree view.
    QDockWidget* m_debugDock;          ///< Contains the debug DocWindow widget.
    QDockWidget* m_documentationDock;  ///< Contains the documentation DocWindow widget.
    QDockWidget* m_cmdHistoryDock;     ///< Contains the undo/redo viewer widget.
    QDockWidget* m_propertyDock;       ///< Contains the property browser widget.
    QDockWidget* m_logDock;            ///< Contains the log window widget.
    BirdViewDockWidget* m_birdViewDock;///< Contains the bird's eye view

    DocWindow*   m_docWindow;          ///< Documentation window.
    BirdView*    m_birdView;           ///< Bird View window
    QUndoView*   m_pQUndoView;         ///< Undo / Redo Viewer
    RefactoringAssistant* m_refactoringAssist;  ///< Refactoring assistant.

    // KAction pointers to enable/disable actions
    KRecentFilesAction* fileOpenRecent;
    QAction* printPreview;
    QAction* filePrint;
    QAction* editCut;
    QAction* editCopy;
    QAction* editPaste;
    QAction* editUndo;
    QAction* editRedo;

    QAction* viewShowTree;
    QAction* viewShowDebug;
    QAction* viewShowDoc;
    QAction* viewShowLog;
    QAction* viewShowCmdHistory;
    QAction* viewShowBirdView;

    KActionMenu* newDiagram;
    QAction* viewClearDiagram;

    KToggleAction* viewSnapToGrid;
    KToggleAction* viewShowGrid;
    QAction* viewExportImage;
    QAction* viewProperties;

    QAction* zoom100Action;

    QAction* m_langAct[Uml::ProgrammingLanguage::Reserved + 1];
    QAction* deleteSelectedWidget;
    QAction* deleteDiagram;
    QToolButton* m_newSessionButton;
    WorkToolBar* m_toolsbar;
    QTimer* m_clipTimer;
    QTimer* m_copyTimer;

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
    QTabWidget* m_tabWidget;

    /**
     * Layout supports the dynamic management of the diagram representation (tabbed/stacked)
     * if tabbed diagrams is enabled it contains m_tabWidget
     * if tabbed diagrams is disabled it contains m_viewStack
     */
    QVBoxLayout* m_layout;

    QString m_imageMimeType;  ///< Default mime type to use for image export.

    SettingsDialog* m_settingsDialog;  ///< The global UML settings dialog.

    UMLViewImageExporterAll* m_imageExporterAll;  ///< Used to export all the views.

    /**
     * Statusbar items
     */
    QLabel*      m_zoomValueLbl;
    QWidget*     m_defaultZoomWdg;
    QPushButton* m_pZoomOutPB;
    QPushButton* m_pZoomInPB;
    StatusBarToolButton* m_pZoomFitSBTB;
    StatusBarToolButton* m_pZoomFullSBTB;
    QSlider* m_pZoomSlider;
    QLabel* m_statusBarMessage;
    /**
     * The running XHTML documentation generator. Null when no generation is running.
     */
    XhtmlGenerator* m_xhtmlGenerator;

    QUndoStack* m_pUndoStack;  ///< UndoStack used to store actions, to provide Undo/Redo feature.
    bool m_undoEnabled; ///< Undo enabled flag

    bool m_hasBegunMacro;  ///< Macro creation flag.

    QPointer<DiagramPrintPage> m_printSettings; ///< printer diagram settings
    QPrinter *m_printer;               ///< print instance
    static bool s_shuttingDown;

private:
    Q_SIGNAL void sigCutSuccessful();

    friend class UMLAppPrivate;
    friend class UMLView;
};

#define logDebug0(s) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                     UMLApp::app()->logDebug(QStringLiteral(s))
#define logInfo0(s)  UMLApp::app()->logInfo(QStringLiteral(s))
#define logWarn0(s)  UMLApp::app()->logWarn(QStringLiteral(s))
#define logError0(s) UMLApp::app()->logError(QStringLiteral(s))

#define logDebug1(s, a) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                        do { QString fmt = QString(QStringLiteral(s)).arg(a); UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo1(s, a)  do { QString fmt = QString(QStringLiteral(s)).arg(a); UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn1(s, a)  do { QString fmt = QString(QStringLiteral(s)).arg(a); UMLApp::app()->logWarn(fmt);  } while (0)
#define logError1(s, a) do { QString fmt = QString(QStringLiteral(s)).arg(a); UMLApp::app()->logError(fmt); } while (0)

#define logDebug2(s, a, b) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                           do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b); UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo2(s, a, b)  do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b); UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn2(s, a, b)  do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b); UMLApp::app()->logWarn(fmt);  } while (0)
#define logError2(s, a, b) do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b); UMLApp::app()->logError(fmt); } while (0)

#define logDebug3(s, a, b, c) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                              do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c); UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo3(s, a, b, c)  do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c); UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn3(s, a, b, c)  do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c); UMLApp::app()->logWarn(fmt);  } while (0)
#define logError3(s, a, b, c) do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c); UMLApp::app()->logError(fmt); } while (0)

#define logDebug4(s, a, b, c, d) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                                 do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c).arg(d); UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo4(s, a, b, c, d)  do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c).arg(d); UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn4(s, a, b, c, d)  do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c).arg(d); UMLApp::app()->logWarn(fmt);  } while (0)
#define logError4(s, a, b, c, d) do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c).arg(d); UMLApp::app()->logError(fmt); } while (0)

#define logDebug5(s, a, b, c, d, e) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                                    do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c).arg(d).arg(e); \
                                    UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo5(s, a, b, c, d, e)  do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c).arg(d).arg(e); \
                                    UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn5(s, a, b, c, d, e)  do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c).arg(d).arg(e); \
                                    UMLApp::app()->logWarn(fmt);  } while (0)
#define logError5(s, a, b, c, d, e) do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c).arg(d).arg(e); \
                                    UMLApp::app()->logError(fmt); } while (0)

#define logDebug6(s, a, b, c, d, e, f) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
        do { QString fmt = QString(QStringLiteral(s)).arg(a).arg(b).arg(c).arg(d).arg(e).arg(f); UMLApp::app()->logDebug(fmt); } while (0)

#endif // UML_H
