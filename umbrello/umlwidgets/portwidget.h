/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef PORTWIDGET_H
#define PORTWIDGET_H

#include "pinportbase.h"

class UMLPort;

/**
 * Defines a graphical version of the UML2 port.  Most of the functionality
 * comes from the @ref PinPortBase class from which this class inherits.
 *
 * @short A graphical version of a port on a component.
 * @author Oliver Kellogg
 * @see PinPortBase, UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class PortWidget : public PinPortBase
{
    Q_OBJECT
public:
    PortWidget(UMLScene *scene, UMLPort *d, UMLWidget *owner = 0);
    virtual ~PortWidget();

    UMLWidget* ownerWidget() const;

public slots:
    void slotMenuSelection(QAction* action);

};

#endif
