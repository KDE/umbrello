/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef PORT_H
#define PORT_H

#include "umlcanvasobject.h"

/**
 * This class contains the non-graphical information required for a UML Port.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * @short Non-graphical information for a Port.
 * @author Oliver Kellogg
 * @see UMLCanvasObject
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLPort : public UMLCanvasObject
{
    Q_OBJECT
public:

    explicit UMLPort(const QString & name = QString(), Uml::ID::Type id = Uml::ID::None);
    virtual ~UMLPort();

    virtual void init();

    virtual UMLObject* clone() const;

    void saveToXMI1(QXmlStreamWriter& writer);

protected:

    bool load1(QDomElement & element);

};

#endif
