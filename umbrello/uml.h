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
#if QT_VERSION < 0x050000
#include <kurl.h>
#endif
#include <ksharedconfig.h>

// qt includes
#include <QPointer>
#if QT_VERSION >= 0x050000
#include <QUrl>
#endif

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
class KMenu;
#if QT_VERSION < 0x050000
class KUndoStack;
#endif
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
#if QT_VERSION >= 0x050000
class QUndoStack;
#endif
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
    explicit UMLApp(QWidget* parent = 0);
    ~UMLApp();

    static UMLApp* app();

#if QT_VERSION >= 0x050000
    void openDocumentFile(const QUrl& url=QUrl());
#else
    void openDocumentFile(const KUrl& url=KUrl());
#endif

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

#if QT_VERSION >= 0x050000
    QTabWidget *tabWidget();
#else
    KTabWidget *tabWidget();
#endif

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

public slots:
    void slotExecGenerationWizard();
    void slotImportingWizard();

    void slotFileNew();
    void slotFileOpen();

#if QT_VERSION >= 0x050000
    void slotFileOpenRecent(const QUrl& url);
#else
    void slotFileOpenRecent(const KUrl& url);
#endif
    void slotFileSave();
    bool slotFileSaveAs();
    void slotFileClose();
    bool slotPrintSettings();
    void slotPrintPreview();
    void slotPrintPreviewPaintRequested(QPrinter *printer);
    void slotFilePrint();
    void slotFileQuit();
    void slotFileExportDocbook();
    void slotFileExportXhtml();
    void slotFind();
    void slotFindNext();
    void slotFindPrevious();
    void slotEditCut();
    void slotEditCopy();
    void slotEditPaste();
    void slotStatusMsg(const QString &text);
    void slotClassDiagram();
    void slotObjectDiagram();
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
    void slotPrefs(MultiPageDialogBase::PageType page = MultiPageDialogBase::GeneralPage);
    void slotApplyPrefs();
    void slotImportClass();
    void slotImportProject();
    void slotUpdateViews();
    void slotShowTreeView(bool state);
    void slotShowDebugView(bool state);
    void slotShowDocumentationView(bool state);
    void slotShowCmdHistoryView(bool state);
    void slotShowLogView(bool state);
    void slotShowBirdView(bool state);
    void slotCurrentViewClearDiagram();
    void slotCurrentViewToggleSnapToGrid();
    void slotCurrentViewToggleShowGrid();
    void slotCurrentViewExportImage();
    void slotViewsExportImages();
    void slotCurrentProperties();
    void slotClassWizard();
    void slotAddDefaultDatatypes();
    void slotCurrentViewChanged();
    void slotSnapToGridToggled(bool gridOn);
    void slotShowGridToggled(bool gridOn);
    void slotSelectAll();
    void slotDeleteSelected();
    void slotDeleteDiagram();
#if QT_VERSION >= 0x050000
    void slotCloseDiagram(int index);
#else
    void slotCloseDiagram(QWidget* tab);
#endif
    void slotGenerateAllCode();

    void slotSetZoom(QAction* action);
    void slotZoomSliderMoved(int value);
    void slotZoomFit();
    void slotZoom100();
    void slotZoomOut();
    void slotZoomIn();

    void slotBirdViewChanged(const QPointF& newCenter);

    void setupZoomMenu();

    void slotEditUndo();
    void slotEditRedo();

#if QT_VERSION >= 0x050000
    void slotTabChanged(int index);
#else
    void slotTabChanged(QWidget* tab);
#endif
    void slotChangeTabLeft();
    void slotChangeTabRight();
    void slotMoveTabLeft();
    void slotMoveTabRight();

    void slotXhtmlDocGenerationFinished(bool status);
    bool slotOpenFileInEditor(const QUrl &file, int startCursor = 0, int endCursor = 0);

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
    void setLang_vala();
    void setLang_xmlschema();
    void setLang_none();
#if QT_VERSION >= 0x050000
    void slotDiagramPopupMenu(const QPoint& point);
#endif

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
    QListWidget* m_logWindow;          ///< Logging window.
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
#if QT_VERSION >= 0x050000
    QAction* deleteSelectedWidget;
    QAction* deleteDiagram;
#else
    KAction* deleteSelectedWidget;
    KAction* deleteDiagram;
#endif
    QToolButton* m_newSessionButton;
    KMenu* m_diagramMenu;
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
#if QT_VERSION >= 0x050000
    QTabWidget* m_tabWidget;
#else
    KTabWidget* m_tabWidget;
#endif

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

#if QT_VERSION >= 0x050000
    QUndoStack* m_pUndoStack;  ///< UndoStack used to store actions, to provide Undo/Redo feature.
#else
    KUndoStack* m_pUndoStack;  ///< UndoStack used to store actions, to provide Undo/Redo feature.
#endif
    bool m_undoEnabled; ///< Undo enabled flag

    bool m_hasBegunMacro;  ///< Macro creation flag.

    QPointer<DiagramPrintPage> m_printSettings; ///< printer diagram settings
    QPrinter *m_printer;               ///< print instance
    static bool s_shuttingDown;

signals:
    void sigCutSuccessful();

    friend class UMLAppPrivate;
    friend class UMLView;
};

#define logDebug0(s) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                     UMLApp::app()->logDebug(QLatin1String(s))
#define logInfo0(s)  UMLApp::app()->logInfo(QLatin1String(s))
#define logWarn0(s)  UMLApp::app()->logWarn(QLatin1String(s))
#define logError0(s) UMLApp::app()->logError(QLatin1String(s))

#define logDebug1(s, a) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                        do { QString fmt = QString(QLatin1String(s)).arg(a); UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo1(s, a)  do { QString fmt = QString(QLatin1String(s)).arg(a); UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn1(s, a)  do { QString fmt = QString(QLatin1String(s)).arg(a); UMLApp::app()->logWarn(fmt);  } while (0)
#define logError1(s, a) do { QString fmt = QString(QLatin1String(s)).arg(a); UMLApp::app()->logError(fmt); } while (0)

#define logDebug2(s, a, b) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                           do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b); UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo2(s, a, b)  do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b); UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn2(s, a, b)  do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b); UMLApp::app()->logWarn(fmt);  } while (0)
#define logError2(s, a, b) do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b); UMLApp::app()->logError(fmt); } while (0)

#define logDebug3(s, a, b, c) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                              do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c); UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo3(s, a, b, c)  do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c); UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn3(s, a, b, c)  do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c); UMLApp::app()->logWarn(fmt);  } while (0)
#define logError3(s, a, b, c) do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c); UMLApp::app()->logError(fmt); } while (0)

#define logDebug4(s, a, b, c, d) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                                 do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c).arg(d); UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo4(s, a, b, c, d)  do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c).arg(d); UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn4(s, a, b, c, d)  do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c).arg(d); UMLApp::app()->logWarn(fmt);  } while (0)
#define logError4(s, a, b, c, d) do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c).arg(d); UMLApp::app()->logError(fmt); } while (0)

#define logDebug5(s, a, b, c, d, e) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
                                    do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c).arg(d).arg(e); \
                                    UMLApp::app()->logDebug(fmt); } while (0)
#define logInfo5(s, a, b, c, d, e)  do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c).arg(d).arg(e); \
                                    UMLApp::app()->logInfo(fmt);  } while (0)
#define logWarn5(s, a, b, c, d, e)  do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c).arg(d).arg(e); \
                                    UMLApp::app()->logWarn(fmt);  } while (0)
#define logError5(s, a, b, c, d, e) do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c).arg(d).arg(e); \
                                    UMLApp::app()->logError(fmt); } while (0)

#define logDebug6(s, a, b, c, d, e, f) if (UMLApp::app()->logToConsole() || Tracer::instance()->isEnabled(DBG_SRC)) \
        do { QString fmt = QString(QLatin1String(s)).arg(a).arg(b).arg(c).arg(d).arg(e).arg(f); UMLApp::app()->logDebug(fmt); } while (0)

#endif // UML_H
