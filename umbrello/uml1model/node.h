/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLNode : public UMLCanvasObject
{
    Q_OBJECT
public:

    explicit UMLNode(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLNode();

    virtual void init();

    virtual UMLObject* clone() const;

    void SaveToXMI1(QDomDocument & qDoc, QDomElement & qElement);

protected:

    bool load1(QDomElement & element);

};

#endif
