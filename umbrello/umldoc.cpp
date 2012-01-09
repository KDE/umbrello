/***************************************************************************
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  copyright (C) 2002-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

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
#include "enum.h"
#include "entity.h"
#include "docwindow.h"
#include "operation.h"
#include "attribute.h"
#include "template.h"
#include "enumliteral.h"
#include "stereotype.h"
#include "classifierlistitem.h"
#include "object_factory.h"
#include "import_rose.h"
#include "model_utils.h"
#include "uml.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlview.h"
#include "entityconstraint.h"
#include "idchangelog.h"
#include "listpopupmenu.h"
#include "cmds.h"
#include "diagramprintpage.h"
#include "umlscene.h"
#include "worktoolbar.h"

// kde includes
#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <ktar.h>
#include <ktempdir.h>
#include <ktemporaryfile.h>
#include <kinputdialog.h>
#include <ktabwidget.h>
#include <kapplication.h>

// qt includes
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>
#include <QtGui/QPainter>
#include <QtGui/QLabel>
#include <QtGui/QUndoStack>
#include <QtGui/QPrinter>
#include <QtXml/QDomElement>
#include <QtXml/QDomDocument>

// Update this version when changing the XMI file format
#define XMI_FILE_VERSION "1.5.9"

/**
 * Constructor for the fileclass of the application.
 */
UMLDoc::UMLDoc()
  : m_datatypeRoot(0),
    m_stereoList(UMLStereotypeList()),
    m_Name(i18n("UML Model")),
    m_modelID("m1"),
    m_count(0),
    m_modified(false),
    m_doc_url(KUrl()),
    m_pChangeLog(0),
    m_bLoading(false),
    m_Doc(QString()),
    m_pAutoSaveTimer(0),
    m_nViewID(Uml::id_None),
    m_bTypesAreResolved(false),
    m_pTabPopupMenu(0),
    m_pCurrentRoot(0),
    m_bClosing(false)
{
    DEBUG_REGISTER(DBG_SRC);
}

/**
 * Initialize the UMLDoc.
 * To be called after the constructor, before anything else.
 */
void UMLDoc::init()
{
    // Initialize predefined folders.
    const QString nativeRootName[Uml::ModelType::N_MODELTYPES] = {
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
        m_root[i] = new UMLFolder(nativeRootName[i], STR2ID(nativeRootName[i]));
        m_root[i]->setLocalName(localizedRootName[i]);
    }
    createDatatypeFolder();

    // Connect signals.
    UMLApp * pApp = UMLApp::app();
    connect(this, SIGNAL(sigDiagramCreated(Uml::IDType)), pApp, SLOT(slotUpdateViews()));
    connect(this, SIGNAL(sigDiagramRemoved(Uml::IDType)), pApp, SLOT(slotUpdateViews()));
    connect(this, SIGNAL(sigDiagramRenamed(Uml::IDType)), pApp, SLOT(slotUpdateViews()));
    connect(this, SIGNAL(sigCurrentViewChanged()),        pApp, SLOT(slotCurrentViewChanged()));
}

/**
 * Create the datatype folder and add it to the logical folder.
 */
void UMLDoc::createDatatypeFolder()
{
    delete m_datatypeRoot;
    m_datatypeRoot = new UMLFolder("Datatypes", "Datatypes");
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
    delete m_datatypeRoot;
    delete m_pChangeLog;
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
    DEBUG(DBG_SRC) << "to folder " << *f;
    f->addView(view);

    UMLApp * pApp = UMLApp::app();
    if ( pApp->listView() ) {
        connect(this, SIGNAL(sigObjectRemoved(UMLObject*)), view->umlScene(), SLOT(slotObjectRemoved(UMLObject*)));
    }

    pApp->setCurrentView(view);
    if ( !m_bLoading ) {
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
 * @param enforceOneView   switch to determine if we have a current view or not.
 *                         most of the time, we DO want this, except when exiting the program.
 */
void UMLDoc::removeView(UMLView *view , bool enforceCurrentView)
{
    if (!view) {
        uError() << "UMLDoc::removeView(UMLView *view) called with view = 0";
        return;
    }
    DEBUG(DBG_SRC) << "<" << view->umlScene()->name() << ">";
    if ( UMLApp::app()->listView() ) {
        disconnect(this, SIGNAL(sigObjectRemoved(UMLObject*)),
                   view->umlScene(), SLOT(slotObjectRemoved(UMLObject*)));
    }
    view->hide();
    //remove all widgets before deleting view
    view->umlScene()->removeAllWidgets();
    UMLFolder *f = view->umlScene()->folder();
    if (f == 0) {
        uError() << view->umlScene()->name() << ": view->getFolder() returns NULL";
        return;
    }
    f->removeView(view);
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView == view) {
        UMLApp::app()->setCurrentView(0);
        UMLViewList viewList;
        m_root[Uml::ModelType::Logical]->appendViews(viewList);
        UMLView* firstView = 0;
        if ( !viewList.isEmpty() ) {
            firstView =  viewList.first();
        }

        if (!firstView && enforceCurrentView) {  //create a diagram
            createDiagram(m_root[Uml::ModelType::Logical], Uml::DiagramType::Class, false);
            qApp->processEvents();
            m_root[Uml::ModelType::Logical]->appendViews(viewList);
            firstView = viewList.first();
        }

        if ( firstView ) {
            changeCurrentView( firstView->umlScene()->getID() );
            UMLApp::app()->setDiagramMenuItemsState(true);
        }
    }
}

/**
 * Sets the URL of the document.
 *
 * @param url   The KUrl to set.
 */
void UMLDoc::setUrl(const KUrl &url)
{
    m_doc_url = url;
}

/**
 * Returns the KUrl of the document.
 *
 * @return  The KUrl of this UMLDoc.
 */
const KUrl& UMLDoc::url() const
{
    return m_doc_url;
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
    m_Doc = "";
    DocWindow* dw = UMLApp::app()->docWindow();
    if (dw) {
        dw->newDocumentation();
    }

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
        for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
            m_root[i]->removeAllObjects();
        }
        // Restore the datatype folder, it has been deleted above.
        createDatatypeFolder();
        // this creates to much items only Logical View should be created
        listView->init();
        /* Remove any stereotypes.
        if (m_stereoList.count() > 0) {
            UMLStereotype *s;
            for (UMLStereotypeListIt sit(m_stereoList); (s = sit.current()) != 0; ++sit)
                delete s;
            m_stereoList.clear();
        }
        */
    }
    m_bClosing = false;
    m_bTypesAreResolved = false;
}

/**
 * Initializes the document generally.
 *
 * @return  True if operation successful.
 */
bool UMLDoc::newDocument()
{
    closeDocument();
    UMLApp::app()->setCurrentView(0);
    m_doc_url.setFileName(i18n("Untitled"));
    //see if we need to start with a new diagram
    Settings::OptionState optionState = Settings::optionState();
    Uml::DiagramType dt = optionState.generalState.diagram;
    Uml::ModelType mt = Model_Utils::convert_DT_MT(dt);
    if (mt == Uml::ModelType::N_MODELTYPES) {  // don't allow no diagram
        dt = Uml::DiagramType::Class;
        mt = Uml::ModelType::Logical;
    }
    createDiagram(m_root[mt], dt, false);

    UMLApp::app()->initGenerator();

    setModified(false);
    initSaveTimer();

    UMLApp::app()->enableUndo(false);
    UMLApp::app()->clearUndoStack();

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
bool UMLDoc::openDocument(const KUrl& url, const char* format /* =0 */)
{
    Q_UNUSED(format);
    if (url.fileName().length() == 0) {
        newDocument();
        return false;
    }

    m_doc_url = url;
    closeDocument();
    // IMPORTANT: set m_bLoading to true
    // _AFTER_ the call of UMLDoc::closeDocument()
    // as it sets m_bLoading to false after it was temporarily
    // changed to true to block recording of changes in redo-buffer
    m_bLoading = true;
    QString tmpfile;
    KIO::NetAccess::download(url, tmpfile, UMLApp::app());
    QFile file( tmpfile );
    if ( !file.exists() ) {
        KMessageBox::error(0, i18n("The file %1 does not exist.", url.pathOrUrl()), i18n("Load Error"));
        m_doc_url.setFileName(i18n("Untitled"));
        m_bLoading = false;
        newDocument();
        return false;
    }

    // status of XMI loading
    bool status = false;

    // check if the xmi file is a compressed archive like tar.bzip2 or tar.gz
    QString filetype = m_doc_url.fileName();
    QString mimetype = "";
    if (filetype.indexOf(QRegExp("\\.tgz$")) != -1) {
        mimetype = "application/x-gzip";
    } else if (filetype.indexOf(QRegExp("\\.tar.bz2$")) != -1) {
        mimetype = "application/x-bzip";
    }

    if (mimetype.isEmpty() == false) {
        KTar archive(tmpfile, mimetype);
        if (archive.open(QIODevice::ReadOnly) == false) {
            KMessageBox::error(0, i18n("The file %1 seems to be corrupted.", url.pathOrUrl()), i18n("Load Error"));
            m_doc_url.setFileName(i18n("Untitled"));
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
                entryMimeType = KMimeType::findByPath(*it, 0, true)->name();
                if (entryMimeType == "application/x-uml") {
                    foundXMI = true;
                    break;
                }
            }
        }

        // if we found an XMI file, we have to extract it to a temporary file
        if (foundXMI == true) {
            KTempDir tmp_dir;
            KArchiveEntry * entry;
            KArchiveFile * fileEntry;

            // try to cast the file entry in the archive to an archive entry
            entry = const_cast<KArchiveEntry*>(rootDir->entry(*it));
            if (entry == 0) {
                KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.", url.pathOrUrl()),
                                   i18n("Load Error"));
                m_doc_url.setFileName(i18n("Untitled"));
                m_bLoading = false;
                newDocument();
                return false;
            }

            // now try to cast the archive entry to a file entry, so that we can
            // extract the file
            fileEntry = dynamic_cast<KArchiveFile*>(entry);
            if (fileEntry == 0) {
                KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.", url.pathOrUrl()),
                                   i18n("Load Error"));
                m_doc_url.setFileName(i18n("Untitled"));
                m_bLoading = false;
                newDocument();
                return false;
            }

            // now we can extract the file to the temporary directory
            fileEntry->copyTo(tmp_dir.name());

            // now open the extracted file for reading
            QFile xmi_file(tmp_dir.name() + *it);
            if( !xmi_file.open( QIODevice::ReadOnly ) ) {
                KMessageBox::error(0, i18n("There was a problem loading the extracted file: %1", url.pathOrUrl()),
                                   i18n("Load Error"));
                m_doc_url.setFileName(i18n("Untitled"));
                m_bLoading = false;
                newDocument();
                return false;
            }
            status = loadFromXMI( xmi_file, ENC_UNKNOWN );

            // close the extracted file and the temporary directory
            xmi_file.close();
        } else {
            KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.", url.pathOrUrl()),
                               i18n("Load Error"));
            m_doc_url.setFileName(i18n("Untitled"));
            m_bLoading = false;
            newDocument();
            return false;
        }

        archive.close();
    } else {
        // no, it seems to be an ordinary file
        if( !file.open( QIODevice::ReadOnly ) ) {
            KMessageBox::error(0, i18n("There was a problem loading file: %1", url.pathOrUrl()),
                               i18n("Load Error"));
            m_doc_url.setFileName(i18n("Untitled"));
            m_bLoading = false;
            newDocument();
            return false;
        }
        if (filetype.endsWith(QLatin1String(".mdl"))) {
            status = Import_Rose::loadFromMDL(file);
        }
        else {
            status = loadFromXMI( file, ENC_UNKNOWN );
        }
    }

    file.close();
    KIO::NetAccess::removeTempFile( tmpfile );
    if ( !status ) {
        KMessageBox::error(0, i18n("There was a problem loading file: %1", url.pathOrUrl()),
                           i18n("Load Error"));
        m_bLoading = false;
        newDocument();
        return false;
    }
    setModified(false);
    m_bLoading = false;
    initSaveTimer();

    UMLApp::app()->enableUndo(false);
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
bool UMLDoc::saveDocument(const KUrl& url, const char * format)
{
    Q_UNUSED(format);
    m_doc_url = url;
    bool uploaded = true;

    // first, we have to find out which format to use
    QString strFileName = url.path(KUrl::RemoveTrailingSlash);
    QFileInfo fileInfo(strFileName);
    QString fileExt = fileInfo.completeSuffix();
    QString fileFormat = "xmi";
    if (fileExt == "xmi" || fileExt == "bak.xmi") {
        fileFormat = "xmi";
    } else if (fileExt == "xmi.tgz" || fileExt == "bak.xmi.tgz") {
        fileFormat = "tgz";
    } else if (fileExt == "xmi.tar.bz2" || fileExt == "bak.xmi.tar.bz2") {
        fileFormat = "bz2";
    } else {
        fileFormat = "xmi";
    }

    initSaveTimer();

    if (fileFormat == "tgz" || fileFormat == "bz2") {
        KTar * archive;
        KTemporaryFile tmp_tgz_file;
        tmp_tgz_file.setAutoRemove(false);
        tmp_tgz_file.open();

        // first we have to check if we are saving to a local or remote file
        if (url.isLocalFile()) {
            if (fileFormat == "tgz") {  // check tgz or bzip
                archive = new KTar(url.toLocalFile(), "application/x-gzip");
            } else {
                archive = new KTar(url.toLocalFile(), "application/x-bzip");
            }
        } else {
            if (fileFormat == "tgz") {  // check tgz or bzip2
                archive = new KTar(tmp_tgz_file.fileName(), "application/x-gzip");
            } else {
                archive = new KTar(tmp_tgz_file.fileName(), "application/x-bzip");
            }
        }

        // now check if we can write to the file
        if (archive->open(QIODevice::WriteOnly) == false) {
            KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
            delete archive;
            return false;
        }

        // we have to create a temporary xmi file
        // we will add this file later to the archive
        KTemporaryFile tmp_xmi_file;
        tmp_xmi_file.setAutoRemove(false);
        if ( !tmp_xmi_file.open() ) {
            KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
            delete archive;
            return false;
        }
        saveToXMI(tmp_xmi_file); // save XMI to this file...

        // now add this file to the archive, but without the extension
        QString tmpQString = url.fileName();
        if (fileFormat == "tgz") {
            tmpQString.remove(QRegExp("\\.tgz$"));
        }
        else {
            tmpQString.remove(QRegExp("\\.tar\\.bz2$"));
        }
        archive->addLocalFile(tmp_xmi_file.fileName(), tmpQString);

        if (!archive->close()) {
            KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
            delete archive;
            return false;
        }
        // now the xmi file was added to the archive, so we can delete it
        tmp_xmi_file.setAutoRemove(true);

        // now we have to check, if we have to upload the file
        if ( !url.isLocalFile() ) {
            uploaded = KIO::NetAccess::upload( tmp_tgz_file.fileName(), m_doc_url, UMLApp::app() );
        }

        // now the archive was written to disk (or remote) so we can delete the
        // objects
        tmp_tgz_file.setAutoRemove(true);
        delete archive;

    }
    else {
        // save as normal uncompressed XMI

        KTemporaryFile tmpfile; // we need this tmp file if we are writing to a remote file
        tmpfile.setAutoRemove(false);

        // save in _any_ case to a temp file
        // -> if something goes wrong during saveToXmi, the
        //     original content is preserved
        //     ( e.g. if umbrello dies in the middle of the document model parsing
        //      for saveToXMI due to some problems )
        /// @TODO insert some checks in saveToXMI to detect a failed save attempt

        // lets open the file for writing
        if ( !tmpfile.open() ) {
            KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
            return false;
        }
        saveToXMI(tmpfile); // save the xmi stuff to it

        // if it is a remote file, we have to upload the tmp file
        if ( !url.isLocalFile() ) {
            uploaded = KIO::NetAccess::upload( tmpfile.fileName(), m_doc_url, UMLApp::app() );
        }
        else {
            // now remove the original file
#ifdef Q_WS_WIN
            tmpfile.setAutoRemove(true);
            KIO::FileCopyJob* fcj = KIO::file_copy( tmpfile.fileName(), url, -1, KIO::Overwrite );
#else
            KIO::FileCopyJob* fcj = KIO::file_move( tmpfile.fileName(), url, -1, KIO::Overwrite );
#endif
            if ( KIO::NetAccess::synchronousRun( fcj, (QWidget*)UMLApp::app() ) == false ) {
                KMessageBox::error(0, i18n("There was a problem saving file: %1", url.pathOrUrl()), i18n("Save Error"));
                m_doc_url.setFileName(i18n("Untitled"));
                return false;
            }
        }
    }
    if ( !uploaded ) {
        KMessageBox::error(0, i18n("There was a problem uploading file: %1", url.pathOrUrl()), i18n("Save Error"));
        m_doc_url.setFileName(i18n("Untitled"));
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
    connect(this, SIGNAL(sigDiagramChanged(Uml::DiagramType)), tb, SLOT(slotCheckToolBar(Uml::DiagramType)));
    //new signals below
    return;
}

/**
 * Finds a view (diagram) by the ID given to method.
 *
 * @param id   The ID of the view to search for.
 * @return  Pointer to the view found, or NULL if not found.
 */
UMLView * UMLDoc::findView(Uml::IDType id)
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
UMLView * UMLDoc::findView(Uml::DiagramType type, const QString &name,
                           bool searchAllScopes /* =false */)
{
    Uml::ModelType mt = Model_Utils::convert_DT_MT(type);
    return m_root[mt]->findView(type, name, searchAllScopes);
}

/**
 * Used to find a reference to a @ref UMLObject by its ID.
 *
 * @param id   The @ref UMLObject to find.
 * @return  Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* UMLDoc::findObjectById(Uml::IDType id)
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
 * Find a UMLStereotype by its unique ID.
 * @param id   the unique ID
 * @return the found stereotype or NULL
 */
UMLStereotype * UMLDoc::findStereotypeById(Uml::IDType id)
{
    foreach (UMLStereotype *s , m_stereoList) {
        if (s->id() == id)
            return s;
    }
    return 0;
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
 * @param modelType    The Mmdel type in which to search for the object
 * @param name         The raw name of the @ref UMLObject to find.
 * @param type         ObjectType of the object to find
 * @return  Pointer to the UMLObject found, or NULL if not found.
 */
UMLObject* UMLDoc::findUMLObjectRaw(Uml::ModelType::Value modelType,
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

//:TODO:
//    /**
//     * Used to find a reference to a @ref UMLObject given its non-numeric
//     * ID string.
//     * Only used for intermediate processing while loading files
//     * containing objects with non-numeric xmi.id's.
//     *
//     * @param idStr   The AuxId for the @ref UMLObject to find.*/
//     * @return  Pointer to the UMLObject found, or NULL if not found.
//     */
//    UMLObject* UMLDoc::findObjectByAuxId(const QString &idStr)
//    {
//    }

/**
 * Used to find a @ref UMLClassifier by its name.
 *
 * @param name   The name of the @ref UMLObject to find.
 */
UMLClassifier* UMLDoc::findUMLClassifier(const QString &name)
{
    //this is used only by code generator so we don't need to look at Datatypes
    UMLObject * obj = findUMLObject(name);
    return dynamic_cast<UMLClassifier*>(obj);
}

/**
 * Adds a UMLObject thats already created but doesn't change
 * any ids or signal.  Use AddUMLObjectPaste if pasting.
 *
 * @param object   The object to add.
 * @return  True if the object was actually added.
 */
bool UMLDoc::addUMLObject(UMLObject* object)
{
    UMLObject::ObjectType ot = object->baseType();
    if (ot == UMLObject::ot_Attribute || ot == UMLObject::ot_Operation || ot == UMLObject::ot_EnumLiteral
            || ot == UMLObject::ot_EntityAttribute || ot == UMLObject::ot_Template || ot == UMLObject::ot_Stereotype) {
        DEBUG(DBG_SRC) << object->name() << ": not adding type " << ot;
        return false;
    }
    UMLPackage *pkg = object->umlPackage();
    if (pkg == 0) {
        pkg = currentRoot();
        DEBUG(DBG_SRC) << object->name() << ": no parent package set, assuming "
                       << pkg->name();
        object->setUMLPackage( pkg );
    }

    return pkg->addObject(object);
}

/**
 * Add a UMLStereotype to the application.
 * @param s  the stereotype to be added
 */
void UMLDoc::addStereotype(UMLStereotype *s)
{
    if (! m_stereoList.contains(s)) {
        m_stereoList.append(s);
    }
}

/**
 * Remove a UMLStereotype from the application.
 * @param s   the stereotype to be removed
 */
void UMLDoc::removeStereotype(UMLStereotype *s)
{
    if (m_stereoList.contains(s)) {
        m_stereoList.removeAll(s);
    }
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
bool UMLDoc::isUnique(const QString &name)
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
        UMLPackage *parentPkg = static_cast<UMLPackage*>(parentItem->umlObject());
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
bool UMLDoc::isUnique(const QString &name, UMLPackage *package)
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
 * Finds a UMLStereotype by its name.
 *
 * @param name   The name of the UMLStereotype to find.
 * @return  Pointer to the UMLStereotype found, or NULL if not found.
 */
UMLStereotype* UMLDoc::findStereotype(const QString &name)
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
    s = new UMLStereotype(name, STR2ID(name));
    addStereotype(s);
    return s;
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
 *                  The bool is set to true if the assocation
 *                  matched with swapped roles, else it is set
 *                  to false.
 * @return  Pointer to the UMLAssociation found or NULL if not found.
 */
UMLAssociation * UMLDoc::findAssociation(Uml::AssociationType assocType,
        const UMLObject *roleAObj,
        const UMLObject *roleBObj,
        bool *swap)
{
    UMLAssociationList assocs = associations();
    UMLAssociation *ret = 0;
    foreach ( UMLAssociation* a , assocs ) {
        if (a->getAssocType() != assocType) {
            continue;
        }
        if (a->getObject(Uml::A) == roleAObj && a->getObject(Uml::B) == roleBObj) {
            return a;
        }
        if (a->getObject(Uml::A) == roleBObj && a->getObject(Uml::B) == roleAObj) {
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
UMLAssociation* UMLDoc::createUMLAssociation(UMLObject *a, UMLObject *b, Uml::AssociationType type)
{
    bool swap;
    UMLAssociation *assoc = findAssociation(type, a, b, &swap);
    if (assoc == 0) {
        assoc = new UMLAssociation(type, a, b );
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
    foreach (UMLAssociation* a,  assocs ) {
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
QString UMLDoc::uniqueViewName(const Uml::DiagramType type)
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
            dname = i18n("collaboration diagram");
            break;
        case Uml::DiagramType::State:
            dname = i18n( "state diagram" );
            break;
        case Uml::DiagramType::Activity:
            dname = i18n( "activity diagram" );
            break;
        case Uml::DiagramType::Component:
            dname = i18n( "component diagram" );
            break;
        case Uml::DiagramType::Deployment:
            dname = i18n( "deployment diagram" );
            break;
        case Uml::DiagramType::EntityRelationship:
            dname = i18n( "entity relationship diagram" );
            break;
        default:
            uWarning() << "called with unknown diagram type";
            break;
    }
    QString name = dname;
    for (int number = 0; findView(type, name, true); ++number) {
        name = dname + '_' + QString::number(number);
    }
    return name;
}

/**
 * Returns true when loading a document file.
 * @return the value of the flag
 */
bool UMLDoc::loading() const
{
    return m_bLoading;
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
 * Returns the m_bClosing flag.
 * @return the value of the flag
 */
bool UMLDoc::closing() const
{
    return m_bClosing;
}

/**
 * Creates a diagram of the given type.
 *
 * @param folder       The folder in which tp create the diagram.
 * @param type         The type of diagram to create.
 * @param askForName   If true shows a dialog box asking for name,
 *                     else uses a default name.
 * @return Pointer to the UMLView of the new diagram.
 */
UMLView* UMLDoc::createDiagram(UMLFolder *folder, Uml::DiagramType type, bool askForName /*= true */)
{
    DEBUG(DBG_SRC) << "folder=" << folder->name() << " / type=" << type.toString();
    bool ok = true;
    QString name,
    dname = uniqueViewName(type);

    while (true) {
        if (askForName)  {
            name = KInputDialog::getText(i18nc("diagram name", "Name"), i18n("Enter name:"), dname, &ok, (QWidget*)UMLApp::app());
        } else {
            name = dname;
        }
        if (!ok)  {
            break;
        }
        if (name.length() == 0)  {
            KMessageBox::error(0, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
        } else if (!findView(type, name)) {
            UMLView* view = new UMLView(folder);
            view->umlScene()->setOptionState( Settings::optionState() );
            view->umlScene()->setName( name );
            view->umlScene()->setType( type );
            view->umlScene()->setID( UniqueID::gen() );
            addView(view);
            emit sigDiagramCreated( view->umlScene()->getID() );
            setModified(true);
            UMLApp::app()->enablePrint(true);
            changeCurrentView( view->umlScene()->getID() );
            return view;
        } else {
            KMessageBox::error(0, i18n("A diagram is already using that name."), i18n("Not a Unique Name"));
        }
    }//end while
    return 0;
}

/**
 * Used to rename a document. This method takes care of everything.
 * You just need to give the ID of the diagram to the method.
 *
 * @param id   The ID of the diagram to rename.
 */
void UMLDoc::renameDiagram(Uml::IDType id)
{
    bool ok = false;

    UMLView *view = findView(id);
    Uml::DiagramType type = view->umlScene()->type();

    QString oldName= view->umlScene()->name();
    while (true) {
        QString name = KInputDialog::getText(i18nc("renaming diagram", "Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)UMLApp::app());

        if (!ok) {
            break;
        }
        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
        }
        else if (!findView(type, name)) {
            view->umlScene()->setName(name);
            emit sigDiagramRenamed(id);
            setModified(true);
            break;
        }
        else {
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
    bool ok = false;
    QString oldName= o->name();
    while (true) {
        QString name = KInputDialog::getText(i18nc("renaming uml object", "Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)UMLApp::app());
        if (!ok)  {
            break;
        }
        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        }
        else if (isUnique(name)) {
            UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(o,name));
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
    bool ok = false;
    UMLClassifier* p = dynamic_cast<UMLClassifier *>(o->parent());
    if (!p) {
        DEBUG(DBG_SRC) << "Can not create object, no parent found.";
        return;
    }

    QString oldName= o->name();
    while (true) {
        QString name = KInputDialog::getText(i18nc("renaming child uml object", "Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)UMLApp::app());
        if (!ok) {
            break;
        }
        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        }
        else {
            if (p->findChildObject(name) == 0
                    || ((o->baseType() == UMLObject::ot_Operation) && KMessageBox::warningYesNo(0,
                            i18n( "The name you entered was not unique.\nIs this what you wanted?" ),
                            i18n( "Name Not Unique"),KGuiItem(i18n("Use Name")),KGuiItem(i18n("Enter New Name"))) == KMessageBox::Yes) ) {
                UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObject(o,name));
                setModified(true);
                break;
            } else {
                KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
            }
        }
    }
}

/**
 * Changes the current view (diagram) to the view with the given ID.
 *
 * @param id   The ID of the view to change to.
 */
void UMLDoc::changeCurrentView(Uml::IDType id)
{
    DEBUG(DBG_SRC) << "id=" << ID2STR(id);
    UMLApp* pApp = UMLApp::app();
    UMLView* view = findView(id);
    if (view) {
        view->umlScene()->setIsOpen(true);
        pApp->setCurrentView(view);
        emit sigDiagramChanged(view->umlScene()->type());
        pApp->setDiagramMenuItemsState( true );
        setModified(true);
        emit sigCurrentViewChanged();
        // when clicking on a tab, the documentation of diagram is upated in docwindow
        pApp->docWindow()->showDocumentation(view);
    }
    else {
        uWarning() << "New current view was not found with id=" << ID2STR(id) << "!";
    }
}

/**
 * Deletes a diagram from the current file.
 *
 * @param id   The ID of the diagram to delete.
 */
void UMLDoc::removeDiagram(Uml::IDType id)
{
    UMLApp::app()->docWindow()->updateDocumentation(true);
    UMLView* umlview = findView(id);
    if (!umlview) {
        uError() << "Request to remove diagram " << ID2STR(id) << ": Diagram not found!";
        return;
    }
    if (KMessageBox::warningContinueCancel(0, i18n("Are you sure you want to delete diagram %1?",
                                                   umlview->umlScene()->name()), i18n("Delete Diagram"),
                                           KGuiItem( i18n("&Delete"), "edit-delete")) == KMessageBox::Continue) {
        removeView(umlview);
        emit sigDiagramRemoved(id);
        setModified(true);
        /* if (infoWidget->isVisible()) {
               emit sigDiagramChanged(Uml::DiagramType::Undefined);
               UMLApp::app()->enablePrint(false);
           }
        */ //FIXME sort out all the KActions for when there's no diagram
        //also remove the buttons from the WorkToolBar, then get rid of infowidget
    }
}

/**
 * Return the currently selected root folder.
 * This will be an element from the m_root[] array.
 * @return the currently selected root folder or NULL
 */
UMLFolder *UMLDoc::currentRoot()
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
    while (f->umlPackage()) {
        f = static_cast<UMLFolder*>(f->umlPackage());
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
void UMLDoc::setCurrentRoot(Uml::ModelType rootType)
{
    m_pCurrentRoot = m_root[rootType];
}

/**
 * Removes an @ref UMLObject from the current file.  If this object
 * is being represented on a diagram it will also delete all those
 * representations.
 *
 * @param umlobject   Pointer to the UMLObject to delete.
 */
void UMLDoc::removeUMLObject(UMLObject* umlobject)
{
    UMLApp::app()->docWindow()->updateDocumentation(true);
    UMLObject::ObjectType type = umlobject->baseType();

    umlobject->setUMLStereotype(0);  // triggers possible cleanup of UMLStereotype
    if (dynamic_cast<UMLClassifierListItem*>(umlobject))  {
        UMLClassifier* parent = dynamic_cast<UMLClassifier*>(umlobject->parent());
        if (parent == 0) {
            uError() << "parent of umlobject is NULL";
            return;
        }
        if (type == UMLObject::ot_Operation) {
            parent->removeOperation(static_cast<UMLOperation*>(umlobject));
        } else if (type == UMLObject::ot_EnumLiteral) {
            UMLEnum *e = static_cast<UMLEnum*>(parent);
            e->removeEnumLiteral(static_cast<UMLEnumLiteral*>(umlobject));
        } else if (type == UMLObject::ot_EntityAttribute) {
            UMLEntity *ent = static_cast<UMLEntity*>(parent);
            ent->removeEntityAttribute(static_cast<UMLClassifierListItem*>(umlobject));
        } else if ( type == UMLObject::ot_UniqueConstraint || type == UMLObject::ot_ForeignKeyConstraint ||
                    type == UMLObject::ot_CheckConstraint) {
            UMLEntity* ent = static_cast<UMLEntity*>( parent );
            ent->removeConstraint( static_cast<UMLEntityConstraint*>( umlobject ) );
        } else {
            UMLClassifier* pClass = dynamic_cast<UMLClassifier*>(parent);
            if (pClass == 0)  {
                uError() << "parent of umlobject has unexpected type "
                         << parent->baseType();
                return;
            }
            if (type == UMLObject::ot_Attribute) {
                pClass->removeAttribute(static_cast<UMLAttribute*>(umlobject));
            } else if (type == UMLObject::ot_Template) {
                pClass->removeTemplate(static_cast<UMLTemplate*>(umlobject));
            } else {
                uError() << "umlobject has unexpected type " << type;
            }
        }
    } else {
        if (type == UMLObject::ot_Association) {
            UMLAssociation *a = (UMLAssociation *)umlobject;
            removeAssociation(a, false);  // don't call setModified here, it's done below
        } else {
            UMLPackage* pkg = umlobject->umlPackage();
            if (pkg) {
                pkg->removeObject(umlobject);
            } else {
                uError() << umlobject->name() << ": parent package is not set !";
            }
        }
        emit sigObjectRemoved(umlobject);
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
 * Return the m_modelID (currently this a fixed value:
 * Umbrello supports only a single document.)
 */
Uml::IDType UMLDoc::modelID() const
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
    QDomDocument doc;

    QDomProcessingInstruction xmlHeading =
        doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(xmlHeading);

    QDomElement root = doc.createElement( "XMI" );
    root.setAttribute( "xmi.version", "1.2" );
    QDateTime now = QDateTime::currentDateTime();
    root.setAttribute( "timestamp", now.toString(Qt::ISODate));
    root.setAttribute( "verified", "false");
    root.setAttribute( "xmlns:UML", "http://schema.omg.org/spec/UML/1.3");
    doc.appendChild( root );

    QDomElement header = doc.createElement( "XMI.header" );
    QDomElement meta = doc.createElement( "XMI.metamodel" );
    meta.setAttribute( "xmi.name", "UML" );
    meta.setAttribute( "xmi.version", "1.3" );
    meta.setAttribute( "href", "UML.xml" );
    header.appendChild( meta );

    /**
     * bugs.kde.org/56184 comment by M. Alanen 2004-12-19:
     * " XMI.model requires xmi.version. (or leave the whole XMI.model out,
     *   it's not required) "
    QDomElement model = doc.createElement( "XMI.model" );
    QFile* qfile = dynamic_cast<QFile*>(&file);
    if (qfile) {
        QString modelName = qfile->name();
        modelName = modelName.section('/', -1 );
        modelName = modelName.section('.', 0, 0);
        model.setAttribute( "xmi.name", modelName );
        model.setAttribute( "href", qfile->name() );
    }
     */

    QDomElement documentation = doc.createElement( "XMI.documentation" );

    // If we consider it useful we might add user and contact details
    // QDomElement owner = doc.createElement( "XMI.owner" );
    // owner.appendChild( doc.createTextNode( "Jens Kruger" ) ); // Add a User
    // documentation.appendChild( owner );

    // QDomElement contact = doc.createElement( "XMI.contact" );
    // contact.appendChild( doc.createTextNode( "je.krueger@web.de" ) );       // add a contact
    // documentation.appendChild( contact );

    QDomElement exporter = doc.createElement( "XMI.exporter" );
    exporter.appendChild( doc.createTextNode( "umbrello uml modeller http://uml.sf.net" ) );
    documentation.appendChild( exporter );

    QDomElement exporterVersion = doc.createElement( "XMI.exporterVersion" );
    exporterVersion.appendChild( doc.createTextNode( XMI_FILE_VERSION ) );
    documentation.appendChild( exporterVersion );

    // all files are now saved with correct Unicode encoding, we add this
    // information to the header, so that the file will be loaded correctly
    QDomElement exporterEncoding = doc.createElement( "XMI.exporterEncoding" );
    exporterEncoding.appendChild( doc.createTextNode( "UnicodeUTF8" ) );
    documentation.appendChild( exporterEncoding );

    header.appendChild( documentation );

    // See comment on <XMI.model> above
    // header.appendChild( model );
    header.appendChild( meta );
    root.appendChild( header );

    QDomElement content = doc.createElement( "XMI.content" );

    QDomElement contentNS = doc.createElement( "UML:Namespace.contents" );

    QDomElement objectsElement = doc.createElement( "UML:Model" );
    objectsElement.setAttribute( "xmi.id", ID2STR(m_modelID) );
    objectsElement.setAttribute( "name", m_Name );
    objectsElement.setAttribute( "isSpecification", "false" );
    objectsElement.setAttribute( "isAbstract", "false" );
    objectsElement.setAttribute( "isRoot", "false" );
    objectsElement.setAttribute( "isLeaf", "false" );

    QDomElement ownedNS = doc.createElement( "UML:Namespace.ownedElement" );

    // Save stereotypes and toplevel datatypes first so that upon loading
    // they are known first.
    // There is a bug causing duplication of the same stereotype in m_stereoList.
    // As a workaround, we use a string list to memorize which stereotype has been saved.
    QStringList stereoNames;
    foreach (UMLStereotype *s , m_stereoList) {
        QString stName = s->name();
        if (!stereoNames.contains(stName)) {
            s->saveToXMI(doc, ownedNS);
            stereoNames.append(stName);
        }
    }
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_root[i]->saveToXMI(doc, ownedNS);
    }

    objectsElement.appendChild( ownedNS );

    content.appendChild( objectsElement );

    root.appendChild( content );

    // Save the XMI extensions: docsettings, diagrams, listview, and codegeneration.
    QDomElement extensions = doc.createElement( "XMI.extensions" );
    extensions.setAttribute( "xmi.extender", "umbrello" );

    QDomElement docElement = doc.createElement( "docsettings" );
    Uml::IDType viewID = Uml::id_None;
    UMLView *currentView = UMLApp::app()->currentView();
    if (currentView) {
        viewID = currentView->umlScene()->getID();
    }
    docElement.setAttribute( "viewid", ID2STR(viewID) );
    docElement.setAttribute( "documentation", m_Doc );
    docElement.setAttribute( "uniqueid", ID2STR(UniqueID::get()) );
    extensions.appendChild( docElement );

    //  save listview
    UMLApp::app()->listView()->saveToXMI(doc, extensions);

    // save code generator
    CodeGenerator *codegen = UMLApp::app()->generator();
    if (codegen) {
        QDomElement codeGenElement = doc.createElement( "codegeneration" );
        codegen->saveToXMI( doc, codeGenElement );
        extensions.appendChild( codeGenElement );
    }

    root.appendChild( extensions );

    QTextStream stream( &file );
    stream.setCodec("UTF-8");
    stream << doc.toString();
}

/**
 * Checks the given XMI file if it was saved with correct Unicode
 * encoding set or not.
 *
 * @param file   The file to be checked.
 */
short UMLDoc::encoding(QIODevice & file)
{
    QTextStream stream( &file );
    stream.setCodec("UTF-8");
    QString data = stream.readAll();
    QString error;
    int line;
    QDomDocument doc;
    if ( !doc.setContent( data, false, &error, &line ) ) {
        uWarning() << "Can not set content: " << error << " Line: " << line;
        return ENC_UNKNOWN;
    }

    // we start at the beginning and go to the point in the header where we can
    // find out if the file was saved using Unicode
    QDomNode node = doc.firstChild();
    while (node.isComment() || node.isProcessingInstruction()) {
        node = node.nextSibling();
    }
    QDomElement root = node.toElement();
    if ( root.isNull() ) {
        return ENC_UNKNOWN;
    }
    //  make sure it is an XMI file
    if ( root.tagName() != "XMI" ) {
        return ENC_UNKNOWN;
    }
    node = node.firstChild();

    if ( node.isNull() ) {
        return ENC_UNKNOWN;
    }

    QDomElement element = node.toElement();
    // check header
    if ( element.isNull() || element.tagName() != "XMI.header" ) {
        return ENC_UNKNOWN;
    }

    QDomNode headerNode = node.firstChild();
    while ( !headerNode.isNull() ) {
        QDomElement headerElement = headerNode.toElement();
        // the information if Unicode was used is now stored in the
        // XMI.documentation section of the header
        if (headerElement.isNull() ||
                headerElement.tagName() != "XMI.documentation") {
            headerNode = headerNode.nextSibling();
            continue;
        }
        QDomNode docuNode = headerNode.firstChild();
        while ( !docuNode.isNull() ) {
            QDomElement docuElement = docuNode.toElement();
            // a tag XMI.exporterEncoding was added since version 1.2 to
            // mark a file as saved with Unicode
            if (! docuElement.isNull() &&
                    docuElement.tagName() == "XMI.exporterEncoding") {
                // at the moment this if isn't really necessary, but maybe
                // later we will have other encoding standards
                if (docuElement.text() == QString("UnicodeUTF8")) {
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
bool UMLDoc::loadFromXMI( QIODevice & file, short encode )
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
    QTextStream stream( &file );
    if (encode == ENC_UNICODE) {
        stream.setCodec("UTF-8");
    }

    QString data = stream.readAll();
    qApp->processEvents();  // give UI events a chance
    QString error;
    int line;
    QDomDocument doc;
    if ( !doc.setContent( data, false, &error, &line ) ) {
        uWarning() << "Can not set content:" << error << " Line:" << line;
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
    if ( root.isNull() ) {
        return false;
    }
    //  make sure it is an XMI file
    if( root.tagName() != "XMI" ) {
        return false;
    }

    m_nViewID = Uml::id_None;
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
        if (outerTag == "XMI.header") {
            QDomNode headerNode = node.firstChild();
            if ( !validateXMIHeader(headerNode) ) {
                return false;
            }
            recognized = true;
        } else if (outerTag == "XMI.extensions") {
            QDomNode extensionsNode = node.firstChild();
            while (! extensionsNode.isNull()) {
                loadExtensionsFromXMI(extensionsNode);
                extensionsNode = extensionsNode.nextSibling();
            }
            recognized = true;
        }
        if (outerTag != "XMI.content" ) {
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
            if (tag == "umlobjects"  // for bkwd compat.
                    || tagEq(tag, "Subsystem")
                    || tagEq(tag, "Model") ) {
                if( !loadUMLObjectsFromXMI( element ) ) {
                    uWarning() << "failed load on objects";
                    return false;
                }
                m_Name = element.attribute( "name", i18n("UML Model") );
                UMLListView *lv = UMLApp::app()->listView();
                lv->setTitle(0, m_Name);
                seen_UMLObjects = true;
            } else if (tagEq(tag, "Package") ||
                       tagEq(tag, "Class") ||
                       tagEq(tag, "Interface")) {
                // These tests are only for foreign XMI files that
                // are missing the <Model> tag (e.g. NSUML)
                QDomElement parentElem = node.toElement();
                if( !loadUMLObjectsFromXMI( parentElem ) ) {
                    uWarning() << "failed load on model objects";
                    return false;
                }
                seen_UMLObjects = true;
            } else if (tagEq(tag, "TaggedValue")) {
                // This tag is produced here, i.e. outside of <UML:Model>,
                // by the Unisys.JCR.1 Rose-to-XMI tool.
                if (! seen_UMLObjects) {
                    DEBUG(DBG_SRC) << "skipping TaggedValue because not seen_UMLObjects";
                    continue;
                }
                tag = element.attribute("tag", "");
                if (tag != "documentation") {
                    continue;
                }
                QString modelElement = element.attribute("modelElement", "");
                if (modelElement.isEmpty()) {
                    DEBUG(DBG_SRC) << "skipping TaggedValue(documentation) because "
                                   << "modelElement.isEmpty()";
                    continue;
                }
                UMLObject *o = findObjectById(STR2ID(modelElement));
                if (o == 0) {
                    DEBUG(DBG_SRC) << "TaggedValue(documentation): cannot find object"
                                   << " for modelElement " << modelElement;
                    continue;
                }
                QString value = element.attribute("value", "");
                if (! value.isEmpty()) {
                    o->setDoc(value);
                }
            } else {
                // for backward compatibility
                loadExtensionsFromXMI(child);
            }
        }
    }
#ifdef VERBOSE_DEBUGGING
    DEBUG(DBG_SRC) << "m_objectList.count() is " << m_objectList.count();
#endif
    resolveTypes();
    // set a default code generator if no <XMI.extensions><codegeneration> tag seen
    if (UMLApp::app()->generator() == 0) {
        UMLApp::app()->setGenerator(UMLApp::app()->defaultLanguage());
    }
    emit sigWriteToStatusBar( i18n("Setting up the document...") );
    qApp->processEvents();  // give UI events a chance
    activateAllViews();

    UMLView *viewToBeSet = 0;
    if (m_nViewID != Uml::id_None) {
        viewToBeSet = findView( m_nViewID );
    }
    if (viewToBeSet) {
        changeCurrentView( m_nViewID );
    } else {
        createDiagram(m_root[Uml::ModelType::Logical], Uml::DiagramType::Class, false);
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
    m_bTypesAreResolved = true;
    writeToStatusBar( i18n("Resolving object references...") );
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
       UMLFolder *obj = m_root[i];
#ifdef VERBOSE_DEBUGGING
        DEBUG(DBG_SRC) << "UMLDoc: invoking resolveRef() for " << obj->getName()
                       << " (id=" << ID2STR(obj->getID()) << ")";
#endif
        obj->resolveRef();
    }
    qApp->processEvents();  // give UI events a chance
}

/**
 * Ensures the XMI file is a valid UML file.
 * Currently only checks for metamodel=UML.
 *
 * @param headerNode   The <XMI.header> node
 */
bool UMLDoc::validateXMIHeader(QDomNode& headerNode)
{
    QDomElement headerElement = headerNode.toElement();
    while ( !headerNode.isNull() ) {
        /*  //Seems older Umbrello files used a different metamodel, so don't validate it for now
          if( !headerElement.isNull() && headerElement.tagName() == "XMI.metamodel" ) {
              String metamodel = headerElement.attribute("xmi.name", "");
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
    emit sigSetStatusbarProgress( 0 );
    emit sigSetStatusbarProgressSteps( 10 );
    m_count = 0;
     */
    emit sigWriteToStatusBar( i18n("Loading UML elements...") );

    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment()) {
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (tagEq(type, "Model")) {
            bool foundUmbrelloRootFolder = false;
            QString name = tempElement.attribute("name");
            for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
                if (name == m_root[i]->name()) {
                    m_pCurrentRoot = m_root[i];
                    m_root[i]->loadFromXMI(tempElement);
                    foundUmbrelloRootFolder = true;
                    break;
                }
            }
            if (foundUmbrelloRootFolder) {
                continue;
            }
        }
        // From here on, it's support for stereotypes, pre 1.5.5 versions, and foreign files
        if (tagEq(type, "Namespace.ownedElement") ||
                tagEq(type, "Namespace.contents") ||
                tagEq(type, "Model")) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore the <UML:Namespace.ownedElement> tag is of no
            // significance.
            if ( !loadUMLObjectsFromXMI( tempElement ) ) {
                uWarning() << "failed load on " << type;
                return false;
            }
            continue;
        }
        if (Model_Utils::isCommonXMIAttribute(type)) {
            continue;
        }
        if (! tempElement.hasAttribute("xmi.id")) {
            QString idref = tempElement.attribute("xmi.idref", "");
            if (! idref.isEmpty()) {
                DEBUG(DBG_SRC) << "resolution of xmi.idref " << idref
                               << " is not yet implemented";
            } else {
                uError() << "Cannot load " << type
                         << " because xmi.id is missing";
            }
            continue;
        }
        QString stID = tempElement.attribute("stereotype", "");
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type, stID);
        if ( !pObject ) {
            uWarning() << "Unknown type of umlobject to create: " << type;
            // We want a best effort, therefore this is handled as a
            // soft error.
            continue;
        }
        UMLObject::ObjectType ot = pObject->baseType();
        // Set the parent root folder.
        UMLPackage *pkg = 0;
        if (ot == UMLObject::ot_Datatype) {
            pkg = m_datatypeRoot;
        } else {
            Uml::ModelType guess = Model_Utils::guessContainer(pObject);
            if (guess != Uml::ModelType::N_MODELTYPES) {
                pkg = m_root[guess];
            }
            else {
                uError() << "Guess is Uml::ModelType::N_MODELTYPES - package not set correctly for "
                         << pObject->name() << " / base type " << pObject->baseTypeStr();
                pkg = m_root[Uml::ModelType::Logical];
            }
        }
        pObject->setUMLPackage(pkg);

        bool status = pObject->loadFromXMI( tempElement );
        if ( !status ) {
            delete pObject;
            return false;
        }
        pkg = pObject->umlPackage();
        if (ot == UMLObject::ot_Stereotype) {
            UMLStereotype *s = static_cast<UMLStereotype*>(pObject);
            UMLStereotype *exist = findStereotype(pObject->name());
            if (exist) {
                if (exist->id() == pObject->id()) {
                    delete pObject;
                } else {
                    DEBUG(DBG_SRC) << "Stereotype " << pObject->name()
                                   << "(id=" << ID2STR(pObject->id())
                                   << ") already exists with id="
                                   << ID2STR(exist->id());
                    addStereotype(s);
                }
            } else {
                addStereotype(s);
            }
            continue;
        }
        if (pkg) {
            pkg->addObject(pObject);
        }
        else {
            uError() << "Package is NULL for " << pObject->name();
            return false;
        }

        /* FIXME see comment at loadUMLObjectsFromXMI
        emit sigSetStatusbarProgress( ++m_count );
         */
    }
    return true;
}

/**
 * Sets m_nViewID.
 */
void UMLDoc::setMainViewID(Uml::IDType viewID)
{
    m_nViewID = viewID;
}

/**
 * Loads umbrello specific extensions from XMI to the UMLDoc.
 * The extension tags are: "docsettings", "diagrams", "listview",
 * and "codegeneration".
 */
void UMLDoc::loadExtensionsFromXMI(QDomNode& node)
{
    QDomElement element = node.toElement();
    QString tag = element.tagName();

    if (tag == "docsettings") {
        QString viewID = element.attribute( "viewid", "-1" );
        m_Doc = element.attribute( "documentation", "" );
        QString uniqueid = element.attribute( "uniqueid", "0" );

        m_nViewID = STR2ID(viewID);
        UniqueID::set(STR2ID(uniqueid));
        UMLApp::app()->docWindow()->newDocumentation();

    } else if (tag == "diagrams" || tag == "UISModelElement") {
        // For backward compatibility only:
        // Since version 1.5.5 diagrams are saved as part of the UMLFolder.
        QDomNode diagramNode = node.firstChild();
        if (tag == "UISModelElement") {          // Unisys.IntegratePlus.2
            element = diagramNode.toElement();
            tag = element.tagName();
            if (tag != "uisOwnedDiagram") {
                uError() << "unknown child node " << tag;
                return;
            }
            diagramNode = diagramNode.firstChild();
        }
        if ( !loadDiagramsFromXMI( diagramNode ) ) {
            uWarning() << "failed load on diagrams";
        }

    } else if (tag == "listview") {
        //FIXME: Need to resolveTypes() before loading listview,
        //       else listview items are duplicated.
        resolveTypes();
        if ( !UMLApp::app()->listView()->loadFromXMI( element ) ) {
            uWarning() << "failed load on listview";
        }

    } else if (tag == "codegeneration") {
        QDomNode cgnode = node.firstChild();
        QDomElement cgelement = cgnode.toElement();
        while ( !cgelement.isNull() ) {
            QString nodeName = cgelement.tagName();
            QString lang = cgelement.attribute("language","UNKNOWN");
            Uml::ProgrammingLanguage pl = Uml::ProgrammingLanguage::fromString(lang);
            CodeGenerator *g = UMLApp::app()->setGenerator(pl);
            g->loadFromXMI(cgelement);
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
bool UMLDoc::loadDiagramsFromXMI( QDomNode & node )
{
    emit sigWriteToStatusBar( i18n("Loading diagrams...") );
    emit sigResetStatusbarProgress();
    emit sigSetStatusbarProgress( 0 );
    emit sigSetStatusbarProgressSteps( 10 ); //FIX ME
    QDomElement element = node.toElement();
    if ( element.isNull() ) {
        return true;  //return ok as it means there is no umlobjects
    }
    const Settings::OptionState state = Settings::optionState();
    UMLView * pView = 0;
    int count = 0;
    while ( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "diagram" || tag == "UISDiagram") {
            pView = new UMLView(0);
            // IMPORTANT: Set OptionState of new UMLView _BEFORE_
            // reading the corresponding diagram:
            // + allow using per-diagram color and line-width settings
            // + avoid crashes due to uninitialized values for lineWidth
            pView->umlScene()->setOptionState( state );
            bool success = false;
            if (tag == "UISDiagram") {
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
            Uml::ModelType mt = Model_Utils::convert_DT_MT(pView->umlScene()->type());
            pView->umlScene()->setFolder(m_root[mt]);
            pView->hide();
            addView( pView );
            emit sigSetStatusbarProgress( ++count );
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
 * Returns a list of the packages in this UMLDoc,
 *
 * @return List of UMLPackages.
 */
UMLPackageList UMLDoc::packages(bool includeNested /* = true */)
{
    UMLPackageList packageList;
    m_root[Uml::ModelType::Logical]->appendPackages(packageList, includeNested);
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
UMLClassifierList UMLDoc::concepts(bool includeNested /* =true */)
{
    UMLClassifierList conceptList;
    m_root[Uml::ModelType::Logical]->appendClassifiers(conceptList, includeNested);
    return conceptList;
}

/**
 * Returns a list of the classes in this UMLDoc.
 *
 * @param includeNested   Whether to include the concepts from
 *                        nested packages (default: true.)
 * @return  List of UML classes.
 */
UMLClassifierList UMLDoc::classes(bool includeNested /* =true */)
{
    UMLClassifierList conceptList;
    m_root[Uml::ModelType::Logical]->appendClasses(conceptList, includeNested);
    return conceptList;
}

/**
 * Returns a list of the classes and interfaces in this UMLDoc.
 *
 * @param includeNested   Whether to include the concepts from
 *                        nested packages (default: true.)
 * @return  List of UML concepts.
 */
UMLClassifierList UMLDoc::classesAndInterfaces(bool includeNested /* =true */)
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
UMLEntityList UMLDoc::entities( bool includeNested /* =true */ )
{
    UMLEntityList entityList;
    m_root[Uml::ModelType::EntityRelationship]->appendEntities(entityList, includeNested);
    return entityList;
}

/**
 * Returns a list of the interfaces in this UMLDoc.
 *
 * @param includeNested   Whether to include the concepts from
 *                        nested packages (default: true.)
 * @return  List of UML interfaces.
 */
UMLClassifierList UMLDoc::interfaces(bool includeNested /* =true */)
{
    UMLClassifierList interfaceList;
    m_root[Uml::ModelType::Logical]->appendInterfaces(interfaceList, includeNested);
    return interfaceList;
}

/**
 * Returns a list of the datatypes in this UMLDoc.
 *
 * @return  List of datatypes.
 */
UMLClassifierList UMLDoc::datatypes()
{
    UMLObjectList objects = m_datatypeRoot->containedObjects();
    UMLClassifierList datatypeList;
    foreach (UMLObject *obj , objects) {
        if (obj->baseType() == UMLObject::ot_Datatype) {
            datatypeList.append(static_cast<UMLClassifier*>(obj));
        }
    }
    return datatypeList;
}

/**
 * Returns a list of the associations in this UMLDoc.
 *
 * @return  List of UML associations.
 */
UMLAssociationList UMLDoc::associations()
{
    UMLAssociationList associationList;
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        UMLAssociationList assocs = m_root[i]->getAssociations();

        foreach (UMLAssociation* a, assocs ) {
            associationList.append(a);
        }
    }
    return associationList;
}

/**
 * Controls the printing of the program.
 *
 * @param pPrinter  The printer (object) to use.
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
        Uml::IDType id = STR2ID(sID);
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
UMLViewList UMLDoc::viewIterator()
{
    UMLViewList accumulator;
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        m_root[i]->appendViews(accumulator, true);
    }
    return accumulator;
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
bool UMLDoc::isModified()
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
    Uml::IDType result = assignNewID(obj->id());
    obj->setID(result);

    //If it is a CONCEPT then change the ids of all its operations and attributes
    if (obj->baseType() == UMLObject::ot_Class ) {
        UMLClassifier *c = static_cast<UMLClassifier*>(obj);
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

    if (obj->baseType() == UMLObject::ot_Interface || obj->baseType() == UMLObject::ot_Class ) {
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
UMLFolder *UMLDoc::rootFolder(Uml::ModelType mt)
{
    if (mt < Uml::ModelType::Logical || mt >= Uml::ModelType::N_MODELTYPES) {
        uError() << "illegal input value " << mt;
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
Uml::ModelType UMLDoc::rootFolderType(UMLObject *obj)
{
    for (int i = 0; i < Uml::ModelType::N_MODELTYPES; ++i) {
        const Uml::ModelType m = Uml::ModelType(Uml::ModelType::Value(i));
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
IDChangeLog* UMLDoc::changeLog()
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
Uml::IDType UMLDoc::assignNewID(Uml::IDType oldID)
{
    Uml::IDType result = UniqueID::gen();
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
bool UMLDoc::addUMLView(UMLView * pView )
{
    if (!pView || !m_pChangeLog) {
        return false;
    }

    int i = 0;
    QString viewName = pView->umlScene()->name();
    QString name = viewName;
    while ( findView(pView->umlScene()->type(), name) != 0) {
        name = viewName + '_' + QString::number(++i);
    }
    if (i) { //If name was modified
        pView->umlScene()->setName(name);
    }
    Uml::IDType result = assignNewID(pView->umlScene()->getID());
    pView->umlScene()->setID(result);

    pView->umlScene()->activateAfterLoad( true );
    pView->umlScene()->endPartialWidgetPaste();
    pView->umlScene()->setOptionState( Settings::optionState() );
    addView(pView);
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
void UMLDoc::settingsChanged(Settings::OptionState optionState)
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
    if ( m_pAutoSaveTimer ) {
        m_pAutoSaveTimer->stop();
        disconnect(m_pAutoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
        delete m_pAutoSaveTimer;
        m_pAutoSaveTimer = 0;
    }
    Settings::OptionState optionState = Settings::optionState();
    if ( optionState.generalState.autosave ) {
        m_pAutoSaveTimer = new QTimer(this);
        connect( m_pAutoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
        m_pAutoSaveTimer->setSingleShot( false );
        m_pAutoSaveTimer->start( optionState.generalState.autosavetime * 60000 );
    }
    return;
}

/**
 * Called after a specified time to autosave the document.
 */
void UMLDoc::slotAutoSave()
{
    //Only save if modified.
    if ( !m_modified ) {
        return;
    }
    KUrl tempUrl = m_doc_url;
    if ( tempUrl.fileName() == i18n("Untitled") ) {
        tempUrl.setPath( QDir::homePath() + i18n( "/autosave%1", QString(".xmi") ) );
        saveDocument( tempUrl );
        m_doc_url.setFileName( i18n("Untitled") );
        m_modified = true;
        UMLApp::app()->setModified( m_modified );
    } else {
        // 2004-05-17 Achim Spangler
        KUrl orgDocUrl = m_doc_url;
        QString orgFileName = m_doc_url.fileName();
        // don't overwrite manually saved file with autosave content
        QString fileName = tempUrl.fileName();
        Settings::OptionState optionState = Settings::optionState();
        fileName.replace( ".xmi", optionState.generalState.autosavesuffix );
        tempUrl.setFileName( fileName );
        // End Achim Spangler

        saveDocument( tempUrl );
        // 2004-05-17 Achim Spangler
        // re-activate m_modified if autosave is writing to other file
        // than the main project file->autosave-suffix != ".xmi"
        if ( ".xmi" != optionState.generalState.autosavesuffix ) {
            m_modified = true;
            UMLApp::app()->setModified( m_modified );
        }
        // restore original file name -
        // UMLDoc::saveDocument() sets doc_url to filename which is given as autosave-filename
        setUrl( orgDocUrl );
        UMLApp * pApp = UMLApp::app();
        pApp->setCaption(orgFileName, isModified() );
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
            UMLApp::app()->tabWidget()->setTabText( UMLApp::app()->tabWidget()->indexOf(view), view->umlScene()->name() );
        }
        emit sigDiagramRenamed( view->umlScene()->getID() );
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
        DEBUG(DBG_SRC) << "CodeGenerator is still NULL";
        return;
    }
    QStringList entries = cg->defaultDatatypes();
    QStringList::Iterator end(entries.end());
    for (QStringList::Iterator it = entries.begin(); it != end; ++it) {
        createDatatype(*it);
    }
}

/**
 * Add a datatype if it doesn't already exist.
 * Used by code generators and attribute dialog.
 */
void UMLDoc::createDatatype(const QString &name)
{
    UMLObjectList datatypes = m_datatypeRoot->containedObjects();
    UMLObject* umlobject = Model_Utils::findUMLObject(datatypes, name,
                                                      UMLObject::ot_Datatype, m_datatypeRoot);
    if (!umlobject) {
        Object_Factory::createUMLObject(UMLObject::ot_Datatype, name, m_datatypeRoot);
    }
    UMLApp::app()->listView()->closeDatatypesFolder();
}

/**
 * Make a popup menu for the tabs
 * signalled from tabWidget's contextMenu().
 */
void UMLDoc::slotDiagramPopupMenu(QWidget* umlview, const QPoint& point)
{
    UMLView* view = (UMLView*) umlview;
    if (m_pTabPopupMenu != 0) {
        m_pTabPopupMenu->hide();
        delete m_pTabPopupMenu;
        m_pTabPopupMenu = 0;
    }

    UMLListViewItem::ListViewType type = UMLListViewItem::lvt_Unknown;
    switch ( view->umlScene()->type() ) {
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
        uWarning() << "unknown diagram type " << view->type();
        return;
    }//end switch

    m_pTabPopupMenu = new ListPopupMenu(UMLApp::app()->mainViewWidget(), type, 0);
    m_pTabPopupMenu->popup(point);
    connect(m_pTabPopupMenu, SIGNAL(triggered(QAction*)), view->umlScene(), SLOT(slotMenuSelection(QAction*)));
}

/**
 * Find and return the user selected type of the popup menu.
 * See also m_pTabPopupMenu and slotDiagramPopupMenu.
 * @param action  the selected action
 * @return the selected menu type
 */
ListPopupMenu::MenuType UMLDoc::popupMenuSelection(QAction* action)
{
    if (m_pTabPopupMenu) {
        return m_pTabPopupMenu->getMenuType(action);
    }
    else {
        return ListPopupMenu::mt_Undefined;
    }
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
 * Function for comparing tags in XMI files.
 */
bool UMLDoc::tagEq (const QString& inTag, const QString& inPattern)
{
    QString tag = inTag;
    QString pattern = inPattern;
    tag.remove( QRegExp("^\\w+:") );  // remove leading "UML:" or other
    int patSections = pattern.count('.') + 1;
    QString tagEnd = tag.section('.', -patSections);
    return (tagEnd.toLower() == pattern.toLower());
}

#include "umldoc.moc"
