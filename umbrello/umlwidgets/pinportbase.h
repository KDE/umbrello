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
    PinPortBase(UMLScene *scene, WidgetType type, UMLWidget *owner, Uml::ID::Type id);
    virtual ~PinPortBase();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    virtual QRectF boundingRect() const;

    virtual UMLWidget* ownerWidget() const;

    void updateWidget();
    void setName(const QString &strName);
    void moveWidgetBy(qreal diffX, qreal diffY);
    virtual void notifyParentResize();
 
    UMLWidget* onWidget(const QPointF& p);
    UMLWidget* widgetWithID(Uml::ID::Type id);

    FloatingTextWidget *floatingTextWidget();
    void setFloatingTextWidget(FloatingTextWidget *ft);

    void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);
    bool loadFromXMI(QDomElement& qElement);

public slots:
    virtual void slotMenuSelection(QAction* action);

protected:
    void init(UMLWidget *owner = 0);

private:
    enum ConnectedSide {
        Top,
        Right,
        Bottom,
        Left
    };

    bool isAboveParent() const;
    bool isBelowParent() const;
    bool isLeftOfParent() const;
    bool isRightOfParent() const;
    qreal getNewXOnJumpToTopOrBottom() const;
    void jumpToTopOfParent();
    void jumpToBottomOfParent();
    qreal getNewYOnJumpToSide() const;
    void jumpToLeftOfParent();
    void jumpToRightOfParent();

protected:
    FloatingTextWidget *m_pName;

private:
    ConnectedSide m_connectedSide;
};

#endif
