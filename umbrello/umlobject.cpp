/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "umlobject.h"
#include "umldoc.h"
#include "package.h"

UMLObject::UMLObject(QObject * parent, QString Name, int id)
  : QObject(parent, "AnUMLObject") {
	init();
	m_nId = id;
	m_Name = Name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject::UMLObject(QObject * parent) : QObject(parent) {
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
	m_Stereotype = m_Doc = "";
	m_bAbstract = false;
	m_bStatic = false;
}

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

bool UMLObject::saveToXMI( QDomDocument & /*qDoc*/, QDomElement & qElement ) {
	/*
	  Call after required actions in child class.
	  Type must be set in the child class.
	*/
	qElement.setAttribute( "xmi.id", m_nId );
	qElement.setAttribute( "name", m_Name );
	qElement.setAttribute( "documentation", m_Doc );
	if (m_pUMLPackage)
		qElement.setAttribute( "packageid", m_pUMLPackage->getID() );
	qElement.setAttribute( "scope", m_Scope );
	qElement.setAttribute( "stereotype", m_Stereotype );
	qElement.setAttribute( "abstract", m_bAbstract );
 	qElement.setAttribute( "static", m_bStatic );
	return true;
}

bool UMLObject::loadFromXMI( QDomElement & element ) {
	QString id = element.attribute( "xmi.id", "-1" );
	m_Name = element.attribute( "name", "" );
	m_Doc = element.attribute( "documentation", "" );
	QString scope = element.attribute( "scope", "-1" );
	m_Stereotype = element.attribute( "stereotype", "" );
	QString abstract = element.attribute( "abstract", "0" );
	QString staticScope = element.attribute( "static", "0" );

	m_nId = id.toInt();
	m_Scope = (Scope)scope.toInt();
	m_bAbstract = (bool)abstract.toInt();
	m_bStatic = (bool)staticScope.toInt();
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
		if (pkgObj)
			m_pUMLPackage = static_cast<UMLPackage *>(pkgObj);
	}
	return true;
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
		pkgObj = umldoc->findUMLObject(ot_Package, _name);
		if (pkgObj == NULL) {
			kdDebug() << "UMLObject::setPackage: creating UMLPackage "
				  << _name << " for " << m_Name << endl;
			pkgObj = umldoc->createUMLObject(ot_Package, _name);
		}
	}
	m_pUMLPackage = static_cast<UMLPackage *>(pkgObj);
	emit modified();
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

#include "umlobject.moc"
