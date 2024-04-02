/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef COMPONENTWIDGET_H
#define COMPONENTWIDGET_H

#include "umlwidget.h"

class UMLComponent;

#define COMPONENT_MARGIN 10

/**
 * Defines a graphical version of the Component.  Most of the functionality
 * will come from the @ref UMLComponent class.
 *
 * @short A graphical version of a Component.
 * @author Jonathan Riddell
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class ComponentWidget : public UMLWidget 
{
    Q_OBJECT
public:
    ComponentWidget(UMLScene * scene, UMLComponent *c);
    virtual ~ComponentWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    virtual void moveWidgetBy(qreal diffX, qreal diffY);
    virtual void adjustAssocs(qreal dx, qreal dy);
    virtual void adjustUnselectedAssocs(qreal dx, qreal dy);

    virtual void saveToXMI(QXmlStreamWriter& writer);

    /**
     * Emitted when the component widget is moved.
     * Provides the delta X and delta Y amount by which the widget was moved
     * relative to the previous position.
     * Slots into PortWidget::slotCompMoved()
     * @param diffX The difference between previous and new X value.
     * @param diffY The difference between previous and new Y value.
     */
    Q_SIGNAL void sigCompMoved(qreal diffX, qreal diffY);

protected:
    QSizeF minimumSize() const;

};

#endif
