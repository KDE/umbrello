/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef PINPORTBASE_H
#define PINPORTBASE_H

#include "umlwidget.h"

class FloatingTextWidget;

/**
 * @short Abstract base class for PinWidget and PortWidget
 * @author Oliver Kellogg
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class PinPortBase : public UMLWidget
{
    Q_OBJECT
public:
    PinPortBase(UMLScene *scene, WidgetType type, UMLObject *o);
    PinPortBase(UMLScene *scene, WidgetType type, UMLWidget *a, Uml::ID::Type id = Uml::ID::None);
    virtual ~PinPortBase();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    virtual UMLWidget* ownerWidget();
    virtual void connectOwnerMotion() = 0;
    bool hasOwner(const UMLWidget* const o) const;
    void setOwnerWidget(UMLWidget* const ownerWidget);

    void setName(const QString &strName);
    void updateWidget();
    void moveWidgetBy(qreal diffX, qreal diffY);
    void attachToOwner();
    void detachFromOwner();
 
    UMLWidget* onWidget(const QPointF& p);
    UMLWidget* widgetWithID(Uml::ID::Type id);

    FloatingTextWidget *floatingTextWidget();
    void setFloatingTextWidget(FloatingTextWidget *ft);

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    bool loadFromXMI(QDomElement& qElement);

public slots:
    void slotOwnerMoved(qreal diffX, qreal diffY);
    virtual void slotMenuSelection(QAction* action);

protected:
    void init(UMLWidget *owner = 0);

    FloatingTextWidget *m_pName;
    bool m_motionConnected;

private:
    UMLWidget* m_pOw;
};

#endif
