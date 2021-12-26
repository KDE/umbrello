/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umldoc.h"

// app includes
#include "debug_utils.h"
#include "uniqueid.h"
#include "association.h"
#include "package.h"
#include "folder.h"
#include "codegenerator.h"
#include "classifier.h"
#include "dialog_utils.h"
#include "enum.h"
#include "entity.h"
#include "docwindow.h"
#include "operation.h"
#include "attribute.h"
#include "template.h"
#include "enumliteral.h"
#include "stereotype.h"
#include "datatype.h"
#include "classifierlistitem.h"
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
#include "umlscene.h"
#include "version.h"
#include "worktoolbar.h"
#include "models/diagramsmodel.h"
#include "models/objectsmodel.h"
#include "models/stereotypesmodel.h"

// kde includes
#include <kio/job.h>
#if QT_VERSION < 0x050000
#include <kio/netaccess.h>
#endif
#if QT_VERSION >= 0x050000
#include <KJobWidgets>
#endif
#include <KLocalizedString>
#include <KMessageBox>
#if QT_VERSION < 0x050000
#include <kmimetype.h>
#endif
#include <ktar.h>
#if QT_VERSION < 0x050000
#include <ktempdir.h>
#include <ktabwidget.h>
#include <ktemporaryfile.h>
#endif

// qt includes
#include <QApplication>
#include <QBuffer>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QListWidget>
#if QT_VERSION >= 0x050000
#include <QMimeDatabase>
#endif
#include <QPainter>
#include <QPrinter>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QTemporaryDir>
#include <QTemporaryFile>
#endif
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
    m_datatypeRoot(0),
    m_stereoList(UMLStereotypeList()),
    m_Name(i18n("UML Model")),
    m_modelID("m1"),
    m_count(0),
    m_modified(false),
#if QT_VERSION >= 0x050000
    m_doc_url(QUrl()),
#else
    m_doc_url(KUrl()),
#endif
    m_pChangeLog(0),
    m_bLoading(false),
    m_importing(false),
    m_Doc(QString()),
    m_pAutoSaveTimer(0),
    m_nViewID(Uml::ID::None),
    m_bTypesAreResolved(true),
    m_pCurrentRoot(0),
    m_bClosing(false),
    m_diagramsModel(new DiagramsModel),
    m_objectsModel(new ObjectsModel),
    m_stereotypesModel(new StereotypesModel(m_stereoList)),
    m_resolution(0.0)
{
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i)
        m_root[i] = 0;
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
    m_datatypeRoot = new UMLFolder(QLatin1String("Datatypes"), "Datatypes");
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
    if (view == 0) {
        uError() << "argument is NULL";
        return;
    }
    UMLFolder *f = view->umlScene()->folder();
    if (f == 0) {
        uError() << "view folder is not set";
        return;
    }
    DEBUG(DBG_SRC) << view->umlScene()->name() << " to folder " << *f << " (" << f->name() << ")";
    f->addView(view);
    m_diagramsModel->addDiagram(view);

    UMLApp * pApp = UMLApp::app();
    if (pApp->listView()) {
        connect(this, SIGNAL(sigObjectRemoved(UMLObject*)), view->umlScene(), SLOT(slotObjectRemoved(UMLObject*)));
    }

    if (!m_bLoading || pApp->currentView() == 0) {
        pApp->setCurrentView(view);
    }
    if (!m_bLoading) {
        view->show();
        emit sigDiagramChanged(view->umlScene()->type());
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
    if (!view) {
        uError() << "UMLDoc::removeView(UMLView *view) called with view = 0";
        return;
    }
    DEBUG(DBG_SRC) << "<" << view->umlScene()->name() << ">";
    if (UMLApp::app()->listView()) {
        disconnect(this, SIGNAL(sigObjectRemoved(UMLObject*)),
                   view->umlScene(), SLOT(slotObjectRemoved(UMLObject*)));
    }
    view->hide();
    UMLFolder *f = view->umlScene()->folder();
    if (f == 0) {
        uError() << view->umlScene()->name() << ": view->getFolder() returns NULL";
        return;
    }
    m_diagramsModel->removeDiagram(view);
    f->removeView(view);
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView == view) {
        UMLApp::app()->setCurrentView(0);
        UMLViewList viewList;
        m_root[Uml::ModelType::Logical]->appendViews(viewList);
        UMLView* firstView = 0;
        if (!viewList.isEmpty()) {
            firstView =  viewList.first();
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
    }
    delete view;
}

/**
 * Sets the URL of the document.
 *
 * @param url   The KUrl to set.
 */
#if QT_VERSION >= 0x050000
void UMLDoc::setUrl(const QUrl &url)
#else
void UMLDoc::setUrl(const KUrl &url)
#endif
{
    m_doc_url = url;
}

/**
 * Returns the KUrl of the document.
 *
 * @return  The KUrl of this UMLDoc.
 */
#if QT_VERSION >= 0x050000
const QUrl& UMLDoc::url() const
#else
const KUrl& UMLDoc::url() const
#endif
{
    return m_doc_url;
}

/**
 * Sets the URL of the document to "Untitled".
 */
void UMLDoc::setUrlUntitled()
{
#if QT_VERSION >= 0x050000
    m_doc_url.setUrl(m_doc_url.toString(QUrl::RemoveFilename) + i18n("Untitled"));
#else
    m_doc_url.setFileName(i18n("Untitled"));
#endif
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
    int want_save = KMessageBox::warningYesNoCancel(win,
                                     i18n("The current file has been modified.\nDo you want to save it?"),
                                     i18nc("warning message", "Warning"),
                                     KStandardGuiItem::save(), KStandardGuiItem::discard());
    switch(want_save) {
    case KMessageBox::Yes:
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

    case KMessageBox::No:
        setModified(false);
        closeDocument();
        completed=true;
        break;

    case KMessageBox::Cancel:
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
            foreach(UMLStereotype *s, stereotypes()) {
                m_stereotypesModel->removeStereotype(s);
                delete s;
            }
            m_stereoList.clear();
        }

        // Restore the datatype folder, it has been deleted above.
        createDatatypeFolder();
        // this creates too many items, only Logical View should be created
        listView->init();
    }
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
    UMLApp::app()->setCurrentView(0);
    setUrlUntitled();
    setResolution(qApp->desktop()->logicalDpiX());
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
#if QT_VERSION >= 0x050000
bool UMLDoc::openDocument(const QUrl& url, const char* format /* =0 */)
#else
bool UMLDoc::openDocument(const KUrl& url, const char* format /* =0 */)
#endif
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
#if QT_VERSION >= 0x050000
    QTemporaryFile tmpfile;
    tmpfile.open();
    QUrl dest(QUrl::fromLocalFile(tmpfile.fileName()));
    DEBUG(DBG_SRC) << "UMLDoc::openDocument: copy from " << url << " to " << dest << ".";
    KIO::FileCopyJob *job = KIO::file_copy(url, dest, -1, KIO::Overwrite);
    KJobWidgets::setWindow(job, UMLApp::app());
    job->exec();
    QFile file(tmpfile.fileName());
    if (job->error() || !file.exists()) {
        if (!file.exists())
            DEBUG(DBG_SRC) << "UMLDoc::openDocument: temporary file <" << file.fileName() << "> failed!";
        if (job->error())
           DEBUG(DBG_SRC) << "UMLDoc::openDocument: " << job->errorString();
        KMessageBox::error(0, i18n("The file <%1> does not exist.", url.toString()), i18n("Load Error"));
        setUrlUntitled();
        m_bLoading = false;
        newDocument();
        return false;
    }
#else
    QString tmpfile;
    KIO::NetAccess::download(url, tmpfile, UMLApp::app());

    QFile file(tmpfile);
    if (!file.exists()) {
        KMessageBox::error(0, i18n("The file %1 does not exist.", url.pathOrUrl()), i18n("Load Error"));
        setUrlUntitled();
        m_bLoading = false;
        newDocument();
        return false;
    }
#endif
    // status of XMI loading
    bool status = false;

    // check if the xmi file is a compressed archive like tar.bzip2 or tar.gz
    QString filetype = m_doc_url.fileName();
    QString mimetype;
    if (filetype.endsWith(QLatin1String(".tgz")) || filetype.endsWith(QLatin1String(".tar.gz"))) {
        mimetype = QLatin1String("application/x-gzip");
    } else if (filetype.endsWith(QLatin1String(".tar.bz2"))) {
        mimetype = QLatin1String("application/x-bzip");
    }

    if (mimetype.isEmpty() == false) {
#if QT_VERSION >= 0x050000
        KTar archive(file.fileName(), mimetype);
#else
        KTar archive(tmpfile, mimetype);
#endif
        if (archive.open(QIODevice::ReadOnly) == false) {
#if QT_VERSION >= 0x050000
            KMessageBox::error(0, i18n("The file %1 seems to be corrupted.", url.toString()), i18n("Load Error"));
#else
            KMessageBox::error(0, i18n("The file %1 seems to be corrupted.", url.pathOrUrl()), i18n("Load Error"));
            KIO::NetAccess::removeTempFile(tmpfile);
#endif
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
#if QT_VERSION >= 0x050000
                QMimeDatabase db;
                entryMimeType = db.mimeTypeForFile(*it, QMimeDatabase::MatchExtension).name();
#else
                entryMimeType = KMimeType::findByPath(*it, 0, true)->name();
#endif
                if (entryMimeType == QLatin1String("application/x-uml")) {
                    foundXMI = true;
                    break;
                }
            }
        }

        // if we found an XMI file, we have to extract it to a temporary file
        if (foundXMI == true) {
#if QT_VERSION >= 0x050000
            QTemporaryDir tmp_dir;
#else
            KTempDir tmp_dir;
#endif
            KArchiveEntry * entry;
            KArchiveFile * fileEntry;

            // try to cast the file entry in the archive to an archive entry
            entry = const_cast<KArchiveEntry*>(rootDir->entry(*it));
            if (entry == 0) {
#if QT_VERSION >= 0x050000
                KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.", url.toString()),
                                   i18n("Load Error"));
#else
                KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.", url.pathOrUrl()),
                                   i18n("Load Error"));
                KIO::NetAccess::removeTempFile(tmpfile);
#endif
                setUrlUntitled();
                m_bLoading = false;
                newDocument();
                return false;
            }

            // now try to cast the archive entry to a file entry, so that we can
            // extract the file
            fileEntry = dynamic_cast<KArchiveFile*>(entry);
            if (fileEntry == 0) {
#if QT_VERSION >= 0x050000
                KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.", url.toString()),
                                   i18n("Load Error"));
#else
                KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.", url.pathOrUrl()),
                                   i18n("Load Error"));
                KIO::NetAccess::removeTempFile(tmpfile);
#endif
                setUrlUntitled();
                m_bLoading = false;
                newDocument();
                return false;
            }

            // now we can extract the file to the temporary directory
#if QT_VERSION >= 0x050000
            fileEntry->copyTo(tmp_dir.path() + QLatin1Char('/'));

            // now open the extracted file for reading
            QFile xmi_file(tmp_dir.path() + QLatin1Char('/') + *it);
#else
            fileEntry->copyTo(tmp_dir.name());

            // now open the extracted file for reading
            QFile xmi_file(tmp_dir.name() + *it);
#endif
            if(!xmi_file.open(QIODevice::ReadOnly)) {
#if QT_VERSION >= 0x050000
                KMessageBox::error(0, i18n("There was a problem loading the extracted file: %1", url.toString()),
                                   i18n("Load Error"));
#else
                KMessageBox::error(0, i18n("There was a problem loading the extracted file: %1", url.pathOrUrl()),
                                   i18n("Load Error"));
                KIO::NetAccess::removeTempFile(tmpfile);
#endif
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
#if QT_VERSION >= 0x050000
                KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.", url.toString()),
                                   i18n("Load Error"));
#else
            KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.", url.pathOrUrl()),
                               i18n("Load Error"));
            KIO::NetAccess::removeTempFile(tmpfile);
#endif
            setUrlUntitled();
            m_bLoading = false;
            newDocument();
            return false;
        }

        archive.close();
    } else {
        // no, it seems to be an ordinary file
        if (!file.open(QIODevice::ReadOnly)) {
#if QT_VERSION >= 0x050000
            KMessageBox::error(0, i18n("There was a problem loading file: %1", url.toString()),
                               i18n("Load Error"));
#else
            KMessageBox::error(0, i18n("There was a problem loading file: %1", url.pathOrUrl()),
                               i18n("Load Error"));
            KIO::NetAccess::removeTempFile(tmpfile);
#endif
            setUrlUntitled();
            m_bLoading = false;
            newDocument();
            return false;
        }
        if (filetype.endsWith(QLatin1String(".mdl"))) {
            setUrlUntitled();
            m_bTypesAreResolved = false;
            status = Import_Rose::loadFromMDL(file);
            if (status) {
                if (UMLApp::app()->currentView() == 0) {
                    QString name = createDiagramName(Uml::DiagramType::Class, false);
                    createDiagram(m_root[Uml::ModelType::Logical], Uml::DiagramType::Class, name);
                    setCurrentRoot(Uml::ModelType::Logical);
                }
            }
        }
        else if (filetype.endsWith(QLatin1String(".zargo"))) {
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
#if QT_VERSION < 0x050000
    KIO::NetAccess::removeTempFile(tmpfile);
#endif
    m_bLoading = false;
    m_bTypesAreResolved = true;
    if (!status) {
#if QT_VERSION >= 0x050000
        QString msg = i18n("There was a problem loading file: %1", url.toString());
#else
        QString msg = i18n("There was a problem loading file: %1", url.pathOrUrl());
#endif
        if (m_d->errors.size() > 0)
            msg += QLatin1String("<br/>") + i18n("Reason: %1", m_d->errors.join(QLatin1String("<br/>")));
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
#if QT_VERSION >= 0x050000
bool UMLDoc::saveDocument(const QUrl& url, const char * format)
#else
bool UMLDoc::saveDocument(const KUrl& url, const char * format)
#endif
{
    Q_UNUSED(format);
    m_doc_url = url;
    bool uploaded = true;

    // first, we have to find out which format to use
#if QT_VERSION >= 0x050000
    QString strFileName = url.path();
#else
    QString strFileName = url.path(KUrl::RemoveTrailingSlash);
#endif
    QFileInfo fileInfo(strFileName);
    QString fileExt = fileInfo.completeSuffix();
    QString fileFormat = QLatin1String("xmi");
    if (fileExt == QLatin1String("xmi") || fileExt == QLatin1String("bak.xmi")) {
        fileFormat = QLatin1String("xmi");
    } else if (fileExt == QLatin1String("xmi.tgz") || fileExt == QLatin1String("bak.xmi.tgz")) {
        fileFormat = QLatin1String("tgz");
    } else if (fileExt == QLatin1String("xmi.tar.bz2") || fileExt == QLatin1String("bak.xmi.tar.bz2")) {
        fileFormat = QLatin1String("bz2");
    } else {
        fileFormat = QLatin1String("xmi");
    }

    initSaveTimer();

    if (fileFormat == QLatin1String("tgz") || fileFormat == QLatin1String("bz2")) {
        KTar * archive;
#if QT_VERSION >= 0x050000
        QTemporaryFile tmp_tgz_file;
#else
        KTemporaryFile tmp_tgz_file;
#endif
        tmp_tgz_file.setAutoRemove(false);
        tmp_tgz_file.open();

        // first we have to check if we are saving to a local or remote file
        if (url.isLocalFile()) {
            if (fileFormat == QLatin1String("tgz")) {  // check tgz or bzip
                archive = new KTar(url.toLocalFile(), QLatin1String("application/x-gzip"));
            } else {
                archive = new KTar(url.toLocalFile(), QLatin1String("application/x-bzip"));
            }
        } else {
            if (fileFormat == QLatin1String("tgz")) {  // check tgz or bzip2
                archive = new KTar(tmp_tgz_file.fileName(), QLatin1String("application/x-gzip"));
            } else {
                archive = new KTar(tmp_tgz_file.fileName(), QLatin1String("application/x-bzip"));
            }
        }

        // now check if we can write to the file
        if (archive->open(QIODevice::WriteOnly) == false) {
            uError() << "could not open" << archive->fileName();
#if QT_VERSION >= 0x050000
            KMessageBox::error(0, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
#else
            KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
#endif
            delete archive;
            return false;
        }

        // we have to create a temporary xmi file
        // we will add this file later to the archive
#if QT_VERSION >= 0x050000
        QTemporaryFile tmp_xmi_file;
#else
        KTemporaryFile tmp_xmi_file;
#endif
        tmp_xmi_file.setAutoRemove(false);
        if (!tmp_xmi_file.open()) {
            uError() << "could not open" << tmp_xmi_file.fileName();
#if QT_VERSION >= 0x050000
            KMessageBox::error(0, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
#else
            KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
#endif
            delete archive;
            return false;
        }
        saveToXMI(tmp_xmi_file); // save XMI to this file...

        // now add this file to the archive, but without the extension
        QString tmpQString = url.fileName();
        if (fileFormat == QLatin1String("tgz")) {
            tmpQString.remove(QRegExp(QLatin1String("\\.tgz$")));
        }
        else {
            tmpQString.remove(QRegExp(QLatin1String("\\.tar\\.bz2$")));
        }
        archive->addLocalFile(tmp_xmi_file.fileName(), tmpQString);

        if (!archive->close()) {
            uError() << "could not close" << archive->fileName();
#if QT_VERSION >= 0x050000
            KMessageBox::error(0, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
#else
            KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
#endif
            delete archive;
            return false;
        }
        // now the xmi file was added to the archive, so we can delete it
        tmp_xmi_file.setAutoRemove(true);

        // now we have to check, if we have to upload the file
        if (!url.isLocalFile()) {
#if QT_VERSION >= 0x050000
            KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(tmp_tgz_file.fileName()), m_doc_url);
            KJobWidgets::setWindow(job, UMLApp::app());
            job->exec();
            uploaded = !job->error();
#else
            uploaded = KIO::NetAccess::upload(tmp_tgz_file.fileName(), m_doc_url, UMLApp::app());
#endif
            if (!uploaded)
                uError() << "could not upload file" << tmp_tgz_file.fileName() << "to" << url;
        }

        // now the archive was written to disk (or remote) so we can delete the
        // objects
        tmp_tgz_file.setAutoRemove(true);
        delete archive;

    }
    else {
        // save as normal uncompressed XMI

#if QT_VERSION >= 0x050000
        QTemporaryFile tmpfile; // we need this tmp file if we are writing to a remote file
#else
        KTemporaryFile tmpfile; // we need this tmp file if we are writing to a remote file
#endif
        tmpfile.setAutoRemove(false);

        // save in _any_ case to a temp file
        // -> if something goes wrong during saveToXMI, the
        //     original content is preserved
        //     (e.g. if umbrello dies in the middle of the document model parsing
        //      for saveToXMI due to some problems)
        /// @todo insert some checks in saveToXMI to detect a failed save attempt

        // lets open the file for writing
        if (!tmpfile.open()) {
            uError() << "could not open" << tmpfile.fileName();
#if QT_VERSION >= 0x050000
            KMessageBox::error(0, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
#else
            KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
#endif
            return false;
        }
        saveToXMI(tmpfile); // save the xmi stuff to it
        tmpfile.close();

        // if it is a remote file, we have to upload the tmp file
        if (!url.isLocalFile()) {
#if QT_VERSION >= 0x050000
            KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(tmpfile.fileName()), m_doc_url);
            KJobWidgets::setWindow(job, UMLApp::app());
            job->exec();
            uploaded = !job->error();
#else
            uploaded = KIO::NetAccess::upload(tmpfile.fileName(), m_doc_url, UMLApp::app());
#endif
            if (!uploaded)
                uError() << "could not upload file" << tmpfile.fileName() << "to" << url;
        }
        else {
            // now remove the original file
#ifdef Q_OS_WIN
            tmpfile.setAutoRemove(true);
#if QT_VERSION >= 0x050000
            KIO::FileCopyJob* fcj = KIO::file_copy(QUrl::fromLocalFile(tmpfile.fileName()), url, -1, KIO::Overwrite);
#else
            KIO::FileCopyJob* fcj = KIO::file_copy(tmpfile.fileName(), url, -1, KIO::Overwrite);
#endif
#else
#if QT_VERSION >= 0x050000
            KIO::FileCopyJob* fcj = KIO::file_move(QUrl::fromLocalFile(tmpfile.fileName()), url, -1, KIO::Overwrite);
#else
            KIO::FileCopyJob* fcj = KIO::file_move(tmpfile.fileName(), url, -1, KIO::Overwrite);
#endif
#endif
#if QT_VERSION >= 0x050000
            KJobWidgets::setWindow(fcj, (QWidget*)UMLApp::app());
            fcj->exec();
            if (fcj->error()) {
                uError() << "Could not move" << tmpfile.fileName() << "to" << url;
                KMessageBox::error(0, i18n("There was a problem saving: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
#else
            if (KIO::NetAccess::synchronousRun(fcj, (QWidget*)UMLApp::app()) == false) {
                KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
#endif
                setUrlUntitled();
                return false;
            }
        }
    }
    if (!uploaded) {
#if QT_VERSION >= 0x050000
        KMessageBox::error(0, i18n("There was a problem uploading: %1", url.url(QUrl::PreferLocalFile)), i18n("Save Error"));
#else
        KMessageBox::error(0, i18n("There was a problem uploading file: %1", url.pathOrUrl()), i18n("Save Error"));
#endif
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
    UMLView *v = 0;
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
        uWarning() << "UMLDoc::findView : Returning null because DiagramType "
                   << type << " cannot be mapped to ModelType";
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
    UMLObject *o = 0;
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
                                 UMLObject *currentObj /* = 0 */)
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
    return 0;
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
    if (folder == 0)
        return 0;
    UMLObjectList list = folder->containedObjects();
    if (list.size() == 0)
        return 0;
    return Model_Utils::findUMLObjectRaw(list, name, type, 0);
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
    if (folder == 0)
        return 0;
    UMLObjectList list = folder->containedObjects();
    if (list.size() == 0)
        return 0;
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
        DEBUG(DBG_SRC) << object->name() << ": not adding type " << object->baseTypeStr();
        return false;
    }
    UMLPackage *pkg = object->umlPackage();
    if (pkg == 0) {
        pkg = currentRoot();
        DEBUG(DBG_SRC) << object->name() << ": no parent package set, assuming "
                       << pkg->name();
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
    emit sigWriteToStatusBar(text);
}

/**
 * Simple removal of an object.
 * @param object   the UMLObject to be removed
 */
void UMLDoc::slotRemoveUMLObject(UMLObject* object)
{
    //m_objectList.remove(object);
    UMLPackage *pkg = object->umlPackage();
    if (pkg == 0) {
        uError() << object->name() << ": parent package is not set !";
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
    UMLListViewItem *parentItem = 0;

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
    if (parentItem != 0 && Model_Utils::typeIsContainer(parentItem->type())) {
        UMLPackage *parentPkg = parentItem->umlObject()->asUMLPackage();
        return isUnique(name, parentPkg);
    }

    uError() << name << ": Not currently in a package";
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
        return (package->findObject(name) == 0);
    }

    // Not currently in a package: ERROR
    uError() << name << " (2): Not currently in a package";
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
    foreach (UMLStereotype *s, m_stereoList) {
        if (s->name() == name) {
            return s;
        }
    }
    return 0;
}

/**
 * Finds or creates a stereotype for the parent object.
 * @param name   the name of the stereotype
 * @return the found stereotype object or a just created one
 */
UMLStereotype* UMLDoc::findOrCreateStereotype(const QString &name)
{
    UMLStereotype *s = findStereotype(name);
    if (s != 0) {
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
    foreach (UMLStereotype *s, m_stereoList) {
        if (s->id() == id)
            return s;
    }
    return 0;
}

/**
 * Add a UMLStereotype to the application.
 * @param s  the stereotype to be added
 */
void UMLDoc::addStereotype(UMLStereotype *s)
{
    if (m_stereotypesModel->addStereotype(s))
        emit sigObjectCreated(s);
}

/**
 * Remove a UMLStereotype from the application.
 * @param s   the stereotype to be removed
 */
void UMLDoc::removeStereotype(UMLStereotype *s)
{
    if (m_stereotypesModel->removeStereotype(s))
        emit sigObjectRemoved(s);
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
    if (pkg == 0) {
        uError() << assoc->name() << ": parent package is not set !";
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
    UMLAssociation *ret = 0;
    foreach (UMLAssociation* a, assocs) {
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
        *swap = (ret != 0);
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
    if (assoc == 0) {
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
    if (assoc == 0) {
        return;
    }

    // First, check that this association has not already been added.
    // This may happen when loading old XMI files where all the association
    // information was taken from the <UML:AssocWidget> tag.
    UMLAssociationList assocs = associations();
    foreach (UMLAssociation* a,  assocs) {
        // check if its already been added (shouldn't be the case right now
        // as UMLAssociations only belong to one associationwidget at a time)
        if (a == assoc) {
            DEBUG(DBG_SRC) << "duplicate addition attempted";
            return;
        }
    }

    // If we get here it's really a new association.

    // Add the UMLAssociation at the owning UMLPackage.
    UMLPackage *pkg = assoc->umlPackage();
    if (pkg == 0) {
        uError() << assoc->name() << ": parent package is not set !";
        return;
    }
    pkg->addObject(assoc);

    // I don't believe this appropriate, UMLAssociations ARENT UMLWidgets -b.t.
    // emit sigObjectCreated(o);

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
            uWarning() << "called with unknown diagram type";
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
            KMessageBox::error(0, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
        } else if (findView(type, name)) {
                KMessageBox::error(0, i18n("A diagram is already using that name."), i18n("Not a Unique Name"));
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
    DEBUG(DBG_SRC) << "folder=" << folder->name()
                   << " / type=" << Uml::DiagramType::toString(type)
                   << " / name=" << name;

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
        emit sigDiagramCreated(id);
        setModified(true);
        UMLApp::app()->enablePrint(true);
        changeCurrentView(id);
        return view;
    }
    return 0;
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
            KMessageBox::error(0, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
        } else if (!findView(type, name)) {
            view->umlScene()->setName(name);
            emit sigDiagramRenamed(id);
            setModified(true);
            break;
        } else {
            KMessageBox::error(0, i18n("A diagram is already using that name."), i18n("Not a Unique Name"));
        }
    }
}

/**
 * Used to rename a @ref UMLObject.  The @ref UMLObject is to be an
 * actor, use case or concept.
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
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (isUnique(name)) {
            UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(o, name));
            setModified(true);
            break;
        } else {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        }
    }
    return;
}

/**
 * Used to rename an operation or attribute of a concept.
 *
 * @param o The attribute or operation to rename.
 */
void UMLDoc::renameChildUMLObject(UMLObject *o)
{
    UMLClassifier* p = o->umlParent()->asUMLClassifier();
    if (!p) {
        DEBUG(DBG_SRC) << "Cannot create object, no parent found.";
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
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (p->findChildObject(name) == 0
                    || ((o->baseType() == UMLObject::ot_Operation) && KMessageBox::warningYesNo(0,
                            i18n("The name you entered was not unique.\nIs this what you wanted?"),
                            i18n("Name Not Unique"), KGuiItem(i18n("Use Name")), KGuiItem(i18n("Enter New Name"))) == KMessageBox::Yes)) {
                UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(o, name));
                setModified(true);
                break;
        } else {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
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
    DEBUG(DBG_SRC) << "id=" << Uml::ID::toString(id);
    UMLView* view = findView(id);
    if (view) {
        UMLScene* scene = view->umlScene();
        scene->setIsOpen(true);
        UMLApp::app()->setCurrentView(view);
        emit sigDiagramChanged(scene->type());
        UMLApp::app()->setDiagramMenuItemsState(true);
        setModified(true);
        emit sigCurrentViewChanged();
        // when clicking on a tab, the documentation of diagram is upated in docwindow
        UMLApp::app()->docWindow()->showDocumentation(scene);
    }
    else {
        uWarning() << "New current view was not found with id=" << Uml::ID::toString(id) << "!";
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
        uError() << "Request to remove diagram " << Uml::ID::toString(id) << ": Diagram not found!";
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
        uError() << "Request to remove diagram " << Uml::ID::toString(id) << ": Diagram not found!";
        return;
    }

    removeView(umlview);
    emit sigDiagramRemoved(id);
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
    if (currentView == 0) {
        if (m_pCurrentRoot) {
            return m_pCurrentRoot;
        }
        uError() << "m_pCurrentRoot is NULL";
        return 0;
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
    if (umlobject == 0) {
        uError() << "called with NULL parameter";
        return;
    }
    UMLApp::app()->docWindow()->updateDocumentation(true);
    UMLObject::ObjectType type = umlobject->baseType();

    umlobject->setUMLStereotype(0);  // triggers possible cleanup of UMLStereotype
    if (umlobject->asUMLClassifierListItem())  {
        UMLClassifier* parent = umlobject->umlParent()->asUMLClassifier();
        if (parent == 0) {
            uError() << "parent of umlobject is NULL";
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
            if (pClass == 0)  {
                uError() << "parent of umlobject has unexpected type "
                         << parent->baseType();
                return;
            }
            if (type == UMLObject::ot_Attribute) {
                pClass->removeAttribute(umlobject->asUMLAttribute());
            } else if (type == UMLObject::ot_Template) {
                pClass->removeTemplate(umlobject->asUMLTemplate());
                if (deleteObject)
                    delete umlobject->asUMLTemplate();
            } else {
                uError() << "umlobject has unexpected type " << type;
            }
        }
    } else if (type == UMLObject::ot_Association) {
        UMLAssociation *a = umlobject->asUMLAssociation();
        removeAssociation(a, false);  // don't call setModified here, it's done below
        emit sigObjectRemoved(umlobject);
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
                if (rootPkg == 0) {
                    uError() << umlobject->name() << ": root package is not set !";
                    return;
                }
                UMLObjectList rootObjects = rootPkg->containedObjects();
                // Store the associations to remove in a buffer because we
                // should not remove elements from m_objectList while it is
                // being iterated over.
                UMLAssociationList assocsToRemove;
                foreach (UMLObject *obj, rootObjects) {
                    uIgnoreZeroPointer(obj);
                    if (obj->baseType() == UMLObject::ot_Association) {
                        UMLAssociation *assoc = obj->asUMLAssociation();
                        if (c->hasAssociation(assoc)) {
                            assocsToRemove.append(assoc);
                        }
                    }
                }
                foreach (UMLAssociation *a, assocsToRemove) {
                    removeAssociation(a, false);
                }
            }
            pkg->removeObject(umlobject);
            emit sigObjectRemoved(umlobject);
            if (deleteObject)
                delete umlobject;
        } else {
            uError() << umlobject->name() << ": parent package is not set !";
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
    emit sigObjectCreated(o);
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
        uDebug() << "screen dpi:" << qApp->desktop()->logicalDpiX()
                 << "file dpi:" <<  resolution
                 << "scale:" << qApp->desktop()->logicalDpiX() / resolution;
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
    if (resolution() != 0.0)
        return (qreal)qApp->desktop()->logicalDpiX() / resolution();
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
    writer.setCodec("UTF-8");
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(1);
    // writer.writeProcessingInstruction(QLatin1String("xml"));
    writer.writeStartDocument();
    QString expoText(QLatin1String("umbrello uml modeller "));
    expoText += QLatin1String(umbrelloVersion());
    expoText += QLatin1String(" http://umbrello.kde.org");
    if (Settings::optionState().generalState.uml2) {
        writer.writeStartElement(QLatin1String("xmi:XMI"));
        writer.writeAttribute(QLatin1String("xmi:version"), QLatin1String("2.1"));
        writer.writeAttribute(QLatin1String("xmlns:xmi"), QLatin1String("http://schema.omg.org/spec/XMI/2.1"));
        writer.writeAttribute(QLatin1String("xmlns:xsi"), QLatin1String("http://www.w3.org/2001/XMLSchema-instance"));
        writer.writeStartElement(QLatin1String("xmi:Documentation"));
        writer.writeAttribute(QLatin1String("exporter"), expoText);
        writer.writeAttribute(QLatin1String("exporterVersion"), QLatin1String(XMI2_FILE_VERSION));
        writer.writeEndElement();  // xmi:Documentation
        writer.writeStartElement(QLatin1String("uml:Model"));
        writer.writeAttribute(QLatin1String("xmi:id"), Uml::ID::toString(m_modelID));
    } else {
        writer.writeStartElement(QLatin1String("XMI"));
        writer.writeAttribute(QLatin1String("xmi.version"), QLatin1String("1.2"));
        QDateTime now = QDateTime::currentDateTime();
        writer.writeAttribute(QLatin1String("timestamp"), now.toString(Qt::ISODate));
        writer.writeAttribute(QLatin1String("verified"), QLatin1String("false"));

        writer.writeNamespace(QLatin1String("http://schema.omg.org/spec/UML/1.4"), QLatin1String("UML"));
        writer.writeStartElement(QLatin1String("XMI.header"));

        writer.writeStartElement(QLatin1String("XMI.documentation"));
        writer.writeTextElement(QLatin1String("XMI.exporter"), expoText);

        writer.writeTextElement(QLatin1String("XMI.exporterVersion"), QLatin1String(XMI1_FILE_VERSION));

        // all files are now saved with correct Unicode encoding, we add this
        // information to the header, so that the file will be loaded correctly
        writer.writeTextElement(QLatin1String("XMI.exporterEncoding"), QLatin1String("UnicodeUTF8"));
        writer.writeEndElement();  // XMI.documentation
        writer.writeStartElement(QLatin1String("XMI.metamodel"));
        writer.writeAttribute(QLatin1String("xmi.name"), QLatin1String("UML"));
        writer.writeAttribute(QLatin1String("xmi.version"), QLatin1String("1.4"));
        writer.writeAttribute(QLatin1String("href"), QLatin1String("UML.xml"));
        writer.writeEndElement();  // XMI.metamodel
        writer.writeEndElement();  // XMI.header
        writer.writeStartElement(QLatin1String("XMI.content"));             // content
        writer.writeStartElement(QLatin1String("UML:Model"));
        writer.writeAttribute(QLatin1String("xmi.id"), Uml::ID::toString(m_modelID));
    }

    writer.writeAttribute(QLatin1String("name"), m_Name);
    if (! Settings::optionState().generalState.uml2) {
        writer.writeAttribute(QLatin1String("isSpecification"), QLatin1String("false"));
        writer.writeAttribute(QLatin1String("isAbstract"), QLatin1String("false"));
        writer.writeAttribute(QLatin1String("isRoot"), QLatin1String("false"));
        writer.writeAttribute(QLatin1String("isLeaf"), QLatin1String("false"));

        writer.writeStartElement(QLatin1String("UML:Namespace.ownedElement"));  // ownedNS
    }

    // Save stereotypes and toplevel datatypes first so that upon loading
    // they are known first.
    // There is a bug causing duplication of the same stereotype in m_stereoList.
    // As a workaround, we use a string list to memorize which stereotype has been saved.
    QStringList stereoNames;
    foreach (UMLStereotype *s, m_stereoList) {
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
        writer.writeStartElement(QLatin1String("xmi:Extension"));
        writer.writeAttribute(QLatin1String("extender"), QLatin1String("umbrello"));
    } else {
        writer.writeStartElement(QLatin1String("XMI.extensions"));
        writer.writeAttribute(QLatin1String("xmi.extender"), QLatin1String("umbrello"));
    }

    writer.writeStartElement(QLatin1String("docsettings"));
    Uml::ID::Type viewID = Uml::ID::None;
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView) {
        viewID = currentView->umlScene()->ID();
    }
    writer.writeAttribute(QLatin1String("viewid"), Uml::ID::toString(viewID));
    writer.writeAttribute(QLatin1String("documentation"), m_Doc);
    writer.writeAttribute(QLatin1String("uniqueid"), Uml::ID::toString(UniqueID::get()));
    writer.writeEndElement();  // docsettings

    //  save listview
    UMLApp::app()->listView()->saveToXMI(writer);

    // save code generator
    CodeGenerator *codegen = UMLApp::app()->generator();
    if (codegen) {
        writer.writeStartElement(QLatin1String("codegeneration"));
        codegen->saveToXMI(writer);
        writer.writeEndElement();  // codegeneration
    }

    writer.writeEndElement();  // XMI.extensions
    writer.writeEndElement();  // XMI
    writer.writeEndDocument();
}

/**
 * Checks the given XMI file if it was saved with correct Unicode
 * encoding set or not.
 *
 * @param file   The file to be checked.
 */
short UMLDoc::encoding(QIODevice & file)
{
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString data = stream.readAll();
    QString error;
    int line;
    QDomDocument doc;
    if (!doc.setContent(data, false, &error, &line)) {
        uWarning() << "Cannot set content: " << error << " Line: " << line;
        return ENC_UNKNOWN;
    }

    // we start at the beginning and go to the point in the header where we can
    // find out if the file was saved using Unicode
    QDomNode node = doc.firstChild();
    short enc = ENC_UNKNOWN;
    while (node.isComment() || node.isProcessingInstruction()) {
        if (node.isProcessingInstruction()) {
            const QDomProcessingInstruction& pi = node.toProcessingInstruction();
            QRegExp rx(QLatin1String("\\bencoding=['\"]([^'\"]+)['\"]"));
            const int pos = rx.indexIn(pi.data());
            if (pos >= 0) {
                const QString& encData = rx.cap(1);
                if (encData == QLatin1String("UTF-8")) {
                    enc = ENC_UNICODE;
                } else if (encData == QLatin1String("windows-1252")) {
                    enc = ENC_WINDOWS;
                } else {
                    uDebug() << "ProcessingInstruction encoding=" << encData << " is not yet implemented";
                    enc = ENC_OLD_ENC;
                }
            }
        }
        node = node.nextSibling();
    }
    QDomElement root = node.toElement();
    if (root.isNull()) {
        uDebug() << "Null element at " << node.nodeName() << " : " << node.nodeValue();
        return enc;
    }
    //  make sure it is an XMI file
    if (root.tagName() != QLatin1String("XMI") && root.tagName() != QLatin1String("xmi:XMI")) {
        uDebug() << "Unknown tag at " << root.tagName();
        return enc;
    }

    if (node.firstChild().isNull()) {
        uDebug() << "No child at " << node.nodeName() << " : " << node.nodeValue();
        return enc;
    }
    node = node.firstChild();

    QDomElement element = node.toElement();
    // check header
    if (element.isNull()) {
        uDebug() << "No element at " << node.nodeName() << " : " << node.nodeValue();
        return enc;
    }
    if (element.tagName() == QLatin1String("xmi:Documentation")) {
        // UML2 format
        return ENC_UNICODE;
    }
    if (element.tagName() != QLatin1String("XMI.header")) {
        uDebug() << "Expecting XMI.header at " << element.tagName();
        return enc;
    }

    QDomNode headerNode = node.firstChild();
    while (!headerNode.isNull()) {
        QDomElement headerElement = headerNode.toElement();
        // the information if Unicode was used is now stored in the
        // XMI.documentation section of the header
        if (headerElement.isNull() ||
                headerElement.tagName() != QLatin1String("XMI.documentation")) {
            headerNode = headerNode.nextSibling();
            continue;
        }
        QDomNode docuNode = headerNode.firstChild();
        while (!docuNode.isNull()) {
            QDomElement docuElement = docuNode.toElement();
            // a tag XMI.exporterEncoding was added since version 1.2 to
            // mark a file as saved with Unicode
            if (! docuElement.isNull() &&
                    docuElement.tagName() == QLatin1String("XMI.exporterEncoding")) {
                // at the moment this isn't really necessary but maybe
                // later we will have other encoding standards
                if (docuElement.text() == QLatin1String("UnicodeUTF8")) {
                    return ENC_UNICODE; // stop here
                }
            }
            docuNode = docuNode.nextSibling();
        }
        break;
    }
    return ENC_OLD_ENC;
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
        stream.setCodec("UTF-8");
    } else if (encode == ENC_WINDOWS) {
        stream.setCodec("windows-1252");
    }

    QString data = stream.readAll();
    qApp->processEvents();  // give UI events a chance
    QString error;
    int line;
    QDomDocument doc;
    if (!doc.setContent(data, false, &error, &line)) {
        uWarning() << "Cannot set content:" << error << " Line:" << line;
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
    //  make sure it is an XMI file
    if (root.tagName() != QLatin1String("XMI") && root.tagName() != QLatin1String("xmi:XMI")) {
        return false;
    }

    QString versionString = root.attribute(QLatin1String("xmi.version"));
    if (versionString.isEmpty())
        versionString = root.attribute(QLatin1String("xmi:version"));
    if (! versionString.isEmpty()) {
        double version = versionString.toDouble();
        if (version < 1.0) {
            QString error = i18n("Unsupported xmi file version: %1", versionString);
            m_d->errors << error;
            DEBUG(DBG_SRC) << error;
            return false;
        }
    }

    m_nViewID = Uml::ID::None;
    for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isComment()) {
            continue;
        }
        QDomElement element = node.toElement();
        if (element.isNull()) {
            DEBUG(DBG_SRC) << "loadFromXMI: skip empty elem";
            continue;
        }
        bool recognized = false;
        QString outerTag = element.tagName();
        //check header
        if (outerTag == QLatin1String("XMI.header")) {
            QDomNode headerNode = node.firstChild();
            if (!validateXMI1Header(headerNode)) {
                return false;
            }
            recognized = true;
        } else if (outerTag == QLatin1String("XMI.extensions") ||
                   outerTag == QLatin1String("xmi:Extension")) {
            QDomNode extensionsNode = node.firstChild();
            while (! extensionsNode.isNull()) {
                loadExtensionsFromXMI1(extensionsNode);
                extensionsNode = extensionsNode.nextSibling();
            }
            recognized = true;
        } else if (tagEq(outerTag, QLatin1String("Model")) ||
                   tagEq(outerTag, QLatin1String("Package"))) {
            if(!loadUMLObjectsFromXMI(element)) {
                uWarning() << "failed load on objects";
                return false;
            }
            m_Name = element.attribute(QLatin1String("name"), i18n("UML Model"));
            UMLListView *lv = UMLApp::app()->listView();
            lv->setTitle(0, m_Name);
            recognized = true;
        }
        if (outerTag != QLatin1String("XMI.content")) {
            if (!recognized) {
                DEBUG(DBG_SRC) << "skipping <" << outerTag << ">";
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
            if (tag == QLatin1String("umlobjects")  // for bkwd compat.
                    || tagEq(tag, QLatin1String("Subsystem"))
                    || tagEq(tag, QLatin1String("Project"))  // Embarcadero's Describe
                    || tagEq(tag, QLatin1String("Model"))) {
                if(!loadUMLObjectsFromXMI(element)) {
                    uWarning() << "failed load on objects";
                    return false;
                }
                m_Name = element.attribute(QLatin1String("name"), i18n("UML Model"));
                UMLListView *lv = UMLApp::app()->listView();
                lv->setTitle(0, m_Name);
                seen_UMLObjects = true;
            } else if (tagEq(tag, QLatin1String("Package")) ||
                       tagEq(tag, QLatin1String("Class")) ||
                       tagEq(tag, QLatin1String("Interface"))) {
                // These tests are only for foreign XMI files that
                // are missing the <Model> tag (e.g. NSUML)
                QString stID = element.attribute(QLatin1String("stereotype"));
                UMLObject *pObject = Object_Factory::makeObjectFromXMI(tag, stID);
                if (!pObject) {
                    uWarning() << "Unknown type of umlobject to create: " << tag;
                    // We want a best effort, therefore this is handled as a
                    // soft error.
                    continue;
                }
                UMLObject::ObjectType ot = pObject->baseType();
                // Set the parent root folder.
                UMLPackage *pkg = 0;
                if (ot != UMLObject::ot_Stereotype) {
                    if (ot == UMLObject::ot_Datatype) {
                        pkg = m_datatypeRoot;
                    } else {
                        Uml::ModelType::Enum guess = Model_Utils::guessContainer(pObject);
                        if (guess != Uml::ModelType::N_MODELTYPES) {
                            pkg = m_root[guess];
                        }
                        else {
                            uError() << "Guess is Uml::ModelType::N_MODELTYPES - package not set correctly for "
                                     << pObject->name() << " / base type " << pObject->baseTypeStr();
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
            } else if (tagEq(tag, QLatin1String("TaggedValue"))) {
                // This tag is produced here, i.e. outside of <UML:Model>,
                // by the Unisys.JCR.1 Rose-to-XMI tool.
                if (! seen_UMLObjects) {
                    DEBUG(DBG_SRC) << "skipping TaggedValue because not seen_UMLObjects";
                    continue;
                }
                tag = element.attribute(QLatin1String("tag"));
                if (tag != QLatin1String("documentation")) {
                    continue;
                }
                QString modelElement = element.attribute(QLatin1String("modelElement"));
                if (modelElement.isEmpty()) {
                    DEBUG(DBG_SRC) << "skipping TaggedValue(documentation) because "
                                   << "modelElement.isEmpty()";
                    continue;
                }
                UMLObject *o = findObjectById(Uml::ID::fromString(modelElement));
                if (o == 0) {
                    DEBUG(DBG_SRC) << "TaggedValue(documentation): cannot find object"
                                   << " for modelElement " << modelElement;
                    continue;
                }
                QString value = element.attribute(QLatin1String("value"));
                if (! value.isEmpty()) {
                    o->setDoc(value);
                }
            } else {
                // for backward compatibility
                loadExtensionsFromXMI1(child);
            }
        }
    }

    resolveTypes();
    loadDiagrams1();

    // set a default code generator if no <XMI.extensions><codegeneration> tag seen
    if (UMLApp::app()->generator() == 0) {
        UMLApp::app()->setGenerator(UMLApp::app()->defaultLanguage());
    }
    emit sigWriteToStatusBar(i18n("Setting up the document..."));
    qApp->processEvents();  // give UI events a chance
    activateAllViews();

    UMLView *viewToBeSet = 0;
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
    emit sigResetStatusbarProgress();
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
        DEBUG(DBG_SRC) << "UMLDoc: invoking resolveRef() for " << obj->name()
                       << " (id=" << Uml::ID::toString(obj->id()) << ")";
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
        foreach(QDomNode node, i.value())
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
    emit sigResetStatusbarProgress();
    emit sigSetStatusbarProgress(0);
    emit sigSetStatusbarProgressSteps(10);
    m_count = 0;
     */
    emit sigWriteToStatusBar(i18n("Loading UML elements..."));

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
        if (tagEq(type, QLatin1String("Model"))) {
            // Handling of Umbrello native XMI files:
            // We get here from a recursive call to loadUMLObjectsFromXMI()
            // a few lines below, see
            //       if (tagEq(type, "Namespace.ownedElement") ....
            // Inside this Namespace.ownedElement envelope there are the
            // four submodels:
            // <UML:Model name="Logical View">
            // <UML:Model name="Use Case View">
            // <UML:Model name="Component View">
            // <UML:Model name="Deployment View">
            // These are ultimately loaded by UMLFolder::loadFromXMI()
            // Furthermore, in Umbrello native XMI format this
            // Namespace.ownedElement is the container of all stereotypes
            // (<UML:Stereotype>).
            bool foundUmbrelloRootFolder = false;
            QString name = tempElement.attribute(QLatin1String("name"));
            for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
                if (name == m_root[i]->name()) {
                    UMLFolder *curRootSave = m_pCurrentRoot;
                    m_pCurrentRoot = m_root[i];
                    if (!m_pCurrentRoot->loadFromXMI(tempElement)) {
                        uWarning() << "failed load on " << name;
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
        if (tagEq(type, QLatin1String("Namespace.ownedElement")) ||
                tagEq(type, QLatin1String("Namespace.contents")) ||
                tagEq(type, QLatin1String("Element.ownedElement")) ||  // Embarcadero's Describe
                tagEq(type, QLatin1String("Model"))) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore the <UML:Namespace.ownedElement> tag is of no
            // significance.
            // The tagEq(type, "Namespace.contents") and tagEq(type, "Model")
            // tests do not become true for Umbrello native files, only for
            // some foreign XMI files.
            if (!loadUMLObjectsFromXMI(tempElement)) {
                uWarning() << "failed load on " << type;
                return false;
            }
            continue;
        }
        // From here on, it's support for stereotypes, pre 1.5.5 versions,
        // and foreign files
        if (Model_Utils::isCommonXMI1Attribute(type)) {
            continue;
        } else if (tagEq(type, QLatin1String("packagedElement")) ||
                   tagEq(type, QLatin1String("ownedElement"))) {
            type = tempElement.attribute(QLatin1String("xmi:type"));
        }
        if (!tempElement.hasAttribute(QLatin1String("xmi.id")) &&
            !tempElement.hasAttribute(QLatin1String("xmi:id"))) {
            QString idref = tempElement.attribute(QLatin1String("xmi.idref"));
            if (! idref.isEmpty()) {
                DEBUG(DBG_SRC) << "resolution of xmi.idref " << idref
                               << " is not yet implemented";
            } else {
                uError() << "Cannot load " << type
                         << " because xmi.id is missing";
            }
            continue;
        }
        QString stID = tempElement.attribute(QLatin1String("stereotype"));
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type, stID);
        if (!pObject) {
            uWarning() << "Unknown type of umlobject to create: " << type;
            // We want a best effort, therefore this is handled as a
            // soft error.
            continue;
        }
        UMLObject::ObjectType ot = pObject->baseType();
        // Set the parent root folder.
        UMLPackage *pkg = 0;
        if (ot != UMLObject::ot_Stereotype) {
            if (ot == UMLObject::ot_Datatype) {
                pkg = m_datatypeRoot;
            } else {
                Uml::ModelType::Enum guess = Model_Utils::guessContainer(pObject);
                if (guess != Uml::ModelType::N_MODELTYPES) {
                    pkg = m_root[guess];
                }
                else {
                    uError() << "Guess is Uml::ModelType::N_MODELTYPES - package not set correctly for "
                             << pObject->name() << " / base type " << pObject->baseTypeStr();
                    pkg = m_root[Uml::ModelType::Logical];
                }
            }
        }
        pObject->setUMLPackage(pkg);

        bool status = pObject->loadFromXMI(tempElement);
        if (!status) {
            delete pObject;
            return false;
        }
        pkg = pObject->umlPackage();
        if (ot == UMLObject::ot_Stereotype) {
            UMLStereotype *s = pObject->asUMLStereotype();
            UMLStereotype *exist = findStereotype(pObject->name());
            if (exist) {
                if (exist->id() == pObject->id()) {
                    delete pObject;
                } else {
                    DEBUG(DBG_SRC) << "Stereotype " << pObject->name()
                                   << "(id=" << Uml::ID::toString(pObject->id())
                                   << ") already exists with id="
                                   << Uml::ID::toString(exist->id());
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
                DEBUG(DBG_SRC) << "CHECK: adding " << pObject->name()
                               << " to " << pkg->name();
                if (!pkg->addObject(pObject)) {
                    DEBUG(DBG_SRC) << "pkg->addObject failed";
                }
            }
        }
        else if (ot != UMLObject::ot_Stereotype) {
            uError() << "Package is NULL for " << pObject->name();
            return false;
        }

        /* FIXME see comment at loadUMLObjectsFromXMI
        emit sigSetStatusbarProgress(++m_count);
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

    if (tag == QLatin1String("docsettings")) {
        QString viewID = element.attribute(QLatin1String("viewid"), QLatin1String("-1"));
        m_Doc = element.attribute(QLatin1String("documentation"));
        QString uniqueid = element.attribute(QLatin1String("uniqueid"), QLatin1String("0"));

        m_nViewID = Uml::ID::fromString(viewID);
        UniqueID::set(Uml::ID::fromString(uniqueid));
        UMLApp::app()->docWindow()->reset();

    } else if (tag == QLatin1String("diagrams") || tag == QLatin1String("UISModelElement")) {
        // For backward compatibility only:
        // Since version 1.5.5 diagrams are saved as part of the UMLFolder.
        QDomNode diagramNode = node.firstChild();
        if (tag == QLatin1String("UISModelElement")) {          // Unisys.IntegratePlus.2
            element = diagramNode.toElement();
            tag = element.tagName();
            if (tag != QLatin1String("uisOwnedDiagram")) {
                uError() << "unknown child node " << tag;
                return;
            }
            diagramNode = diagramNode.firstChild();
        } else {
            qreal resolution = 0.0;
            QString res = node.toElement().attribute(QLatin1String("resolution"), QLatin1String(""));
            if (!res.isEmpty()) {
               resolution = res.toDouble();
            }
            if (resolution != 0.0) {
               UMLApp::app()->document()->setResolution(resolution);
            } else {
               // see UMLFolder::loadDiagramsFromXMI()
               UMLApp::app()->document()->setResolution(0.0);
            }
        }
        if (!loadDiagramsFromXMI1(diagramNode)) {
            uWarning() << "failed load on diagrams";
        }

    } else if (tag == QLatin1String("listview")) {
        //FIXME: Need to resolveTypes() before loading listview,
        //       else listview items are duplicated.
        resolveTypes();
        if (!UMLApp::app()->listView()->loadFromXMI(element)) {
            uWarning() << "failed load on listview";
        }

    } else if (tag == QLatin1String("codegeneration")) {
        QDomNode cgnode = node.firstChild();
        QDomElement cgelement = cgnode.toElement();
        while (!cgelement.isNull()) {
            QString nodeName = cgelement.tagName();
            QString lang = cgelement.attribute(QLatin1String("language"), QLatin1String("UNKNOWN"));
            Uml::ProgrammingLanguage::Enum pl = Uml::ProgrammingLanguage::fromString(lang);
            if (pl != Uml::ProgrammingLanguage::Reserved) {
                CodeGenerator *g = UMLApp::app()->setGenerator(pl);
                g->loadFromXMI(cgelement);
            } else
                uDebug() << "codegeneration : no setup performed for" << lang;
            cgnode = cgnode.nextSibling();
            cgelement = cgnode.toElement();
        }
        if (UMLApp::app()->generator() == 0) {
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
    emit sigWriteToStatusBar(i18n("Loading diagrams..."));
    emit sigResetStatusbarProgress();
    emit sigSetStatusbarProgress(0);
    emit sigSetStatusbarProgressSteps(10); //FIX ME
    QDomElement element = node.toElement();
    if (element.isNull()) {
        return true;  //return ok as it means there is no umlobjects
    }
    const Settings::OptionState state = Settings::optionState();
    UMLView * pView = 0;
    int count = 0;
    while (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QLatin1String("diagram") || tag == QLatin1String("UISDiagram")) {
            pView = new UMLView(0);
            // IMPORTANT: Set OptionState of new UMLView _BEFORE_
            // reading the corresponding diagram:
            // + allow using per-diagram color and line-width settings
            // + avoid crashes due to uninitialized values for lineWidth
            pView->umlScene()->setOptionState(state);
            bool success = false;
            if (tag == QLatin1String("UISDiagram")) {
                success = pView->umlScene()->loadUISDiagram(element);
            } else {
                success = pView->umlScene()->loadFromXMI(element);
            }
            if (!success) {
                uWarning() << "failed load on viewdata loadfromXMI";
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
                uWarning() << "cannot add " << tag << " because umlScene type "
                           << pView->umlScene()->type() << " cannot be mapped to ModelType";
            }
            emit sigSetStatusbarProgress(++count);
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

    UMLApp::app()->setCurrentView(0);
    emit sigDiagramChanged(Uml::DiagramType::Undefined);
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
 * Returns a list of the classes and interfaces in this UMLDoc.
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
    foreach (UMLObject *obj, objects) {
        uIgnoreZeroPointer(obj);
        if (obj->isUMLDatatype()) {
            datatypeList.append(obj->asUMLClassifier());
        }
    }
    return datatypeList;
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

        foreach (UMLAssociation* a, assocs) {
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
    UMLView * printView = 0;
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
        printView = 0;
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
    foreach(UMLView *v, viewIterator()) {
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
        DEBUG(DBG_SRC) << "no obj || Changelog";
        return false;
    }
    Uml::ID::Type result = assignNewID(obj->id());
    obj->setID(result);

    //If it is a CONCEPT then change the ids of all its operations and attributes
    if (obj->baseType() == UMLObject::ot_Class) {
        UMLClassifier *c = obj->asUMLClassifier();
        UMLClassifierListItemList attributes = c->getFilteredList(UMLObject::ot_Attribute);
        foreach (UMLObject* listItem,  attributes) {
            result = assignNewID(listItem->id());
            listItem->setID(result);
        }

        UMLClassifierListItemList templates = c->getFilteredList(UMLObject::ot_Template);
        foreach (UMLObject* listItem, templates) {
            result = assignNewID(listItem->id());
            listItem->setID(result);
        }
    }

    if (obj->baseType() == UMLObject::ot_Interface || obj->baseType() == UMLObject::ot_Class) {
        UMLOperationList operations(((UMLClassifier*)obj)->getOpList());
        foreach (UMLObject* listItem, operations) {
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
        uError() << "illegal input value " << Uml::ModelType::toString(mt);
        return 0;
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
        m_pChangeLog = 0;
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
        m_pChangeLog = 0;
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
    while (findView(pView->umlScene()->type(), name) != 0) {
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

    emit sigDiagramCreated(pView->umlScene()->ID());

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
        m_pAutoSaveTimer = 0;
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
#if QT_VERSION >= 0x050000
    QUrl tempUrl = m_doc_url;
#else
    KUrl tempUrl = m_doc_url;
#endif
    if (tempUrl.fileName() == i18n("Untitled")) {
#if QT_VERSION >= 0x050000
        tempUrl.setScheme(QLatin1String("file"));
#endif
        tempUrl.setPath(QDir::homePath() + i18n("/autosave%1", QLatin1String(".xmi")));
        saveDocument(tempUrl);
        setUrlUntitled();
        m_modified = true;
        UMLApp::app()->setModified(m_modified);
    } else {
        // 2004-05-17 Achim Spangler
#if QT_VERSION >= 0x050000
        QUrl orgDocUrl = m_doc_url;
#else
        KUrl orgDocUrl = m_doc_url;
#endif
        QString orgFileName = m_doc_url.fileName();
        // don't overwrite manually saved file with autosave content
        QString fileName = tempUrl.fileName();
        Settings::OptionState optionState = Settings::optionState();
        fileName.replace(QLatin1String(".xmi"), optionState.generalState.autosavesuffix);
#if QT_VERSION >= 0x050000
        tempUrl.setUrl(tempUrl.toString(QUrl::RemoveFilename) + fileName);
#else
        tempUrl.setFileName(fileName);
#endif
        // End Achim Spangler

        saveDocument(tempUrl);
        // 2004-05-17 Achim Spangler
        // re-activate m_modified if autosave is writing to other file
        // than the main project file->autosave-suffix != ".xmi"
        if (optionState.generalState.autosavesuffix != QLatin1String(".xmi")) {
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
        emit sigDiagramRenamed(view->umlScene()->ID());
    }
    else {
      uError() << "Cannot signal diagram renamed - view is NULL!";
    }
}

/**
 * Calls the active code generator to create its default datatypes.
 */
void UMLDoc::addDefaultDatatypes()
{
    CodeGenerator *cg = UMLApp::app()->generator();
    if (cg == 0) {
        DEBUG(DBG_SRC) << "CodeGenerator is NULL : Assume UMLPrimitiveTypes";
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
void UMLDoc::createDatatype(const QString &name)
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
            uWarning() << "UMLDoc::createDatatype(" << name
                       << ") : Name already exists but is not a Datatype";
        }
        Object_Factory::createUMLObject(UMLObject::ot_Datatype, name, m_datatypeRoot);
    }
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
    foreach (UMLObject *obj, datatypes) {
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
        uWarning() << "unknown diagram type " << view->umlScene()->type();
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
    tag.remove(QRegExp(QLatin1String("^\\w+:")));  // remove leading "UML:" or other
    int patSections = pattern.count(QLatin1Char('.')) + 1;
    QString tagEnd = tag.section(QLatin1Char('.'), -patSections);
    return (tagEnd.toLower() == pattern.toLower());
}

