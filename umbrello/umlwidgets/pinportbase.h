/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef PINPORTBASE_H
#define PINPORTBASE_H

#include "umlwidget.h"

#include <QScopedPointer>

class FloatingTextWidget;
class ChildWidgetPlacement;

/**
 * @short Abstract base class for PinWidget and PortWidget
 * @author Oliver Kellogg
 * @see UMLWidget
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class PinPortBase : public UMLWidget
{
    Q_OBJECT
public:
    PinPortBase(UMLScene *scene, WidgetType type, UMLWidget *owner, UMLObject *o);
    PinPortBase(UMLScene *scene, WidgetType type, UMLWidget *owner = 0, Uml::ID::Type id = Uml::ID::None);
    virtual ~PinPortBase();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    virtual QRectF boundingRect() const;

    virtual UMLWidget* ownerWidget() const;

    void setInitialPosition(const QPointF &scenePos);

    void updateWidget();
    void setName(const QString &strName);
    void moveWidgetBy(qreal diffX, qreal diffY);
    virtual void notifyParentResize();
 
    UMLWidget* onWidget(const QPointF& p);
    UMLWidget* widgetWithID(Uml::ID::Type id);

    FloatingTextWidget *floatingTextWidget();
    void setFloatingTextWidget(FloatingTextWidget *ft);

    void saveToXMI1(QXmlStreamWriter& writer);
    bool loadFromXMI1(QDomElement& qElement);

public slots:
    virtual void slotMenuSelection(QAction* action);

protected:
    void init(UMLWidget *owner = 0);
    bool activate(IDChangeLog* changeLog = 0);

private:
    ChildWidgetPlacement* createPlacement(WidgetBase::WidgetType type);

protected:
    FloatingTextWidget *m_pName;

private:
    QScopedPointer<ChildWidgetPlacement> m_childPlacement;
};

#endif
