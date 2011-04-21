/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "template.h"

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umltemplatedialog.h"

UMLTemplate::UMLTemplate(UMLObject *parent, const QString& name,
                         Uml::IDType id, const QString& type)
        : UMLClassifierListItem( parent, name, id )
{
    setTypeName( type );
    m_BaseType = UMLObject::ot_Template;
}

UMLTemplate::UMLTemplate(UMLObject *parent)
        : UMLClassifierListItem( parent )
{
    m_BaseType = UMLObject::ot_Template;
}

UMLTemplate::~UMLTemplate()
{
}

QString UMLTemplate::toString(Uml::SignatureType sig)
{
    Q_UNUSED(sig);
    if (m_pSecondary == NULL || m_pSecondary->name() == "class") {
        return name();
    } else {
        return name() + " : " + m_pSecondary->name();
    }
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
    if (m_pSecondary == NULL)
        return "class";
    return m_pSecondary->name();
}

bool UMLTemplate::operator==(const UMLTemplate &rhs) const
{
    if (this == &rhs) {
        return true;
    }
    if ( !UMLObject::operator==( rhs ) ) {
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
    UMLTemplate *clone = new UMLTemplate( (UMLTemplate*) parent());
    copyInto(clone);

    return clone;
}


/**
 * Writes the <UML:TemplateParameter> XMI element.
 */
void UMLTemplate::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    //FIXME: uml13.dtd compliance
    QDomElement attributeElement = UMLObject::save("UML:TemplateParameter", qDoc);
    if (m_pSecondary)
        attributeElement.setAttribute("type", ID2STR(m_pSecondary->id()));
    qElement.appendChild(attributeElement);
}

/**
 * Loads the <UML:TemplateParameter> XMI element.
 */
bool UMLTemplate::load(QDomElement& element)
{
    m_SecondaryId = element.attribute("type", "");
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
