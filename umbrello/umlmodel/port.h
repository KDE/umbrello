/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    void saveToXMI(QXmlStreamWriter& writer);

protected:

    bool load1(QDomElement & element);

};

#endif
