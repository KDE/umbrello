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

#include <kapplication.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <ktempfile.h>

#define XMI_FILE_VERSION "1.2.0"
static const uint undoMax = 30;


UMLDoc::UMLDoc(QWidget *parent, const char *name) : QObject(parent, name) {
	listView = 0;
	currentView = 0;
	uniqueID = 0;
	m_count = 0;
	m_currentcodegenerator = 0;
	objectList.clear();
	objectList.setAutoDelete(false); // DONT autodelete
	m_ViewList.setAutoDelete(true);

	m_codeGenerationXMIParamMap = new QMap<QString, QDomElement>;

	m_pChangeLog = 0;
	m_Doc = "";
	m_modified = false;
	m_bLoading = false;
	m_pAutoSaveTimer = 0;
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
	if(listView)
		connect(this, SIGNAL(sigObjectRemoved(UMLObject *)), view, SLOT(slotObjectRemoved(UMLObject *)));
	m_ViewList.append(view);

	if ( ! m_bLoading ) {
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
void UMLDoc::removeView(UMLView *view , bool enforceCurrentView ) {
	if(!view)
	{
		kdError()<<"UMLDoc::removeView(UMLView *view) called with view = 0"<<endl;
		return;
	}
	if(listView) {
		disconnect(this,SIGNAL(sigObjectRemoved(UMLObject *)), view,SLOT(slotObjectRemoved(UMLObject *)));
	}
	view->hide();
	//remove all widgets before deleting view
	view->removeAllWidgets();
	// m_ViewLiset is set to autodelete!!
	m_ViewList.remove(view);
	if(currentView == view)
	{
		currentView = 0L;
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
	doc_url = url;
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
const KURL& UMLDoc::URL() const {
	return doc_url;
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
////////////////////////////////////////////////////////////////////////////////////////////////////
SettingsDlg::OptionState UMLDoc::getOptionState() {
	return UMLApp::app()-> getOptionState();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::newDocument() {
	/*deleteContents();*/ closeDocument();
	currentView = 0;
	doc_url.setFileName(i18n("Untitled"));
	//see if we need to start with a new diagram
	SettingsDlg::OptionState optionState = getOptionState();

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

	m_bLoading = true;

	doc_url = url;
	QDir d = url.path(1);
	deleteContents();
	QString tmpfile;
	KIO::NetAccess::download( url, tmpfile
#if KDE_IS_VERSION(3,1,90)
					      , UMLApp::app()
#endif
				 );
	QFile file( tmpfile );
	if ( !file.exists() ) {
		KMessageBox::error(0, i18n("The file %1 does not exist.").arg(d.path()), i18n("Load Error"));
		doc_url.setFileName(i18n("Untitled"));
		m_bLoading = false;
		newDocument();
		return false;
	}

	if( !file.open( IO_ReadOnly ) ) {
		KMessageBox::error(0, i18n("There was a problem loading file: %1").arg(d.path()), i18n("Load Error"));
		doc_url.setFileName(i18n("Untitled"));
		m_bLoading = false;
		newDocument();
		return false;
	}
	bool status = loadFromXMI( file, ENC_UNKNOWN );
	file.close();
	KIO::NetAccess::removeTempFile( tmpfile );
	if( !status ) {
		KMessageBox::error(0, i18n("There was a problem loading file: %1").arg(d.path()), i18n("Load Error"));
		m_bLoading = false;
		newDocument();
		return false;
	}
	setModified(false);
	m_bLoading = false;
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
		uploaded = KIO::NetAccess::upload( tmpfile.name(), doc_url
#if KDE_IS_VERSION(3,1,90)
								, UMLApp::app()
#endif
						 );
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
	DocWindow* dw = getDocWindow();
	if (dw) {
		dw->newDocumentation();
	}

        // remove all code generators
        QDictIterator<CodeGenerator> it( m_codeGeneratorDictionary );
        for( ; it.current(); ++it )
                removeCodeGenerator(it.current());

        m_currentcodegenerator = 0;

	if (listView) {
		listView->init();
		removeAllViews();
		if(objectList.count() > 0) {
			// clear our object list. We do this explicitly since setAutoDelete is false for the objectList now.
			for(UMLObject * obj = objectList.first(); obj != 0; obj = objectList.next())
				obj->deleteLater();
			objectList.clear();
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::setupSignals() {
	WorkToolBar *tb = ((UMLApp*)parent()) -> getWorkToolBar();


	connect(this, SIGNAL(sigDiagramChanged(Uml::Diagram_Type)), tb, SLOT(slotCheckToolBar(Uml::Diagram_Type)));
	//new signals below

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

////////////////////////////////////////////////////////////////////////////////////////////////////
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
	for(UMLObject * obj = objectList.first(); obj != 0; obj = objectList.next())
	{
		if(obj -> getID() == id)
			return obj;
		if (obj->getBaseType() == Uml::ot_Package) {
			UMLObject *o = ((UMLPackage*)obj)->findObject(id);
			if (o)
				return o;
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::findUMLObject(QString name, UMLObject_Type type /* = ot_UMLObject */) {
	return findUMLObject(objectList, name, type);
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
					  << name << " (actual type: "
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
	for (UMLObject * o = objectList.first(); o; o = objectList.next()) {
		if (o->getAuxId() == idStr)
			return o;
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
	else if(type == ot_Datatype)
		currentName = i18n("new_datatype");
	else if(type == ot_Enum)
		currentName = i18n("new_enum");
	else if(type == ot_Association)
		currentName = i18n("new_association");
	else
		currentName = i18n("new_object");

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
	//stop it being added twice
	if ( objectList.find(object) == -1)  {
		objectList.append( object );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// a simple removeal of an object
void UMLDoc::slotRemoveUMLObject(UMLObject* object)  {
	objectList.remove(object);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::isUnique(QString name)
{
	UMLListViewItem *parentItem = (UMLListViewItem*)listView->currentItem();
	if (parentItem != NULL && parentItem->getType() == lvt_Package) {
		UMLPackage *parentPkg = (UMLPackage*)parentItem->getUMLObject();
		return (parentPkg->findObject(name) == NULL);
	}
	// Not currently in a package:
	// Check against all objects that _dont_ have a parent package.
	for (UMLObject *obj = objectList.first(); obj; obj = objectList.next())
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
	if( !n.isEmpty() && isUnique(n) )
	{
		name = n;
	}
	else
	{
		name = uniqObjectName(type);
		do {
			name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), name, &ok, (QWidget*)parent());
			if (!ok) {
				return 0;
			}
			if (name.length() == 0) {
				KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
				continue;
			}
			if (! isUnique(name)) {
				KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
				name = "";
			}
		} while (name.isEmpty());
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
	objectList.append(o);
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
	Uml::Scope scope = getOptionState().classState.defaultAttributeScope;
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
	/*
	do {
		UMLOperationDialog operationDialogue(0, op);
		if( operationDialogue.exec() != QDialog::Accepted ) {
			delete op;
			return NULL;
		}
	} while (classifier->checkOperationSignature(op->getName(), op->getParmList()));
	*/
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
//no new i18n							 i18n("An operation with the same name and signature already exists. "
//							      "You can not add it again.")
							 "");
			} else {
				break;
			}
		} while(1);
	}
//FIXMEnow

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

	// Remove the UMLAssociation in this UMLDoc objectList.
	UMLObject *object = (UMLObject *) assoc;
	objectList.remove(object);

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
		if (a->getObjectA() == roleAObj && a->getObjectB() == roleBObj)
			return a;
		if (a->getObjectA() == roleBObj && a->getObjectB() == roleAObj) {
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
		assoc = new UMLAssociation(this, type, a, b );
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
	objectList.append( (UMLObject*) Assoc);

	// I dont believe this appropriate, UMLAssociations ARENT UMLWidgets -b.t.
	// emit sigObjectCreated(o);

	setModified(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDoc::addAssocToConcepts(UMLAssociation* a) {

	int AId = a->getRoleAId();
	int BId = a->getRoleBId();
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
			name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), dname, &ok, (QWidget*)parent());
		} else {
			name = dname;
		}
		if (!ok)  {
			break;
		}
		if (name.length() == 0)  {
			KMessageBox::error(0, i18n("That is an invalid name for a diagram."), i18n("Invalid Name"));
		} else if(!findView(type, name)) {
			UMLView* temp = new UMLView(UMLApp::app()->getMainViewWidget(), this);
			temp->setName( name );
			temp->setType( type );
			temp->setID( getUniqueID() );
			addView(temp);
			temp -> setOptionState( getOptionState() );
			emit sigDiagramCreated(uniqueID);
			setModified(true);
			((UMLApp*)parent())->enablePrint(true);
			changeCurrentView(uniqueID);
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
		QString name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)parent());

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
		QString name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)parent());
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
		QString name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), oldName, &ok, (QWidget*)parent());
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
	if(!umlview)
	{
		kdError()<<"Request to remove diagram "<<id<<": Diagram not found!"<<endl;
		return;
	}
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
void UMLDoc::removeUMLObject(UMLObject* umlobject) {
	getDocWindow()->updateDocumentation(true);
	UMLObject_Type type = umlobject->getBaseType();

	if (objectTypeIsClassifierListItem(type))  {

		UMLClassifier* parent = (UMLClassifier*)umlobject->parent();
		emit sigObjectRemoved(umlobject);
		if (type == ot_Operation) {
			parent->removeOperation(dynamic_cast<UMLOperation*>(umlobject));
		} else if (type == ot_Attribute) {
			UMLClass* pClass = dynamic_cast<UMLClass*>(parent);
			if (pClass)  {
				pClass->removeAttribute(umlobject);
			}
		} else if (type == ot_Template) {
			UMLClass* pClass = dynamic_cast<UMLClass*>(parent);
			if (pClass)  {
				pClass->removeTemplate((UMLTemplate*)umlobject);
			}
		}

	} else {
		if (type == ot_Association) {
			// Remove the UMLAssociation at the concept that plays role B.
			UMLAssociation *a = (UMLAssociation *)umlobject;
			Uml::Association_Type assocType = a->getAssocType();
			int AId = a->getRoleAId();
			int BId = a->getRoleBId();
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
		objectList.remove(umlobject);
	}
	setModified(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::showProperties(UMLObject* object, int page, bool assoc) {
	getDocWindow()->updateDocumentation( false );
	ClassPropDlg* dialogue = new ClassPropDlg((QWidget*)parent(), object, page, assoc);

	bool modified = false;
	if ( dialogue->exec() ) {
		getDocWindow()->showDocumentation(object, true);
		setModified(true);
		modified = true;
	}
	dialogue->close(true);//wipe from memory
	return modified;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDoc::showProperties(ObjectWidget *o) {
	getDocWindow() -> updateDocumentation( false );
	ClassPropDlg *dlg = new ClassPropDlg((QWidget*)parent(), o);

	bool modified = false;
	if(dlg->exec()) {
		getDocWindow() -> showDocumentation( o, true );
		o->getUMLObject();
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
	bool status=true;
	QDomElement docElement = doc.createElement( "docsettings" );
	int viewID = -1;
	if( currentView )
		viewID = currentView -> getID();
	docElement.setAttribute( "viewid", viewID );
	docElement.setAttribute( "documentation", m_Doc );
	docElement.setAttribute( "uniqueid", uniqueID );
	content.appendChild( docElement );

	QDomElement objectsElement = doc.createElement( "UML:Model" );

#ifdef XMI_FLAT_PACKAGES
	// Save packages first so that when loading they are known first.
	// This simplifies the establishing of cross reference links from
	// contained objects to their containing package.
	for (UMLObject *p = objectList.first(); p; p = objectList.next() ) {
		UMLObject_Type t = p->getBaseType();
		if (t != ot_Package)
			continue;
		if (! p->saveToXMI(doc, objectsElement))
			status = false;
	}
#endif

	// Save everything except operations, attributes, and associations.
	// Operations and attributes are owned by classifiers and will show up
	// as their child nodes.
	// Associations are saved in an extra step (see below.)
	for (UMLObject *o = objectList.first(); o; o = objectList.next() ) {
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
		if (t == ot_Attribute || t == ot_Operation || t == ot_Association)
			continue;
		if (! o->saveToXMI(doc, objectsElement))  {
			status = false;
		}

	}

	// Save the UMLAssociations.
	// These are saved last so that upon loading, an association's role
	// objects are known beforehand. This simplifies the establishing of
	// cross reference links from the association to its role objects.
        UMLAssociationList alist = getAssociations();
	for (UMLAssociation * a = alist.first(); a; a = alist.next())
		a->saveToXMI(doc, objectsElement);
	content.appendChild( objectsElement );

	if( !status )
		return status;

	// Save each view/diagram.
	QDomElement diagramsElement = doc.createElement( "diagrams" );
	for(UMLView *pView = m_ViewList.first(); pView && status; pView = m_ViewList.next() )
		status = pView -> saveToXMI( doc, diagramsElement );
	content.appendChild( diagramsElement );
	if( !status )
		return status;
	//  save listview
	status = listView -> saveToXMI( doc, content );

	// save code generators
	QDomElement codeGenElement = doc.createElement( "codegeneration" );
        QDictIterator<CodeGenerator> it( m_codeGeneratorDictionary );
        for( ; it.current(); ++it )
                status = it.current()->saveToXMI ( doc, codeGenElement );
	content.appendChild( codeGenElement );

	root.appendChild( content );
	QTextStream stream( &file );
	stream.setEncoding(QTextStream::UnicodeUTF8);
	stream << doc.toString();

	return status;
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
/*
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
*/
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

	int nViewID = -1;
	while( !node.isNull() ) {
		QDomElement element = node.toElement();

		if (element.isNull()) {
			node = node.nextSibling();
			element = node.toElement();
			continue;
		}
		//check header
		if( element.tagName() == "XMI.header" ) {
			QDomNode headerNode = node.firstChild();
			if ( !validateXMIHeader(headerNode) ) {
				return false;
			}
		}
		if( element.tagName() != "XMI.content" ) {
			node = node.nextSibling();
			element = node.toElement();
			continue;
		}
		//process content
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
			} else if( tag == "umlobjects"  // for bkwd compat.
			         || tag == "UML:Model" ) {
				emit sigResetStatusbarProgress();
				emit sigSetStatusbarProgress( 0 );
				emit sigSetStatusbarProgressSteps( 10 );
						//FIXME need a way to make status bar actually reflect
	                                        //how much of the file has been loaded rather than just
	                                        //counting to 10 (an arbitrary number)
				emit sigWriteToStatusBar( i18n("Loading UML elements...") );
				m_count = 0;
				if( !loadUMLObjectsFromXMI( element ) ) {
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
			} else if( tag == "codegeneration" ) {
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
			node = node.nextSibling();
			element = node.toElement();
		}//end while
		break;
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
bool UMLDoc::loadUMLObjectsFromXMI(QDomElement& element) {
	QDomNode node = element.firstChild();
	QDomElement tempElement = node.toElement();

	while ( !tempElement.isNull() ) {
		QString type = tempElement.tagName();
		if (type == "UML:Namespace.ownedElement") {
			//CHECK: Umbrello currently assumes that nested elements
			// are ownedElements anyway.
			// Therefore the <UML:Namespace.ownedElement> tag is of no
			// significance.
			loadUMLObjectsFromXMI(tempElement);
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
		if (type == "UML:Association" || type == "UML:Generalization") {
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
		} else {
			objectList.append( pObject );
		}

		// Now, we need to add all the UMLObjects held by the package
		// should it have any.
		if (type == "UML:Package") {
			UMLObjectList oList = ((UMLPackage*) pObject)->containedObjects();
			for (UMLObject * obj = oList.first(); obj != 0; obj = oList.next())
				objectList.append(obj);
		}

		emit sigSetStatusbarProgress( ++m_count );
		node = node.nextSibling();
		tempElement = node.toElement();
	}//end while
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLDoc::makeNewUMLObject(QString type) {
	UMLObject* pObject = 0;
	if (type == "UML:UseCase") {
		pObject = new UMLUseCase();
	} else if (type == "UML:Actor") {
		pObject = new UMLActor();
	} else if (type == "UML:Class") {
		pObject = new UMLClass();
	} else if (type == "UML:Package") {
		pObject = new UMLPackage();
	} else if (type == "UML:Component") {
		pObject = new UMLComponent();
	} else if (type == "UML:Node") {
		pObject = new UMLNode();
	} else if (type == "UML:Artifact") {
		pObject = new UMLArtifact();
	} else if (type == "UML:Interface") {
		pObject = new UMLInterface();
	} else if (type == "UML:Datatype"         // for bkwd compat.
		|| type == "UML:DataType") {
		pObject = new UMLDatatype();
	} else if (type == "UML:Enum") {
		pObject = new UMLEnum();
	} else if (type == "UML:Association") {
		pObject = new UMLAssociation(this, Uml::at_Unknown, (UMLObject*)NULL, (UMLObject*) NULL);
	} else if (type == "UML:Generalization") {
		pObject = new UMLAssociation(this, Uml::at_Generalization, NULL, NULL);
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
	SettingsDlg::OptionState state =  getOptionState();
	UMLView * pView = 0;
	int count = 0;
	while( !element.isNull() ) {
		if( element.tagName() == "diagram" ) {
			pView = new UMLView(UMLApp::app()->getMainViewWidget(), this);
			if( !pView->loadFromXMI( element ) ) {
				kdWarning() << "failed load on viewdata loadfromXMI" << endl;
				delete pView;
				return false;
			}
			pView -> hide();
			pView -> setOptionState( state );
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
	currentView = 0L;
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

UMLClassifierList UMLDoc::getConcepts() {
	UMLClassifierList conceptList;
	for(UMLObject *obj = objectList.first(); obj ; obj = objectList.next()) {
		Uml::UMLObject_Type ot = obj->getBaseType();
		if(ot == ot_Class || ot == ot_Interface || ot == ot_Datatype || ot == ot_Enum)  {
			conceptList.append((UMLClassifier *)obj);
		}
	}
	return conceptList;
}

UMLClassifierList UMLDoc::getClassesAndInterfaces() {
	UMLClassifierList conceptList;
	for(UMLObject* obj = objectList.first(); obj ; obj = objectList.next()) {
		Uml::UMLObject_Type ot = obj->getBaseType();
		if(ot == ot_Class || ot == ot_Interface || ot == ot_Enum)  {
			conceptList.append((UMLClassifier *)obj);
		}
	}
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
QPtrList<UMLDatatype> UMLDoc::getDatatypes() {
	QPtrList<UMLDatatype> datatypeList;
	for(UMLObject* obj = objectList.first(); obj ; obj = objectList.next()) {
		if(obj->getBaseType() == ot_Datatype) {
			datatypeList.append((UMLDatatype*)obj);
		}
	}
	return datatypeList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAssociationList UMLDoc::getAssociations() {
	UMLAssociationList associationList;
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

bool UMLDoc::showProperties(UMLWidget * o) {
	// will already be selected so make sure docWindow updates the doc
	// back it the widget
	getDocWindow() -> updateDocumentation( false );
	ClassPropDlg *dlg = new ClassPropDlg((QWidget*)parent(), o);

	bool modified = false;
	if(dlg->exec()) {
		getDocWindow() -> showDocumentation( o -> getUMLObject() , true );
		setModified(true);
		modified = true;
	}
	dlg -> close(true);//wipe from memory
	return modified;
}

void UMLDoc::setModified(bool modified /*=true*/, bool addToUndo /*=true*/) {
	if(!m_bLoading) {
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
	UMLObject * temp = findUMLObject( Obj -> getName() );
	if( temp ) {
		m_pChangeLog->addIDChange( Obj -> getID(), temp -> getID() );
		delete Obj;
		return true;
	}
	int result =  assignNewID(Obj->getID());
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
		UMLClassifierListItemList* operations = ((UMLClassifier*)Obj)->getOpList();
		for(UMLObject* listItem = operations->first(); listItem; listItem = operations->next()) {
			result = assignNewID(listItem->getID());
			listItem->setID(result);
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

int UMLDoc::getUniqueID() {
	return ++uniqueID;
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

/** Adds an already created UMLView to the document, it gets assigned a new ID,
if its name is already in use then the function appends
a number to it to differentiate it from the others; this number is incremental so if
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
	pView->setOptionState( getOptionState() );
	addView(pView);
	setModified(true);
	return true;
}

bool UMLDoc::activateView ( int viewID ) {
	bool status = true;

	for(UMLView *v = viewsNotActivated.first(); v; v = viewsNotActivated.next() )
		if ( v->getID() == viewID) {
			status = v->activateAfterLoad();
			viewsNotActivated.remove();
		}
	m_bLoading = false;
	return status;
}

bool UMLDoc::activateAllViews() {
	bool status = true;
	m_bLoading = true; //this is to prevent document becoming modified when activating a view

	for(UMLView *v = m_ViewList.first(); v; v = m_ViewList.next() )
		status = status && v->activateAfterLoad();
	m_bLoading = false;
	viewsNotActivated.clear();
	return status;
}

void UMLDoc::settingsChanged(SettingsDlg::OptionState optionState) {
	// for each view update settings
	for(UMLView *w = m_ViewList.first() ; w ; w = m_ViewList.next() )
		w -> setOptionState(optionState);
	initSaveTimer();
	return;
}

DocWindow * UMLDoc::getDocWindow() {
	UMLApp* app = (UMLApp*)parent();
	return app -> getDocWindow();
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
	SettingsDlg::OptionState optionState = getOptionState();
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
	if (!m_bLoading) {

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
		m_bLoading = false;

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
		m_bLoading = false;

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
	listView->closeDatatypesFolder();
}

bool UMLDoc::objectTypeIsClassifierListItem(UMLObject_Type type)  {
	if (type == ot_Attribute || type == ot_Operation || type == ot_Template
	    || type == ot_EnumLiteral || type == ot_Stereotype)  {
		return true;
	} else {
		return false;
	}
}

#include "umldoc.moc"
