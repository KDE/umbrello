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

UMLObject::UMLObject(QObject * parent,  QString Name, int id) : QObject(parent, "AnUMLObject") {
	m_nId = id;
	m_BaseType = ot_UMLObject;
	m_Name = Name;
	m_Scope = Public;
	m_Stereotype = m_Package = "";
	m_bAbstract = false;
	m_bStatic = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject::UMLObject(QObject * parent) : QObject(parent) {
	m_BaseType = ot_UMLObject;
	m_nId = -1;
	m_Scope = Public;
	m_Name = "";
	m_Stereotype = m_Package = "";
	m_bAbstract = false;
	m_bStatic = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject::~UMLObject() {
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

	if( m_Package != rhs.m_Package )
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
	qElement.setAttribute( "package", m_Package );
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
	m_Package = element.attribute( "package", "" );
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
	m_Package = _name;
	emit modified();
}

QString UMLObject::getStereotype() {
	return m_Stereotype;
}

QString UMLObject::getPackage() {
	return m_Package;
}

#include "umlobject.moc"
