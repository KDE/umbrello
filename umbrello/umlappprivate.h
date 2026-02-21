/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLAPPPRIVATE_H
#define UMLAPPPRIVATE_H

// app includes
#include "birdview.h"
#include "birdviewdockwidget.h"
#include "cmdcreatediagram.h"
#include "debug_utils.h"
#include "diagramswindow.h"
#include "docwindow.h"
#include "finddialog.h"
#include "findresults.h"
#include "objectswindow.h"
#include "stereotypeswindow.h"
#include "umlapp.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlview.h"

// kde includes
#include <KActionCategory>
#include <KActionCollection>
#include <KToggleAction>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <ktexteditor/configinterface.h>
#endif
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

// qt includes
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QListWidget>
#include <QObject>
#include <QUndoView>
#ifdef WEBKIT_WELCOMEPAGE
#include <QWebView>
#else
#include <QTextBrowser>
#endif

class QWidget;


/**
 * Class UMLAppPrivate holds private class members/methods
 * to reduce the size of the public class and to speed up
 * recompiling.
 * The migration to this class is not complete yet.
 **/
class UMLAppPrivate : public QObject
{
    Q_OBJECT
public:
    UMLApp *parent;
    FindDialog findDialog;
    FindResults findResults;
    QListWidget *logWindow;         ///< Logging window.
    DiagramsWindow *diagramsWindow;
    ObjectsWindow *objectsWindow;
    StereotypesWindow *stereotypesWindow;
    DocWindow*   docWindow;          ///< Documentation window.
    BirdView*    birdView;           ///< Bird View window
    UMLListView* listView;           ///< Listview shows the current open file.
    QUndoView*   pQUndoView;         ///< Undo / Redo Viewer

    BirdViewDockWidget* birdViewDock;///< Contains the bird's eye view
    QDockWidget *editorWindow;
    QDockWidget *logDock;            ///< Contains the log window widget.
    QDockWidget *welcomeWindow;
    QDockWidget* cmdHistoryDock;     ///< Contains the undo/redo viewer widget.
    QDockWidget* debugDock;          ///< Contains the debug DocWindow widget.
    QDockWidget* documentationDock;  ///< Contains the documentation DocWindow widget.
    QDockWidget* listDock;           ///< Contains the UMLListView tree view.

    KTextEditor::Editor *editor;
    KTextEditor::View *view;
    KTextEditor::Document *document;
    KActionCategory *dockCategory;

    explicit UMLAppPrivate(UMLApp *_parent)
      : parent(_parent)
      , findDialog(_parent)
      , diagramsWindow(nullptr)
      , objectsWindow(nullptr)
      , stereotypesWindow(nullptr)
      , birdView(nullptr)
      , listView(nullptr)
      , pQUndoView(nullptr)
      , editorWindow(nullptr)
      , welcomeWindow(nullptr)
      , view(nullptr)
      , document(nullptr)
      , dockCategory(new KActionCategory(i18n("Show/hide window"), parent->actionCollection()))
    {
    }

    ~UMLAppPrivate()
    {
        delete diagramsWindow;
        delete editorWindow;
        delete logWindow;
        delete objectsWindow;
        delete stereotypesWindow;
        delete welcomeWindow;
        delete birdView;
        delete listView;
    }

    bool openFileInEditor(const QUrl &file, int startCursor=0, int endCursor=0);

public Q_SLOTS:
    void slotLogWindowItemDoubleClicked(QListWidgetItem *item)
    {
        QStringList columns = item->text().split(QChar::fromLatin1(':'));

        openFileInEditor(QUrl::fromLocalFile(columns[0]), columns[1].toInt()-1, columns[2].toInt());
    }

    void initActions()
    {
        createBirdWindow();
        createCommandHistoryWindow();
        createDebugWindow();
        createDiagramsWindow();
        createDocumentationWindow();
        createLogWindow();
#ifdef ENABLE_UML_OBJECTS_WINDOW
        createObjectsWindow();
#endif
        createPropertyWindow();
        createStereotypesWindow();
        createTreeWindow();
        createWelcomeWindow();

        parent->tabifyDockWidget(documentationDock, cmdHistoryDock);
        parent->tabifyDockWidget(cmdHistoryDock, logDock);
        //tabifyDockWidget(m_cmdHistoryDock, m_propertyDock);  //:TODO:
        parent->tabifyDockWidget(logDock, debugDock);
        parent->tabifyDockWidget(listDock, stereotypesWindow);
        parent->tabifyDockWidget(stereotypesWindow, diagramsWindow);
    #ifdef ENABLE_UML_OBJECTS_WINDOW
        parent->tabifyDockWidget(diagramsWindow, objectsWindow);
    #endif
        if (welcomeWindow) {
            parent->tabifyDockWidget(welcomeWindow, birdViewDock);
            welcomeWindow->raise();
        }
        listDock->raise();
    }

    void initWidgets()
    {
        /* TODO: On the call to KTextEditor::Editor::instance() Valgrind reports
           "Conditional jump or move depends on uninitialised value(s)".
         */
        editor = KTextEditor::Editor::instance();
    }

    void createBirdWindow()
    {
        birdViewDock = new BirdViewDockWidget(i18n("&Bird's eye view"), parent);
        birdViewDock->setObjectName(QStringLiteral("BirdViewDock"));
        parent->addDockWidget(Qt::RightDockWidgetArea, birdViewDock);
        dockCategory->addAction(QStringLiteral("view_show_bird"), birdViewDock->toggleViewAction());
    }

    void createCommandHistoryWindow()
    {
        cmdHistoryDock = new QDockWidget(i18n("Co&mmand history"), parent);
        cmdHistoryDock->setObjectName(QStringLiteral("CmdHistoryDock"));
        parent->addDockWidget(Qt::LeftDockWidgetArea, cmdHistoryDock);
        pQUndoView = new QUndoView(cmdHistoryDock);
        pQUndoView->setCleanIcon(Icon_Utils::SmallIcon(Icon_Utils::it_UndoView));
        pQUndoView->setStack(parent->m_pUndoStack);
        cmdHistoryDock->setWidget(pQUndoView);
        dockCategory->addAction(QStringLiteral("view_show_undo"), cmdHistoryDock->toggleViewAction());
    }

    void createDebugWindow()
    {
        debugDock = new QDockWidget(i18n("&Debug"), parent);
        debugDock->setObjectName(QStringLiteral("DebugDock"));
        parent->addDockWidget(Qt::LeftDockWidgetArea, debugDock);
        debugDock->setWidget(Tracer::instance());
        dockCategory->addAction(QStringLiteral("view_show_debug"), debugDock->toggleViewAction());
    }

    void createDiagramsWindow()
    {
        diagramsWindow = new DiagramsWindow(i18n("&Diagrams"), parent);
        parent->addDockWidget(Qt::LeftDockWidgetArea, diagramsWindow);
        dockCategory->addAction(QStringLiteral("view_show_diagrams"), diagramsWindow->toggleViewAction());
    }

    void createDocumentationWindow()
    {
        documentationDock = new QDockWidget(i18n("Doc&umentation"), parent);
        documentationDock->setObjectName(QStringLiteral("DocumentationDock"));
        parent->addDockWidget(Qt::LeftDockWidgetArea, documentationDock);
        docWindow = new DocWindow(parent->document(), documentationDock);
        docWindow->setObjectName(QStringLiteral("DOCWINDOW"));
        documentationDock->setWidget(docWindow);
        dockCategory->addAction(QStringLiteral("view_show_doc"), documentationDock->toggleViewAction());
    }

    void createLogWindow()
    {
        logWindow = new QListWidget;
        QFont mono;
        mono.setFamily(QStringLiteral("Monospace"));
        logWindow->setFont(mono);
        connect(logWindow, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(slotLogWindowItemDoubleClicked(QListWidgetItem *)));

        logDock = new QDockWidget(i18n("&Log"), parent);
        logDock->setObjectName(QStringLiteral("LogDock"));
        parent->addDockWidget(Qt::LeftDockWidgetArea, logDock);
        logDock->setWidget(logWindow);
        dockCategory->addAction(QStringLiteral("view_show_log"), logDock->toggleViewAction());
    }

    void createObjectsWindow()
    {
        objectsWindow = new ObjectsWindow(i18n("&UML Objects"), parent);
        parent->addDockWidget(Qt::LeftDockWidgetArea, objectsWindow);
        dockCategory->addAction(QStringLiteral("view_show_objects"), objectsWindow->toggleViewAction());
    }

    void createPropertyWindow()
    {
        //m_propertyDock = new QDockWidget(i18n("&Properties"), this);
        //m_propertyDock->setObjectName(QStringLiteral("PropertyDock"));
        //addDockWidget(Qt::LeftDockWidgetArea, m_propertyDock);  //:TODO:
    }

    void createStereotypesWindow()
    {
        stereotypesWindow = new StereotypesWindow(i18n("&Stereotypes"), parent);
        parent->addDockWidget(Qt::LeftDockWidgetArea, stereotypesWindow);
        dockCategory->addAction(QStringLiteral("view_show_stereotypes"), stereotypesWindow->toggleViewAction());
    }

    void createTreeWindow()
    {
        listDock = new QDockWidget(i18n("&Tree View"), parent);
        listDock->setObjectName(QStringLiteral("TreeViewDock"));
        parent->addDockWidget(Qt::LeftDockWidgetArea, listDock);
        listView = new UMLListView(listDock);
        //m_listView->setSorting(-1);
        listView->setDocument(parent->document());
        listView->init();
        listDock->setWidget(listView);
        dockCategory->addAction(QStringLiteral("view_show_tree"), listDock->toggleViewAction());
    }

    void createWelcomeWindow()
    {
        QString file = findWelcomeFile();
        if (file.isEmpty())
            return;
        QString html = readWelcomeFile(file);
        if (html.isEmpty())
            return;
        // qDebug() << html;
        welcomeWindow = new QDockWidget(i18n("Welcome"), parent);
        welcomeWindow->setObjectName(QStringLiteral("WelcomeDock"));
#ifdef WEBKIT_WELCOMEPAGE
        QWebView *view = new QWebView;
        view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
        view->setContextMenuPolicy(Qt::NoContextMenu);
        connect(view, SIGNAL(linkClicked(const QUrl)), this, SLOT(slotWelcomeWindowLinkClicked(const QUrl)));
        view->setHtml(html);
        view->show();
        welcomeWindow->setWidget(view);
#else
        QTextBrowser *tb = new QTextBrowser(dynamic_cast<QWidget*>(this));
        tb->setOpenExternalLinks(true);
        tb->setOpenLinks(false);
        tb->setHtml(html);
        connect(tb, SIGNAL(anchorClicked(const QUrl)), this, SLOT(slotWelcomeWindowLinkClicked(const QUrl)));
        welcomeWindow->setWidget(tb);
#endif
        parent->addDockWidget(Qt::RightDockWidgetArea, welcomeWindow);
        dockCategory->addAction(QStringLiteral("view_show_welcome"), welcomeWindow->toggleViewAction());
    }

    void slotWelcomeWindowLinkClicked(const QUrl &url)
    {
        //qDebug() << url;
        if (url.scheme() == QStringLiteral("mailto") || url.scheme().startsWith(QStringLiteral("http"))) {
            QDesktopServices::openUrl(url);
            return;
        }
        QStringList list = url.toString().split(QLatin1Char('-'));
        list.removeLast();
        QString key;
        for(const QString &s: list) {
            QString a = s;
            a[0] = a[0].toUpper();
            key.append(a);
        }
        Uml::DiagramType::Enum type = Uml::DiagramType::fromString(key);
        if (type == Uml::DiagramType::Undefined)
            return;
        QString diagramName = UMLApp::app()->document()->createDiagramName(type);
        if (!diagramName.isEmpty())
            UMLApp::app()->executeCommand(new Uml::CmdCreateDiagram(UMLApp::app()->document(), type, diagramName));
    }

    /**
     * Create bird's view window in a dock widget.
     */
    void createBirdView(UMLView *view)
    {
        if (birdView) {
            delete birdView;
        }
        birdView = new BirdView(birdViewDock, view);
        connect(birdView, SIGNAL(viewPositionChanged(QPointF)), this, SLOT(slotBirdViewChanged(QPointF)));
        connect(birdViewDock, SIGNAL(sizeChanged(QSize)), birdView, SLOT(slotDockSizeChanged(QSize)));
    }

    void deleteBirdView()
    {
        disconnect(birdView, SIGNAL(viewPositionChanged(QPointF)), this, SLOT(slotBirdViewChanged(QPointF)));
        disconnect(birdViewDock, SIGNAL(sizeChanged(QSize)), birdView, SLOT(slotDockSizeChanged(QSize)));

        if (birdView) {
            delete birdView;
        }
        birdView = nullptr;
    }

    /**
     * Slot for changes of the bird view's rectangle by moving.
     * @param delta   change value for a move
     */
    void slotBirdViewChanged(const QPointF& delta)
    {
        birdView->setSlotsEnabled(false);
        UMLView* view = parent->currentView();
        QPointF oldCenter = view->mapToScene(view->viewport()->rect().center());
        QPointF newCenter = oldCenter + delta;
        view->centerOn(newCenter);
        // DEBUG() << "view moved with: " << delta;
        birdView->setSlotsEnabled(true);
    }

private:
    QString findWelcomeFile();
    QString readWelcomeFile(const QString &file);
};

#endif
