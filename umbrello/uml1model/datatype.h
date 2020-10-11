/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DATATYPE_H
#define DATATYPE_H

#include "classifier.h"

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
    virtual void saveToXMI1(QDomDocument & qDoc, QDomElement & qElement);

protected:
    bool m_isRef;
    bool m_isActive;
};

#endif
