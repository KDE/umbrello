/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include "umldoc.h"
#include "umlobject.h"
#include "umldoc.h"
#include "package.h"

UMLObject::UMLObject(UMLObject * parent, const QString &name, int id)
  : QObject(parent, "UMLObject" ) {
	init();
	m_nId = id;
	m_Name = name;
}

UMLObject::UMLObject(UMLDoc* parent, const QString &name, int id)
  :  QObject(parent,"UMLObject") {
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
		UMLDoc* umldoc = dynamic_cast<UMLDoc *>( parent() );
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

bool UMLObject::saveToXMI( QDomDocument & /*qDoc*/, QDomElement & qElement ) {
	/*
	  Call after required actions in child class.
	  Type must be set in the child class.
	*/
	qElement.setAttribute( "xmi.id", m_nId );
	qElement.setAttribute( "name", m_Name );
	if (! m_Doc.isEmpty())
		qElement.setAttribute( "documentation", m_Doc );
	if (m_pUMLPackage)
		qElement.setAttribute( "packageid", m_pUMLPackage->getID() );
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
	else
		qElement.setAttribute( "isAbstract", "false" );
	if (m_bStatic)
 		qElement.setAttribute( "ownerScope", "classifier" );
	else
 		qElement.setAttribute( "ownerScope", "instance" );
	return true;
}

bool UMLObject::loadFromXMI( QDomElement & element ) {
	QString id = element.attribute( "xmi.id", "-1" );
	m_Name = element.attribute( "name", "" );
	m_Doc = element.attribute( "documentation", "" );

	if (element.hasAttribute("scope")) {        // for bkwd compat.
		QString scope = element.attribute( "scope", "200" );
		m_Scope = (Scope)scope.toInt();
	} else {
		QString visibility = element.attribute( "visibility", "public" );
		if (visibility == "private")
			m_Scope = Uml::Private;
		else if (visibility == "protected")
			m_Scope = Uml::Protected;
		else
			m_Scope = Uml::Public;
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

	m_nId = id.toInt();
	if( m_nId == -1 || m_Scope == -1 )
		return false;

	QString pkgId = element.attribute( "packageid", "-1" );
	if (pkgId == "-1") {
		// Old files used "package" instead so test for it.
		QString pkgName = element.attribute( "package", "" );
		if (pkgName != "") {
			setPackage( pkgName );
		}
	} else {
		UMLDoc* umldoc = dynamic_cast<UMLDoc *>( parent() );
		if (umldoc == NULL) {
			kdDebug() << "UMLObject::loadFromXMI: cannot set package on "
				  << m_Name << endl;
			return true;  // soft error
		}
		UMLObject *pkgObj = umldoc->findUMLObject( pkgId.toInt() );
		if (pkgObj == NULL) {
			kdDebug() << "UMLObject::loadFromXMI: cannot resolve packageid "
				  << pkgId.toInt() << endl;
			return true;  // soft error
		}
		m_pUMLPackage = dynamic_cast<UMLPackage *>(pkgObj);
		if (m_pUMLPackage == NULL) {
			kdDebug() << "UMLObject::loadFromXMI: object of packageid "
				  << pkgId.toInt() << " is not a package" << endl;
			return true;  // soft error
		}
	}
	if (m_pUMLPackage)
		m_pUMLPackage->addObject(this);
	return true;
}


#include "umlobject.moc"
