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

#include "umlwidget.h"

class UMLPort;
class FloatingTextWidget;

/**
 * Defines a graphical version of the port.  Most of the functionality
 * will come from the @ref UMLWidget class from which class inherits from.
 *
 * @short A graphical version of an port.
 * @author Oliver Kellogg
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class PortWidget : public UMLWidget
{
    Q_OBJECT
public:
    PortWidget(UMLScene *scene, UMLPort *d);
    virtual ~PortWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    virtual void updateWidget();
    virtual void moveWidgetBy(qreal diffX, qreal diffY);
    void attachToOwningComponent();
 
    UMLWidget* onWidget(const QPointF& p);
    UMLWidget* widgetWithID(Uml::ID::Type id);

    FloatingTextWidget *floatingTextWidget();
    void setFloatingTextWidget(FloatingTextWidget *ft);

    bool loadFromXMI(QDomElement& qElement);
    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

public slots:
    void slotCompMoved(qreal diffX, qreal diffY);
    void slotMenuSelection(QAction* action);

protected:

    static const QSizeF FixedSize;
    FloatingTextWidget *m_pName;

};

#endif
