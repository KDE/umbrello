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
#include "associationwidgetdata.h"
#include "association.h"
#include "class.h"
#include "package.h"
#include "component.h"
#include "node.h"
#include "artifact.h"
#include "interface.h"
#include "docwindow.h"
#include "objectwidget.h"
#include "operation.h"
#include "attribute.h"
#include "template.h"
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
#include "dialogs/classpropertiespage.h"

#include <qpainter.h>
#include <qtimer.h>
#include <qbuffer.h>
#include <qdir.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <ktempfile.h>

#define FILE_VERSION 5
#define XMI_FILE_VERSION "1.1.5"
static const uint undoMax = 30;

//#include "diagram/diagram.h"
using Umbrello::Diagram;
using Umbrello::DiagramView;




UMLDoc::UMLDoc(QWidget *parent, const char *name) : QObject(parent, name) {
	pViewList = new QPtrList<UMLView>();

	listView = 0;
	currentView = 0;
	uniqueID = 0;
	objectList.clear();
	objectList.setAutoDelete(true);
	diagrams.setAutoDelete(true);

	pViewList->setAutoDelete(true);
	m_pChangeLog = 0;
	m_Doc = "";
	m_modified = false;
	loading = false;
	m_pAutoSaveTimer = 0;
	UMLApp * pApp = UMLApp::app();
	connect(this, SIGNAL(sigDiagramCreated(int)), pApp, SLOT(slotUpdateViews()));
	connect(this, SIGNAL(diagramCreated(Umbrello::Diagram*)), pApp, SLOT(slotUpdateViews()));
	connect(this, SIGNAL(sigDiagramRemoved(int)), pApp, SLOT(slotUpdateViews()));
	connect(this, SIGNAL(sigDiagramRenamed(int)), pApp, SLOT(slotUpdateViews()));
	connect(this, SIGNAL( sigCurrentViewChanged() ), pApp, SLOT( slotCurrentViewChanged() ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLDoc::~UMLDoc() {
	delete m_pChangeLog;
	m_pChangeLog = 0;
	delete pViewList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::addView(UMLView *view) {
	if(listView)
		connect(this, SIGNAL(sigObjectRemoved(UMLObject *)), view, SLOT(slotObjectRemoved(UMLObject *)));
	pViewList->append(view);

	if ( ! loading ) {
		if(currentView == 0) {
			currentView = view;
			view -> show();
			emit sigDiagramChanged(view ->getType());
		} else {
			view -> hide();
		}
	}
	UMLApp * pApp = UMLApp::app();
	pApp->setDiagramMenuItemsState(true);
	pApp->slotUpdateViews();
	pApp->setCurrentView(view);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::removeView(UMLView *view) {
	if(listView) {
		disconnect(this,SIGNAL(sigObjectRemoved(UMLObject *)), view,SLOT(slotObjectRemoved(UMLObject *)));
	}
	view->hide();
	//remove all widgets before deleting view
	//need to do to stop crashes.  These can occur depending on order of
	//children being deleted.
	view->removeAllWidgets();
	pViewList->remove(view);
	if(view == currentView) {
		currentView = 0;
		if (UMLView* firstView = pViewList->first()) {
			changeCurrentView( firstView->getID() );
			UMLApp::app()->setDiagramMenuItemsState(true);
		} else {
			UMLApp::app()->setDiagramMenuItemsState(false);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::setURL(const KURL &url) {
	doc_url = url;
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const KURL& UMLDoc::URL() const {
	return doc_url;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::slotUpdateAllViews(UMLView *sender) {
	if(pViewList) {
		for(UMLView *w = pViewList->first(); w; w = pViewList->next()) {
			if(w != sender) {
				w->repaint();
			}
		}
	}
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::saveModified() {
	bool completed(true);

	if(m_modified) {
		UMLApp *win=(UMLApp *) parent();
		int want_save = KMessageBox::warningYesNoCancel(win, i18n("The current file has been modified.\nDo you want to save it?"), i18n("Warning"));
		switch(want_save) {
			case KMessageBox::Yes:
				if (doc_url.fileName() == i18n("Untitled")) {
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
				};
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
	}
	return completed;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::closeDocument() {
	deleteContents();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::newDocument() {
	deleteContents();
	currentView = 0;
	doc_url.setFileName(i18n("Untitled"));
	//see if we need to start with a new diagram
	SettingsDlg::OptionState optionState = UMLApp::app()-> getOptionState();

	switch( optionState.generalState.diagram ) {
		case SettingsDlg::diagram_usecase:
			createDiagram( Uml::dt_UseCase, false);
			break;

		case SettingsDlg::diagram_class:
			createDiagram( Uml::dt_Class, false );
			break;

		case SettingsDlg::diagram_sequence:
			createDiagram( Uml::dt_Sequence, false );
			break;

		case SettingsDlg::diagram_collaboration:
			createDiagram( Uml::dt_Collaboration, false );
			break;

		case SettingsDlg::diagram_state:
			createDiagram( Uml::dt_State, false );
			break;

		case SettingsDlg::diagram_activity:
			createDiagram( Uml::dt_Activity, false );
			break;

	        case SettingsDlg::diagram_component:
			createDiagram( Uml::dt_Component, false );
			break;

	        case SettingsDlg::diagram_deployment:
			createDiagram( Uml::dt_Deployment, false );
			break;
		default:
			break;
	}//end switch

	setModified(false);
	initSaveTimer();

	UMLApp::app()->enableUndo(false);
	clearUndoStack();
	addToUndoStack();

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::openDocument(const KURL& url, const char */*format =0*/) {
	if(url.fileName().length() == 0) {
		newDocument();
		return false;
	}

	loading = true;

	doc_url = url;
	QDir d = url.path(1);
	deleteContents();
	QString tmpfile;
	KIO::NetAccess::download( url, tmpfile );
	QFile file( tmpfile );
	if ( !file.exists() ) {
		KMessageBox::error(0, i18n("The file %1 does not exist.").arg(d.path()), i18n("Load Error"));
		doc_url.setFileName(i18n("Untitled"));
		loading = false;
		newDocument();
		return false;
	}

	if( !file.open( IO_ReadOnly ) ) {
		KMessageBox::error(0, i18n("There was a problem loading file: %1").arg(d.path()), i18n("Load Error"));
		doc_url.setFileName(i18n("Untitled"));
		loading = false;
		newDocument();
		return false;
	}
	// FIXME: for now only check the file extension... after the filter is tested maybe
	// we should do a better check here
	bool status = false;
	if(file.name().endsWith(".uml"))  //binary file
	{
		QDataStream stream(&file);
		status = serialize(&stream,false, FILE_VERSION);
		if (status) {
			QString newFileName = url.fileName(false);
			doc_url.setFileName(newFileName.replace(newFileName.length() - 4, 4, ".xmi"));
			setModified();
		}
	} else { 	// XML file
		status = loadFromXMI( file );
	}
	file.close();
	KIO::NetAccess::removeTempFile( tmpfile );
	if( !status ) {
		KMessageBox::error(0, i18n("There was a problem loading file: %1").arg(d.path()), i18n("Load Error"));
		loading = false;
		newDocument();
		return false;
	}
	setModified(false);
	loading = false;
	initSaveTimer();

	((UMLApp*)parent())->enableUndo(false);
	clearUndoStack();
	addToUndoStack();

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::saveDocument(const KURL& url, const char * /*format =0*/) {
	doc_url = url;
	QDir d = doc_url.path(1);
	QFile file;
	KTempFile tmpfile;
	bool uploaded = true;
	initSaveTimer();
	if ( url.isLocalFile() )
		file.setName( d.path() );
	else
		file.setName( tmpfile.name() );

	if( !file.open( IO_WriteOnly ) ) {
		KMessageBox::error(0, i18n("There was a problem saving file: %1").arg(d.path()),
				   i18n("Save Error"));
		return false;
	}
	bool status = saveToXMI( file );
	file.close();
	if ( !url.isLocalFile() ) {
		uploaded = KIO::NetAccess::upload( tmpfile.name(), doc_url );
		tmpfile.unlink();
	}
	if (!status ) {
		KMessageBox::error(0, i18n("There was a problem saving file: %1").arg(d.path()), i18n("Save Error"));
		doc_url.setFileName(i18n("Untitled"));
	}
	if( !uploaded ) {
		KMessageBox::error(0, i18n("There was a problem uploading file: %1").arg(d.path()), i18n("Save Error"));
		doc_url.setFileName(i18n("Untitled"));
	}
	setModified(false);
	return (status && uploaded);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::deleteContents() {
	m_Doc = "";
	DocWindow* dw = getDocWindow();
	if (dw) {
		dw->newDocumentation();
	}

	if (listView) {
		listView->init();
		removeAllViews();
		if(objectList.count() > 0) {
			objectList.clear();
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::setupSignals() {
	WorkToolBar *tb = ((UMLApp*)parent()) -> getWorkToolBar();
	connect(this, SIGNAL(sigDiagramCreated(int)), listView, SLOT(slotDiagramCreated(int)));
	connect(this, SIGNAL(diagramCreated(Umbrello::Diagram*)), listView, SLOT( diagramCreated(Umbrello::Diagram*)));
	connect(this, SIGNAL(sigDiagramRemoved(int)), listView, SLOT(slotDiagramRemoved(int)));
	connect(this, SIGNAL(sigChildObjectCreated(UMLObject *)), listView, SLOT(slotChildObjectCreated(UMLObject *)));

	connect(this, SIGNAL(sigDiagramRenamed(int)), listView, SLOT(slotDiagramRenamed(int)));
	connect(this, SIGNAL(sigObjectChanged(UMLObject *)), listView, SLOT(slotObjectChanged(UMLObject *)));
	connect(this, SIGNAL(sigChildObjectChanged(UMLObject *)), listView, SLOT(slotChildObjectChanged(UMLObject *)));
	connect(this, SIGNAL(sigObjectRemoved(UMLObject *)), listView, SLOT(slotObjectRemoved(UMLObject *)));

	connect(this, SIGNAL(sigDiagramChanged(Uml::Diagram_Type)), tb, SLOT(slotCheckToolBar(Uml::Diagram_Type)));
	//new signals below
	connect(this, SIGNAL(sigObjectCreated(UMLObject *)), listView, SLOT(slotObjectCreated(UMLObject *)));
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void UMLDoc::setupListView(UMLListView *lv) {
	listView = lv;
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QWidget* UMLDoc::getMainViewWidget() {
	UMLApp* app = (UMLApp*)parent();
	return app->getMainViewWidget();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLView * UMLDoc::findView(int id) {
	if(pViewList) {
		for(UMLView *w = pViewList->first(); w; w = pViewList->next()) {
			if(w->getID() ==id) {
				return w;
			}
		}
	}
	kdDebug() << "Unable to find a view identified by " << id << endl;
	return 0;
}

Diagram* UMLDoc::findDiagram(int id)
{
	for(Diagram *d = diagrams.first(); d; diagrams.next())
	{
  	if(d->getID() == id)
			return d;
	}
	return 0L;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLView * UMLDoc::findView(Diagram_Type type, QString name) {
	if(pViewList) {
		for(UMLView *w = pViewList->first(); w; w = pViewList->next()) {
			if( (w->getType() == type) && ( w->getName() == name) ) {
				return w;
			}
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::findUMLObject(int id) {
	for(UMLObject * obj = objectList.first(); obj != 0; obj = objectList.next())
		if(obj -> getID() == id)
			return obj;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::findUMLObject(UMLObject_Type type, QString name) {
	for(UMLObject * obj = objectList.first(); obj != 0; obj = objectList.next())
		if(obj -> getName() == name && type == obj -> getBaseType())
			return obj;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLClassifier* UMLDoc::findUMLClassifier(QString name) {
	// could be either UMLClass or UMLInterface..
	UMLObject * obj = findUMLObject(ot_Class, name);
	if (!obj)
		obj = findUMLObject(ot_Interface, name);
	return dynamic_cast<UMLClassifier*>(obj);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString	UMLDoc::uniqObjectName(const UMLObject_Type type) {
	QString	currentName;
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
	else
		currentName = i18n("new_object");

	QString name = currentName;
	for (int number = 0; findUMLObject(type, name); ++number,
	        name = currentName + "_" + QString::number(number))
		;
	return name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createUMLObject(const std::type_info &type)
{
//adapter.. just transform and forward request
	UMLObject_Type t;
	if( type == typeid(UMLClass) )
	{
		t = ot_Class;
	}
	else if ( type == typeid(UMLUseCase) )
	{
		t = ot_UseCase;
	}
	else if ( type == typeid(UMLActor) )
	{
		t = ot_Actor;
	}
	else if ( type == typeid(UMLPackage) )
	{
		t = ot_Package;
	}
	else if ( type == typeid(UMLComponent) )
	{
		t = ot_Component;
	}
	else if ( type == typeid(UMLNode) )
	{
		t = ot_Node;
	}
	else if ( type == typeid(UMLArtifact) )
	{
		t = ot_Artifact;
	}
	else if ( type == typeid(UMLInterface) )
	{
		t = ot_Interface;
	}
	else
	{
		return static_cast<UMLObject*>(0L);
	}
	return createUMLObject(t);
}
UMLObject* UMLDoc::createUMLObject(UMLObject_Type type, const QString &n) {
	bool ok = false;
	int id;
	UMLObject *o = 0L;
	QString name;
	if( n.length() != 0 && !(o = findUMLObject(type,n)) )
	{
		name = n;
	}
	else
	{
		name = uniqObjectName(type);
		do {
		name = KLineEditDlg::getText(i18n("Enter name:"), name, &ok, (QWidget*)parent());
		if (!ok) {
			return 0;
		}
		if (name.length() == 0) {
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
			continue;
		}
		o = findUMLObject(type, name);
		if (o) {
			KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
		}
		}while( name.length() == 0 || o != 0L );
	}

	id = getUniqueID();
	if(type == ot_Actor) {
		o = new UMLActor(this, name, id);
	} else if(type == ot_UseCase) {
		o = new UMLUseCase(this,name, id);
	} else if(type == ot_Class ) {
		o = new UMLClass (this, name, id);
	} else if(type == ot_Package) {
		o = new UMLPackage(this, name, id);
	} else if(type == ot_Component) {
		o = new UMLComponent(this, name, id);
	} else if(type == ot_Node) {
		o = new UMLNode(this, name, id);
	} else if(type == ot_Artifact) {
		o = new UMLArtifact(this, name, id);
	} else if(type == ot_Interface) {
		o = new UMLInterface(this, name, id);
	} else {
		kdWarning() << "CreateUMLObject(int) error" << endl;
		return (UMLObject*)0L;
	}
	objectList.append(o);
	emit sigObjectCreated(o);
	setModified(true);
	return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createUMLObject(UMLObject* umlobject, UMLObject_Type type) {
	if(type == ot_Attribute) {
		return createAttribute(umlobject);
	} else if(type == ot_Operation) {
		return createOperation(umlobject);
	} else if(type == ot_Template) {
		return createTemplate(umlobject);
	} else {
		kdDebug() << "ERROR _CREATEUMLOBJECT" << endl;
		return NULL;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createAttribute(UMLObject* umlobject) {
	int id = getUniqueID();
	QString currentName = dynamic_cast<UMLClass *>(umlobject)->uniqChildName(Uml::ot_Attribute);
	UMLAttribute* newAttribute = new UMLAttribute(umlobject, currentName, id);

	int button = QDialog::Accepted;
	bool goodName = false;

	while (button==QDialog::Accepted && !goodName) {
		UMLAttributeDialog attributeDialogue(0, newAttribute);
		button = attributeDialogue.exec();
		QString name = newAttribute->getName();

		if(name.length() == 0) {
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		} else if ( ((UMLClass*)umlobject)->findChildObject(Uml::ot_Attribute, name).count() > 0 ) {
			KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
		} else {
			goodName = true;
		}
	}

	if (button != QDialog::Accepted) {
		return NULL;
	}

	((UMLClass*)umlobject)->addAttribute((UMLAttribute*)newAttribute);

	setModified(true);
	emit sigChildObjectCreated(newAttribute);
	emit sigWidgetUpdated(umlobject);
	return newAttribute;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createTemplate(UMLObject* umlobject) {
	int id = getUniqueID();
	QString currentName = dynamic_cast<UMLClass*>(umlobject)->uniqChildName(Uml::ot_Template);
	UMLTemplate* newTemplate = new UMLTemplate(umlobject, currentName, id);

	int button = QDialog::Accepted;
	bool goodName = false;

	while (button==QDialog::Accepted && !goodName) {
		UMLTemplateDialog templateDialogue(0, newTemplate);
		button = templateDialogue.exec();
		QString name = newTemplate->getName();

		if(name.length() == 0) {
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		} else if ( ((UMLClass*)umlobject)->findChildObject(Uml::ot_Template, name).count() > 0 ) {
			KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
		} else {
			goodName = true;
		}
	}

	if (button != QDialog::Accepted) {
		return NULL;
	}

	((UMLClass*)umlobject)->addTemplate((UMLTemplate*)newTemplate);

	setModified(true);
	emit sigChildObjectCreated(newTemplate);
	emit sigWidgetUpdated(umlobject);
	return newTemplate;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::createOperation(UMLObject* umlobject) {
	UMLOperation* newOperation = 0;
	int id = getUniqueID();
	QString currentName;
	if (umlobject->getBaseType() == ot_Class || umlobject->getBaseType() == ot_Interface) {
		currentName = static_cast<UMLClassifier*>(umlobject)->uniqChildName(Uml::ot_Operation);
	} else {
		kdDebug() << "creating operation for something which isn't a class or an interface" << endl;
	}
	newOperation = new UMLOperation(umlobject, currentName, id);

	int button = QDialog::Accepted;
	bool goodName = false;

	while (button==QDialog::Accepted && !goodName) {
		UMLOperationDialog operationDialogue(0, newOperation);
		button = operationDialogue.exec();
		QString name = newOperation->getName();

		if(name.length() == 0) {
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		} else {
			goodName = true;
		}
	}

	if (button != QDialog::Accepted) {
		return NULL;
	}

	if (umlobject->getBaseType() == ot_Class || umlobject->getBaseType() == ot_Interface ) {
		((UMLClassifier*)umlobject)->addOperation((UMLOperation*)newOperation);
	} else {
		kdWarning() << "creating operation for something which isn't a class or an interface" << endl;
	}

	setModified(true);
	emit sigChildObjectCreated(newOperation);
	emit sigWidgetUpdated(umlobject);
	return newOperation;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
		/*
void UMLDoc::removeAssociation(Association_Type assocType, int AId, int BId) {
	removeAssocFromConcepts(assoc);
	UMLObject *object = NULL;
	for (UMLObject *o = objectList.first(); o; o = objectList.next()) {
		if (o -> getBaseType() != ot_Association)
			continue;
		UMLAssociation *a = (UMLAssociation *)o;
		if (a->getAssocType() != assocType ||
		    a->getRoleAId() != AId || a->getRoleBId() != BId) {
			continue;
		}
		// Remove the UMLAssociation at the concept that plays role B.
		QPtrList<UMLClassifier> concepts = getConcepts();
		for (UMLClassifier *c = concepts.first(); c; c = concepts.next())
			if (BId == c->getID())
				c->removeAssociation(a);
		object = o;
	}
	if (object == NULL)
		return;
	// Remove the UMLAssociation in this UMLDoc.
	emit sigObjectRemoved(object);
	objectList.remove(object);
	setModified(true);
}
	*/
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::removeAssociation (UMLAssociation * assoc) {

	if(!assoc)
		return;

	removeAssocFromConcepts(assoc);

	// Remove the UMLAssociation in this UMLDoc objectList.
	UMLObject *object = (UMLObject *) assoc;
	objectList.remove(object);

	// I dont believe this appropriate, UMLAssociations ARENT UMLWidgets -b.t.
	// emit sigObjectRemoved(object);

	setModified(true);
}

void UMLDoc::removeAssocFromConcepts(UMLAssociation *assoc)
{
	QPtrList<UMLClassifier> concepts = getConcepts();
	for (UMLClassifier *c = concepts.first(); c; c = concepts.next())
		if (c->hasAssociation(assoc))
			c->removeAssociation(assoc);
}

UMLAssociation* UMLDoc::createUMLAssociation(UMLObject *a, UMLObject *b, Uml::Association_Type type)
{
	UMLAssociation *assoc = new UMLAssociation( this );
	assoc->setAssocType(type);
	assoc->setObjectA(a);
	assoc->setObjectB(b);
	addAssociation(assoc);
	return assoc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::addAssociation(UMLAssociation *Assoc)
{
	// First, check that this association has not already been added.
	// This may happen as long as we are still in transition from the old
	// widget based association fabrication. (See explanation at method
	// addAssocInViewAndDoc() in file umlview.h.)
	QPtrList<UMLAssociation> assocs = getAssociations();
	UMLAssociation *a;
	for (a = assocs.first(); a; a = assocs.next()) {
		// check if its already been added (shouldnt be the case right now
		// as UMLAssociations only belong to one associaitonwidget at a time right now)
		if (a == Assoc)
		{
			return;
		}
	}

	// If we get here it's really a new association, so lets
	// add it to our concept list and the document.

	// Add the UMLAssociation at the appropriate concept.
	addAssocToConcepts(Assoc);

	// Add the UMLAssociation in this UMLDoc.
	objectList.append( (UMLObject*) Assoc);

	// I dont believe this appropriate, UMLAssociations ARENT UMLWidgets -b.t.
	// emit sigObjectCreated(o);

	setModified(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::addAssocToConcepts(UMLAssociation* a) {
	int AId = a->getRoleAId();
	int BId = a->getRoleBId();
	QPtrList<UMLClassifier> concepts = getConcepts();
	for (UMLClassifier *c = concepts.first(); c; c = concepts.next()) {
		switch (a->getAssocType()) {
			// for the next cases should add association to all classes involved
			// in the interaction.
			case Uml::at_Generalization:
			case Uml::at_Association:
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


Umbrello::Diagram* UMLDoc::UcreateDiagram(Diagram::DiagramType dType, const QString& name)
{
	int id = getUniqueID();
	Diagram *diagram = new Diagram(dType, this, id, name);
	diagrams.append(diagram);
	emit diagramCreated(diagram);
	return diagram;

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::createDiagram(Diagram_Type type, bool askForName /*= true */) {
	bool 	ok = true;
	QString	name,
	dname = uniqViewName(type);

	while(true) {
		if( askForName )
			name = KLineEditDlg::getText(i18n("Enter name:"), dname, &ok, (QWidget*)parent());
		else
			name = dname;
		if(!ok)
			break;
		if(name.length() == 0)
			KMessageBox::error(0, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
		else if(!findView(type, name)) {
			UMLViewData * pData = new UMLViewData();
			pData -> setName( name );
			pData -> setType( type );
			pData -> setID( ++uniqueID );
			UMLView* temp = new UMLView(UMLApp::app()->getMainViewWidget(), pData, this);
			addView(temp);
			temp -> setOptionState( ((UMLApp *) parent()) -> getOptionState() );
			emit sigDiagramCreated(uniqueID);
			setModified(true);
			((UMLApp*)parent())->enablePrint(true);
			changeCurrentView(uniqueID);
			break;
		} else
			KMessageBox::error(0, i18n("A diagram is already using that name."), i18n("Not a Unique Name"));
	}//end while
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::renameDiagram(int id) {
	bool ok = false;

	UMLView *temp =  findView(id);
	Diagram_Type type = temp->getType();

	QString oldName= temp->getName();
	while(true) {
		QString name = KLineEditDlg::getText(i18n("Enter name:"), oldName, &ok, (QWidget*)parent());

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
		QString name = KLineEditDlg::getText(i18n("Enter name:"), oldName, &ok, (QWidget*)parent());
		if(!ok)
			break;
		if(name.length() == 0)
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		else if(!findUMLObject(o->getBaseType(), name)) {
			o->setName(name);
			emit sigObjectChanged(o);
			emit sigWidgetUpdated(o);
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
		QString name = KLineEditDlg::getText(i18n("Enter name:"), oldName, &ok, (QWidget*)parent());
		if(!ok)
			break;
		if(name.length() == 0)
			KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
		else {
			if((dynamic_cast<UMLClassifier *>(p)->findChildObject(o->getBaseType(), name)
			        .count() == 0)
			        || ((o->getBaseType() == Uml::ot_Operation) && KMessageBox::warningYesNo( kapp -> mainWidget() ,
			                i18n( "The name you entered was not unique!\nIs this what you wanted?" ),
			                i18n( "Name Not Unique")) == KMessageBox::Yes) ) {
				o->setName(name);
				emit sigChildObjectChanged(o);
				emit sigWidgetUpdated(p);
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
	if (w != currentView && w) {
		UMLApp* pApp = UMLApp::app();
		pApp->setCurrentView(w);
		currentView = w;
		emit sigDiagramChanged(w->getType());
		pApp->setDiagramMenuItemsState( true );
	}
	emit sigCurrentViewChanged();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::removeDiagram(int id) {
	getDocWindow()->updateDocumentation(true);
	UMLView* umlview = findView(id);
	if (KMessageBox::warningYesNo(0, i18n("Are you sure you want to delete diagram %1?").arg(umlview->getName()), i18n("Delete Diagram")) == KMessageBox::Yes) {
		removeView(umlview);
		emit sigDiagramRemoved(id);
		setModified(true);
/*		if(infoWidget->isVisible()) {
			emit sigDiagramChanged(dt_Undefined);
			((UMLApp*)parent())->enablePrint(false);
		}
*/ //FIXME sort out all the KActions for when there's no diagram
   //also remove the buttons from the WorkToolBar, then get rid of infowidget
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::removeUMLObject(UMLObject *o) {
	getDocWindow() -> updateDocumentation( true );
	UMLObject_Type type = o->getBaseType();
 	if (type >= ot_Actor && type <= ot_Association) {
 		if (type == ot_Association) {
 			// Remove the UMLAssociation at the concept that plays role B.
 			UMLAssociation *a = (UMLAssociation *)o;
 			Uml::Association_Type assocType = a->getAssocType();
 			int AId = a->getRoleAId();
 			int BId = a->getRoleBId();
 			QPtrList<UMLClassifier> concepts = getConcepts();
 			for (UMLClassifier *c = concepts.first(); c; c = concepts.next()) {
 				switch (assocType) {
 					case Uml::at_Generalization:
 						if (AId == c->getID())
 							c->removeAssociation(a);
 						break;
 					case Uml::at_Aggregation:
 					case Uml::at_Composition:
 						if (BId == c->getID())
 							c->removeAssociation(a);
 						break;
 						/*
 					case Uml::at_Association:
 						// CHECK: doesnt seem correct
 						if (AId == c->getID() || BId == c->getID())
 							c->removeAssociation(a);
 							*/
 					default:
 						break;
 				}
 			}
 		}
		emit sigObjectRemoved(o);
		objectList.remove(o);
		setModified(true);
		return;
	}
	//must be att or op
	UMLClassifier *p = (UMLClassifier*)o->parent();
	emit sigObjectRemoved(o);
	if (type == ot_Operation) {
		p->removeOperation(o);
		delete o;
	} else if (type == ot_Attribute) {
		UMLClass *pClass = dynamic_cast<UMLClass *>(p);
		if(pClass)
			pClass->removeAttribute(o);
	} else if (type == ot_Template) {
		UMLClass *pClass = dynamic_cast<UMLClass *>(p);
		if(pClass)
			pClass->removeTemplate((UMLTemplate*)o);
	}
	emit sigWidgetUpdated(p);

	setModified(true);

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::showProperties(UMLObject* object, int page, bool assoc) {
	getDocWindow()->updateDocumentation( false );
	ClassPropDlg* dialogue = new ClassPropDlg((QWidget*)parent(), object, page, assoc);

	if ( dialogue->exec() ) {
		getDocWindow()->showDocumentation(object, true);
		emit sigWidgetUpdated(object);
		emit sigObjectChanged(object);
		setModified(true);
	}
	dialogue->close(true);//wipe from memory
/*
	if(typeid(*object) == typeid(UMLClassifier))
	{
	kdDebug()<<"showing props for class"<<endl;
	ClassPropertiesPage *p = new ClassPropertiesPage(dynamic_cast<UMLClassifier*>(object),0L,"class page" );
	p->show();
	}
	else
	{
	kdDebug()<<"object is of type "<<typeid(*object).name()<<" and not of type "<<typeid(UMLClassifier).name()<<endl;
	}
*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::showProperties(ObjectWidget *o) {
	getDocWindow() -> updateDocumentation( false );
	ClassPropDlg *dlg = new ClassPropDlg((QWidget*)parent(), o);

	if(dlg->exec()) {
		getDocWindow() -> showDocumentation( o, true );
		UMLObject * object = o -> getUMLObject();
		emit sigWidgetUpdated(object);
		emit sigObjectChanged(object);
		setModified(true);
	}
	dlg -> close(true);//wipe from memory
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::signalChildUMLObjectUpdate(UMLObject *o) {
	UMLObject *p = (UMLObject *)o->parent();
	emit sigChildObjectChanged(o);
	emit sigWidgetUpdated(p);
	setModified(true);
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::signalChildUMLObjectCreated(UMLObject *o) {
	UMLObject *p = (UMLObject *)o->parent();
	emit sigChildObjectCreated(o);
	emit sigWidgetUpdated(p);
	setModified(true);
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::signalUMLObjectCreated(UMLObject * o) {
	emit sigObjectCreated(o);
	setModified(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::signalUMLObjectChanged(UMLObject * o) {
	emit sigObjectChanged(o);
	emit sigWidgetUpdated(o);
	setModified(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::serialize(QDataStream *s, bool archive, int fileversion) {
	bool status = true;
	if(archive) {
		int viewID = -1;
		//int i = 0;
		if(currentView)
			viewID = currentView -> getID();
		getDocWindow() -> updateDocumentation( false );
		//save current file
		*s << 0xabcd 	//magic number
		<< fileversion	//version
		<< uniqueID
		<< viewID
		<< m_Doc;
		//save each UMLObject
		UMLObject * obj;
		*s << objectList.count();
		for(obj=objectList.first();obj!=0;obj=objectList.next()) {
			status = obj -> serialize(s, archive, fileversion);
			if(!status)
				return status;
		}

		//save each view/diagram
		*s << pViewList -> count();

		UMLView *w;
		for(w=pViewList->first(); w!=0; w=pViewList->next()) {
			w->getID();
			status = w -> serialize(s, archive, fileversion);
			if(!status)
				return status;


		}

		listView -> serialize(s, archive, fileversion);
	}//end save
	else {

		listView -> setLoading(true);
		int magic, count, type, viewID;

		UMLObject * temp;
		int steps;

		loading = true;
		//make sure right file type;
		*s >> magic;
		if(magic != 0xabcd)
			return false;
		*s >> version;
		fileversion = version;
		*s >> uniqueID >> viewID;
		if (version > 2)
			listView -> setLoading(true);
		if (version > 4)
			*s >> m_Doc;
		//load UMLObjects
		*s >> count;
		getDocWindow() -> newDocumentation();
		for(int i=0;i<count;i++) {
			*s >> type;
                                UMLUseCase * uc = new UMLUseCase(this);
                                temp = (UMLObject *)uc;

			if((version > 4 && type == ot_Actor)
				|| (version < 5 && type == /* ot_Actor */ 100)) {
				UMLActor * a = new UMLActor(this);
				temp = (UMLObject *)a;
			} else if((version > 4 && type == ot_UseCase)
				|| (version < 5 && type == /* ot_UseCase */ 101)) {
				UMLUseCase * uc = new UMLUseCase(this);
				temp = (UMLObject *)uc;
			} else if((version > 4 && type == ot_Class)
				|| (version < 5 && type == /*ot_Concept */ 102)) {
				UMLClass * c = new UMLClass(this);
				temp = (UMLObject *)c;
			} else if(version > 4 && type == ot_Interface) {
				UMLInterface * c = new UMLInterface(this);
				temp = (UMLObject *)c;
			} else
				return false;
			status = temp -> serialize(s, archive, fileversion);
			if(!status)
				return status;
			objectList.append(temp);
		}//end for i

		this->emit sigWriteToStatusBar( i18n("Loading Diagrams...") );
		//load each views/diagrams
		*s >> count;
		steps = count - 1;
		//Show the progress of the load of diagrams
		this->emit sigResetStatusbarProgress();
		this->emit sigSetStatusbarProgress( 0 );
		this->emit sigSetStatusbarProgressSteps( steps );

		viewsNotActivated.clear();
		UMLApp* app = (UMLApp*)parent();
		SettingsDlg::OptionState state =  app -> getOptionState();
		for(int i=0;i<count;i++) {
			UMLView * v = new UMLView(UMLApp::app()->getMainViewWidget(), new UMLViewData(), this);
			v -> hide();
			v -> setOptionState( state );
			status = v -> serialize(s, archive, fileversion);

			if ( !status )
				return status;

			addView(v);
			viewsNotActivated.append(v);
			this-> emit sigSetStatusbarProgress( i );
		}//end for

		this->emit sigResetStatusbarProgress();
		this->emit sigWriteToStatusBar( i18n("Loading UML elements...") );


		if(version > 4 )
		{
			listView -> setLoading(false);
			listView -> serialize(s, archive, fileversion);
		}
		else
		{
			listView -> setLoading(false);
			for(UMLView *v = pViewList->first(); v ; v = pViewList->next())
				emit sigDiagramCreated(v->getData()->getID());

			for(UMLObject *o = objectList.first(); o ; o = objectList.next())
			{
				emit sigObjectCreated(o);
				if (o->getBaseType() == Uml::ot_Class)
				{
					QPtrList<UMLOperation> *opList = dynamic_cast<UMLClassifier *>(o)->getOpList();
					for (UMLOperation *op = opList->first(); op; op = opList->next())
						emit sigChildObjectCreated(op);

					QPtrList<UMLAttribute> *attList = dynamic_cast<UMLClass *>(o)->getAttList();
					for (UMLAttribute *att = attList->first(); att; att = attList->next())
						emit sigChildObjectCreated(att);

					QPtrList<UMLTemplate>* templateList = dynamic_cast<UMLClass *>(o)->getTemplateList();
					for (UMLTemplate* theTemplate = templateList->first(); theTemplate;
					     theTemplate = templateList->next())
						emit sigChildObjectCreated(theTemplate);
				}
			}
		}

		loading = false;
		this->emit sigWriteToStatusBar( i18n("Setting up the document...") );
		//activate all diagrams after load to make sure all widgets get the correct ids they need
		currentView = 0;
		activateAllViews();
		if( findView( viewID ) )
			changeCurrentView( viewID );
	}//end else
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::saveToXMI(QIODevice& file) {
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
	header.appendChild( documentation );

	header.appendChild( model );
        header.appendChild( meta );
	root.appendChild( header );

	QDomElement content = doc.createElement( "XMI.content" );
	bool status=true;
	QDomElement docElement = doc.createElement( "docsettings" );
	int viewID = -1;
	if( currentView )
		viewID = currentView -> getID();
	docElement.setAttribute( "viewid", viewID );
	docElement.setAttribute( "documentation", m_Doc );
	docElement.setAttribute( "uniqueid", uniqueID );
	content.appendChild( docElement );
	//  save each UMLObject
	QDomElement objectsElement = doc.createElement( "umlobjects" );
	for(UMLObject *pObject = objectList.first(); pObject && status; pObject = objectList.next() )
		status = pObject -> saveToXMI( doc, objectsElement );
	content.appendChild( objectsElement );
	if( !status )
		return status;
	//  save each view/diagram
	QDomElement diagramsElement = doc.createElement( "diagrams" );
	for(UMLView *pView = pViewList->first(); pView && status; pView = pViewList->next() )
		status = pView -> saveToXMI( doc, diagramsElement );
	content.appendChild( diagramsElement );
	if( !status )
		return status;
	//  save listview
	status = listView -> saveToXMI( doc, content );

	root.appendChild( content );
	QTextStream stream( &file );
	stream << doc.toString();
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::loadFromXMI( QIODevice & file ) {
	QTextStream stream( &file );
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

	int nViewID = -1;
	bool cont = true;
	while( !node.isNull() && cont ) {
		QDomElement element = node.toElement();

		//check header
		if( !element.isNull() && element.tagName() == "XMI.header" ) {
			QDomNode headerNode = node.firstChild();
			if ( !validateXMIHeader(headerNode) ) {
				return false;
			}
		}
		if( !element.isNull() && element.tagName() == "XMI.content" ) {
			node = node.firstChild();
			element = node.toElement();
			while( !element.isNull() ) {
				QString tag = element.tagName();
				if( tag == "docsettings" ) {
					QString viewID = element.attribute( "viewid", "-1" );
					m_Doc = element.attribute( "documentation", "" );
					QString uniqueid = element.attribute( "uniqueid", "0" );

					nViewID = viewID.toInt();
					uniqueID = uniqueid.toInt();
					getDocWindow() -> newDocumentation();
				} else if( tag == "umlobjects" ) {
					QDomNode objectNode = node.firstChild();
					if( !loadUMLObjectsFromXMI( objectNode ) ) {
						kdWarning() << "failed load on objects" << endl;
						return false;
					}
				} else if( tag == "diagrams" ) {
					QDomNode diagramNode = node.firstChild();

					if( !loadDiagramsFromXMI( diagramNode ) ) {
						kdWarning() << "failed load on diagrams" << endl;
						return false;
					}
				} else if( tag == "listview" ) {
					if( !listView -> loadFromXMI( element ) ) {
						kdWarning() << "failed load on listview" << endl;
						return false;
					}
				}
				node = node.nextSibling();
				element = node.toElement();
			}//end while
			break;
		}//end if docsettings
		else {
			node = node.nextSibling();
			element = node.toElement();
		}
	}//end while
	emit sigWriteToStatusBar( i18n("Setting up the document...") );
	currentView = 0;
	activateAllViews();

	if( findView( nViewID ) ) {
		changeCurrentView( nViewID );
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
bool UMLDoc::loadUMLObjectsFromXMI( QDomNode & node ) {
	QDomElement element = node.toElement();
	if( element.isNull() )
		return true;//return ok as it means there is no umlobjects
	emit sigResetStatusbarProgress();
	emit sigSetStatusbarProgress( 0 );
	emit sigSetStatusbarProgressSteps( 10 ); //FIX ME
	emit sigWriteToStatusBar( i18n("Loading UML elements...") );
	UMLObject * pObject = 0;
	int count = 0;

	while ( !element.isNull() ) {
		pObject = 0;
		QString type = element.tagName();
		if (type != "UML:Association") {
 			//For the time being, we skip loading asociations from
 			// here. Instead, we will get them from the association widgets.
 			// meaning that UML:Association nodes are effectively ignored
 			// for the nonce.
			pObject = makeNewUMLObject(type);
			if( !pObject ) {
				kdWarning() << "Given wrong type of umlobject to create:" << type << endl;
				return false;
			}
			if( !pObject -> loadFromXMI( element ) ) {
				return false;
			}
			objectList.append( pObject );
		}
		emit sigSetStatusbarProgress( ++count );
		node = node.nextSibling();
		element = node.toElement();
	}//end while
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::makeNewClassifierObject(QString type, QDomElement& element) {
	UMLObject* pObject = 0;
	if (type == "UML:Operation") {
		UMLOperation* newOperation = new UMLOperation(this);
		if( !newOperation->loadFromXMI(element) ) {
			return false;
		}
		pObject = newOperation;
	} else if (type == "UML:Attribute") {
		UMLAttribute* newAttribute = new UMLAttribute(this);
		if( !newAttribute->loadFromXMI(element) ) {
			return false;
		}
		pObject = newAttribute;
	} else if (type == "template") {
		UMLTemplate* newTemplate = new UMLTemplate(this);
		if ( !newTemplate->loadFromXMI(element) ) {
			return false;
		}
		pObject = newTemplate;
	}
	return pObject;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::makeNewUMLObject(QString type) {
	UMLObject* pObject = 0;
	if (type == "UML:UseCase") {
		pObject = new UMLUseCase(this);
	} else if (type == "UML:Actor") {
		pObject = new UMLActor(this);
	} else if (type == "UML:Class") {
		pObject = new UMLClass(this);
	} else if (type == "UML:Package") {
		pObject = new UMLPackage(this);
	} else if (type == "UML:Component") {
		pObject = new UMLComponent(this);
	} else if (type == "UML:Node") {
		pObject = new UMLNode(this);
	} else if (type == "UML:Artifact") {
		pObject = new UMLArtifact(this);
	} else if (type == "UML:Interface") {
		pObject = new UMLInterface(this);
	} else if (type == "UML:Association") {
		pObject = new UMLAssociation(this);
	}
	return pObject;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::loadDiagramsFromXMI( QDomNode & node ) {
	emit sigWriteToStatusBar( i18n("Loading Diagrams...") );
	emit sigResetStatusbarProgress();
	emit sigSetStatusbarProgress( 0 );
	emit sigSetStatusbarProgressSteps( 10 ); //FIX ME
	QDomElement element = node.toElement();
	if( element.isNull() )
		return true;//return ok as it means there is no umlobjects
	UMLApp* app = (UMLApp*)parent();
	SettingsDlg::OptionState state =  app -> getOptionState();
	UMLView * pView = 0;
	int count = 0;
	while( !element.isNull() ) {
		if( element.tagName() == "diagram" ) {
			pView = new UMLView(UMLApp::app()->getMainViewWidget(), new UMLViewData(), this);
			pView -> hide();
			pView -> setOptionState( state );
			if( !pView -> getData() -> loadFromXMI( element ) ) {
				kdWarning() << "failed load on viewdata loadfromXMI" << endl;
				return false;
			}
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
	for(UMLView *v = pViewList->first(); v; v = pViewList->next())
		v->removeAllAssociations();
	pViewList -> clear();
	currentView = 0;
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
	QPtrList<UMLClassifier> namesList( getConcepts() );
	UMLClassifier* pConcept = 0;
	for(pConcept=namesList.first(); pConcept!=0 ;pConcept=namesList.next())
	{
		types.append( pConcept->getName() );
	}
	return types;
}

QPtrList<UMLClassifier> UMLDoc::getConcepts() {
	QPtrList<UMLClassifier> conceptList;
	for(UMLObject *obj = objectList.first(); obj ; obj = objectList.next())
		if(obj -> getBaseType() == ot_Class || obj->getBaseType() == ot_Interface)
			conceptList.append((UMLClassifier *)obj);
	return conceptList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLInterface> UMLDoc::getInterfaces() {
	QPtrList<UMLInterface> interfaceList;
	for(UMLObject* obj = objectList.first(); obj ; obj = objectList.next()) {
		if(obj->getBaseType() == ot_Interface) {
			interfaceList.append((UMLInterface*)obj);
		}
	}
	return interfaceList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLAssociation> UMLDoc::getAssociations() {
	QPtrList<UMLAssociation> associationList;
	for(UMLObject *obj = objectList.first(); obj ; obj = objectList.next())
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

void UMLDoc::showProperties(UMLWidget * o) {
	// will already be selected so make sure docWindow updates the doc
	// back it the widget
	getDocWindow() -> updateDocumentation( false );
	ClassPropDlg *dlg = new ClassPropDlg((QWidget*)parent(), o);

	if(dlg->exec()) {
		getDocWindow() -> showDocumentation( o -> getUMLObject() , true );
		emit sigWidgetUpdated(o -> getUMLObject());
		emit sigObjectChanged(o -> getUMLObject());
		setModified(true);
	}
	dlg -> close(true);//wipe from memory
}

void UMLDoc::setModified(bool modified /*=true*/, bool addToUndo /*=true*/) {
	if(!loading) {
		m_modified = modified;
		((UMLApp *) parent())->setModified(modified);

		if (modified && addToUndo) {
			addToUndoStack();
			clearRedoStack();
		}
	}
}

bool UMLDoc::addUMLObjectPaste(UMLObject* Obj) {
	if(!Obj || !m_pChangeLog) {
		kdDebug() << "no Obj || Changelog" << endl;
		return false;
	}
	UMLObject * temp = findUMLObject(Obj->getBaseType(), Obj -> getName() );
	if( temp ) {
		m_pChangeLog->addIDChange( Obj -> getID(), temp -> getID() );
		delete Obj;
		return true;
	}
	int result =  assignNewID(Obj->getID());
	Obj->setID(result);

	//If it is a CONCEPT then change the ids of all its operations and attributes
	if(Obj->getBaseType() == ot_Class ) {

		QPtrList<UMLAttribute>* attibutes = ((UMLClass *)Obj)->getAttList();
		for(UMLObject *o = attibutes->first(); o; o = attibutes->next()) {
			result = assignNewID(o->getID());
			o->setID(result);
		}

		QPtrList<UMLTemplate>* templates = ((UMLClass *)Obj)->getTemplateList();
		for(UMLObject* o = templates->first(); o; o = templates->next()) {
			result = assignNewID(o->getID());
			o->setID(result);
		}
	}

	if(Obj->getBaseType() == ot_Interface || Obj->getBaseType() == ot_Class ) {
		QPtrList<UMLOperation>* operations = ((UMLClassifier*)Obj)->getOpList();
		for(UMLObject *o = operations->first(); o; o = operations->next()) {
			result =  assignNewID(o->getID());
			o->setID(result);
		}
	}

	objectList.append(Obj);
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

/** Assigns a New ID to an Object, and also logs the assignment to its internal
ChangeLog */
int UMLDoc::assignNewID(int OldID) {
	int result = ++uniqueID;
	if(m_pChangeLog)
		m_pChangeLog->addIDChange(OldID, result);
	return result;
}

/** Adds an already created UMLView to the document, it gets assigned a new ID,
if its name is already in use then the function appends
a number to it to differentiate it from the others; this number is incremental so if
number 1 is in use then it tries 2 and then 3 and so on */
bool UMLDoc::addUMLView(UMLViewData * pViewData ) {
	if(!pViewData || !m_pChangeLog)
		return false;

	int i = 0;
	QString viewName = (QString)pViewData->getName();
	QString name = viewName;
	while(true) {
		UMLView * o = findView(pViewData->getType(), name);
		if(o) {
			name = viewName;
			name += "_" + QString::number(++i);
			o = findView(pViewData->getType(), name);
		} else {
			if(i) //If name was modified
				pViewData->setName(name);
			break;
		}
	}
	int result = assignNewID(pViewData->getID());
	pViewData->setID(result);
	UMLView* pView = new UMLView(UMLApp::app()->getMainViewWidget(), pViewData, this);

	if (!pView->activateAfterSerialize( true ) ) {
		kdDebug()<<"Error activating diagram"<<endl;
		return false;
	}
	pView->endPartialWidgetPaste();
	pView->setOptionState( ((UMLApp *) parent()) -> getOptionState() );
	addView(pView);
	setModified(true);
	return true;
}


bool UMLDoc::activateView ( int viewID ) {
	bool status = true;

	for(UMLView *v = viewsNotActivated.first(); v; v = viewsNotActivated.next() )
		if ( v->getID() == viewID) {
			status = v->activateAfterSerialize();
			viewsNotActivated.remove();
		}
	loading = false;
	return status;
}
bool UMLDoc::activateAllViews() {
	bool status = true;
	loading = true; //this is to prevent document becoming modified when activating a view

	for(UMLView *v = pViewList -> first(); v; v = pViewList -> next() )
		status = status && v->activateAfterSerialize();
	loading = false;
	viewsNotActivated.clear();
	return status;
}

void UMLDoc::settingsChanged(SettingsDlg::OptionState optionState) {
	// for each view update settings
	if( pViewList )
		for(UMLView *w = pViewList -> first() ; w ; w = pViewList -> next() )
			w -> setOptionState(optionState);
	initSaveTimer();
	return;
}

DocWindow * UMLDoc::getDocWindow() {
	UMLApp* app = (UMLApp*)parent();
	return app -> getDocWindow();
}

void UMLDoc::getAssciationListAllViews( UMLView * view, UMLObject * object, AssociationWidgetDataList & list ) {
	UMLView * tempView = 0;
	AssociationWidget * assocWidget = 0;
	AssociationWidgetData * assocData = 0,* tempData = 0;
	AssociationWidgetDataList tempList;

	for( tempView = pViewList->first(); tempView != 0; tempView = pViewList->next() ) {
		if( view != tempView && view->getType() == tempView->getType() ) {
			AssociationWidgetList viewList;
			tempView->getWidgetAssocs( object, viewList );

			AssociationWidgetListIt view_it( viewList );

			assocWidget = 0;
			tempList.clear();
			while( ( assocWidget = view_it.current() ) ) {
				AssociationWidgetDataListIt it( list );
				tempData = 0;
				bool bAdd = true;
				assocData = assocWidget -> getData();


				while( bAdd && ( tempData = it.current()  ) ) {
					if( tempData -> getAssocType() == assocData -> getAssocType() ) {
						if( tempData -> getWidgetAID() == assocData -> getWidgetAID() &&
						        tempData -> getWidgetBID() == assocData -> getWidgetBID() )
							bAdd = false;
					}
					if( bAdd )
						tempList.append( assocData );
					++it;
				}
				++view_it;
			}//end while
			AssociationWidgetDataListIt temp_it( tempList );
			tempData = 0;
			while( ( tempData = temp_it.current() ) ) {
				list.append( tempData );
				++temp_it;
			}
		}//end if
	}//end for
	return;
}

void UMLDoc::editCopy() {
	UMLApp * app =( UMLApp *) parent();
	app -> slotEditCopy();
}

void UMLDoc::editCut() {
	UMLApp * app =( UMLApp *) parent();
	app -> slotEditCut();
}

void UMLDoc::editPaste() {
	UMLApp * app =( UMLApp *) parent();
	app -> slotEditPaste();
}

bool UMLDoc::getPasteState() {
	UMLApp * app =( UMLApp *) parent();
	return app -> getPasteState();
}

bool UMLDoc::getCutCopyState() {
	UMLApp * app =( UMLApp *) parent();
	return app -> getCutCopyState();
}

void UMLDoc::initSaveTimer() {
	if( m_pAutoSaveTimer ) {
		m_pAutoSaveTimer -> stop();
		disconnect( m_pAutoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
		delete m_pAutoSaveTimer;
		m_pAutoSaveTimer = 0;
	}
	int time[] = { 5 , 10, 15 , 30 };
	SettingsDlg::OptionState optionState = UMLApp::app()->getOptionState();
	if( optionState.generalState.autosave ) {
		m_pAutoSaveTimer = new QTimer(this, "_AUTOSAVETIMER_" );
		connect( m_pAutoSaveTimer, SIGNAL( timeout() ), this, SLOT( slotAutoSave() ) );
		m_pAutoSaveTimer -> start( time[ optionState.generalState.time ] * 60000, false );
	}
	return;
}

void UMLDoc::slotAutoSave() {
	//Only save if modified.
	if( !m_modified ) {
		return;
	}
	KURL tempURL = doc_url;
	if( tempURL.fileName() == i18n("Untitled") ) {
		tempURL.setPath( QDir::homeDirPath() + i18n("/autosave%1").arg(".xmi") );
		saveDocument( tempURL );
		m_modified = true;
	} else {
		saveDocument( tempURL );
	}
	return;
}

void UMLDoc::signalDiagramRenamed(UMLView * pView ) {
	emit sigDiagramRenamed( pView -> getID() );
	return;
}

void UMLDoc::addToUndoStack() {
	if (!loading) {

		QBuffer* buffer = new QBuffer();
		buffer->open(IO_WriteOnly);
		QDataStream* undoData = new QDataStream();
		undoData->setDevice(buffer);
		saveToXMI(*buffer);
		buffer->close();
		undoStack.prepend(undoData);

		if (undoStack.count() > 1) {
			((UMLApp*)parent())->enableUndo(true);
		}
	}
}

void UMLDoc::clearUndoStack() {
	undoStack.setAutoDelete(true);
	undoStack.clear();
	((UMLApp*)parent())->enableRedo(false);
	undoStack.setAutoDelete(false);
	clearRedoStack();
}

void UMLDoc::clearRedoStack() {
	redoStack.setAutoDelete(true);
	redoStack.clear();
	((UMLApp*)parent())->enableRedo(false);
	redoStack.setAutoDelete(false);
}

void UMLDoc::loadUndoData() {
	if (undoStack.count() > 1) {
		int currentViewID = currentView->getID();
		loading = true;
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
		loading = false;

		undoStack.setAutoDelete(true);
		if (undoStack.count() <= 1) {
			((UMLApp*)parent())->enableUndo(false);
		}
		if (redoStack.count() >= 1) {
			((UMLApp*)parent())->enableRedo(true);
		}
		while (undoStack.count() > undoMax) {
			undoStack.removeLast();
		}
		if (currentView->getID() != currentViewID) {
			changeCurrentView(currentViewID);
		}
		undoStack.setAutoDelete(false);
	} else {
		kdWarning() << "no data in undostack" << endl;
	}
}

void UMLDoc::loadRedoData() {
	if (redoStack.count() >= 1) {
		int currentViewID = currentView->getID();
		loading = true;
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
		loading = false;

		redoStack.setAutoDelete(true);
		if (redoStack.count() < 1) {
			((UMLApp*)parent())->enableRedo(false);
		}
		if (undoStack.count() > 1) {
			((UMLApp*)parent())->enableUndo(true);
		}
		if (currentView->getID() != currentViewID) {
			changeCurrentView(currentViewID);
		}
		redoStack.setAutoDelete(false);
	} else {
		kdWarning() << "no data in redostack" << endl;
	}
}



#include "umldoc.moc"
