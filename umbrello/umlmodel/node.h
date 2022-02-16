/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef NODE_H
#define NODE_H

#include "umlcanvasobject.h"

/**
 * This class contains the non-graphical information required for a UML Node.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * @short Non-graphical information for a Node.
 * @author Jonathan Riddell
 * @see UMLCanvasObject
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLNode : public UMLCanvasObject
{
    Q_OBJECT
public:

    explicit UMLNode(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLNode();

    virtual void init();

    virtual UMLObject* clone() const;

    void saveToXMI(QXmlStreamWriter& writer);

protected:

    bool load1(QDomElement & element);

};

#endif
