/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef WIDGET_UTILS_H
#define WIDGET_UTILS_H

#include "basictypes.h"
#include "messagewidgetlist.h"
#include "umlwidgetlist.h"
#include "widgetbase.h"

#include <QBrush>
#include <QDomDocument>
#include <QPointF>
#include <QLineF>
#include <QPolygonF>

class QGraphicsItem;
class QGraphicsRectItem;
class QXmlStreamWriter;

/**
 * General purpose widget utilities.
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
namespace Widget_Utils
{
    UMLWidget* findWidget(Uml::ID::Type id,
                          const UMLWidgetList& widgets,
                          const MessageWidgetList* messages = 0);

    QGraphicsRectItem* decoratePoint(const QPointF& p, QGraphicsItem* parent = 0);

    void drawCrossInEllipse(QPainter *p, const QRectF& ellipse);
    void drawTriangledRect(QPainter *painter, const QRectF& rect, const QSizeF& triSize);
//    void drawArrowHead(QPainter *painter, const QPointF& arrowPos,
//                       const QSizeF& arrowSize, Qt::ArrowType arrowType,
//                       bool solid = false);
//    void drawRoundedRect(QPainter *painter, const QRectF& rect, qreal xRadius,
//            qreal yRadius, Uml::Corners corners);

    QString pointToString(const QPointF& point);
    QPointF stringToPoint(const QString& str);

    bool loadPixmapFromXMI(QDomElement &qElement, QPixmap &pixmap);
    void savePixmapToXMI(QXmlStreamWriter& stream, const QPixmap& pixmap);

    bool loadGradientFromXMI(QDomElement &qElement, QGradient *&gradient);
    void saveGradientToXMI(QXmlStreamWriter& stream, const QGradient *gradient);

    bool loadBrushFromXMI(QDomElement &qElement, QBrush &brush);
    void saveBrushToXMI(QXmlStreamWriter& stream, const QBrush& brush);

    bool hasSmallerX(const UMLWidget* widget1, const UMLWidget* widget2);
    bool hasSmallerY(const UMLWidget* widget1, const UMLWidget* widget2);

    QLineF closestPoints(const QPolygonF& self, const QPolygonF& other);

    QString defaultWidgetName(WidgetBase::WidgetType type);
    QString newTitle(WidgetBase::WidgetType type);
    QString newText(WidgetBase::WidgetType type);
    QString renameTitle(WidgetBase::WidgetType type);
    QString renameText(WidgetBase::WidgetType type);

    void ensureNestedVisible(UMLWidget *self, UMLWidgetList widgetList);
}

#endif
