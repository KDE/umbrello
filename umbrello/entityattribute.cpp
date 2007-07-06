/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "entityattribute.h"
// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "umlcanvasobject.h"
#include "umldoc.h"
#include "uml.h"
#include "dialogs/umlentityattributedialog.h"
#include "object_factory.h"

UMLEntityAttribute::UMLEntityAttribute( const UMLObject *parent, const QString& name,
                                        Uml::IDType id, Uml::Visibility s,
                                        UMLObject *type, const QString& iv )
        : UMLAttribute(parent, name, id, s, type, iv) {
    init();
    if (m_pSecondary) {
        m_pSecondary->setBaseType(Uml::ot_Entity);
    }
}

UMLEntityAttribute::UMLEntityAttribute(const UMLObject *parent) : UMLAttribute(parent) {
    init();
}

UMLEntityAttribute::~UMLEntityAttribute() { }

void UMLEntityAttribute::init() {
    m_BaseType = Uml::ot_EntityAttribute;
    m_indexType = Uml::None;
    m_autoIncrement = false;
    m_null = false;
}

QString UMLEntityAttribute::getAttributes() const{
    return m_attributes;
}

void UMLEntityAttribute::setAttributes(const QString& attributes) {
    m_attributes = attributes;
}

QString UMLEntityAttribute::getValues() const{
    return m_values;
}

void UMLEntityAttribute::setValues(const QString& values) {
    m_values = values;
}

bool UMLEntityAttribute::getAutoIncrement() const{
    return m_autoIncrement;
}

void UMLEntityAttribute::setAutoIncrement(const bool autoIncrement) {
    m_autoIncrement = autoIncrement;
}

Uml::DBIndex_Type UMLEntityAttribute::getIndexType() const{
    return m_indexType;
}

void UMLEntityAttribute::setIndexType(const Uml::DBIndex_Type indexType) {
    m_indexType = indexType;
}

bool UMLEntityAttribute::getNull() const{
    return m_null;
}

void UMLEntityAttribute::setNull(const bool nullIn) {
    m_null = nullIn;
}

QString UMLEntityAttribute::toString(Uml::Signature_Type sig) {
    QString s;
    //FIXME

    if(sig == Uml::st_ShowSig || sig == Uml::st_NoSig) {
        s=m_Vis.toString(true) + ' ';
    } else
        s = "";

    if(sig == Uml::st_ShowSig || sig == Uml::st_SigNoVis) {
        QString string = s + getName() + " : " + getTypeName();
        if(m_InitialValue.length() > 0)
            string += " = " + m_InitialValue;
        return string;
    } else
        return s + getName();
}

bool UMLEntityAttribute::operator==( UMLEntityAttribute &rhs) {
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    // The type name is the only distinguishing criterion.
    // (Some programming languages might support more, but others don't.)
    if (m_pSecondary != rhs.m_pSecondary)
        return false;

    return true;
}

void UMLEntityAttribute::copyInto(UMLEntityAttribute *rhs) const
{
    // call the parent first.
    UMLClassifierListItem::copyInto(rhs);

    // Copy all datamembers
    rhs->m_pSecondary = m_pSecondary;
    rhs->m_SecondaryId = m_SecondaryId;
    rhs->m_InitialValue = m_InitialValue;
    rhs->m_ParmKind = m_ParmKind;
}

UMLObject* UMLEntityAttribute::clone() const
{
    UMLEntityAttribute* clone = new UMLEntityAttribute( (UMLEntityAttribute*)parent() );
    copyInto(clone);

    return clone;
}


void UMLEntityAttribute::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement entityattributeElement = UMLObject::save("UML:EntityAttribute", qDoc);
    if (m_pSecondary == NULL) {
        kDebug() << "UMLEntityAttribute::saveToXMI(" << m_Name
        << "): m_pSecondary is NULL, using local name "
        << m_SecondaryId << endl;
        entityattributeElement.setAttribute( "type", m_SecondaryId );
    } else {
        entityattributeElement.setAttribute( "type", ID2STR(m_pSecondary->getID()) );
    }
    entityattributeElement.setAttribute( "initialValue", m_InitialValue );
    entityattributeElement.setAttribute( "dbindex_type", m_indexType );
    entityattributeElement.setAttribute( "values", m_values );
    entityattributeElement.setAttribute( "attributes", m_attributes );
    entityattributeElement.setAttribute( "auto_increment", m_autoIncrement );
    entityattributeElement.setAttribute( "allow_null", m_null );
    qElement.appendChild( entityattributeElement );
}

bool UMLEntityAttribute::load( QDomElement & element ) {
    if (! UMLAttribute::load(element))
        return false;
    int indexType = element.attribute( "dbindex_type", "1100" ).toInt();
    m_indexType = ( Uml::DBIndex_Type )indexType;
    m_values = element.attribute( "values", "" );
    m_attributes = element.attribute( "attributes", "" );
    m_autoIncrement = ( bool )element.attribute( "auto_increment", "" ).toInt();
    m_null = ( bool )element.attribute( "allow_null", "" ).toInt();
    return true;
}

bool UMLEntityAttribute::showPropertiesDialog(QWidget* parent) {
    UMLEntityAttributeDialog dialog(parent, this);
    return dialog.exec();
}

#include "entityattribute.moc"

