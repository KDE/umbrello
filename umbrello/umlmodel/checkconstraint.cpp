/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
                          const QString& name, Uml::ID::Type id)
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
    : UMLEntityConstraint(parent) 
{
    init();
}

/**
 * Overloaded '==' operator.
 */
bool UMLCheckConstraint::operator==(const UMLCheckConstraint &rhs) const
{
    if (this == &rhs)
        return true;

    if (!UMLObject::operator==(rhs))
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
    UMLCheckConstraint *target = lhs->asUMLCheckConstraint();

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
    UMLCheckConstraint *clone = new UMLCheckConstraint(umlParent());
    copyInto(clone);
    return clone;
}

/**
 * Returns a string representation of the UMLCheckConstraint.
 * Reimplements function from @ref UMLClassifierListItem.
 */
QString UMLCheckConstraint::toString(Uml::SignatureType::Enum sig, bool) const
{
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::SigNoVis) {
        s = name() ;
    }

    return s;
}

QString UMLCheckConstraint::getFullyQualifiedName(const QString& separator,
                                                  bool includeRoot) const
{
    Q_UNUSED(separator); Q_UNUSED(includeRoot);
    return this->name();
}

/**
 * Creates the <UML:UniqueConstraint> XMI element.
 */
void UMLCheckConstraint::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("CheckConstraint"));

    writer.writeTextElement(QString(), m_CheckCondition);

    UMLObject::save1end(writer);
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
bool UMLCheckConstraint::load1(QDomElement & element)
{
    QDomNode node = element.firstChild();

    QDomText checkConstraintText = node.toText();
    if (checkConstraintText.isNull())
        m_CheckCondition = QString();
    else
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

