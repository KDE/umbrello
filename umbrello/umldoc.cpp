/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2007                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "umldoc.h"

// qt includes
#include <qpainter.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qbuffer.h>
#include <qdir.h>
#include <qregexp.h>
#include <qlabel.h>

// kde includes
#include <kapplication.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kprinter.h>
#include <ktar.h>
#include <ktempdir.h>
#include <ktempfile.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <ktabwidget.h>

// app includes
#include "uniqueid.h"
#include "associationwidget.h"
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
#include "entityattribute.h"
#include "stereotype.h"
#include "classifierlistitem.h"
#include "object_factory.h"
#include "import_rose.h"
#include "model_utils.h"
#include "widget_utils.h"
#include "uml.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlview.h"
#include "clipboard/idchangelog.h"
#include "dialogs/classpropdlg.h"
#include "codegenerators/codegenfactory.h"
#include "listpopupmenu.h"
#include "version.h"

#define XMI_FILE_VERSION UMBRELLO_VERSION
// For the moment, the XMI_FILE_VERSION changes with each UMBRELLO_VERSION.
// But someday that may stabilize ;)

using namespace Uml;

static const uint undoMax = 30;

UMLDoc::UMLDoc() {
    m_Name = i18n("UML Model");
    m_modelID = "m1";
    m_count = 0;
    m_pChangeLog = 0;
    m_Doc = "";
    m_modified = false;
    m_bLoading = false;
    m_bTypesAreResolved = false;
    m_pAutoSaveTimer = 0;
    m_nViewID = Uml::id_None;
    m_pTabPopupMenu = 0;
    m_pCurrentRoot = NULL;
}

void UMLDoc::init() {
    // Initialize predefined folders.
    const QString nativeRootName[Uml::N_MODELTYPES] = {
        "Logical View",
        "Use Case View",
        "Component View",
        "Deployment View",
        "Entity Relationship Model"
    };
    const QString localizedRootName[Uml::N_MODELTYPES] = {
        i18n("Logical View"),
        i18n("Use Case View"),
        i18n("Component View"),
        i18n("Deployment View"),
        i18n("Entity Relationship Model")
    };
    for (int i = 0; i < Uml::N_MODELTYPES; i++) {
        m_root[i] = new UMLFolder(nativeRootName[i], STR2ID(nativeRootName[i]));
        m_root[i]->setLocalName(localizedRootName[i]);
    }
    m_datatypeRoot = new UMLFolder("Datatypes", "Datatypes");
    m_datatypeRoot->setLocalName(i18n("Datatypes"));
    m_datatypeRoot->setUMLPackage(m_root[Uml::mt_Logical]);
    m_root[Uml::mt_Logical]->addObject(m_datatypeRoot);

    // Connect signals.
    UMLApp * pApp = UMLApp::app();
    connect(this, SIGNAL(sigDiagramCreated(Uml::IDType)), pApp, SLOT(slotUpdateViews()));
    connect(this, SIGNAL(sigDiagramRemoved(Uml::IDType)), pApp, SLOT(slotUpdateViews()));
    connect(this, SIGNAL(sigDiagramRenamed(Uml::IDType)), pApp, SLOT(slotUpdateViews()));
    connect(this, SIGNAL( sigCurrentViewChanged() ), pApp, SLOT( slotCurrentViewChanged() ) );
}

UMLDoc::~UMLDoc() {
    delete m_pChangeLog;
    m_pChangeLog = 0;
}

void UMLDoc::addView(UMLView *view) {
    if (view == NULL) {
        kError() << "UMLDoc::addView: argument is NULL" << endl;
        return;
    }
    UMLFolder *f = view->getFolder();
    if (f == NULL) {
        kError() << "UMLDoc::addView: view folder is not set" << endl;
        return;
    }
    f->addView(view);

    UMLApp * pApp = UMLApp::app();
    if ( pApp->getListView() )
        connect(this, SIGNAL(sigObjectRemoved(UMLObject *)), view, SLOT(slotObjectRemoved(UMLObject *)));

    pApp->setCurrentView(view);
    if ( ! m_bLoading ) {
        view -> show();
        emit sigDiagramChanged(view ->getType());
    }

    Settings::OptionState optionState = Settings::getOptionState();
    KTabWidget* tabWidget = NULL;
    if (optionState.generalState.tabdiagrams) {
        tabWidget = UMLApp::app()->tabWidget();
        tabWidget->addTab(view, view->getName());
        tabWidget->setTabIconSet(view, Widget_Utils::iconSet(view->getType()));
    }
    pApp->setDiagramMenuItemsState(true);
    pApp->slotUpdateViews();
    pApp->setCurrentView(view);
    if (tabWidget) {
        tabWidget->showPage(view);
        tabWidget->setCurrentPage(tabWidget->currentPageIndex());
    }
}

void UMLDoc::removeView(UMLView *view , bool enforceCurrentView ) {
    if(!view)
    {
        kError()<<"UMLDoc::removeView(UMLView *view) called with view = 0"<<endl;
        return;
    }
    if ( UMLApp::app()->getListView() ) {
        disconnect(this,SIGNAL(sigObjectRemoved(UMLObject *)), view,SLOT(slotObjectRemoved(UMLObject *)));
    }
    view->hide();
    //remove all widgets before deleting view
    view->removeAllWidgets();
    UMLFolder *f = view->getFolder();
    if (f == NULL) {
        kError() << "UMLDoc::removeView(" << view->getName()
            << "): view->getFolder() returns NULL" << endl;
        return;
    }
    f->removeView(view);
    UMLView *currentView = UMLApp::app()->getCurrentView();
    if (currentView == view)
    {
        UMLApp::app()->setCurrentView(NULL);
        UMLViewList viewList;
        m_root[mt_Logical]->appendViews(viewList);
        UMLView* firstView = viewList.first();
        if (!firstView && enforceCurrentView) //create a diagram
        {
            createDiagram(m_root[mt_Logical], dt_Class, false);
            kapp->processEvents();
            m_root[mt_Logical]->appendViews(viewList);
            firstView = viewList.first();
        }

        if ( firstView )
        {
            changeCurrentView( firstView->getID() );
            UMLApp::app()->setDiagramMenuItemsState(true);
        }
    }
}

void UMLDoc::setURL(const KURL &url) {
    m_doc_url = url;
    return;
}

const KURL& UMLDoc::URL() const {
    return m_doc_url;
}

bool UMLDoc::saveModified() {
    bool completed(true);
    if (!m_modified)
        return completed;

    UMLApp *win = UMLApp::app();
    int want_save = KMessageBox::warningYesNoCancel(win, i18n("The current file has been modified.\nDo you want to save it?"), i18n("Warning"),KStdGuiItem::save(),KStdGuiItem::discard());
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
            saveDocument(URL());
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

void UMLDoc::closeDocument() {
    UMLApp::app()->setGenerator(Uml::pl_Reserved);  // delete the codegen
    m_Doc = "";
    DocWindow* dw = UMLApp::app()->getDocWindow();
    if (dw) {
        dw->newDocumentation();
    }

    UMLListView *listView = UMLApp::app()->getListView();
    if (listView) {
        listView->init();
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
        UMLObject *obj;
        for (int i = 0; i < Uml::N_MODELTYPES; i++)
            m_root[i]->removeAllObjects();
        // Restore the datatype folder, it has been deleted above.
        m_datatypeRoot = new UMLFolder("Datatypes", "Datatypes");
        m_datatypeRoot->setLocalName(i18n("Datatypes"));
        m_datatypeRoot->setUMLPackage(m_root[Uml::mt_Logical]);
        m_root[Uml::mt_Logical]->addObject(m_datatypeRoot);
        listView->theDatatypeFolder()->setUMLObject(m_datatypeRoot);
        /* Remove any stereotypes.
        if (m_stereoList.count() > 0) {
            UMLStereotype *s;
            for (UMLStereotypeListIt sit(m_stereoList); (s = sit.current()) != 0; ++sit)
                delete s;
            m_stereoList.clear();
        }
         */
    }
    m_bTypesAreResolved = false;
}

bool UMLDoc::newDocument() {
    closeDocument();
    UMLApp::app()->setCurrentView(NULL);
    m_doc_url.setFileName(i18n("Untitled"));
    //see if we need to start with a new diagram
    Settings::OptionState optionState = Settings::getOptionState();
    Uml::Diagram_Type dt = optionState.generalState.diagram;
    Uml::Model_Type mt = Model_Utils::convert_DT_MT(dt);
    if (mt == Uml::N_MODELTYPES) {  // don't allow no diagram
        dt = Uml::dt_Class;
        mt = Uml::mt_Logical;
    }
    createDiagram(m_root[mt], dt, false);

    UMLApp::app()->initGenerator();
    addDefaultDatatypes();
    addDefaultStereotypes();

    setModified(false);
    initSaveTimer();

    UMLApp::app()->enableUndo(false);
    clearUndoStack();
    addToUndoStack();

    return true;
}

bool UMLDoc::openDocument(const KURL& url, const char* /*format =0*/) {
    if(url.fileName().length() == 0) {
        newDocument();
        return false;
    }

    m_doc_url = url;
    QDir d = url.path(1);
    closeDocument();
    // IMPORTANT: set m_bLoading to true
    // _AFTER_ the call of UMLDoc::closeDocument()
    // as it sets m_bLoading to false afer it was temporarily
    // changed to true to block recording of changes in redo-buffer
    m_bLoading = true;
    QString tmpfile;
    KIO::NetAccess::download( url, tmpfile, UMLApp::app() );
    QFile file( tmpfile );
    if ( !file.exists() ) {
        KMessageBox::error(0, i18n("The file %1 does not exist.").arg(d.path()), i18n("Load Error"));
        m_doc_url.setFileName(i18n("Untitled"));
        m_bLoading = false;
        newDocument();
        return false;
    }

    // status of XMI loading
    bool status = false;

    // check if the xmi file is a compressed archive like tar.bzip2 or tar.gz
    QString filetype = m_doc_url.fileName(true);
    QString mimetype = "";
    if (filetype.find(QRegExp("\\.tgz$")) != -1)
    {
        mimetype = "application/x-gzip";
    } else if (filetype.find(QRegExp("\\.tar.bz2$")) != -1) {
        mimetype = "application/x-bzip2";
    }

    if (mimetype.isEmpty() == false)
    {
        KTar archive(tmpfile, mimetype);
        if (archive.open(IO_ReadOnly) == false)
        {
            KMessageBox::error(0, i18n("The file %1 seems to be corrupted.").arg(d.path()), i18n("Load Error"));
            m_doc_url.setFileName(i18n("Untitled"));
            m_bLoading = false;
            newDocument();
            return false;
        }

        // get the root directory and all entries in
        const KArchiveDirectory * rootDir = archive.directory();
        QStringList entries = rootDir->entries();
        QString entryMimeType;
        bool foundXMI = false;
        QStringList::Iterator it;
        QStringList::Iterator end(entries.end());

        // now go through all entries till we find an xmi file
        for (it = entries.begin(); it != end; ++it)
        {
            // only check files, we do not go in subdirectories
            if (rootDir->entry(*it)->isFile() == true)
            {
                // we found a file, check the mimetype
                entryMimeType = KMimeType::findByPath(*it, 0, true)->name();
                if (entryMimeType == "application/x-uml")
                {
                    foundXMI = true;
                    break;
                }
            }
        }

        // if we found an XMI file, we have to extract it to a temporary file
        if (foundXMI == true)
        {
            KTempDir tmp_dir;
            KArchiveEntry * entry;
            KArchiveFile * fileEntry;

            // try to cast the file entry in the archive to an archive entry
            entry = const_cast<KArchiveEntry*>(rootDir->entry(*it));
            if (entry == 0)
            {
                KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.").arg(d.path()), i18n("Load Error"));
                m_doc_url.setFileName(i18n("Untitled"));
                m_bLoading = false;
                newDocument();
                return false;
            }

            // now try to cast the archive entry to a file entry, so that we can
            // extract the file
            fileEntry = dynamic_cast<KArchiveFile*>(entry);
            if (fileEntry == 0)
            {
                KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.").arg(d.path()), i18n("Load Error"));
                m_doc_url.setFileName(i18n("Untitled"));
                m_bLoading = false;
                newDocument();
                return false;
            }

            // now we can extract the file to the temporary directory
            fileEntry->copyTo(tmp_dir.name());

            // now open the extracted file for reading
            QFile xmi_file(tmp_dir.name() + *it);
            if( !xmi_file.open( IO_ReadOnly ) )
            {
                KMessageBox::error(0, i18n("There was a problem loading the extracted file: %1").arg(d.path()), i18n("Load Error"));
                m_doc_url.setFileName(i18n("Untitled"));
                m_bLoading = false;
                newDocument();
                return false;
            }
            status = loadFromXMI( xmi_file, ENC_UNKNOWN );

            // close the extracted file and the temporary directory
            xmi_file.close();
            tmp_dir.unlink();

        } else {
            KMessageBox::error(0, i18n("There was no XMI file found in the compressed file %1.").arg(d.path()), i18n("Load Error"));
            m_doc_url.setFileName(i18n("Untitled"));
            m_bLoading = false;
            newDocument();
            return false;
        }

        archive.close();
    } else {
        // no, it seems to be an ordinary file
        if( !file.open( IO_ReadOnly ) ) {
            KMessageBox::error(0, i18n("There was a problem loading file: %1").arg(d.path()), i18n("Load Error"));
            m_doc_url.setFileName(i18n("Untitled"));
            m_bLoading = false;
            newDocument();
            return false;
        }
        if (filetype.endsWith(".mdl"))
            status = Import_Rose::loadFromMDL(file);
        else
            status = loadFromXMI( file, ENC_UNKNOWN );
    }

    file.close();
    KIO::NetAccess::removeTempFile( tmpfile );
    if( !status )
    {
        KMessageBox::error(0, i18n("There was a problem loading file: %1").arg(d.path()), i18n("Load Error"));
        m_bLoading = false;
        newDocument();
        return false;
    }
    setModified(false);
    m_bLoading = false;
    initSaveTimer();

    UMLApp::app()->enableUndo(false);
    clearUndoStack();
    addToUndoStack();
    // for compatibility
    addDefaultStereotypes();

    return true;
}

bool UMLDoc::saveDocument(const KURL& url, const char * /* format */) {
    m_doc_url = url;
    QDir d = m_doc_url.path(1);
    QFile file;
    bool uploaded = true;

    // first, we have to find out which format to use
    QString strFileName = url.path(-1);
    QFileInfo fileInfo(strFileName);
    QString fileExt = fileInfo.extension();
    QString fileFormat = "xmi";
    if (fileExt == "xmi" || fileExt == "bak.xmi")
    {
        fileFormat = "xmi";
    } else if (fileExt == "xmi.tgz" || fileExt == "bak.xmi.tgz") {
        fileFormat = "tgz";
    } else if (fileExt == "xmi.tar.bz2" || fileExt == "bak.xmi.tar.bz2") {
        fileFormat = "bz2";
    } else {
        fileFormat = "xmi";
    }

    initSaveTimer();

    if (fileFormat == "tgz" || fileFormat == "bz2")
    {
        KTar * archive;
        KTempFile tmp_tgz_file;

        // first we have to check if we are saving to a local or remote file
        if (url.isLocalFile())
        {
            if (fileFormat == "tgz") // check tgz or bzip2
            {
                archive = new KTar(d.path(), "application/x-gzip");
            } else {
                archive = new KTar(d.path(), "application/x-bzip2");
            }
        } else {
            if (fileFormat == "tgz") // check tgz or bzip2
            {
                archive = new KTar(tmp_tgz_file.name(), "application/x-gzip");
            } else {
                archive = new KTar(tmp_tgz_file.name(), "application/x-bzip2");
            }
        }

        // now check if we can write to the file
        if (archive->open(IO_WriteOnly) == false)
        {
            KMessageBox::error(0, i18n("There was a problem saving file: %1").arg(d.path()), i18n("Save Error"));
            return false;
        }

        // we have to create a temporary xmi file
        // we will add this file later to the archive
        KTempFile tmp_xmi_file;
        file.setName(tmp_xmi_file.name());
        if( !file.open( IO_WriteOnly ) ) {
            KMessageBox::error(0, i18n("There was a problem saving file: %1").arg(d.path()), i18n("Save Error"));
            return false;
        }
        saveToXMI(file); // save XMI to this file...
        file.close(); // ...and close it

        // now add this file to the archive, but without the extension
        QString tmpQString = url.fileName();
        if (fileFormat == "tgz")
        {
            tmpQString.replace(QRegExp("\\.tgz$"), "");
        } else {
            tmpQString.replace(QRegExp("\\.tar\\.bz2$"), "");
        }
        archive->addLocalFile(tmp_xmi_file.name(), tmpQString);
        archive->close();

#if KDE_IS_VERSION(3,4,89)
        if (!archive->closeSucceeded())
        {
            KMessageBox::error(0, i18n("There was a problem saving file: %1").arg(d.path()), i18n("Save Error"));
            return false;
        }
#endif
        // now the xmi file was added to the archive, so we can delete it
        tmp_xmi_file.close();
        tmp_xmi_file.unlink();

        // now we have to check, if we have to upload the file
        if ( !url.isLocalFile() ) {
            uploaded = KIO::NetAccess::upload( tmp_tgz_file.name(), m_doc_url,
                                               UMLApp::app() );
        }

        // now the archive was written to disk (or remote) so we can delete the
        // objects
        tmp_tgz_file.close();
        tmp_tgz_file.unlink();
        delete archive;

    } else {
        // save as normal uncompressed XMI

        KTempFile tmpfile; // we need this tmp file if we are writing to a remote file

        // save in _any_ case to a temp file
        // -> if something goes wrong during saveToXmi, the
        //     original content is preserved
        //     ( e.g. if umbrello dies in the middle of the document model parsing
        //      for saveToXMI due to some problems )
        /// @TODO insert some checks in saveToXMI to detect a failed save attempt
        file.setName( tmpfile.name() );

        // lets open the file for writing
        if( !file.open( IO_WriteOnly ) ) {
            KMessageBox::error(0, i18n("There was a problem saving file: %1").arg(d.path()), i18n("Save Error"));
            return false;
        }
        saveToXMI(file); // save the xmi stuff to it
        file.close();
        tmpfile.close();

        // if it is a remote file, we have to upload the tmp file
        if ( !url.isLocalFile() ) {
            uploaded = KIO::NetAccess::upload( tmpfile.name(), m_doc_url, UMLApp::app() );
        } else {
            // now remove the original file
            if ( KIO::NetAccess::file_move( tmpfile.name(), d.path(), -1, true ) == false ) {
                KMessageBox::error(0, i18n("There was a problem saving file: %1").arg(d.path()), i18n("Save Error"));
                m_doc_url.setFileName(i18n("Untitled"));
                return false;
            }
        }
    }
    if( !uploaded )
    {
        KMessageBox::error(0, i18n("There was a problem uploading file: %1").arg(d.path()), i18n("Save Error"));
        m_doc_url.setFileName(i18n("Untitled"));
    }
    setModified(false);
    return uploaded;
}

void UMLDoc::setupSignals() {
    WorkToolBar *tb = UMLApp::app() -> getWorkToolBar();


    connect(this, SIGNAL(sigDiagramChanged(Uml::Diagram_Type)), tb, SLOT(slotCheckToolBar(Uml::Diagram_Type)));
    //new signals below

    return;
}

UMLView * UMLDoc::findView(Uml::IDType id) {
    UMLView *v = NULL;
    for (int i = 0; i < Uml::N_MODELTYPES; i++) {
        v = m_root[i]->findView(id);
        if (v)
            break;
    }
    return v;
}

UMLView * UMLDoc::findView(Uml::Diagram_Type type, const QString &name,
                           bool searchAllScopes /* =false */) {
    Uml::Model_Type mt = Model_Utils::convert_DT_MT(type);
    return m_root[mt]->findView(type, name, searchAllScopes);
}

UMLObject* UMLDoc::findObjectById(Uml::IDType id) {
    UMLObject *o = NULL;
    for (int i = 0; i < Uml::N_MODELTYPES; i++) {
        if (id == m_root[i]->getID())
            return m_root[i];
        o = m_root[i]->findObjectById(id);
        if (o)
            return o;
    }
    o = findStereotypeById(id);
    return o;
}

UMLStereotype * UMLDoc::findStereotypeById(Uml::IDType id) {
    for (UMLStereotype *s = m_stereoList.first(); s; s = m_stereoList.next() ) {
        if (s->getID() == id)
            return s;
    }
    return NULL;
}

UMLObject* UMLDoc::findUMLObject(const QString &name,
                                 Uml::Object_Type type /* = ot_UMLObject */,
                                 UMLObject *currentObj /* = NULL */) {
    UMLObject *o = m_datatypeRoot->findObject(name);
    if (o)
        return o;
    for (int i = 0; i < Uml::N_MODELTYPES; i++) {
        UMLObjectList list = m_root[i]->containedObjects();
        o = Model_Utils::findUMLObject(list, name, type, currentObj);
        if (o)
            return o;
        if ((type == ot_UMLObject || type == ot_Folder) &&
             name == m_root[i]->getName())
            return m_root[i];
    }
    return NULL;
}

UMLClassifier* UMLDoc::findUMLClassifier(const QString &name) {
    //this is used only by code generator so we don't need to look at Datatypes
    UMLObject * obj = findUMLObject(name);
    return dynamic_cast<UMLClassifier*>(obj);
}

/**
  *   Adds a UMLObject thats already created but doesn't change
  *   any ids or signal.  Used by the list view.  Use
  *   AddUMLObjectPaste if pasting.
  */
bool UMLDoc::addUMLObject(UMLObject* object) {
    Object_Type ot = object->getBaseType();
    if (ot == ot_Attribute || ot == ot_Operation || ot == ot_EnumLiteral
            || ot == ot_EntityAttribute || ot == ot_Template || ot == ot_Stereotype) {
        kDebug() << "UMLDoc::addUMLObject(" << object->getName()
            << "): not adding type " << ot << endl;
        return false;
    }
    UMLPackage *pkg = object->getUMLPackage();
    if (pkg == NULL) {
        pkg = currentRoot();
        kDebug() << "UMLDoc::addUMLObject(" << object->getName()
            << "): no parent package set, assuming " << pkg->getName() << endl;
        object->setUMLPackage( pkg );
    }
    return pkg->addObject(object);
}

void UMLDoc::addStereotype(const UMLStereotype *s) {
    if (! m_stereoList.contains(s))
        m_stereoList.append(s);
}

void UMLDoc::removeStereotype(const UMLStereotype *s) {
    if (m_stereoList.contains(s))
        m_stereoList.remove(s);
}

void UMLDoc::writeToStatusBar(const QString &text) {
    emit sigWriteToStatusBar(text);
}

// simple removal of an object
void UMLDoc::slotRemoveUMLObject(UMLObject* object)  {
    //m_objectList.remove(object);
    UMLPackage *pkg = object->getUMLPackage();
    if (pkg == NULL) {
        kError() << "UMLDoc::slotRemoveUMLObject(" << object->getName()
            << "): parent package is not set !" << endl;
        return;
    }
    pkg->removeObject(object);
}

bool UMLDoc::isUnique(const QString &name)
{
    UMLListView *listView = UMLApp::app()->getListView();
    UMLListViewItem *currentItem = (UMLListViewItem*)listView->currentItem();
    UMLListViewItem *parentItem = 0;

    // check for current item, if its a package, then we do a check on that
    // otherwise, if current item exists, find its parent and check if thats
    // a package..
    if(currentItem)
    {
        // its possible that the current item *is* a package, then just
        // do check now
        if (Model_Utils::typeIsContainer(currentItem->getType()))
            return isUnique (name, (UMLPackage*) currentItem->getUMLObject());
        parentItem = (UMLListViewItem*)currentItem->parent();
    }

    // item is in a package so do check only in that
    if (parentItem != NULL && Model_Utils::typeIsContainer(parentItem->getType())) {
        UMLPackage *parentPkg = static_cast<UMLPackage*>(parentItem->getUMLObject());
        return isUnique(name, parentPkg);
    }

    kError() << "UMLDoc::isUnique(" << name << "): Not currently in a package"
        << endl;
    /* Check against all objects that _don't_ have a parent package.
    for (UMLObjectListIt oit(m_objectList); oit.current(); ++oit) {
        UMLObject *obj = oit.current();
        if (obj->getUMLPackage() == NULL && obj->getName() == name)
            return false;
    }
     */
    return true;
}

bool UMLDoc::isUnique(const QString &name, UMLPackage *package)
{
    // if a package, then only do check in that
    if (package)
        return (package->findObject(name) == NULL);

    // Not currently in a package: ERROR
    kError() << "UMLDoc::isUnique(2)(" << name << "): Not currently in a package"
        << endl;
    /* Check against all objects that _don't_ have a parent package.
    for (UMLObjectListIt oit(m_objectList); oit.current(); ++oit) {
        UMLObject *obj = oit.current();
        if (obj->getUMLPackage() == NULL && obj->getName() == name)
            return false;
    }
     */
    return true;
}

UMLStereotype* UMLDoc::findStereotype(const QString &name) {
    UMLStereotype *s;
    for (UMLStereotypeListIt it(m_stereoList); (s = it.current()) != NULL; ++it) {
        if (s->getName() == name)
            return s;
    }
    return NULL;
}

UMLStereotype* UMLDoc::findOrCreateStereotype(const QString &name) {
    UMLStereotype *s = findStereotype(name);
    if (s != NULL) {
        return s;
    }
    s = new UMLStereotype(name, STR2ID(name));
    addStereotype(s);
    //emit modified();
    return s;
}

void UMLDoc::removeAssociation (UMLAssociation * assoc, bool doSetModified /*=true*/) {
    if(!assoc)
        return;

    // Remove the UMLAssociation from m_objectList.
    UMLPackage *pkg = assoc->getUMLPackage();
    if (pkg == NULL) {
        kError() << "UMLDoc::removeAssociation(" << assoc->getName()
            << "): parent package is not set !" << endl;
        return;
    }
    pkg->removeObject(assoc);

    if (doSetModified)  // so we will save our document
        setModified(true, false);
}

UMLAssociation * UMLDoc::findAssociation(Uml::Association_Type assocType,
        const UMLObject *roleAObj,
        const UMLObject *roleBObj,
        bool *swap)
{
    UMLAssociationList assocs = getAssociations();
    UMLAssociation *a, *ret = NULL;
    for (a = assocs.first(); a; a = assocs.next()) {
        if (a->getAssocType() != assocType)
            continue;
        if (a->getObject(Uml::A) == roleAObj && a->getObject(Uml::B) == roleBObj)
            return a;
        if (a->getObject(Uml::A) == roleBObj && a->getObject(Uml::B) == roleAObj) {
            ret = a;
        }
    }
    if (swap)
        *swap = (ret != NULL);
    return ret;
}

// create AND add an association. Used by refactoring assistant.
UMLAssociation* UMLDoc::createUMLAssociation(UMLObject *a, UMLObject *b, Uml::Association_Type type)
{
    bool swap;
    UMLAssociation *assoc = findAssociation(type, a, b, &swap);
    if (assoc == NULL) {
        assoc = new UMLAssociation(type, a, b );
        addAssociation(assoc);
    }
    return assoc;
}

void UMLDoc::addAssociation(UMLAssociation *Assoc)
{
    if (Assoc == NULL)
        return;

    // First, check that this association has not already been added.
    // This may happen when loading old XMI files where all the association
    // information was taken from the <UML:AssocWidget> tag.
    UMLAssociationList assocs = getAssociations();
    for (UMLAssociationListIt ait(assocs); ait.current(); ++ait) {
        UMLAssociation *a = ait.current();
        // check if its already been added (shouldn't be the case right now
        // as UMLAssociations only belong to one associationwidget at a time)
        if (a == Assoc)
        {
            kDebug() << "UMLDoc::addAssociation: duplicate addition attempted"
                << endl;
            return;
        }
    }

    // If we get here it's really a new association.

    // Add the UMLAssociation at the owning UMLPackage.
    UMLPackage *pkg = Assoc->getUMLPackage();
    if (pkg == NULL) {
        kError() << "UMLDoc::addAssociation(" << Assoc->getName()
            << "): parent package is not set !" << endl;
        return;
    }
    pkg->addObject(Assoc);

    // I don't believe this appropriate, UMLAssociations ARENT UMLWidgets -b.t.
    // emit sigObjectCreated(o);

    setModified(true);
}

QString UMLDoc::uniqViewName(const Uml::Diagram_Type type) {
    QString dname;
    if(type == dt_UseCase)
        dname = i18n("use case diagram");
    else if(type == dt_Class)
        dname = i18n("class diagram");
    else if(type == dt_Sequence)
        dname = i18n("sequence diagram");
    else if(type == dt_Collaboration)
        dname = i18n("collaboration diagram");
    else if( type == dt_State )
        dname = i18n( "state diagram" );
    else if( type == dt_Activity )
        dname = i18n( "activity diagram" );
    else if( type == dt_Component )
        dname = i18n( "component diagram" );
    else if( type == dt_Deployment )
        dname = i18n( "deployment diagram" );
    else if( type == dt_EntityRelationship )
        dname = i18n( "entity relationship diagram" );
    else {
        kWarning() << "uniqViewName() called with unknown diagram type" << endl;
    }
    QString name = dname;
    for (int number = 0; findView(type, name, true); ++number,
            name = dname + '_' + QString::number(number))
        ;
    return name;
}

bool UMLDoc::loading() const {
    return m_bLoading;
}

void UMLDoc::setLoading(bool state /* = true */) {
    m_bLoading = state;
}

UMLView* UMLDoc::createDiagram(UMLFolder *folder, Uml::Diagram_Type type, bool askForName /*= true */) {
    bool ok = true;
    QString name,
    dname = uniqViewName(type);

    while(true) {
        if (askForName)  {
            name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), dname, &ok, (QWidget*)UMLApp::app());
        } else {
            name = dname;
        }
        if (!ok)  {
            break;
        }
        if (name.length() == 0)  {
            KMessageBox::error(0, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
        } else if(!findView(type, name)) {
            UMLView* temp = new UMLView(folder);
            temp -> setOptionState( Settings::getOptionState() );
            temp->setName( name );
            temp->setType( type );
            temp->setID( UniqueID::gen() );
            addView(temp);
            emit sigDiagramCreated( temp->getID() );
            setModified(true, false);
            UMLApp::app()->enablePrint(true);
            changeCurrentView( temp->getID() );
            return temp;
        } else {
            KMessageBox::error(0, i18n("A diagram is already using that name."), i18n("Not a Unique Name"));
        }
    }//end while
    return 0;
}

void UMLDoc::renameDiagram(Uml::IDType id) {
    bool ok = false;

    UMLView *temp =  findView(id);
    Diagram_Type type = temp->getType();

    QString oldName= temp->getName();
    while(true) {
        QString name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)UMLApp::app());

        if(!ok)
            break;
        if(name.length() == 0)
            KMessageBox::error(0, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
        else if(!findView(type, name)) {
            temp->setName(name);

            emit sigDiagramRenamed(id);
            setModified(true);
            break;
        } else
            KMessageBox::error(0, i18n("A diagram is already using that name."), i18n("Not a Unique Name"));
    }
}

void UMLDoc::renameUMLObject(UMLObject *o) {
    bool ok = false;
    QString oldName= o->getName();
    while(true) {
        QString name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)UMLApp::app());
        if(!ok)
            break;
        if(name.length() == 0)
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        else if (isUnique(name)) {
            o->setName(name);
            setModified(true);
            break;
        } else {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        }
    }
    return;
}

void UMLDoc::renameChildUMLObject(UMLObject *o) {
    bool ok = false;
    UMLClassifier* p = dynamic_cast<UMLClassifier *>(o->parent());
    if(!p) {
        kDebug() << "Can't create object, no parent found" << endl;
        return;
    }

    QString oldName= o->getName();
    while(true) {
        QString name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)UMLApp::app());
        if(!ok)
            break;
        if(name.length() == 0)
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        else {
            if (p->findChildObject(name) == NULL
                    || ((o->getBaseType() == Uml::ot_Operation) && KMessageBox::warningYesNo( kapp -> mainWidget() ,
                            i18n( "The name you entered was not unique.\nIs this what you wanted?" ),
                            i18n( "Name Not Unique"),i18n("Use Name"),i18n("Enter New Name")) == KMessageBox::Yes) ) {
                o->setName(name);
                setModified(true);
                break;
            } else {
                KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
            }
        }
    }
}

void UMLDoc::changeCurrentView(Uml::IDType id) {
    UMLApp* pApp = UMLApp::app();
    UMLView* w = findView(id);
    if (w) {
        pApp->setCurrentView(w);
        emit sigDiagramChanged(w->getType());
        pApp->setDiagramMenuItemsState( true );
        setModified(true);
    }
    emit sigCurrentViewChanged();
}

void UMLDoc::removeDiagram(Uml::IDType id) {
    UMLApp::app()->getDocWindow()->updateDocumentation(true);
    UMLView* umlview = findView(id);
    if(!umlview)
    {
        kError()<<"Request to remove diagram " << ID2STR(id) << ": Diagram not found!"<<endl;
        return;
    }
    if (KMessageBox::warningContinueCancel(0, i18n("Are you sure you want to delete diagram %1?").arg(umlview->getName()), i18n("Delete Diagram"),KGuiItem( i18n("&Delete"), "editdelete")) == KMessageBox::Continue) {
        removeView(umlview);
        emit sigDiagramRemoved(id);
        setModified(true);
        /* if (infoWidget->isVisible()) {
               emit sigDiagramChanged(dt_Undefined);
               UMLApp::app()->enablePrint(false);
           }
        */ //FIXME sort out all the KActions for when there's no diagram
        //also remove the buttons from the WorkToolBar, then get rid of infowidget
    }
}

UMLFolder *UMLDoc::currentRoot() {
    UMLView *currentView = UMLApp::app()->getCurrentView();
    if (currentView == NULL) {
        if (m_pCurrentRoot)
            return m_pCurrentRoot;
        kError() << "UMLDoc::currentRoot: m_pCurrentRoot is NULL" << endl;
        return NULL;
    }
    UMLFolder *f = currentView->getFolder();
    while (f->getUMLPackage()) {
        f = static_cast<UMLFolder*>(f->getUMLPackage());
    }
    return f;
}

void UMLDoc::setCurrentRoot(Uml::Model_Type rootType) {
    m_pCurrentRoot = m_root[rootType];
}

void UMLDoc::removeUMLObject(UMLObject* umlobject) {
    UMLApp::app()->getDocWindow()->updateDocumentation(true);
    Object_Type type = umlobject->getBaseType();

    umlobject->setUMLStereotype(NULL);  // triggers possible cleanup of UMLStereotype
    if (dynamic_cast<UMLClassifierListItem*>(umlobject))  {
        UMLClassifier* parent = dynamic_cast<UMLClassifier*>(umlobject->parent());
        if (parent == NULL) {
            kError() << "UMLDoc::removeUMLObject: parent of umlobject is NULL"
                << endl;
            return;
        }
        if (type == ot_Operation) {
            parent->removeOperation(static_cast<UMLOperation*>(umlobject));
        } else if (type == ot_EnumLiteral) {
            UMLEnum *e = static_cast<UMLEnum*>(parent);
            e->removeEnumLiteral(static_cast<UMLEnumLiteral*>(umlobject));
        } else if (type == ot_EntityAttribute) {
            UMLEntity *ent = static_cast<UMLEntity*>(parent);
            ent->removeEntityAttribute(static_cast<UMLClassifierListItem*>(umlobject));
        } else {
            UMLClassifier* pClass = dynamic_cast<UMLClassifier*>(parent);
            if (pClass == NULL)  {
                kError() << "UMLDoc::removeUMLObject: parent of umlobject has "
                    << "unexpected type " << parent->getBaseType() << endl;
                return;
            }
            if (type == ot_Attribute) {
                pClass->removeAttribute(static_cast<UMLAttribute*>(umlobject));
            } else if (type == ot_Template) {
                pClass->removeTemplate(static_cast<UMLTemplate*>(umlobject));
            } else {
                kError() << "UMLDoc::removeUMLObject: umlobject has "
                    << "unexpected type " << type << endl;
            }
        }
    } else {
        if (type == ot_Association) {
            UMLAssociation *a = (UMLAssociation *)umlobject;
            removeAssociation(a, false);  // don't call setModified here, it's done below
        } else {
            UMLPackage* pkg = umlobject->getUMLPackage();
            if (pkg) {
                pkg->removeObject(umlobject);
            } else {
                kError() << "UMLDoc::removeUMLObject(" << umlobject->getName()
                    << "): parent package is not set !" << endl;
            }
        }
        emit sigObjectRemoved(umlobject);
    }
    setModified(true);
}

void UMLDoc::signalUMLObjectCreated(UMLObject * o) {
    emit sigObjectCreated(o);
    /* This is the wrong place to do:
               setModified(true);
       Instead, that should be done by the callers when object creation and all
       its side effects (e.g. new widget in view, new list view item, etc.) is
       finalized.
     */
}

void UMLDoc::setName(const QString& name) {
    m_Name = name;
}

QString UMLDoc::getName() const {
    return m_Name;
}

Uml::IDType UMLDoc::getModelID() const {
    return m_modelID;
}

void UMLDoc::saveToXMI(QIODevice& file) {
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

    /**
     * See comment on <XMI.model> above
    header.appendChild( model );
     */
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
    QValueList<Uml::IDType> stereoIDs;
    for (UMLStereotype *s = m_stereoList.first(); s; s = m_stereoList.next() ) {
        QString stName = s->getName();
        Uml::IDType stID = s->getID();
        if (!stereoNames.contains(stName) && !stereoIDs.contains(stID)) {
            s->saveToXMI(doc, ownedNS);
            stereoNames.append(stName);
            stereoIDs.append(stID);
        } else {
            kDebug() << "UMLDoc::saveToXMI: encountered duplicated stereotype "
                << stName << " (id " << ID2STR(stID) << "), see bug 144924" << endl;
        }
    }
    for (int i = 0; i < Uml::N_MODELTYPES; i++) {
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
    UMLView *currentView = UMLApp::app()->getCurrentView();
    if (currentView)
        viewID = currentView->getID();
    docElement.setAttribute( "viewid", ID2STR(viewID) );
    docElement.setAttribute( "documentation", m_Doc );
    docElement.setAttribute( "uniqueid", ID2STR(UniqueID::get()) );
    extensions.appendChild( docElement );

    //  save listview
    UMLApp::app()->getListView()->saveToXMI(doc, extensions);

    // save code generator
    CodeGenerator *codegen = UMLApp::app()->getGenerator();
    if (codegen) {
        QDomElement codeGenElement = doc.createElement( "codegeneration" );
        codegen->saveToXMI( doc, codeGenElement );
        extensions.appendChild( codeGenElement );
    }

    root.appendChild( extensions );

    QTextStream stream( &file );
    stream.setEncoding(QTextStream::UnicodeUTF8);
    stream << doc.toString();
}

short UMLDoc::getEncoding(QIODevice & file)
{
    QTextStream stream( &file );
    stream.setEncoding(QTextStream::UnicodeUTF8);
    QString data = stream.read();
    QString error;
    int line;
    QDomDocument doc;
    if( !doc.setContent( data, false, &error, &line ) )
    {
        kWarning()<<"Can't set content: "<<error<<" Line: "<<line<<endl;
        return ENC_UNKNOWN;
    }

    // we start at the beginning and go to the point in the header where we can
    // find out if the file was saved using Unicode
    QDomNode node = doc.firstChild();
    while (node.isComment() || node.isProcessingInstruction())
    {
        node = node.nextSibling();
    }
    QDomElement root = node.toElement();
    if( root.isNull() )
    {
        return ENC_UNKNOWN;
    }
    //  make sure it is an XMI file
    if( root.tagName() != "XMI" )
    {
        return ENC_UNKNOWN;
    }
    node = node.firstChild();

    if ( node.isNull() )
        return ENC_UNKNOWN;

    QDomElement element = node.toElement();
    // check header
    if( element.isNull() || element.tagName() != "XMI.header" )
        return ENC_UNKNOWN;

    QDomNode headerNode = node.firstChild();
    while ( !headerNode.isNull() )
    {
        QDomElement headerElement = headerNode.toElement();
        // the information if Unicode was used is now stored in the
        // XMI.documentation section of the header
        if (headerElement.isNull() ||
                headerElement.tagName() != "XMI.documentation") {
            headerNode = headerNode.nextSibling();
            continue;
        }
        QDomNode docuNode = headerNode.firstChild();
        while ( !docuNode.isNull() )
        {
            QDomElement docuElement = docuNode.toElement();
            // a tag XMI.exporterEncoding was added since version 1.2 to
            // mark a file as saved with Unicode
            if (! docuElement.isNull() &&
                    docuElement.tagName() == "XMI.exporterEncoding")
            {
                // at the moment this if isn't really necessary, but maybe
                // later we will have other encoding standards
                if (docuElement.text() == QString("UnicodeUTF8"))
                {
                    return ENC_UNICODE; // stop here
                }
            }
            docuNode = docuNode.nextSibling();
        }
        break;
    }
    return ENC_OLD_ENC;
}

bool UMLDoc::loadFromXMI( QIODevice & file, short encode )
{
    // old Umbrello versions (version < 1.2) didn't save the XMI in Unicode
    // this wasn't correct, because non Latin1 chars where lost
    // to ensure backward compatibility we have to ensure to load the old files
    // with non Unicode encoding
    if (encode == ENC_UNKNOWN)
    {
        if ((encode = getEncoding(file)) == ENC_UNKNOWN)
            return false;
        file.reset();
    }
    QTextStream stream( &file );
    if (encode == ENC_UNICODE)
    {
        stream.setEncoding(QTextStream::UnicodeUTF8);
    }

    QString data = stream.read();
    kapp->processEvents();  // give UI events a chance
    QString error;
    int line;
    QDomDocument doc;
    if( !doc.setContent( data, false, &error, &line ) ) {
        kWarning()<<"Can't set content:"<<error<<" Line:"<<line<<endl;
        return false;
    }
    kapp->processEvents();  // give UI events a chance
    QDomNode node = doc.firstChild();
    //Before Umbrello 1.1-rc1 we didn't add a <?xml heading
    //so we allow the option of this being missing
    while (node.isComment() || node.isProcessingInstruction()) {
        node = node.nextSibling();
    }

    QDomElement root = node.toElement();
    if( root.isNull() ) {
        return false;
    }
    //  make sure it is an XMI file
    if( root.tagName() != "XMI" ) {
        return false;
    }

    m_nViewID = Uml::id_None;
    for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement element = node.toElement();
        if (element.isNull()) {
            kDebug() << "loadFromXMI: skip empty elem" << endl;
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
            if (!recognized)
                kDebug() << "UMLDoc::loadFromXMI: skipping <"
                    << outerTag << ">" << endl;
            continue;
        }
        bool seen_UMLObjects = false;
        //process content
        for (QDomNode child = node.firstChild(); !child.isNull();
                child = child.nextSibling()) {
            if (child.isComment())
                continue;
            element = child.toElement();
            QString tag = element.tagName();
            if (tag == "umlobjects"  // for bkwd compat.
                    || tagEq(tag, "Subsystem")
                    || tagEq(tag, "Model") ) {
                if( !loadUMLObjectsFromXMI( element ) ) {
                    kWarning() << "failed load on objects" << endl;
                    return false;
                }
                m_Name = element.attribute( "name", i18n("UML Model") );
                UMLListView *lv = UMLApp::app()->getListView();
                lv->setColumnText(0, m_Name);
                seen_UMLObjects = true;
            } else if (tagEq(tag, "Package") ||
                       tagEq(tag, "Class") ||
                       tagEq(tag, "Interface")) {
                // These tests are only for foreign XMI files that
                // are missing the <Model> tag (e.g. NSUML)
                QDomElement parentElem = node.toElement();
                if( !loadUMLObjectsFromXMI( parentElem ) ) {
                    kWarning() << "failed load on model objects" << endl;
                    return false;
                }
                seen_UMLObjects = true;
            } else if (tagEq(tag, "TaggedValue")) {
                // This tag is produced here, i.e. outside of <UML:Model>,
                // by the Unisys.JCR.1 Rose-to-XMI tool.
                if (! seen_UMLObjects) {
                    kDebug() << "skipping TaggedValue because not seen_UMLObjects"
                        << endl;
                    continue;
                }
                tag = element.attribute("tag", "");
                if (tag != "documentation") {
                    continue;
                }
                QString modelElement = element.attribute("modelElement", "");
                if (modelElement.isEmpty()) {
                    kDebug() << "skipping TaggedValue(documentation) because "
                        << "modelElement.isEmpty()" << endl;
                    continue;
                }
                UMLObject *o = findObjectById(STR2ID(modelElement));
                if (o == NULL) {
                    kDebug() << "TaggedValue(documentation): cannot find object"
                        << " for modelElement " << modelElement << endl;
                    continue;
                }
                QString value = element.attribute("value", "");
                if (! value.isEmpty())
                    o->setDoc(value);
            } else {
                // for backward compatibility
                loadExtensionsFromXMI(child);
            }
        }
    }
#ifdef VERBOSE_DEBUGGING
    kDebug() << "UMLDoc::m_objectList.count() is " << m_objectList.count() << endl;
#endif
    resolveTypes();
    // set a default code generator if no <XMI.extensions><codegeneration> tag seen
    if (UMLApp::app()->getGenerator() == NULL)
        UMLApp::app()->setGenerator(UMLApp::app()->getDefaultLanguage());
    emit sigWriteToStatusBar( i18n("Setting up the document...") );
    kapp->processEvents();  // give UI events a chance
    activateAllViews();

    UMLView *viewToBeSet = NULL;
    if (m_nViewID != Uml::id_None)
        viewToBeSet = findView( m_nViewID );
    if (viewToBeSet) {
        changeCurrentView( m_nViewID );
        Settings::OptionState optionState = Settings::getOptionState();
        if (optionState.generalState.tabdiagrams) {
            UMLApp::app()->tabWidget()->showPage(viewToBeSet);
        }
    } else {
        createDiagram(m_root[mt_Logical], Uml::dt_Class, false);
        m_pCurrentRoot = m_root[mt_Logical];
    }
    emit sigResetStatusbarProgress();
    return true;
}

void UMLDoc::resolveTypes() {
    // Resolve the types.
    // This is done in a separate pass because of possible forward references.
    if (m_bTypesAreResolved)
        return;
    m_bTypesAreResolved = true;
    writeToStatusBar( i18n("Resolving object references...") );
    for (int i = 0; i < Uml::N_MODELTYPES; i++) {
       UMLFolder *obj = m_root[i];
#ifdef VERBOSE_DEBUGGING
        kDebug() << "UMLDoc: invoking resolveRef() for " << obj->getName()
            << " (id=" << ID2STR(obj->getID()) << ")" << endl;
#endif
        obj->resolveRef();
    }
    kapp->processEvents();  // give UI events a chance
}

bool UMLDoc::validateXMIHeader(QDomNode& headerNode) {
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

bool UMLDoc::loadUMLObjectsFromXMI(QDomElement& element) {
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
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (tagEq(type, "Model")) {
            bool foundUmbrelloRootFolder = false;
            QString name = tempElement.attribute("name");
            for (int i = 0; i < Uml::N_MODELTYPES; i++) {
                if (name == m_root[i]->getName()) {
                    m_pCurrentRoot = m_root[i];
                    m_root[i]->loadFromXMI(tempElement);
                    foundUmbrelloRootFolder = true;
                    break;
                }
            }
            if (foundUmbrelloRootFolder)
                continue;
        }
        // From here on, it's support for stereotypes, pre 1.5.5 versions, and foreign files
        if (tagEq(type, "Namespace.ownedElement") ||
                tagEq(type, "Namespace.contents") ||
                tagEq(type, "Model")) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore the <UML:Namespace.ownedElement> tag is of no
            // significance.
            if( !loadUMLObjectsFromXMI( tempElement ) ) {
                kWarning() << "failed load on " << type << endl;
                return false;
            }
            continue;
        }
        if (Model_Utils::isCommonXMIAttribute(type))
            continue;
        if (! tempElement.hasAttribute("xmi.id")) {
            QString idref = tempElement.attribute("xmi.idref", "");
            if (! idref.isEmpty()) {
                kDebug() << "resolution of xmi.idref " << idref
                    << " is not yet implemented" << endl;
            } else {
                kError() << "Cannot load " << type
                    << " because xmi.id is missing" << endl;
            }
            continue;
        }
        QString stID = tempElement.attribute("stereotype", "");
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type, stID);
        if( !pObject ) {
            kWarning() << "Unknown type of umlobject to create: " << type << endl;
            // We want a best effort, therefore this is handled as a
            // soft error.
            continue;
        }
        Uml::Object_Type ot = pObject->getBaseType();
        // Set the parent root folder.
        UMLPackage *pkg = NULL;
        if (ot == Uml::ot_Datatype) {
            pkg = m_datatypeRoot;
        } else {
            Uml::Model_Type guess = Model_Utils::guessContainer(pObject);
            if (guess != Uml::N_MODELTYPES)
                pkg = m_root[guess];
        }
        pObject->setUMLPackage(pkg);

        bool status = pObject -> loadFromXMI( tempElement );
        if ( !status ) {
            //delete pObject;
            // Unfortunately we cannot do this because the pObject
            // may be still referenced by other model objects.
            kError() << "loadFromXMI failed for " << pObject->getName() << " xmi.id="
                << ID2STR(pObject->getID()) << endl;
            continue;
        }
        pkg = pObject->getUMLPackage();
        if (ot == ot_Stereotype) {
            UMLStereotype *s = static_cast<UMLStereotype*>(pObject);
            UMLStereotype *exist = findStereotype(pObject->getName());
            if (exist) {
                if (exist->getID() == pObject->getID()) {
                    delete pObject;
                } else {
                    kDebug() << "Stereotype " << pObject->getName()
                        << "(id=" << ID2STR(pObject->getID())
                        << ") already exists with id="
                        << ID2STR(exist->getID()) << endl;
                    addStereotype(s);
                }
            } else {
                addStereotype(s);
            }
            continue;
        }
        if (pkg == NULL)
            kError() << "UMLDoc::loadUMLObjectsFromXMI: pkg is NULL for "
                << pObject->getName() << " xmi.id="
                << ID2STR(pObject->getID()) << endl;
        else
            pkg->addObject(pObject);

        /* FIXME see comment at loadUMLObjectsFromXMI
        emit sigSetStatusbarProgress( ++m_count );
         */
    }
    return true;
}

void UMLDoc::setMainViewID(Uml::IDType viewID) {
    m_nViewID = viewID;
}

void UMLDoc::loadExtensionsFromXMI(QDomNode& node) {
    QDomElement element = node.toElement();
    QString tag = element.tagName();

    if (tag == "docsettings") {
        QString viewID = element.attribute( "viewid", "-1" );
        m_Doc = element.attribute( "documentation", "" );
        QString uniqueid = element.attribute( "uniqueid", "0" );

        m_nViewID = STR2ID(viewID);
        UniqueID::set(STR2ID(uniqueid));
        UMLApp::app()->getDocWindow() -> newDocumentation();

    } else if (tag == "diagrams" || tag == "UISModelElement") {
        // For backward compatibility only:
        // Since version 1.5.5 diagrams are saved as part of the UMLFolder.
        QDomNode diagramNode = node.firstChild();
        if (tag == "UISModelElement") {          // Unisys.IntegratePlus.2
            element = diagramNode.toElement();
            tag = element.tagName();
            if (tag != "uisOwnedDiagram") {
                kError() << "unknown child node " << tag << endl;
                return;
            }
            diagramNode = diagramNode.firstChild();
        }
        if( !loadDiagramsFromXMI( diagramNode ) ) {
            kWarning() << "failed load on diagrams" << endl;
        }

    } else if (tag == "listview") {
        //FIXME: Need to resolveTypes() before loading listview,
        //       else listview items are duplicated.
        resolveTypes();
        if( !UMLApp::app()->getListView() -> loadFromXMI( element ) ) {
            kWarning() << "failed load on listview" << endl;
        }

    } else if (tag == "codegeneration") {
        QDomNode cgnode = node.firstChild();
        QDomElement cgelement = cgnode.toElement();
        while( !cgelement.isNull() ) {
            QString nodeName = cgelement.tagName();
            QString lang = cgelement.attribute("language","UNKNOWN");
            Uml::Programming_Language pl = Model_Utils::stringToProgLang(lang);
            CodeGenerator *g = UMLApp::app()->setGenerator(pl);
            g->loadFromXMI(cgelement);
            cgnode = cgnode.nextSibling();
            cgelement = cgnode.toElement();
        }
        if (UMLApp::app()->getGenerator() == NULL)
            UMLApp::app()->setGenerator(UMLApp::app()->getDefaultLanguage());
    }
}

// For backward compatibility only:
// Since version 1.5.5 diagrams are saved as part of the UMLFolder.
bool UMLDoc::loadDiagramsFromXMI( QDomNode & node ) {
    emit sigWriteToStatusBar( i18n("Loading diagrams...") );
    emit sigResetStatusbarProgress();
    emit sigSetStatusbarProgress( 0 );
    emit sigSetStatusbarProgressSteps( 10 ); //FIX ME
    QDomElement element = node.toElement();
    if( element.isNull() )
        return true;//return ok as it means there is no umlobjects
    const Settings::OptionState state = Settings::getOptionState();
    UMLView * pView = 0;
    int count = 0;
    while( !element.isNull() ) {
        QString tag = element.tagName();
        if (tag == "diagram" || tag == "UISDiagram") {
            pView = new UMLView(NULL);
            // IMPORTANT: Set OptionState of new UMLView _BEFORE_
            // reading the corresponding diagram:
            // + allow using per-diagram color and line-width settings
            // + avoid crashes due to uninitialized values for lineWidth
            pView -> setOptionState( state );
            bool success = false;
            if (tag == "UISDiagram") {
                success = pView->loadUISDiagram(element);
            } else {
                success = pView->loadFromXMI(element);
            }
            if (!success) {
                kWarning() << "failed load on viewdata loadfromXMI" << endl;
                delete pView;
                return false;
            }
            // Put diagram in default predefined folder.
            // @todo pass in the parent folder - it might be a user defined one.
            Uml::Model_Type mt = Model_Utils::convert_DT_MT(pView->getType());
            pView->setFolder(m_root[mt]);
            pView -> hide();
            addView( pView );
            emit sigSetStatusbarProgress( ++count );
            kapp->processEvents();  // give UI events a chance
        }
        node = node.nextSibling();
        element = node.toElement();
    }
    return true;
}

void UMLDoc::removeAllViews() {
    for (int i = 0; i < Uml::N_MODELTYPES; i++)
        m_root[i]->removeAllViews();
    UMLApp::app()->setCurrentView(NULL);
    emit sigDiagramChanged(dt_Undefined);
    UMLApp::app()->setDiagramMenuItemsState(false);
}

UMLClassifierList UMLDoc::getConcepts(bool includeNested /* =true */) {
    UMLClassifierList conceptList;
    m_root[mt_Logical]->appendClassifiers(conceptList, includeNested);
    return conceptList;
}

UMLClassifierList UMLDoc::getClasses(bool includeNested /* =true */) {
    UMLClassifierList conceptList;
    m_root[mt_Logical]->appendClasses(conceptList, includeNested);
    return conceptList;
}

UMLClassifierList UMLDoc::getClassesAndInterfaces(bool includeNested /* =true */) {
    UMLClassifierList conceptList;
    m_root[mt_Logical]->appendClassesAndInterfaces(conceptList, includeNested);
    return conceptList;
}

UMLClassifierList UMLDoc::getInterfaces(bool includeNested /* =true */) {
    UMLClassifierList interfaceList;
    m_root[mt_Logical]->appendInterfaces(interfaceList, includeNested);
    return interfaceList;
}

UMLClassifierList UMLDoc::getDatatypes() {
    UMLObjectList objects = m_datatypeRoot->containedObjects();
    UMLClassifierList datatypeList;
    UMLObject *obj;
    for (UMLObjectListIt oit(objects); (obj = oit.current()) != NULL; ++oit) {
        if (obj->getBaseType() == ot_Datatype) {
            datatypeList.append(static_cast<UMLClassifier*>(obj));
        }
    }
    return datatypeList;
}

UMLAssociationList UMLDoc::getAssociations() {
    UMLAssociationList associationList;
    for (int i = 0; i < Uml::N_MODELTYPES; i++) {
        UMLAssociationList assocs = m_root[i]->getAssociations();
        UMLAssociation *a;
        for (UMLAssociationListIt ait(assocs); (a = ait.current()) != NULL; ++ait)
            associationList.append(a);
    }
    return associationList;
}

void UMLDoc::print(KPrinter * pPrinter) {
    UMLView * printView = 0;
    int count = QString(pPrinter -> option("kde-uml-count")).toInt();
    QPainter painter(pPrinter);
    for(int i = 0;i < count;i++) {
        if(i>0)
            pPrinter -> newPage();
        QString diagram = i18n("kde-uml-Diagram") + QString("%1").arg(i);
        QString sID = pPrinter -> option(diagram);
        Uml::IDType id = STR2ID(sID);
        printView = findView(id);

        if(printView)
            printView ->print(pPrinter, painter);
        printView = 0;
    }
    painter.end();
}

UMLViewList UMLDoc::getViewIterator() {
    UMLViewList accumulator;
    for (int i = 0; i < Uml::N_MODELTYPES; i++)
        m_root[i]->appendViews(accumulator, true);
    return accumulator;
}

void UMLDoc::setModified(bool modified /*=true*/, bool addToUndo /*=true*/) {
    if(!m_bLoading) {
        m_modified = modified;
        UMLApp::app()->setModified(modified);

        if (modified && addToUndo) {
            addToUndoStack();
            clearRedoStack();
        }
    }
}

bool UMLDoc::assignNewIDs(UMLObject* Obj) {
    if(!Obj || !m_pChangeLog) {
        kDebug() << "no Obj || Changelog" << endl;
        return false;
    }
    Uml::IDType result = assignNewID(Obj->getID());
    Obj->setID(result);

    //If it is a CONCEPT then change the ids of all its operations and attributes
    if(Obj->getBaseType() == ot_Class ) {
        UMLClassifier *c = static_cast<UMLClassifier*>(Obj);
        UMLClassifierListItemList attributes = c->getFilteredList(ot_Attribute);
        for(UMLObject* listItem = attributes.first(); listItem; listItem = attributes.next()) {
            result = assignNewID(listItem->getID());
            listItem->setID(result);
        }

        UMLClassifierListItemList templates = c->getFilteredList(ot_Template);
        for(UMLObject* listItem = templates.first(); listItem; listItem = templates.next()) {
            result = assignNewID(listItem->getID());
            listItem->setID(result);
        }
    }

    if(Obj->getBaseType() == ot_Interface || Obj->getBaseType() == ot_Class ) {
        UMLOperationList operations(((UMLClassifier*)Obj)->getOpList());
        for(UMLObject* listItem = operations.first(); listItem; listItem = operations.next()) {
            result = assignNewID(listItem->getID());
            listItem->setID(result);
        }
    }

    setModified(true);

    return true;
}

UMLFolder *UMLDoc::getRootFolder(Uml::Model_Type mt) {
    if (mt < Uml::mt_Logical || mt >= Uml::N_MODELTYPES) {
        kError() << "UMLDoc::getRootFolder: illegal input value " << mt << endl;
        return NULL;
    }
    return m_root[mt];
}

Uml::Model_Type UMLDoc::rootFolderType(UMLObject *obj) {
    for (int i = 0; i < Uml::N_MODELTYPES; i++) {
        const Uml::Model_Type m = (Uml::Model_Type)i;
        if (obj == m_root[m])
            return m;
    }
    return Uml::N_MODELTYPES;
}

/** Read property of IDChangeLog* m_pChangeLog. */
IDChangeLog* UMLDoc::getChangeLog() {
    return m_pChangeLog;
}

/** Opens a Paste session,
Deletes the Old ChangeLog and Creates an empty one */

void UMLDoc::beginPaste() {
    if(m_pChangeLog) {
        delete m_pChangeLog;
        m_pChangeLog = 0;
    }
    m_pChangeLog = new IDChangeLog;
}

/** Closes a Paste session,
Deletes the ChangeLog */
void UMLDoc::endPaste() {
    if(m_pChangeLog) {
        delete m_pChangeLog;
        m_pChangeLog = 0;
    }
}

/** Assigns a New ID to an Object, and also logs the assignment to its internal
ChangeLog */
Uml::IDType UMLDoc::assignNewID(Uml::IDType OldID) {
    Uml::IDType result = UniqueID::gen();
    if (m_pChangeLog) {
        m_pChangeLog->addIDChange(OldID, result);
    }
    return result;
}

/** Adds an already created UMLView to the document, it gets assigned a new ID.
    If its name is already in use then the function appends a number to it to
    differentiate it from the others; this number is incremental so if
    number 1 is in use then it tries 2 and then 3 and so on */
bool UMLDoc::addUMLView(UMLView * pView ) {
    if(!pView || !m_pChangeLog)
        return false;

    int i = 0;
    QString viewName = (QString)pView->getName();
    QString name = viewName;
    while( findView(pView->getType(), name) != NULL) {
        name = viewName + '_' + QString::number(++i);
    }
    if(i) //If name was modified
        pView->setName(name);
    Uml::IDType result = assignNewID(pView->getID());
    pView->setID(result);

    pView->activateAfterLoad( true );
    pView->endPartialWidgetPaste();
    pView->setOptionState( Settings::getOptionState() );
    addView(pView);
    setModified(true);
    return true;
}

void UMLDoc::activateAllViews() {
    // store old setting - for restore of last setting
    bool m_bLoading_old = m_bLoading;
    m_bLoading = true; //this is to prevent document becoming modified when activating a view

    for (int i = 0; i < Uml::N_MODELTYPES; i++)
        m_root[i]->activateViews();
    m_bLoading = m_bLoading_old;
}

void UMLDoc::settingsChanged(Settings::OptionState optionState) {
    for (int i = 0; i < Uml::N_MODELTYPES; i++)
        m_root[i]->setViewOptions(optionState);
    initSaveTimer();
}

void UMLDoc::initSaveTimer() {
    if( m_pAutoSaveTimer ) {
        m_pAutoSaveTimer -> stop();
        disconnect( m_pAutoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
        delete m_pAutoSaveTimer;
        m_pAutoSaveTimer = 0;
    }
    Settings::OptionState optionState = Settings::getOptionState();
    if( optionState.generalState.autosave ) {
        m_pAutoSaveTimer = new QTimer(this, "_AUTOSAVETIMER_" );
        connect( m_pAutoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
        m_pAutoSaveTimer->start( optionState.generalState.autosavetime * 60000, false );
    }
    return;
}

void UMLDoc::slotAutoSave() {
    //Only save if modified.
    if( !m_modified ) {
        return;
    }
    KURL tempURL = m_doc_url;
    if( tempURL.fileName() == i18n("Untitled") ) {
        tempURL.setPath( QDir::homeDirPath() + i18n("/autosave%1").arg(".xmi") );
        saveDocument( tempURL );
        m_doc_url.setFileName( i18n("Untitled") );
        m_modified = true;
        UMLApp::app()->setModified( m_modified );
    } else {
        // 2004-05-17 Achim Spangler
        KURL orgDocUrl = m_doc_url;
        QString orgFileName = m_doc_url.fileName();
        // don't overwrite manually saved file with autosave content
        QString fileName = tempURL.fileName();
        Settings::OptionState optionState = Settings::getOptionState();
        fileName.replace( ".xmi", optionState.generalState.autosavesuffix );
        tempURL.setFileName( fileName );
        // End Achim Spangler

        saveDocument( tempURL );
        // 2004-05-17 Achim Spangler
        // re-activate m_modified if autosave is writing to other file
        // than the main project file -> autosave-suffix != ".xmi"
        if ( ".xmi" != optionState.generalState.autosavesuffix ) {
            m_modified = true;
            UMLApp::app()->setModified( m_modified );
        }
        // restore original file name -
        // UMLDoc::saveDocument() sets doc_url to filename which is given as autosave-filename
        setURL( orgDocUrl );
        UMLApp * pApp = UMLApp::app();
        pApp->setCaption(orgFileName, isModified() );
        // End Achim Spangler
    }
}

void UMLDoc::signalDiagramRenamed(UMLView* pView ) {
    Settings::OptionState optionState = Settings::getOptionState();
    if (optionState.generalState.tabdiagrams)
        UMLApp::app()->tabWidget()->setTabLabel( pView, pView->getName() );
    emit sigDiagramRenamed( pView -> getID() );
}

void UMLDoc::addToUndoStack() {
    Settings::OptionState optionState = Settings::getOptionState();
    if (!m_bLoading && optionState.generalState.undo) {
        QBuffer* buffer = new QBuffer();
        buffer->open(IO_WriteOnly);
        QDataStream* undoData = new QDataStream();
        undoData->setDevice(buffer);
        saveToXMI(*buffer);
        buffer->close();
        undoStack.prepend(undoData);

        if (undoStack.count() > 1) {
            UMLApp::app()->enableUndo(true);
        }
    }
}

void UMLDoc::clearUndoStack() {
    undoStack.setAutoDelete(true);
    undoStack.clear();
    UMLApp::app()->enableRedo(false);
    undoStack.setAutoDelete(false);
    clearRedoStack();
}

void UMLDoc::clearRedoStack() {
    redoStack.setAutoDelete(true);
    redoStack.clear();
    UMLApp::app()->enableRedo(false);
    redoStack.setAutoDelete(false);
}

void UMLDoc::loadUndoData() {
    if (undoStack.count() < 1) {
        kWarning() << "no data in undostack" << endl;
        return;
    }
    UMLView *currentView = UMLApp::app()->getCurrentView();
    if (currentView == NULL) {
        kWarning() << "UMLDoc::loadUndoData: currentView is NULL" << endl;
        undoStack.setAutoDelete(true);
        undoStack.clear();
        undoStack.setAutoDelete(false);
        UMLApp::app()->enableUndo(false);
        return;
    }
    Uml::IDType currentViewID = currentView->getID();
    // store old setting - for restore of last setting
    bool m_bLoading_old = m_bLoading;
    m_bLoading = true;
    closeDocument();
    redoStack.prepend( undoStack.take(0) );
    QDataStream* undoData = undoStack.getFirst();
    QBuffer* buffer = static_cast<QBuffer*>( undoData->device() );
    buffer->open(IO_ReadOnly);
    loadFromXMI(*buffer);
    buffer->close();

    setModified(true, false);
    m_bLoading = m_bLoading_old;

    undoStack.setAutoDelete(true);
    if (undoStack.count() <= 1) {
        UMLApp::app()->enableUndo(false);
    }
    if (redoStack.count() >= 1) {
        UMLApp::app()->enableRedo(true);
    }
    while (undoStack.count() > undoMax) {
        undoStack.removeLast();
    }
    undoStack.setAutoDelete(false);

    currentView = UMLApp::app()->getCurrentView();
    if (currentView) {
        if (currentView->getID() != currentViewID)
            changeCurrentView( currentView->getID() );
        currentView->resizeCanvasToItems();
    }
}

void UMLDoc::loadRedoData() {
    if (redoStack.count() >= 1) {
        UMLView *currentView = UMLApp::app()->getCurrentView();
        Uml::IDType currentViewID = currentView->getID();
        // store old setting - for restore of last setting
        bool m_bLoading_old = m_bLoading;
        m_bLoading = true;
        closeDocument();
        undoStack.prepend( redoStack.getFirst() );
        QDataStream* redoData = redoStack.getFirst();
        redoStack.removeFirst();
        QBuffer* buffer = static_cast<QBuffer*>( redoData->device() );
        buffer->open(IO_ReadOnly);
        loadFromXMI(*buffer);
        buffer->close();

        setModified(true, false);
        currentView = UMLApp::app()->getCurrentView();
        currentView->resizeCanvasToItems();
        m_bLoading = m_bLoading_old;

        redoStack.setAutoDelete(true);
        if (redoStack.count() < 1) {
            UMLApp::app()->enableRedo(false);
        }
        if (undoStack.count() > 1) {
            UMLApp::app()->enableUndo(true);
        }
        if (currentView->getID() != currentViewID) {
            changeCurrentView(currentViewID);
        }
        redoStack.setAutoDelete(false);
    } else {
        kWarning() << "no data in redostack" << endl;
    }
}

void UMLDoc::addDefaultDatatypes() {
    CodeGenerator *cg = UMLApp::app()->getGenerator();
    if (cg == NULL) {
        kDebug() << "UMLDoc::addDefaultDatatypes: CodeGenerator is still NULL"
            << endl;
        return;
    }
    QStringList entries = cg->defaultDatatypes();
    QStringList::Iterator end(entries.end());
    for (QStringList::Iterator it = entries.begin(); it != end; ++it)
        createDatatype(*it);
}

void UMLDoc::createDatatype(const QString &name)  {
    UMLObjectList datatypes = m_datatypeRoot->containedObjects();
    UMLObject* umlobject = Model_Utils::findUMLObject(datatypes, name,
                                                      ot_Datatype, m_datatypeRoot);
    if (!umlobject) {
        Object_Factory::createUMLObject(ot_Datatype, name, m_datatypeRoot);
    }
    UMLApp::app()->getListView()->closeDatatypesFolder();
}

void UMLDoc::slotDiagramPopupMenu(QWidget* umlview, const QPoint& point) {
    UMLView* view = (UMLView*) umlview;
    if(m_pTabPopupMenu != 0) {
        m_pTabPopupMenu->hide();
        delete m_pTabPopupMenu;
        m_pTabPopupMenu = 0;
    }
    Settings::OptionState optionState = Settings::getOptionState();
    if (! optionState.generalState.tabdiagrams)
        return;

    Uml::ListView_Type type = lvt_Unknown;
    switch( view->getType() ) {
    case dt_Class:
        type = lvt_Class_Diagram;
        break;

    case dt_UseCase:
        type = lvt_UseCase_Diagram;
        break;

    case dt_Sequence:
        type = lvt_Sequence_Diagram;
        break;

    case dt_Collaboration:
        type = lvt_Collaboration_Diagram;
        break;

    case dt_State:
        type = lvt_State_Diagram;
        break;

    case dt_Activity:
        type = lvt_Activity_Diagram;
        break;

    case dt_Component:
        type = lvt_Component_Diagram;
        break;

    case dt_Deployment:
        type = lvt_Deployment_Diagram;
        break;

    case dt_EntityRelationship:
        type = lvt_EntityRelationship_Diagram;
        break;

    default:
        kWarning() << "unknown diagram type in slotDiagramPopupMenu()" << endl;
        break;
    }//end switch

    m_pTabPopupMenu = new ListPopupMenu(UMLApp::app()->getMainViewWidget(), type);
    m_pTabPopupMenu->popup(point);
    connect(m_pTabPopupMenu, SIGNAL(activated(int)), view, SLOT(slotMenuSelection(int)));
}

void UMLDoc::addDefaultStereotypes() {
    CodeGenerator *gen = UMLApp::app()->getGenerator();
    if (gen)
        gen->createDefaultStereotypes();
}

const UMLStereotypeList& UMLDoc::getStereotypes() {
    return m_stereoList;
}


#include "umldoc.moc"

