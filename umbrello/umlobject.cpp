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
#include "package.h"

UMLObject::UMLObject(UMLObject * parent, const QString &name, int id)
  : QObject(parent, "UMLObject" ) {
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
UMLObject::UMLObject(UMLObject * parent) : QObject(parent) {
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
	m_Stereotype = "";
	m_Doc = "";
	m_bAbstract = false;
	m_bStatic = false;
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
	if( m_Name != rhs.m_Name )
		return false;

	if( m_BaseType != rhs.m_BaseType )
		return false;

	if( m_Doc != rhs.m_Doc )
		return false;

	if( m_pUMLPackage != rhs.m_pUMLPackage )
		return false;

	if( m_Scope != rhs.m_Scope )
		return false;

	if( m_Stereotype != rhs.m_Stereotype )
		return false;

	if( m_bAbstract != rhs.m_bAbstract )
		return false;

	return true;

}

bool UMLObject::getAbstract() const{
	return m_bAbstract;
}

void UMLObject::setAbstract(bool bAbstract) {
	m_bAbstract = bAbstract;
	emit modified();
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

void UMLObject::setStereotype(QString _name) {
	m_Stereotype = _name;
	emit modified();
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

QString UMLObject::getStereotype() {
	return m_Stereotype;
}

QString UMLObject::getPackage() {
	if (m_pUMLPackage == NULL)
		return "";
	return m_pUMLPackage->getName();
}

UMLPackage* UMLObject::getUMLPackage() {
	return m_pUMLPackage;
}

QString UMLObject::getAuxId() const {
	return m_AuxId;
}

bool UMLObject::saveToXMI( QDomDocument & /*qDoc*/, QDomElement & qElement ) {
	/*
	  Call after required actions in child class.
	  Type must be set in the child class.
	*/
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
	if (! m_Stereotype.isEmpty())
		qElement.setAttribute( "stereotype", m_Stereotype );
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
	return true;
}

bool UMLObject::loadFromXMI( QDomElement & element ) {
	UMLDoc* umldoc = UMLApp::app()->getDocument();
	if (umldoc == NULL) {
		kdError() << "UMLObject::loadFromXMI: umldoc is NULL" << endl;
		return false;
	}
	QString id = element.attribute( "xmi.id", "" );
	if (id.isEmpty() || id == "-1") {
		kdError() << "nonexistent or illegal xmi.id" << endl;
		return false;
	}
	if (id.contains(QRegExp("\\D"))) {
		m_AuxId = id;
		m_nId = umldoc->getUniqueID();
	} else {
		m_nId = id.toInt();
	}

	m_Name = element.attribute( "name", "" );
	if (element.hasAttribute("documentation"))  // for bkwd compat.
		m_Doc = element.attribute( "documentation", "" );
	else
		m_Doc = element.attribute( "comment", "" );  //CHECK: need a UML:Comment?

	m_Scope = Uml::Public;
	if (element.hasAttribute("scope")) {        // for bkwd compat.
		QString scope = element.attribute( "scope", "200" );
		int nScope = scope.toInt();
		if (nScope >= Uml::Public && nScope <= Uml::Protected)
			m_Scope = (Scope)nScope;
		else
			kdError() << "illegal scope" << endl;  // soft error
	} else {
		QString visibility = element.attribute( "visibility", "public" );
		if (visibility == "private")
			m_Scope = Uml::Private;
		else if (visibility == "protected")
			m_Scope = Uml::Protected;
		else if (visibility != "public")
			kdError() << "illegal scope" << endl;  // soft error
	}

	m_Stereotype = element.attribute( "stereotype", "" );

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
		if (pkgObj == NULL) {
			kdDebug() << "UMLObject::loadFromXMI: cannot resolve packageid "
				  << pkgId << endl;
			return true;  // soft error
		}
		m_pUMLPackage = dynamic_cast<UMLPackage *>(pkgObj);
		if (m_pUMLPackage == NULL) {
			kdDebug() << "UMLObject::loadFromXMI: object of packageid "
				  << pkgId << " is not a package" << endl;
			return true;  // soft error
		}
	}
	/**** End of XMI_FLAT_PACKAGES and old files handling ****************/

	if (m_pUMLPackage)
		m_pUMLPackage->addObject(this);
	//umldoc->signalUMLObjectCreated(this);
	return true;
}


#include "umlobject.moc"
