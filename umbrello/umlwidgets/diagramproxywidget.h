/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2019-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
class QAction;
class QDomDocument;
class QDomElement;
class QStyleOptionGraphicsItem;
class QXmlStreamWriter;

class DiagramProxyWidget {
public:
    DiagramProxyWidget(UMLWidget *widget, qreal borderWidth = 5);
    virtual ~DiagramProxyWidget() {}
    Uml::ID::Type diagramLink() const;
    UMLScene *linkedDiagram() const;
    bool setDiagramLink(const Uml::ID::Type &id);
    const QRectF &clientRect();
    const QRectF &sceneRect();
    void setClientRect(const QRectF &rect);

    bool isProxyWidget();
    UMLWidget *getProxiedWidget(const QPointF &p);
    QPointF mapFromClient(const QPointF &p);
    QRectF mapFromClient(const QRectF &r);
    QPointF mapToClient(const QPointF &pos);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    bool showLinkedDiagram() const;
    void setShowLinkedDiagram(bool showLinkedDiagram);

    QRectF iconRect() const;
    void setIconRect(const QRectF &iconRect);

protected:
    DiagramProxyWidget& operator=(const DiagramProxyWidget& other);
    bool activate(IDChangeLog* changeLog = 0);
    bool loadFromXMI1(QDomElement &qElement);
    void saveToXMI1(QXmlStreamWriter& writer);
    void setupEvent(QGraphicsSceneMouseEvent &e, QGraphicsSceneMouseEvent *event, const QPointF &pos);
    void setupEvent(QGraphicsSceneContextMenuEvent &e, QGraphicsSceneContextMenuEvent *event, const QPointF &pos);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void slotMenuSelection(QAction *action);

private:
    Uml::ID::Type m_diagramLinkId; ///< id of linked diagram
    QPointer<UMLScene> m_linkedDiagram; ///< pointer to linked diagram
    QRectF m_clientRect; ///< widget area for embedded diagram
    QRectF m_sceneRect;  ///< scene rectangle used for internal calculations
    QRectF m_iconRect; ///< widget area for diagram icon
    UMLWidget *m_widget;
    qreal m_borderWidth;
    bool m_showLinkedDiagram;
};

#endif // DIAGRAMPROXYWIDGET_H
