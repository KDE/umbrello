/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef FLOATINGDASHLINEWIDGET_H
#define FLOATINGDASHLINEWIDGET_H

#include "umlwidget.h"
class CombinedFragmentWidget;

#define FLOATING_DASH_LINE_MARGIN 25
#define FLOATING_DASH_LINE_TEXT_MARGIN 5

/* how many pixels a user could click around a point */
#define POINT_DELTA 5

/**
 * This class is used to draw dash lines for UML combined fragments. A FloatingDashLineWidget
 * belongs to one @ref CombinedFragmentWidget instance.
 *
 * The FloatingDashLineWidget class inherits from the @ref UMLWidget class.
 *
 * @short  A dash line for UML combined fragments.
 * @author Thomas GALLINARI <tg8187@yahoo.fr>
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class FloatingDashLineWidget : public UMLWidget
{
    Q_OBJECT
public:
    explicit FloatingDashLineWidget(UMLScene * scene, Uml::ID::Type id = Uml::ID::None, CombinedFragmentWidget *parent = nullptr);
    ~FloatingDashLineWidget();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option = nullptr, QWidget *widget = nullptr);

    void slotMenuSelection(QAction* action);

    bool onLine(const QPointF& point);

    void setText(const QString& text);

    void setY(qreal y);
    void setYMin(qreal yMin);
    void setYMax(qreal yMax);
    qreal getYMin() const;
    qreal getDiffY() const;

    void saveToXMI(QXmlStreamWriter& writer);
    bool loadFromXMI(QDomElement & qElement);

private:
    /**
     * Text associated to the dash line
     */
    QString m_text;

    /**
     * Minimum value of the Y-coordinate of the dash line
     * (= y-coordinate of the combined fragment)
     */
    qreal m_yMin;

    /**
     * Maximum value of the Y-coordinate of the dash line
     * (= y-coordinate of the combined fragment + height of the combined fragment)
     */
    qreal m_yMax;
    CombinedFragmentWidget *m_parent;
};

#endif
