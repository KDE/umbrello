/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
    PortWidget(UMLScene *scene, UMLPort *d, UMLWidget *owner = nullptr);
    virtual ~PortWidget();

    UMLWidget* ownerWidget() const;

    Q_SLOT void slotMenuSelection(QAction* action);
};

#endif
