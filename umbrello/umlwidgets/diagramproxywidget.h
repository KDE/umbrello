/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2019                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DIAGRAMPROXYWIDGET_H
#define DIAGRAMPROXYWIDGET_H

#include <basictypes.h>

#include <QPointer>
#include <QPointF>
#include <QRectF>
#include <QGraphicsSceneEvent>

class UMLWidget;
class UMLScene;
class IDChangeLog;
class QDomDocument;
class QDomElement;

class DiagramProxyWidget {
public:
    DiagramProxyWidget(UMLWidget *widget, qreal borderWidth = 5);
    virtual ~DiagramProxyWidget() {}
    Uml::ID::Type diagramLink();
    UMLScene *linkedDiagram();
    bool setDiagramLink(const Uml::ID::Type &id);

    bool isProxyWidget();
    UMLWidget *getProxiedWidget(const QPointF &p);
    QPointF mapFromClient(const QPointF &p);
    QRectF mapFromClient(const QRectF &r);
    QPointF mapToClient(const QPointF &pos);

protected:
    Uml::ID::Type m_diagramLinkId; ///< id of linked diagram
    QPointer<UMLScene> m_linkedDiagram; ///< pointer to linked diagram
    QRectF m_clientRect; ///< widget area for embedded diagram
    QRectF m_sceneRect;  ///< scene rectangle used for internal calculations
    UMLWidget *m_widget;
    qreal m_borderWidth;

    DiagramProxyWidget& operator=(const DiagramProxyWidget& other);
    bool activate(IDChangeLog* changeLog = 0);
    bool loadFromXMI1(QDomElement &qElement);
    void saveToXMI1(QDomDocument &qDoc, QDomElement &qElement);
    void setupEvent(QGraphicsSceneMouseEvent &e, QGraphicsSceneMouseEvent *event, const QPointF &pos);
    void setupEvent(QGraphicsSceneContextMenuEvent &e, QGraphicsSceneContextMenuEvent *event, const QPointF &pos);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // DIAGRAMPROXYWIDGET_H
