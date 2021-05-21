/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "template.h"

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umltemplatedialog.h"

/**
 * Sets up a template.
 *
 * @param parent   The parent of this UMLTemplate (i.e. its concept).
 * @param name     The name of this UMLTemplate.
 * @param id       The unique id given to this UMLTemplate.
 * @param type     The type of this UMLTemplate.
 */
UMLTemplate::UMLTemplate(UMLObject *parent, const QString& name,
                         Uml::ID::Type id, const QString& type)
        : UMLClassifierListItem(parent, name, id)
{
    setTypeName(type);
    m_BaseType = UMLObject::ot_Template;
}

/**
 * Sets up a template.
 *
 * @param parent    The parent of this UMLTemplate (i.e. its concept).
 */
UMLTemplate::UMLTemplate(UMLObject *parent)
        : UMLClassifierListItem(parent)
{
    m_BaseType = UMLObject::ot_Template;
}

/**
 * Destructor.
 */
UMLTemplate::~UMLTemplate()
{
}

QString UMLTemplate::toString(Uml::SignatureType::Enum sig, bool withStereotype)
{
    Q_UNUSED(sig);
    QString s;

    if (m_pSecondary == 0 || m_pSecondary->name() == QLatin1String("class")) {
        s = name();
    } else {
        s = name() + QLatin1String(" : ") + m_pSecondary->name();
    }
    if (withStereotype) {
        QString st = stereotype(true);
        if (!st.isEmpty())
            s += QLatin1String(" ") + st;
    }
    return s;
}

/**
 * Overrides method from UMLClassifierListItem.
 * Returns the type name of the UMLTemplate.
 * If the template parameter is a class, there is no separate
 * type object. In this case, getTypeName() returns "class".
 *
 * @return  The type name of the UMLClassifierListItem.
 */
QString UMLTemplate::getTypeName() const
{
    if (m_pSecondary == 0)
        return QLatin1String("class");
    return m_pSecondary->name();
}

/**
 * Overloaded '==' operator.
 */
bool UMLTemplate::operator==(const UMLTemplate &rhs) const
{
    if (this == &rhs) {
        return true;
    }
    if (!UMLObject::operator==(rhs)) {
        return false;
    }
    if (m_pSecondary != rhs.m_pSecondary) {
        return false;
    }
    return true;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLTemplate::copyInto(UMLObject *lhs) const
{
    UMLClassifierListItem::copyInto(lhs);
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLTemplate::clone() const
{
    UMLTemplate *clone = new UMLTemplate(umlParent()->asUMLTemplate());
    copyInto(clone);

    return clone;
}

/**
 * Writes the <UML:TemplateParameter> XMI element.
 */
void UMLTemplate::saveToXMI1(QXmlStreamWriter& writer)
{
    //FIXME: uml13.dtd compliance
    UMLObject::save1(QLatin1String("UML:TemplateParameter"), writer);
    if (m_pSecondary)
        writer.writeAttribute(QLatin1String("type"), Uml::ID::toString(m_pSecondary->id()));
    writer.writeEndElement();
}

/**
 * Loads the <UML:TemplateParameter> XMI element.
 */
bool UMLTemplate::load1(QDomElement& element)
{
    m_SecondaryId = element.attribute(QLatin1String("type"));
    return true;
}

/**
 * Display the properties configuration dialog for the template.
 *
 * @return  Success status.
 */
bool UMLTemplate::showPropertiesDialog(QWidget* parent)
{
    UMLTemplateDialog dialog(parent, this);
    return dialog.exec();
}
