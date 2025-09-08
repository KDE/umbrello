/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DATATYPE_H
#define DATATYPE_H

#include "umlclassifier.h"

/**
 * This class contains the non-graphical information required for a UML Datatype.
 * This class inherits from @ref UMLClassifier which contains most
 * of the information.
 *
 * @short Non-graphical information for a Datatype.
 * @see UMLClassifier
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLDatatype : public UMLClassifier
{
    Q_OBJECT
public:
    UMLDatatype(const QString &name=QString(), Uml::ID::Type id=Uml::ID::None);
    virtual ~UMLDatatype();

    void setOriginType(UMLClassifier *origType);
    UMLClassifier * originType() const;

    void setIsReference(bool isRef = true);
    bool isReference() const;

    void setActive(bool active = true);
    bool isActive() const;

    virtual bool load1(QDomElement & element);
    virtual void saveToXMI(QXmlStreamWriter& writer);

protected:
    bool m_isRef;
    bool m_isActive;
};

#endif
