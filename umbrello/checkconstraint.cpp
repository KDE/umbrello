/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

//own header
#include "checkconstraint.h"

// app includes
#include "debug_utils.h"
#include "umlcheckconstraintdialog.h"

/**
 * Sets up a constraint.
 *
 * @param parent    The parent of this UMLCheckConstraint.
 * @param name      The name of this UMLCheckConstraint.
 * @param id        The unique id given to this UMLCheckConstraint.
 */
UMLCheckConstraint::UMLCheckConstraint(UMLObject *parent,
                          const QString& name, Uml::IDType id)
    : UMLEntityConstraint(parent, name, id)
{
    init();
}

/**
 * Sets up a constraint.
 *
 * @param parent    The parent of this UMLCheckConstraint.
 */
UMLCheckConstraint::UMLCheckConstraint(UMLObject *parent)
    : UMLEntityConstraint( parent ) 
{
    init();
}

/**
 * Overloaded '==' operator.
 */
bool UMLCheckConstraint::operator==(const UMLCheckConstraint &rhs) const
{
    if ( this == &rhs )
        return true;

    if ( !UMLObject::operator==( rhs ) )
        return false;

    return true;
}

/**
 * Destructor.
 */
UMLCheckConstraint::~UMLCheckConstraint()
{
}

/**
 * Copy the internal presentation of this object into the UMLCheckConstraint
 * object.
 */
void UMLCheckConstraint::copyInto(UMLObject *lhs) const
{
    UMLCheckConstraint *target = static_cast<UMLCheckConstraint*>(lhs);

    // call the parent first.
    UMLEntityConstraint::copyInto(target);

    // Copy all datamembers
    target->m_CheckCondition = m_CheckCondition;
}

/**
 * Make a clone of the UMLCheckConstraint.
 */
UMLObject* UMLCheckConstraint::clone() const
{
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLCheckConstraint *clone = new UMLCheckConstraint( static_cast<UMLObject*>(parent()) );
    copyInto(clone);
    return clone;
}

/**
 * Returns a string representation of the UMLCheckConstraint.
 *
 * @param sig   If true will show the attribute type and initial value.
 * @return  Returns a string representation of the UMLAttribute.
 */
QString UMLCheckConstraint::toString(Uml::SignatureType sig)
{
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::SigNoVis) {
        s = name() ;
    }

    return s;
}

QString UMLCheckConstraint::getFullyQualifiedName(const QString& separator,
                                                  bool includeRoot ) const
{
    Q_UNUSED(separator); Q_UNUSED(includeRoot);
    return this->name();
}

/**
 * Creates the <UML:UniqueConstraint> XMI element.
 */
void UMLCheckConstraint::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement checkConstraintElement = UMLObject::save("UML:CheckConstraint", qDoc);

    QDomNode checkCondition  = qDoc.createTextNode( m_CheckCondition );
    checkConstraintElement.appendChild( checkCondition );

    qElement.appendChild( checkConstraintElement );
}

/**
 * Display the properties configuration dialog for the attribute.
 */
bool UMLCheckConstraint::showPropertiesDialog(QWidget* parent)
{
    UMLCheckConstraintDialog dialog(parent, this);
    return dialog.exec();
}

/**
 * Loads the <UML:CheckConstraint> XMI element.
 */
bool UMLCheckConstraint::load( QDomElement & element )
{
    QDomNode node = element.firstChild();

    QDomText checkConstraintText = node.toText();
    if ( checkConstraintText.isNull() )
        return false;

    m_CheckCondition = checkConstraintText.data();

    return true;
}

/**
 * Initialises Check Constraint
 */
void UMLCheckConstraint::init() 
{
    m_BaseType = UMLObject::ot_CheckConstraint;
    m_CheckCondition.clear();
}

#include "checkconstraint.moc"
