/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umldoc.h"

// app includes
#include "debug_utils.h"
#include "uniqueid.h"
#include "umlassociation.h"
#include "package.h"
#include "folder.h"
#include "codegenerator.h"
#include "umlclassifier.h"
#include "dialog_utils.h"
#include "umlenum.h"
#include "entity.h"
#include "docwindow.h"
#include "operation.h"
#include "umlattribute.h"
#include "template.h"
#include "enumliteral.h"
#include "stereotype.h"
#include "umldatatype.h"
#include "umlclassifierlistitem.h"
#include "object_factory.h"
#include "import_argo.h"
#include "import_rose.h"
#include "model_utils.h"
#include "uml.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlview.h"
#include "entityconstraint.h"
#include "idchangelog.h"
#include "umllistviewpopupmenu.h"
#include "cmds.h"
#include "diagramprintpage.h"
#include "umlmessagebox.h"
#include "umlscene.h"
#include "version.h"
#include "worktoolbar.h"
#include "encodinghelper.h"
#include "models/diagramsmodel.h"
#include "models/objectsmodel.h"
#include "models/stereotypesmodel.h"

// kde includes
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <ktar.h>

// qt includes
#include <QApplication>
#include <QBuffer>
#include <QDateTime>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QListWidget>
#include <QMimeDatabase>
#include <QPainter>
#include <QPrinter>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTimer>
#include <QXmlStreamWriter>

DEBUG_REGISTER(UMLDoc)

class UMLDoc::Private
{
public:
    UMLDoc *parent;
    QStringList errors; ///< holds loading errors

    Private(UMLDoc *p) : parent(p) {}
};

/**
 * Constructor for the fileclass of the application.
 */
UMLDoc::UMLDoc()
  : m_d(new Private(this)),
    m_datatypeRoot(nullptr),
    m_stereoList(UMLStereotypeList()),
    m_Name(i18n("UML Model")),
    m_modelID("m1"),
    m_count(0),
    m_modified(false),
    m_doc_url(QUrl()),
    m_pChangeLog(nullptr),
    m_bLoading(false),
    m_importing(false),
    m_Doc(QString()),
    m_pAutoSaveTimer(nullptr),
    m_nViewID(Uml::ID::None),
    m_bTypesAreResolved(true),
    m_pCurrentRoot(nullptr),
    m_bClosing(false),
    m_diagramsModel(new DiagramsModel),
    m_objectsModel(new ObjectsModel),
    m_stereotypesModel(new StereotypesModel(m_stereoList)),
    m_resolution(0.0)
{
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i)
        m_root[i] = nullptr;
}

/**
 * Initialize the UMLDoc.
 * To be called after the constructor, before anything else.
 */
void UMLDoc::init()
{
    // Initialize predefined folders.
    const char* nativeRootName[Uml::ModelType::N_MODELTYPES] = {
        "Logical View",
        "Use Case View",
        "Component View",
        "Deployment View",
        "Entity Relationship Model"
    };
    const QString localizedRootName[Uml::ModelType::N_MODELTYPES] = {
        i18n("Logical View"),
        i18n("Use Case View"),
        i18n("Component View"),
        i18n("Deployment View"),
        i18n("Entity Relationship Model")
    };
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        const QString rootName = QString::fromLatin1(nativeRootName[i]);
        QString id = rootName;
        id.replace(QLatin1Char(' '), QLatin1Char('_'));
        m_root[i] = new UMLFolder(rootName, Uml::ID::fromString(id));
        m_root[i]->setLocalName(localizedRootName[i]);
    }
    createDatatypeFolder();

    // Connect signals.
    UMLApp * pApp = UMLApp::app();
    connect(this, SIGNAL(sigDiagramCreated(Uml::ID::Type)), pApp, SLOT(slotUpdateViews()));
    connect(this, SIGNAL(sigDiagramRemoved(Uml::ID::Type)), pApp, SLOT(slotUpdateViews()));
    connect(this, SIGNAL(sigDiagramRenamed(Uml::ID::Type)), pApp, SLOT(slotUpdateViews()));
    connect(this, SIGNAL(sigCurrentViewChanged()),          pApp, SLOT(slotCurrentViewChanged()));
}

/**
 * Create the datatype folder and add it to the logical folder.
 */
void UMLDoc::createDatatypeFolder()
{
    delete m_datatypeRoot;
    m_datatypeRoot = new UMLFolder(QStringLiteral("Datatypes"), "Datatypes");
    m_datatypeRoot->setLocalName(i18n("Datatypes"));
    m_datatypeRoot->setUMLPackage(m_root[Uml::ModelType::Logical]);
    Q_ASSERT(m_root[Uml::ModelType::Logical]);
    m_root[Uml::ModelType::Logical]->addObject(m_datatypeRoot);
}

/**
 * Destructor for the fileclass of the application.
 */
UMLDoc::~UMLDoc()
{
    UMLApp * pApp = UMLApp::app();
    disconnect(this, SIGNAL(sigDiagramCreated(Uml::ID::Type)), pApp, SLOT(slotUpdateViews()));
    disconnect(this, SIGNAL(sigDiagramRemoved(Uml::ID::Type)), pApp, SLOT(slotUpdateViews()));
    disconnect(this, SIGNAL(sigDiagramRenamed(Uml::ID::Type)), pApp, SLOT(slotUpdateViews()));
    disconnect(this, SIGNAL(sigCurrentViewChanged()),          pApp, SLOT(slotCurrentViewChanged()));

    disconnect(m_pAutoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
    delete m_pAutoSaveTimer;

    m_root[Uml::ModelType::Logical]->removeObject(m_datatypeRoot);
    delete m_datatypeRoot;

    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        delete m_root[i];
    }
    delete m_pChangeLog;
    qDeleteAll(m_stereoList);
    delete m_stereotypesModel;
    delete m_diagramsModel;
    delete m_objectsModel;
    delete m_d;
}

/**
 * Adds a view to the document which represents the document
 * contents. Usually this is your main view.
 *
 * @param view   Pointer to the UMLView to add.
 */
void UMLDoc::addView(UMLView *view)
{
    if (view == nullptr) {
        logError0("UMLDoc::addView argument is NULL");
        return;
    }
    UMLFolder *f = view->umlScene()->folder();
    if (f == nullptr) {
        logError0("UMLDoc::addView view folder is not set");
        return;
    }
    logDebug2("UMLDoc::addView %1 to folder %2", view->umlScene()->name(), f->name());
    f->addView(view);
    m_diagramsModel->addDiagram(view);

    UMLApp * pApp = UMLApp::app();
    if (pApp->listView()) {
        connect(this, SIGNAL(sigObjectRemoved(UMLObject*)), view->umlScene(), SLOT(slotObjectRemoved(UMLObject*)));
    }

    if (!m_bLoading || pApp->currentView() == nullptr) {
        pApp->setCurrentView(view);
    }
    if (!m_bLoading) {
        view->show();
        Q_EMIT sigDiagramChanged(view->umlScene()->type());
    }

    pApp->setDiagramMenuItemsState(true);
    pApp->slotUpdateViews();
}

/**
 * Removes a view from the list of currently connected views.
 *
 * @param view             Pointer to the UMLView to remove.
 * @param enforceCurrentView   Switch to determine if we have a current view or not.
 *                         Most of the time, we DO want this, except when exiting the program.
 */
void UMLDoc::removeView(UMLView *view, bool enforceCurrentView)
{
    Q_UNUSED(enforceCurrentView)
    if (!view) {
        logWarn0("UMLDoc::removeView(UMLView  *view) called with view = nullptr");
        return;
    }
    logDebug1("UMLDoc::removeView %1", view->umlScene()->name());
    if (UMLApp::app()->listView()) {
        disconnect(this, SIGNAL(sigObjectRemoved(UMLObject*)),
                   view->umlScene(), SLOT(slotObjectRemoved(UMLObject*)));
    }
    view->hide();
    UMLFolder *f = view->umlScene()->folder();
    if (f == nullptr) {
        logWarn1("UMLDoc::removeView %1 : view->getFolder() returns NULL", view->umlScene()->name());
        return;
    }
    m_diagramsModel->removeDiagram(view);
    f->removeView(view);
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView == view) {
        UMLApp::app()->setCurrentView(nullptr);
#if 0
    /* Enabling this code may result in crashes on closing models with many diagrams:
       #0  in QListData::size (this=0x30) at /usr/include/qt5/QtCore/qlist.h:115
       #1  in QList<QTreeWidgetItem*>::count (this=0x30) at /usr/include/qt5/QtCore/qlist.h:359
       #2  in QTreeWidgetItem::childCount (this=0x0) at /usr/include/qt5/QtWidgets/qtreewidget.h:193
       #3  in UMLListView::findView (this=0x13470e0, v=0x249dc80) at /umbrello/master/umbrello/umllistview.cpp:1382
       #4  in UMLApp::setCurrentView (this=0xc767c0, view=0x249dc80, updateTreeView=true)
           at /umbrello/master/umbrello/uml.cpp:3360
       #5  in UMLDoc::changeCurrentView (this=0xe0de50, id="JBFSUyhmy1cS") at /umbrello/master/umbrello/umldoc.cpp:1839
       #6  in UMLApp::slotTabChanged (this=0xc767c0, index=1) at /umbrello/master/umbrello/uml.cpp:3408
       #7  in UMLApp::qt_static_metacall (_o=0xc767c0, _c=QMetaObject::InvokeMetaMethod, _id=81, _a=0x7fffffffb5e0)
           at /umbrello/master/build/umbrello/libumbrello_autogen/EWIEGA46WW/moc_uml.cpp:564
       #8  in doActivate<false> (sender=0x126dc70, signal_index=7, argv=0x7fffffffb5e0) at kernel/qobject.cpp:3898
       #9  in QMetaObject::activate (sender=<optimized out>, m=m@entry=0x7ffff65c5560 <QTabWidget::staticMetaObject>,
           local_signal_index=local_signal_index@entry=0, argv=argv@entry=0x7fffffffb5e0) at kernel/qobject.cpp:3946
       #10 in QTabWidget::currentChanged (this=<optimized out>, _t1=<optimized out>) at .moc/moc_qtabwidget.cpp:326
       #11 in doActivate<false> (sender=0x1276980, signal_index=7, argv=0x7fffffffb6e0) at kernel/qobject.cpp:3898
       #12 in QMetaObject::activate (sender=<optimized out>, m=m@entry=0x7ffff65c3fc0 <QTabBar::staticMetaObject>,
           local_signal_index=local_signal_index@entry=0, argv=argv@entry=0x7fffffffb6e0) at kernel/qobject.cpp:3946
       #13 in QTabBar::currentChanged (this=<optimized out>, _t1=<optimized out>) at .moc/moc_qtabbar.cpp:338
       #14 in UMLApp::setCurrentView (this=0xc767c0, view=0x249dc80, updateTreeView=true)
           at /umbrello/master/umbrello/uml.cpp:3342
       #15 in UMLDoc::changeCurrentView (this=0xe0de50, id="JBFSUyhmy1cS") at /umbrello/master/umbrello/umldoc.cpp:1839
       #16 in UMLDoc::removeView (this=0xe0de50, view=0x23c0040, enforceCurrentView=false)
           at /umbrello/master/umbrello/umldoc.cpp:334
       #17 in UMLFolder::removeAllViews (this=0xe34e70) at /umbrello/master/umbrello/umlmodel/folder.cpp:242
       #18 in UMLDoc::removeAllViews (this=0xe0de50) at /umbrello/master/umbrello/umldoc.cpp:2985
       #19 in UMLDoc::closeDocument (this=0xe0de50) at /umbrello/master/umbrello/umldoc.cpp:462
       #20 in UMLDoc::newDocument (this=0xe0de50) at /umbrello/master/umbrello/umldoc.cpp:495
       #21 in UMLApp::slotFileNew (this=0xc767c0) at /umbrello/master/umbrello/uml.cpp:1371
       #22 in UMLApp::slotFileClose (this=0xc767c0) at /umbrello/master/umbrello/uml.cpp:1556
     */
        UMLViewList viewList;
        m_root[Uml::ModelType::Logical]->appendViews(viewList);
        UMLView *firstView = nullptr;
        if (!viewList.isEmpty()) {
            firstView =  viewList.first();
            /* Tried this:
            if (firstView == view)
                firstView = 0;
             *********************** but it didn't help. */
        }

        if (!firstView && enforceCurrentView) {  //create a diagram
            QString name = createDiagramName(Uml::DiagramType::Class, false);
            createDiagram(m_root[Uml::ModelType::Logical], Uml::DiagramType::Class, name);
            qApp->processEvents();
            m_root[Uml::ModelType::Logical]->appendViews(viewList);
            firstView = viewList.first();
        }

        if (firstView) {
            changeCurrentView(firstView->umlScene()->ID());
            UMLApp::app()->setDiagramMenuItemsState(true);
        }
#endif
    }
    blockSignals(true);
    delete view;
    blockSignals(false);
}

/**
 * Sets the URL of the document.
 *
 * @param url   The KUrl to set.
 */
void UMLDoc::setUrl(const QUrl &url)
{
    m_doc_url = url;
}

/**
 * Returns the KUrl of the document.
 *
 * @return  The KUrl of this UMLDoc.
 */
const QUrl& UMLDoc::url() const
{
    return m_doc_url;
}

/**
 * Sets the URL of the document to "Untitled".
 */
void UMLDoc::setUrlUntitled()
{
    m_doc_url.setUrl(m_doc_url.toString(QUrl::RemoveFilename) + i18n("Untitled"));
}

/**
 * "save modified" - Asks the user for saving if the document
 * is modified.
 *
 * @return  True if document can be closed.
 */
bool UMLDoc::saveModified()
{
    bool completed(true);
    if (!m_modified) {
        return completed;
    }

    UMLApp *win = UMLApp::app();
    int want_save = UmlMessageBox::warningYesNoCancel(win,
                                     i18n("The current file has been modified.\nDo you want to save it?"),
                                     i18nc("warning message", "Warning"),
                                     KStandardGuiItem::save(), KStandardGuiItem::discard());
    switch(want_save) {
    case UmlMessageBox::Yes:
        if (m_doc_url.fileName() == i18n("Untitled")) {
            if (win->slotFileSaveAs()) {
                closeDocument();
                completed=true;
            } else {
                completed=false;
            }
        } else {
            saveDocument(url());
            closeDocument();
            completed=true;
        }
        break;

    case UmlMessageBox::No:
        setModified(false);
        closeDocument();
        completed=true;
        break;

    case UmlMessageBox::Cancel:
        completed=false;
        break;

    default:
        completed=false;
        break;
    }
    return completed;
}

/**
 * Closes the current document.
 */
void UMLDoc::closeDocument()
{
    m_bClosing = true;
    UMLApp::app()->setGenerator(Uml::ProgrammingLanguage::Reserved);  // delete the codegen
    m_Doc = QString();
    DocWindow* dw = UMLApp::app()->docWindow();
    if (dw) {
        dw->reset();
    }
    UMLApp::app()->logWindow()->clear();

    UMLListView *listView = UMLApp::app()->listView();
    if (listView) {
        listView->clean();
        // store old setting - for restore of last setting
        bool m_bLoading_old = m_bLoading;
        m_bLoading = true; // This is to prevent document becoming modified.
        // For reference, here is an example of a call sequence that would
        // otherwise result in futile addToUndoStack() calls:
        //  removeAllViews()  =>
        //   UMLView::removeAllAssociations()  =>
        //    UMLView::removeAssoc()  =>
        //     UMLDoc::setModified(true, true)  =>
        //      addToUndoStack().
        removeAllViews();
        m_bLoading = m_bLoading_old;
        // Remove all objects from the predefined folders.
        // @fixme With advanced code generation enabled, this crashes.
        removeAllObjects();

        // Remove any stereotypes.
        if (stereotypes().count() > 0) {
            for(UMLStereotype *s : stereotypes()) {
                m_stereotypesModel->removeStereotype(s);
                //delete s;
                // This may crash when selecting File -> Close with following stacktrace:
                // #5  UMLDoc::closeDocument (this=0x2053620) at umbrello/umldoc.cpp:440
                // #6  UMLDoc::newDocument (this=0x2053620) at umbrello/umldoc.cpp:463
                // #7  UMLApp::slotFileNew (this=0x1aab900) at umbrello/uml.cpp:1239
                // #8  UMLApp::slotFileClose (this=0x1aab900) at umbrello/uml.cpp:1395
                // #9  UMLApp::qt_static_metacall (_o=0x1aab900, _c=QMetaObject::InvokeMetaMethod, _id=8, _a=0x7ffeb9ad1f10)
                //     at build/umbrello/libumbrello_autogen/EWIEGA46WW/moc_uml.cpp:490
            }
            m_stereoList.clear();
        }

        // Restore the datatype folder, it has been deleted above.
        createDatatypeFolder();
        // this creates too many items, only Logical View should be created
        listView->init();
    }

    // Clear undo stack, as there is now nothing left that could be undone
    UMLApp::app()->clearUndoStack();

    // Otherwise the bird view used an invalidated view
    UMLApp::app()->deleteBirdView();

    UMLApp::app()->enablePrint(false);

    m_bClosing = false;
}

/**
 * Initializes the document generally.
 *
 * @return  True if operation successful.
 */
bool UMLDoc::newDocument()
{
    bool state = UMLApp::app()->document()->loading();
    UMLApp::app()->document()->setLoading(true);

    closeDocument();
    UMLApp::app()->setCurrentView(nullptr);
    setUrlUntitled();
    setResolution(Dialog_Utils::logicalDpiXForWidget());
    //see if we need to start with a new diagram
    Settings::OptionState optionState = Settings::optionState();
    Uml::DiagramType::Enum dt = optionState.generalState.diagram;
    Uml::ModelType::Enum mt = Model_Utils::convert_DT_MT(dt);
    if (mt == Uml::ModelType::N_MODELTYPES) {  // don't allow no diagram
        dt = Uml::DiagramType::Class;
        mt = Uml::ModelType::Logical;
    }
    QString name = createDiagramName(dt, false);
    createDiagram(m_root[mt], dt, name);

    UMLApp::app()->initGenerator();

    setModified(false);
    initSaveTimer();

    UMLApp::app()->enableUndoAction(false);
    UMLApp::app()->clearUndoStack();

    UMLApp::app()->document()->setLoading(state);
    return true;
}

/**
 * Loads the document by filename and format and emits the
 * updateViews() signal.
 *
 * @param url      The filename in KUrl format.
 * @param format   The format (optional.)
 * @return  True if operation successful.
 */
bool UMLDoc::openDocument(const QUrl& url, const char *format /* = nullptr */)
{
    Q_UNUSED(format);
    if (url.fileName().length() == 0) {
        newDocument();
        return false;
    }

    m_doc_url = url;
    closeDocument();
    setResolution(0.0);
    // IMPORTANT: set m_bLoading to true
    // _AFTER_ the call of UMLDoc::closeDocument()
    // as it sets m_bLoading to false after it was temporarily
    // changed to true to block recording of changes in redo-buffer
    m_bLoading = true;
    m_d->errors.clear();
    QTemporaryFile tmpfile;
    tmpfile.open();
    QUrl dest(QUrl::fromLocalFile(tmpfile.fileName()));
    logDebug2("UMLDoc::openDocument: copy from %1 to %2", url.path(), dest.path());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    KIO::Job* job = KIO::copy(url, dest, KIO::Overwrite);
#else
    KIO::FileCopyJob *job = KIO::file_copy(url, dest, -1, KIO::Overwrite);
#endif
    KJobWidgets::setWindow(job, UMLApp::app());
    job->exec();
    QFile file(tmpfile.fileName());
    if (job->error() || !file.exists()) {
        if (!file.exists())
            logDebug1("UMLDoc::openDocument: temporary file <%1> failed", file.fileName());
        if (job->error())
           logDebug1("UMLDoc::openDocument: %1", job->errorString());
        KMessageBox::error(nullptr, i18n("The file <%1> does not exist.", url.toString()), i18n("Load Error"));
        setUrlUntitled();
        m_bLoading = false;
        newDocument();
        return false;
    }
    // status of XMI loading
    bool status = false;

    // check if the xmi file is a compressed archive like tar.bzip2 or tar.gz
    QString filetype = m_doc_url.fileName();
    QString mimetype;
    if (filetype.endsWith(QStringLiteral(".tgz")) || filetype.endsWith(QStringLiteral(".tar.gz"))) {
        mimetype = QStringLiteral("application/x-gzip");
    } else if (filetype.endsWith(QStringLiteral(".tar.bz2"))) {
        mimetype = QStringLiteral("application/x-bzip");
    }

    if (mimetype.isEmpty() == false) {
        KTar archive(file.fileName(), mimetype);
        if (archive.open(QIODevice::ReadOnly) == false) {
            KMessageBox::error(nullptr, i18n("The file %1 seems to be corrupted.", url.toString()), i18n("Load Error"));
            setUrlUntitled();
            m_bLoading = false;
            newDocument();
            return false;
        }

        // get the root directory and all entries in
        const KArchiveDirectory * rootDir = archive.directory();
        const QStringList entries = rootDir->entries();
        QString entryMimeType;
        bool foundXMI = false;
        QStringList::ConstIterator it;
        QStringList::ConstIterator end(entries.end());

        // now go through all entries till we find an xmi file
        for (it = entries.begin(); it != end; ++it) {
            // only check files, we do not go in subdirectories
            if (rootDir->entry(*it)->isFile() == true) {
                // we found a file, check the mimetype
                QMimeDatabase db;
                entryMimeType = db.mimeTypeForFile(*it, QMimeDatabase::MatchExtension).name();
                if (entryMimeType == QStringLiteral("application/x-uml")) {
                    foundXMI = true;
                    break;
                }
            }
        }

        // if we found an XMI file, we have to extract it to a temporary file
        if (foundXMI == true) {
            QTemporaryDir tmp_dir;
            KArchiveEntry * entry;
            KArchiveFile * fileEntry;

            // try to cast the file entry in the archive to an archive entry
            entry = const_cast<KArchiveEntry*>(rootDir->entry(*it));
            if (entry == nullptr) {
                KMessageBox::error(nullptr, i18n("There was no XMI file found in the compressed file %1.", url.toString()),
                                   i18n("Load Error"));
                setUrlUntitled();
                m_bLoading = false;
                newDocument();
                return false;
            }

            // now try to cast the archive entry to a file entry, so that we can
            // extract the file
            fileEntry = dynamic_cast<KArchiveFile*>(entry);
            if (fileEntry == nullptr) {
                KMessageBox::error(nullptr, i18n("There was no XMI file found in the compressed file %1.", url.toString()),
                                   i18n("Load Error"));
                setUrlUntitled();
                m_bLoading = false;
                newDocument();
                return false;
            }

            // now we can extract the file to the temporary directory
            fileEntry->copyTo(tmp_dir.path() + QLatin1Char('/'));

            // now open the extracted file for reading
            QFile xmi_file(tmp_dir.path() + QLatin1Char('/') + *it);
            if(!xmi_file.open(QIODevice::ReadOnly)) {
                KMessageBox::error(nullptr, i18n("There was a problem loading the extracted file: %1", url.toString()),
                                   i18n("Load Error"));
                setUrlUntitled();
                m_bLoading = false;
                newDocument();
                return false;
            }
            m_bTypesAreResolved = false;
            status = loadFromXMI(xmi_file, ENC_UNKNOWN);

            // close the extracted file and the temporary directory
            xmi_file.close();
        } else {
                KMessageBox::error(nullptr, i18n("There was no XMI file found in the compressed file %1.", url.toString()),
                                   i18n("Load Error"));
            setUrlUntitled();
            m_bLoading = false;
            newDocument();
            return false;
        }

        archive.close();
    } else {
        // no, it seems to be an ordinary file
        if (!file.open(QIODevice::ReadOnly)) {
            KMessageBox::error(nullptr, i18n("There was a problem loading file: %1", url.toString()),
                               i18n("Load Error"));
            setUrlUntitled();
            m_bLoading = false;
            newDocument();
            return false;
        }
        if (filetype.endsWith(QStringLiteral(".mdl"))) {
            setUrlUntitled();
            m_bTypesAreResolved = false;
            status = Import_Rose::loadFromMDL(file);
            // qApp->processEvents();  // give UI events a chance
            // activateAllViews();
            if (status) {
                if (UMLApp::app()->currentView() == nullptr) {
                    QString name = createDiagramName(Uml::DiagramType::Class, false);
                    createDiagram(m_root[Uml::ModelType::Logical], Uml::DiagramType::Class, name);
                    setCurrentRoot(Uml::ModelType::Logical);
                }
            }
        }
        else if (filetype.endsWith(QStringLiteral(".zargo"))) {
            setUrlUntitled();
            status = Import_Argo::loadFromZArgoFile(file);
        }
        else {
            m_bTypesAreResolved = false;
            status = loadFromXMI(file, ENC_UNKNOWN);
        }
    }

    if (file.isOpen())
        file.close();
    m_bLoading = false;
    m_bTypesAreResolved = true;
    if (!status) {
        QString msg = i18n("There was a problem loading file: %1", url.toString());
        if (m_d->errors.size() > 0)
            msg += QStringLiteral("<br/>") + i18n("Reason: %1", m_d->errors.join(QStringLiteral("<br/>")));
        KMessageBox::error(nullptr, msg, i18n("Load Error"));
        newDocument();
        return false;
    }
    setModified(false);
    initSaveTimer();

    UMLApp::app()->enableUndoAction(false);
    UMLApp::app()->clearUndoStack();
    // for compatibility
    addDefaultStereotypes();

    return true;
}

/**
 * Saves the document using the given filename and format.
 *
 * @param url      The filename in KUrl format.
 * @param format   The format (optional.)
 * @return  True if operation successful.
 */
bool UMLDoc::saveDocument(const QUrl& url, const char * format)
{
    Q_UNUSED(format);
    m_doc_url = url;
    bool uploaded = true;

    // first, we have to find out which format to use
    QString strFileName = url.path();
    QFileInfo fileInfo(strFileName);
    QString fileExt = fileInfo.completeSuffix();
    QString fileFormat = QStringLiteral("xmi");
    if (fileExt == QStringLiteral("xmi") || fileExt == QStringLiteral("bak.xmi")) {
        fileFormat = QStringLiteral("xmi");
    } else if (fileExt == QStringLiteral("xmi.tgz") || fileExt == QStringLiteral("bak.xmi.tgz")) {
        fileFormat = QStringLiteral("tgz");
    } else if (fileExt == QStringLiteral("xmi.tar.bz2") || fileExt == QStringLiteral("bak.xmi.tar.bz2")) {
        fileFormat = QStringLiteral("bz2");
    } else {
        fileFormat = QStringLiteral("xmi");
    }

    initSaveTimer();

    if (fileFormat == QStringLiteral("tgz") || fileFormat == QStringLiteral("bz2")) {
        KTar * archive;
        QTemporaryFile tmp_tgz_file;
        tmp_tgz_file.setAutoRemove(false);
        tmp_tgz_file.open();

        // first we have to check if we are saving to a local or remote file
        if (url.isLocalFile()) {
            if (fileFormat == QStringLiteral("tgz")) {  // check tgz or bzip
                archive = new KTar(url.toLocalFile(), QStringLiteral("application/x-gzip"));
            } else {
                archive = new KTar(url.toLocalFile(), QStringLiteral("application/x-bzip"));
            }
        } else {
            if (fileFormat == QStringLiteral("tgz")) {  // check tgz or bzip2
                archive = new KTar(tmp_tgz_file.fileName(), QStringLiteral("application/x-gzip"));
            } else {
                archive = new KTar(tmp_tgz_file.fileName(), QStringLiteral("application/x-bzip"));
            }
        }

        // now check if we can write to the file
        if (archive->open(QIODevice::WriteOnly) == false) {
            logError1("UMLDoc::saveDocument could not open %1", archive->fileName());
            KMessageBox::error(nullptr, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
            delete archive;
            return false;
        }

        // we have to create a temporary xmi file
        // we will add this file later to the archive
        QTemporaryFile tmp_xmi_file;
        tmp_xmi_file.setAutoRemove(false);
        if (!tmp_xmi_file.open()) {
            logError1("UMLDoc::saveDocument could not open %1", tmp_xmi_file.fileName());
            KMessageBox::error(nullptr, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
            delete archive;
            return false;
        }
        saveToXMI(tmp_xmi_file); // save XMI to this file...

        // now add this file to the archive, but without the extension
        QString tmpQString = url.fileName();
        if (fileFormat == QStringLiteral("tgz")) {
            tmpQString.remove(QRegularExpression(QStringLiteral("\\.tgz$")));
        }
        else {
            tmpQString.remove(QRegularExpression(QStringLiteral("\\.tar\\.bz2$")));
        }
        archive->addLocalFile(tmp_xmi_file.fileName(), tmpQString);

        if (!archive->close()) {
            logError1("UMLDoc::saveDocument could not close %1", archive->fileName());
            KMessageBox::error(nullptr, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
            delete archive;
            return false;
        }
        // now the xmi file was added to the archive, so we can delete it
        tmp_xmi_file.setAutoRemove(true);

        // now we have to check, if we have to upload the file
        if (!url.isLocalFile()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            KIO::Job* job = KIO::copy(QUrl::fromLocalFile(tmp_tgz_file.fileName()), m_doc_url);
#else
            KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(tmp_tgz_file.fileName()), m_doc_url);
#endif
            KJobWidgets::setWindow(job, UMLApp::app());
            job->exec();
            uploaded = !job->error();
            if (!uploaded) {
                logError2("UMLDoc::saveDocument could not upload file %1 to %2", tmp_tgz_file.fileName(),
                          url.toString());
            }
        }

        // now the archive was written to disk (or remote) so we can delete the
        // objects
        tmp_tgz_file.setAutoRemove(true);
        delete archive;

    }
    else {
        // save as normal uncompressed XMI

        QTemporaryFile tmpfile; // we need this tmp file if we are writing to a remote file
        tmpfile.setAutoRemove(false);

        // save in _any_ case to a temp file
        // -> if something goes wrong during saveToXMI, the
        //     original content is preserved
        //     (e.g. if umbrello dies in the middle of the document model parsing
        //      for saveToXMI due to some problems)
        /// @todo insert some checks in saveToXMI to detect a failed save attempt

        // lets open the file for writing
        if (!tmpfile.open()) {
            logError1("UMLDoc::saveDocument could not open %1", tmpfile.fileName());
            KMessageBox::error(nullptr, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
            return false;
        }
        saveToXMI(tmpfile); // save the xmi stuff to it
        tmpfile.close();

        // if it is a remote file, we have to upload the tmp file
        if (!url.isLocalFile()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            KIO::Job* job = KIO::copy(QUrl::fromLocalFile(tmpfile.fileName()), m_doc_url);
#else
            KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(tmpfile.fileName()), m_doc_url);
#endif
            KJobWidgets::setWindow(job, UMLApp::app());
            job->exec();
            uploaded = !job->error();
            if (!uploaded)
                logError2("UMLDoc::saveDocument could not upload file %1 to %2", tmpfile.fileName(), url.toString());
        }
        else {
            // now remove the original file
#ifdef Q_OS_WIN
            tmpfile.setAutoRemove(true);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            KIO::Job* job = KIO::copy(QUrl::fromLocalFile(tmpfile.fileName()), url, KIO::Overwrite);
#else
            KIO::FileCopyJob* fcj = KIO::file_copy(QUrl::fromLocalFile(tmpfile.fileName()), url, -1, KIO::Overwrite);
#endif
#else
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            KIO::Job* fcj = KIO::move(QUrl::fromLocalFile(tmpfile.fileName()), url, KIO::Overwrite);
#else
            KIO::FileCopyJob* fcj = KIO::file_move(QUrl::fromLocalFile(tmpfile.fileName()), url, -1, KIO::Overwrite);
#endif
#endif
            KJobWidgets::setWindow(fcj, (QWidget*)UMLApp::app());
            fcj->exec();
            if (fcj->error()) {
                logError2("UMLDoc::saveDocument Could not move %1 to %2", tmpfile.fileName(), url.toString());
                KMessageBox::error(nullptr, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
                setUrlUntitled();
                return false;
            }
        }
    }
    if (!uploaded) {
        KMessageBox::error(nullptr, i18n("There was a problem uploading: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
        setUrlUntitled();
    }
    setModified(false);
    return uploaded;
}

/**
 * Sets up the signals needed by the program for it to work.
 */
void UMLDoc::setupSignals()
{
    WorkToolBar *tb = UMLApp::app()->workToolBar();
    connect(this, SIGNAL(sigDiagramChanged(Uml::DiagramType::Enum)), tb, SLOT(slotCheckToolBar(Uml::DiagramType::Enum)));
}

/**
 * Finds a view (diagram) by the ID given to method.
 *
 * @param id   The ID of the view to search for.
 * @return  Pointer to the view found, or NULL if not found.
 */
UMLView * UMLDoc::findView(Uml::ID::Type id) const
{
    UMLView  *v = nullptr;
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        v = m_root[i]->findView(id);
        if (v) {
            break;
        }
    }
    return v;
}

/**
 * Finds a view (diagram) by the type and name given.
 *
 * @param type            The type of view to find.
 * @param name            The name of the view to find.
 * @param searchAllScopes Search in all subfolders (default: false.)
 * @return  Pointer to the view found, or NULL if not found.
 */
UMLView * UMLDoc::findView(Uml::DiagramType::Enum type, const QString &name,
                           bool searchAllScopes /* =false */) const
{
    Uml::ModelType::Enum mt = Model_Utils::convert_DT_MT(type);
    if (mt == Uml::ModelType::N_MODELTYPES) {
        logWarn1("UMLDoc::findView : Returning null because DiagramType %1 cannot be mapped to ModelType", type);
        return nullptr;
    }
    return m_root[mt]->findView(type, name, searchAllScopes);
}

/**
 * Used to find a reference to a @ref UMLObject by its ID.
 *
 * @param id   The @ref UMLObject to find.
 * @return  Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* UMLDoc::findObjectById(Uml::ID::Type id)
{
    UMLObject  *o = nullptr;
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        if (id == m_root[i]->id()) {
            return m_root[i];
        }
        o = m_root[i]->findObjectById(id);
        if (o) {
            return o;
        }
    }
    o = findStereotypeById(id);
    return o;
}

/**
 * Used to find a @ref UMLObject by its type and name.
 *
 * @param name         The name of the @ref UMLObject to find.
 * @param type         ObjectType of the object to find (optional.)
 *                     When the given type is ot_UMLObject the type is
 *                     disregarded, i.e. the given name is the only
 *                     search criterion.
 * @param currentObj   Object relative to which to search (optional.)
 *                     If given then the enclosing scope(s) of this
 *                     object are searched before the global scope.
 * @return  Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* UMLDoc::findUMLObject(const QString &name,
                                 UMLObject::ObjectType type /* = ot_UMLObject */,
                                 UMLObject  *currentObj /* = nullptr */)
{
    UMLObject *o = m_datatypeRoot->findObject(name);
    if (o) {
        return o;
    }
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        UMLObjectList list = m_root[i]->containedObjects();
        if (list.size() == 0)
            continue;
        o = Model_Utils::findUMLObject(list, name, type, currentObj);
        if (o) {
            return o;
        }
        if ((type == UMLObject::ot_UMLObject || type == UMLObject::ot_Folder) &&
             name == m_root[i]->name()) {
            return m_root[i];
        }
    }
    return nullptr;
}

/**
 * Used to find a @ref UMLObject by its type and raw name.
 *
 * @param modelType    The model type in which to search for the object
 * @param name         The raw name of the @ref UMLObject to find.
 * @param type         ObjectType of the object to find
 * @return  Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* UMLDoc::findUMLObjectRaw(Uml::ModelType::Enum modelType,
                                    const QString &name,
                                    UMLObject::ObjectType type)
{
    return findUMLObjectRaw(rootFolder(modelType), name, type);
}

/**
 * Used to find a @ref UMLObject by its type and raw name.
 *
 * @param folder       The UMLFolder in which to search for the object
 * @param name         The raw name of the @ref UMLObject to find.
 * @param type         ObjectType of the object to find
 * @return  Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* UMLDoc::findUMLObjectRaw(UMLFolder *folder,
                                    const QString &name,
                                    UMLObject::ObjectType type)
{
    if (folder == nullptr)
        return nullptr;
    UMLObjectList list = folder->containedObjects();
    if (list.size() == 0)
        return nullptr;
    return Model_Utils::findUMLObjectRaw(list, name, type, nullptr);
}

/**
 * Used to find a @ref UMLObject by its type and raw name recursively
 *
 * @param modelType    The model type in which to search for the object
 * @param name         The raw name of the @ref UMLObject to find.
 * @param type         ObjectType of the object to find
 * @return  Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* UMLDoc::findUMLObjectRecursive(Uml::ModelType::Enum modelType,
                                          const QString &name,
                                          UMLObject::ObjectType type)
{
    return findUMLObjectRecursive(rootFolder(modelType), name, type);
}

/**
 * Used to find a @ref UMLObject by its type and raw name recursively
 *
 * @param folder       The UMLFolder in which to search for the object
 * @param name         The raw name of the @ref UMLObject to find.
 * @param type         ObjectType of the object to find
 * @return  Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* UMLDoc::findUMLObjectRecursive(UMLFolder *folder,
                                          const QString &name,
                                          UMLObject::ObjectType type)
{
    if (folder == nullptr)
        return nullptr;
    UMLObjectList list = folder->containedObjects();
    if (list.size() == 0)
        return nullptr;
    return Model_Utils::findUMLObjectRecursive(list, name, type);
}

/**
 * Used to find a @ref UMLClassifier by its name.
 *
 * @param name   The name of the @ref UMLObject to find.
 */
UMLClassifier* UMLDoc::findUMLClassifier(const QString &name)
{
    //this is used only by code generator so we don't need to look at Datatypes
    UMLObject * obj = findUMLObject(name);
    return obj->asUMLClassifier();
}

/**
 * Adds a UMLObject that is already created but doesn't change
 * any ids or signal.  Use AddUMLObjectPaste if pasting.
 *
 * @param object   The object to add.
 * @return  True if the object was actually added.
 */
bool UMLDoc::addUMLObject(UMLObject* object)
{
    if (object->isUMLStereotype()) {
        logDebug2("UMLDoc::addUMLObject %1: not adding type %2",
                  object->name(), object->baseTypeStr());
        return false;
    }
    UMLPackage *pkg = object->umlPackage();
    if (pkg == nullptr) {
        pkg = currentRoot();
        logDebug2("UMLDoc::addUMLObject %1: no parent package set, assuming %2",
                 object->name(), pkg->name());
        object->setUMLPackage(pkg);
    }

    // FIXME restore stereotype
    UMLClassifierListItem *c = object->asUMLClassifierListItem();
    if (c) {
        if (!pkg->subordinates().contains(c))
            pkg->subordinates().append(c);
        return true;
    }
    return pkg->addObject(object);
}

/**
 * Write text to the status bar.
 * @param text   the text to write
 */
void UMLDoc::writeToStatusBar(const QString &text)
{
    Q_EMIT sigWriteToStatusBar(text);
}

/**
 * Simple removal of an object.
 * @param object   the UMLObject to be removed
 */
void UMLDoc::slotRemoveUMLObject(UMLObject* object)
{
    //m_objectList.remove(object);
    UMLPackage *pkg = object->umlPackage();
    if (pkg == nullptr) {
        logError1("UMLDoc::slotRemoveUMLObject %1 : parent package is not set", object->name());
        return;
    }
    pkg->removeObject(object);
}

/**
 * Returns true if the given name is unique within its scope.
 *
 * @param name   The name to check.
 * @return  True if name is unique.
 */
bool UMLDoc::isUnique(const QString &name) const
{
    UMLListView *listView = UMLApp::app()->listView();
    UMLListViewItem *currentItem = (UMLListViewItem*)listView->currentItem();
    UMLListViewItem  *parentItem = nullptr;

    // check for current item, if its a package, then we do a check on that
    // otherwise, if current item exists, find its parent and check if thats
    // a package..
    if (currentItem) {
        // its possible that the current item *is* a package, then just
        // do check now
        if (Model_Utils::typeIsContainer(currentItem->type())) {
            return isUnique (name, (UMLPackage*) currentItem->umlObject());
        }
        parentItem = (UMLListViewItem*)currentItem->parent();
    }

    // item is in a package so do check only in that
    if (parentItem != nullptr && Model_Utils::typeIsContainer(parentItem->type())) {
        UMLPackage *parentPkg = parentItem->umlObject()->asUMLPackage();
        return isUnique(name, parentPkg);
    }

    logError1("UMLDoc::isUnique %1: Not currently in a package", name);
    /* Check against all objects that _don't_ have a parent package.
    for (UMLObjectListIt oit(m_objectList); oit.current(); ++oit) {
        UMLObject *obj = oit.current();
        if ((obj->getUMLPackage() == 0) && (obj->getName() == name))
            return false;
    }
     */
    return true;
}

/**
 * Returns true if the given name is unique within its scope of given package.
 *
 * @param name      The name to check.
 * @param package   The UMLPackage in which we have to determine the unique-ness
 * @return      True if name is unique.
 */
bool UMLDoc::isUnique(const QString &name, UMLPackage *package) const
{
    // if a package, then only do check in that
    if (package) {
        return (package->findObject(name) == nullptr);
    }

    // Not currently in a package: ERROR
    logError1("UMLDoc::isUnique %1 (2): Not currently in a package", name);
    /* Check against all objects that _don't_ have a parent package.
    for (UMLObjectListIt oit(m_objectList); oit.current(); ++oit) {
        UMLObject *obj = oit.current();
        if ((obj->getUMLPackage() == 0) && (obj->getName() == name))
            return false;
    }
     */
    return true;
}

/**
 * Creates a stereotype for the parent object.
 * @param name   the name of the stereotype
 */
UMLStereotype* UMLDoc::createStereotype(const QString &name)
{
    UMLStereotype *s = new UMLStereotype(name, Uml::ID::fromString(name));
    addStereotype(s);
    return s;
}

/**
 * Finds a UMLStereotype by its name.
 *
 * @param name   The name of the UMLStereotype to find.
 * @return  Pointer to the UMLStereotype found, or NULL if not found.
 */
UMLStereotype* UMLDoc::findStereotype(const QString &name) const
{
    for(UMLStereotype *s : m_stereoList) {
        if (s->name() == name) {
            return s;
        }
    }
    return nullptr;
}

/**
 * Finds or creates a stereotype for the parent object.
 * @param name   the name of the stereotype
 * @return the found stereotype object or a just created one
 */
UMLStereotype* UMLDoc::findOrCreateStereotype(const QString &name)
{
    UMLStereotype *s = findStereotype(name);
    if (s != nullptr) {
        return s;
    }
    return createStereotype(name);
}

/**
 * Find a UMLStereotype by its unique ID.
 * @param id   the unique ID
 * @return the found stereotype or NULL
 */
UMLStereotype * UMLDoc::findStereotypeById(Uml::ID::Type id) const
{
    for(UMLStereotype *s : m_stereoList) {
        if (s->id() == id)
            return s;
    }
    return nullptr;
}

/**
 * Add a UMLStereotype to the application.
 * @param s  the stereotype to be added
 */
void UMLDoc::addStereotype(UMLStereotype *s)
{
    if (m_stereotypesModel->addStereotype(s))
        Q_EMIT sigObjectCreated(s);
}

/**
 * Remove a UMLStereotype from the application.
 * @param s   the stereotype to be removed
 */
void UMLDoc::removeStereotype(UMLStereotype *s)
{
    if (m_stereotypesModel->removeStereotype(s))
        Q_EMIT sigObjectRemoved(s);
}

/**
 * Add a stereotype if it doesn't already exist.
 * Used by code generators, operations and attribute dialog.
 */
void UMLDoc::addDefaultStereotypes()
{
    CodeGenerator *gen = UMLApp::app()->generator();
    if (gen) {
        gen->createDefaultStereotypes();
    }
}

/**
 * Returns a list of the stereotypes in this UMLDoc.
 *
 * @return  List of UML stereotypes.
 */
const UMLStereotypeList& UMLDoc::stereotypes() const
{
    return m_stereoList;
}

/**
 * Removes an association.
 *
 * @param assoc    Pointer to the UMLAssociation to remove.
 * @param doSetModified  Whether to mark the document as modified (default: true.)
 */
void UMLDoc::removeAssociation (UMLAssociation * assoc, bool doSetModified /*=true*/)
{
    if (!assoc) {
        return;
    }

    // Remove the UMLAssociation from m_objectList.
    UMLPackage *pkg = assoc->umlPackage();
    if (pkg == nullptr) {
        logError1("UMLDoc::removeAssociation %1: parent package is not set", assoc->name());
        return;
    }
    pkg->removeObject(assoc);

    if (doSetModified) {  // so we will save our document
        setModified(true);
    }
}

/**
 * Finds an association.
 *
 * @param assocType Type of the UMLAssociation to seek.
 * @param roleAObj  Pointer to the role A UMLCanvasObject.
 * @param roleBObj  Pointer to the role B UMLCanvasObject.
 * @param swap      Optional pointer to boolean.
 *                  The bool is set to true if the association
 *                  matched with swapped roles, else it is set
 *                  to false.
 * @return  Pointer to the UMLAssociation found or NULL if not found.
 */
UMLAssociation * UMLDoc::findAssociation(Uml::AssociationType::Enum assocType,
        const UMLObject *roleAObj,
        const UMLObject *roleBObj,
        bool *swap) const
{
    UMLAssociationList assocs = associations();
    UMLAssociation *ret = nullptr;
    for(UMLAssociation *a : assocs) {
        if (a->getAssocType() != assocType) {
            continue;
        }
        if (a->getObject(Uml::RoleType::A) == roleAObj && a->getObject(Uml::RoleType::B) == roleBObj) {
            return a;
        }
        if (a->getObject(Uml::RoleType::A) == roleBObj && a->getObject(Uml::RoleType::B) == roleAObj) {
            ret = a;
        }
    }
    if (swap) {
        *swap = (ret != nullptr);
    }
    return ret;
}

/**
 * Creates AND adds an association between two UMLObjects.
 * Used by refactoring assistant.
 * NOTE: this method does not check if the association is valid / legal
 *
 * @param a      The UMLObject "A" for the association (source)
 * @param b      The UMLObject "B" for the association (destination)
 * @param type   The association's type
 * @return  The Association created
 */
UMLAssociation* UMLDoc::createUMLAssociation(UMLObject *a, UMLObject *b,
                                             Uml::AssociationType::Enum type)
{
    bool swap;
    UMLAssociation *assoc = findAssociation(type, a, b, &swap);
    if (assoc == nullptr) {
        assoc = new UMLAssociation(type, a, b);
        assoc->setUMLPackage(a->umlPackage());
        addAssociation(assoc);
    }
    return assoc;
}

/**
 * Adds an association.
 *
 * @param assoc    Pointer to the UMLAssociation to add.
 */
void UMLDoc::addAssociation(UMLAssociation *assoc)
{
    if (assoc == nullptr) {
        return;
    }

    // First, check that this association has not already been added.
    // This may happen when loading old XMI files where all the association
    // information was taken from the <UML:AssocWidget> tag.
    UMLAssociationList assocs = associations();
    for(UMLAssociation *a :  assocs) {
        // check if its already been added (shouldn't be the case right now
        // as UMLAssociations only belong to one associationwidget at a time)
        if (a == assoc) {
            logDebug1("UMLDoc::addAssociation(%1) duplicate addition attempted",
                      assoc->name());
            return;
        }
    }

    // If we get here it's really a new association.

    // Add the UMLAssociation at the owning UMLPackage.
    UMLPackage *pkg = assoc->umlPackage();
    if (pkg == nullptr) {
        logError1("UMLDoc::addAssociation %1: parent package is not set", assoc->name());
        return;
    }
    pkg->addObject(assoc);

    // I don't believe this appropriate, UMLAssociations ARENT UMLWidgets -b.t.
    // Q_EMIT sigObjectCreated(o);

    setModified(true);
}

/**
 * Returns a name for the new object, appended with a number
 * if the default name is taken e.g. class diagram, class
 * diagram_1 etc.
 * @param type   the diagram type
 * @return the unique view name
 */
QString UMLDoc::uniqueViewName(const Uml::DiagramType::Enum type) const
{
    QString dname;
    switch (type) {
        case Uml::DiagramType::UseCase:
            dname = i18n("use case diagram");
            break;
        case Uml::DiagramType::Class:
            dname = i18n("class diagram");
            break;
        case Uml::DiagramType::Sequence:
            dname = i18n("sequence diagram");
            break;
        case Uml::DiagramType::Collaboration:
            dname = i18n("communication diagram");
            break;
        case Uml::DiagramType::Object:
            dname = i18n("object diagram");
            break;
        case Uml::DiagramType::State:
            dname = i18n("state diagram");
            break;
        case Uml::DiagramType::Activity:
            dname = i18n("activity diagram");
            break;
        case Uml::DiagramType::Component:
            dname = i18n("component diagram");
            break;
        case Uml::DiagramType::Deployment:
            dname = i18n("deployment diagram");
            break;
        case Uml::DiagramType::EntityRelationship:
            dname = i18n("entity relationship diagram");
            break;
        default:
            logWarn0("UMLDoc::uniqueViewName called with unknown diagram type");
            break;
    }
    QString name = dname;
    for (int number = 1; findView(type, name, true); ++number) {
        name = dname + QLatin1Char('_') + QString::number(number);
    }
    return name;
}

/**
 * Returns true when loading a document file.
 * @return the value of the flag
 */
bool UMLDoc::loading() const
{
    return m_bLoading || !m_bTypesAreResolved;
}

/**
 * Sets loading boolean flag to the value given.
 * @param state   value to set
 */
void UMLDoc::setLoading(bool state /* = true */)
{
    m_bLoading = state;
}

/**
 * Returns true when importing file(s).
 * @return the value of the flag
 */
bool UMLDoc::importing() const
{
    return m_importing;
}
/**
 * Sets importing boolean flag to the value given.
 * @param state   value to set
 */
void UMLDoc::setImporting(bool state /* = true */)
{
    m_importing = state;
}

/**
 * Returns the m_bClosing flag.
 * @return the value of the flag
 */
bool UMLDoc::closing() const
{
    return m_bClosing;
}

/**
 * Creates the name of the given diagram type.
 * @param type         The type of diagram to create.
 * @param askForName   If true shows a dialog box asking for name,
 *                     else uses a default name.
 * @return             name of the new diagram
 */
QString UMLDoc::createDiagramName(Uml::DiagramType::Enum type, bool askForName /*= true */)
{
    QString defaultName = uniqueViewName(type);
    QString name = defaultName;

    while (true) {
        if (askForName && !Dialog_Utils::askName(i18nc("diagram name", "Name"), i18n("Enter name:"), name))
            break;

        if (name.length() == 0)  {
            KMessageBox::error(nullptr, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
        } else if (findView(type, name)) {
                KMessageBox::error(nullptr, i18n("A diagram is already using that name."), i18n("Not a Unique Name"));
        } else {
            return name;
        }
    } // end while
    return QString();
}

/**
 * Creates a diagram of the given type.
 *
 * @param folder   the folder in which tp create the diagram.
 * @param type     the type of diagram to create
 * @param name     the name for the diagram to create
 * @param id       optional ID of new diagram
 * @return         pointer to the UMLView of the new diagram
 */
UMLView* UMLDoc::createDiagram(UMLFolder *folder, Uml::DiagramType::Enum type, const QString& name, Uml::ID::Type id)
{
    logDebug3("UMLDoc::createDiagram folder=%1 / type=%2 / name=%3",
             folder->name(), Uml::DiagramType::toString(type), name);

    if (id == Uml::ID::None) {
        id = UniqueID::gen();
    }

    if (name.length() > 0) {
        UMLView* view = new UMLView(folder);
        view->umlScene()->setOptionState(Settings::optionState());
        view->umlScene()->setName(name);
        view->umlScene()->setType(type);
        view->umlScene()->setID(id);
        addView(view);
        Q_EMIT sigDiagramCreated(id);
        setModified(true);
        UMLApp::app()->enablePrint(true);
        changeCurrentView(id);
        return view;
    }
    return nullptr;
}

/**
 * Used to rename a document. This method takes care of everything.
 * You just need to give the ID of the diagram to the method.
 *
 * @param id   The ID of the diagram to rename.
 */
void UMLDoc::renameDiagram(Uml::ID::Type id)
{
    UMLView *view = findView(id);
    Uml::DiagramType::Enum type = view->umlScene()->type();

    QString name = view->umlScene()->name();
    while (true) {
        bool ok = Dialog_Utils::askName(i18nc("renaming diagram", "Name"),
                                        i18n("Enter name:"),
                                        name);
        if (!ok) {
            break;
        }
        if (name.length() == 0) {
            KMessageBox::error(nullptr, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
        } else if (!findView(type, name)) {
            view->umlScene()->setName(name);
            Q_EMIT sigDiagramRenamed(id);
            setModified(true);
            break;
        } else {
            KMessageBox::error(nullptr, i18n("A diagram is already using that name."), i18n("Not a Unique Name"));
        }
    }
}

/**
 * Used to rename a @ref UMLObject.  The @ref UMLObject is to be an
 * actor, use case or classifier.
 *
 * @param o The object to rename.
 */
void UMLDoc::renameUMLObject(UMLObject *o)
{
    QString name = o->name();
    while (true) {
        bool ok = Dialog_Utils::askName(i18nc("renaming uml object", "Name"),
                                        i18n("Enter name:"),
                                        name);
        if (!ok) {
            break;
        }
        if (name.length() == 0) {
            KMessageBox::error(nullptr, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (isUnique(name)) {
            UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(o, name));
            setModified(true);
            break;
        } else {
            KMessageBox::error(nullptr, i18n("That name is already being used."), i18n("Not a Unique Name"));
        }
    }
    return;
}

/**
 * Used to rename an operation or attribute of a classifier.
 *
 * @param o The attribute or operation to rename.
 */
void UMLDoc::renameChildUMLObject(UMLObject *o)
{
    UMLClassifier* p = o->umlParent()->asUMLClassifier();
    if (!p) {
        logDebug1("UMLDoc::renameChildUMLObject: Cannot process object, invalid parent %1",
                  o->umlParent()->name());
        return;
    }

    QString name = o->name();
    while (true) {
        bool ok = Dialog_Utils::askName(i18nc("renaming child uml object", "Name"),
                                        i18n("Enter name:"),
                                        name);
        if (!ok) {
            break;
        }
        if (name.length() == 0) {
            KMessageBox::error(nullptr, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (p->findChildObject(name) == nullptr
                    || ((o->baseType() == UMLObject::ot_Operation) && UmlMessageBox::warningYesNo(nullptr,
                            i18n("The name you entered was not unique.\nIs this what you wanted?"),
                            i18n("Name Not Unique"), KGuiItem(i18n("Use Name")), KGuiItem(i18n("Enter New Name"))) == UmlMessageBox::Yes)) {
                UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(o, name));
                setModified(true);
                break;
        } else {
            KMessageBox::error(nullptr, i18n("That name is already being used."), i18n("Not a Unique Name"));
        }
    }
}

/**
 * Changes the current view (diagram) to the view with the given ID.
 *
 * @param id   The ID of the view to change to.
 */
void UMLDoc::changeCurrentView(Uml::ID::Type id)
{
    logDebug1("UMLDoc::changeCurrentView id=%1", Uml::ID::toString(id));
    UMLView* view = findView(id);
    if (view) {
        UMLScene* scene = view->umlScene();
        scene->setIsOpen(true);
        UMLApp::app()->setCurrentView(view);
        Q_EMIT sigDiagramChanged(scene->type());
        UMLApp::app()->setDiagramMenuItemsState(true);
        setModified(true);
        Q_EMIT sigCurrentViewChanged();
        // when clicking on a tab, the documentation of diagram is upated in docwindow
        UMLApp::app()->docWindow()->showDocumentation(scene);
    }
    else {
        logWarn1("UMLDoc::changeCurrentView: New current view was not found with id %1", Uml::ID::toString(id));
    }
}

/**
 * Deletes a diagram from the current file.
 *
 * Undo command
 *
 * @param id   The ID of the diagram to delete.
 */
void UMLDoc::removeDiagram(Uml::ID::Type id)
{
    UMLView* umlView = findView(id);
    if (!umlView) {
        logError1("UMLDoc::removeDiagram : diagram with id %1 not found", Uml::ID::toString(id));
        return;
    }

    UMLScene* umlScene = umlView->umlScene();
    if (Dialog_Utils::askDeleteDiagram(umlScene->name())) {
        UMLApp::app()->executeCommand(new Uml::CmdRemoveDiagram(
            umlScene->folder(),
            umlScene->type(),
            umlScene->name(),
            id
        ));
    }
}

/**
 * Deletes a diagram from the current file.
 *
 * @param id   The ID of the diagram to delete.
 */
void UMLDoc::removeDiagramCmd(Uml::ID::Type id)
{
    UMLApp::app()->docWindow()->updateDocumentation(true);
    UMLView* umlview = findView(id);
    if (!umlview) {
        logError1("UMLDoc::removeDiagramCmd : diagram with id %1 not found", Uml::ID::toString(id));
        return;
    }

    removeView(umlview);
    Q_EMIT sigDiagramRemoved(id);
    setModified(true);
}

/**
 * Return the currently selected root folder.
 * This will be an element from the m_root[] array.
 * @return the currently selected root folder or NULL
 */
UMLFolder *UMLDoc::currentRoot() const
{
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView == nullptr) {
        if (m_pCurrentRoot) {
            return m_pCurrentRoot;
        }
        logError0("UMLDoc::currentRoot : m_pCurrentRoot is NULL");
        return nullptr;
    }
    UMLFolder *f = currentView->umlScene()->folder();
    while (f && f->umlPackage()) {
        f = f->umlParent()->asUMLFolder();
    }
    return f;
}

/**
 * Set the current root folder.
 *
 * @param rootType    The type of the root folder to set.
 *                    The element from m_root[] which is indexed
 *                    by this type is selected.
 */
void UMLDoc::setCurrentRoot(Uml::ModelType::Enum rootType)
{
    m_pCurrentRoot = m_root[rootType];
}

/**
 * Removes an @ref UMLObject from the current file.  If this object
 * is being represented on a diagram it will also delete all those
 * representations.
 *
 * @param umlobject   Pointer to the UMLObject to delete.
 * @param deleteObject Delete the UMLObject instance.
 */
void UMLDoc::removeUMLObject(UMLObject* umlobject, bool deleteObject)
{
    if (umlobject == nullptr) {
        logError0("UMLDoc::removeUMLObject called with NULL parameter");
        return;
    }
    UMLApp::app()->docWindow()->updateDocumentation(true);
    UMLObject::ObjectType type = umlobject->baseType();

    umlobject->setUMLStereotype(nullptr);  // triggers possible cleanup of UMLStereotype
    if (umlobject->asUMLClassifierListItem())  {
        UMLClassifier* parent = umlobject->umlParent()->asUMLClassifier();
        if (parent == nullptr) {
            logError0("UMLDoc::removeUMLObject: parent of umlobject is NULL");
            return;
        }
        if (type == UMLObject::ot_Operation) {
            parent->removeOperation(umlobject->asUMLOperation());
            if (deleteObject)
                delete umlobject->asUMLOperation();
        } else if (type == UMLObject::ot_EnumLiteral) {
            UMLEnum *e = parent->asUMLEnum();
            e->removeEnumLiteral(umlobject->asUMLEnumLiteral());
        } else if (type == UMLObject::ot_EntityAttribute) {
            UMLEntity *ent = parent->asUMLEntity();
            ent->removeEntityAttribute(umlobject->asUMLClassifierListItem());
        } else if (type == UMLObject::ot_UniqueConstraint || type == UMLObject::ot_ForeignKeyConstraint ||
                    type == UMLObject::ot_CheckConstraint) {
            UMLEntity* ent = parent->asUMLEntity();
            ent->removeConstraint(umlobject->asUMLEntityConstraint());
        } else {
            UMLClassifier* pClass = parent->asUMLClassifier();
            if (pClass == nullptr)  {
                logError1("UMLDoc::removeUMLObject: parent of umlobject has unexpected type %1",
                          parent->baseType());
                return;
            }
            if (type == UMLObject::ot_Attribute) {
                pClass->removeAttribute(umlobject->asUMLAttribute());
            } else if (type == UMLObject::ot_Template) {
                pClass->removeTemplate(umlobject->asUMLTemplate());
                if (deleteObject)
                    delete umlobject->asUMLTemplate();
            } else {
                logError1("UMLDoc::removeUMLObject: umlobject has unexpected type %1", type);
            }
        }
    } else if (type == UMLObject::ot_Association) {
        UMLAssociation *a = umlobject->asUMLAssociation();
        removeAssociation(a, false);  // don't call setModified here, it's done below
        Q_EMIT sigObjectRemoved(umlobject);
        if (deleteObject)
            delete a;
    } else {
        UMLPackage* pkg = umlobject->umlPackage();
        if (pkg) {
            // Remove associations that this object may participate in.
            UMLCanvasObject *c = umlobject->asUMLCanvasObject();
            if (c) {
                // In the current implementation, all associations live in the
                // root folder.
                UMLPackage* rootPkg = Model_Utils::rootPackage(c);
                if (rootPkg == nullptr) {
                    logError1("UMLDoc::removeUMLObject %1: root package is not set", umlobject->name());
                    return;
                }
                UMLObjectList rootObjects = rootPkg->containedObjects();
                // Store the associations to remove in a buffer because we
                // should not remove elements from m_objectList while it is
                // being iterated over.
                UMLAssociationList assocsToRemove;
                for(UMLObject *obj : rootObjects) {
                    uIgnoreZeroPointer(obj);
                    if (obj->baseType() == UMLObject::ot_Association) {
                        UMLAssociation *assoc = obj->asUMLAssociation();
                        if (c->hasAssociation(assoc)) {
                            assocsToRemove.append(assoc);
                        }
                    }
                }
                for(UMLAssociation *a : assocsToRemove) {
                    removeAssociation(a, false);
                }
            }
            pkg->removeObject(umlobject);
            Q_EMIT sigObjectRemoved(umlobject);
            if (deleteObject)
                delete umlobject;
        } else {
            logError1("UMLDoc::removeUMLObject %1: parent package is not set", umlobject->name());
        }
    }
    setModified(true);
}

/**
 * Signal that a UMLObject has been created.
 *
 * @param o The object that has been created.
 */
void UMLDoc::signalUMLObjectCreated(UMLObject * o)
{
    Q_EMIT sigObjectCreated(o);
    /* This is the wrong place to do:
               setModified(true);
       Instead, that should be done by the callers when object creation and all
       its side effects (e.g. new widget in view, new list view item, etc.) is
       finalized.
     */
}

/**
 * Set the name of this model.
 */
void UMLDoc::setName(const QString& name)
{
    m_Name = name;
}

/**
 * Return the name of this model.
 */
QString UMLDoc::name() const
{
    return m_Name;
}

/**
 * Set coordinates resolution for current document.

 * @param resolution document resolution in DPI
 */
void UMLDoc::setResolution(qreal resolution)
{
    m_resolution = resolution;
    if (!qFuzzyIsNull(resolution)) {
        const qreal logicalDpiX = Dialog_Utils::logicalDpiXForWidget();
        logDebug3("UMLDoc::setResolution screen dpi: %1, file dpi: %2, scale: %3",
                  logicalDpiX, resolution, logicalDpiX / resolution);
    }
}

/**
 * Returns coordinates resolution for current document.

 * @return document resolution in DPI
 */
qreal UMLDoc::resolution() const
{
    return m_resolution;
}

/**
 * Returns scale factor for recalculation of document coordinates.

 * @return scale factor
 */
qreal UMLDoc::dpiScale() const
{
#ifdef ENABLE_XMIRESOLUTION
    if (!qFuzzyIsNull(resolution()))
        return (qreal)Dialog_Utils::logicalDpiXForWidget() / resolution();
    else
#endif
        return 1.0;
}

/**
 * Return the m_modelID (currently this a fixed value:
 * Umbrello supports only a single document.)
 */
Uml::ID::Type UMLDoc::modelID() const
{
    return m_modelID;
}

/**
 * This method is called for saving the given model as a XMI file.
 * It is virtual and calls the corresponding saveToXMI() functions
 * of the derived classes.
 *
 * @param file   The file to be saved to.
 */
void UMLDoc::saveToXMI(QIODevice& file)
{
    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    if (Settings::optionState().generalState.uml2)
        writer.setAutoFormattingIndent(2);
    else
        writer.setAutoFormattingIndent(1);
    // writer.writeProcessingInstruction(QStringLiteral("xml"));
    writer.writeStartDocument();
    QString expoText(QStringLiteral("umbrello uml modeller "));
    expoText += QLatin1String(umbrelloVersion());
    expoText += QStringLiteral(" http://umbrello.kde.org");
    if (Settings::optionState().generalState.uml2) {
        writer.writeStartElement(QStringLiteral("xmi:XMI"));
        writer.writeAttribute(QStringLiteral("xmi:version"), QStringLiteral("2.1"));
        writer.writeAttribute(QStringLiteral("xmlns:xmi"), QStringLiteral("http://schema.omg.org/spec/XMI/2.1"));
        writer.writeAttribute(QStringLiteral("xmlns:xsi"), QStringLiteral("http://www.w3.org/2001/XMLSchema-instance"));
        writer.writeNamespace(QStringLiteral("http://schema.omg.org/spec/UML/2.1"), QStringLiteral("uml"));
        writer.writeStartElement(QStringLiteral("xmi:Documentation"));
        writer.writeAttribute(QStringLiteral("exporter"), expoText);
        writer.writeAttribute(QStringLiteral("exporterVersion"), QStringLiteral(XMI2_FILE_VERSION));
        writer.writeEndElement();  // xmi:Documentation
        writer.writeStartElement(QStringLiteral("uml:Model"));
        writer.writeAttribute(QStringLiteral("xmi:id"), Uml::ID::toString(m_modelID));
    } else {
        writer.writeStartElement(QStringLiteral("XMI"));
        writer.writeAttribute(QStringLiteral("xmi.version"), QStringLiteral("1.2"));
        QDateTime now = QDateTime::currentDateTime();
        writer.writeAttribute(QStringLiteral("timestamp"), now.toString(Qt::ISODate));
        writer.writeAttribute(QStringLiteral("verified"), QStringLiteral("false"));

        writer.writeNamespace(QStringLiteral("http://schema.omg.org/spec/UML/1.4"), QStringLiteral("UML"));
        writer.writeStartElement(QStringLiteral("XMI.header"));

        writer.writeStartElement(QStringLiteral("XMI.documentation"));
        writer.writeTextElement(QStringLiteral("XMI.exporter"), expoText);

        writer.writeTextElement(QStringLiteral("XMI.exporterVersion"), QStringLiteral(XMI1_FILE_VERSION));

        // all files are now saved with correct Unicode encoding, we add this
        // information to the header, so that the file will be loaded correctly
        writer.writeTextElement(QStringLiteral("XMI.exporterEncoding"), QStringLiteral("UnicodeUTF8"));
        writer.writeEndElement();  // XMI.documentation
        writer.writeStartElement(QStringLiteral("XMI.metamodel"));
        writer.writeAttribute(QStringLiteral("xmi.name"), QStringLiteral("UML"));
        writer.writeAttribute(QStringLiteral("xmi.version"), QStringLiteral("1.4"));
        writer.writeAttribute(QStringLiteral("href"), QStringLiteral("UML.xml"));
        writer.writeEndElement();  // XMI.metamodel
        writer.writeEndElement();  // XMI.header
        writer.writeStartElement(QStringLiteral("XMI.content"));             // content
        writer.writeStartElement(QStringLiteral("UML:Model"));
        writer.writeAttribute(QStringLiteral("xmi.id"), Uml::ID::toString(m_modelID));
    }

    writer.writeAttribute(QStringLiteral("name"), m_Name);
    if (! Settings::optionState().generalState.uml2) {
        writer.writeAttribute(QStringLiteral("isSpecification"), QStringLiteral("false"));
        writer.writeAttribute(QStringLiteral("isAbstract"), QStringLiteral("false"));
        writer.writeAttribute(QStringLiteral("isRoot"), QStringLiteral("false"));
        writer.writeAttribute(QStringLiteral("isLeaf"), QStringLiteral("false"));

        writer.writeStartElement(QStringLiteral("UML:Namespace.ownedElement"));  // ownedNS
    }

    // Save stereotypes and toplevel datatypes first so that upon loading
    // they are known first.
    // There is a bug causing duplication of the same stereotype in m_stereoList.
    // As a workaround, we use a string list to memorize which stereotype has been saved.
    QStringList stereoNames;
    for(UMLStereotype *s : m_stereoList) {
        QString stName = s->name();
        if (!stereoNames.contains(stName)) {
            s->saveToXMI(writer);
            stereoNames.append(stName);
        }
    }
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_root[i]->saveToXMI(writer);
    }

    if (! Settings::optionState().generalState.uml2) {
        writer.writeEndElement();      // UML:Namespace.ownedElement
    }

    writer.writeEndElement();    // uml:Model

    if (! Settings::optionState().generalState.uml2) {
        writer.writeEndElement();  // XMI.content
    }

    // Save the XMI extensions: docsettings, diagrams, listview, and codegeneration.
    if (Settings::optionState().generalState.uml2) {
        writer.writeStartElement(QStringLiteral("xmi:Extension"));
        writer.writeAttribute(QStringLiteral("extender"), QStringLiteral("umbrello"));
    } else {
        writer.writeStartElement(QStringLiteral("XMI.extensions"));
        writer.writeAttribute(QStringLiteral("xmi.extender"), QStringLiteral("umbrello"));
    }

    writer.writeStartElement(QStringLiteral("docsettings"));
    Uml::ID::Type viewID = Uml::ID::None;
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView) {
        viewID = currentView->umlScene()->ID();
    }
    writer.writeAttribute(QStringLiteral("viewid"), Uml::ID::toString(viewID));
    writer.writeAttribute(QStringLiteral("documentation"), m_Doc);
    writer.writeAttribute(QStringLiteral("uniqueid"), Uml::ID::toString(UniqueID::get()));
    writer.writeEndElement();  // docsettings

    //  save listview
    UMLApp::app()->listView()->saveToXMI(writer);

    // save code generator
    CodeGenerator *codegen = UMLApp::app()->generator();
    if (codegen) {
        writer.writeStartElement(QStringLiteral("codegeneration"));
        codegen->saveToXMI(writer);
        writer.writeEndElement();  // codegeneration
    }

    writer.writeEndElement();  // XMI.extensions
    writer.writeEndElement();  // XMI
    writer.writeEndDocument();
}

/**
 * If the given XMI file has a processing instruction then extract the
 * encoding info from the processing instruction.  If that info is unrecognized
 * then return ENC_OLD_ENC, else return the encoding found.
 * If the file does not have a processing instruction then give a warning but
 * return ENC_UNICODE.  This is an optimistic assumption in the interest of
 * best effort loading.
 * The value ENC_UNKNOWN is only returned in case of a grave error.
 *
 * @param file   The file to be checked.
 */
short UMLDoc::encoding(QIODevice & file)
{
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    QString data = stream.readAll();
    QString error;
    int line;
    QDomDocument doc;
    if (!doc.setContent(data, false, &error, &line)) {
        logWarn2("UMLDoc::encoding cannot set content : Error %1, line %2", error, line);
        return ENC_UNKNOWN;
    }

    // we start at the beginning and go to the point in the header where we can
    // find out if the file was saved using Unicode
    QDomNode node = doc.firstChild();
    short enc = ENC_UNKNOWN;
    while (node.isComment() || node.isProcessingInstruction()) {
        if (node.isProcessingInstruction()) {
            const QDomProcessingInstruction& pi = node.toProcessingInstruction();
            QRegularExpression rx(QStringLiteral("\\bencoding=['\"]([^'\"]+)['\"]"));
            const int pos = pi.data().indexOf(rx);
            if (pos >= 0) {
                QRegularExpressionMatch rm = rx.match(pi.data());
                const QString& encData = rm.captured(1);
                if (QString::compare(encData, QStringLiteral("UTF-8"), Qt::CaseInsensitive) == 0) {
                    enc = ENC_UNICODE;
                } else if (QString::compare(encData, QStringLiteral("windows-1252"), Qt::CaseInsensitive) == 0) {
                    enc = ENC_WINDOWS;
                } else {
                    logDebug1("UMLDoc::encoding : ProcessingInstruction encoding=%1 is not yet implemented",
                              encData);
                    enc = ENC_OLD_ENC;
                }
            }
        }
        node = node.nextSibling();
    }
    if (enc == ENC_UNKNOWN) {
        logWarn0("UMLDoc::encoding : No ProcessingInstruction found, assuming ENC_UNICODE");
        enc = ENC_UNICODE;
    }
    return enc;
}

/**
 * Load a given XMI model from a file. If the encoding of the file
 * is already known it can be passed to the function. If this info
 * isn't given, loadFromXMI will check which encoding was used.
 *
 * @param file     The file to be loaded.
 * @param encode   The encoding used.
 */
bool UMLDoc::loadFromXMI(QIODevice & file, short encode)
{
    // old Umbrello versions (version < 1.2) didn't save the XMI in Unicode
    // this wasn't correct, because non Latin1 chars where lost
    // to ensure backward compatibility we have to ensure to load the old files
    // with non Unicode encoding
    if (encode == ENC_UNKNOWN) {
        if ((encode = encoding(file)) == ENC_UNKNOWN) {
            return false;
        }
        file.reset();
    }
    QTextStream stream(&file);
    if (encode == ENC_UNICODE) {
        EncodingHelper::setEncoding(stream, QStringLiteral("UTF-8"));
    } else if (encode == ENC_WINDOWS) {
        EncodingHelper::setEncoding(stream, QStringLiteral("windows-1252"));
    }

    QString data = stream.readAll();
    qApp->processEvents();  // give UI events a chance
    QString error;
    int line;
    QDomDocument doc;
    if (!doc.setContent(data, false, &error, &line)) {
        logWarn2("UMLDoc::loadFromXMI cannot set content : Error %1, line %2", error, line);
        return false;
    }
    qApp->processEvents();  // give UI events a chance
    QDomNode node = doc.firstChild();
    //Before Umbrello 1.1-rc1 we didn't add a <?xml heading
    //so we allow the option of this being missing
    while (node.isComment() || node.isProcessingInstruction()) {
        node = node.nextSibling();
    }

    QDomElement root = node.toElement();
    if (root.isNull()) {
        return false;
    }

    m_nViewID = Uml::ID::None;

    QString outerTag = root.tagName();
    // The element <XMI> / <xmi:XMI> is optional
    if (outerTag == QStringLiteral("XMI") || outerTag == QStringLiteral("xmi:XMI")) {
        QString versionString = root.attribute(QStringLiteral("xmi.version"));
        if (versionString.isEmpty())
            versionString = root.attribute(QStringLiteral("xmi:version"));
        if (! versionString.isEmpty()) {
            double version = versionString.toDouble();
            if (version < 1.0) {
                QString error = i18n("Unsupported xmi file version: %1", versionString);
                m_d->errors << error;
                logDebug1("UMLDoc::loadFromXMI %1", error);
                return false;
            }
            if (version >= 2.0) {
                Settings::optionState().generalState.uml2 = true;
            }
        }
        for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (node.isComment()) {
                continue;
            }
            QDomElement element = node.toElement();
            if (element.isNull()) {
                logDebug0("loadFromXMI: skip empty elem");
                continue;
            }
            bool recognized = false;
            outerTag = element.tagName();
            //check header
            if (outerTag == QStringLiteral("XMI.header")) {
                QDomNode headerNode = node.firstChild();
                if (!validateXMI1Header(headerNode)) {
                    return false;
                }
                recognized = true;
            } else if (outerTag == QStringLiteral("XMI.extensions") ||
                       outerTag == QStringLiteral("xmi:Extension")) {
                QDomNode extensionsNode = node.firstChild();
                while (! extensionsNode.isNull()) {
                    loadExtensionsFromXMI1(extensionsNode);
                    extensionsNode = extensionsNode.nextSibling();
                }
                recognized = true;
            } else if (tagEq(outerTag, QStringLiteral("Model")) ||
                       tagEq(outerTag, QStringLiteral("Package")) ||
                       tagEq(outerTag, QStringLiteral("packagedElement"))) {
                if (!loadUMLObjectsFromXMI(element)) {
                    logWarn1("loadUMLObjectsFromXMI returned false for outerTag %1", outerTag);
                    continue;  //return false;
                }
                m_Name = element.attribute(QStringLiteral("name"), i18n("UML Model"));
                UMLListView *lv = UMLApp::app()->listView();
                lv->setTitle(0, m_Name);
                recognized = true;
            }
            if (outerTag != QStringLiteral("XMI.content")) {
                if (!recognized) {
                    logDebug1("UMLDoc::loadFromXMI skipping <%1>", outerTag);
                }
                continue;
            }
            bool seen_UMLObjects = false;
            //process content
            for (QDomNode child = node.firstChild(); !child.isNull();
                    child = child.nextSibling()) {
                if (child.isComment()) {
                    continue;
                }
                element = child.toElement();
                QString tag = element.tagName();
                if (tag == QStringLiteral("umlobjects")  // for bkwd compat.
                        || tagEq(tag, QStringLiteral("Subsystem"))
                        || tagEq(tag, QStringLiteral("Project"))  // Embarcadero's Describe
                        || tagEq(tag, QStringLiteral("Model"))) {
                    if (!loadUMLObjectsFromXMI(element)) {
                        logWarn0("UMLDoc::loadFromXMI failed load on objects");
                        return false;
                    }
                    m_Name = element.attribute(QStringLiteral("name"), i18n("UML Model"));
                    UMLListView *lv = UMLApp::app()->listView();
                    lv->setTitle(0, m_Name);
                    seen_UMLObjects = true;
                } else if (tagEq(tag, QStringLiteral("Package")) ||
                           tagEq(tag, QStringLiteral("Class")) ||
                           tagEq(tag, QStringLiteral("Interface")) ||
                           tagEq(tag, QStringLiteral("DataType"))) {
                    // These tests are only for foreign XMI files that
                    // are missing the <Model> tag (e.g. NSUML)
                    QString stID = element.attribute(QStringLiteral("stereotype"));
                    UMLObject *pObject = Object_Factory::makeObjectFromXMI(tag, stID);
                    if (!pObject) {
                        logWarn1("UMLDoc::loadFromXMI Unknown type of umlobject to create: %1", tag);
                        // We want a best effort, therefore this is handled as a
                        // soft error.
                        continue;
                    }
                    UMLObject::ObjectType ot = pObject->baseType();
                    // Set the parent root folder.
                    UMLPackage  *pkg = nullptr;
                    if (ot != UMLObject::ot_Stereotype) {
                        if (ot == UMLObject::ot_Datatype) {
                            pkg = m_datatypeRoot;
                        } else {
                            Uml::ModelType::Enum guess = Model_Utils::guessContainer(pObject);
                            if (guess != Uml::ModelType::N_MODELTYPES) {
                                pkg = m_root[guess];
                            }
                            else {
                                logError2("UMLDoc::loadFromXMI: guessContainer failed - package not set correctly for %1 / base type %2",
                                          pObject->name(), pObject->baseTypeStr());
                                pkg = m_root[Uml::ModelType::Logical];
                            }
                        }
                    }
                    pObject->setUMLPackage(pkg);
                    bool status = pObject->loadFromXMI(element);
                    if (!status) {
                        delete pObject;
                        return false;
                    }
                    seen_UMLObjects = true;
                } else if (tagEq(tag, QStringLiteral("TaggedValue"))) {
                    // This tag is produced here, i.e. outside of <UML:Model>,
                    // by the Unisys.JCR.1 Rose-to-XMI tool.
                    if (! seen_UMLObjects) {
                        logDebug0("skipping TaggedValue because not seen_UMLObjects");
                        continue;
                    }
                    tag = element.attribute(QStringLiteral("tag"));
                    if (tag != QStringLiteral("documentation")) {
                        continue;
                    }
                    QString modelElement = element.attribute(QStringLiteral("modelElement"));
                    if (modelElement.isEmpty()) {
                        logDebug0("skipping TaggedValue(documentation) because modelElement.isEmpty()");
                        continue;
                    }
                    UMLObject *o = findObjectById(Uml::ID::fromString(modelElement));
                    if (o == nullptr) {
                        logDebug1("TaggedValue(documentation): cannot find object for modelElement %1",
                                  modelElement);
                        continue;
                    }
                    QString value = element.attribute(QStringLiteral("value"));
                    if (! value.isEmpty()) {
                        o->setDoc(value);
                    }
                } else if (tagEq(tag, QStringLiteral("ownedComment"))) {
                    m_Doc = Model_Utils::loadCommentFromXMI(element);
                } else {
                    // for backward compatibility
                    loadExtensionsFromXMI1(child);
                }
            }
        }
    } else if (tagEq(outerTag, QStringLiteral("Model")) ||
               tagEq(outerTag, QStringLiteral("Package"))) {
        if (!loadUMLObjectsFromXMI(root)) {
            logWarn0("UMLDoc::loadFromXMI (without XMI element) failed load on objects");
            return false;
        }
        m_Name = root.attribute(QStringLiteral("name"), i18n("UML Model"));
        UMLListView *lv = UMLApp::app()->listView();
        lv->setTitle(0, m_Name);
    } else {
        logError1("UMLDoc::loadFromXMI failed load: Unrecognized outer element %1", outerTag);
        return false;
    }

    resolveTypes();
    loadDiagrams1();

    Q_EMIT sigWriteToStatusBar(i18n("Setting up the document..."));
    qApp->processEvents();  // give UI events a chance
    activateAllViews();

    UMLView *viewToBeSet = nullptr;
    if (m_nViewID != Uml::ID::None) {
        viewToBeSet = findView(m_nViewID);
    }
    if (viewToBeSet) {
        changeCurrentView(m_nViewID);
    } else {
        QString name = createDiagramName(Uml::DiagramType::Class, false);
        createDiagram(m_root[Uml::ModelType::Logical], Uml::DiagramType::Class, name);
        m_pCurrentRoot = m_root[Uml::ModelType::Logical];
    }
    Q_EMIT sigResetStatusbarProgress();
    return true;
}

/**
 * Type resolution pass.
 */
void UMLDoc::resolveTypes()
{
    // Resolve the types.
    // This is done in a separate pass because of possible forward references.
    if (m_bTypesAreResolved) {
        return;
    }
    writeToStatusBar(i18n("Resolving object references..."));
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
       UMLFolder *obj = m_root[i];
#ifdef VERBOSE_DEBUGGING
        logDebug2("UMLDoc::resolveTypes calling resolveRef for %1 (id=%2)",
                  obj->name(), Uml::ID::toString(obj->id()));
#endif
        obj->resolveRef();
    }
    m_bTypesAreResolved = true;
    qApp->processEvents();  // give UI events a chance
}

/**
 * Load all diagrams collected from the xmi file.
 *
 * Loading diagrams is implemented as additional pass to avoid unresolved
 * uml objects which are defined later in the xmi file.
 */
bool UMLDoc::loadDiagrams1()
{
    bool result = true;
    DiagramsMap::const_iterator i;
    for (i = m_diagramsToLoad.constBegin(); i != m_diagramsToLoad.constEnd(); i++) {
        UMLFolder *f = i.key();
        for(QDomNode node : i.value())
            if (!f->loadDiagramsFromXMI1(node))
                result = false;
    }

    m_diagramsToLoad.clear();
    return result;
}

/**
 * Add a xml node containing a diagram to the list of diagrams to load.
 * Helper function for loadDiagrams().
 *
 * @param folder pointer to UMFolder instance the diagrams belongs to
 * @param node xml document node containing the diagram
 */
void UMLDoc::addDiagramToLoad(UMLFolder *folder, QDomNode node)
{
    if (m_diagramsToLoad.contains(folder))
        m_diagramsToLoad[folder].append(node);
    else
        m_diagramsToLoad[folder] = QList<QDomNode>() << node;
}

DiagramsModel *UMLDoc::diagramsModel() const
{
    return m_diagramsModel;
}

ObjectsModel *UMLDoc::objectsModel() const
{
    return m_objectsModel;
}

void UMLDoc::setLoadingError(const QString &text)
{
    m_d->errors << text;
}

StereotypesModel *UMLDoc::stereotypesModel() const
{
    return m_stereotypesModel;
}

/**
 * Ensures the XMI file is a valid UML file.
 * Currently only checks for metamodel=UML.
 *
 * @param headerNode   The <XMI.header> node
 */
bool UMLDoc::validateXMI1Header(QDomNode& headerNode)
{
    QDomElement headerElement = headerNode.toElement();
    while (!headerNode.isNull()) {
        /*  //Seems older Umbrello files used a different metamodel, so don't validate it for now
          if(!headerElement.isNull() && headerElement.tagName() == "XMI.metamodel") {
              String metamodel = headerElement.attribute("xmi.name");
              if (metamodel != "UML") {
                  return false;
              }
          }
        */
        headerNode = headerNode.nextSibling();
        headerElement = headerNode.toElement();
    }
    return true;
}

/**
 * Loads all UML objects from XMI into the current UMLDoc.
 *
 * @return  True if operation successful.
 */
bool UMLDoc::loadUMLObjectsFromXMI(QDomElement& element)
{
    /* FIXME need a way to make status bar actually reflect
       how much of the file has been loaded rather than just
       counting to 10 (an arbitrary number)
    Q_EMIT sigResetStatusbarProgress();
    Q_EMIT sigSetStatusbarProgress(0);
    Q_EMIT sigSetStatusbarProgressSteps(10);
    m_count = 0;
     */
    Q_EMIT sigWriteToStatusBar(i18n("Loading UML elements..."));

    // For Umbrello native XMI files, when called from loadFromXMI() we
    // get here with Element.tagName() == "UML:Model" from the XMI input:
    // <UML:Model name="UML Model">
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment()) {
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        QString xmiType = tempElement.attribute(QStringLiteral("xmi:type"));
        if (tagEq(type, QStringLiteral("packagedElement")) && !xmiType.isEmpty()) {
            type = xmiType;
        }
        if (tagEq(type, QStringLiteral("Model"))) {
            // Handling of Umbrello native XMI files:
            // We get here from a recursive call to loadUMLObjectsFromXMI()
            // a few lines below, see
            //       if (tagEq(type, "Namespace.ownedElement") ....
            // Inside this Namespace.ownedElement envelope there are the
            // four submodels.
            // In UML2 mode:
            //   <packagedElement xmi:type="uml:Model" name="Logical View">
            //   <packagedElement xmi:type="uml:Model" name="Use Case View">
            //   <packagedElement xmi:type="uml:Model" name="Component View">
            //   <packagedElement xmi:type="uml:Model" name="Deployment View">
            // In UML1 mode:
            //   <UML:Model name="Logical View">
            //   <UML:Model name="Use Case View">
            //   <UML:Model name="Component View">
            //   <UML:Model name="Deployment View">
            // These are ultimately loaded by UMLFolder::loadFromXMI()
            // Furthermore, in UML1 mode native XMI format this
            // Namespace.ownedElement is the container of all stereotypes
            // (<UML:Stereotype>).
            bool foundUmbrelloRootFolder = false;
            QString name = tempElement.attribute(QStringLiteral("name"));
            for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
                if (name == m_root[i]->name()) {
                    UMLFolder *curRootSave = m_pCurrentRoot;
                    m_pCurrentRoot = m_root[i];
                    if (!m_pCurrentRoot->loadFromXMI(tempElement)) {
                        logWarn1("UMLDoc::loadUMLObjectsFromXMI failed load on %1", name);
                        m_pCurrentRoot = curRootSave;
                        return false;
                    }
                    foundUmbrelloRootFolder = true;
                    break;
                }
            }
            if (foundUmbrelloRootFolder) {
                continue;
            }
        }
        if (Model_Utils::isCommonXMI1Attribute(type)) {
            continue;
        }
        if (tagEq(type, QStringLiteral("Namespace.ownedElement")) ||
                tagEq(type, QStringLiteral("Namespace.contents")) ||
                tagEq(type, QStringLiteral("Element.ownedElement")) ||  // Embarcadero's Describe
                tagEq(type, QStringLiteral("Model")) ||
                type == QStringLiteral("ownedElement")) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore the <UML:Namespace.ownedElement> tag is of no
            // significance.
            // The tagEq(type, "Namespace.contents") and tagEq(type, "Model")
            // tests do not become true for Umbrello native files, only for
            // some foreign XMI files.
            if (!loadUMLObjectsFromXMI(tempElement)) {
                logWarn1("UMLDoc::loadUMLObjectsFromXMI failed load on type %1", type);
            }
            continue;
        }
        // From here on, it's support for stereotypes, pre 1.5.5 versions,
        // and foreign files
        if (!tempElement.hasAttribute(QStringLiteral("xmi.id")) &&
            !tempElement.hasAttribute(QStringLiteral("xmi:id"))) {
            QString idref = tempElement.attribute(QStringLiteral("xmi.idref"));
            if (! idref.isEmpty()) {
                logDebug1("UMLDoc::loadUMLObjectsFromXMI resolution of xmi.idref %1"
                          " is not yet implemented", idref);
            } else {
                logError1("UMLDoc::loadUMLObjectsFromXMI cannot load type %1 because xmi.id is missing",
                          type);
            }
            continue;
        }
        if (UMLDoc::tagEq(type, QStringLiteral("ownedComment"))) {
            m_Doc = Model_Utils::loadCommentFromXMI(tempElement);
            continue;
        }
        QString stID = tempElement.attribute(QStringLiteral("stereotype"));
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type, stID);
        if (!pObject) {
            logWarn1("UMLDoc::loadUMLObjectsFromXMI unknown type of umlobject to create: %1", type);
            // We want a best effort, therefore this is handled as a
            // soft error.
            continue;
        }
        UMLObject::ObjectType ot = pObject->baseType();
        // Set the parent root folder.
        UMLPackage  *pkg = nullptr;
        if (ot != UMLObject::ot_Stereotype) {
            if (ot == UMLObject::ot_Datatype) {
                pkg = m_datatypeRoot;
            } else {
                Uml::ModelType::Enum guess = Model_Utils::guessContainer(pObject);
                if (guess != Uml::ModelType::N_MODELTYPES) {
                    pkg = m_root[guess];
                }
                else {
                    logError2("UMLDoc::loadUMLObjectsFromXMI guessContainer failed - package not set correctly for %1 / base type %2",
                              pObject->name(), pObject->baseTypeStr());
                    pkg = m_root[Uml::ModelType::Logical];
                }
            }
        }
        pObject->setUMLPackage(pkg);

        bool status = pObject->loadFromXMI(tempElement);
        if (!status) {
            delete pObject;
            continue;
        }
        pkg = pObject->umlPackage();
        if (ot == UMLObject::ot_Stereotype) {
            UMLStereotype *s = pObject->asUMLStereotype();
            UMLStereotype *exist = findStereotype(pObject->name());
            if (exist) {
                if (exist->id() == pObject->id()) {
                    delete pObject;
                } else {
                    logDebug3("UMLDoc::loadUMLObjectsFromXMI Stereotype %1 (id=%2) "
                              "already exists with id=%3", pObject->name(),
                              Uml::ID::toString(pObject->id()), Uml::ID::toString(exist->id()));
                    addStereotype(s);
                }
            } else {
                addStereotype(s);
            }
            continue;
        }
        if (pkg) {
            UMLObjectList objects = pkg->containedObjects();
            if (! objects.contains(pObject)) {
                logDebug2("UMLDoc::loadUMLObjectsFromXMI CHECK: adding %1 to %2",
                          pObject->name(), pkg->name());
                if (!pkg->addObject(pObject)) {
                    logDebug0("- pkg->addObject failed");
                }
            }
        }
        else if (ot != UMLObject::ot_Stereotype) {
            logError1("UMLDoc::loadUMLObjectsFromXMI: Package is NULL for %1", pObject->name());
            return false;
        }

        /* FIXME see comment at loadUMLObjectsFromXMI
        Q_EMIT sigSetStatusbarProgress(++m_count);
         */
    }
    return true;
}

/**
 * Sets m_nViewID.
 */
void UMLDoc::setMainViewID(Uml::ID::Type viewID)
{
    m_nViewID = viewID;
}

/**
 * Loads umbrello specific extensions from XMI to the UMLDoc.
 * The extension tags are: "docsettings", "diagrams", "listview",
 * and "codegeneration".
 */
void UMLDoc::loadExtensionsFromXMI1(QDomNode& node)
{
    QDomElement element = node.toElement();
    QString tag = element.tagName();

    if (tag == QStringLiteral("docsettings")) {
        QString viewID = element.attribute(QStringLiteral("viewid"), QStringLiteral("-1"));
        m_Doc = element.attribute(QStringLiteral("documentation"));
        QString uniqueid = element.attribute(QStringLiteral("uniqueid"), QStringLiteral("0"));

        m_nViewID = Uml::ID::fromString(viewID);
        UniqueID::set(Uml::ID::fromString(uniqueid));
        UMLApp::app()->docWindow()->reset();

    } else if (tag == QStringLiteral("diagrams") || tag == QStringLiteral("UISModelElement")) {
        // For backward compatibility only:
        // Since version 1.5.5 diagrams are saved as part of the UMLFolder.
        QDomNode diagramNode = node.firstChild();
        if (tag == QStringLiteral("UISModelElement")) {          // Unisys.IntegratePlus.2
            element = diagramNode.toElement();
            tag = element.tagName();
            if (tag != QStringLiteral("uisOwnedDiagram")) {
                logError1("UMLDoc::loadExtensionsFromXMI1 unknown child node %1", tag);
                return;
            }
            diagramNode = diagramNode.firstChild();
        } else {
            qreal resolution = 0.0;
            QString res = node.toElement().attribute(QStringLiteral("resolution"), QStringLiteral(""));
            if (!res.isEmpty()) {
               resolution = res.toDouble();
            }
            if (!qFuzzyIsNull(resolution)) {
               UMLApp::app()->document()->setResolution(resolution);
            } else {
               // see UMLFolder::loadDiagramsFromXMI()
               UMLApp::app()->document()->setResolution(0.0);
            }
        }
        if (!loadDiagramsFromXMI1(diagramNode)) {
            logWarn0("UMLDoc::loadExtensionsFromXMI1 failed load on diagrams");
        }

    } else if (tag == QStringLiteral("listview")) {
        //FIXME: Need to resolveTypes() before loading listview,
        //       else listview items are duplicated.
        resolveTypes();
        if (!UMLApp::app()->listView()->loadFromXMI(element)) {
            logWarn0("UMLDoc::loadExtensionsFromXMI1 failed load on listview");
        }

    } else if (tag == QStringLiteral("codegeneration")) {
        QDomNode cgnode = node.firstChild();
        QDomElement cgelement = cgnode.toElement();
        while (!cgelement.isNull()) {
            QString nodeName = cgelement.tagName();
            QString lang = cgelement.attribute(QStringLiteral("language"), QStringLiteral("UNKNOWN"));
            Uml::ProgrammingLanguage::Enum pl = Uml::ProgrammingLanguage::fromString(lang);
            if (pl != Uml::ProgrammingLanguage::Reserved) {
                CodeGenerator *g = UMLApp::app()->setGenerator(pl);
                g->loadFromXMI(cgelement);
            } else
                logDebug0("UMLDoc::loadExtensionsFromXMI1 codegeneration : "
                          "no setup required for UML primitive types");
            cgnode = cgnode.nextSibling();
            cgelement = cgnode.toElement();
        }
        if (UMLApp::app()->generator() == nullptr) {
            UMLApp::app()->setGenerator(UMLApp::app()->defaultLanguage());
        }
    }
}

/**
 * Loads all diagrams from XMI into the current UMLDoc.
 * For backward compatibility only:
 * Since version 1.5.5 diagrams are saved as part of the UMLFolder.
 *
 * @return  True if operation successful.
 */
bool UMLDoc::loadDiagramsFromXMI1(QDomNode & node)
{
    Q_EMIT sigWriteToStatusBar(i18n("Loading diagrams..."));
    Q_EMIT sigResetStatusbarProgress();
    Q_EMIT sigSetStatusbarProgress(0);
    Q_EMIT sigSetStatusbarProgressSteps(10); //FIX ME
    QDomElement element = node.toElement();
    if (element.isNull()) {
        return true;  //return ok as it means there is no umlobjects
    }
    const Settings::OptionState state = Settings::optionState();
    UMLView  *pView = nullptr;
    int count = 0;
    while (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QStringLiteral("diagram") || tag == QStringLiteral("UISDiagram")) {
            pView = new UMLView(nullptr);
            // IMPORTANT: Set OptionState of new UMLView _BEFORE_
            // reading the corresponding diagram:
            // + allow using per-diagram color and line-width settings
            // + avoid crashes due to uninitialized values for lineWidth
            pView->umlScene()->setOptionState(state);
            bool success = false;
            if (tag == QStringLiteral("UISDiagram")) {
                success = pView->umlScene()->loadUISDiagram(element);
            } else {
                success = pView->umlScene()->loadFromXMI(element);
            }
            if (!success) {
                logWarn0("UMLDoc::loadDiagramsFromXMI failed load on viewdata loadfromXMI");
                delete pView;
                return false;
            }
            // Put diagram in default predefined folder.
            // @todo pass in the parent folder - it might be a user defined one.
            Uml::ModelType::Enum mt = Model_Utils::convert_DT_MT(pView->umlScene()->type());
            if (mt != Uml::ModelType::N_MODELTYPES) {
                pView->umlScene()->setFolder(m_root[mt]);
                pView->hide();
                addView(pView);
            } else {
                logWarn2("UMLDoc::loadDiagramsFromXMI cannot add %1 because scene type %2 cannot be mapped to ModelType",
                         tag, pView->umlScene()->type());
            }
            Q_EMIT sigSetStatusbarProgress(++count);
            qApp->processEvents();  // give UI events a chance
        }
        node = node.nextSibling();
        element = node.toElement();
    }
    return true;
}

/**
 * Call to remove all the views (diagrams) in the current file.
 */
void UMLDoc::removeAllViews()
{
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_root[i]->removeAllViews();
    }

    UMLApp::app()->setCurrentView(nullptr);
    Q_EMIT sigDiagramChanged(Uml::DiagramType::Undefined);
    UMLApp::app()->setDiagramMenuItemsState(false);
}

/**
 * Call to remove all objects in the current file.
 */
void UMLDoc::removeAllObjects()
{
    m_root[Uml::ModelType::Logical]->removeObject(m_datatypeRoot);

    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_root[i]->removeAllObjects();
    }
}

/**
 * Returns a list of the packages in this UMLDoc,
 *
 * @return List of UMLPackages.
 */
UMLPackageList UMLDoc::packages(bool includeNested /* = true */, Uml::ModelType::Enum model) const
{
    UMLPackageList packageList;
    m_root[model]->appendPackages(packageList, includeNested);
    return packageList;
}

/**
 * Returns the datatype folder.
 *
 * @return  Pointer to the predefined folder for datatypes.
 */
UMLFolder * UMLDoc::datatypeFolder() const
{
    return m_datatypeRoot;
}

/**
 * Returns a list of the concepts in this UMLDoc.
 *
 * @param includeNested   Whether to include the concepts from
 *                        nested packages (default: true.)
 * @return  List of UML concepts.
 */
UMLClassifierList UMLDoc::concepts(bool includeNested /* =true */) const
{
    UMLClassifierList conceptList;
    m_root[Uml::ModelType::Logical]->appendClassifiers(conceptList, includeNested);
    return conceptList;
}

/**
 * Returns a list of the classes, interfaces, and enumerations in this UMLDoc.
 *
 * @param includeNested   Whether to include the concepts from
 *                        nested packages (default: true.)
 * @return  List of UML concepts.
 */
UMLClassifierList UMLDoc::classesAndInterfaces(bool includeNested /* =true */) const
{
    UMLClassifierList conceptList;
    m_root[Uml::ModelType::Logical]->appendClassesAndInterfaces(conceptList, includeNested);
    return conceptList;
}

/**
 * Returns a list of the entities in this UMLDoc.
 *
 * @param includeNested   Whether to include the entities from
 *                        nested packages (default: true.)
 * @return  List of UML Entities.
 */
UMLEntityList UMLDoc::entities(bool includeNested /* =true */) const
{
    UMLEntityList entityList;
    m_root[Uml::ModelType::EntityRelationship]->appendEntities(entityList, includeNested);
    return entityList;
}

/**
 * Returns a list of the datatypes in this UMLDoc.
 *
 * @param includeInactive  Include inactive datatypes which may have accrued by
 *                         changing the active programming language.
 * @return  List of datatypes.
 */
UMLClassifierList UMLDoc::datatypes(bool includeInactive /* = false */) const
{
    UMLObjectList objects = m_datatypeRoot->containedObjects(includeInactive);
    UMLClassifierList datatypeList;
    for(UMLObject *obj : objects) {
        uIgnoreZeroPointer(obj);
        if (obj->isUMLDatatype()) {
            datatypeList.append(obj->asUMLClassifier());
        }
    }
    return datatypeList;
}

/**
 * Seek the datatype of the given name in the Datatypes folder.
 *
 * @param name             Name of the datatype
 * @param includeInactive  Include inactive datatypes in the search.
 * @return  List of datatypes.
 */
UMLDatatype * UMLDoc::findDatatype(QString name, bool includeInactive /* = false */)
{
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    name = Model_Utils::normalize(name);
    for(UMLClassifier *c : datatypes(includeInactive)) {
        UMLDatatype *type = dynamic_cast<UMLDatatype*>(c);
        if (!type)
            continue;
        if (caseSensitive) {
            if (type->name() == name)
                return type;
        } else if (type->name().toLower() == name.toLower()) {
            return type;
        }
    }
    return nullptr;
}

/**
 * Returns a list of the associations in this UMLDoc.
 *
 * @return  List of UML associations.
 */
UMLAssociationList UMLDoc::associations() const
{
    UMLAssociationList associationList;
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        UMLAssociationList assocs = m_root[i]->getAssociations();

        for(UMLAssociation* a : assocs) {
            associationList.append(a);
        }
    }
    return associationList;
}

/**
 * Controls the printing of the program.
 *
 * @param pPrinter  The printer (object) to use.
 * @param selectPage  The DiagramPrintPage by which diagrams are selected for printing
 */
void UMLDoc::print(QPrinter * pPrinter, DiagramPrintPage * selectPage)
{
    UMLView  *printView = nullptr;
    int count = selectPage->printUmlCount();
    QPainter painter(pPrinter);
    for (int i = 0; i < count; ++i) {
        if (i>0) {
            pPrinter->newPage();
        }
        QString sID = selectPage->printUmlDiagram(i);
        Uml::ID::Type id = Uml::ID::fromString(sID);
        printView = findView(id);

        if (printView) {
            printView->umlScene()->print(pPrinter, painter);
        }
        printView = nullptr;
    }
    painter.end();
}

/**
 * Return the list of views for this document.
 *
 * @return  List of UML views.
 */
UMLViewList UMLDoc::viewIterator() const
{
    UMLViewList accumulator;
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_root[i]->appendViews(accumulator, true);
    }
    return accumulator;
}

/**
 * Return a list of filtered views of this document by type.

 * @param type diagram type to filter
 * @return  List of UML views.
 */
UMLViewList UMLDoc::views(Uml::DiagramType::Enum type) const
{
    UMLViewList result;
    for(UMLView *v : viewIterator()) {
        if (type == Uml::DiagramType::Undefined || v->umlScene()->type() == type)
            result.append(v);
    }
    return result;
}

/**
 * Sets the modified flag for the document after a modifying
 * action on the view connected to the document.
 *
 * @param modified   The value to set the modified flag to.
 */
void UMLDoc::setModified(bool modified /*=true*/)
{
    if (!m_bLoading) {
        m_modified = modified;
        UMLApp::app()->setModified(modified);
    }
}

/**
 * Returns if the document is modified or not. Use this to
 * determine if your document needs saving by the user on
 * closing.
 *
 * @return  True if this UMLDoc is modified.
 */
bool UMLDoc::isModified() const
{
    return m_modified;
}

/**
 * Assigns an already created UMLObject a new ID.
 * If the object is a classifier then the operations/attributes
 * are also assigned new IDs.
 *
 * @param obj   Pointer to the UMLObject to add.
 * @return  True if operation successful.
 */
bool UMLDoc::assignNewIDs(UMLObject* obj)
{
    if (!obj || !m_pChangeLog) {
        logDebug0("UMLDoc::assignNewIDs: no obj || Changelog");
        return false;
    }
    Uml::ID::Type result = assignNewID(obj->id());
    obj->setID(result);

    //If it is a CONCEPT then change the ids of all its operations and attributes
    if (obj->baseType() == UMLObject::ot_Class) {
        UMLClassifier *c = obj->asUMLClassifier();
        UMLClassifierListItemList attributes = c->getFilteredList(UMLObject::ot_Attribute);
        for(UMLObject* listItem: attributes) {
            result = assignNewID(listItem->id());
            listItem->setID(result);
        }

        UMLClassifierListItemList templates = c->getFilteredList(UMLObject::ot_Template);
        for(UMLObject* listItem : templates) {
            result = assignNewID(listItem->id());
            listItem->setID(result);
        }
    }

    if (obj->baseType() == UMLObject::ot_Interface || obj->baseType() == UMLObject::ot_Class) {
        UMLOperationList operations(((UMLClassifier*)obj)->getOpList());
        for(UMLObject *listItem : operations) {
            result = assignNewID(listItem->id());
            listItem->setID(result);
        }
    }

    setModified(true);

    return true;
}

/**
 * Return the predefined root folder of the given type.
 */
UMLFolder *UMLDoc::rootFolder(Uml::ModelType::Enum mt) const
{
    if (mt < Uml::ModelType::Logical || mt >= Uml::ModelType::N_MODELTYPES) {
        logError1("UMLDoc::rootFolder: illegal model type value %1", mt);
        return nullptr;
    }
    return m_root[mt];
}

/**
 * Return the corresponding Model_Type if the given object
 * is one of the root folders.
 * When the given object is not one of the root folders then
 * return Uml::ModelType::N_MODELTYPES.
 */
Uml::ModelType::Enum UMLDoc::rootFolderType(UMLObject *obj) const
{
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        const Uml::ModelType::Enum m = Uml::ModelType::fromInt(i);
        if (obj == m_root[m]) {
            return m;
        }
    }
    return Uml::ModelType::N_MODELTYPES;
}

/**
 * Read property of IDChangeLog* m_pChangeLog.
 *
 * @return  Pointer to the IDChangeLog object.
 */
IDChangeLog* UMLDoc::changeLog() const
{
    return m_pChangeLog;
}

/**
 * Opens a Paste session, deletes the old ChangeLog and
 * creates an empty one.
 */
void UMLDoc::beginPaste()
{
    if (m_pChangeLog) {
        delete m_pChangeLog;
        m_pChangeLog = nullptr;
    }
    m_pChangeLog = new IDChangeLog;
}

/**
 * Closes a paste session, deletes the ChangeLog.
 */
void UMLDoc::endPaste()
{
    if (m_pChangeLog) {
        delete m_pChangeLog;
        m_pChangeLog = nullptr;
    }
}

/**
 * Assigns a New ID to an Object, and also logs the assignment
 * to its internal ChangeLog.
 *
 * @param oldID   The present ID of the object.
 * @return  The new ID assigned to the object.
 */
Uml::ID::Type UMLDoc::assignNewID(Uml::ID::Type oldID)
{
    Uml::ID::Type result = UniqueID::gen();
    if (m_pChangeLog) {
        m_pChangeLog->addIDChange(oldID, result);
    }
    return result;
}

/**
 * Returns the documentation for the project.
 *
 * @return  The documentation text of this UMLDoc.
 */
QString UMLDoc::documentation() const
{
    return m_Doc;
}

/**
 * Sets the documentation for the project.
 *
 * @param doc   The documentation to set for this UMLDoc.
 */
void UMLDoc::setDocumentation(const QString &doc)
{
    m_Doc = doc;
}

/**
 * Adds an already created UMLView to the document, it gets
 * assigned a new ID, if its name is already in use then the
 * function appends a number to it to differentiate it from
 * the others; this number is incremental so if number 1 is in
 * use then it tries 2 and then 3 and so on
 *
 * @param pView   Pointer to the UMLView to add.
 * @return  True if operation successful.
 */
bool UMLDoc::addUMLView(UMLView * pView)
{
    if (!pView || !m_pChangeLog) {
        return false;
    }

    Uml::ID::Type oldID = pView->umlScene()->ID();

    int i = 0;
    QString viewName = pView->umlScene()->name();
    QString name = viewName;
    while (findView(pView->umlScene()->type(), name) != nullptr) {
        name = viewName + QLatin1Char('_') + QString::number(++i);
    }
    if (i) { //If name was modified
        pView->umlScene()->setName(name);
    }

    Uml::ID::Type newID = assignNewID(oldID);
    pView->umlScene()->setID(newID);

    pView->umlScene()->activateAfterLoad(true);
    pView->umlScene()->endPartialWidgetPaste();
    pView->umlScene()->setOptionState(Settings::optionState());
    addView(pView);

    Q_EMIT sigDiagramCreated(pView->umlScene()->ID());

    setModified(true);
    return true;
}

/**
 * Activate all the diagrams/views after loading so all their
 * widgets keep their IDs.
 */
void UMLDoc::activateAllViews()
{
    // store old setting - for restore of last setting
    bool m_bLoading_old = m_bLoading;
    m_bLoading = true; //this is to prevent document becoming modified when activating a view

    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_root[i]->activateViews();
    }
    m_bLoading = m_bLoading_old;
}

/**
 * Sets the default settings to the given settings.
 * @param optionState   settings
 */
void UMLDoc::settingsChanged(Settings::OptionState &optionState)
{
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_root[i]->setViewOptions(optionState);
    }
    initSaveTimer();
}

/**
 * Sets up the autosave timer.
 */
void UMLDoc::initSaveTimer()
{
    if (m_pAutoSaveTimer) {
        m_pAutoSaveTimer->stop();
        disconnect(m_pAutoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
        delete m_pAutoSaveTimer;
        m_pAutoSaveTimer = nullptr;
    }
    Settings::OptionState optionState = Settings::optionState();
    if (optionState.generalState.autosave) {
        m_pAutoSaveTimer = new QTimer(this);
        connect(m_pAutoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
        m_pAutoSaveTimer->setSingleShot(false);
        m_pAutoSaveTimer->start(optionState.generalState.autosavetime * 60000);
    }
}

/**
 * Called after a specified time to autosave the document.
 */
void UMLDoc::slotAutoSave()
{
    //Only save if modified.
    if (!m_modified) {
        return;
    }
    QUrl tempUrl = m_doc_url;
    if (tempUrl.fileName() == i18n("Untitled")) {
        tempUrl.setScheme(QStringLiteral("file"));
        tempUrl.setPath(QDir::homePath() + i18n("/autosave%1", QStringLiteral(".xmi")));
        saveDocument(tempUrl);
        setUrlUntitled();
        m_modified = true;
        UMLApp::app()->setModified(m_modified);
    } else {
        // 2004-05-17 Achim Spangler
        QUrl orgDocUrl = m_doc_url;
        QString orgFileName = m_doc_url.fileName();
        // don't overwrite manually saved file with autosave content
        QString fileName = tempUrl.fileName();
        Settings::OptionState optionState = Settings::optionState();
        fileName.replace(QStringLiteral(".xmi"), optionState.generalState.autosavesuffix);
        tempUrl.setUrl(tempUrl.toString(QUrl::RemoveFilename) + fileName);
        // End Achim Spangler

        saveDocument(tempUrl);
        // 2004-05-17 Achim Spangler
        // re-activate m_modified if autosave is writing to other file
        // than the main project file->autosave-suffix != ".xmi"
        if (optionState.generalState.autosavesuffix != QStringLiteral(".xmi")) {
            m_modified = true;
            UMLApp::app()->setModified(m_modified);
        }
        // restore original file name -
        // UMLDoc::saveDocument() sets doc_url to filename which is given as autosave-filename
        setUrl(orgDocUrl);
        UMLApp * pApp = UMLApp::app();
        pApp->setCaption(orgFileName, isModified());
        // End Achim Spangler
    }
}

/**
 * Signal a view/diagram has been renamed.
 */
void UMLDoc::signalDiagramRenamed(UMLView* view)
{
    if (view) {
        Settings::OptionState optionState = Settings::optionState();
        if (optionState.generalState.tabdiagrams) {
            UMLApp::app()->tabWidget()->setTabText(UMLApp::app()->tabWidget()->indexOf(view), view->umlScene()->name());
        }
        Q_EMIT sigDiagramRenamed(view->umlScene()->ID());
    }
    else {
      logError0("Cannot signal diagram renamed - view is NULL!");
    }
}

/**
 * Calls the active code generator to create its default datatypes.
 */
void UMLDoc::addDefaultDatatypes()
{
    CodeGenerator *cg = UMLApp::app()->generator();
    if (cg == nullptr) {
        logDebug0("CodeGenerator is NULL : Assume UMLPrimitiveTypes");
        for (int i = 0; i < Uml::PrimitiveTypes::n_types; i++) {
            createDatatype(Uml::PrimitiveTypes::toString(i));
        }
    } else {
        QStringList entries = cg->defaultDatatypes();
        QStringList::Iterator end(entries.end());
        for (QStringList::Iterator it = entries.begin(); it != end; ++it) {
            createDatatype(*it);
        }
    }
    UMLApp::app()->listView()->closeDatatypesFolder();
}

/**
 * Add a datatype if it doesn't already exist.
 * Used by addDefaultDatatypes().
 */
UMLDatatype * UMLDoc::createDatatype(const QString &name)
{
    UMLObjectList datatypes = m_datatypeRoot->containedObjects(true);
    UMLObject* umlobject = Model_Utils::findUMLObject(datatypes, name,
                                                      UMLObject::ot_Datatype, m_datatypeRoot);
    UMLDatatype *dt = nullptr;
    if (umlobject)
        dt = umlobject->asUMLDatatype();
    if (dt) {
        dt->setActive(true);
        signalUMLObjectCreated(umlobject);
        qApp->processEvents();
    } else {
        if (umlobject) {
            logWarn1("UMLDoc::createDatatype(%1) : Name already exists but is not a Datatype", name);
        }
        umlobject = Object_Factory::createUMLObject(UMLObject::ot_Datatype,
                                                    Model_Utils::normalize(name), m_datatypeRoot);
        dt = dynamic_cast<UMLDatatype*>(umlobject);
    }
    return dt;
}

/**
 * Remove a datatype by name.
 * Used when changing the active programming language.
 */
void UMLDoc::removeDatatype(const QString &name)
{
    UMLObjectList datatypes = m_datatypeRoot->containedObjects();
    // We don't use Model_Utils::findUMLObject because that function considers
    // case sensitivity of the active language, which we don't want here.
    for(UMLObject *obj : datatypes) {
        uIgnoreZeroPointer(obj);
        if (obj->name() == name) {
            removeUMLObject(obj);
            break;
        }
    }
}

/**
 * Make a popup menu for the tabs
 * signalled from tabWidget's contextMenu().
 */
void UMLDoc::slotDiagramPopupMenu(QWidget* umlview, const QPoint& point)
{
    UMLView* view = (UMLView*) umlview;

    UMLListViewItem::ListViewType type = UMLListViewItem::lvt_Unknown;
    switch (view->umlScene()->type()) {
    case Uml::DiagramType::Class:
        type = UMLListViewItem::lvt_Class_Diagram;
        break;

    case Uml::DiagramType::UseCase:
        type = UMLListViewItem::lvt_UseCase_Diagram;
        break;

    case Uml::DiagramType::Sequence:
        type = UMLListViewItem::lvt_Sequence_Diagram;
        break;

    case Uml::DiagramType::Collaboration:
        type = UMLListViewItem::lvt_Collaboration_Diagram;
        break;

    case Uml::DiagramType::State:
        type = UMLListViewItem::lvt_State_Diagram;
        break;

    case Uml::DiagramType::Activity:
        type = UMLListViewItem::lvt_Activity_Diagram;
        break;

    case Uml::DiagramType::Component:
        type = UMLListViewItem::lvt_Component_Diagram;
        break;

    case Uml::DiagramType::Deployment:
        type = UMLListViewItem::lvt_Deployment_Diagram;
        break;

    case Uml::DiagramType::EntityRelationship:
        type = UMLListViewItem::lvt_EntityRelationship_Diagram;
        break;

    default:
        logWarn1("UMLDoc::slotDiagramPopupMenu: unknown diagram type %1", view->umlScene()->type());
        return;
    }//end switch

    UMLListViewItem item((UMLListView *)0, QString(), type);
    UMLListViewPopupMenu popup(UMLApp::app()->mainViewWidget(), &item);
    QAction *triggered = popup.exec(point);
    view->umlScene()->slotMenuSelection(triggered);
}

/**
 * Function for comparing tags in XMI files.
 */
bool UMLDoc::tagEq (const QString& inTag, const QString& inPattern)
{
    QString tag = inTag;
    QString pattern = inPattern;
    tag.remove(QRegularExpression(QStringLiteral("^\\w+:")));  // remove leading "UML:" or other
    int patSections = pattern.count(QLatin1Char('.')) + 1;
    QString tagEnd = tag.section(QLatin1Char('.'), -patSections);
    return (tagEnd.toLower() == pattern.toLower());
}

