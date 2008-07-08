/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

//own header
#include "uniqueconstraint.h"
// qt/kde includes
#include <kdebug.h>
// app includes
#include "entity.h"
#include "entityattribute.h"
#include "umldoc.h"
#include "uml.h"
#include "dialogs/umlattributedialog.h"
#include "dialogs/umluniqueconstraintdialog.h"
#include "object_factory.h"


UMLUniqueConstraint::UMLUniqueConstraint(UMLObject *parent,
                          const QString& name, Uml::IDType id)
    : UMLEntityConstraint(parent, name, id)
{
    init();
}

UMLUniqueConstraint::UMLUniqueConstraint(UMLObject *parent)
    : UMLEntityConstraint( parent )
{
    init();
}

bool UMLUniqueConstraint::operator==(const  UMLUniqueConstraint &rhs)
{
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    return true;
}

UMLUniqueConstraint::~UMLUniqueConstraint()
{
}

void UMLUniqueConstraint::copyInto(UMLObject *lhs) const
{
    UMLUniqueConstraint *target = static_cast<UMLUniqueConstraint*>(lhs);

    // call the parent first.
    UMLEntityConstraint::copyInto(target);

    // Copy all datamembers
    target->m_EntityAttributeList.clear();
    bool valid = true;
    foreach( UMLEntityAttribute* attr, m_EntityAttributeList ) {
       if ( !valid )
           break;
       valid = target->addEntityAttribute( attr );
    }

    if ( !valid ) {
        target->m_EntityAttributeList.clear();
        uDebug() <<"Copying Attributes Failed : Target list cleared instead";
    }
}

UMLObject* UMLUniqueConstraint::clone() const
{
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLUniqueConstraint *clone = new UMLUniqueConstraint( static_cast<UMLObject*>(parent()) );
    copyInto(clone);
    return clone;
}

QString UMLUniqueConstraint::toString(Uml::Signature_Type sig )
{
     QString s;

    if(sig == Uml::st_ShowSig || sig == Uml::st_ShowSig || sig == Uml::st_SigNoVis) {
        s = getName() + ':';

        if ( static_cast<UMLEntity*>( parent() )->isPrimaryKey( this ) ) {
           s += "Primary Key (";
        } else {
           s += "Unique (";
        }

        bool first = true;
        foreach( UMLEntityAttribute* att, m_EntityAttributeList ) {
            if ( first ) {
               first = false;
            } else
                s += ',';
            s += att->getName();
        }
        s +=  ')' ;
    }

    return s;
}

QString UMLUniqueConstraint::getFullyQualifiedName(const QString& separator,
                                                   bool includeRoot) const
{
    Q_UNUSED(separator); Q_UNUSED(includeRoot);
    return this->getName();
}

void UMLUniqueConstraint::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement uniqueConstraintElement = UMLObject::save("UML:UniqueConstraint", qDoc);

    UMLEntity* parentEnt = static_cast<UMLEntity*>( parent() );
    if ( parentEnt->isPrimaryKey( this ) ) {
        uniqueConstraintElement.setAttribute( "isPrimary", "1" );
    } else {
        uniqueConstraintElement.setAttribute( "isPrimary", "0" );
    }

    foreach( UMLEntityAttribute* att, m_EntityAttributeList ) {
        att->saveToXMI(qDoc,uniqueConstraintElement);
    }

    qElement.appendChild( uniqueConstraintElement );
}

bool UMLUniqueConstraint::showPropertiesDialog(QWidget* parent)
{
    UMLUniqueConstraintDialog dialog(parent, this);
    return dialog.exec();
}

bool UMLUniqueConstraint::load( QDomElement & element )
{

    int isPrimary = element.attribute( "isPrimary", "0" ).toInt();
    UMLEntity* parentEnt = static_cast<UMLEntity*>(parent());

    if ( isPrimary == 1 ) {
        parentEnt->setAsPrimaryKey(this);
    }

    QDomNode node = element.firstChild();
    while ( !node.isNull() ) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (Uml::tagEq(tag, "EntityAttribute")) {

            QString attName = tempElement.attribute("name","" );
            UMLObject* obj = parentEnt->findChildObject( attName );

            UMLEntityAttribute* entAtt = static_cast<UMLEntityAttribute*>(obj);
            if ( entAtt == NULL )
                continue;

            m_EntityAttributeList.append(entAtt);

        } else {
            uWarning() << "unknown child type in UMLUniqueConstraint::load";
        }

        node = node.nextSibling();
    }

    return true;
}


bool UMLUniqueConstraint::hasEntityAttribute(UMLEntityAttribute* attr)
{
    if ( m_EntityAttributeList.indexOf( attr ) == -1 ) {
        //not present
        return false;
    }

    // else present
    return true;
}

bool UMLUniqueConstraint::addEntityAttribute(UMLEntityAttribute* attr)
{
    UMLEntity *owningParent = dynamic_cast<UMLEntity*>(parent());

    if ( hasEntityAttribute( attr ) ) {
        uDebug() << "Unique Constraint already contains"<<attr->getName();
        return false;

    }
    if (owningParent == NULL) {
        uError() << m_Name
            << "): parent " << owningParent->getName()
            << " is not a UMLEntity";
        return false;
    }

    if ( owningParent->findChildObjectById( attr->getID() ) == NULL ) {
        uError()
            << " parent " << owningParent->getName()
            << " does not contain attribute " << attr->getName();
        return false;
    }

    //else add the attribute to the Entity Attribute List
    m_EntityAttributeList.append( attr );

    return true;
}

bool UMLUniqueConstraint::removeEntityAttribute(UMLEntityAttribute* attr)
{
    UMLEntity *owningParent = dynamic_cast<UMLEntity*>(parent());

    if (owningParent == NULL) {
        uError() << m_Name
            << "): parent " << owningParent->getName()
            << " is not a UMLEntity";
        return false;
    }

    /*
     * The attribute may already be removed from the Entity when this function
     * is called. So checking this is not right
     *
     * if ( owningParent->findChildObjectById( attr->getID() ) == NULL ) {
     *    uError()
     *        << " parent " << owningParent->getName()
     *        << " does not contain attribute " << attr->getName();
     *    return false;
     * }
     */

    //else remove the attribute from the Entity Attribute List
    if ( m_EntityAttributeList.removeAll( attr ) ) {
        return true;
    }

    return false;
}

void UMLUniqueConstraint::init()
{
    m_BaseType = Uml::ot_UniqueConstraint;
}

void UMLUniqueConstraint::clearAttributeList()
{
    m_EntityAttributeList.clear();
}

