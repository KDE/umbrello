/*
 *  copyright (C) 2002-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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

UMLEntityAttribute::UMLEntityAttribute( const UMLObject *parent, QString Name, Uml::IDType id,
                                        Uml::Visibility s, QString type, QString iv )
        : UMLClassifierListItem(parent, Name, id) {
    m_InitialValue = iv;
    m_BaseType = Uml::ot_EntityAttribute;
    m_Vis = s;
    m_ParmKind = Uml::pd_In;
    m_indexType = Uml::None;
    m_values = "";
    m_attributes = "";
    m_autoIncrement = false;
    m_null = false;
    if (!type.isEmpty()) {
        UMLDoc* pDoc = UMLApp::app()->getDocument();
        m_pSecondary = pDoc->findUMLObject(type);
        if (m_pSecondary == NULL) {
            m_pSecondary = Object_Factory::createUMLObject(Uml::ot_EntityAttribute, type);
        }
    }
}

UMLEntityAttribute::UMLEntityAttribute(const UMLObject *parent) : UMLClassifierListItem(parent) {
    m_BaseType = Uml::ot_EntityAttribute;
    m_Vis = Uml::Visibility::Private;
    m_ParmKind = Uml::pd_In;
}

UMLEntityAttribute::~UMLEntityAttribute() { }

QString UMLEntityAttribute::getInitialValue() {
    return m_InitialValue;
}

void UMLEntityAttribute::setInitialValue(const QString &iv) {
    if (m_InitialValue != iv) {
        m_InitialValue = iv;
        emit modified();
    }
}

QString UMLEntityAttribute::getAttributes() {
    return m_attributes;
}

void UMLEntityAttribute::setAttributes(const QString& attributes) {
    m_attributes = attributes;
}

QString UMLEntityAttribute::getValues() {
    return m_values;
}

void UMLEntityAttribute::setValues(const QString& values) {
    m_values = values;
}

bool UMLEntityAttribute::getAutoIncrement() {
    return m_autoIncrement;
}

void UMLEntityAttribute::setAutoIncrement(const bool autoIncrement) {
    m_autoIncrement = autoIncrement;
}

Uml::DBIndex_Type UMLEntityAttribute::getIndexType() {
    return m_indexType;
}

void UMLEntityAttribute::setIndexType(const Uml::DBIndex_Type indexType) {
    m_indexType = indexType;
}

bool UMLEntityAttribute::getNull() {
    return m_null;
}

void UMLEntityAttribute::setNull(const bool nullIn) {
    m_null = nullIn;
}

void UMLEntityAttribute::setParmKind(Uml::Parameter_Direction pk) {
    m_ParmKind = pk;
}

Uml::Parameter_Direction UMLEntityAttribute::getParmKind() const {
    return m_ParmKind;
}

QString UMLEntityAttribute::toString(Uml::Signature_Type sig) {
    QString s;
    //FIXME

    if(sig == Uml::st_ShowSig || sig == Uml::st_NoSig) {
        s=m_Vis.toString(true) + " ";
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
    m_SecondaryId = element.attribute( "type", "" );
    // We use the m_SecondaryId as a temporary store for the xmi.id
    // of the attribute type model object.
    // It is resolved later on, when all classes have been loaded.
    // This deferred resolution is required because the xmi.id may
    // be a forward reference, i.e. it may identify a model object
    // that has not yet been loaded.
    if (m_SecondaryId.isEmpty()) {
        // Perhaps the type is stored in a child node:
        QDomNode node = element.firstChild();
        while (!node.isNull()) {
            if (node.isComment()) {
                node = node.nextSibling();
                continue;
            }
            QDomElement tempElement = node.toElement();
            QString tag = tempElement.tagName();
            if (!Uml::tagEq(tag, "type")) {
                node = node.nextSibling();
                continue;
            }
            m_SecondaryId = tempElement.attribute( "xmi.id", "" );
            if (m_SecondaryId.isEmpty())
                m_SecondaryId = tempElement.attribute( "xmi.idref", "" );
            if (m_SecondaryId.isEmpty()) {
                QDomNode inner = node.firstChild();
                QDomElement tmpElem = inner.toElement();
                m_SecondaryId = tmpElem.attribute( "xmi.id", "" );
                if (m_SecondaryId.isEmpty())
                    m_SecondaryId = tmpElem.attribute( "xmi.idref", "" );
            }
            break;
        }
        if (m_SecondaryId.isEmpty()) {
            kError() << "UMLEntityAttribute::load(" << m_Name << "): "
            << "cannot find type." << endl;
            return false;
        }
    }
    m_InitialValue = element.attribute( "initialValue", "" );
    if (m_InitialValue.isEmpty()) {
        // for backward compatibility
        m_InitialValue = element.attribute( "value", "" );
    }
    int indexType = element.attribute( "dbindex_type", "1100" ).toInt();
    m_indexType = ( Uml::DBIndex_Type )indexType;
    m_values = element.attribute( "values", "" );
    m_attributes = element.attribute( "attributes", "" );
    m_autoIncrement = ( bool )element.attribute( "auto_increment", "" ).toInt();
    m_null = ( bool )element.attribute( "allow_null", "" ).toInt();
    return true;
}

bool UMLEntityAttribute::showPropertiesDialogue(QWidget* parent) {
    UMLEntityAttributeDialog dialogue(parent, this);
    return dialogue.exec();
}



