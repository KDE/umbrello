/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef NODEWIDGET_H
#define NODEWIDGET_H

#include "umlwidget.h"

class UMLNode;

/**
 * Defines a graphical version of the Node.  Most of the functionality
 * will come from the @ref UMLNode class.
 *
 * @short A graphical version of a Node.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class NodeWidget : public UMLWidget
{
public:

    NodeWidget(UMLScene * scene, UMLNode *n);
    virtual ~NodeWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);

    void saveToXMI(QXmlStreamWriter& writer);

protected:
    QSizeF minimumSize() const;

    static const int DEPTH = 30;  ///< pixels on Z axis
};

#endif
