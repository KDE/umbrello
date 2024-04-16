/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    qreal getX() const;
    qreal getY() const;
    QPointF getPos() const;

    void updateWidget();
    void setName(const QString &strName);
    void moveWidgetBy(qreal diffX, qreal diffY);
    virtual void notifyParentResize();
 
    UMLWidget* onWidget(const QPointF& p);
    UMLWidget* widgetWithID(Uml::ID::Type id);

    FloatingTextWidget *floatingTextWidget();
    void setFloatingTextWidget(FloatingTextWidget *ft);

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement& qElement);

public Q_SLOTS:
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
