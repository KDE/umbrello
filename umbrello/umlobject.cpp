/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qregexp.h>
#include <kdebug.h>
#include "umlobject.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "package.h"
#include "stereotype.h"

UMLObject::UMLObject(const UMLObject * parent, const QString &name, int id)
  : QObject(const_cast<UMLObject*>(parent), "UMLObject" ) {
	init();
	m_nId = id;
	m_Name = name;
}

UMLObject::UMLObject(const QString &name, int id)
  :  QObject(UMLApp::app()->getDocument(), "UMLObject") {
	init();
	m_nId = id;
	m_Name = name;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject::UMLObject(const UMLObject * parent)
  : QObject(const_cast<UMLObject*>(parent)) {
	init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject::~UMLObject() {
}

void UMLObject::init() {
	m_BaseType = ot_UMLObject;
	m_nId = -1;
	m_pUMLPackage = NULL;
	m_Name = "";
	m_Scope = Public;
	m_pStereotype = NULL;
	m_Doc = "";
	m_bAbstract = false;
	m_bStatic = false;
	m_bInPaste = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool UMLObject::acceptAssociationType(Uml::Association_Type)
{// A UMLObject accepts nothing. This should be reimplemented by the subclasses
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLObject::setID(int NewID) {
	m_nId = NewID;
	emit modified();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLObject::setName(QString strName) {
	m_Name = strName;
	emit modified();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLObject::getName() const {
	return m_Name;
}

QString UMLObject::getFullyQualifiedName(QString separator /* = "::" */) const {
	QString fqn;
	if (m_pUMLPackage) {
		fqn = m_pUMLPackage->getFullyQualifiedName(separator);
		fqn.append(separator);
	}
	fqn.append(m_Name);
	return fqn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLObject::operator==(UMLObject & rhs ) {
	if( this == &rhs )
		return true;

	//don't compare IDs, these are program specific and
	//don't mean the objects are the same
	//***** CHECK: Who put in this comment? What was the reason?
	//***** Currently some operator== in umbrello compare the IDs
	//***** while others don't.

	if( m_Name != rhs.m_Name )
		return false;

	// Packages create different namespaces, therefore they should be
	// part of the equality test.
	if( m_pUMLPackage != rhs.m_pUMLPackage )
		return false;

	// Making the type part of an object's identity has its problems:
	// Not all programming languages support declarations of the same
	// name but different type.
	// In such cases, the code generator is responsible for generating
	// the appropriate error message.
	if( m_BaseType != rhs.m_BaseType )
		return false;

	// The documentation should not be part of the equality test.
	// If two objects are the same but differ only in their documentation,
	// what does that mean?
	//if( m_Doc != rhs.m_Doc )
	//	return false;

	// The scope should not be part of the equality test.
	// What does it mean if two objects are the same but differ in their
	// scope? - I'm not aware of any programming language that would
	// support that.
	//if( m_Scope != rhs.m_Scope )
	//	return false;

	// See comments above
	//if( m_pStereotype != rhs.m_pStereotype )
	//	return false;

	// See comments above
	//if( m_bAbstract != rhs.m_bAbstract )
	//	return false;

	// See comments above
	//if( m_bStatic != rhs.m_bStatic )
	//	return false;

	return true;
}

void UMLObject::copyInto(UMLObject *rhs) const
{
	UMLDoc *umldoc = UMLApp::app()->getDocument();

	// Data members with copy constructor
	rhs->m_Doc = m_Doc;
	rhs->m_pStereotype = m_pStereotype;
	rhs->m_bAbstract = m_bAbstract;
	rhs->m_bStatic = m_bStatic;
	rhs->m_BaseType = m_BaseType;
	rhs->m_Scope = m_Scope;
	rhs->m_pUMLPackage = m_pUMLPackage;

	// We don't want the same name existing twice.
	rhs->m_Name = umldoc->uniqObjectName(m_BaseType, m_Name);

	// Create a new ID.
	rhs->m_nId = umldoc->getUniqueID();

	// Hope that the parent from QObject is okay.
	if (rhs->parent() != parent())
		kdDebug() << "copyInto has a wrong parent" << endl;
}


bool UMLObject::getAbstract() const{
	return m_bAbstract;
}

void UMLObject::setAbstract(bool bAbstract) {
	m_bAbstract = bAbstract;
	emit modified();
}

void UMLObject::setInPaste(bool bInPaste /* =true */) {
	m_bInPaste = bInPaste;
}

/** Returns true if this UMLObject has classifier scope, otherwise false (the default). */
bool UMLObject::getStatic() const
{
	return m_bStatic;
}
/** Sets the value for m_bStatic. */
void UMLObject::setStatic(const bool bStatic)
{
	m_bStatic = bStatic;
	emit modified();
}

void UMLObject::emitModified()
{
	emit modified();
}

void UMLObject::setDoc(QString d) {
	m_Doc = d;
	emit modified();
}

UMLObject_Type UMLObject::getBaseType() const {
	return m_BaseType;
}

int UMLObject::getID() const {
	return m_nId;
}

QString UMLObject::getDoc() const {
	return m_Doc;
}

Scope UMLObject::getScope() const {
	return m_Scope;
}

void UMLObject::setScope(Scope s) {
	m_Scope = s;
	emit modified();
}

void UMLObject::setUMLStereotype(UMLStereotype *stereo) {
	if (stereo == m_pStereotype)
		return;
	if (stereo) {
		stereo->incrRefCount();
	}
	if (m_pStereotype) {
		m_pStereotype->decrRefCount();
		if (m_pStereotype->refCount() == 0) {
			UMLDoc *pDoc = UMLApp::app()->getDocument();
			pDoc->removeStereotype(m_pStereotype);
			delete m_pStereotype;
		}
	}
	m_pStereotype = stereo;
	emit modified();
}

void UMLObject::setStereotype(QString _name) {
	if (_name.isEmpty()) {
		setUMLStereotype(NULL);
		return;
	}
	UMLDoc *pDoc = UMLApp::app()->getDocument();
	UMLStereotype *s = pDoc->findOrCreateStereotype(_name);
	setUMLStereotype(s);
}

void UMLObject::setPackage(QString _name) {
	// TBD: Resolve nested packages given in _name (e.g. A::B::C)
	UMLObject *pkgObj = NULL;
	if (_name != "") {
		UMLDoc* umldoc = UMLApp::app()->getDocument();
		if (umldoc == NULL) {
			kdError() << "UMLObject::setPackage: cannot set package name on "
				  << m_Name << endl;
			return;
		}
		pkgObj = umldoc->findUMLObject(_name, ot_Package);
		if (pkgObj == NULL) {
			kdDebug() << "UMLObject::setPackage: creating UMLPackage "
				  << _name << " for " << m_Name << endl;
			pkgObj = umldoc->createUMLObject(ot_Package, _name);
		}
	}
	setUMLPackage( static_cast<UMLPackage *>(pkgObj) );
}

void UMLObject::setUMLPackage(UMLPackage* pPkg) {
	m_pUMLPackage = pPkg;
	emit modified();
}

const UMLStereotype * UMLObject::getUMLStereotype() {
	return m_pStereotype;
}

QString UMLObject::getStereotype() {
	if (m_pStereotype == NULL)
		return "";
	return m_pStereotype->getName();
}

QString UMLObject::getPackage(QString separator /* ="::" */) {
	if (m_pUMLPackage == NULL)
		return "";
	QStringList pkgList;
	UMLPackage* pkg = m_pUMLPackage;
	while (pkg != NULL) {
		pkgList.prepend(pkg->getName());
		pkg = pkg->getUMLPackage();
	}
	return pkgList.join(separator);
}

UMLPackageList UMLObject::getPackages() const {
	UMLPackageList pkgList;
	UMLPackage* pkg = m_pUMLPackage;
	while (pkg != NULL) {
		pkgList.prepend(pkg);
		pkg = pkg->getUMLPackage();
	}
	return pkgList;
}

UMLPackage* UMLObject::getUMLPackage() {
	return m_pUMLPackage;
}

QString UMLObject::getAuxId() const {
	return m_AuxId;
}

QDomElement UMLObject::save( QString tag, QDomDocument & qDoc ) {
	/*
	  Call as the first action of saveToXMI() in child class:
	  This creates the QDomElement with which to work.
	*/
	QDomElement qElement = qDoc.createElement(tag);

	qElement.setAttribute( "xmi.id", m_nId );
	if (!m_Name.isEmpty())
		qElement.setAttribute( "name", m_Name );
	if (! m_Doc.isEmpty())
		qElement.setAttribute( "comment", m_Doc );  //CHECK: uml13.dtd compliance
#ifdef XMI_FLAT_PACKAGES
	if (m_pUMLPackage)             //FIXME: uml13.dtd compliance
		qElement.setAttribute( "package", m_pUMLPackage->getID() );
#endif
	switch (m_Scope) {
		case Uml::Public:
			qElement.setAttribute( "visibility", "public" );
			break;
		case Uml::Protected:
			qElement.setAttribute( "visibility", "protected" );
			break;
		case Uml::Private:
			qElement.setAttribute( "visibility", "private" );
			break;
	}
	if (m_pStereotype != NULL)
		qElement.setAttribute( "stereotype", m_pStereotype->getID() );
	if (m_bAbstract)
		qElement.setAttribute( "isAbstract", "true" );
	/* else
		qElement.setAttribute( "isAbstract", "false" );
	 *** isAbstract defaults to false if not set **********/
	if (m_bStatic)
		qElement.setAttribute( "ownerScope", "classifier" );
	/* else
		qElement.setAttribute( "ownerScope", "instance" );
	 *** ownerScope defaults to instance if not set **********/
	return qElement;
}

bool UMLObject::load( QDomElement& ) {
	// This body is not usually executed because child classes
	// overwrite the load method.
	return true;
}

bool UMLObject::loadFromXMI( QDomElement & element, bool loadID /* =true */) {
	UMLDoc* umldoc = UMLApp::app()->getDocument();
	if (umldoc == NULL) {
		kdError() << "UMLObject::loadFromXMI: umldoc is NULL" << endl;
		return false;
	}
	// Read the name first so that if we encounter a problem, the error
	// message can say the name.
	m_Name = element.attribute( "name", "" );
	if (loadID) {
		QString id = element.attribute( "xmi.id", "" );
		if (id.isEmpty() || id == "-1") {
			kdError() << "UMLObject::loadFromXMI(" << m_Name
				  << "): nonexistent or illegal xmi.id" << endl;
			return false;
		}
		if (id.contains(QRegExp("\\D"))) {
			m_AuxId = id;
			m_nId = umldoc->getUniqueID();
		} else {
			m_nId = id.toInt();
		}
	}

	if (element.hasAttribute("documentation"))  // for bkwd compat.
		m_Doc = element.attribute( "documentation", "" );
	else
		m_Doc = element.attribute( "comment", "" );  //CHECK: need a UML:Comment?

	m_Scope = Uml::Public;
	if (element.hasAttribute("scope")) {        // for bkwd compat.
		QString scope = element.attribute( "scope", "" );
		if (scope == "instance_level")         // nsuml compat.
			m_bStatic = false;
		else if (scope == "classifier_level")  // nsuml compat.
			m_bStatic = true;
		else {
			int nScope = scope.toInt();
			if (nScope >= Uml::Public && nScope <= Uml::Protected)
				m_Scope = (Scope)nScope;
			else
				kdError() << "UMLObject::loadFromXMI(" << m_Name
					<< "): illegal scope" << endl;  // soft error
		}
	} else {
		QString visibility = element.attribute( "visibility", "public" );
		if (visibility == "private"
		    || visibility == "private_vis")    // for compatibility with other programs
			m_Scope = Uml::Private;
		else if (visibility == "protected"
		    || visibility == "protected_vis")  // for compatibility with other programs
			m_Scope = Uml::Protected;
	}

	QString stereo = element.attribute( "stereotype", "" );
	if (!stereo.isEmpty() && stereo != "-1") {
		if (stereo.contains(QRegExp("\\D"))) {
			// Old versions saved the stereotype name instead of the xmi.id.
			setStereotype( stereo );
		} else {
			int stereoID = stereo.toInt();
			m_pStereotype = umldoc->findStereotype(stereoID);
			if (m_pStereotype)
				m_pStereotype->incrRefCount();
			else
				kdError() << "UMLObject::loadFromXMI(" << m_Name << "): "
					  << "UMLStereotype " << stereoID << " not found"
					  << endl;
		}
	}

	if( element.hasAttribute("abstract") ) {     // for bkwd compat.
		QString abstract = element.attribute( "abstract", "0" );
		m_bAbstract = (bool)abstract.toInt();
	} else {
		QString isAbstract = element.attribute( "isAbstract", "false" );
		m_bAbstract = (isAbstract == "true");
	}

	if( element.hasAttribute("static") ) {       // for bkwd compat.
		QString staticScope = element.attribute( "static", "0" );
		m_bStatic = (bool)staticScope.toInt();
	} else {
		QString ownerScope = element.attribute( "ownerScope", "instance" );
		m_bStatic = (ownerScope == "classifier");
	}

	/**** Handle XMI_FLAT_PACKAGES and old files *************************/
	QString pkg = element.attribute( "packageid", "-1" );
	// Some interim versions used "packageid" so test for it.
	int pkgId = -1;
	if (pkg != "-1") {
		pkgId = pkg.toInt();
	} else {
		pkg = element.attribute( "package", "" );
		if (! pkg.isEmpty()) {
			if (pkg.contains(QRegExp("\\D")))
				// Old versions saved the package name instead of the xmi.id.
				setPackage( pkg );
			else
				pkgId = pkg.toInt();
		}
	}
	if (pkgId != -1) {
		UMLObject *pkgObj = umldoc->findUMLObject( pkgId );
		if (pkgObj != NULL) {
			m_pUMLPackage = dynamic_cast<UMLPackage *>(pkgObj);
			if (m_pUMLPackage == NULL)  // soft error
				kdError() << "UMLObject::loadFromXMI(" << m_Name
					  << "): object of packageid "
					  << pkgId << " is not a package" << endl;
		} else {  // soft error
			kdError() << "UMLObject::loadFromXMI(" << m_Name
				  << "): cannot resolve packageid "
				  << pkgId << endl;
		}
	}
	/**** End of XMI_FLAT_PACKAGES and old files handling ****************/

	// If the name is not set, let's check whether the attributes are saved
	// as child nodes.
	// We only do this for the regular loadFromXMI call (when the loading
	// of the xmi.id is done.)
	if (loadID && m_Name.isEmpty()) {
		QDomNode node = element.firstChild();
		QDomElement elem = node.toElement();
		while( !elem.isNull() ) {
			QString tag = elem.tagName();
			if (tagEq(tag, "name")) {
				m_Name = elem.attribute("xmi.value", "");
				if (m_Name.isEmpty())
					m_Name = elem.text();
			} else if (tagEq(tag, "visibility")) {
				QString vis = elem.attribute("xmi.value", "");
				if (vis.isEmpty())
					vis = elem.text();
				if (vis == "private" || vis == "private_vis")
					m_Scope = Uml::Private;
				else if (vis == "protected" || vis == "protected_vis")
					m_Scope = Uml::Protected;
			} else if (tagEq(tag, "isAbstract")) {
				QString isAbstract = elem.attribute("xmi.value", "");
				if (isAbstract.isEmpty())
					isAbstract = elem.text();
				m_bAbstract = (isAbstract == "true");
			} else if (tagEq(tag, "ownerScope")) {
				QString ownerScope = elem.attribute("xmi.value", "");
				if (ownerScope.isEmpty())
					ownerScope = elem.text();
				m_bStatic = (ownerScope == "classifier");
			}
			node = node.nextSibling();
			elem = node.toElement();
		}
	}

	// Operations, attributes, enum literals, stereotypes, associations,
	// and association role objects get added and signaled elsewhere.
	if (m_BaseType != ot_Operation && m_BaseType != ot_Attribute &&
	    m_BaseType != ot_EnumLiteral && m_BaseType != ot_Stereotype &&
	    m_BaseType != ot_Association && m_BaseType != ot_UMLObject) {
		if (m_bInPaste) {
			m_pUMLPackage = NULL;  // forget any old parent
			UMLListView *listView = UMLApp::app()->getListView();
			UMLListViewItem *parentItem = (UMLListViewItem*)listView->currentItem();
			if (parentItem) {
				ListView_Type lvt = parentItem->getType();
				if (lvt == lvt_Package ||
				    lvt == lvt_Class ||
				    lvt == lvt_Interface) {
					UMLObject *o = parentItem->getUMLObject();
					m_pUMLPackage = static_cast<UMLPackage*>( o );
				}
			}
		}

		if (m_pUMLPackage)
			m_pUMLPackage->addObject(this);
		else
			umldoc->addUMLObject(this);
		umldoc->signalUMLObjectCreated(this);
	}
	return load(element);
}

kdbgstream& operator<< (kdbgstream& s, const UMLObject& a) {
	s << a.getName();
	return s;
}

#include "umlobject.moc"
