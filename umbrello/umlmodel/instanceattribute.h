/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLINSTANCEATTRIBUTE_H
#define UMLINSTANCEATTRIBUTE_H

#include "umlobject.h"

class UMLInstance;
class UMLAttribute;

/**
 * This class is used to set up information for an instanceattribute.
 * It has a pointer to the attribute which it represents and a value.
 * Member UMLObject::m_pSecondary is used for storing the pointer to the
 * UMLAttribute.
 * Member m_value is used for storing the value.
 * If the value is empty but the associated UMLAttribute has a non empty
 * initial value then the attribute initial value is copied to m_value.
 * A UMLInstanceAttribute is strictly slaved to its corresponding
 * UMLClassifier attribute.  This means that a UMLInstanceAttribute is
 * not created or removed by the user; instead, it is created or removed
 * automatically when the UMLClassifier attribute is created or removed.
 *
 * @short Sets up instanceattribute information
 * @author Lays Rodrigues
 * @author Ralf Habacker
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLInstanceAttribute : public UMLObject
{
    Q_OBJECT
public:
    UMLInstanceAttribute(UMLInstance *parent,
                         UMLAttribute *umlAttr, const QString& value = QString());

    void setAttribute(UMLAttribute *umlAttr);
    UMLAttribute *getAttribute() const;

    void setValue(const QString& value);
    QString getValue() const;

    QString toString() const;

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual bool showPropertiesDialog(QWidget* parent = 0);

protected:
    virtual bool load1(QDomElement &element);

private:
    void init();
    QString m_value;
};

#endif // UMLINSTANCEATTRIBUTE_H
