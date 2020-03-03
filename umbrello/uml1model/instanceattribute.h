/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLINSTANCEATTRIBUTE_H
#define UMLINSTANCEATTRIBUTE_H

#include "attribute.h"
#include "basictypes.h"

/**
 * This class is used to set up information for an instanceattribute.
 * It has type, name, and default value.
 *
 * @short Sets up instanceattribute information
 * @author Lays Rodrigues
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLInstanceAttribute : public UMLAttribute
{
    Q_OBJECT
public:
    UMLInstanceAttribute(UMLObject* parent, const QString& name,
                         Uml::ID::Type id = Uml::ID::None,
                         Uml::Visibility::Enum s = Uml::Visibility::Private,
                         UMLObject *type = 0, const QString& value = QString());

    explicit UMLInstanceAttribute(UMLObject *parent);

    void setAttributes(const QString& attributes);
    QString getAttributes() const;

    virtual void saveToXMI1(QDomDocument& qDoc, QDomElement& qElement);

    virtual bool showPropertiesDialog(QWidget* parent = 0);

protected:
    virtual bool load1(QDomElement &element);

private:
    void init();
    QString m_attributes;
};

#endif // UMLINSTANCEATTRIBUTE_H
