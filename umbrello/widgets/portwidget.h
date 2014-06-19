/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef PORTWIDGET_H
#define PORTWIDGET_H

#include "pinportbase.h"

class UMLPort;

/**
 * Defines a graphical version of the port.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of a port on a component.
 * @author Oliver Kellogg
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class PortWidget : public PinPortBase
{
    Q_OBJECT
public:
    PortWidget(UMLScene *scene, UMLPort *d);
    virtual ~PortWidget();

    UMLWidget* ownerWidget();
    void connectOwnerMotion();

public slots:
    void slotMenuSelection(QAction* action);

};

#endif
