/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "enumliteral.h"

// kde includes
#if QT_VERSION < 0x050000
#include <kinputdialog.h>
#endif
#include <KLocalizedString>

// qt includes
#if QT_VERSION >= 0x050000
#include <QInputDialog>
#endif

/**
 * Sets up an enum literal.
 * @param parent    The parent of this UMLEnumLiteral.
 * @param name      The name of this UMLEnumLiteral.
 * @param id        The unique id given to this UMLEnumLiteral.
 */
UMLEnumLiteral::UMLEnumLiteral(UMLObject *parent,
                               const QString& name, Uml::ID::Type id)
  : UMLClassifierListItem(parent, name, id)
{
    m_BaseType = UMLObject::ot_EnumLiteral;
}

/**
 * Sets up an enum literal.
 * @param parent    The parent of this UMLEnumLiteral.
 */
UMLEnumLiteral::UMLEnumLiteral(UMLObject *parent)
  : UMLClassifierListItem(parent)
{
    m_BaseType = UMLObject::ot_EnumLiteral;
}

/**
 * Destructor.
 */
UMLEnumLiteral::~UMLEnumLiteral()
{
}

/**
 * Overloaded '==' operator
 */
bool UMLEnumLiteral::operator==(const UMLEnumLiteral& rhs) const
{
    if (this == &rhs)  {
        return true;
    }
    if (!UMLObject::operator==(rhs))  {
        return false;
    }
    return true;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLEnumLiteral::copyInto(UMLObject *lhs) const
{
    UMLClassifierListItem::copyInto(lhs);
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLEnumLiteral::clone() const
{
    UMLEnumLiteral *clone = new UMLEnumLiteral((UMLObject *) parent());
    copyInto(clone);

    return clone;
}

/**
 * Creates the <UML:EnumLiteral> XMI element.
 */
void UMLEnumLiteral::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement attributeElement = UMLObject::save(QLatin1String("UML:EnumerationLiteral"), qDoc);
    qElement.appendChild(attributeElement);
}

/**
 * Loads the <UML:EnumLiteral> XMI element (empty.)
 */
bool UMLEnumLiteral::load(QDomElement& element)
{
    Q_UNUSED(element);
    return true;
}

/**
 * Display the properties configuration dialog for the enum literal.
 */
bool UMLEnumLiteral::showPropertiesDialog(QWidget* parent)
{
    bool ok;
#if QT_VERSION >= 0x050000
    QString enumName = QInputDialog::getText(parent,
                                             i18nc("enum name", "Name"), i18n("Enter name:"),
                                             QLineEdit::Normal,
                                             name(), &ok);
#else
    QString enumName = KInputDialog::getText(i18nc("enum name", "Name"), i18n("Enter name:"), name(), &ok, parent);
#endif
    if (ok && !enumName.isEmpty())  {
        setName(enumName);
        return true;
    } else {
        return false;
    }
}
