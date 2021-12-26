/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "entityattribute.h"

// app includes
#include "debug_utils.h"
//#include "umlcanvasobject.h"
#include "umldoc.h"
#include "uml.h"
#include "umlentityattributedialog.h"
#include "object_factory.h"

// qt includes
#include <QRegExp>

/**
 * Sets up an entityattribute.
 * @param parent    The parent of this UMLEntityAttribute.
 * @param name      The name of this UMLEntityAttribute.
 * @param id        The unique id given to this UMLEntityAttribute.
 * @param s         The visibility of the UMLEntityAttribute.
 * @param type      The type of this UMLEntityAttribute.
 * @param iv        The initial value of the entityattribute.
 */
UMLEntityAttribute::UMLEntityAttribute(UMLObject *parent, const QString& name,
                                        Uml::ID::Type id, Uml::Visibility::Enum s,
                                        UMLObject *type, const QString& iv)
  : UMLAttribute(parent, name, id, s, type, iv)
{
    init();
}

/**
 * Sets up an entityattribute.
 * @param parent    The parent of this UMLEntityAttribute.
 */
UMLEntityAttribute::UMLEntityAttribute(UMLObject *parent)
 : UMLAttribute(parent)
{
    init();
}

/**
 * Destructor.
 */
UMLEntityAttribute::~UMLEntityAttribute()
{
}

/**
 * Initialize members of this class.
 * Auxiliary method used by constructors.
 */
void UMLEntityAttribute::init()
{
    m_BaseType = UMLObject::ot_EntityAttribute;
    m_indexType = UMLEntityAttribute::None;
    m_autoIncrement = false;
    m_null = false;
}

/**
 * Returns the value of the UMLEntityAttribute's attributes property.
 * @return  The value of the UMLEntityAttribute's attributes property.
 */
QString UMLEntityAttribute::getAttributes() const
{
    return m_attributes;
}

/**
 * Sets the UMLEntityAttribute's attributes property.
 * @param attributes  The new value for the attributes property.
 */
void UMLEntityAttribute::setAttributes(const QString& attributes)
{
    m_attributes = attributes;
}

/**
 * Returns the UMLEntityAttribute's length/values property.
 * @return  The new value of the length/values property.
 */
QString UMLEntityAttribute::getValues() const
{
    return m_values;
}

/**
 * Sets the UMLEntityAttribute's length/values property.
 * @param values    The new value of the length/values property.
 */
void UMLEntityAttribute::setValues(const QString& values)
{
    m_values = values;
}

/**
 * Returns the UMLEntityAttribute's auto_increment boolean
 * @return  The UMLEntityAttribute's auto_increment boolean
 */
bool UMLEntityAttribute::getAutoIncrement() const
{
    return m_autoIncrement;
}

/**
 * Sets the UMLEntityAttribute's auto_increment boolean
 * @param autoIncrement  The UMLEntityAttribute's auto_increment boolean
 */
void UMLEntityAttribute::setAutoIncrement(const bool autoIncrement)
{
    m_autoIncrement = autoIncrement;
}

/**
 * Returns the UMLEntityAttribute's index type property.
 * @return  The value of the UMLEntityAttribute's index type property.
 */
UMLEntityAttribute::DBIndex_Type UMLEntityAttribute::indexType() const
{
    return m_indexType;
}

/**
 * Sets the initial value of the UMLEntityAttribute's index type property.
 * @param indexType  The initial value of the UMLEntityAttribute's index type property.
 */
void UMLEntityAttribute::setIndexType(const UMLEntityAttribute::DBIndex_Type indexType)
{
    m_indexType = indexType;
}

/**
 * Returns the UMLEntityAttribute's allow null value.
 * @return  The UMLEntityAttribute's allow null value.
 */
bool UMLEntityAttribute::getNull() const
{
    return m_null;
}

/**
 * Sets the initial value of the UMLEntityAttribute's allow null value.
 * @param nullIn   The initial value of the UMLEntityAttribute's allow null value.
 */
void UMLEntityAttribute::setNull(const bool nullIn)
{
    m_null = nullIn;
}

/**
 * Returns a string representation of the UMLEntityAttribute.
 * @param sig   If true will show the entityattribute type and initial value.
 * @return  Returns a string representation of the UMLEntityAttribute.
 */
QString UMLEntityAttribute::toString(Uml::SignatureType::Enum sig,
                                     bool withStereotype) const
{
    Q_UNUSED(withStereotype);
    QString s;
    //FIXME

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::NoSig) {
        s = Uml::Visibility::toString(m_visibility, true) + QLatin1Char(' ');
    }

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::SigNoVis) {
        QString string = s + name() + QLatin1String(" : ") + getTypeName();
        if(m_InitialValue.length() > 0)
            string += QLatin1String(" = ") + m_InitialValue;
        return string;
    } else
        return s + name();
}

/**
 * Overloaded '==' operator
 */
bool UMLEntityAttribute::operator==(const UMLEntityAttribute &rhs) const
{
    if(this == &rhs)
        return true;

    if(!UMLObject::operator==(rhs))
        return false;

    // The type name is the only distinguishing criterion.
    // (Some programming languages might support more, but others don't.)
    if (m_pSecondary != rhs.m_pSecondary)
        return false;

    return true;
}

/**
 * Copy the internal presentation of this object into the UMLEntityAttribute
 * object.
 */
void UMLEntityAttribute::copyInto(UMLObject *lhs) const
{
    UMLEntityAttribute *target = lhs->asUMLEntityAttribute();

    // call the parent first.
    UMLClassifierListItem::copyInto(target);

    // Copy all datamembers
    target->m_pSecondary = m_pSecondary;
    target->m_SecondaryId = m_SecondaryId;
    target->m_InitialValue = m_InitialValue;
    target->m_ParmKind = m_ParmKind;
}

/**
 * Make a clone of the UMLEntityAttribute.
 */
UMLObject* UMLEntityAttribute::clone() const
{
    UMLEntityAttribute* clone = new UMLEntityAttribute(umlParent()->asUMLEntityAttribute());
    copyInto(clone);

    return clone;
}

/**
 * Creates the <UML:EntityAttribute> XMI element.
 */
void UMLEntityAttribute::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QLatin1String("EntityAttribute"), QLatin1String("ownedAttribute"));
    if (m_pSecondary == 0) {
        uDebug() << name() << ": m_pSecondary is 0, using local name " << m_SecondaryId;
        writer.writeAttribute(QLatin1String("type"), m_SecondaryId);
    } else {
        writer.writeAttribute(QLatin1String("type"), Uml::ID::toString(m_pSecondary->id()));
    }
    writer.writeAttribute(QLatin1String("initialValue"), m_InitialValue);
    writer.writeAttribute(QLatin1String("dbindex_type"), QString::number(m_indexType));
    writer.writeAttribute(QLatin1String("values"), m_values);
    writer.writeAttribute(QLatin1String("attributes"), m_attributes);
    writer.writeAttribute(QLatin1String("auto_increment"), QString::number(m_autoIncrement));
    writer.writeAttribute(QLatin1String("allow_null"), QString::number(m_null));
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:EntityAttribute> XMI element.
 */
bool UMLEntityAttribute::load1(QDomElement & element)
{
    if (! UMLAttribute::load1(element))
        return false;
    int indexType = element.attribute(QLatin1String("dbindex_type"), QLatin1String("1100")).toInt();
    m_indexType = (UMLEntityAttribute::DBIndex_Type)indexType;
    m_values = element.attribute(QLatin1String("values"));
    m_attributes = element.attribute(QLatin1String("attributes"));
    m_autoIncrement = (bool)element.attribute(QLatin1String("auto_increment")).toInt();
    m_null = (bool)element.attribute(QLatin1String("allow_null")).toInt();
    return true;
}

/**
 * Display the properties configuration dialog for the entityattribute.
 */
bool UMLEntityAttribute::showPropertiesDialog(QWidget* parent)
{
    UMLEntityAttributeDialog dialog(parent, this);
    return dialog.exec();
}

