/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "actor.h"
#include "associationwidget.h"
#include "association.h"
#include "class.h"
#include "package.h"
#include "component.h"
#include "codegenerator.h"
#include "classimport.h"
#include "node.h"
#include "artifact.h"
#include "interface.h"
#include "datatype.h"
#include "enum.h"
#include "docwindow.h"
#include "objectwidget.h"
#include "operation.h"
#include "attribute.h"
#include "template.h"
#include "enumliteral.h"
#include "stereotype.h"
#include "classifierlistitem.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umlview.h"
#include "usecase.h"
#include "clipboard/idchangelog.h"
#include "dialogs/classpropdlg.h"
#include "dialogs/umlattributedialog.h"
#include "dialogs/umltemplatedialog.h"
#include "dialogs/umloperationdialog.h"
#include "inputdialog.h"

#include <qpainter.h>
#include <qtimer.h>
#include <qbuffer.h>
#include <qdir.h>
#include <qregexp.h>

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
#if KDE_IS_VERSION(3,2,0)
# include <ktempdir.h>
#endif
#include <ktempfile.h>

#define XMI_FILE_VERSION "1.2.90"
// Hmm, if the XMI_FILE_VERSION is meant to reflect the umbrello version
// then the version number "1.3" is prone to create confusion with the UML
// DTD version...

static const uint undoMax = 30;


UMLDoc::UMLDoc() {
	m_currentView = 0;
	m_uniqueID = 0;
	m_count = 0;
	m_classImporter = 0;
	m_currentcodegenerator = 0;
	m_objectList.clear();
	m_objectList.setAutoDelete(false); // DONT autodelete
	m_ViewList.setAutoDelete(true);

	m_codeGenerationXMIParamMap = new QMap<QString, QDomElement>;

	m_pChangeLog = 0;
	m_Doc = "";
	m_modified = false;
	m_bLoading = false;
	m_pAutoSaveTimer = 0;
	m_nViewID = -1;
	m_highestIDforForeignFile = 0;
	UMLApp * pApp = UMLApp::app();
	connect(this, SIGNAL(sigDiagramCreated(int)), pApp, SLOT(slotUpdateViews()));
	connect(this, SIGNAL(sigDiagramRemoved(int)), pApp, SLOT(slotUpdateViews()));
	connect(this, SIGNAL(sigDiagramRenamed(int)), pApp, SLOT(slotUpdateViews()));
	connect(this, SIGNAL( sigCurrentViewChanged() ), pApp, SLOT( slotCurrentViewChanged() ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLDoc::~UMLDoc() {
	delete m_pChangeLog;
	m_pChangeLog = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::addView(UMLView *view) {
	UMLApp * pApp = UMLApp::app();
	if ( pApp->getListView() )
		connect(this, SIGNAL(sigObjectRemoved(UMLObject *)), view, SLOT(slotObjectRemoved(UMLObject *)));
	m_ViewList.append(view);

	if ( ! m_bLoading ) {
		if (m_currentView == NULL) {
			m_currentView = view;
			view -> show();
			emit sigDiagramChanged(view ->getType());
		} else {
			view -> hide();
		}
	}
	pApp->setDiagramMenuItemsState(true);
	pApp->slotUpdateViews();
	pApp->setCurrentView(view);

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::removeView(UMLView *view , bool enforceCurrentView ) {
	if(!view)
	{
		kdError()<<"UMLDoc::removeView(UMLView *view) called with view = 0"<<endl;
		return;
	}
	if ( UMLApp::app()->getListView() ) {
		disconnect(this,SIGNAL(sigObjectRemoved(UMLObject *)), view,SLOT(slotObjectRemoved(UMLObject *)));
	}
	view->hide();
	//remove all widgets before deleting view
	view->removeAllWidgets();
	// m_ViewLiset is set to autodelete!!
	m_ViewList.remove(view);
	if (m_currentView == view)
	{
		m_currentView = NULL;
		UMLView* firstView = m_ViewList.first();
		if (!firstView && enforceCurrentView) //create a diagram
		{
			createDiagram( dt_Class, false );
			firstView = m_ViewList.first();
			//UMLApp::app()->setDiagramMenuItemsState(false);
		}

		if ( firstView )
		{
			changeCurrentView( firstView->getID() );
			UMLApp::app()->setDiagramMenuItemsState(true);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::setURL(const KURL &url) {
	m_doc_url = url;
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const KURL& UMLDoc::URL() const {
	return m_doc_url;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::slotUpdateAllViews(UMLView *sender) {
	for(UMLView *w = m_ViewList.first(); w; w = m_ViewList.next()) {
		if(w != sender) {
			w->repaint();
		}
	}
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::saveModified() {
	bool completed(true);
	if (!m_modified)
		return completed;

	UMLApp *win = UMLApp::app();
	int want_save = KMessageBox::warningYesNoCancel(win, i18n("The current file has been modified.\nDo you want to save it?"), i18n("Warning"));
	switch(want_save) {
		case KMessageBox::Yes:
			if (m_doc_url.fileName() == i18n("Untitled")) {
				if (win->slotFileSaveAs()) {
					deleteContents();
					completed=true;
				} else {
					completed=false;
				}
			} else {
				saveDocument(URL());
				deleteContents();
				completed=true;
			}
			break;

		case KMessageBox::No:
			setModified(false);
			deleteContents();
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
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::closeDocument() {
	deleteContents();
}

bool UMLDoc::newDocument() {
	/*deleteContents();*/ closeDocument();
	m_currentView = NULL;
	m_doc_url.setFileName(i18n("Untitled"));
	//see if we need to start with a new diagram
	Settings::OptionState optionState = UMLApp::app()->getOptionState();

	switch( optionState.generalState.diagram ) {
		case Settings::diagram_usecase:
			createDiagram( Uml::dt_UseCase, false);
			break;

		case Settings::diagram_class:
			createDiagram( Uml::dt_Class, false );
			break;

		case Settings::diagram_sequence:
			createDiagram( Uml::dt_Sequence, false );
			break;

		case Settings::diagram_collaboration:
			createDiagram( Uml::dt_Collaboration, false );
			break;

		case Settings::diagram_state:
			createDiagram( Uml::dt_State, false );
			break;

		case Settings::diagram_activity:
			createDiagram( Uml::dt_Activity, false );
			break;

		case Settings::diagram_component:
			createDiagram( Uml::dt_Component, false );
			break;

		case Settings::diagram_deployment:
			createDiagram( Uml::dt_Deployment, false );
			break;
		default:
			break;
	}//end switch

	addDefaultDatatypes();

	setModified(false);
	initSaveTimer();

	UMLApp::app()->enableUndo(false);
	clearUndoStack();
	addToUndoStack();

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::openDocument(const KURL& url, const char* /*format =0*/) {
	if(url.fileName().length() == 0) {
		newDocument();
		return false;
	}

	m_doc_url = url;
	QDir d = url.path(1);
	deleteContents();
	// IMPORTANT: set m_bLoading to true
	// _AFTER_ the call of UMLDoc::deleteContents()
	// as it sets m_bLoading to false afer it was temporarily
	// changed to true to block recording of changes in redo-buffer
	m_bLoading = true;
	QString tmpfile;
	KIO::NetAccess::download( url, tmpfile
#if KDE_IS_VERSION(3,1,90)
					      , UMLApp::app()
#endif
				 );
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

# if KDE_IS_VERSION(3,1,90)
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

		// now go through all entries till we find an xmi file
		for (it = entries.begin(); it != entries.end(); it++)
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
	} else
# endif
	{
		// no, it seems to be an ordinary file
		if( !file.open( IO_ReadOnly ) ) {
			KMessageBox::error(0, i18n("There was a problem loading file: %1").arg(d.path()), i18n("Load Error"));
			m_doc_url.setFileName(i18n("Untitled"));
			m_bLoading = false;
			newDocument();
			return false;
		}
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

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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

#if KDE_IS_VERSION(3,2,0)
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
		QString tmpQString = url.filename();
		if (fileFormat == "tgz")
		{
			tmpQString.replace(QRegExp("\\.tgz$"), "");
		} else {
			tmpQString.replace(QRegExp("\\.tar\\.bz2$"), "");
		}
		archive->addLocalFile(tmp_xmi_file.name(), tmpQString);
		archive->close();

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

	} else
// stop HERE with the special handling of the KDE_IS_VERSION(3,2,0)
// as otherwise, the _DEFAULT!!!_ case of saving uncompressed XMI
// isn't handled anymore!!
#endif
	{
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
		saveToXMI( file ); // save the xmi stuff to it
		file.close();
		tmpfile.close();

		// if it is a remote file, we have to upload the tmp file
		if ( !url.isLocalFile() ) {
			uploaded = KIO::NetAccess::upload( tmpfile.name(), m_doc_url
# if KDE_IS_VERSION(3,1,90)
									, UMLApp::app()
# endif
							 );
		} else {
			// now remove the original file
			if ( KIO::file_move( tmpfile.name(), d.path(), -1, true ) == false ) {
				KMessageBox::error(0, i18n("There was a problem saving file: %1").arg(d.path()), i18n("Save Error"));
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

void UMLDoc::setCurrentCodeGenerator ( CodeGenerator * gen ) {
	addCodeGenerator(gen); // wont add IF it already exists
	m_currentcodegenerator = gen;
}

CodeGenerator* UMLDoc::getCurrentCodeGenerator() {
	return m_currentcodegenerator;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::deleteContents() {

	m_Doc = "";
	DocWindow* dw = UMLApp::app()->getDocWindow();
	if (dw) {
		dw->newDocumentation();
	}

	// remove all code generators
	QDictIterator<CodeGenerator> it( m_codeGeneratorDictionary );
	for( ; it.current(); ++it )
		removeCodeGenerator(it.current());

	m_currentcodegenerator = 0;

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
		if (m_objectList.count() > 0) {
			// clear our object list. We do this explicitly since setAutoDelete is false for the objectList now.
			for(UMLObject * obj = m_objectList.first(); obj != 0; obj = m_objectList.next())
				obj->deleteLater();
			m_objectList.clear();
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::setupSignals() {
	WorkToolBar *tb = UMLApp::app() -> getWorkToolBar();


	connect(this, SIGNAL(sigDiagramChanged(Uml::Diagram_Type)), tb, SLOT(slotCheckToolBar(Uml::Diagram_Type)));
	//new signals below

	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::addCodeGenerator ( CodeGenerator * gen)
{
	if(!gen)
		return false;

	QString tag = gen->getLanguage(); // this should be unique

	if(m_codeGeneratorDictionary.find(tag))
		return false; // return false, we already have some object with this tag in the list
	else
		m_codeGeneratorDictionary.insert(tag, gen);

	return true;
}

bool UMLDoc::hasCodeGeneratorXMIParams ( QString lang )
{
	if (m_codeGenerationXMIParamMap->contains(lang))
		return true;
	return false;
}

QDomElement UMLDoc::getCodeGeneratorXMIParams ( QString lang )
{
	return ((*m_codeGenerationXMIParamMap)[lang]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Remove a CodeGenerator object
 */
bool UMLDoc::removeCodeGenerator ( CodeGenerator * remove_object ) {
	QString lang = remove_object->getLanguage();
	if(!(lang.isEmpty()) && m_codeGeneratorDictionary.find(lang))
	{
		m_codeGenerationXMIParamMap->erase(lang);
		m_codeGeneratorDictionary.remove(lang);
		delete remove_object;
	} else
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CodeGenerator * UMLDoc::findCodeGeneratorByLanguage (QString lang) {
	return m_codeGeneratorDictionary.find(lang);
}

ClassImport * UMLDoc::classImport() {
	if (m_classImporter == NULL)
		m_classImporter = new ClassImport(this);
	return m_classImporter;
}

UMLView * UMLDoc::findView(int id) {
	for(UMLView *w = m_ViewList.first(); w; w = m_ViewList.next()) {
		if(w->getID() ==id) {
			return w;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLView * UMLDoc::findView(Diagram_Type type, QString name) {
	for(UMLView *w = m_ViewList.first(); w; w = m_ViewList.next()) {
		if( (w->getType() == type) && ( w->getName() == name) ) {
			return w;
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::findUMLObject(int id) {
	for(UMLObject * obj = m_objectList.first(); obj != 0; obj = m_objectList.next())
	{
		if(obj -> getID() == id)
			return obj;
		UMLObject *o;
		UMLObject_Type t = obj->getBaseType();
		switch (t) {
			case Uml::ot_Package:
				o = ((UMLPackage*)obj)->findObject(id);
				if (o)
					return o;
				break;
			case Uml::ot_Interface:
			case Uml::ot_Class:
			case Uml::ot_Enum:
				o = ((UMLClassifier*)obj)->findChildObject(id);
				if (o)
					return o;
				if (t == ot_Interface || t == ot_Class) {
					o = ((UMLPackage*)obj)->findObject(id);
					if (o)
						return o;
				}
				break;
			default:
				break;
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::findUMLObject(QString name, UMLObject_Type type /* = ot_UMLObject */) {
	return findUMLObject(m_objectList, name, type);
}

UMLObject* UMLDoc::findUMLObject(UMLObjectList inList, QString name,
				 UMLObject_Type type /* = ot_UMLObject */) {
	QStringList components = QStringList::split("::", name);
	QString nameWithoutFirstPrefix;
	if (components.size() > 1) {
		name = components.front();
		components.pop_front();
		nameWithoutFirstPrefix = components.join("::");
	}
	for (UMLObject * obj = inList.first(); obj != 0; obj = inList.next()) {
		if (obj->getName() != name)
			continue;
		UMLObject_Type foundType = obj->getBaseType();
		if (nameWithoutFirstPrefix.isEmpty()) {
			if (type != ot_UMLObject && type != foundType) {
				kdDebug() << "findUMLObject: type mismatch for "
					  << name << " (seeking type: "
					  << type << ", found type: "
					  << foundType << ")" << endl;
				continue;
			}
			return obj;
		}
		if (foundType != Uml::ot_Package) {
			kdDebug() << "findUMLObject: found \"" << name
				  << "\" is not a package (?)" << endl;
			continue;
		}
		UMLPackage *pkg = static_cast<UMLPackage*>(obj);
		return findUMLObject( pkg->containedObjects(),
				      nameWithoutFirstPrefix, type );
	}
	return NULL;
}

UMLObject* UMLDoc::findObjectByIdStr(QString idStr) {
	for (UMLObject * o = m_objectList.first(); o; o = m_objectList.next()) {
		if (o->getAuxId() == idStr)
			return o;
		UMLObject *inner = NULL;
		switch (o->getBaseType()) {
			case Uml::ot_Package:
				inner = ((UMLPackage*)o)->findObjectByIdStr(idStr);
				if (inner)
					return inner;
				break;
			case Uml::ot_Interface:
			case Uml::ot_Class:
			case Uml::ot_Enum:
				inner = ((UMLClassifier*)o)->findChildObjectByIdStr(idStr);
				if (inner)
					return inner;
				break;
			default:
				break;
		}
	}
	return NULL;
}

UMLClassifier* UMLDoc::findUMLClassifier(QString name) {
	// could be either UMLClass or UMLInterface..
	//this is used only by code generator so we don't need to look at Datatypes
	UMLObject * obj = findUMLObject(name);
	return dynamic_cast<UMLClassifier*>(obj);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString	UMLDoc::uniqObjectName(const UMLObject_Type type, QString prefix) {
	QString	currentName = prefix;
	if (currentName.isEmpty()) {
		if(type == ot_Class)
			currentName = i18n("new_class");
		else if(type == ot_Actor)
			currentName = i18n("new_actor");
		else if(type == ot_UseCase)
			currentName = i18n("new_usecase");
		else if(type == ot_Package)
			currentName = i18n("new_package");
		else if(type == ot_Component)
			currentName = i18n("new_component");
		else if(type == ot_Node)
			currentName = i18n("new_node");
		else if(type == ot_Artifact)
			currentName = i18n("new_artifact");
		else if(type == ot_Interface)
			currentName = i18n("new_interface");
		else if(type == ot_Datatype)
			currentName = i18n("new_datatype");
		else if(type == ot_Enum)
			currentName = i18n("new_enum");
		else if(type == ot_Association)
			currentName = i18n("new_association");
		else
			currentName = i18n("new_object");
	}
	QString name = currentName;
	for (int number = 1; !isUnique(name); number++)  {
		name = currentName + "_" + QString::number(number);
	}
	return name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/**
  *   Adds a UMLObject thats already created but doesn't change
  *   any ids or signal.  Used by the list view.  Use
  *   AddUMLObjectPaste if pasting.
  */
void UMLDoc::addUMLObject(UMLObject* object) {
	UMLObject_Type ot = object->getBaseType();
	if (ot == ot_Attribute || ot == ot_Operation || ot == ot_EnumLiteral) {
		kdDebug() << "UMLDoc::addUMLObject(" << object->getName()
			<< "): not adding type " << ot << endl;
		return;
	}
	UMLPackage *pkg = object->getUMLPackage();
	if (pkg != NULL) {
		// CHECK: If UMLDoc::addUMLObject is invoked on an object
		// that is inside a package then that is really a misuse.
		// The following is nothing but a hack to deal with such
		// misuse:
		pkg->addObject(object);
		kdDebug() << "UMLDoc::addUMLObject(" << object->getName()
			  << "): bad call, adding at containing package instead"
			  << endl;
		// end of hack
		return;
	}
	//stop it being added twice
	if ( m_objectList.find(object) == -1)  {
		m_objectList.append( object );
		if (object->getID() > m_highestIDforForeignFile)
			m_highestIDforForeignFile = object->getID();
	} else {
		kdDebug() << "UMLDoc::addUMLObject: not adding " << object->getName()
			  << " because already there." << endl;
	}
}

void UMLDoc::writeToStatusBar(const QString &text) {
	emit sigWriteToStatusBar(text);
}

// simple removal of an object
void UMLDoc::slotRemoveUMLObject(UMLObject* object)  {
	m_objectList.remove(object);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::isUnique(QString name)
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
		if(currentItem->getType() == lvt_Package)
			return isUnique (name, (UMLPackage*) currentItem->getUMLObject());
		parentItem = (UMLListViewItem*)currentItem->parent();
	}

	// item is in a package so do check only in that
	if (parentItem != NULL && parentItem->getType() == lvt_Package) {
		UMLPackage *parentPkg = (UMLPackage*)parentItem->getUMLObject();
		return isUnique(name, parentPkg);
	}

	// Not currently in a package:
	// Check against all objects that _dont_ have a parent package.
	for (UMLObject *obj = m_objectList.first(); obj; obj = m_objectList.next())
		if (obj->getUMLPackage() == NULL && obj->getName() == name)
			return false;
	return true;
}

bool UMLDoc::isUnique(QString name, UMLPackage *package)
{

	// if a package, then only do check in that
	if (package)
		return (package->findObject(name) == NULL);

	// Not currently in a package:
	// Check against all objects that _dont_ have a parent package.
	for (UMLObject *obj = m_objectList.first(); obj; obj = m_objectList.next())
		if (obj->getUMLPackage() == NULL && obj->getName() == name)
			return false;
	return true;

}

UMLObject* UMLDoc::createUMLObject(const std::type_info &type)
{
//adapter.. just transform and forward request
	UMLObject_Type t;
	if ( type == typeid(UMLClass) ) {
		t = ot_Class;
	} else if ( type == typeid(UMLUseCase) ) {
		t = ot_UseCase;
	} else if ( type == typeid(UMLActor) ) {
		t = ot_Actor;
	} else if ( type == typeid(UMLPackage) ) {
		t = ot_Package;
	} else if ( type == typeid(UMLComponent) ) {
		t = ot_Component;
	} else if ( type == typeid(UMLNode) ) {
		t = ot_Node;
	} else if ( type == typeid(UMLArtifact) ) {
		t = ot_Artifact;
	} else if ( type == typeid(UMLInterface) )  {
		t = ot_Interface;
	} else if ( type == typeid(UMLDatatype) )  {
		t = ot_Datatype;
	} else if ( type == typeid(UMLEnum) )  {
		t = ot_Enum;
	} else {
		return static_cast<UMLObject*>(0L);
	}
	return createUMLObject(t);
}

UMLObject* UMLDoc::createUMLObject(UMLObject_Type type, const QString &n,
				   UMLPackage *parentPkg /* = NULL */) {
	bool ok = false;
	int id;
	QString name;
	if( !n.isEmpty() && isUnique(n, parentPkg) )
	{
		name = n;
	}
	else
	{
		name = uniqObjectName(type);
		bool bValidNameEntered = false;
		do {
			name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), name, &ok, (QWidget*)UMLApp::app());
			if (!ok) {
				return 0;
			}
			if (name.length() == 0) {
				KMessageBox::error(0, i18n("That is an invalid name."),
						   i18n("Invalid Name"));
				continue;
			}
			if (getCurrentCodeGenerator() != NULL &&
			    getCurrentCodeGenerator()->isReservedKeyword(name)) {
				KMessageBox::error(0, i18n("This is a reserved keyword for the language of the configured code generator."),
						   i18n("Reserved Keyword"));
				continue;
      }
			if (! isUnique(name, parentPkg)) {
				KMessageBox::error(0, i18n("That name is already being used."),
						   i18n("Not a Unique Name"));
				continue;
			}
			bValidNameEntered = true;
		} while (bValidNameEntered == false);
	}
	UMLObject *o = NULL;
	id = getUniqueID();
	if(type == ot_Actor) {
		o = new UMLActor(name, id);
	} else if(type == ot_UseCase) {
		o = new UMLUseCase(name, id);
	} else if(type == ot_Class ) {
		o = new UMLClass (name, id);
	} else if(type == ot_Package) {
		o = new UMLPackage(name, id);
	} else if(type == ot_Component) {
		o = new UMLComponent(name, id);
	} else if(type == ot_Node) {
		o = new UMLNode(name, id);
	} else if(type == ot_Artifact) {
		o = new UMLArtifact(name, id);
	} else if(type == ot_Interface) {
		o = new UMLInterface(name, id);
	} else if(type == ot_Datatype) {
		o = new UMLDatatype(name, id);
	} else if(type == ot_Enum) {
		o = new UMLEnum(name, id);
	} else {
		kdWarning() << "CreateUMLObject(int) error" << endl;
		return (UMLObject*)0L;
	}
	o->setUMLPackage(parentPkg);
	m_objectList.append(o);
	emit sigObjectCreated(o);
	setModified(true);
	return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createChildObject(UMLObject* umlobject, UMLObject_Type type) {
	UMLObject* returnObject = NULL;
	if(type == ot_Attribute) {
		UMLClass *umlclass = dynamic_cast<UMLClass *>(umlobject);
		if (umlclass)
			returnObject = createAttribute(umlclass);
	} else if(type == ot_Operation) {
		UMLClassifier *umlclassifier = dynamic_cast<UMLClassifier *>(umlobject);
		if (umlclassifier)
			returnObject = createOperation(umlclassifier);
	} else if(type == ot_Template) {
		UMLClass *umlclass = dynamic_cast<UMLClass *>(umlobject);
		if (umlclass)
			returnObject = createTemplate(umlclass);
	} else if(type == ot_EnumLiteral) {
		UMLEnum* umlenum = dynamic_cast<UMLEnum*>(umlobject);
		if (umlenum) {
			returnObject = createEnumLiteral(umlenum);
		}
	} else {
		kdDebug() << "ERROR UMLDoc::createChildObject type:" << type << endl;
	}
	return returnObject;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createAttribute(UMLClass* umlclass, const QString &name /*=null*/) {
	int id = getUniqueID();
	QString currentName;
	if (name == QString::null)  {
		currentName = umlclass->uniqChildName(Uml::ot_Attribute);
	} else {
		currentName = name;
	}
	const Settings::OptionState optionState = UMLApp::app()->getOptionState();
	Uml::Scope scope = optionState.classState.defaultAttributeScope;
	UMLAttribute* newAttribute = new UMLAttribute(umlclass, currentName, id, "int", scope);

	int button = QDialog::Accepted;
	bool goodName = false;

	//check for name == QString::null stops dialogue being shown
	//when creating attribute via list view
	while (button==QDialog::Accepted && !goodName && name == QString::null) {
		UMLAttributeDialog attributeDialogue(0, newAttribute);
		button = attributeDialogue.exec();
		QString name = newAttribute->getName();

		if(name.length() == 0) {
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		} else if ( umlclass->findChildObject(Uml::ot_Attribute, name).count() > 0 ) {
			KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
		} else {
			goodName = true;
		}
	}

	if (button != QDialog::Accepted) {
		return NULL;
	}

	umlclass->addAttribute(newAttribute);

	// addUMLObject(newAttribute);

	emit sigObjectCreated(newAttribute);
	return newAttribute;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createTemplate(UMLClass* umlclass) {
	int id = getUniqueID();
	QString currentName = umlclass->uniqChildName(Uml::ot_Template);
	UMLTemplate* newTemplate = new UMLTemplate(umlclass, currentName, id);

	int button = QDialog::Accepted;
	bool goodName = false;

	while (button==QDialog::Accepted && !goodName) {
		UMLTemplateDialog templateDialogue(0, newTemplate);
		button = templateDialogue.exec();
		QString name = newTemplate->getName();

		if(name.length() == 0) {
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		} else if ( umlclass->findChildObject(Uml::ot_Template, name).count() > 0 ) {
			KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
		} else {
			goodName = true;
		}
	}

	if (button != QDialog::Accepted) {
		return NULL;
	}

	umlclass->addTemplate(newTemplate);

	emit sigObjectCreated(newTemplate);
	return newTemplate;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createEnumLiteral(UMLEnum* umlenum) {
	int id = getUniqueID();
	QString currentName = umlenum->uniqChildName(Uml::ot_EnumLiteral);
	UMLEnumLiteral* newEnumLiteral = new UMLEnumLiteral(umlenum, currentName, id);

	bool ok = true;
	bool goodName = false;

	while (ok && !goodName) {
		ok = newEnumLiteral->showPropertiesDialogue( UMLApp::app() );
		QString name = newEnumLiteral->getName();

		if(name.length() == 0) {
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		} else {
			goodName = true;
		}
	}

	if (!ok) {
		return NULL;
	}

	umlenum->addEnumLiteral(newEnumLiteral);

	emit sigObjectCreated(newEnumLiteral);
	return newEnumLiteral;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createStereotype(UMLClassifier* classifier, UMLObject_Type list) {
	int id = getUniqueID();
	QString currentName = classifier->uniqChildName(Uml::ot_Stereotype);
	UMLStereotype* newStereotype = new UMLStereotype(classifier, currentName, id, list);

	bool ok = true;
	bool goodName = false;

	while (ok && !goodName) {
		ok = newStereotype->showPropertiesDialogue( UMLApp::app() );
		QString name = newStereotype->getName();

		if(name.length() == 0) {
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		} else {
			goodName = true;
		}
	}

	if (!ok) {
		return NULL;
	}

	classifier->addStereotype(newStereotype, list);

	emit sigObjectCreated(newStereotype);
	return newStereotype;
}

UMLOperation* UMLDoc::createOperation(UMLClassifier* classifier,
				      const QString &name /*=null*/,
				      UMLAttributeList *params  /*=NULL*/)
{
	if(!classifier)
	{
		kdWarning() << "UMLDoc::createOperation called with classifier == NULL"
			    << endl;
		return NULL;
	}
	bool nameNotSet = (name == QString::null || name.isEmpty());
	if (! nameNotSet) {
		UMLOperation *existingOp = classifier->checkOperationSignature(name, params);
		if (existingOp)
			return existingOp;
	}
	UMLOperation *op = new UMLOperation(NULL, name, getUniqueID());
	if (params)
	{
		UMLAttributeListIt it(*params);
		for( ; it.current(); ++it ) {
			UMLAttribute *par = it.current();
			int parID = getUniqueID();
			par->setID(parID);
			op->addParm(par);
		}
	}
	if (nameNotSet || params == NULL) {
		if (nameNotSet)
			op->setName( classifier->uniqChildName(Uml::ot_Operation) );
		do {
			UMLOperationDialog operationDialogue(0, op);
			if( operationDialogue.exec() != QDialog::Accepted ) {
				delete op;
				return NULL;
			} else if (classifier->checkOperationSignature(op->getName(), op->getParmList())) {
				KMessageBox::information(0,
				 i18n("An operation with the same name and signature already exists. You can not add it again."));
			} else {
				break;
			}
		} while(1);
	}

	// operation name is ok, formally add it to the classifier
	classifier->addOperation( op );

	// addUMLObject(newOperation);
	sigObjectCreated(op);
	return op;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::removeAssociation (UMLAssociation * assoc) {

	if(!assoc)
		return;

	removeAssocFromConcepts(assoc);

	// Remove the UMLAssociation from m_objectList.
	UMLObject *object = (UMLObject *) assoc;
	m_objectList.remove(object);

	// I dont believe this appropriate, UMLAssociations ARENT UMLWidgets -b.t.
	// emit sigObjectRemoved(object);

	// so we will save our document
	setModified(true, false);

}

void UMLDoc::removeAssocFromConcepts(UMLAssociation *assoc)
{
	UMLClassifierList concepts = getConcepts();
	for (UMLClassifier *c = concepts.first(); c; c = concepts.next())
		if (c->hasAssociation(assoc))
			c->removeAssociation(assoc);
}

UMLAssociation * UMLDoc::findAssociation(Uml::Association_Type assocType,
					 UMLObject *roleAObj,
					 UMLObject *roleBObj,
					 bool *swap)
{
	UMLAssociationList assocs = getAssociations();
	UMLAssociation *a, *ret = NULL;
	for (a = assocs.first(); a; a = assocs.next()) {
		if (a->getAssocType() != assocType)
			continue;
		if (a->getObject(A) == roleAObj && a->getObject(B) == roleBObj)
			return a;
		if (a->getObject(A) == roleBObj && a->getObject(B) == roleAObj) {
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

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::addAssociation(UMLAssociation *Assoc)
{
	if (Assoc == NULL)
		return;

	// First, check that this association has not already been added.
	// This may happen when loading old XMI files where all the association
	// information was taken from the <UML:AssocWidget> tag.
	UMLAssociationList assocs = getAssociations();
	UMLAssociation *a;
	for (a = assocs.first(); a; a = assocs.next()) {
		// check if its already been added (shouldnt be the case right now
		// as UMLAssociations only belong to one associationwidget at a time right now)
		if (a == Assoc)
		{
			return;
		}
	}

	// If we get here it's really a new association, so lets
	// add it to our concept list and the document.

	// This is the one and only place where the UMLAssociation is assigned
	// its unique ID (unless it was successfully loaded from XMI.)
	if (Assoc->getID() == -1)
		Assoc->setID( getUniqueID() );

	// Add the UMLAssociation at the appropriate concept.
	addAssocToConcepts(Assoc);

	// Add the UMLAssociation in this UMLDoc.
	m_objectList.append( (UMLObject*) Assoc);

	// I dont believe this appropriate, UMLAssociations ARENT UMLWidgets -b.t.
	// emit sigObjectCreated(o);

	setModified(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::addAssocToConcepts(UMLAssociation* a) {

	int AId = a->getRoleId(A);
	int BId = a->getRoleId(B);
	UMLClassifierList concepts = getConcepts();
	for (UMLClassifier *c = concepts.first(); c; c = concepts.next()) {
		switch (a->getAssocType()) {
			// for the next cases should add association to all classes involved
			// in the interaction.
			case Uml::at_Generalization:
			case Uml::at_Realization:
			case Uml::at_Association:
			case Uml::at_Association_Self:
			case Uml::at_UniAssociation:
			case Uml::at_Aggregation:
			case Uml::at_Composition:
				if (AId == c->getID() || (BId == c->getID()))
					c->addAssociation(a);
				break;
			default:  // We don't support any other associations for the moment
				break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLDoc::uniqViewName(const Diagram_Type type) {
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
	else {
		kdWarning() << "uniqViewName() called with unknown diagram type" << endl;
	}
	QString name = dname;
	for (int number = 0; findView(type, name); ++number,
		name = dname + "_" + QString::number(number))
		;
	return name;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::createDiagram(Diagram_Type type, bool askForName /*= true */) {
	bool ok = true;
	QString	name,
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
			UMLView* temp = new UMLView();
			temp -> setOptionState( UMLApp::app()->getOptionState() );
			temp->setName( name );
			temp->setType( type );
			temp->setID( getUniqueID() );
			addView(temp);
			emit sigDiagramCreated(m_uniqueID);
			setModified(true);
			UMLApp::app()->enablePrint(true);
			changeCurrentView(m_uniqueID);
			break;
		} else {
			KMessageBox::error(0, i18n("A diagram is already using that name."), i18n("Not a Unique Name"));
		}
	}//end while
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::renameDiagram(int id) {
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
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::renameUMLObject(UMLObject *o) {
	bool ok = false;
	QString oldName= o->getName();
	while(true) {
		QString name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)UMLApp::app());
		if(!ok)
			break;
		if(name.length() == 0)
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		else if (isUnique(name))  /* o->getBaseType() used to be considered here
					     but I don't think it should be  --okellogg */
		{
			o->setName(name);
			setModified(true);
			break;
		} else {
			KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
		}
	}
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::renameChildUMLObject(UMLObject *o) {
	bool ok = false;
	UMLObject* p = (UMLObject *)o->parent();
	if(!p) {
		kdDebug() << "Can't create object, no parent found" << endl;
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
			if((dynamic_cast<UMLClassifier *>(p)->findChildObject(o->getBaseType(), name)
				.count() == 0)
				|| ((o->getBaseType() == Uml::ot_Operation) && KMessageBox::warningYesNo( kapp -> mainWidget() ,
					i18n( "The name you entered was not unique.\nIs this what you wanted?" ),
					i18n( "Name Not Unique")) == KMessageBox::Yes) ) {
				o->setName(name);
				setModified(true);
				break;
			} else {
				KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::changeCurrentView(int id) {
	UMLView* w = findView(id);
	if (w != m_currentView && w) {
		UMLApp* pApp = UMLApp::app();
		pApp->setCurrentView(w);
		m_currentView = w;
		emit sigDiagramChanged(w->getType());
		pApp->setDiagramMenuItemsState( true );
	}
	emit sigCurrentViewChanged();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::removeDiagram(int id) {
	UMLApp::app()->getDocWindow()->updateDocumentation(true);
	UMLView* umlview = findView(id);
	if(!umlview)
	{
		kdError()<<"Request to remove diagram "<<id<<": Diagram not found!"<<endl;
		return;
	}
	if (KMessageBox::warningContinueCancel(0, i18n("Are you sure you want to delete diagram %1?").arg(umlview->getName()), i18n("Delete Diagram"),KGuiItem( i18n("&Delete"), "editdelete")) == KMessageBox::Continue) {
		removeView(umlview);
		emit sigDiagramRemoved(id);
		setModified(true);
/*		if(infoWidget->isVisible()) {
			emit sigDiagramChanged(dt_Undefined);
			UMLApp::app()->enablePrint(false);
		}
*/ //FIXME sort out all the KActions for when there's no diagram
   //also remove the buttons from the WorkToolBar, then get rid of infowidget
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::removeUMLObject(UMLObject* umlobject) {
	UMLApp::app()->getDocWindow()->updateDocumentation(true);
	UMLObject_Type type = umlobject->getBaseType();

	if (objectTypeIsClassifierListItem(type))  {
		UMLClassifier* parent = (UMLClassifier*)umlobject->parent();
		if (parent == NULL) {
			kdError() << "UMLDoc::removeUMLObject: parent of umlobject is NULL"
				  << endl;
			return;
		}
		emit sigObjectRemoved(umlobject);
		if (type == ot_Operation) {
			parent->removeOperation(static_cast<UMLOperation*>(umlobject));
		} else {
			UMLClass* pClass = dynamic_cast<UMLClass*>(parent);
			if (pClass == NULL)  {
				kdError() << "UMLDoc::removeUMLObject: parent of umlobject has "
					  << "unexpected type " << parent->getBaseType() << endl;
				return;
			}
			if (type == ot_Attribute) {
				pClass->removeAttribute(umlobject);
			} else if (type == ot_Template) {
				pClass->removeTemplate(static_cast<UMLTemplate*>(umlobject));
			} else if (type == ot_Stereotype) {
				pClass->removeStereotype(static_cast<UMLStereotype*>(umlobject));
			} else {
				kdError() << "UMLDoc::removeUMLObject: umlobject has "
					  << "unexpected type " << type << endl;
			}
		}
	} else {
		if (type == ot_Association) {
			// Remove the UMLAssociation at the concept that plays role B.
			UMLAssociation *a = (UMLAssociation *)umlobject;
			Uml::Association_Type assocType = a->getAssocType();
			int AId = a->getRoleId(A);
			int BId = a->getRoleId(B);
			UMLClassifierList concepts = getConcepts();
			for (UMLClassifier *c = concepts.first(); c; c = concepts.next()) {
				switch (assocType) {
					case Uml::at_Generalization:
					case Uml::at_Realization:
						if (AId == c->getID())
							c->removeAssociation(a);
						break;
					case Uml::at_Aggregation:
					case Uml::at_Composition:
						if (BId == c->getID())
							c->removeAssociation(a);
						break;
					case Uml::at_Association:
					case Uml::at_Association_Self:
					case Uml::at_UniAssociation:
						// CHECK: doesnt seem correct
						// But we DO need to remove uni-associations, etc. from the concept, -b.t.
						if (AId == c->getID() || BId == c->getID())
							c->removeAssociation(a);
					default:
						break;
				}
			}
		}
		UMLPackage* pkg = umlobject->getUMLPackage();
		if (pkg)  {
			pkg->removeObject(umlobject);
		}
		emit sigObjectRemoved(umlobject);
		m_objectList.remove(umlobject);
	}
	setModified(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::showProperties(UMLObject* object, int page, bool assoc) {
	UMLApp::app()->getDocWindow()->updateDocumentation( false );
	ClassPropDlg* dialogue = new ClassPropDlg((QWidget*)UMLApp::app(), object, page, assoc);

	bool modified = false;
	if ( dialogue->exec() ) {
		UMLApp::app()->getDocWindow()->showDocumentation(object, true);
		setModified(true);
		modified = true;
	}
	dialogue->close(true);//wipe from memory
	return modified;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::showProperties(ObjectWidget *o) {
	UMLApp::app()->getDocWindow() -> updateDocumentation( false );
	ClassPropDlg *dlg = new ClassPropDlg((QWidget*)UMLApp::app(), o);

	bool modified = false;
	if(dlg->exec()) {
		UMLApp::app()->getDocWindow() -> showDocumentation( o, true );
		setModified(true);
		modified = true;
	}
	dlg -> close(true);//wipe from memory
	return modified;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::signalUMLObjectCreated(UMLObject * o) {
	emit sigObjectCreated(o);
	setModified(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::saveToXMI(QIODevice& file) {
	QDomDocument doc;

	QDomProcessingInstruction xmlHeading =
		doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(xmlHeading);

	QDomElement root = doc.createElement( "XMI" );
	root.setAttribute( "xmi.version", "1.2" );
	root.setAttribute( "timestamp", "");
	root.setAttribute( "verified", "false");
	root.setAttribute( "xmlns:UML", "org.omg/standards/UML");
	doc.appendChild( root );

	QDomElement header = doc.createElement( "XMI.header" );
	QDomElement meta = doc.createElement( "XMI.metamodel" );
	meta.setAttribute( "xmi.name", "UML" );
	meta.setAttribute( "xmi.version", "1.3" );
	meta.setAttribute( "href", "UML.xml" );
	header.appendChild( meta );

	QDomElement model = doc.createElement( "XMI.model" );
	QFile* qfile = dynamic_cast<QFile*>(&file);
	if (qfile) {
		QString modelName = qfile->name();
		modelName = modelName.section('/', -1 );
		modelName = modelName.section('.', 0, 0);
		model.setAttribute( "xmi.name", modelName );
		model.setAttribute( "href", qfile->name() );
	}

	QDomElement documentation = doc.createElement( "XMI.documentation" );

// If we consider it useful we might add user and contact details
//	QDomElement owner = doc.createElement( "XMI.owner" );
//	owner.appendChild( doc.createTextNode( "Jens Krüger" ) ); // Add a User
//	documentation.appendChild( owner );

//	QDomElement contact = doc.createElement( "XMI.contact" );
//	contact.appendChild( doc.createTextNode( "je.krueger@web.de" ) );       // add a contact
//	documentation.appendChild( contact );

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

	header.appendChild( model );
	header.appendChild( meta );
	root.appendChild( header );

	QDomElement content = doc.createElement( "XMI.content" );

	QDomElement objectsElement = doc.createElement( "UML:Model" );

#ifdef XMI_FLAT_PACKAGES
	// Save packages first so that when loading they are known first.
	// This simplifies the establishing of cross reference links from
	// contained objects to their containing package.
	for (UMLObject *p = m_objectList.first(); p; p = m_objectList.next() ) {
		UMLObject_Type t = p->getBaseType();
		if (t != ot_Package)
			continue;
		p->saveToXMI(doc, objectsElement);
	}
#endif

	// Save everything except operations, attributes, and associations.
	// Operations and attributes are owned by classifiers and will show up
	// as their child nodes.
	// Associations are saved in an extra step (see below.)
	for (UMLObject *o = m_objectList.first(); o; o = m_objectList.next() ) {
		UMLObject_Type t = o->getBaseType();
#if defined (XMI_FLAT_PACKAGES)
		if (t == ot_Package)
			continue;
#else
		// Objects contained in a package are already saved by
		// UMLPackage::saveToXMI().
		if (o->getUMLPackage())
			continue;
#endif
		if (t == ot_Association)
			continue;
		if (t == ot_Stereotype || t == ot_Template) {
			kdDebug() << "UMLDoc::saveToXMI(" << o->getName()
				  << "): FIXME: type " << t
				  << " is not supposed to be in m_objectList"
				  << endl;
			continue;
		}
		if (t == ot_EnumLiteral || t == ot_Attribute || t == ot_Operation) {
			kdError() << "UMLDoc::saveToXMI(" << o->getName()
				  << "): internal error: type " << t
				  << " is not supposed to be in m_objectList"
				  << endl;
			continue;
		}
		o->saveToXMI(doc, objectsElement);
	}

	// Save the UMLAssociations.
	// These are saved last so that upon loading, an association's role
	// objects are known beforehand. This simplifies the establishing of
	// cross reference links from the association to its role objects.
	UMLAssociationList alist = getAssociations();
	for (UMLAssociation * a = alist.first(); a; a = alist.next())
		a->saveToXMI(doc, objectsElement);
	content.appendChild( objectsElement );

	root.appendChild( content );

	// Save the XMI extensions: docsettings, diagrams, listview, and codegeneration.
	QDomElement extensions = doc.createElement( "XMI.extensions" );
	extensions.setAttribute( "xmi.extender", "umbrello" );

	QDomElement docElement = doc.createElement( "docsettings" );
	int viewID = -1;
	if( m_currentView )
		viewID = m_currentView -> getID();
	docElement.setAttribute( "viewid", viewID );
	docElement.setAttribute( "documentation", m_Doc );
	docElement.setAttribute( "uniqueid", m_uniqueID );
	extensions.appendChild( docElement );

	// Save each view/diagram.
	QDomElement diagramsElement = doc.createElement( "diagrams" );
	for(UMLView *pView = m_ViewList.first(); pView; pView = m_ViewList.next() )
		pView -> saveToXMI( doc, diagramsElement );
	extensions.appendChild( diagramsElement );

	//  save listview
	UMLApp::app()->getListView() -> saveToXMI( doc, extensions );

	// save code generators
	QDomElement codeGenElement = doc.createElement( "codegeneration" );
	QDictIterator<CodeGenerator> it( m_codeGeneratorDictionary );
	for( ; it.current(); ++it )
		it.current()->saveToXMI ( doc, codeGenElement );
	extensions.appendChild( codeGenElement );

	root.appendChild( extensions );

	QTextStream stream( &file );
	stream.setEncoding(QTextStream::UnicodeUTF8);
	stream << doc.toString();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
		kdWarning()<<"Can't set content: "<<error<<" Line: "<<line<<endl;
		return ENC_UNKNOWN;
	}

	// we start at the beginning and go to the point in the header where we can
	// find out if the file was saved using Unicode
	QDomNode node = doc.firstChild();
	if (node.isProcessingInstruction())
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

	if ( !node.isNull() )
	{
		QDomElement element = node.toElement();

		// check header
		if( !element.isNull() && element.tagName() == "XMI.header" )
		{
			QDomNode headerNode = node.firstChild();
			QDomElement headerElement = headerNode.toElement();
			while ( !headerNode.isNull() )
			{
				// the information if Unicode was used is now stored in the
				// XMI.documenation section of the header
				if (! headerElement.isNull() && headerElement.tagName() ==
							"XMI.documentation")
				{
					QDomNode docuNode = headerNode.firstChild();
					QDomElement docuElement = docuNode.toElement();
					while ( !docuNode.isNull() )
					{
						// a tag XMI.exporterEncoding was added since version 1.2 to
						// mark a file as saved with Unicode
						if (! docuElement.isNull() && docuElement.tagName() ==
									"XMI.exporterEncoding")
						{
							// at the moment this if isn't really neccesary, but maybe
							// later we will have other encoding standards
							if (docuElement.text() == QString("UnicodeUTF8"))
							{
								return ENC_UNICODE; // stop here
							}
						}
						docuNode = docuNode.nextSibling();
						docuElement = docuNode.toElement();
					}
					return ENC_OLD_ENC;
				}
				headerNode = headerNode.nextSibling();
				headerElement = headerNode.toElement();
			}
			return ENC_OLD_ENC;
		}
	} else {
		return ENC_UNKNOWN;
	}
	return ENC_OLD_ENC; // never reached
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
	QString error;
	int line;
	QDomDocument doc;
	if( !doc.setContent( data, false, &error, &line ) ) {
		kdWarning()<<"Can't set content:"<<error<<" Line:"<<line<<endl;
		return false;
	}
	QDomNode node = doc.firstChild();
	//Before Umbrello 1.1-rc1 we didn't add a <?xml heading
	//so we allow the option of this being missing
	if (node.isProcessingInstruction()) {
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
	node = node.firstChild();

	m_nViewID = -1;
	m_highestIDforForeignFile = 0;
	while( !node.isNull() ) {
		QDomElement element = node.toElement();

		if (element.isNull()) {
			kdDebug() << "loadFromXMI: skip empty elem" << endl;
			node = node.nextSibling();
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
				kdDebug() << "UMLDoc::loadFromXMI: skipping <"
					  << outerTag << ">" << endl;
			node = node.nextSibling();
			continue;
		}
		//process content
		QDomNode child = node.firstChild();
		element = child.toElement();
		while( !element.isNull() ) {
			QString tag = element.tagName();
			if (tag == "umlobjects"  // for bkwd compat.
				 || tagEq(tag, "Model") ) {
				if( !loadUMLObjectsFromXMI( element ) ) {
					kdWarning() << "failed load on objects" << endl;
					return false;
				}
			} else if (tagEq(tag, "Package") ||
				   tagEq(tag, "Class") ||
				   tagEq(tag, "Interface")) {
				// These tests are only for foreign XMI files that
				// are missing the <Model> tag (e.g. NSUML)
				QDomElement parentElem = node.toElement();
				if( !loadUMLObjectsFromXMI( parentElem ) ) {
					kdWarning() << "failed load on model objects" << endl;
					return false;
				}
				break;
			} else {
				// for backward compatibility
				loadExtensionsFromXMI(child);
			}
			child = child.nextSibling();
			element = child.toElement();
		}//end while
		node = node.nextSibling();
	}//end while

	if (m_nViewID == -1) {
		m_uniqueID = m_highestIDforForeignFile;
		// We must do this because there is no <docsettings> to
		// tell us the highest ID when loading foreign XMI files.
		kdDebug() << "UMLDoc::loadFromXMI: Setting m_uniqueID to "
			  << m_uniqueID << " because no <docsettings> encountered"
			  << endl;
		// Let's make it the next round number.
		int roundUpFactor = (m_uniqueID < 1000 ? 100 :
				     m_uniqueID < 10000 ? 1000 : 10000);
		m_uniqueID = roundUpFactor * ((m_uniqueID - 1) / roundUpFactor + 1);
	}

	emit sigWriteToStatusBar( i18n("Setting up the document...") );
	m_currentView = NULL;
	activateAllViews();

	if( m_nViewID != -1 && findView( m_nViewID ) ) {
		changeCurrentView( m_nViewID );
	} else {
		createDiagram( Uml::dt_Class, false );
	}
	emit sigResetStatusbarProgress();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::validateXMIHeader(QDomNode& headerNode) {
	QDomElement headerElement = headerNode.toElement();
	while ( !headerNode.isNull() ) {
/*  //Seems older Umbrello files used a different metamodel, so don't validate it for now
  if( !headerElement.isNull() && headerElement.tagName() == "XMI.metamodel" ) {
			QString metamodel = headerElement.attribute("xmi.name", "");
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
////////////////////////////////////////////////////////////////////////////////////////////////////
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

	QDomNode node = element.firstChild();
	QDomElement tempElement = node.toElement();

	while ( !tempElement.isNull() ) {
		QString type = tempElement.tagName();
		if (tagEq(type, "Namespace.ownedElement") ||
		    tagEq(type, "Namespace.contents")) {
			//CHECK: Umbrello currently assumes that nested elements
			// are ownedElements anyway.
			// Therefore the <UML:Namespace.ownedElement> tag is of no
			// significance.
			if( !loadUMLObjectsFromXMI( tempElement ) ) {
				kdWarning() << "failed load on " << type << endl;
				return false;
			}
			node = node.nextSibling();
			tempElement = node.toElement();
			continue;
		} else if (tagEq(type, "name") || tagEq(type, "isSpecification") ||
			   tagEq(type, "isRoot") || tagEq(type, "isLeaf") ||
			   tagEq(type, "isAbstract")) {
			kdDebug() << "UMLDoc::loadUMLObjectsFromXMI: skipping tag "
				  << type << endl;
			node = node.nextSibling();
			tempElement = node.toElement();
			continue;
		}
		UMLObject *pObject = makeNewUMLObject(type);
		if( !pObject ) {
			kdWarning() << "Unknown type of umlobject to create: " << type << endl;
			// We want a best effort, therefore this is handled as a
			// soft error.
			node = node.nextSibling();
			tempElement = node.toElement();
			continue;
		}
		bool status = pObject -> loadFromXMI( tempElement );
		if (tagEq(type, "Association") || tagEq(type, "Generalization")) {
			if ( !status ) {
				// Some interim umbrello versions saved empty UML:Associations,
				// thus we tolerate problems loading them.
				// May happen when dealing with the pre-1.2 file format.
				// In this case all association info is given in the
				// UML:AssocWidget section.  --okellogg
				// removeAssociation((UMLAssociation*)pObject);
				delete pObject;
			} else {
				addAssociation((UMLAssociation*) pObject);
			}
		} else if ( !status ) {
			delete pObject;
			return false;
		}

		/* FIXME see comment at loadUMLObjectsFromXMI
		emit sigSetStatusbarProgress( ++m_count );
		 */
		node = node.nextSibling();
		tempElement = node.toElement();
	}//end while

	return true;
}

void UMLDoc::loadExtensionsFromXMI(QDomNode& node) {
	QDomElement element = node.toElement();
	QString tag = element.tagName();

	if (tag == "docsettings") {
		QString viewID = element.attribute( "viewid", "-1" );
		m_Doc = element.attribute( "documentation", "" );
		QString uniqueid = element.attribute( "uniqueid", "0" );

		m_nViewID = viewID.toInt();
		m_uniqueID = uniqueid.toInt();
		UMLApp::app()->getDocWindow() -> newDocumentation();

	} else if (tag == "diagrams") {
		QDomNode diagramNode = node.firstChild();
		if( !loadDiagramsFromXMI( diagramNode ) ) {
			kdWarning() << "failed load on diagrams" << endl;
		}

	} else if (tag == "listview") {
		if( !UMLApp::app()->getListView() -> loadFromXMI( element ) ) {
			kdWarning() << "failed load on listview" << endl;
		}

	} else if (tag == "codegeneration") {
		QDomNode cgnode = node.firstChild();
		QDomElement cgelement = cgnode.toElement();
		// save for later on
		while( !cgelement.isNull() ) {
			QString nodeName = cgelement.tagName();
			QString lang = cgelement.attribute("language","UNKNOWN");
			m_codeGenerationXMIParamMap->insert(lang, cgelement);
			cgnode = cgnode.nextSibling();
			cgelement = cgnode.toElement();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::makeNewUMLObject(QString type) {
	UMLObject* pObject = 0;
	if (tagEq(type, "UseCase")) {
		pObject = new UMLUseCase();
	} else if (tagEq(type, "Actor")) {
		pObject = new UMLActor();
	} else if (tagEq(type, "Class")) {
		pObject = new UMLClass();
	} else if (tagEq(type, "Package")) {
		pObject = new UMLPackage();
	} else if (tagEq(type, "Component")) {
		pObject = new UMLComponent();
	} else if (tagEq(type, "Node")) {
		pObject = new UMLNode();
	} else if (tagEq(type, "Artifact")) {
		pObject = new UMLArtifact();
	} else if (tagEq(type, "Interface")) {
		pObject = new UMLInterface();
	} else if (tagEq(type, "DataType")
		|| tagEq(type, "Datatype")) {	// for bkwd compat.
		pObject = new UMLDatatype();
	} else if (tagEq(type, "Enumeration") ||
		   tagEq(type, "Enum")) {	// for bkwd compat.
		pObject = new UMLEnum();
	} else if (tagEq(type, "Association")) {
		pObject = new UMLAssociation(Uml::at_Unknown, (UMLObject*)NULL, (UMLObject*) NULL);
	} else if (tagEq(type, "Generalization")) {
		pObject = new UMLAssociation(Uml::at_Generalization, NULL, NULL);
	}
	return pObject;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::loadDiagramsFromXMI( QDomNode & node ) {
	emit sigWriteToStatusBar( i18n("Loading diagrams...") );
	emit sigResetStatusbarProgress();
	emit sigSetStatusbarProgress( 0 );
	emit sigSetStatusbarProgressSteps( 10 ); //FIX ME
	QDomElement element = node.toElement();
	if( element.isNull() )
		return true;//return ok as it means there is no umlobjects
	Settings::OptionState state = UMLApp::app()->getOptionState();
	UMLView * pView = 0;
	int count = 0;
	while( !element.isNull() ) {
		if( element.tagName() == "diagram" ) {
			pView = new UMLView();
			// IMPORTANT: Set OptionState of new UMLView _BEFORE_
			// reading the corresponding diagram:
			// + allow using per-diagram color and line-width settings
			// + avoid crashes due to uninitialized values for lineWidth
			pView -> setOptionState( state );
			if( !pView->loadFromXMI( element ) ) {
				kdWarning() << "failed load on viewdata loadfromXMI" << endl;
				delete pView;
				return false;
			}
			pView -> hide();
			addView( pView );
			emit sigSetStatusbarProgress( ++count );
		}
		node = node.nextSibling();
		element = node.toElement();
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::removeAllViews() {
	for(UMLView *v = m_ViewList.first(); v; v = m_ViewList.next()) {
		v->removeAllAssociations(); // note : It may not be apparent, but when we remove all associations
						// from a view, it also causes any UMLAssociations that lack parent
						// associaiton widgets (but once had them) to remove themselves from
						// this document.
		removeView(v, false);
	}
	m_ViewList.clear();
	m_currentView = NULL;
	emit sigDiagramChanged(dt_Undefined);
	UMLApp::app()->setDiagramMenuItemsState(false);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QStringList UMLDoc::getModelTypes()
{
	QStringList types;
	//insert "standard" (frequently used) types --FIXME, make this language dependant.
	types.append("void");
	types.append("int");
	types.append("long");
	types.append("bool");
	types.append("string");
	types.append("double");
	types.append("float");
	types.append("date");

	//now add the Classes and Interfaces (both are Concepts)
	UMLClassifierList namesList( getConcepts() );
	UMLClassifier* pConcept = 0;
	for(pConcept=namesList.first(); pConcept!=0 ;pConcept=namesList.next())
	{
		types.append( pConcept->getName() );
	}
	return types;
}

UMLClassifierList UMLDoc::getConcepts(bool includeNested /* =true */) {
	UMLClassifierList conceptList;
	for(UMLObject *obj = m_objectList.first(); obj ; obj = m_objectList.next()) {
		Uml::UMLObject_Type ot = obj->getBaseType();
		if(ot == ot_Class || ot == ot_Interface || ot == ot_Datatype || ot == ot_Enum)  {
			conceptList.append((UMLClassifier *)obj);
		} else if (includeNested && ot == ot_Package) {
			UMLPackage *pkg = static_cast<UMLPackage *>(obj);
			pkg->appendClassifiers(conceptList);
		}
	}
	return conceptList;
}

UMLClassifierList UMLDoc::getClassesAndInterfaces(bool includeNested /* =true */) {
	UMLClassifierList conceptList;
	for(UMLObject* obj = m_objectList.first(); obj ; obj = m_objectList.next()) {
		Uml::UMLObject_Type ot = obj->getBaseType();
		if(ot == ot_Class || ot == ot_Interface || ot == ot_Enum)  {
			conceptList.append((UMLClassifier *)obj);
		} else if (includeNested && ot == ot_Package) {
			UMLPackage *pkg = static_cast<UMLPackage *>(obj);
			pkg->appendClassesAndInterfaces(conceptList);
		}
	}
	return conceptList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLInterfaceList UMLDoc::getInterfaces(bool includeNested /* =true */) {
	UMLInterfaceList interfaceList;
	for(UMLObject* obj = m_objectList.first(); obj ; obj = m_objectList.next()) {
		Uml::UMLObject_Type ot = obj->getBaseType();
		if (ot == ot_Interface) {
			interfaceList.append((UMLInterface*)obj);
		} else if (includeNested && ot == ot_Package) {
			UMLPackage *pkg = static_cast<UMLPackage *>(obj);
			pkg->appendInterfaces(interfaceList);
		}
	}
	return interfaceList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLDatatype> UMLDoc::getDatatypes() {
	QPtrList<UMLDatatype> datatypeList;
	for(UMLObject* obj = m_objectList.first(); obj ; obj = m_objectList.next()) {
		if(obj->getBaseType() == ot_Datatype) {
			datatypeList.append((UMLDatatype*)obj);
		}
	}
	return datatypeList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAssociationList UMLDoc::getAssociations() {
	UMLAssociationList associationList;
	for(UMLObject *obj = m_objectList.first(); obj ; obj = m_objectList.next())
		if(obj -> getBaseType() == ot_Association)
			associationList.append((UMLAssociation *)obj);
	return associationList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::print(KPrinter * pPrinter) {
	UMLView * printView = 0;
	int id = -1;
	int count = QString(pPrinter -> option("kde-uml-count")).toInt();
	QPainter painter(pPrinter);
	for(int i = 0;i < count;i++) {
		if(i>0)
			pPrinter -> newPage();
		QString diagram = i18n("kde-uml-Diagram") + QString("%1").arg(i);
		QString sID = pPrinter -> option(diagram);
		id = sID.toInt();
		printView = findView(id);

		if(printView)
			printView ->print(pPrinter, painter);
		printView = 0;
	}
	painter.end();
}
////////////////////////////////////////////////////////////////////////////////////////////////////

bool UMLDoc::showProperties(UMLWidget * o) {
	// will already be selected so make sure docWindow updates the doc
	// back it the widget
	UMLApp::app()->getDocWindow() -> updateDocumentation( false );
	ClassPropDlg *dlg = new ClassPropDlg((QWidget*)UMLApp::app(), o);

	bool modified = false;
	if(dlg->exec()) {
		UMLApp::app()->getDocWindow() -> showDocumentation( o -> getUMLObject() , true );
		setModified(true);
		modified = true;
	}
	dlg -> close(true);//wipe from memory
	return modified;
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
		kdDebug() << "no Obj || Changelog" << endl;
		return false;
	}
	int result = assignNewID(Obj->getID());
	Obj->setID(result);

	//If it is a CONCEPT then change the ids of all its operations and attributes
	if(Obj->getBaseType() == ot_Class ) {

		UMLClassifierListItemList* attributes = ((UMLClass *)Obj)->getAttList();
		for(UMLObject* listItem = attributes->first(); listItem; listItem = attributes->next()) {
			result = assignNewID(listItem->getID());
			listItem->setID(result);
		}

		UMLClassifierListItemList* templates = ((UMLClass *)Obj)->getTemplateList();
		for(UMLObject* listItem = templates->first(); listItem; listItem = templates->next()) {
			result = assignNewID(listItem->getID());
			listItem->setID(result);
		}
	}

	if(Obj->getBaseType() == ot_Interface || Obj->getBaseType() == ot_Class ) {
		UMLClassifierListItemList operations(((UMLClassifier*)Obj)->getOpList());
		for(UMLObject* listItem = operations.first(); listItem; listItem = operations.next()) {
			result = assignNewID(listItem->getID());
			listItem->setID(result);
		}
	}

	setModified(true);

	return true;
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

int UMLDoc::getUniqueID() {
	return ++m_uniqueID;
}

/** Assigns a New ID to an Object, and also logs the assignment to its internal
ChangeLog */
int UMLDoc::assignNewID(int OldID) {
	int result = getUniqueID();
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
		name = viewName + "_" + QString::number(++i);
	}
	if(i) //If name was modified
		pView->setName(name);
	int result = assignNewID(pView->getID());
	pView->setID(result);

	if (!pView->activateAfterLoad( true ) ) {
		kdDebug()<<"Error activating diagram"<<endl;
		return false;
	}
	pView->endPartialWidgetPaste();
	pView->setOptionState( UMLApp::app()->getOptionState() );
	addView(pView);
	setModified(true);
	return true;
}

bool UMLDoc::activateAllViews() {
	bool status = true;
	// store old setting - for restore of last setting
	bool m_bLoading_old = m_bLoading;
	m_bLoading = true; //this is to prevent document becoming modified when activating a view

	for(UMLView *v = m_ViewList.first(); v; v = m_ViewList.next() )
		status = status && v->activateAfterLoad();
	m_bLoading = m_bLoading_old;
	return status;
}

void UMLDoc::settingsChanged(Settings::OptionState optionState) {
	// for each view update settings
	for(UMLView *w = m_ViewList.first() ; w ; w = m_ViewList.next() )
		w -> setOptionState(optionState);
	initSaveTimer();
	return;
}

void UMLDoc::initSaveTimer() {
	if( m_pAutoSaveTimer ) {
		m_pAutoSaveTimer -> stop();
		disconnect( m_pAutoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
		delete m_pAutoSaveTimer;
		m_pAutoSaveTimer = 0;
	}
	Settings::OptionState optionState = UMLApp::app()->getOptionState();
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
		m_modified = true;
	} else {
		// 2004-05-17 Achim Spangler
		KURL orgDocUrl = m_doc_url;
		QString orgFileName = m_doc_url.fileName();
		// don't overwrite manually saved file with autosave content
		QString fileName = tempURL.fileName();
		Settings::OptionState optionState = UMLApp::app()->getOptionState();
		fileName.replace( ".xmi", optionState.generalState.autosavesuffix );
		tempURL.setFileName( fileName );
		// End Achim Spangler

		saveDocument( tempURL );
		// 2004-05-17 Achim Spangler
		// re-activate m_modified if autosave is writing to other file
		// than the main project file -> autosave-suffix != ".xmi"
		if ( ".xmi" != optionState.generalState.autosavesuffix )
			setModified( true );
		// restore original file name -
		// UMLDoc::saveDocument() sets doc_url to filename which is given as autosave-filename
		setURL( orgDocUrl );
		UMLApp * pApp = UMLApp::app();
		pApp->setCaption(orgFileName, isModified() );
		// End Achim Spangler
	}
}

void UMLDoc::signalDiagramRenamed(UMLView * pView ) {
	emit sigDiagramRenamed( pView -> getID() );
	return;
}

void UMLDoc::addToUndoStack() {
	Settings::OptionState optionState = UMLApp::app()->getOptionState();
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
	if (undoStack.count() > 1) {
		int currentViewID = m_currentView->getID();
		// store old setting - for restore of last setting
		bool m_bLoading_old = m_bLoading;
		m_bLoading = true;
		deleteContents();
		redoStack.prepend( undoStack.getFirst() );
		undoStack.removeFirst();
		QDataStream* undoData = undoStack.getFirst();
		QBuffer* buffer = static_cast<QBuffer*>( undoData->device() );
		buffer->open(IO_ReadOnly);
		loadFromXMI(*buffer);
		buffer->close();

		setModified(true, false);
		getCurrentView()->resizeCanvasToItems();
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
		if (m_currentView->getID() != currentViewID) {
			changeCurrentView(currentViewID);
		}
		undoStack.setAutoDelete(false);
	} else {
		kdWarning() << "no data in undostack" << endl;
	}
}

void UMLDoc::loadRedoData() {
	if (redoStack.count() >= 1) {
		int currentViewID = m_currentView->getID();
		// store old setting - for restore of last setting
		bool m_bLoading_old = m_bLoading;
		m_bLoading = true;
		deleteContents();
		undoStack.prepend( redoStack.getFirst() );
		QDataStream* redoData = redoStack.getFirst();
		redoStack.removeFirst();
		QBuffer* buffer = static_cast<QBuffer*>( redoData->device() );
		buffer->open(IO_ReadOnly);
		loadFromXMI(*buffer);
		buffer->close();

		setModified(true, false);
		getCurrentView()->resizeCanvasToItems();
		m_bLoading = m_bLoading_old;

		redoStack.setAutoDelete(true);
		if (redoStack.count() < 1) {
			UMLApp::app()->enableRedo(false);
		}
		if (undoStack.count() > 1) {
			UMLApp::app()->enableUndo(true);
		}
		if (m_currentView->getID() != currentViewID) {
			changeCurrentView(currentViewID);
		}
		redoStack.setAutoDelete(false);
	} else {
		kdWarning() << "no data in redostack" << endl;
	}
}

void UMLDoc::addDefaultDatatypes() {
	UMLApp::app()->getGenerator()->createDefaultDatatypes();
}

void UMLDoc::createDatatype(QString name)  {
	UMLObject* umlobject = findUMLObject(name, ot_Datatype);
	if (!umlobject) {
		createUMLObject(ot_Datatype, name);
	}
	UMLApp::app()->getListView()->closeDatatypesFolder();
}

bool UMLDoc::objectTypeIsClassifierListItem(UMLObject_Type type)  {
	if (type == ot_Attribute || type == ot_Operation || type == ot_Template
	    || type == ot_EnumLiteral || type == ot_Stereotype)  {
		return true;
	} else {
		return false;
	}
}

void UMLDoc::addObject(UMLObject* o)
{
	m_objectList.append(o);
	emit sigObjectCreated(o);
	setModified(true);
}

#include "umldoc.moc"
